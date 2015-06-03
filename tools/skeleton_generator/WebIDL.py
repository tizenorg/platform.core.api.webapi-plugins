#
# Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import sys
from pprint import pprint

try:
    import lex
    import yacc
except ImportError:
    try:
        from ply import lex
        from ply import yacc
    except ImportError:
        print 'python ply is requred.'
        sys.exit(1)

class XObject(object):
    def __init__(self):
        self.name = ""
        self.type = ""
        self.typedChilds = dict()
        self.extras = []
        self.parent = None

    def add(self, child):
        if isinstance(child, XObject):
            type = child.type
            child.parent = self
        else:
            type = 'string'
        if not self.typedChilds.has_key(type) or not self.typedChilds[type]:
            self.typedChilds[type] = []
        self.typedChilds[type].append(child)
        #self.typedChilds.append(child)

    @property
    def childs(self):
        cs = []
        for t in self.typedChilds:
            cs += self.typedChilds[t]
        return cs

    def getTypes(self, type):
        if type in self.typedChilds:
            return self.typedChilds[type]
        else:
            return []

    def get(self, type, name):
        tcs = self.typedChilds[type]
        for o in tcs:
            if o.name == name:
                return o
        return None


class XDepthPrintable(XObject):
    def __init__(self):
        super(XDepthPrintable, self).__init__()
        self.depth = 0

    def add(self, child):
        super(XDepthPrintable, self).add(child)
        if isinstance(child, XDepthPrintable):
            child.depth += 1


class XType(XObject):
    def __init__(self, name, unions=None, array=0, nullable=None, sequence=None, unsigned=None, unrestricted=None):
        super(XType, self).__init__()
        self.array = array
        self.nullable = nullable
        self.sequence = sequence
        self.unsigned = unsigned
        self.unrestricted = unrestricted
        self.name = name

        if isinstance(name, XType):
            self.array += name.array
            self.name = name.name
            self.unsigned = name.unsigned if unsigned == None else False
            self.sequence = name.sequence if sequence == None else False
            self.nullable = name.nullable if nullable == None else False
            self.unrestricted = name.unrestricted if unrestricted == None else False

        self.union = []
        if isinstance(unions, list):
            for t in unions:
                self.union.append(t)

    def __repr__(self):
        s = ""
        if self.unsigned:
            s += 'unsigned '

        s += self.name

        if self.union:
            s += "("
            s += " or ".join([repr(u) for u in self.union])
            s += ")"
        if self.sequence:
            s = "sequence<"+s+">"
        s += "?" if self.nullable else ""
        for i in range(self.array):
            s+= "[]"
        return s

XVoid = XType('Void')

class XDefinition(XDepthPrintable):
    def __init__(self, name, type):
        super(XDefinition, self).__init__()
        self.name = name
        self.type = type

    def desc(self):
        return ""

    def __repr__(self):
        indent = "".ljust(self.depth*4)
        s = indent + "<%s:%s %s>" % (self.type, self.name, self.desc())
        for ch in self.childs:
            ss = indent.ljust(4) + ch if isinstance(ch, str) else repr(ch)
            s += "\n" + indent + ss
        return s

class XModule(XDefinition):
    def __init__(self, name):
        super(XModule, self).__init__(name, 'Module')

class XEnum(XDefinition):
    def __init__(self, name):
        super(XEnum, self).__init__(name, 'Enum')

class XTypedef(XDefinition):
    def __init__(self, name, xtype):
        super(XTypedef, self).__init__(name, 'Typedef')
        self.xtype = xtype

    def desc(self):
        return "%s = %s" % (self.name, self.xtype)

class XAttribute(XDepthPrintable):
    def __init__(self, name, xtype, inherit=False, readonly=False):
        super(XAttribute, self).__init__()
        self.type = 'Attribute'
        self.name = name
        self.inherit = inherit
        self.readonly = readonly
        self.xtype = xtype
        self.extAttr = []


    def __repr__(self):
        indent = "".ljust(self.depth*4)
        s = indent+"<Attribute: %s %s>" % (self.name, self.xtype)
        if self.readonly:
            s += " readonly"
        if self.inherit:
            s += " inherit"
        return s

