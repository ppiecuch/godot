#!/usr/bin/env python3
"""
Nakama v1 API code generator.

Generates C++ FlatBuffers and Protocol Buffers code from api.proto/api.fbs
schema definitions, then applies post-generation patches for Godot compatibility.

Usage:
    python3 build.py [--flatbuffers-only] [--protobuf-only] [--skip-compile]

Requirements:
    - flatc binary in thirdparty/bin/flatbuffers-1_12/ (shared module tools)
    - protoc binary (optional, for protobuf generation -- searched in PATH
      or specify via PROTOC env var)
"""

import os
import re
import sys
import shutil
import argparse
import subprocess

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

# FlatBuffers tools (shared across modules in thirdparty/bin/)
GDEXT_DIR = os.path.normpath(os.path.join(SCRIPT_DIR, "..", "..", ".."))
FLATB_DIR = os.path.join(GDEXT_DIR, "thirdparty", "bin", "flatbuffers-1_12")
FLATC = os.path.join(FLATB_DIR, "flatc")
FLATB_INCLUDE = os.path.join(FLATB_DIR, "include")

# Generated output files
API_FBS = os.path.join(SCRIPT_DIR, "api.fbs")
API_PROTO = os.path.join(SCRIPT_DIR, "api.proto")
API_GENERATED_H = os.path.join(SCRIPT_DIR, "api_generated.h")
API_GENERATED_CPP = os.path.join(SCRIPT_DIR, "api_generated.cpp")
V1_PROTO_DIR = os.path.join(SCRIPT_DIR, "v1_proto")


def run(cmd, **kwargs):
    """Run a subprocess command, raising on failure."""
    print(f"  > {' '.join(cmd)}")
    subprocess.check_call(cmd, **kwargs)


def patch_file(filepath, replacements):
    """Apply a list of (pattern, replacement) regex substitutions to a file."""
    with open(filepath, "r") as f:
        content = f.read()

    for pattern, replacement in replacements:
        content = re.sub(pattern, replacement, content)

    with open(filepath, "w") as f:
        f.write(content)


def patch_file_insert_after(filepath, needle, lines_to_insert):
    """Insert lines after every occurrence of needle in the file."""
    with open(filepath, "r") as f:
        content = f.read()

    insert_text = "\n".join(lines_to_insert)
    content = content.replace(needle, needle + "\n" + insert_text)

    with open(filepath, "w") as f:
        f.write(content)


# ---- FlatBuffers generation ------------------------------------------------

def generate_flatbuffers():
    """Generate C++ code from FlatBuffers schema."""
    print("*** FlatBuffers generation")

    if not os.path.isfile(FLATC):
        sys.exit(f"Error: flatc not found at {FLATC}")

    # Generate .fbs from .proto if it doesn't exist
    if not os.path.isfile(API_FBS):
        print("  Generating api.fbs from api.proto ...")
        run([FLATC, "--proto", API_PROTO], cwd=SCRIPT_DIR)

    # Generate C++ from .fbs
    print("  Generating C++ from api.fbs ...")
    run([
        FLATC, "--cpp",
        "--force-empty-vectors",
        "--scoped-enums",
        "--oneof-union",
        "--gen-all",
        "--gen-object-api",
        "--gen-name-strings",
        "--cpp-str-type", "CharString",
        "--cpp-str-flex-ctor",
        "--cpp-dictionary-api",
        API_FBS,
    ], cwd=SCRIPT_DIR)

    # Post-generation patches on api_generated.h
    print("  Patching api_generated.h ...")

    # Rename anonymous union types to meaningful names.
    # Uses plain string replacement (not regex) to match the original sed behavior:
    # e.g. Anonymous0Builder -> AuthenticateMethodBuilder
    anonymous_renames = {
        "Anonymous0": "AuthenticateMethod",
        "Anonymous1": "AuthenticateResult",
        "Anonymous2": "EnvelopeContent",
        "Anonymous9": "TopicType",
        "AuthenticateResult5": "ScoreOperator",
    }
    with open(API_GENERATED_H, "r") as f:
        content = f.read()
    # Apply in order: longest match first to avoid partial replacements
    for old, new in sorted(anonymous_renames.items(), key=lambda x: -len(x[0])):
        content = content.replace(old, new)
    with open(API_GENERATED_H, "w") as f:
        f.write(content)

    # Fix _timezone macro conflict (POSIX defines _timezone)
    patch_file(API_GENERATED_H, [
        (r"namespace server \{", "#undef _timezone\n\nnamespace server {"),
    ])

    # Rename 'assert' to 'assertion' to avoid macro conflicts
    # Use word boundary to avoid replacing 'assertion' itself
    patch_file(API_GENERATED_H, [
        (r"\bassert\b", "assertion"),
    ])

    # Generate the standalone compilation stub
    print("  Generating api_generated.cpp ...")
    cpp_content = """\
#include "flatbuffers/flatbuffers.h"
#include <string>
#include <map>
struct Variant {
\tVariant() { }
\ttemplate <typename T> Variant(const T &dummy) { }
};
using CharString = std::string;
using Array = std::vector<Variant>;
using Dictionary = std::map<std::string, Variant>;
#include "api_generated.h"
"""
    with open(API_GENERATED_CPP, "w") as f:
        f.write(cpp_content)


