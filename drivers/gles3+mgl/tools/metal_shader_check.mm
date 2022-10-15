/*************************************************************************/
/*  metal_shader_check.mm                                                */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

// Reference:
// ----------
// https://github.com/marrony/metal-compute-pipeline/blob/master/main.m
// https://github.com/codecat/ccpp/blob/master/ccpp.h
// https://github.com/DoughIt/Preprocessor/blob/master/src/Source/Preprocessor.cpp
// https://github.com/colinw7/CPrePro/blob/master/src/CPrePro.cpp
// https://github.com/SManufact/RetroArch/blob/master/gfx/common/metal/Context.m
// https://github.com/robovm/apple-ios-samples/blob/master/MetalShaderShowcase/MetalShaderShowcase/AAPLRenderer.mm
// https://github.com/egomeh/floaty-boaty-go-go/blob/master/src/Graphics/shaderpreprocessor.cpp
// https://github.com/ConfettiFX/The-Forge/blob/master/Common_3/Renderer/Metal/MetalShaderReflection.mm
//
// Building:
// ---------
// g++ -std=c++11 -o metal_shader_check metal_shader_check.mm -framework Metal -framework Foundation

#include <unistd.h>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#import <Metal/Metal.h>
#import <TargetConditionals.h>

std::vector<std::string> search_paths;
std::map<std::string, std::vector<std::string>> processed_files;

typedef std::tuple<std::string, long, long, long, std::string> uniform_info_t; // name, buffer, offset, size, godot type hint

std::vector<std::string> conditionals;
std::vector<uniform_info_t> uniforms; // info
size_t uniform_buffer_size = 0;
std::vector<std::pair<std::string, long>> attributes; // name, index
std::vector<std::pair<std::string, std::string>> texunits;

std::vector<std::string> shader_lines;

bool output_attribs = true;

static inline NSString *ec(const char *src) {
	return [NSString stringWithCString:src encoding:[NSString defaultCStringEncoding]];
}
static inline std::string ce(NSString *str) {
	return std::string([str UTF8String]);
}

static std::istream &safe_get_line(std::istream &is, std::string &t) {
	t.clear();
	// The characters in the stream are read one-by-one using a std::streambuf.
	// That is faster than reading them one-by-one using the std::istream.
	// Code that uses streambuf this way must be guarded by a sentry object.
	// The sentry object performs various tasks,
	// such as thread synchronization and updating the stream state.
	std::istream::sentry se(is, true);
	std::streambuf *sb = is.rdbuf();

	for (;;) {
		int c = sb->sbumpc();
		switch (c) {
			case '\n':
				return is;
			case '\r':
				if (sb->sgetc() == '\n')
					sb->sbumpc();
				return is;
			case EOF:
				// Also handle the case when the last line has no line ending
				if (t.empty())
					is.setstate(std::ios::eofbit);
				return is;
			default:
				t += (char)c;
		}
	}
}

static std::vector<std::string> read_all(const std::string &file_name) {
	std::vector<std::string> all;
	std::ifstream infile(file_name);
	if (!infile.is_open()) {
		for (const auto &p : search_paths) {
			infile.open(p + file_name);
			if (infile.is_open()) {
				break;
			}
		}
	}
	if (!infile.is_open()) {
		NSLog(@"*** %s: cannot open file", file_name.c_str());
		exit(EXIT_FAILURE);
	}
	for (std::string line; safe_get_line(infile, line);) {
		if (line.size() > 0) {
			all.push_back(line);
		}
	}
	return all;
}