class XArgument(XDepthPrintable):
    def __init__(self, name, xtype, default=None, optional=False, ellipsis=None):
        super(XArgument, self).__init__()
        self.name = name
        self.xtype = xtype
        self.default = default
        self.optional = optional
        self.ellipsis = ellipsis

    def __repr__(self):
        s = "optional" if self.optional else ""
        s += " %s" % repr(self.xtype)
        if self.ellipsis:
            s += " ..."
        s += " %s" % self.name
        if self.default:
            s += " = " + self.default

        return s

class XOperation(XDepthPrintable):
    def __init__(self, name, returnType=XVoid, arguments=[], static=False, specials=[], raises=[]):
        super(XOperation, self).__init__()
        self.type = 'Operation'
        self.name = name
        self.returnType = returnType
        self.static = static
        self.specials = specials
        self.arguments = arguments
        self.raises = raises
        self.extAttr = []

    def setStatic(self, static):
        self.static = static

    def addSpecial(self, sp):
        self.specials.append(sp)

    def __repr__(self):
        indent = "".ljust(self.depth*4)
        args = ", ".join([repr(arg) for arg in self.arguments])
        return indent+"<function %s(%s) : %s>" % (self.name, args, self.returnType)

class XExtendedAttribute(XDepthPrintable):
    def __init__(self, name, identity=None, arguments=[]):
        super(XExtendedAttribute,self).__init__()
        self.type = "ExtendedAttribute"
        self.name = name
        self.identity = identity
        self.arguments = arguments

    def __repr__(self):
        indent = "".ljust(self.depth*4)
        s = self.name
        if self.identity:
            s += " = %s" % self.identity
        if self.arguments and len(self.arguments)>0:
            s += " ("
            s += ", ".join([x.name for x in self.arguments])
            s += " )"
        return indent + "<X-Attr %s>"%s

class XInterface(XDefinition):
    def __init__(self, name, inherit=None):
        super(XInterface, self).__init__(name, 'Interface')
        self.inherit = inherit;

    def desc(self):
        return (" : %s" % self.inherit) if self.inherit else ""


class XImplements(XDefinition):
    def __init__(self, name, impl):
        super(XImplements, self).__init__(name, 'Implements')
        self.impl = impl

    def desc(self):
        return "implements %s" % self.impl

class XDictionaryMember(XDepthPrintable):
    def __init__(self, name, xtype, default):
        super(XDictionaryMember, self).__init__()
        self.name = name
        self.xtype = xtype
        self.default = default

    def __repr__(self):
        indent = "".ljust(self.depth*4)
        return indent + "<%s %s = %s>" % (self.xtype, self.name, self.default)

class XDictionary(XDefinition):
    def __init__(self, name, inherit=None):
        super(XDictionary, self).__init__(name, 'Dictionary')
        self.inherit = inherit

    def desc(self):
        return (" : %s" % self.inherit) if self.inherit else ""

class XConst(XDepthPrintable):
    def __init__(self, name, xtype, value):
        super(XConst, self).__init__()
        self.name = name
        self.xtype = xtype
        self.value = value

    def __repr__(self):
        indent = "".ljust(self.depth*4)
        return indent + "<const %s %s = %s>" % (self.xtype, self.name, self.value)

