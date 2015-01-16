# Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import WebIDL
from collections import OrderedDict
from Queue import Queue
from datetime import date
import jinja2

cppPrimitiveMap = {
        'DOMString':'std::string',
        'object':'picojson::object',
        'boolean':'bool',
        'byte':'double',
        'octet':'double',
        'short':'double',
        'long':'double',
        'long long': 'double',
        'unsigned short': 'double',
        'unsigned long': 'double',
        'unsigned long long': 'double',
        'float':'double',
        'double':'double'
        }

class IndentPrintable(object):
    def __init__(self, isIndent=True, tabstep=2, ignoreLF=False):
        self.moduleName = moduleName
        self.depth = 0
        self.buf = ''
        self.isIndent = isIndent
        self.tabstep=tabstep
        self.ignoreLF = ignoreLF

    def indepth(self):
        self.depth+=1

    def outdepth(self):
        self.depth-=1

    def indent(self):
        if self.isIndent:
            return ''.ljust(self.depth * self.tabstep)
        else:
            return ''

    def dprint(self, string):
        for s in string.split('\n'):
            self.buf += self.indent()
            self.buf += s
            if not self.ignoreLF:
                self.buf += '\n'
            else:
                self.buf += ' '

    def output(self):
        return self.buf

class Compiler(IndentPrintable):

    def __init__(self, tree, indent=True, tabstep=4, ignoreLF=False):
        super(Compiler, self).__init__(indent, tabstep, ignoreLF)
        self.tree = tree
        self.tplEnv = jinja2.Environment(
                loader=jinja2.FileSystemLoader('./tpl'),
                trim_blocks=True,
                lstrip_blocks=True)

    TODOSTR = "todo:"

    def todo(self, formatstring):
        self.dprint(("// %s " % Compiler.TODOSTR) +formatstring)

    def prepare(self, module):
        self.ctx = dict()
        self.ctx['interfaces'] = dict()
        self.ctx['modules'] = []
        self.ctx['implementedObject'] = None
        self.ctx['implementedClass'] = []
        self.ctx['export'] = dict()
        self.ctx['callback'] = dict()
        self.ctx['dictionary'] = dict()
        self.ctx['enum'] = dict()
        self.ctx['typedef'] = dict()
        self.ctx['activeObjects'] = set()
        self.ctx['implementOperations'] = dict()
        self.ctx['exportedInterface'] = []
        #self.ctx['Tizen'] = []
        #self.ctx['Window'] = []
        self.ctx['cmdtable'] = dict()
        self.ctx['exportModule'] = module

        self.q = Queue()

        for m in self.tree:
            self.q.put(m)

        while self.q.qsize()>0:
            self.prepareX(self.q.get())

        for m in [m for m in self.tree if m.name.lower() == module.lower()]:
            for iface in m.getTypes('Interface'):
                xctor = iface.constructor if hasattr(iface, 'constructor') else None
                if xctor:
                    xctor.primitiveArgs = [x for x in xctor.arguments if x.xtype.name in cppPrimitiveMap]
                    attributes = iface.getTypes('Attribute')
                    ctorArgs = [arg.name for arg in xctor.arguments] if xctor else []
                    for attr in attributes:
                        if attr.name in ctorArgs:
                            attr.existIn = 'ctor'
                        if xctor:
                            for x in xctor.arguments:
                                if x.xtype.name in self.ctx['interfaces'] or x.xtype.name in self.ctx['dictionary']:
                                    argtypes = dict(self.ctx['interfaces'].items() + self.ctx['dictionary'].items())
                                    argtype = argtypes[x.xtype.name]
                                    for child in argtype.childs:
                                        if child.name == attr.name:
                                            attr.existIn = x.name + "." + child.name
                                    break
                    if len([x for x in attributes if not hasattr(x, "existIn")]) > 0:
                        native_function = iface.name + "Constructor"
                        native_cmd = iface.name + "_constructor"
                        self.ctx['cmdtable'][native_function] = native_cmd
                if iface.inherit in [x.name for x in self.ctx['exportedInterface']]:
                    iface.private = True
                #if iface.name in self.ctx['callback']:
                    #iface.isListener = True

                for operation in iface.getTypes('Operation'):
                    if hasattr(iface, 'exported') and iface.exported:
                        #operation.native_cmd = iface.name+'_'+operation.name
                        #operation.native_function = iface.name+(operation.name.title())
                        #self.ctx['cmdtable'][operation.native_function] = operation.native_cmd
                        native_cmd = iface.name+'_'+operation.name
                        native_function = iface.name+(operation.name.title())
                        self.ctx['cmdtable'][native_function] = native_cmd
                    operation.argnames = [a.name for a in operation.arguments]
                    operation.primitiveArgs = [x for x in operation.arguments if x.xtype.name in cppPrimitiveMap]
                    for arg in operation.arguments:
                        if arg.xtype.name in self.ctx['interfaces'] and \
                            hasattr(self.ctx['interfaces'][arg.xtype.name], 'constructor'):
                            arg.isPlatformObject = True

                        if arg.xtype.name in self.ctx['enum']:
                            arg.isEnum = True
                            arg.enums = self.ctx['enum'][arg.xtype.name]
                        if arg.xtype.name in self.ctx['typedef']:
                            arg.isTypedef = True
                            t = self.ctx['typedef'][arg.xtype.name]
                            if t.union:
                                union_names = [t.name for t in t.union]
                                arg.isEnum = reduce(lambda x, y: x & y, [ x in self.ctx['enum'] for x in union_names])
                                if arg.isEnum :
                                    arg.enums = reduce(lambda x,y: x+y, [ self.ctx['enum'][x] for x in self.ctx['enum'] if x in union_names])
                                arg.isTypes = reduce(lambda x, y: x & y, [ x in m.getTypes('Interface') for x in union_names])
                        if arg.xtype.name in self.ctx['callback']:
                            callback = self.ctx['callback'][arg.xtype.name]
                            if callback.functionOnly:
                                arg.functionOnly = True
                            arg.isListener = True
                            arg.listenerType = self.ctx['callback'][arg.xtype.name]
                            operation.async = True
                            m.async = True
                            if arg.listenerType.get('Operation', 'onsuccess'):
                                arg.listenerType.callbackType = 'success'
                            elif arg.listenerType.get('Operation', 'onerror'):
                                arg.listenerType.callbackType = 'error'

                    for xiface in self.ctx['exportedInterface']:
                        if operation.returnType.name == xiface.name:
                            operation.returnInternal = xiface
                            break

    def prepareX(self, x):
        if isinstance(x, WebIDL.XImplements) and (x.name in ['Tizen', 'Window']) and x.parent.name.lower() == self.ctx['exportModule'].lower():
            self.ctx['implementedClass'].append(x.impl)
            #self.ctx[x.name].append(x.impl)
            impl = x.parent.get('Interface', x.impl)
            impl.implements = x.name

        if isinstance(x, WebIDL.XAttribute) and isinstance(x.parent, WebIDL.XInterface):
            if x.parent.name in self.ctx['implementedClass']:
                self.ctx['implementedObject'] = x.name
                #inheritIface = x.parent.parent.get('Interface', x.xtype.name)
                inheritIface = self.ctx['interfaces'][x.xtype.name]
                if inheritIface :
                    #self.ctx[x.parent.implements] = inheritIface.name
                    inheritIface.exported = x.parent.implements
                    self.ctx['exportedInterface'].append(inheritIface)

        if isinstance(x, WebIDL.XInterface):
            xcallback = next((a for a in x.getTypes('ExtendedAttribute') if a.name == 'Callback'), None)
            if xcallback:
                self.ctx['callback'][x.name] = x
            x.functionOnly = xcallback and xcallback.identity == 'FunctionOnly'
            self.ctx['interfaces'][x.name] = x;

            xctor = next((c for c in x.getTypes('ExtendedAttribute') if c.name == 'Constructor'), None)
            if xctor:
                x.constructor = xctor
                x.exported = x.name

        if isinstance(x, WebIDL.XOperation):
            module = x.parent.parent
            #if module.get('Interface', x.returnType.name):
            if x.returnType.name in self.ctx['interfaces']:
                self.ctx['activeObjects'].add(x.returnType.name)
                #inheritIface = module.get('Interface', x.returnType.name)
                inheritIface = self.ctx['interfaces'][x.returnType.name]
                if inheritIface :
                    inheritIface.exported = inheritIface.name
                    self.ctx['exportedInterface'].append(inheritIface)

        if isinstance(x, WebIDL.XDictionary):
            self.ctx['dictionary'][x.name] = x

        if isinstance(x, WebIDL.XTypedef):
            self.ctx['typedef'][x.name] = x.xtype

        if isinstance(x, WebIDL.XEnum):
            self.ctx['enum'][x.name] = x.childs
            cppPrimitiveMap[x.name] = 'std::string'

        if isinstance(x, WebIDL.XObject):
            for child in x.childs:
                self.q.put(child)

    TPL_API_JS = "tpl_api.js"

    def makeJSStub(self, module):
        self.prepare(module)
        tpl = self.tplEnv.get_template(Compiler.TPL_API_JS)

        modules = self.tree if module == None else [m for m in self.tree if m.name.lower() == module.lower()]
        return tpl.render({'modules':modules,
            'callbacks':self.ctx['callback'],
            #'tizen': self.ctx['Tizen'],
            #'window': self.ctx['Window'],
            'cmdtable' : self.ctx['cmdtable'],
            'year' : date.today().year})


    TPL_EXTENSION_H = "tpl_extension.h"
    TPL_EXTENSION_CC = "tpl_extension.cc"
    TPL_INSTANCE_H = "tpl_instance.h"
    TPL_INSTANCE_CC = "tpl_instance.cc"

    def makeCppStubs(self, module):
        extension_h = self.makeCppStub(module, Compiler.TPL_EXTENSION_H)
        extension_cc = self.makeCppStub(module, Compiler.TPL_EXTENSION_CC)
        instance_h = self.makeCppStub(module, Compiler.TPL_INSTANCE_H)
        instance_cc = self.makeCppStub(module, Compiler.TPL_INSTANCE_CC)

        return {
                module+'_extension.h':extension_h,
                module+'_extension.cc':extension_cc,
                module+'_instance.h':instance_h,
                module+'_instance.cc':instance_cc}

    def makeCppStub(self, module, tpl_file):
        tpl = self.tplEnv.get_template(tpl_file)
        vals = dict()
        vals['module'] = {
                'upper':module.upper(),
                'lower':module.lower(),
                'title':module.title()
        }


        #m = [m for m in self.tree if m.name.lower() == module.lower()]
        m = [m for m in self.tree]
        vals['moduleObj'] = m[0] if len(m)>0 else None
        #vals['tizen'] = self.ctx['Tizen']
        #vals['window'] = self.ctx['Window']
        vals['cmdtable'] = self.ctx['cmdtable']
        vals['year'] = date.today().year

        return tpl.render(vals)

