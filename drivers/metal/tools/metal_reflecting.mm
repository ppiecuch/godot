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
// g++ -o metal_reflecting metal_reflecting.mm -framework Metal -framework Foundation

#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <regex>

#import <Metal/Metal.h>

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

static void build_metal_header(const std::string &filename, const std::string &include, const std::string &class_suffix);

static inline std::string up(std::string str) { std::transform(str.begin(), str.end(), str.begin(), ::toupper); return str; }

static inline NSString* ec(const char* src) { return [NSString stringWithCString:src encoding:[NSString defaultCStringEncoding]]; }
static inline std::string ce(NSString *str) { return std::string([str UTF8String]); }

static inline std::string rp(std::string str, char from_char, char to_char) {
	std::replace(str.begin(), str.end(), from_char, to_char);
	return str;
}

static std::string rp(std::string str, const std::string& from_str, const std::string& to_str) {
	size_t start_pos = 0;
	while((start_pos = str.find(from_str, start_pos)) != std::string::npos) {
		str.replace(start_pos, from_str.length(), to_str);
		start_pos += to_str.length(); // handles case where 'to' is a substring of 'from'
	}
	return str;
}

static std::string title(std::string str) {
	str[0] = ::toupper(str[0]);
	for (int i = 1; i < str.size() - 1; i++) {
		if (str[i - 1] == '_') {
			str[i] = ::toupper(str[i]);
		} else {
			str[i] = ::tolower(str[i]);
		}
	}
	return str;
}

