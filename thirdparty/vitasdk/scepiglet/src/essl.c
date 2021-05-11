/*************************************************************************/
/*  essl.c                                                               */
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

//
// Copyright (c) 2020 by SonicMastr <sonicmastr@gmail.com>
//
// This file is part of Pigs In A Blanket
//
// Pigs in a Blanket is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <psp2/shacccg.h>

#include "essl.h"
#include "shacccg/paramquery.h"

typedef struct EsslParameterListNode EsslParameterListNode;

typedef struct EsslParameterListNode {
	EsslParameterListNode *next;

	EsslParameter parameter;
	size_t nameLength;
} EsslParameterListNode;

typedef struct EsslParameterList {
	EsslParameterListNode *head;
	size_t count;
	size_t totalNameSize;
} EsslParameterList;

inline size_t EsslParameterSetName(EsslParameter *parameter, SceShaccCgParameter shaccParameter, const char *parentName) {
	const char *paramName = sceShaccCgGetParameterName(shaccParameter);

	size_t nameLength = strlen(paramName) + 1;

	if (parentName != NULL)
		nameLength += strlen(parentName);

	parameter->parameterName = calloc(nameLength, sizeof(char));
	if (parameter->parameterName == NULL)
		return 0;

	if (parentName)
		strcat(parameter->parameterName, parentName);

	strcat(parameter->parameterName, paramName);

	return nameLength;
}

size_t EsslParameterCreate(EsslParameter *parameter, SceShaccCgParameter shaccParameter, EsslParameterType type, size_t elementCount, const char *parentName) {
	size_t columnCount = sceShaccCgGetParameterColumns(shaccParameter);
	size_t rowCount = sceShaccCgGetParameterRows(shaccParameter);
	size_t vectorWidth = sceShaccCgGetParameterVectorWidth(shaccParameter);

	SceShaccCgParameterClass parameterClass = sceShaccCgGetParameterClass(shaccParameter);
	SceShaccCgParameterBaseType baseType = sceShaccCgGetParameterBaseType(shaccParameter);
	switch (parameterClass) {
		case SCE_SHACCCG_PARAMETERCLASS_SCALAR:
			if (type == ESSL_PARAMETER_TYPE_ATTRIBUTE) {
				switch (baseType) {
					case SCE_SHACCCG_BASETYPE_BOOL:
					case SCE_SHACCCG_BASETYPE_CHAR:
					case SCE_SHACCCG_BASETYPE_SHORT:
					case SCE_SHACCCG_BASETYPE_INT:
					case SCE_SHACCCG_BASETYPE_FIXED:
					case SCE_SHACCCG_BASETYPE_HALF:
					case SCE_SHACCCG_BASETYPE_FLOAT:
						parameter->format = ESSL_PARAMETER_FORMAT_FLOAT;
						break;
					default:
						return 0;
				}
			} else {
				switch (baseType) {
					case SCE_SHACCCG_BASETYPE_BOOL:
						parameter->format = ESSL_PARAMETER_FORMAT_BOOL;
						break;
					case SCE_SHACCCG_BASETYPE_CHAR:
					case SCE_SHACCCG_BASETYPE_SHORT:
					case SCE_SHACCCG_BASETYPE_INT:
						parameter->format = ESSL_PARAMETER_FORMAT_INT;
						break;
					case SCE_SHACCCG_BASETYPE_FIXED:
					case SCE_SHACCCG_BASETYPE_HALF:
					case SCE_SHACCCG_BASETYPE_FLOAT:
						parameter->format = ESSL_PARAMETER_FORMAT_FLOAT;
						break;
					default:
						return 0;
				}
			}
			break;
		case SCE_SHACCCG_PARAMETERCLASS_VECTOR:
			if (type == ESSL_PARAMETER_TYPE_ATTRIBUTE) {
				switch (baseType) {
					case SCE_SHACCCG_BASETYPE_BOOL:
					case SCE_SHACCCG_BASETYPE_CHAR:
					case SCE_SHACCCG_BASETYPE_SHORT:
					case SCE_SHACCCG_BASETYPE_INT:
					case SCE_SHACCCG_BASETYPE_FIXED:
					case SCE_SHACCCG_BASETYPE_HALF:
					case SCE_SHACCCG_BASETYPE_FLOAT:
						parameter->format = ESSL_PARAMETER_FORMAT_FLOAT + (vectorWidth - 1);
						break;
					default:
						return 0;
				}
			} else {
				switch (baseType) {
					case SCE_SHACCCG_BASETYPE_BOOL:
						parameter->format = ESSL_PARAMETER_FORMAT_BOOL + (vectorWidth - 1);
						break;
					case SCE_SHACCCG_BASETYPE_CHAR:
					case SCE_SHACCCG_BASETYPE_SHORT:
					case SCE_SHACCCG_BASETYPE_INT:
						parameter->format = ESSL_PARAMETER_FORMAT_INT + (vectorWidth - 1);
						break;
					case SCE_SHACCCG_BASETYPE_FIXED:
					case SCE_SHACCCG_BASETYPE_HALF:
					case SCE_SHACCCG_BASETYPE_FLOAT:
						parameter->format = ESSL_PARAMETER_FORMAT_FLOAT + (vectorWidth - 1);
						break;
					default:
						return 0;
				}
			}
			break;
		case SCE_SHACCCG_PARAMETERCLASS_MATRIX:
			if (columnCount == 4 && rowCount == 4)
				parameter->format = ESSL_PARAMETER_FORMAT_MAT4;
			else if (columnCount == 3 && rowCount == 3)
				parameter->format = ESSL_PARAMETER_FORMAT_MAT3;
			else if (columnCount == 2 && rowCount == 2)
				parameter->format = ESSL_PARAMETER_FORMAT_MAT2;
			else
				return 0;
			break;
		case SCE_SHACCCG_PARAMETERCLASS_SAMPLER:
			switch (baseType) {
				case SCE_SHACCCG_BASETYPE_SAMPLER2D:
				case SCE_SHACCCG_BASETYPE_ISAMPLER2D:
				case SCE_SHACCCG_BASETYPE_USAMPLER2D:
					parameter->format = ESSL_PARAMETER_FORMAT_SAMPLER2D;
					break;
				case SCE_SHACCCG_BASETYPE_SAMPLERCUBE:
				case SCE_SHACCCG_BASETYPE_ISAMPLERCUBE:
				case SCE_SHACCCG_BASETYPE_USAMPLERCUBE:
					parameter->format = ESSL_PARAMETER_FORMAT_SAMPLERCUBE;
					break;
				default:
					return 0;
					break;
			}
			break;
		default:
			// Unreachable.
			break;
	}

	parameter->type = type;
	parameter->elementCount = elementCount;
	parameter->state = ESSL_PARAMETER_STATE_ACTIVE;

	if (elementCount == 1)
		return EsslParameterSetName(parameter, shaccParameter, parentName);
	else
		return 1;
}