import sys
import getopt

def printhelp():
    print '%s [-t js|cpp] [-d outdirectory] [-m modulename] webidl' % sys.argv[0]

if __name__ == '__main__':
    argv = sys.argv[1:]
    try:
        opts, args = getopt.getopt(argv, "htm:d:")
    except getopt.GetoptError:
        printhelp()
        sys.exit(2)

    outDirectory = None
    only = None
    moduleName = None

    for opt, arg in opts:
        if opt == '-h':
            printhelp()
            sys.exit()
        elif opt == '-t':
            only = arg
        elif opt == '-d':
            outDirectory = arg
        elif opt == '-m':
            moduleName = arg

    if len(args)<1:
        printhelp()
        sys.exit(2)

    if not moduleName:
        import os
        basename = os.path.basename(args[0])
        moduleName = basename[0:basename.find('.')]

    widl = ""
    for arg in args:
        f = open(arg)
        widl += f.read()
        f.close()

    p = WebIDL.Parser()
    tree = p.parse(widl)
    c = Compiler(tree)

    jscode = c.makeJSStub(moduleName)
    cppcode = c.makeCppStubs(moduleName)

    if outDirectory:
        if only != 'cpp':
            f = open("%s/%s_api.js" % (outDirectory, moduleName), 'w')
            f.write(jscode)
            f.close()
        if only != 'js':
            for cc in cppcode:
                f = open("%s/%s" % (outDirectory, cc), 'w')
                f.write(cppcode[cc])
                f.close()
    else:
        if only != 'cpp':
            print jscode
            pass

        if only != 'js':
            for cc in cppcode:
                print cppcode[cc]