def compile_flatbuffers():
    """Compile the generated FlatBuffers C++ to verify correctness."""
    print("  Compiling api_generated.cpp (verification) ...")
    run([
        "g++", "--std=c++11", "-c",
        "-I", FLATB_INCLUDE,
        API_GENERATED_CPP,
    ], cwd=SCRIPT_DIR)


# ---- Protocol Buffers generation -------------------------------------------

def find_protoc():
    """Find protoc binary - check env var, then PATH."""
    env_protoc = os.environ.get("PROTOC")
    if env_protoc and os.path.isfile(env_protoc):
        return env_protoc
    return shutil.which("protoc")


def generate_protobuf():
    """Generate C++ code from Protocol Buffers schema."""
    print("*** Protocol Buffers generation")

    protoc = find_protoc()
    if not protoc:
        print("  Warning: protoc not found. Skipping protobuf generation.")
        print("  Set PROTOC env var or install protobuf to enable.")
        return False

    # Clean and create output directory
    if os.path.isdir(V1_PROTO_DIR):
        shutil.rmtree(V1_PROTO_DIR)
    os.makedirs(V1_PROTO_DIR, exist_ok=True)

    # Generate C++ from .proto
    print(f"  Using protoc: {protoc}")
    run([protoc, f"--cpp_out={V1_PROTO_DIR}", API_PROTO], cwd=SCRIPT_DIR)

    # Patch: add assert macro workaround
    print("  Patching v1_proto/api.pb.h ...")
    pb_h = os.path.join(V1_PROTO_DIR, "api.pb.h")
    patch_file_insert_after(pb_h, "@@protoc_insertion_point(includes)", [
        '#pragma push_macro("assert")',
        "#undef assert",
    ])

    return True


def compile_protobuf():
    """Compile the generated protobuf C++ to verify correctness."""
    protoc = find_protoc()
    if not protoc:
        return

    # Infer include path from protoc location (../include relative to bin/)
    protoc_dir = os.path.dirname(os.path.realpath(protoc))
    proto_include = os.path.join(os.path.dirname(protoc_dir), "include")

    if not os.path.isdir(proto_include):
        print(f"  Warning: protobuf include dir not found at {proto_include}, skipping compile.")
        return

    pb_cc = os.path.join(V1_PROTO_DIR, "api.pb.cc")
    if os.path.isfile(pb_cc):
        print("  Compiling v1_proto/api.pb.cc (verification) ...")
        run([
            "g++", "-c", "--std=c++11",
            "-I", proto_include,
            pb_cc,
        ], cwd=SCRIPT_DIR)


# ---- Main ------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="Nakama v1 API code generator")
    parser.add_argument("--flatbuffers-only", action="store_true",
                        help="Only generate FlatBuffers code")
    parser.add_argument("--protobuf-only", action="store_true",
                        help="Only generate Protocol Buffers code")
    parser.add_argument("--skip-compile", action="store_true",
                        help="Skip compilation verification step")
    args = parser.parse_args()

    os.chdir(SCRIPT_DIR)

    if not args.protobuf_only:
        generate_flatbuffers()
        if not args.skip_compile:
            compile_flatbuffers()

    if not args.flatbuffers_only:
        if generate_protobuf() and not args.skip_compile:
            compile_protobuf()

    print("\n*** Done.")


if __name__ == "__main__":
    main()