inline void EsslParameterListLink(EsslParameterList *parameterList, EsslParameterListNode *node) {
	node->next = parameterList->head;
	parameterList->head = node;
	parameterList->count++;
	parameterList->totalNameSize += node->nameLength;
}

int EsslParameterListCreateArray(EsslParameterList *parameterList, SceShaccCgParameter parameter, EsslParameterType paramType, const char *parentName);
int EsslParameterListCreateStructure(EsslParameterList *parameterList, SceShaccCgParameter parameter, EsslParameterType paramType, const char *parentName, int arrayParent) {
	EsslParameterListNode *currentNode = NULL;
	SceShaccCgParameter currentParameter = sceShaccCgGetFirstStructParameter(parameter);
	SceShaccCgParameterClass parameterClass;

	char *paramName;
	const char *structName = sceShaccCgGetParameterName(parameter);

	if (arrayParent != 0) {
		paramName = malloc((strlen(parentName) + strlen(structName) + 4) * sizeof(char));
		if (paramName == NULL)
			return 0;

		sprintf(paramName, "%s[%s].", parentName, structName);
	} else {
		if (parentName != NULL) {
			paramName = malloc((strlen(parentName) + strlen(structName) + 2) * sizeof(char));
			if (paramName == NULL)
				return 0;

			sprintf(paramName, "%s%s.", parentName, structName);
		} else {
			paramName = malloc((strlen(structName) + 2) * sizeof(char));
			if (paramName == NULL)
				return 0;

			sprintf(paramName, "%s.", structName);
		}
	}

	while (currentParameter != NULL) {
		if (sceShaccCgIsParameterReferenced(currentParameter) == 0)
			goto next;

		parameterClass = sceShaccCgGetParameterClass(currentParameter);
		switch (parameterClass) {
			case SCE_SHACCCG_PARAMETERCLASS_STRUCT:
				if (EsslParameterListCreateStructure(parameterList, currentParameter, paramType, paramName, 0) == 0)
					goto fail;
				break;
			case SCE_SHACCCG_PARAMETERCLASS_ARRAY:
				if (EsslParameterListCreateArray(parameterList, currentParameter, paramType, paramName) == 0)
					goto fail;
				break;
			case SCE_SHACCCG_PARAMETERCLASS_SCALAR:
			case SCE_SHACCCG_PARAMETERCLASS_VECTOR:
			case SCE_SHACCCG_PARAMETERCLASS_MATRIX:
			case SCE_SHACCCG_PARAMETERCLASS_SAMPLER:
				currentNode = malloc(sizeof(EsslParameterListNode));
				if (currentNode == NULL)
					goto fail;

				currentNode->nameLength = EsslParameterCreate(&currentNode->parameter, currentParameter, paramType, 1, paramName);
				if (currentNode->nameLength == 0)
					goto fail;

				EsslParameterListLink(parameterList, currentNode);
				break;
			default:
				break;
		}
	next:
		currentParameter = sceShaccCgGetNextParameter(currentParameter);
		continue;

	fail:
		if (currentNode != NULL)
			free(currentNode);

		free(paramName);
		return 0;
	}

	free(paramName);
	return 1;
}