class Parser:
    def __init__(self, debug = 0, verbose = 0):
        self.tokens += self.argument_name_keyword + self.predefined_types

        self.debug = debug

        self.lex = lex.lex(module = self, debug = debug)
        self.yacc = yacc.yacc(module = self, debug = debug)

        self.verbose = verbose

    def trace(self, txt):
        if self.verbose:
            print txt

    def parse(self, webidl):
        return self.yacc.parse(webidl, debug = self.debug, lexer = self.lex)

    tokens = (
            'integer_hex',
            'integer_oct',
            'integer_dec',
            'float',
            'identifier',
            'string',
            'ellipsis',
            'scope',
            'other'
    )

    argument_name_keyword = (
            'module', # module is removed 2012 feb 07
            'raises', # raises is removed 2012 feb 07
            'setraises', # setraises is removed 2012 feb 07
            'getraises', # getraises is removed 2012 feb 07
            'inherits', # inherits is removed 2012 feb 07
            'attribute',
            'callback',
            'const',
            'creator',
            'deleter',
            'dictionary',
            'enum',
            'exception',
            'getter',
            'implements',
            'inherit',
            'interface',
            'legacycaller',
            'partial',
            'setter',
            'static',
            'stringifier',
            'typedef',
            'unrestricted',
            'readonly'
    )

    predefined_types = (
            'Date',
            'DOMString',
            'Infinity',
            'NaN',
            'any',
            'boolean',
            'byte',
            'double',
            'false',
            'long',
            'null',
            'object',
            'octet',
            'or',
            'optional',
            'sequence',
            'short',
            'true',
            'unsigned',
            'void'
    )

    literals = '-,:;<=>?()[]{}'

    def t_newline(self, t):
        r'\n+'
        t.lexer.lineno += len(t.value)

    t_ignore_whitespace = r'[\t\n\r ]+|[\t\n\r ]*((//.*|/\*.*?\*/)[\t\n\r ]*)+'

    def t_block_comment(self, t):
        r'/\*(.|\n)*?\*/'
        t.lexer.lineno += t.value.count('\n')

    t_ellipsis = r'\.\.\.'

    t_scope = r'::'

    def t_integer_hex(self, t):
        r'-?0[xX][0-9A-F-a-f]+'
        t.value = int(t.value, 16)
        return t

    def t_integer_oct(self, t):
        r'-?0[0-7]+'
        t.value = int(t.value, 8)
        return t

    def t_integer_dec(self, t):
        r'-?[1-9][0-9]*|0'
        t.value = int(t.value)
        return t

    '''
    def t_integer(self, t):
        #r'-?(0([0-7]*|[Xx][0-9A-Fa-f]+)|[1-9][0-9]*)'
        r'-?0[0-7]+|-?0[Xx][0-9A-Fa-f]+|-?[0-9][0-9]*'
        z = 0
        if t.value[0] == '-':
            z = 1

        if len(t.value) > 1 and t.value[z] == '0' and (t.value[z+1] == 'x' or t.value[z+1] == 'X'):
            if t.value[z+2:].find('8') and t.value[z+2:].find('9'):
                t.value = int(t.value, 8)
            else:
                t.value = int(t.value, 16)
        else:
            t.value = int(t.value)
        return t
    '''

    def t_float(self, t):
        r'-?(([0-9]+\.[0-9]*|[0-9]*\.[0-9]+)([Ee][+-]?[0-9]+)?|[0-9]+[Ee][+-]?[0-9]+)'
        t.value = float(t.value)
        return t

    def t_string(self, t):
        r'"[^"]*"'
        t.value = t.value[1:-1]
        return t

    def t_identifier(self, t):
        r'[A-Z_a-z][0-9A-Z_a-z]*'
        if t.value in self.argument_name_keyword + self.predefined_types:
            t.type = t.value
        if t.value == 'true':
            t.value = True
        if t.value == 'false':
            t.value = False
        if t.value == 'null':
            t.value = None
        return t

    def t_error(self, t):
        print "Illegal character '%s'" % t.value[0]
        t.lexer.skip(1)

    def p_Definitions(self, p):
        '''
        Definitions : ExtendedAttributeList Definition Definitions
                    | Empty
        '''

        if len(p) > 2:
            d = p[2]
            for extattr in p[1]:
                d.add(extattr[0])
            p[0] = [d] + p[3]
        else:
            p[0] = []

    def p_Definition(self, p):
        '''
        Definition : Module
                   | CallbackOrInterface
                   | Partial
                   | Dictionary
                   | Exception
                   | Enum
                   | Typedef
                   | ImplementsStatement
        '''
        p[0] = p[1]

    def p_Module(self, p):
        'Module : module identifier "{" Definitions "}" ";"'
        module = XModule(p[2])
        for d in p[4]:
            module.add(d)

        p[0] = module

    def p_CallbackOrInterface(self, p):
        '''
        CallbackOrInterface : callback CallbackRestOrInterface
                            | Interface
        '''
        if len(p) > 2:
            p[0] = ['callback', p[2]]
        else:
            p[0] = p[1]

    def p_CallbackRestOrInterface(self, p):
        '''
        CallbackRestOrInterface : CallbackRest
                                | Interface
        '''
        p[0] = p[1]

    def p_Interface(self, p):
        'Interface : interface identifier Inheritance "{" InterfaceMembers "}" ";"'
        interface = XInterface(p[2], p[3])
        for member in p[5]:
            interface.add(member)
        p[0] = interface

    def p_Partial(self, p):
        'Partial : partial PartialDefinition'
        p[1][0] = 'partial ' + p[1][0]
        p[0] = p[1]

    def p_PartialDefinition(self, p):
        '''
        PartialDefinition : PartialInterface
                           | PartialDictionary
        '''
        p[0] = p[1]

    def p_PartialInterface(self, p):
        'PartialInterface : interface identifier "{" InterfaceMembers "}" ";"'
        p[0] = ['interface', p[2], p[4]]

    def p_InterfaceMembers(self, p):
        '''
        InterfaceMembers : ExtendedAttributeList InterfaceMember InterfaceMembers
                         | Empty
        '''
        if len(p) > 2:
            member = p[2]
            for extattr in p[1]:
                member.add(extattr)
            p[0] = [member] + p[3]
        else:
            p[0] = []

    def p_InterfaceMember(self, p):
        '''
        InterfaceMember : Const
                        | AttributeOrOperation
        '''
        p[0] = p[1]

    def p_Dictionary(self, p):
        'Dictionary : dictionary identifier Inheritance "{" DictionaryMembers "}" ";"'
        dictionary = XDictionary(p[2], p[3])
        for m in p[5]:
            dictionary.add(m)
        p[0] = dictionary

    def p_DictionaryMembers(self, p):
        '''
        DictionaryMembers : ExtendedAttributeList DictionaryMember DictionaryMembers
                          | Empty
        '''
        if p[1] != None:
            p[0] = [p[2]] + p[3]
        else:
            p[0] = []

    def p_DictionaryMember(self, p):
        'DictionaryMember : Type identifier Default ";"'
        p[0] = XDictionaryMember(p[2], p[1], p[3])

    def p_PartialDictionary(self, p):
        'PartialDictionary : dictionary identifier "{" DictionaryMembers "}" ";"'
        p[0] = ['dictinary', p[2], p[4]]

    def p_Default(self, p):
        '''
        Default : "=" DefaultValue
                | Empty
        '''
        p[0] = p[1]

    def p_DefaultValue(self, p):
        '''
        DefaultValue : ConstValue
                     | string
        '''
        p[0] = p[1]

    def p_Exception(self, p):
        'Exception : exception identifier Inheritance "{" ExceptionMembers "}" ";"'
        p[0] = ['exception', p[2], p[5]]

    def p_ExceptionMembers(self, p):
        '''
        ExceptionMembers : ExtendedAttributeList ExceptionMember ExceptionMembers
                         | Empty
        '''
        p[0] = [p[2]] + p[3]

    def p_Inheritance(self, p):
        '''
        Inheritance : ":" identifier
                    | Empty
        '''
        if len(p) > 2:
            p[0] = p[2]
        else:
            p[0] = []

    def p_Enum(self, p):
        'Enum : enum identifier "{" EnumValueList "}" ";"'
        enum = XEnum(p[2])
        for e in p[4]:
            enum.add(e)
        p[0] = enum

    def p_EnumValueList(self, p):
        'EnumValueList : string EnumValues'
        p[0] = [p[1]] + p[2]

    def p_EnumValues(self, p):
        '''
        EnumValues : "," string EnumValues
                   | Empty
        '''
        if len(p) > 2:
            p[0] = [p[2]] + p[3]
        else:
            p[0] = []

    def p_CallbackRest(self, p):
        'CallbackRest : identifier "=" ReturnType "{" ArgumentList "}" ";"'
        p[0] = ['callback', p[1], '????']

    def p_Typedef(self, p):
        'Typedef : typedef ExtendedAttributeList Type identifier ";"'
        typedef = XTypedef(p[4], p[3])
        for exattr in p[2]:
            typedef.add(exattr)
        p[0] = typedef

    def p_ImplementsStatement(self, p):
        'ImplementsStatement : identifier implements identifier ";"'
        p[0] = XImplements(p[1], p[3])

    def p_Const(self, p):
        'Const : const ConstType identifier "=" ConstValue ";"'
        p[0] = XConst(p[3], p[2], p[5])
        #p[0] = ['const', p[3], p[5]]

    def p_ConstValue(self, p):
        '''
        ConstValue : BooleanLiteral
                   | FloatLiteral
                   | integer
                   | null
        '''
        p[0] = p[1]

    def p_BooleanLiteral(self, p):
        '''
        BooleanLiteral : true
                       | false
        '''
        p[0] = p[1]

    def p_FloatLiteral(self, p):
        '''
        FloatLiteral : float
                     | "-" Infinity
                     | Infinity
                     | NaN
        '''
        p[0] = p[1]

    def p_integer(self, p):
        '''
        integer : integer_hex
                | integer_oct
                | integer_dec
        '''
        p[0] = p[1]

    def p_AttributeOrOperation(self, p):
        '''
        AttributeOrOperation : stringifier StringifierAttributeOrOperation
                             | Attribute
                             | Operation
        '''
        if len(p) > 2:
            p[0] = p[2]
        else:
            p[0] = p[1]

    def p_StringifierAttributeOrOperation(self, p):
        '''
        StringifierAttributeOrOperation : Attribute
                                        | OperationRest
                                        | ";"
        '''
        if p[1] != ';':
            p[0] = p[1]
        else:
            pass

    def p_Attribute(self, p):
        'Attribute : Inherit ReadOnly attribute Type identifier GetRaises SetRaises Raises ";"'
        p[0] = XAttribute(p[5], p[4], p[1], p[2])

    def p_Inherit(self, p):
        '''
        Inherit : inherit
                | Empty
        '''
        p[0] = p[1]

    def p_ReadOnly(self, p):
        '''
        ReadOnly : readonly
                 | Empty
        '''
        p[0] = p[1]

    def p_Operation(self, p):
        'Operation : Qualifiers OperationRest'

        operation = p[2]
        operation.setStatic(p[1] == 'static')
        if isinstance(p[1], list):
            for sp in p[1]:
                operation.addSpecial(sp)

        p[0] = operation


    def p_Qualifiers(self, p):
        '''
        Qualifiers : static
                   | Specials
        '''
        p[0] = p[1]

    def p_Specials(self, p):
        '''
        Specials : Special Specials
                 | Empty
        '''
        if len(p) > 2:
            p[0] = p[1] + p[2]
        else:
            p[0] = []

    def p_Special(self, p):
        '''
        Special : getter
                | setter
                | creator
                | deleter
                | legacycaller
        '''
        p[0] = p[1]

    def p_GetRaises(self, p):
        '''
        GetRaises : getraises ExceptionList
                  | Empty
        '''
        if len(p) > 2:
            p[0] = p[2]
        else:
            p[0] = []
    def p_SetRaises(self, p):
        '''
        SetRaises : setraises ExceptionList
                  | Empty
        '''
        if len(p) > 2:
            p[0] = p[2]
        else:
            p[0] = []

    def p_OperationRest(self, p):
        'OperationRest : ReturnType OptionalIdentifier "(" ArgumentList ")" Raises ";"'
        p[0] = XOperation(p[2], p[1], p[4], False, [], p[6])

    def p_OptionalIdentifier(self, p):
        '''
        OptionalIdentifier : identifier
                           | Empty
        '''
        p[0] = p[1]

    def p_Raises(self, p):
        '''
        Raises : raises ExceptionList
               | Empty
        '''
        if len(p) > 2:
            p[0] = p[2]
        else:
            p[0] = []

    def p_ExceptionList(self, p):
        'ExceptionList : "(" ScopedNameList ")"'
        p[0] = p[2]

    def p_ArgumentList(self, p):
        '''
        ArgumentList : Argument Arguments
                     | Empty
        '''
        if len(p) > 2:
            p[0] = [p[1]] + p[2]
        else:
            p[0] = []

    def p_Arguments(self, p):
        '''
        Arguments : "," Argument Arguments
                  | Empty
        '''
        if len(p) > 3:
            p[0] = [p[2]] + p[3]
        else:
            p[0] = []

    def p_Argument(self, p):
        'Argument : ExtendedAttributeList OptionalOrRequiredArgument'
        p[0] = p[2]

    def p_OptionalOrRequiredArgument(self, p):
        '''
        OptionalOrRequiredArgument : optional Type ArgumentName Default
                                   | Type Ellipsis ArgumentName
        '''
        if p[1] == 'optional':
            p[0] = XArgument(p[3], p[2], p[4], True)
        else:
            p[0] = XArgument(p[3], p[1], None, False, p[2] != None)

    def p_ArgumentName(self, p):
        '''
        ArgumentName : ArgumentNameKeyword
                     | identifier
        '''
        p[0] = p[1]

    def p_Ellipsis(self, p):
        '''
        Ellipsis : ellipsis
                 | Empty
        '''
        p[0] = p[1]

    def p_ExceptionMember(self, p):
        '''
        ExceptionMember : Const
                        | ExceptionField
        '''
        p[0] = p[1]

    def p_ExceptionField(self, p):
        'ExceptionField : Type identifier ";"'
        p[0] = p[2]

    def p_ExtendedAttributeList(self, p):
        '''
        ExtendedAttributeList : "[" ExtendedAttribute ExtendedAttributes "]"
                              | Empty
        '''
        if p[1] != None:
            p[0] = [p[2]] + p[3]
        else:
            p[0] = []

    def p_ExtendedAttributes(self, p):
        '''
        ExtendedAttributes : "," ExtendedAttribute ExtendedAttributes
                           | Empty
        '''
        if p[1] != None:
            p[0] = [p[2]] + p[3]
        else:
            p[0] = []


    def p_ExtendedAttribute(self, p):
        '''
        ExtendedAttribute : "(" ExtendedAttributeInner ")" ExtendedAttributeRest
                          | "[" ExtendedAttributeInner "]" ExtendedAttributeRest
                          | "{" ExtendedAttributeInner "}" ExtendedAttributeRest
                          | ExtendedAttributeMember ExtendedAttributeRest
        '''
        if len(p) > 3:
            p[0] = [p[2]] + p[4]
        else:
            p[0] = [p[1]]
            if p[2]: p[0] += p[2]

    def p_ExtendedAttributeRest(self, p):
        '''
        ExtendedAttributeRest : ExtendedAttribute
                              | Empty
        '''
        p[0] = p[1]

    def p_ExtendedAttributeInner(self, p):
        '''
        ExtendedAttributeInner : "(" ExtendedAttributeInner ")" ExtendedAttributeInner
                               | "[" ExtendedAttributeInner "]" ExtendedAttributeInner
                               | "{" ExtendedAttributeInner "}" ExtendedAttributeInner
                               | OtherOrComma ExtendedAttributeInner
                               | Empty
        '''
        if p[1] == None:
            p[0] = []
        elif len(p) > 3:
            p[0] = [p[2]] + p[4]
        else:
            p[0] = [p[1]] + p[2]

    def p_Other(self, p):
        '''
        Other : integer
              | float
              | identifier
              | string
              | other
              | "-"
              | "."
              | ellipsis
              | ":"
              | ";"
              | "<"
              | "="
              | ">"
              | "?"
              | Date
              | DOMString
              | Infinity
              | NaN
              | any
              | boolean
              | byte
              | double
              | false
              | long
              | null
              | object
              | octet
              | or
              | optional
              | sequence
              | short
              | true
              | unsigned
              | void
              | ArgumentNameKeyword
        '''
        p[0] = p[1]


    def p_ArgumentNameKeyword(self, p):
        '''
        ArgumentNameKeyword : attribute
                            | callback
                            | const
                            | creator
                            | deleter
                            | dictionary
                            | enum
                            | exception
                            | getter
                            | implements
                            | inherit
                            | interface
                            | legacycaller
                            | partial
                            | setter
                            | static
                            | stringifier
                            | typedef
                            | unrestricted
        '''
        p[0] = p[1]

    def p_OtherOrComma(self, p):
        '''
        OtherOrComma : ExtendedAttributeMember
                     | ","
        '''
        if p[1] == ',':
            p[0] = []
        else:
            p[0] = p[1]

    def p_Type(self, p):
        '''
        Type : SingleType
             | UnionType TypeSuffix
        '''

        if len(p) > 2:
            p[0] = XType("", p[1], p[2]['array'], p[2]['nullable'])
        else:
            p[0] = p[1]

    def p_SingleType(self, p):
        '''
        SingleType : NonAnyType
                   | any TypeSuffixStartingWithArray
        '''

        if p[1] == 'any':
            p[0] = XType('any', None, p[2]['array'])
        else:
            p[0] = p[1]


    def p_UnionType(self, p):
        'UnionType : "(" UnionMemberType or UnionMemberType UnionMemberTypes ")"'

        p[0] = [p[2]] + [p[4]]
        if p[5]:
            p[0] += [p[5]]

    def p_UnionMemberType(self, p):
        '''
        UnionMemberType : NonAnyType
                        | UnionType TypeSuffix
                        | any "[" "]" TypeSuffix
        '''
        if len(p)>2:
            if p[1] == 'any':
                p[0] = XType('any', None, p[4]['array']+1, p[4]['nullable'])
            else:
                p[0] = []
        else:
            p[0] = p[1]

    def p_UnionMemberTypes(self, p):
        '''
        UnionMemberTypes : or UnionMemberType UnionMemberTypes
                         | Empty
        '''
        if len(p) > 2:
            p[0] = p[2] + p[3]
        else:
            p[0] = []

    def p_NonAnyType(self, p):
        '''
        NonAnyType : PrimitiveType TypeSuffix
                   | DOMString TypeSuffix
                   | identifier TypeSuffix
                   | sequence "<" Type ">" Null
                   | object TypeSuffix
                   | Date TypeSuffix
        '''
        sequence = False
        typename = p[1]
        nullable = False
        array = 0
        if p[1] == 'sequence':
            typename = None
            sequence = True
            nullable = p[5]
        else:
            tf = p[2]
            if tf:
                nullable = tf['nullable']
                array = tf['array']

        p[0] = XType(typename, None, array, nullable, sequence)

    def p_ConstType(self, p):
        '''
        ConstType : PrimitiveType Null
                  | identifier Null
        '''
        p[0] = p[1]

    def p_PrimitiveType(self, p):
        '''
        PrimitiveType : UnsignedIntegerType
                      | UnrestrictedFloatType
                      | boolean
                      | byte
                      | octet
        '''
        p[0] = p[1]

    def p_UnrestrictedFloatType(self, p):
        '''
        UnrestrictedFloatType : unrestricted FloatType
                              | FloatType
        '''
        if len(p) > 2:
            p[0] = p[2]
            p[0].unrestricted = True
        else:
            p[0] = p[1]

    def p_FloatType(self, p):
        '''
        FloatType : float
                  | double
        '''
        p[0] = XType(p[1], None, 0, False, False, False)

    def p_UnsignedIntegerType(self, p):
        '''
        UnsignedIntegerType : unsigned IntegerType
                            | IntegerType
        '''

        typename = p[1] if p[1] != 'unsigned' else p[2]
        p[0] = XType(typename, None, 0, False, False, p[1] == 'unsigned')

    def p_IntegerType(self, p):
        '''
        IntegerType : short
                    | long OptionalLong
        '''
        if p[1] == 'long' and p[2] != None:
            p[0] = 'long long'
        else:
            p[0] = p[1]

    def p_OptionalLong(self, p):
        '''
        OptionalLong : long
                     | Empty
        '''
        p[0] = p[1]

    def p_TypeSuffix(self, p):
        '''
        TypeSuffix : "[" "]" TypeSuffix
                   | "?" TypeSuffixStartingWithArray
                   | Empty
        '''
        p[0] = {'nullable':False, 'array':0}

        if p[1]:
            if p[1] == '?':
                p[0]['nullable'] = True
                p[0]['array'] += p[2]['array']
            if p[1] == "[" and p[2] == "]":
                p[0]['array'] += 1
                p[0]['array'] += p[3]['array']



    def p_TypeSuffixStartingWithArray(self, p):
        '''
        TypeSuffixStartingWithArray : "[" "]" TypeSuffix
                                    | Empty
        '''
        p[0] = {'array':0}
        if p[1] != None:
            p[0]['array'] += 1
            if p[3]:
                p[0]['array'] += p[3]['array']

    def p_Null(self, p):
        '''
        Null : "?"
             | Empty
        '''
        p[0] = p[1]

    def p_ReturnType(self, p):
        '''
        ReturnType : Type
                   | void
        '''
        if p[1] == 'void':
            p[0] = XVoid
        else:
            p[0] = p[1]

    def p_ScopedNameList(self, p):
        'ScopedNameList : ScopedName ScopedNames'
        p[0] = [p[1]] + p[2]

    def p_ScopedNames(self, p):
        '''
        ScopedNames : "," ScopedName ScopedNames
                    | Empty
        '''
        if len(p) > 2:
            p[0] = [p[2]] + p[3]
        else:
            p[0] = []

    def p_ScopedName(self, p):
        '''
        ScopedName : AbsoluteScopedName
                   | RelativeScopedName
        '''
        p[0] = p[1]

    def p_AbsoluteScopedName(self, p):
        'AbsoluteScopedName : scope identifier ScopedNameParts'
        p[0] = '::' + p[2] + p[3]

    def p_RelativeScopedName(self, p):
        'RelativeScopedName : identifier ScopedNameParts'
        p[0] = p[1] + p[2]

    def p_ScopedNameParts(self, p):
        '''
        ScopedNameParts : scope identifier ScopedNameParts
                        | Empty
        '''
        if len(p) > 2:
            p[0] = '::' + p[2] + p[3]
        else:
            p[0] = ""

    def p_ExtendedAttributeNoArgs(self, p):
        'ExtendedAttributeNoArgs : identifier'
        p[0] = XExtendedAttribute(p[1])

    def p_ExtendedAttributeArgList(self, p):
        'ExtendedAttributeArgList : identifier "(" ArgumentList ")"'
        p[0] = XExtendedAttribute(p[1], None, p[3])

    def p_ExtendedAttributeIdent(self, p):
        'ExtendedAttributeIdent : identifier "=" identifier'
        p[0] = XExtendedAttribute(p[1], p[3])

    def p_ExtendedAttributeNamedArgList(self, p):
        'ExtendedAttributeNamedArgList : identifier "=" identifier "(" ArgumentList ")"'
        p[0] = XExtendedAttribute(p[1], p[3], p[5])

    def p_ExtendedAttributeMember(self, p):
        '''
        ExtendedAttributeMember : ExtendedAttributeNoArgs
                                | ExtendedAttributeArgList
                                | ExtendedAttributeIdent
                                | ExtendedAttributeNamedArgList
        '''
        p[0] = p[1]

    def p_Empty(self, p):
        'Empty :'
        pass

    def p_error(self, p):
        try:
            raise Exception("Syntax error at '%s'" % p.value, p.lineno)
        except AttributeError:
            raise Exception("Syntax error")

if __name__ == '__main__':
    args = sys.argv[1:]
    if len(sys.argv) > 2:
        sys.exit()

    f = open(args[0])
    widl = f.read()
    f.close()


#    p = Parser(debug=1, verbose=1)
    p = Parser()

    tree = p.parse(widl)

    from pprint import pprint

    pprint(tree)