static std::string mtlDataTypeToString(MTLDataType mtlType) {
	switch (mtlType) {
		case MTLDataTypeNone:
			return "None";
		case MTLDataTypeStruct:
			return "Struct";
		case MTLDataTypeArray:
			return "Array";
		case MTLDataTypeFloat:
			return "Float";
		case MTLDataTypeFloat2:
			return "Float2";
		case MTLDataTypeFloat3:
			return "Float3";
		case MTLDataTypeFloat4:
			return "Float4";
		case MTLDataTypeFloat2x2:
			return "Float2x2";
		case MTLDataTypeFloat2x3:
			return "Float2x3";
		case MTLDataTypeFloat2x4:
			return "Float2x4";
		case MTLDataTypeFloat3x2:
			return "Float3x2";
		case MTLDataTypeFloat3x3:
			return "Float3x3";
		case MTLDataTypeFloat3x4:
			return "Float3x4";
		case MTLDataTypeFloat4x2:
			return "Float4x2";
		case MTLDataTypeFloat4x3:
			return "Float4x3";
		case MTLDataTypeFloat4x4:
			return "Float4x4";
		case MTLDataTypeHalf:
			return "Half";
		case MTLDataTypeHalf2:
			return "Half2";
		case MTLDataTypeHalf3:
			return "Half3";
		case MTLDataTypeHalf4:
			return "Half4";
		case MTLDataTypeHalf2x2:
			return "Half2x2";
		case MTLDataTypeHalf2x3:
			return "Half2x3";
		case MTLDataTypeHalf2x4:
			return "Half2x4";
		case MTLDataTypeHalf3x2:
			return "Half3x2";
		case MTLDataTypeHalf3x3:
			return "Half3x3";
		case MTLDataTypeHalf3x4:
			return "Half3x4";
		case MTLDataTypeHalf4x2:
			return "Half4x2";
		case MTLDataTypeHalf4x3:
			return "Half4x3";
		case MTLDataTypeHalf4x4:
			return "Half4x4";
		case MTLDataTypeInt:
			return "Int";
		case MTLDataTypeInt2:
			return "Int2";
		case MTLDataTypeInt3:
			return "Int3";
		case MTLDataTypeInt4:
			return "Int4";
		case MTLDataTypeUInt:
			return "UInt";
		case MTLDataTypeUInt2:
			return "UInt2";
		case MTLDataTypeUInt3:
			return "UInt3";
		case MTLDataTypeUInt4:
			return "UInt4";
		case MTLDataTypeShort:
			return "Short";
		case MTLDataTypeShort2:
			return "Short2";
		case MTLDataTypeShort3:
			return "Short3";
		case MTLDataTypeShort4:
			return "Short4";
		case MTLDataTypeUShort:
			return "UShort";
		case MTLDataTypeUShort2:
			return "UShort2";
		case MTLDataTypeUShort3:
			return "UShort3";
		case MTLDataTypeUShort4:
			return "UShort4";
		case MTLDataTypeChar:
			return "Char";
		case MTLDataTypeChar2:
			return "Char2";
		case MTLDataTypeChar3:
			return "Char3";
		case MTLDataTypeChar4:
			return "Char4";
		case MTLDataTypeUChar:
			return "UChar";
		case MTLDataTypeUChar2:
			return "UChar2";
		case MTLDataTypeUChar3:
			return "UChar3";
		case MTLDataTypeUChar4:
			return "UChar4";
		case MTLDataTypeBool:
			return "Bool";
		case MTLDataTypeBool2:
			return "Bool2";
		case MTLDataTypeBool3:
			return "Bool3";
		case MTLDataTypeBool4:
			return "Bool4";
		case MTLDataTypeTexture:
			return "Texture";
		case MTLDataTypeSampler:
			return "Sampler";
		case MTLDataTypePointer:
			return "Pointer";

#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 110000
		case MTLDataTypeR8Unorm:
			return "R8Unorm";
		case MTLDataTypeR8Snorm:
			return "R8Snorm";
		case MTLDataTypeR16Unorm:
			return "R16Unorm";
		case MTLDataTypeR16Snorm:
			return "R16Snorm";
		case MTLDataTypeRG8Unorm:
			return "RG8Unorm";
		case MTLDataTypeRG8Snorm:
			return "RG8Snorm";
		case MTLDataTypeRG16Unorm:
			return "RG16Unorm";
		case MTLDataTypeRG16Snorm:
			return "RG16Snorm";
		case MTLDataTypeRGBA8Unorm:
			return "RGBA8Unorm";
		case MTLDataTypeRGBA8Unorm_sRGB:
			return "RGBA8Unorm_sRGB";
		case MTLDataTypeRGBA8Snorm:
			return "RGBA8Snorm";
		case MTLDataTypeRGBA16Unorm:
			return "RGBA16Unorm";
		case MTLDataTypeRGBA16Snorm:
			return "RGBA16Snorm";
		case MTLDataTypeRGB10A2Unorm:
			return "RGB10A2Unorm";
		case MTLDataTypeRG11B10Float:
			return "RG11B10Float";
		case MTLDataTypeRGB9E5Float:
			return "RGB9E5Float";
		case MTLDataTypeComputePipeline:
			return "ComputePipeline";
		case MTLDataTypeVisibleFunctionTable:
			return "VisibleFunctionTable";
		case MTLDataTypeIntersectionFunctionTable:
			return "IntersectionFunctionTable";
		case MTLDataTypePrimitiveAccelerationStructure:
			return "PrimitiveAccelerationStructure";
		case MTLDataTypeInstanceAccelerationStructure:
			return "InstanceAccelerationStructure";
#endif

#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 110000
		case MTLDataTypeLong:
			return "Long";
		case MTLDataTypeLong2:
			return "Long2";
		case MTLDataTypeLong3:
			return "Long3";
		case MTLDataTypeLong4:
			return "Long4";
		case MTLDataTypeULong:
			return "ULong";
		case MTLDataTypeULong2:
			return "ULong2";
		case MTLDataTypeULong3:
			return "ULong3";
		case MTLDataTypeULong4:
			return "ULong4";
#endif

		case MTLDataTypeIndirectCommandBuffer:
			return "IndirectCommandBuffer";
		case MTLDataTypeRenderPipeline:
			return "RenderPipeline";
	}
	return "<unknown>";
}