int EsslParameterListCreateArray(EsslParameterList *parameterList, SceShaccCgParameter parameter, EsslParameterType paramType, const char *parentName) {
	const char *arrayName = sceShaccCgGetParameterName(parameter);
	char *paramName = NULL;

	EsslParameterListNode *currentNode = NULL;
	SceShaccCgParameter currentParameter = sceShaccCgGetArrayParameter(parameter, 0);
	SceShaccCgParameterClass parameterClass;

	if (sceShaccCgIsParameterReferenced(parameter) == 0)
		return 1;

	parameterClass = sceShaccCgGetParameterClass(currentParameter);
	switch (parameterClass) {
		case SCE_SHACCCG_PARAMETERCLASS_STRUCT:
			if (parentName != NULL) {
				paramName = malloc((strlen(parentName) + strlen(arrayName) + 1) * sizeof(char));
				if (paramName == NULL)
					goto fail;

				sprintf(paramName, "%s%s", parentName, arrayName);
			} else
				paramName = (char *)arrayName;

			while (currentParameter != NULL) {
				if (EsslParameterListCreateStructure(parameterList, currentParameter, paramType, paramName, 1) == 0)
					goto fail;

				currentParameter = sceShaccCgGetNextParameter(currentParameter);
			}

			if (paramName != arrayName)
				free(paramName);
			break;
		case SCE_SHACCCG_PARAMETERCLASS_SCALAR:
		case SCE_SHACCCG_PARAMETERCLASS_VECTOR:
		case SCE_SHACCCG_PARAMETERCLASS_MATRIX:
		case SCE_SHACCCG_PARAMETERCLASS_SAMPLER:
			currentNode = malloc(sizeof(EsslParameterListNode));
			if (currentNode == NULL)
				goto fail;

			if (EsslParameterCreate(&currentNode->parameter, currentParameter, paramType, sceShaccCgGetArraySize(parameter), NULL) == 0)
				goto fail;

			currentNode->nameLength = EsslParameterSetName(&currentNode->parameter, parameter, parentName);
			if (currentNode->nameLength == 0)
				goto fail;

			EsslParameterListLink(parameterList, currentNode);
			break;
		default:
			goto fail;
	}

	return 1;
fail:
	if (currentNode != NULL)
		free(currentNode);

	return 0;
}

int EsslParameterListCreate(EsslParameterList *parameterList, const SceShaccCgCompileOutput *compileOutput, EsslParameterType paramType) {
	SceShaccCgParameterVariability targetVariabiltiy = SCE_SHACCCG_VARIABILITY_INVALID;
	SceShaccCgParameterDirection targetDirection = SCE_SHACCCG_DIRECTION_INVALID;

	switch (paramType) {
		case ESSL_PARAMETER_TYPE_ATTRIBUTE:
			targetVariabiltiy = SCE_SHACCCG_VARIABILITY_VARYING;
			targetDirection = SCE_SHACCCG_DIRECTION_IN;
			break;
		case ESSL_PARAMETER_TYPE_UNIFORM:
			targetVariabiltiy = SCE_SHACCCG_VARIABILITY_UNIFORM;
			targetDirection = SCE_SHACCCG_DIRECTION_IN;
			break;
		default:
			return 1;
	}

	EsslParameterListNode *currentNode = NULL;
	SceShaccCgParameter currentParameter = sceShaccCgGetFirstParameter(compileOutput);
	SceShaccCgParameterClass parameterClass = SCE_SHACCCG_PARAMETERCLASS_INVALID;

	while (currentParameter != NULL) {
		if ((sceShaccCgGetParameterDirection(currentParameter) != targetDirection) || (sceShaccCgGetParameterVariability(currentParameter) != targetVariabiltiy))
			goto next;

		if (sceShaccCgIsParameterReferenced(currentParameter) == 0)
			goto next;

		parameterClass = sceShaccCgGetParameterClass(currentParameter);
		switch (parameterClass) {
			case SCE_SHACCCG_PARAMETERCLASS_STRUCT:
				if (EsslParameterListCreateStructure(parameterList, currentParameter, paramType, NULL, 0) == 0)
					goto fail;
				break;
			case SCE_SHACCCG_PARAMETERCLASS_ARRAY:
				if (EsslParameterListCreateArray(parameterList, currentParameter, paramType, NULL) == 0)
					goto fail;
				break;
			case SCE_SHACCCG_PARAMETERCLASS_SCALAR:
			case SCE_SHACCCG_PARAMETERCLASS_VECTOR:
			case SCE_SHACCCG_PARAMETERCLASS_MATRIX:
			case SCE_SHACCCG_PARAMETERCLASS_SAMPLER:
				currentNode = malloc(sizeof(EsslParameterListNode));
				if (currentNode == NULL)
					goto fail;

				currentNode->nameLength = EsslParameterCreate(&currentNode->parameter, currentParameter, paramType, 1, NULL);
				if (currentNode->nameLength == 0)
					goto fail;

				EsslParameterListLink(parameterList, currentNode);
				break;
			default:
				break;
		}

	next:
		currentParameter = sceShaccCgGetNextParameter(currentParameter);
		continue;

	fail:
		if (currentNode != NULL)
			free(currentNode);

		return 0;
	}

	return 1;
}

