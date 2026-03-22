#!/bin/bash
#
# Update breakpad_client headers from Google Breakpad upstream.
# Extracts only client/ and common/ directories needed for crash handler integration.
# Removes tests, tools, senders, and other unused files to minimize footprint.
#

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

echo "==> Cleaning previous extraction..."
rm -rf client common *.zip breakpad-master

echo "==> Downloading breakpad master..."
curl -L -O https://github.com/google/breakpad/archive/refs/heads/master.zip

echo "==> Extracting archive..."
unzip -q master.zip

echo "==> Extracting client/ (platform handlers)..."
mkdir -p client
mv breakpad-master/src/client/ios client/
mv breakpad-master/src/client/linux client/
mv breakpad-master/src/client/mac client/
mv breakpad-master/src/client/windows client/
mv breakpad-master/src/client/*.h client/
mv breakpad-master/src/client/*.cc client/

echo "==> Extracting common/ (shared utilities)..."
mkdir -p common
mv breakpad-master/src/common/android common/
mv breakpad-master/src/common/linux common/
mv breakpad-master/src/common/mac common/
mv breakpad-master/src/common/windows common/
mv breakpad-master/src/common/*.h common/
mv breakpad-master/src/common/*.cc common/

echo "==> Removing unit tests and test infrastructure..."
find . -type f -name "*_unittest.*" -delete
find . -type f -name "*_test.*" -delete
find . -type f -name "test_assembler.*" -delete
find . -type d -name "tests" -exec rm -rf {} + 2>/dev/null || true
find . -type d -name "testapp" -exec rm -rf {} + 2>/dev/null || true
find . -type d -name "testcases" -exec rm -rf {} + 2>/dev/null || true
find . -type d -name "unittests" -exec rm -rf {} + 2>/dev/null || true

echo "==> Removing crash report senders (not used — we handle uploads in Godot)..."
rm -rf client/linux/sender
rm -rf client/mac/sender
rm -rf client/windows/sender

echo "==> Removing macOS Framework wrapper (not needed — we use the handler directly)..."
rm -rf client/mac/Framework

echo "==> Removing gcov instrumentation..."
rm -rf client/mac/gcov

echo "==> Removing iOS BreakpadController (Obj-C wrapper not used by Godot)..."
rm -f client/ios/BreakpadController.h
rm -f client/ios/BreakpadController.mm

echo "==> Removing symbol/debug-info processing (not needed at runtime)..."
# DWARF, STABS, and symbol table parsers are for post-mortem tools, not the crash client.
rm -f common/dwarf_cu_to_module.cc common/dwarf_cu_to_module.h
rm -f common/dwarf_cfi_to_module.cc common/dwarf_cfi_to_module.h
rm -f common/dwarf_line_to_module.cc common/dwarf_line_to_module.h
rm -f common/dwarf_range_list_handler.cc common/dwarf_range_list_handler.h
rm -f common/stabs_reader.cc common/stabs_reader.h
rm -f common/stabs_to_module.cc common/stabs_to_module.h
rm -f common/module.cc common/module.h
rm -f common/language.cc common/language.h
rm -f common/long_string_dictionary.cc common/long_string_dictionary.h

echo "==> Removing Linux dump_symbols and symbol upload tools..."
rm -f common/linux/dump_symbols.cc common/linux/dump_symbols.h
rm -f common/linux/synth_elf.cc common/linux/synth_elf.h
rm -f common/linux/google_crashdump_uploader.cc common/linux/google_crashdump_uploader.h
rm -f common/linux/http_upload.cc common/linux/http_upload.h

echo "==> Removing macOS dump_syms tool sources..."
rm -f common/mac/dump_syms.cc common/mac/dump_syms.h
rm -f common/mac/arch_utilities.cc common/mac/arch_utilities.h

echo "==> Removing Windows symbol processing..."
rm -f common/windows/omap.cc common/windows/omap.h
rm -f common/windows/omap_internal.h
rm -f common/windows/pdb_source_line_writer.cc common/windows/pdb_source_line_writer.h
rm -f common/windows/http_upload.cc common/windows/http_upload.h
rm -f common/windows/sym_upload_v2_protocol.cc common/windows/sym_upload_v2_protocol.h
rm -f common/windows/symbol_collector_client.cc common/windows/symbol_collector_client.h

echo "==> Removing test data and symbol files..."
rm -rf client/linux/data

echo "==> Removing Xcode projects, gyp build files, plists, xcconfig..."
find . -name "*.xcodeproj" -type d -exec rm -rf {} + 2>/dev/null || true
find . -name "*.gyp" -delete
find . -name "*.gypi" -delete
find . -name "*.plist" -delete
find . -name "*.xcconfig" -delete

echo "==> Removing misc unused files..."
rm -f common/path_helper.cc common/path_helper.h

echo "==> Cleaning up download artifacts..."
rm -f master.zip
rm -rf breakpad-master

echo "==> Counting remaining files..."
FILE_COUNT=$(find client common -type f | wc -l | tr -d ' ')
echo "==> Done. Extracted $FILE_COUNT files into client/ and common/."