static int sizeMtlDataType(MTLDataType mtlType) {
	// bytes:
	const int HALF_SIZE = 4;
	const int FLOAT_SIZE = 4;
	const int INT_SIZE = 4;
	switch (mtlType) {
		case MTLDataTypeFloat:
			return FLOAT_SIZE;
		case MTLDataTypeFloat2:
			return FLOAT_SIZE * 2;
		case MTLDataTypeFloat3:
			return FLOAT_SIZE * 3;
		case MTLDataTypeFloat4:
			return FLOAT_SIZE * 4;
		case MTLDataTypeFloat2x2:
			return FLOAT_SIZE * 2 * 2;
		case MTLDataTypeFloat2x3:
			return FLOAT_SIZE * 2 * 3;
		case MTLDataTypeFloat2x4:
			return FLOAT_SIZE * 2 * 4;
		case MTLDataTypeFloat3x2:
			return FLOAT_SIZE * 3 * 2;
		case MTLDataTypeFloat3x3:
			return FLOAT_SIZE * 3 * 3;
		case MTLDataTypeFloat3x4:
			return FLOAT_SIZE * 3 * 4;
		case MTLDataTypeFloat4x2:
			return FLOAT_SIZE * 4 * 2;
		case MTLDataTypeFloat4x3:
			return FLOAT_SIZE * 4 * 3;
		case MTLDataTypeFloat4x4:
			return FLOAT_SIZE * 4 * 4;
		case MTLDataTypeHalf:
			return HALF_SIZE;
		case MTLDataTypeHalf2:
			return HALF_SIZE * 2;
		case MTLDataTypeHalf3:
			return HALF_SIZE * 3;
		case MTLDataTypeHalf4:
			return HALF_SIZE * 4;
		case MTLDataTypeHalf2x2:
			return HALF_SIZE * 2 * 2;
		case MTLDataTypeHalf2x3:
			return HALF_SIZE * 2 * 3;
		case MTLDataTypeHalf2x4:
			return HALF_SIZE * 2 * 4;
		case MTLDataTypeHalf3x2:
			return HALF_SIZE * 3 * 2;
		case MTLDataTypeHalf3x3:
			return HALF_SIZE * 3 * 3;
		case MTLDataTypeHalf3x4:
			return HALF_SIZE * 3 * 4;
		case MTLDataTypeHalf4x2:
			return HALF_SIZE * 4 * 2;
		case MTLDataTypeHalf4x3:
			return HALF_SIZE * 4 * 3;
		case MTLDataTypeHalf4x4:
			return HALF_SIZE * 4 * 4;
		case MTLDataTypeInt:
			return INT_SIZE;
		case MTLDataTypeInt2:
			return INT_SIZE * 2;
		case MTLDataTypeInt3:
			return INT_SIZE * 3;
		case MTLDataTypeInt4:
			return INT_SIZE * 4;
		case MTLDataTypeUInt:
			return INT_SIZE;
		case MTLDataTypeUInt2:
			return INT_SIZE * 2;
		case MTLDataTypeUInt3:
			return INT_SIZE * 3;
		case MTLDataTypeUInt4:
			return INT_SIZE * 4;
		case MTLDataTypeShort:
			return HALF_SIZE;
		case MTLDataTypeShort2:
			return HALF_SIZE * 2;
		case MTLDataTypeShort3:
			return HALF_SIZE * 3;
		case MTLDataTypeShort4:
			return HALF_SIZE * 4;
		case MTLDataTypeUShort:
			return HALF_SIZE;
		case MTLDataTypeUShort2:
			return HALF_SIZE * 2;
		case MTLDataTypeUShort3:
			return HALF_SIZE * 3;
		case MTLDataTypeUShort4:
			return HALF_SIZE * 4;
		case MTLDataTypeChar:
			return 1;
		case MTLDataTypeChar2:
			return 2;
		case MTLDataTypeChar3:
			return 3;
		case MTLDataTypeChar4:
			return 4;
		case MTLDataTypeUChar:
			return 1;
		case MTLDataTypeUChar2:
			return 2;
		case MTLDataTypeUChar3:
			return 3;
		case MTLDataTypeUChar4:
			return 4;
		case MTLDataTypeBool:
			return 1;
		case MTLDataTypeBool2:
			return 2;
		case MTLDataTypeBool3:
			return 3;
		case MTLDataTypeBool4:
			return 4;
		case MTLDataTypeTexture:
			return 8;
		case MTLDataTypePointer:
			return 8;
		default: {
			NSLog(@"Unsupported Metal type: %ld", long(mtlType));
			exit(EXIT_FAILURE);
		}
	}
	return -1;
}

