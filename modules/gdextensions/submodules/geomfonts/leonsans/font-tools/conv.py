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


def dump_value(elem, indent=0):
    tabs = "  " * indent
    args_str = []
    if elem["type"] == "Literal":
        return elem["raw"]
    elif elem["type"] == "Identifier":
        return elem["name"]
    elif elem["type"] == "UnaryExpression":
        return elem["operator"] + dump_value(elem["argument"])
    elif elem["type"] == "BinaryExpression":
        return dump_value(elem["left"]) + elem["operator"] + dump_value(elem["right"])
    elif elem["type"] == "CallExpression":
        if elem["callee"]["type"] == "Identifier":
            for prop in elem["arguments"]:
                args_str.append(dump_value(prop, indent + 1))
            return "%s(%s)" % (elem["callee"]["name"], ",".join(args_str))
        elif elem["callee"]["type"] == "MemberExpression":
            # JSON.parse(JSON.stringify(DATA_UU))
            if "name" in elem["callee"]["object"] and elem["callee"]["object"]["name"] == "JSON":
                return "%s" % elem["arguments"][0]["arguments"][0]["name"]
            # JSON.parse(JSON.stringify(DATA_LH)).concat(getLatin3(-52, 9))
            elif "property" in elem["callee"] and elem["callee"]["property"]["name"] == "concat":
                if "callee" in elem["arguments"][0]:
                    return "concatPaths(%s, %s(%s, %s))" % (
                        elem["callee"]["object"]["arguments"][0]["arguments"][0]["name"],
                        elem["arguments"][0]["callee"]["name"],
                        dump_value(elem["arguments"][0]["arguments"][0]),
                        dump_value(elem["arguments"][0]["arguments"][1]),
                    )
                else:
                    return dump_value(elem["arguments"][0])
        print("Unknown call: " + elem["callee"]["type"], file=sys.stderr)
        return ""
    elif elem["type"] == "ObjectExpression":
        if len(elem["properties"]) == 2 and elem["properties"][0]["key"]["name"] == "d":
            d = elem["properties"][0]
            v = elem["properties"][1]
            args_str.append(
                tabs
                + "/* d */ %s, /* v */ %s" % (dump_value(d["value"], indent + 1), dump_value(v["value"], indent + 1))
            )
        else:
            for prop in elem["properties"]:
                args_str.append(
                    tabs + "{ %s, %s }" % (new_key_name(prop["key"]["name"]), dump_value(prop["value"], indent + 1))
                )
        return "{\n" + ",\n".join(args_str) + "}"
    elif elem["type"] == "ArrayExpression":
        for prop in elem["elements"]:
            args_str.append(dump_value(prop, indent + 1))
        return "{" + ",".join(args_str) + "}"
    print("Unknown: " + elem["type"], file=sys.stderr)
    return "?"


if sys.argv[1]:
    f = get_js_code(sys.argv[1])

    doc = parse(f)
    body = doc["body"]

    print("// AUTO-GENERATED - don't edit and use 'conv.sh' to re-generate.")
    print("// AUTO-GENERATED - %s\n" % datetime.now().strftime("%a %b %d %Y %H:%M:%S"))
    print('#include "util.h"\n')

    print("// clang-format off")

    for elem in body:
        if elem["type"] == "FunctionDeclaration":
            if elem["id"]["name"].startswith("getLatin"):
                print("std::vector<FontPath> %s(real_t x, real_t y) {" % (elem["id"]["name"]))
                for item in elem["body"]["body"]:
                    if item["type"] == "VariableDeclaration":
                        for decl in item["declarations"]:
                            print("  %s real_t %s = %s;" % (item["kind"], decl["id"]["name"], dump_value(decl["init"])))
                    elif item["type"] == "ReturnStatement":
                        print("#ifdef __clang__")
                        print("#pragma clang diagnostic push")
                        print('#pragma clang diagnostic ignored "-Wc++11-narrowing"')
                        print("#endif")
                        print("  return %s;" % (dump_value(item["argument"])))
                        print("#ifdef __clang__")
                        print("#pragma clang diagnostic pop")
                        print("#endif")
                print("}\n")
        elif elem["type"] == "VariableDeclaration":
            for decl in elem["declarations"]:
                if decl["init"]["type"] == "ObjectExpression":
                    print("const std::map<char16_t, FontData> %s = {" % decl["id"]["name"])
                    for prop in decl["init"]["properties"]:
                        print(
                            "  { u%s, %s },"
                            % (
                                "'\x80'"
                                if prop["key"]["value"] == "tofu"
                                else "'\\''"
                                if prop["key"]["value"] == "'"
                                else prop["key"]["raw"],
                                dump_value(prop["value"]),
                            )
                        )
                    print("};")
                elif decl["init"]["type"] == "ArrayExpression":
                    print("const std::vector<FontPath> %s = %s;" % (decl["id"]["name"], dump_value(decl["init"])))

    print("// clang-format on")