static std::istream& safe_get_line(std::istream& is, std::string& t) {
	t.clear();
	// The characters in the stream are read one-by-one using a std::streambuf.
	// That is faster than reading them one-by-one using the std::istream.
	// Code that uses streambuf this way must be guarded by a sentry object.
	// The sentry object performs various tasks,
	// such as thread synchronization and updating the stream state.
	std::istream::sentry se(is, true);
	std::streambuf* sb = is.rdbuf();

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

static inline bool file_exists (const std::string& name) {
	return ( access( name.c_str(), F_OK ) != -1 );
}

static std::vector<std::string> read_all(const std::string &file_name) {
	std::vector<std::string> all;
	std::ifstream infile(file_name);
	if(!infile.is_open()) {
		for(const auto &p : search_paths) {
			infile.open(p + file_name);
			if (infile.is_open()) {
				break;
			}
		}
	}
	if(!infile.is_open()) {
		NSLog(@"*** %s: cannot open file", file_name.c_str());
		exit(EXIT_FAILURE);
	}
	for (std::string line; safe_get_line(infile, line);) {
		if(line.size() > 0) {
			all.push_back(line);
		}
	}
	return all;
}


static std::string mtlDataTypeToString(MTLDataType mtlType) {
	switch (mtlType) {
		case MTLDataTypeNone: return "None";
		case MTLDataTypeStruct: return "Struct";
		case MTLDataTypeArray: return "Array";
		case MTLDataTypeFloat: return "Float";
		case MTLDataTypeFloat2: return "Float2";
		case MTLDataTypeFloat3: return "Float3";
		case MTLDataTypeFloat4: return "Float4";
		case MTLDataTypeFloat2x2: return "Float2x2";
		case MTLDataTypeFloat2x3: return "Float2x3";
		case MTLDataTypeFloat2x4: return "Float2x4";
		case MTLDataTypeFloat3x2: return "Float3x2";
		case MTLDataTypeFloat3x3: return "Float3x3";
		case MTLDataTypeFloat3x4: return "Float3x4";
		case MTLDataTypeFloat4x2: return "Float4x2";
		case MTLDataTypeFloat4x3: return "Float4x3";
		case MTLDataTypeFloat4x4: return "Float4x4";
		case MTLDataTypeHalf: return "Half";
		case MTLDataTypeHalf2: return "Half2";
		case MTLDataTypeHalf3: return "Half3";
		case MTLDataTypeHalf4: return "Half4";
		case MTLDataTypeHalf2x2: return "Half2x2";
		case MTLDataTypeHalf2x3: return "Half2x3";
		case MTLDataTypeHalf2x4: return "Half2x4";
		case MTLDataTypeHalf3x2: return "Half3x2";
		case MTLDataTypeHalf3x3: return "Half3x3";
		case MTLDataTypeHalf3x4: return "Half3x4";
		case MTLDataTypeHalf4x2: return "Half4x2";
		case MTLDataTypeHalf4x3: return "Half4x3";
		case MTLDataTypeHalf4x4: return "Half4x4";
		case MTLDataTypeInt: return "Int";
		case MTLDataTypeInt2: return "Int2";
		case MTLDataTypeInt3: return "Int3";
		case MTLDataTypeInt4: return "Int4";
		case MTLDataTypeUInt: return "UInt";
		case MTLDataTypeUInt2: return "UInt2";
		case MTLDataTypeUInt3: return "UInt3";
		case MTLDataTypeUInt4: return "UInt4";
		case MTLDataTypeShort: return "Short";
		case MTLDataTypeShort2: return "Short2";
		case MTLDataTypeShort3: return "Short3";
		case MTLDataTypeShort4: return "Short4";
		case MTLDataTypeUShort: return "UShort";
		case MTLDataTypeUShort2: return "UShort2";
		case MTLDataTypeUShort3: return "UShort3";
		case MTLDataTypeUShort4: return "UShort4";
		case MTLDataTypeChar: return "Char";
		case MTLDataTypeChar2: return "Char2";
		case MTLDataTypeChar3: return "Char3";
		case MTLDataTypeChar4: return "Char4";
		case MTLDataTypeUChar: return "UChar";
		case MTLDataTypeUChar2: return "UChar2";
		case MTLDataTypeUChar3: return "UChar3";
		case MTLDataTypeUChar4: return "UChar4";
		case MTLDataTypeBool: return "Bool";
		case MTLDataTypeBool2: return "Bool2";
		case MTLDataTypeBool3: return "Bool3";
		case MTLDataTypeBool4: return "Bool4";
		case MTLDataTypeTexture: return "Texture";
		case MTLDataTypeSampler: return "Sampler";
		case MTLDataTypePointer: return "Pointer";
		case MTLDataTypeR8Unorm: return "R8Unorm";
		case MTLDataTypeR8Snorm: return "R8Snorm";
		case MTLDataTypeR16Unorm: return "R16Unorm";
		case MTLDataTypeR16Snorm: return "R16Snorm";
		case MTLDataTypeRG8Unorm: return "RG8Unorm";
		case MTLDataTypeRG8Snorm: return "RG8Snorm";
		case MTLDataTypeRG16Unorm: return "RG16Unorm";
		case MTLDataTypeRG16Snorm: return "RG16Snorm";
		case MTLDataTypeRGBA8Unorm: return "RGBA8Unorm";
		case MTLDataTypeRGBA8Unorm_sRGB: return "RGBA8Unorm_sRGB";
		case MTLDataTypeRGBA8Snorm: return "RGBA8Snorm";
		case MTLDataTypeRGBA16Unorm: return "RGBA16Unorm";
		case MTLDataTypeRGBA16Snorm: return "RGBA16Snorm";
		case MTLDataTypeRGB10A2Unorm: return "RGB10A2Unorm";
		case MTLDataTypeRG11B10Float: return "RG11B10Float";
		case MTLDataTypeRGB9E5Float: return "RGB9E5Float";
		case MTLDataTypeRenderPipeline: return "RenderPipeline";
		case MTLDataTypeComputePipeline: return "ComputePipeline";
		case MTLDataTypeIndirectCommandBuffer: return "IndirectCommandBuffer";
		case MTLDataTypeVisibleFunctionTable: return "VisibleFunctionTable";
		case MTLDataTypeIntersectionFunctionTable: return "IntersectionFunctionTable";
		case MTLDataTypePrimitiveAccelerationStructure: return "PrimitiveAccelerationStructure";
		case MTLDataTypeInstanceAccelerationStructure: return "InstanceAccelerationStructure";
	}
	return "<unknown>";
}

static int sizeMtlDataType(MTLDataType mtlType) {
	// bytes:
	const int HALF_SIZE = 4;
	const int FLOAT_SIZE = 4;
	const int INT_SIZE = 4;
	switch (mtlType) {
		case MTLDataTypeFloat: return FLOAT_SIZE;
		case MTLDataTypeFloat2: return FLOAT_SIZE * 2;
		case MTLDataTypeFloat3: return FLOAT_SIZE * 3;
		case MTLDataTypeFloat4: return FLOAT_SIZE * 4;
		case MTLDataTypeFloat2x2: return FLOAT_SIZE * 2 * 2;
		case MTLDataTypeFloat2x3: return FLOAT_SIZE * 2 * 3;
		case MTLDataTypeFloat2x4: return FLOAT_SIZE * 2 * 4;
		case MTLDataTypeFloat3x2: return FLOAT_SIZE * 3 * 2;
		case MTLDataTypeFloat3x3: return FLOAT_SIZE * 3 * 3;
		case MTLDataTypeFloat3x4: return FLOAT_SIZE * 3 * 4;
		case MTLDataTypeFloat4x2: return FLOAT_SIZE * 4 * 2;
		case MTLDataTypeFloat4x3: return FLOAT_SIZE * 4 * 3;
		case MTLDataTypeFloat4x4: return FLOAT_SIZE * 4 * 4;
		case MTLDataTypeHalf: return HALF_SIZE;
		case MTLDataTypeHalf2: return HALF_SIZE * 2;
		case MTLDataTypeHalf3: return HALF_SIZE * 3;
		case MTLDataTypeHalf4: return HALF_SIZE * 4;
		case MTLDataTypeHalf2x2: return HALF_SIZE * 2 * 2;
		case MTLDataTypeHalf2x3: return HALF_SIZE * 2 * 3;
		case MTLDataTypeHalf2x4: return HALF_SIZE * 2 * 4;
		case MTLDataTypeHalf3x2: return HALF_SIZE * 3 * 2;
		case MTLDataTypeHalf3x3: return HALF_SIZE * 3 * 3;
		case MTLDataTypeHalf3x4: return HALF_SIZE * 3 * 4;
		case MTLDataTypeHalf4x2: return HALF_SIZE * 4 * 2;
		case MTLDataTypeHalf4x3: return HALF_SIZE * 4 * 3;
		case MTLDataTypeHalf4x4: return HALF_SIZE * 4 * 4;
		case MTLDataTypeInt: return INT_SIZE;
		case MTLDataTypeInt2: return INT_SIZE * 2;
		case MTLDataTypeInt3: return INT_SIZE * 3;
		case MTLDataTypeInt4: return INT_SIZE * 4;
		case MTLDataTypeUInt: return INT_SIZE;
		case MTLDataTypeUInt2: return INT_SIZE * 2;
		case MTLDataTypeUInt3: return INT_SIZE * 3;
		case MTLDataTypeUInt4: return INT_SIZE * 4;
		case MTLDataTypeShort: return HALF_SIZE;
		case MTLDataTypeShort2: return HALF_SIZE * 2;
		case MTLDataTypeShort3: return HALF_SIZE * 3;
		case MTLDataTypeShort4: return HALF_SIZE * 4;
		case MTLDataTypeUShort: return HALF_SIZE;
		case MTLDataTypeUShort2: return HALF_SIZE * 2;
		case MTLDataTypeUShort3: return HALF_SIZE * 3;
		case MTLDataTypeUShort4: return HALF_SIZE * 4;
		case MTLDataTypeChar: return 1;
		case MTLDataTypeChar2: return 2;
		case MTLDataTypeChar3: return 3;
		case MTLDataTypeChar4: return 4;
		case MTLDataTypeUChar: return 1;
		case MTLDataTypeUChar2: return 2;
		case MTLDataTypeUChar3: return 3;
		case MTLDataTypeUChar4: return 4;
		case MTLDataTypeBool: return 1;
		case MTLDataTypeBool2: return 2;
		case MTLDataTypeBool3: return 3;
		case MTLDataTypeBool4: return 4;
		case MTLDataTypeTexture: return 8;
		case MTLDataTypePointer: return 8;
		default: {
			NSLog(@"Unsupported Metal type: %ld", long(mtlType));
			exit(EXIT_FAILURE);
		}
	}
	return -1;
}

std::string mtlTypeToVariantHint(MTLDataType mtlType) {
	switch(mtlType) {
		case MTLDataTypeFloat: return "Variant::REAL";
		case MTLDataTypeFloat2: return "Variant::VECTOR2";
		case MTLDataTypeFloat3: return "Variant::VECTOR3";
		case MTLDataTypeFloat4: return "Variant::POOL_REAL_ARRAY";
		case MTLDataTypeFloat2x2: return "Variant::TRANSFORM2D";
		case MTLDataTypeFloat2x3: return "";
		case MTLDataTypeFloat2x4: return "";
		case MTLDataTypeFloat3x2: return "";
		case MTLDataTypeFloat3x3: return "Variant::BASIS";
		case MTLDataTypeFloat3x4: return "";
		case MTLDataTypeFloat4x2: return "";
		case MTLDataTypeFloat4x3: return "";
		case MTLDataTypeFloat4x4: return "Variant::TRANSFORM";
		case MTLDataTypeInt: return "Variant::INT";
		case MTLDataTypeInt2: return "Variant::POOL_INT_ARRAY";
		case MTLDataTypeInt3: return "Variant::POOL_INT_ARRAY";
		case MTLDataTypeInt4: return "Variant::POOL_INT_ARRAY";
		case MTLDataTypeUInt: return "Variant::INT";
		case MTLDataTypeUInt2: return "Variant::POOL_INT_ARRAY";
		case MTLDataTypeUInt3: return "Variant::POOL_INT_ARRAY";
		case MTLDataTypeUInt4: return "Variant::POOL_INT_ARRAY";
		case MTLDataTypeShort: return "Variant::INT";
		case MTLDataTypeShort2: return "Variant::POOL_INT_ARRAY";
		case MTLDataTypeShort3: return "Variant::POOL_INT_ARRAY";
		case MTLDataTypeShort4: return "Variant::POOL_INT_ARRAY";
		case MTLDataTypeUShort: return "";
		case MTLDataTypeUShort2: return "Variant::POOL_INT_ARRAY";
		case MTLDataTypeUShort3: return "Variant::POOL_INT_ARRAY";
		case MTLDataTypeUShort4: return "Variant::POOL_INT_ARRAY";
		case MTLDataTypeChar: return "Variant::INT";
		case MTLDataTypeChar2: return "Variant::POOL_BYTE_ARRAY";
		case MTLDataTypeChar3: return "Variant::POOL_BYTE_ARRAY";
		case MTLDataTypeChar4: return "Variant::POOL_BYTE_ARRAY";
		case MTLDataTypeUChar: return "Variant::INT";
		case MTLDataTypeUChar2: return "Variant::POOL_BYTE_ARRAY";
		case MTLDataTypeUChar3: return "Variant::POOL_BYTE_ARRAY";
		case MTLDataTypeUChar4: return "Variant::POOL_BYTE_ARRAY";
		case MTLDataTypeBool: return "Variant::BOOL";
		case MTLDataTypeBool2: return "Variant::POOL_BYTE_ARRAY";
		case MTLDataTypeBool3: return "Variant::POOL_BYTE_ARRAY";
		case MTLDataTypeBool4: return "Variant::POOL_BYTE_ARRAY";
		default: {
			NSLog(@"Unsupported Metal type: %ld", long(mtlType));
			exit(EXIT_FAILURE);
		}
	}
}

/// MAIN

int main(int argc, const char *argv[]) {
	// skip over program name
	--argc; ++argv;

	if (argc < 1) {
		NSLog(@"*** Missing argument for file shader path");
		exit(EXIT_FAILURE);
	}

	std::string src = "";
	std::string input_file(argv[0]);
	std::ifstream infile(input_file);
	if(!infile.is_open()) {
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
				src += "#line 1 \"" + include_file + "\"\n"; line_count++;
				for (const auto &ln : processed_files[include_file]) {
					src += ln + "\n"; line_count++; shader_lines.push_back(ln + "\n");
				}
				line_count++; src += "#line 1 \"" + input_file + "\"\n";
				continue;
			}
		} else if (std::regex_match(line, matches, define_expression)) {
			conditionals.push_back(matches[1]);
		}
		line_count++; src += line + "\n"; shader_lines.push_back(line + "\n");
	}

	id<MTLDevice> device = MTLCreateSystemDefaultDevice();

	NSError *errors = nil;
	id<MTLLibrary> lib = [device newLibraryWithSource: ec(src.c_str()) options:nil error:&errors];
	if (errors && !lib) {
		NSLog(@"[Failed to created library from source] %@", [errors localizedDescription]);
		exit(EXIT_FAILURE);
	}
	if (errors && lib) {
		NSLog(@"[Library created with warning] %@", [errors localizedDescription]);
	}
	id<MTLFunction> vertexProgram = [lib newFunctionWithName: ec("vertexFunction")];
	if (!vertexProgram) {
		NSLog(@"Failed to create vertex program");
		exit(EXIT_FAILURE);
	}
	id<MTLFunction> fragmentProgram = [lib newFunctionWithName: ec("fragmentFunction")];
	if (!fragmentProgram) {
		NSLog(@"Failed to create fragment program");
		exit(EXIT_FAILURE);
	}

	MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
	pipelineStateDescriptor.label = @"DummyPipeline";
	[pipelineStateDescriptor setVertexFunction: vertexProgram];
	[pipelineStateDescriptor setFragmentFunction: fragmentProgram];
	pipelineStateDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
	// added dummy topology
	pipelineStateDescriptor.inputPrimitiveTopology = MTLPrimitiveTopologyClassPoint;

	MTLRenderPipelineReflection* reflection;
	MTLPipelineOption option = MTLPipelineOptionBufferTypeInfo | MTLPipelineOptionArgumentInfo;
	id <MTLRenderPipelineState> pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor options:option reflection:&reflection error:&errors];
	if (errors || !pipelineState) {
		NSLog(@"Failed to created Metal pipeline state: %@", [errors localizedDescription]);
		exit(EXIT_FAILURE);
	}

	for (MTLArgument *arg in reflection.vertexArguments) {
		NSLog(@"Found arguments: %@ index: %ld size: %ld\n", arg.name, long(arg.index), long(arg.bufferDataSize));
		attributes.push_back(std::make_pair(ce(arg.name), arg.index));
		if (arg.bufferDataType == MTLDataTypeStruct) {
			uniform_buffer_size = arg.bufferDataSize;
			for( MTLStructMember *uniform in arg.bufferStructType.members ) {
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

	build_metal_header(input_file, "drivers/metal/shader_metal.h", "");

	return 0;
}

static void build_metal_header(const std::string &filename, const std::string &include, const std::string &class_suffix) {
	std::string out_file = filename + ".gen.h";
	std::ofstream fd(out_file);

	NSLog(@"Writing %s file", out_file.c_str());

	fd << "/* WARNING, THIS FILE WAS GENERATED, DO NOT EDIT */\n";

	std::string out_file_base = out_file;
	out_file_base = out_file_base.substr(std::string::npos==out_file_base.rfind("/") ? 0 : out_file_base.rfind("/") + 1);
	std::string out_file_ifdef = up(rp(out_file_base, ".", "_"));
	fd << "#ifndef " << out_file_ifdef << class_suffix << "_120\n";
	fd << "#define " << out_file_ifdef << class_suffix << "_120\n";

	std::string out_file_class = rp(rp(title(rp(out_file_base, ".metal.gen.h", "")), "_", ""), ".", "") + "Shader" + class_suffix;
	fd << "\n\n";
	fd << "#include \"" << include << "\"\n\n\n";
	fd << "class " + out_file_class + " : public Shader" + class_suffix + " {\n\n";
	fd << "\t virtual String get_shader_name() const { return \"" << out_file_class << "\"; }\n";

	fd << "public:\n\n";

	if (!conditionals.empty()) {
		fd << "\tenum Conditionals {\n";
		for (const auto &x : conditionals) {
			fd << "\t\t" + up(x) + ",\n";
		}
		fd << "\t};\n\n";
	}

	if (!uniforms.empty()) {
		fd << "\tenum Uniforms {\n";
		for (const auto &x : uniforms) {
			fd << "\t\t" + up(std::get<0>(x)) + ",\n";
		}
		fd << "\t};\n\n";
	}

	fd << "\t_FORCE_INLINE_ int get_uniform(Uniforms p_uniform) const { return int(p_uniform); }\n\n";
	fd << "\t_FORCE_INLINE_ int get_uniform_buf(Uniforms p_uniform) const { return std::get<1>(_uniform_pairs[p_uniform]); }\n\n";
	fd << "\t_FORCE_INLINE_ int get_uniform_ofs(Uniforms p_uniform) const { return std::get<2>(_uniform_pairs[p_uniform]); }\n\n";
	fd << "\t_FORCE_INLINE_ int get_uniform_size(Uniforms p_uniform) const { return std::get<3>(_uniform_pairs[p_uniform]); }\n\n";
	if (!conditionals.empty()) {
		fd << "\t_FORCE_INLINE_ void set_conditional(Conditionals p_conditional,bool p_enable)  {  _set_conditional(p_conditional,p_enable); }\n\n";
	}
	fd << "\t_FORCE_INLINE_ template<typename T> cpy(Uniforms p_uniform, const T &p_a) { memcpy(uniform_buffer.get()+get_uniform_ofs(p_uniform), &p_a, sizeof(T)};\n";
	fd << "\t_FORCE_INLINE_ template<typename T> cpy(Uniforms p_uniform, const T &p_a, const T &p_b) { const T src[]={p_a,p_b}; memcpy(uniform_buffer.get()+get_uniform_ofs(p_uniform), src, 2*sizeof(T); };\n";
	fd << "\t_FORCE_INLINE_ template<typename T> cpy(Uniforms p_uniform, const T &p_a, const T &p_b, const T &p_c) { const T src[]={p_a,p_b,p_c}; memcpy(uniform_buffer.get()+get_uniform_ofs(p_uniform), src, 3*sizeof(T); };\n";
	fd << "\t#ifdef DEBUG_ENABLED\n";
	fd << "\t#define _FU ERR_FAIL_COND(p_uniform>" << uniforms.size()-1 << "); if (!is_version_valid()) return; ERR_FAIL_COND( get_active()!=this );\n";
	fd << "\t#else\n";
	fd << "\t#define _FU if (p_uniform>" << uniforms.size()-1 << ") return;\n";
	fd << "\t#endif\n\n";
	fd << "\t_FORCE_INLINE_ void set_uniform(Uniforms p_uniform, float p_value) { _FU cpy(p_uniform,p_value); }\n\n";
	fd << "\t_FORCE_INLINE_ void set_uniform(Uniforms p_uniform, double p_value) { _FU cpy(p_uniform,p_value); }\n\n";
	fd << "\t_FORCE_INLINE_ void set_uniform(Uniforms p_uniform, uint8_t p_value) { _FU cpy(p_uniform,p_value); }\n\n";
	fd << "\t_FORCE_INLINE_ void set_uniform(Uniforms p_uniform, int8_t p_value) { _FU cpy(p_uniform,p_value); }\n\n";
	fd << "\t_FORCE_INLINE_ void set_uniform(Uniforms p_uniform, uint16_t p_value) { _FU cpy(p_uniform,p_value); }\n\n";
	fd << "\t_FORCE_INLINE_ void set_uniform(Uniforms p_uniform, int16_t p_value) { _FU cpy(p_uniform,p_value); }\n\n";
	fd << "\t_FORCE_INLINE_ void set_uniform(Uniforms p_uniform, uint32_t p_value) { _FU cpy(p_uniform,p_value); }\n\n";
	fd << "\t_FORCE_INLINE_ void set_uniform(Uniforms p_uniform, int32_t p_value) { _FU cpy(p_uniform,p_value); }\n\n";
	fd << "\t_FORCE_INLINE_ void set_uniform(Uniforms p_uniform, const Color& p_color) { _FU float col[4]={p_color.r,p_color.g,p_color.b,p_color.a}; cpy(p_uniform,1,col); }\n\n";
	fd << "\t_FORCE_INLINE_ void set_uniform(Uniforms p_uniform, const Vector2& p_vec2) { _FU float vec2[2]={p_vec2.x,p_vec2.y}; cpy(p_uniform,1,vec2); }\n\n";
	fd << "\t_FORCE_INLINE_ void set_uniform(Uniforms p_uniform, const Size2i& p_vec2) { _FU int vec2[2]={p_vec2.x,p_vec2.y}; cpy(p_uniform,1,vec2); }\n\n";
	fd << "\t_FORCE_INLINE_ void set_uniform(Uniforms p_uniform, const Vector3& p_vec3) { _FU float vec3[3]={p_vec3.x,p_vec3.y,p_vec3.z}; cpy(p_uniform,1,vec3); }\n\n";
	fd << "\t_FORCE_INLINE_ void set_uniform(Uniforms p_uniform, float p_a, float p_b) { _FU cpy(p_uniform,p_a,p_b); }\n\n";
	fd << "\t_FORCE_INLINE_ void set_uniform(Uniforms p_uniform, float p_a, float p_b, float p_c) { _FU cpy(p_uniform,p_a,p_b,p_c); }\n\n";
	fd << "\t_FORCE_INLINE_ void set_uniform(Uniforms p_uniform, float p_a, float p_b, float p_c, float p_d) { _FU cpy(p_uniform,p_a,p_b,p_c,p_d); }\n\n";
	fd << R"(
	_FORCE_INLINE_ void set_uniform(Uniforms p_uniform, const Transform& p_transform) {  _FU

		const Transform &tr = p_transform;

		float matrix[16]={ /* build a 16x16 matrix */
			tr.basis.elements[0][0],
			tr.basis.elements[1][0],
			tr.basis.elements[2][0],
			0,
			tr.basis.elements[0][1],
			tr.basis.elements[1][1],
			tr.basis.elements[2][1],
			0,
			tr.basis.elements[0][2],
			tr.basis.elements[1][2],
			tr.basis.elements[2][2],
			0,
			tr.origin.x,
			tr.origin.y,
			tr.origin.z,
			1
		};

		memcpy(uniform_buffer.get()+get_uniform_ofs(p_uniform),&matrix,sizeof(matrix));
	}

	)";

	fd << R"(
	_FORCE_INLINE_ void set_uniform(Uniforms p_uniform, const Transform2D& p_transform) {  _FU

		const Transform2D &tr = p_transform;

		float matrix[16]={ /* build a 16x16 matrix */
			tr.elements[0][0],
			tr.elements[0][1],
			0,
			0,
			tr.elements[1][0],
			tr.elements[1][1],
			0,
			0,
			0,
			0,
			1,
			0,
			tr.elements[2][0],
			tr.elements[2][1],
			0,
			1
		};

		memcpy(uniform_buffer.get()+get_uniform_ofs(p_uniform),&matrix,sizeof(matrix));
	}

	)";

	fd << R"(
	_FORCE_INLINE_ void set_uniform(Uniforms p_uniform, const CameraMatrix& p_matrix) {  _FU

		GLfloat matrix[16];

		for (int i=0;i<4;i++) {
			for (int j=0;j<4;j++) {
				matrix[i*4+j]=p_matrix.matrix[i][j];
			}
		}

		glUniformMatrix4fv(get_uniform(p_uniform),1,false,matrix);
})";

	fd << "\n\n#undef _FU\n\n\n";

	fd << "\tvirtual void init() {\n\n";

	std::vector<std::string> conditionals_found;
	if (!conditionals.empty()) {
		fd << "\t\tstatic const char* _conditional_strings[]={\n";
		for (const auto &x : conditionals) {
			fd << "\t\t\t\"#define " << x << "\\n\",\n";
			conditionals_found.push_back(x);
		}
		fd << "\t\t};\n\n";
	} else {
		fd << "\t\tstatic const char **_conditional_strings=nullptr;\n";
	}

	if (!uniforms.empty()) {
		fd << "\t\tstatic UniformPair _uniform_pairs[]={\n";
		if (!uniforms.empty()) {
			int index = 0; for (const auto &x : uniforms) {
				fd << "\t\t\t{\"" <<  std::get<0>(x) << "\"," << index++ << "},\n";
			}
		}
		fd << "\t\t};\n\n";
	} else {
		fd << "\t\tstatic UniformPair *_uniform_pairs=nullptr;\n";
	}

	if (output_attribs) {
		if (!attributes.empty()) {
			fd << "\t\tstatic AttributePair _attribute_pairs[]={\n";
			for (const auto &x : attributes) {
				fd << "\t\t\t{\"" << x.first << "\"," << x.second << "},\n";
			}
			fd << "\t\t};\n\n";
		} else {
			fd << "\t\tstatic AttributePair *_attribute_pairs=nullptr;\n";
		}
	}

	if (!texunits.empty()) {
		fd << "\t\tstatic TexUnitPair _texunit_pairs[]={\n";
		for (const auto &x : texunits) {
			fd << "\t\t\t{\"" << x.first << "\"," << x.second << "},\n";
		}
		fd << "\t\t};\n\n";
	} else {
		fd << "\t\tstatic TexUnitPair *_texunit_pairs=nullptr;\n";
	}

	fd << "\t\tstatic const char _shader_code[]={\n";
	for (const auto &x : shader_lines) {
		for (const auto &c : x) {
			fd << int(c) << ",";
		}

		fd << int('\n') << ",";
	}
	fd << "\t\t0};\n\n";

	if (output_attribs) {
		fd <<
			"\t\tsetup(_conditional_strings," << conditionals.size()
			<< ",_uniform_pairs," << uniforms.size()
			<< ",_attribute_pairs," << attributes.size()
			<< ", _texunit_pairs," << texunits.size()
			<< ",_shader_code);\n";
	} else {
		fd <<
			"\t\tsetup(_conditional_strings," << conditionals.size()
			<< ",_uniform_pairs," << uniforms.size()
			<< ",_texunit_pairs," << texunits.size()
			<< ",_shader_code);\n";
	}

	fd << "\t}\n\n";

	if (!uniforms.empty()) {
		// full info
		fd << "\tstatic std::tuple<std::string, int, int, int, Variant::Type> _uniform_info[]={\n";
		if (!uniforms.empty()) {
			for (const auto &x : uniforms) {
				fd << "\t\t{\"" <<  std::get<0>(x) << "\"," << std::get<1>(x) << "," << std::get<2>(x) << "," << std::get<3>(x) << "," << std::get<4>(x) << "},\n";
			}
		}
		fd << "\t};\n\n";
	} else {
		fd << "\tstatic std::tuple<std::string, int, int, int, Variant::Type> *_uniform_info=nullptr;\n";
	}

	fd << "    std::unique_ptr<uint8_t> uniform_buffer = nullptr;\n";
	fd << "    const size_t _uniform_buffer_size = " << uniform_buffer_size << ";\n";

	fd << "};\n\n";
	fd << "#endif\n\n";
}
