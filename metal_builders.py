"""Functions used to generate source files during build time

All such functions are invoked in a subprocess on Windows to prevent build flakiness.

"""
from platform_methods import subprocess_main

excluded_cond = ["__METAL_VERSION__"]


class LegacyMetalHeaderStruct:
    def __init__(self):
        self.attributes = []
        self.conditionals = []
        self.enums = {}

        self.included_files = []

        self.lines = []
        self.line_offset = 0


def include_file_in_legacy_metal_header(filename, header_data, depth):
    fs = open(filename, "r")
    line = fs.readline()

    while line:

        line = fs.readline()

        while line.find("#include ") != -1:
            includeline = line.replace("#include ", "").strip()[1:-1]

            import os.path

            included_file = os.path.relpath(os.path.dirname(filename) + "/" + includeline)
            if not included_file in header_data.included_files:
                header_data.included_files += [included_file]
                if include_file_in_legacy_metal_header(included_file, header_data, depth + 1) is None:
                    print("Error in file '" + filename + "': #include " + includeline + "could not be found!")

            line = fs.readline()

        if line.find("#ifdef ") != -1:
            ifdefline = line.replace("#ifdef ", "").strip()

            if line.find("_EN_") != -1:
                enumbase = ifdefline[: ifdefline.find("_EN_")]
                ifdefline = ifdefline.replace("_EN_", "_")
                line = line.replace("_EN_", "_")
                if enumbase not in header_data.enums:
                    header_data.enums[enumbase] = []
                if ifdefline not in header_data.enums[enumbase]:
                    header_data.enums[enumbase].append(ifdefline)

            elif not ifdefline in header_data.conditionals:
                if not ifdefline in excluded_cond:
                    header_data.conditionals += [ifdefline]

        line = line.replace("\r", "")
        line = line.replace("\n", "")

        header_data.lines += [line]

        line = fs.readline()
        header_data.line_offset += 1

    fs.close()

    return header_data


def build_legacy_metal_header(filename, include, class_suffix):
    header_data = LegacyMetalHeaderStruct()
    include_file_in_legacy_metal_header(filename, header_data, 0)

    out_file = filename + ".gen.h"
    fd = open(out_file, "w")

    enum_constants = []

    fd.write("/* WARNING, THIS FILE WAS GENERATED, DO NOT EDIT */\n")

    out_file_base = out_file
    out_file_base = out_file_base[out_file_base.rfind("/") + 1 :]
    out_file_base = out_file_base[out_file_base.rfind("\\") + 1 :]
    out_file_ifdef = out_file_base.replace(".", "_").upper()
    fd.write("#ifndef " + out_file_ifdef + class_suffix.upper() + "\n")
    fd.write("#define " + out_file_ifdef + class_suffix.upper() + "\n")

    out_file_class = (
        out_file_base.replace(".metal.gen.h", "").title().replace("_", "").replace(".", "") + "Shader" + class_suffix
    )
    fd.write("\n\n")
    fd.write('#include "' + include + '"\n\n\n')
    fd.write("class " + out_file_class + " : public Shader" + class_suffix + " {\n\n")
    fd.write('\t virtual String get_shader_name() const { return "' + out_file_class + '"; }\n')

    fd.write("public:\n\n")

    if header_data.conditionals:
        fd.write("\tenum Conditionals {\n")
        for x in header_data.conditionals:
            fd.write("\t\t" + x.upper() + ",\n")
        fd.write("\t};\n\n")

    fd.write("\tvirtual void init() {\n\n")

    enum_value_count = 0

    if header_data.enums:

        fd.write("\t\t//Written using math, given nonstandarity of 64 bits integer constants..\n")
        fd.write("\t\tstatic const Enum _enums[]={\n")

        bitofs = len(header_data.conditionals)
        enum_vals = []

        for xv in header_data.enums:
            x = header_data.enums[xv]
            bits = 1
            amt = len(x)
            while 2**bits < amt:
                bits += 1
            strs = "{"
            for i in range(amt):
                strs += '"#define ' + x[i] + '\\n",'

                c = {}
                c["set_mask"] = "uint64_t(" + str(i) + ")<<" + str(bitofs)
                c["clear_mask"] = (
                    "((uint64_t(1)<<40)-1) ^ (((uint64_t(1)<<" + str(bits) + ") - 1)<<" + str(bitofs) + ")"
                )
                enum_vals.append(c)
                enum_constants.append(x[i])

            strs += "nullptr}"

            fd.write(
                "\t\t\t{(uint64_t(1<<" + str(bits) + ")-1)<<" + str(bitofs) + "," + str(bitofs) + "," + strs + "},\n"
            )
            bitofs += bits

        fd.write("\t\t};\n\n")

        fd.write("\t\tstatic const EnumValue _enum_values[]={\n")

        enum_value_count = len(enum_vals)
        for x in enum_vals:
            fd.write("\t\t\t{" + x["set_mask"] + "," + x["clear_mask"] + "},\n")

        fd.write("\t\t};\n\n")

    conditionals_found = []
    if header_data.conditionals:

        fd.write("\t\tstatic const char* _conditional_strings[]={\n")
        if header_data.conditionals:
            for x in header_data.conditionals:
                fd.write('\t\t\t"#define ' + x + '\\n",\n')
                conditionals_found.append(x)
        fd.write("\t\t};\n\n")
    else:
        fd.write("\t\tstatic const char **_conditional_strings=nullptr;\n")

    fd.write("\t\tstatic const char _shader_code[]={\n")
    for x in header_data.lines:
        for c in x:
            fd.write(str(ord(c)) + ",")

        fd.write(str(ord("\n")) + ",")
    fd.write("\t\t0};\n\n")

    fd.write(
        "\t\tsetup(_conditional_strings,"
        + str(len(header_data.conditionals))
        + ",_enums,"
        + str(len(header_data.enums))
        + ",_enum_values,"
        + str(enum_value_count)
        + ",_shader_code);\n"
    )

    fd.write("\t}\n\n")

    if enum_constants:

        fd.write("\tenum EnumConditionals {\n")
        for x in enum_constants:
            fd.write("\t\t" + x.upper() + ",\n")
        fd.write("\t};\n\n")
        fd.write("\tvoid set_enum_conditional(EnumConditionals p_cond) { _set_enum_conditional(p_cond); }\n")

    fd.write("};\n\n")
    fd.write("#endif\n\n")
    fd.close()


def build_metal_headers(target, source, env):
    for x in source:
        build_legacy_metal_header(str(x), include="drivers/metal/shader_metal.h", class_suffix="Metal")


if __name__ == "__main__":
    subprocess_main(globals())