void *EsslParameterTableCreate(EsslParameterList *parameterLists, size_t *parameterTableSize) {
	uint8_t *parameterTable;
	*parameterTableSize = sizeof(uint32_t) * 3;
	uint8_t *curPtr;

	char *namePtr;
	uint32_t nameOffset;

	*parameterTableSize += sizeof(EsslParameter) * parameterLists[0].count;
	*parameterTableSize += sizeof(EsslParameter) * parameterLists[1].count;

	nameOffset = *parameterTableSize;

	*parameterTableSize += parameterLists[0].totalNameSize;
	*parameterTableSize += parameterLists[1].totalNameSize;

	if (*parameterTableSize == sizeof(uint32_t) * 3)
		return NULL;

	parameterTable = malloc(*parameterTableSize);
	if (parameterTable == NULL)
		return NULL;

	curPtr = parameterTable;
	namePtr = parameterTable + nameOffset;

	for (int i = 2; i >= 0; i--) {
		*(uint32_t *)curPtr = parameterLists[i].count;
		curPtr += 4;

		EsslParameterListNode *node = parameterLists[i].head;
		while (node != NULL) {
			memcpy(curPtr, &node->parameter, sizeof(EsslParameter));

			char *paramName = node->parameter.parameterName;
			size_t nameLength = strlen(paramName) + 1;
			strcpy(namePtr, paramName);

			((EsslParameter *)curPtr)->parameterNameOffset = (uint32_t)namePtr - (uint32_t)curPtr;

			namePtr += nameLength;
			curPtr += sizeof(EsslParameter);

			EsslParameterListNode *prevNode = node;
			node = node->next;

			free(prevNode->parameter.parameterName);
			free(prevNode);
		}
	}

	return parameterTable;
}

void EsslCreateBinary(const SceShaccCgCompileOutput *compileOutput, void **binary, size_t *binarySize, int vertexShader) {
	EsslHeader header = { "GXPES", 0, 0 };
	EsslParameterList parameterLists[3] = { 0 };
	void *parameterTable = NULL;
	size_t parameterTableSize = 0;

	for (int i = 1; i >= 0; i--) {
		EsslParameterType type = i;

		// Don't attempt to process attributes for fragment shader.
		if (vertexShader == 0 && i == 0)
			break;

		if (EsslParameterListCreate(&parameterLists[i], compileOutput, type) == 0)
			goto error;
	}

	parameterTable = EsslParameterTableCreate(parameterLists, &parameterTableSize);
	if (parameterTable == NULL)
		goto error;

	*binarySize = (compileOutput->programSize + 0xB) + parameterTableSize;
	*binary = malloc(*binarySize);
	if (*binary == NULL)
		goto error;

	memcpy(*binary + 0xB, compileOutput->programData, compileOutput->programSize);

	header.parameterTableSize = parameterTableSize;
	header.parameterTableOffset = *binarySize - parameterTableSize;

	memcpy(*binary, &header, sizeof(EsslHeader));
	memcpy(*binary + header.parameterTableOffset, parameterTable, parameterTableSize);

	return;
error:
	*binary = malloc(compileOutput->programSize);
	memcpy(*binary, compileOutput->programData, compileOutput->programSize);
	*binarySize = compileOutput->programSize;

	for (int i = 0; i < 2; i++) {
		EsslParameterListNode *node = parameterLists[i].head;
		while (node != NULL) {
			if (node->parameter.parameterName)
				free(node->parameter.parameterName);

			EsslParameterListNode *prevNode = node;
			node = node->next;

			free(prevNode);
		}
	}

	if (parameterTable != NULL)
		free(parameterTable);

	return;
}