std::string mtlTypeToVariantHint(MTLDataType mtlType) {
	switch (mtlType) {
		case MTLDataTypeFloat:
			return "Variant::REAL";
		case MTLDataTypeFloat2:
			return "Variant::VECTOR2";
		case MTLDataTypeFloat3:
			return "Variant::VECTOR3";
		case MTLDataTypeFloat4:
			return "Variant::POOL_REAL_ARRAY";
		case MTLDataTypeFloat2x2:
			return "Variant::TRANSFORM2D";
		case MTLDataTypeFloat2x3:
			return "";
		case MTLDataTypeFloat2x4:
			return "";
		case MTLDataTypeFloat3x2:
			return "";
		case MTLDataTypeFloat3x3:
			return "Variant::BASIS";
		case MTLDataTypeFloat3x4:
			return "";
		case MTLDataTypeFloat4x2:
			return "";
		case MTLDataTypeFloat4x3:
			return "";
		case MTLDataTypeFloat4x4:
			return "Variant::TRANSFORM";
		case MTLDataTypeInt:
			return "Variant::INT";
		case MTLDataTypeInt2:
			return "Variant::POOL_INT_ARRAY";
		case MTLDataTypeInt3:
			return "Variant::POOL_INT_ARRAY";
		case MTLDataTypeInt4:
			return "Variant::POOL_INT_ARRAY";
		case MTLDataTypeUInt:
			return "Variant::INT";
		case MTLDataTypeUInt2:
			return "Variant::POOL_INT_ARRAY";
		case MTLDataTypeUInt3:
			return "Variant::POOL_INT_ARRAY";
		case MTLDataTypeUInt4:
			return "Variant::POOL_INT_ARRAY";
		case MTLDataTypeShort:
			return "Variant::INT";
		case MTLDataTypeShort2:
			return "Variant::POOL_INT_ARRAY";
		case MTLDataTypeShort3:
			return "Variant::POOL_INT_ARRAY";
		case MTLDataTypeShort4:
			return "Variant::POOL_INT_ARRAY";
		case MTLDataTypeUShort:
			return "";
		case MTLDataTypeUShort2:
			return "Variant::POOL_INT_ARRAY";
		case MTLDataTypeUShort3:
			return "Variant::POOL_INT_ARRAY";
		case MTLDataTypeUShort4:
			return "Variant::POOL_INT_ARRAY";
		case MTLDataTypeChar:
			return "Variant::INT";
		case MTLDataTypeChar2:
			return "Variant::POOL_BYTE_ARRAY";
		case MTLDataTypeChar3:
			return "Variant::POOL_BYTE_ARRAY";
		case MTLDataTypeChar4:
			return "Variant::POOL_BYTE_ARRAY";
		case MTLDataTypeUChar:
			return "Variant::INT";
		case MTLDataTypeUChar2:
			return "Variant::POOL_BYTE_ARRAY";
		case MTLDataTypeUChar3:
			return "Variant::POOL_BYTE_ARRAY";
		case MTLDataTypeUChar4:
			return "Variant::POOL_BYTE_ARRAY";
		case MTLDataTypeBool:
			return "Variant::BOOL";
		case MTLDataTypeBool2:
			return "Variant::POOL_BYTE_ARRAY";
		case MTLDataTypeBool3:
			return "Variant::POOL_BYTE_ARRAY";
		case MTLDataTypeBool4:
			return "Variant::POOL_BYTE_ARRAY";
		default: {
			NSLog(@"Unsupported Metal type: %ld", long(mtlType));
			exit(EXIT_FAILURE);
		}
	}
}

