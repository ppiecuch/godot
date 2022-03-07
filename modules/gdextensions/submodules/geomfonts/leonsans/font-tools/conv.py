import codecs
import os
import sys
from datetime import datetime

import pyjsparser
from pyjsparser import parse

PY3 = sys.version_info >= (3, 0)

if PY3:
    basestring = str
    long = int

def get_js_code(path):
    with codecs.open(path, "r", "utf-8") as f:
        return f.read()

def new_key_name(key):
    return "'" + key + "'"

def dump_value(elem, indent = 0):
    tabs = "  " * indent
    args_str = []
    if elem['type'] == 'Literal':
        return elem['raw']
    elif elem['type'] == 'Identifier':
        return elem['name']
    elif elem['type'] == 'UnaryExpression':
        return elem['operator'] + dump_value(elem['argument'])
    elif elem['type'] == 'BinaryExpression':
        return dump_value(elem['left']) + elem['operator'] + dump_value(elem['right'])
    elif elem['type'] == 'CallExpression':
        for prop in elem['arguments']:
            args_str.append(dump_value(prop, indent + 1))
        return "%s(%s)" % (elem['callee']['name'], ','.join(args_str))
    elif elem['type'] == 'ObjectExpression':
        if len(elem['properties']) == 2 and elem['properties'][0]['key']['name'] == 'd':
            d = elem['properties'][0]
            v = elem['properties'][1]
            args_str.append(tabs + "/* d */ %s, /* v */ %s" % (dump_value(d['value'], indent + 1), dump_value(v['value'], indent + 1)))
        else:
            for prop in elem['properties']:
                args_str.append(tabs + "{ %s, %s }" % (new_key_name(prop['key']['name']), dump_value(prop['value'], indent + 1)))
        return '{\n' + ',\n'.join(args_str) + '}'
    elif elem['type'] == 'ArrayExpression':
        for prop in elem['elements']:
            args_str.append(dump_value(prop, indent + 1))
        return '{' + ','.join(args_str) + '}'
    print("Unknown: "+elem['type'], file=sys.stderr)
    return "?"

for file in ["number.js", "upper.js", "lower.js, "latin.js", "special.js]:
    f = get_js_code("../font/" + file)

    doc = parse(f)
    body = doc['body']

    print('// AUTO-GENERATED - don\'t edit and use \'conv.py\' to re-generate.')
    print('// AUTO-GENERATED - %s\n' % datetime.now().strftime("%a %b %d %Y %H:%M:%S"));
    print('#include "util.h"\n')

    print('// clang-format off')

    for elem in body:
        if elem['type'] == 'VariableDeclaration':
            for decl in elem['declarations']:
                print("const std::map<char, FontData> %s = {" % decl['id']['name'])
                for prop in decl['init']['properties']:
                    print("  { %s, %s }," % (prop['key']['raw'], dump_value(prop['value'])))
                print("};")

    print('// clang-format on')
