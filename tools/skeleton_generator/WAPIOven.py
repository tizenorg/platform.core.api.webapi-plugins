import WebIDL
from collections import OrderedDict
from Queue import Queue
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
        'double':'double'}

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

    def prepare(self):
        self.ctx = dict()
        self.ctx['modules'] = []
        self.ctx['implementedObject'] = None
        self.ctx['implementedClass'] = None
        self.ctx['export'] = dict()
        self.ctx['callback'] = dict()
        self.ctx['dictionary'] = dict()
        self.ctx['enum'] = dict()
        self.ctx['typedef'] = dict()
        self.ctx['activeObjects'] = set()
        self.ctx['implementOperations'] = dict()
        self.ctx['exportedInterface'] = []
        self.ctx['Tizen'] = []
        self.ctx['Window'] = []
        self.ctx['cmdtable'] = dict()

        self.q = Queue()

        for m in self.tree:
            self.q.put(m)

        while self.q.qsize()>0:
            self.prepareX(self.q.get())

        for m in self.tree:
            for iface in m.getTypes('Interface'):
                if iface.inherit in [x.name for x in self.ctx['exportedInterface']]:
                    iface.exported = True
                if iface.name in self.ctx['callback']:
                    iface.isListener = True
                for operation in iface.getTypes('Operation'):
                    if hasattr(iface, 'exported') and iface.exported:
                        operation.native_cmd = iface.name+'_'+operation.name
                        operation.native_function = iface.name+(operation.name.title())
                        self.ctx['cmdtable'][operation.native_function] = operation.native_cmd
                    operation.argnames = [a.name for a in operation.arguments]
                    operation.primitiveArgs = [x for x in operation.arguments if not x.optional and x.xtype.name in cppPrimitiveMap]
                    for arg in operation.arguments:
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
        if isinstance(x, WebIDL.XImplements) and (x.name in ['Tizen', 'Window']):
            self.ctx['implementedClass'] = x.impl
            self.ctx[x.name].append(x.impl)
            impl = x.parent.get('Interface', x.impl)
            impl.implements = x.name

        if isinstance(x, WebIDL.XAttribute) and isinstance(x.parent, WebIDL.XInterface):
            if x.parent.name == self.ctx['implementedClass']:
                self.ctx['implementedObject'] = x.name
                inheritIface = x.parent.parent.get('Interface', x.xtype.name)
                if inheritIface :
                    self.ctx[x.parent.implements] = inheritIface.name
                    inheritIface.exported = True
                    self.ctx['exportedInterface'].append(inheritIface)

        if isinstance(x, WebIDL.XInterface):
            xattrs = [attr.name for attr in x.getTypes('ExtendedAttribute')]
            if 'Callback' in xattrs:
                self.ctx['callback'][x.name] = x

        if isinstance(x, WebIDL.XOperation):
            module = x.parent.parent
            if module.get('Interface', x.returnType.name):
                self.ctx['activeObjects'].add(x.returnType.name)
                inheritIface = module.get('Interface', x.returnType.name)
                if inheritIface :
                    inheritIface.exported = True
                    self.ctx['exportedInterface'].append(inheritIface)

        if isinstance(x, WebIDL.XDictionary):
            self.ctx['dictionary'][x.name] = x

        if isinstance(x, WebIDL.XTypedef):
            self.ctx['typedef'][x.name] = x.xtype

        if isinstance(x, WebIDL.XEnum):
            self.ctx['enum'][x.name] = x.childs

        if isinstance(x, WebIDL.XObject):
            for child in x.childs:
                self.q.put(child)

    TPL_API_JS = "tpl_api.js"

    def makeJSStub(self):
        self.prepare()
        tpl = self.tplEnv.get_template(Compiler.TPL_API_JS)

        return tpl.render({'modules':self.tree,
            'callbacks':self.ctx['callback'],
            'tizen': self.ctx['Tizen'],
            'window': self.ctx['Window'],
            'cmdtable' : self.ctx['cmdtable']})


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
        vals['tizen'] = self.ctx['Tizen']
        vals['window'] = self.ctx['Window']
        vals['cmdtable'] = self.ctx['cmdtable']

        return tpl.render(vals)

import sys
import getopt

def printhelp():
    print '%s [-t js|cpp] [-d outdirectory] [-m modulename] webidl' % sys.argv[0]

if __name__ == '__main__':
    argv = sys.argv[1:]
    try:
        opts, args = getopt.getopt(argv, "ht:d:")
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
        moduleName = args[0][args[0].find('/')+1:args[0].find('.')]

    f = open(args[0])
    widl = f.read()
    f.close()

    p = WebIDL.Parser()
    tree = p.parse(widl)
    c = Compiler(tree)

    jscode = c.makeJSStub()
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
