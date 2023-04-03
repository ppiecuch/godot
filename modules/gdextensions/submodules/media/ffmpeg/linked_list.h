/**************************************************************************/
/*  linked_list.h                                                         */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "core/os/memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct linked_list_node_t {
	char *value;
	struct linked_list_node_t *next;
} list_node_t;

typedef struct linked_list_t {
	list_node_t *start;
	list_node_t *end;
} list_t;

list_node_t *list_create_node(const char *str) {
	list_node_t *node = (list_node_t *)memalloc(sizeof(list_node_t));
	node->value = (char *)memalloc(strlen(str) + 1);
	strcpy(node->value, str);
	node->next = nullptr;
	return node;
}

void list_append(list_t *list, const char *str) {
	if (list->end == nullptr) {
		list->start = list_create_node(str);
		list->end = list->start;
		return;
	}
	list->end->next = list_create_node(str);
	list->end = list->end->next;
}

void list_join(list_t *first, list_t *second) {
	if (second->start == nullptr)
		return;
	first->end->next = second->start;
	first->end = second->end;
	second->start = nullptr;
	second->end = nullptr;
}

void list_free_r(list_node_t *head) {
	if (head == nullptr) {
		return;
	}
	list_free_r(head->next);
	if (head->value != nullptr) {
		memfree(head->value);
	}
	memfree(head);
}

void list_free(list_t *head) {
	list_free_r(head->start);
	head->end = nullptr;
	head->start = nullptr;
}

int list_size(list_t *head) {
	list_node_t *l = head->start;
	int i = 0;
	while (l != nullptr) {
		i++;
		l = l->next;
	}
	return i;
}

#endif /* LINKED_LIST_H */
