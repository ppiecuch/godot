/*************************************************************************/
/*  glm_hashtable.c                                                      */
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

/*
 * Copyright (C) Michael Larson on 1/6/2022
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * hash_table.c
 * MGL
 *
 */

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <strings.h>

#include "glm_hashtable.h"

void initHashTable(HashTable *ptr, GLuint size) {
	size_t len;

	len = sizeof(HashObj) * size;

	ptr->current_name = 1;
	ptr->size = size;
	ptr->keys = (HashObj *)malloc(len);
	assert(ptr->keys);

	bzero(ptr->keys, len);
}

GLuint getNewName(HashTable *table) {
	return table->current_name++;
}

void *searchHashTable(HashTable *table, GLuint name) {
	assert(table);
	assert(name < table->size);

	return table->keys[name].data;
}

void insertHashElement(HashTable *table, GLuint name, void *data) {
	assert(table);

	if (name < table->size) {
		assert(table->keys[name].data == NULL);

		table->keys[name].data = data;

		return;
	}

	// some calls allow the user to specifiy a name...
	while (table->size < name) {
		table->size *= 2;
		table->keys = (HashObj *)realloc(table->keys, table->size);
	}

	table->keys[name].data = data;
}

void deleteHashElement(HashTable *table, GLuint name) {
	assert(table);
	assert(name < table->size);

	table->keys[name].data = NULL;

	// need to have metal delete object
	assert(false);
}
