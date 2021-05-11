/*************************************************************************/
/*  essl.h                                                               */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef ESSL_H_
#define ESSL_H_

#include <stddef.h>
#include <stdint.h>

/**
 * @brief GXPES file header
 *
 */
typedef struct __attribute__((__packed__)) EsslHeader {
	char magic[6]; //!< "GXPES\0"
	uint32_t parameterTableOffset;
	uint32_t parameterTableSize;
} EsslHeader;

/**
 * @brief ESSL Parameter Formats.
 *
 */
typedef enum EsslParameterFormat {
	ESSL_PARAMETER_FORMAT_BOOL,
	ESSL_PARAMETER_FORMAT_BVEC2,
	ESSL_PARAMETER_FORMAT_BVEC3,
	ESSL_PARAMETER_FORMAT_BVEC4,
	ESSL_PARAMETER_FORMAT_INT,
	ESSL_PARAMETER_FORMAT_IVEC2,
	ESSL_PARAMETER_FORMAT_IVEC3,
	ESSL_PARAMETER_FORMAT_IVEC4,
	ESSL_PARAMETER_FORMAT_FLOAT,
	ESSL_PARAMETER_FORMAT_VEC2,
	ESSL_PARAMETER_FORMAT_VEC3,
	ESSL_PARAMETER_FORMAT_VEC4,
	ESSL_PARAMETER_FORMAT_MAT2,
	ESSL_PARAMETER_FORMAT_MAT3,
	ESSL_PARAMETER_FORMAT_MAT4,
	ESSL_PARAMETER_FORMAT_SAMPLER2D,
	ESSL_PARAMETER_FORMAT_SAMPLERCUBE
} EsslParameterFormat;

/**
 * @brief ESSL Parameter types
 *
 */
typedef enum EsslParameterType {
	ESSL_PARAMETER_TYPE_ATTRIBUTE, //!< Vertex attribute input data.
	ESSL_PARAMETER_TYPE_UNIFORM, //!< Uniform input data.
	ESSL_PARAMETER_TYPE_VARYING_INPUT, //!< Varying input data.
	ESSL_PARAMETER_TYPE_VARYING_OUTPUT, //!< Varying output data.
} EsslParameterType;

/**
 * @brief ESSL Parameter States.
 *
 */
typedef enum EsslParameterState {
	ESSL_PARAMETER_STATE_INACTIVE, //!< The parameter is not referenced in the shader.
	ESSL_PARAMETER_STATE_ACTIVE //!< The parameter is referenced in the shader.
} EsslParameterState;

/**
 * @brief Special Varying types.
 */
typedef enum EsslVaryingType {
	ESSL_VARYING_TYPE_TEXCOORD, //!< User-defined varying
	ESSL_VARYING_TYPE_POSITION, //!< gl_Position
	ESSL_VARYING_TYPE_POINTSIZE, //!< gl_PointSize
	ESSL_VARYING_TYPE_FRAGCOORD, //!< gl_FragCoord
	ESSL_VARYING_TYPE_FRONTFACE, //!< gl_FrontFacing
	ESSL_VARYING_TYPE_POINTCOORD, //!< gl_PointCoord
	ESSL_VARYING_TYPE_FRAGCOLOR //!< gl_FragColor
} EsslVaryingType;

/**
 * @brief An ESSL shader parameter
 *
 */
typedef struct EsslParameter {
	union {
		uint32_t parameterNameOffset; //!< Offset to parameter name from the parameter
		char *parameterName;
	};

	uint32_t state; //!< One of ::EsslParameterState
	uint32_t type; //!< One of ::EsslParameterType
	uint32_t elementCount; //!< Number of Array elements
	uint32_t format; //!< One of ::EsslParameterFormat
} EsslParameter;

/**
 * @brief An ESSL shader varying.
 *
 */
typedef struct EsslVaryingParameter {
	EsslParameter header;

	uint32_t varyingType; //!< One of ::EsslVaryingType.
	uint32_t resourceIndex; //!< The resource index. Will be 0xFF for inactive parameters.

	char padding[0x24];
} EsslVaryingParameter;

typedef struct SceShaccCgCompileOutput SceShaccCgCompileOutput; // Forward declaration.

/**
 * @brief Create ESSL/GXPES binary from ShaccCg compile output.
 *
 * @param compileOutput ShaccCg compile output
 * @param binary Pointer to pass the GXPES binary to.
 * @param binarySize Size of the output binary
 */
void EsslCreateBinary(const SceShaccCgCompileOutput *compileOutput, void **binary, size_t *binarySize, int vertexShader);

#endif /* ESSL_H_ */