/// MAIN

int main(int argc, const char *argv[]) {
	// skip over program name
	--argc;
	++argv;

	if (argc < 1) {
		NSLog(@"*** Missing argument for file shader path");
		exit(EXIT_FAILURE);
	}

	std::string src = "";
	std::string input_file(argv[0]);
	std::ifstream infile(input_file);
	if (!infile.is_open()) {
		NSLog(@"*** %s: file not found/cannot open", input_file.c_str());
		exit(EXIT_FAILURE);
	}
	std::vector<std::string> macros;

	std::regex include_expression("^#include.*\"([^\"]+)\".*$");
	std::regex define_expression("^#ifdef\\s+([a-zA-Z_][a-zA-Z0-9_]+).*$");

	std::smatch matches;
	int line_count = 0;
	for (std::string line; safe_get_line(infile, line);) {
		if (std::regex_match(line, matches, include_expression)) {
			if (matches.size() == 2) {
				std::string include_file = matches[1];
				if (processed_files.count(include_file) == 0) {
					processed_files[include_file] = read_all(include_file);
				}
				src += "#line 1 \"" + include_file + "\"\n";
				line_count++;
				for (const auto &ln : processed_files[include_file]) {
					src += ln + "\n";
					line_count++;
					shader_lines.push_back(ln + "\n");
				}
				line_count++;
				src += "#line 1 \"" + input_file + "\"\n";
				continue;
			}
		} else if (std::regex_match(line, matches, define_expression)) {
			conditionals.push_back(matches[1]);
		}
		line_count++;
		src += line + "\n";
		shader_lines.push_back(line + "\n");
	}

	id<MTLDevice> device = MTLCreateSystemDefaultDevice();
	if (!device) {
		NSArray<id<MTLDevice>> *devices = MTLCopyAllDevices();
		if (devices) {
			NSLog(@"   |                                |           | C           C|");
			NSLog(@"   |                                |           | o    A      a|");
			NSLog(@"   |                                |           | m    p      t|");
			NSLog(@"Nr | Name                           | Low power | m    p    M a|");
			NSLog(@"   |                                |           | o    l    a l|");
			NSLog(@"   |                                |           | n    e    c s|");
			NSLog(@"   |                                |           |12312345671212|");
			NSLog(@"---+--------------------------------+-----------+--------------+");
			for (int i = 0; i < devices.count; ++i) {
				if (devices[i]) {
					const int FamilyCapsEnums = 14;
					char family_str[FamilyCapsEnums + 1] = "______________";
					if (@available(macOS 10.15, *)) {
						static MTLGPUFamily family[FamilyCapsEnums] = { MTLGPUFamilyCommon1, MTLGPUFamilyCommon2, MTLGPUFamilyCommon3, MTLGPUFamilyApple1, MTLGPUFamilyApple2, MTLGPUFamilyApple3, MTLGPUFamilyApple4, MTLGPUFamilyApple5, MTLGPUFamilyApple6, MTLGPUFamilyApple7, MTLGPUFamilyMac1, MTLGPUFamilyMac2, MTLGPUFamilyMacCatalyst1, MTLGPUFamilyMacCatalyst2 };
						for (int f = 0; f < FamilyCapsEnums; ++f) {
							if (family[f] == -1) {
								family_str[f] = '?';
							} else if ([devices[i] supportsFamily:family[f]]) {
								family_str[f] = '.';
							} else {
								family_str[f] = ' ';
							}
						}
					}
					if (!device || !devices[i].lowPower) {
						device = devices[i];
					}
					NSLog(@"%2d | %30s |       %3s |%s|", i, [devices[i].name UTF8String], devices[i].lowPower ? "yes" : "no", family_str);
				}
			}
		}
	}
	if (!device) {
		NSLog(@"Failed to created Metal device - not available or not supported");
		exit(EXIT_FAILURE);
	}

	NSError *errors = nil;
	id<MTLLibrary> lib = [device newLibraryWithSource:ec(src.c_str()) options:nil error:&errors];
	if (!lib) {
		NSLog(@"[Failed to created library from source] %@", errors ? [errors localizedDescription] : @"unknown");
		exit(EXIT_FAILURE);
	}
	if (errors && lib) {
		NSLog(@"[Library created with warning] %@", [errors localizedDescription]);
	}
	id<MTLFunction> vertexProgram = [lib newFunctionWithName:ec("vertexFunction")];
	if (!vertexProgram) {
		NSLog(@"Failed to create vertex program");
		NSLog(@"Shader program: %s", src.c_str());
		exit(EXIT_FAILURE);
	}
	id<MTLFunction> fragmentProgram = [lib newFunctionWithName:ec("fragmentFunction")];
	if (!fragmentProgram) {
		NSLog(@"Failed to create fragment program");
		NSLog(@"Shader program: %s", src.c_str());
		exit(EXIT_FAILURE);
	}

	MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
	pipelineStateDescriptor.label = @"DummyPipeline";
	[pipelineStateDescriptor setVertexFunction:vertexProgram];
	[pipelineStateDescriptor setFragmentFunction:fragmentProgram];
	pipelineStateDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
	// added dummy topology
	pipelineStateDescriptor.inputPrimitiveTopology = MTLPrimitiveTopologyClassPoint;

	MTLRenderPipelineReflection *reflection;
	MTLPipelineOption option = MTLPipelineOptionBufferTypeInfo | MTLPipelineOptionArgumentInfo;
	id<MTLRenderPipelineState> pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor options:option reflection:&reflection error:&errors];
	if (errors || !pipelineState) {
		NSLog(@"Failed to created Metal pipeline state: %@", [errors localizedDescription]);
		exit(EXIT_FAILURE);
	}

	for (MTLArgument *arg in reflection.vertexArguments) {
		NSLog(@"Found arguments: %@ index: %ld size: %ld\n", arg.name, long(arg.index), long(arg.bufferDataSize));
		attributes.push_back(std::make_pair(ce(arg.name), arg.index));
		if (arg.bufferDataType == MTLDataTypeStruct) {
			uniform_buffer_size = arg.bufferDataSize;
			for (MTLStructMember *uniform in arg.bufferStructType.members) {
				NSLog(@"  member: %@ type: %s, location: %ld", uniform.name, mtlDataTypeToString(uniform.dataType).c_str(), long(uniform.offset));
				if (uniform.dataType == MTLDataTypeArray) {
					// array
				} else if (uniform.dataType == MTLDataTypeStruct) {
					// struct
				} else {
					uniforms.push_back(std::make_tuple(ce(uniform.name), long(arg.index), long(uniform.offset), long(sizeMtlDataType(uniform.dataType)), mtlTypeToVariantHint(uniform.dataType)));
				}
			}
		}
	}

	return 0;
}
