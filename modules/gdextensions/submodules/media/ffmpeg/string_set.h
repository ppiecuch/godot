/*************************************************************************/
/*  string_set.h                                                         */
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

#ifndef _STRING_SET_H
#define _STRING_SET_H

#include <gdnative_api_struct.gen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linked_list.h"

extern const godot_gdnative_core_api_struct *api;

typedef struct set_bst_node_t {
	char *value;
	int priority;
	struct set_bst_node_t *left;
	struct set_bst_node_t *right;
} set_t;

set_t *set_create_node(const char *root_val) {
	set_t *root = (set_t *)api->godot_alloc(sizeof(set_t));
	root->value = (char *)api->godot_alloc(strlen(root_val) + 1);
	root->priority = rand();
	strcpy(root->value, root_val);
	root->left = NULL;
	root->right = NULL;
	return root;
}

set_t *cw_rot(set_t *root) {
	set_t *new_root = root->left;
	root->left = new_root->right;
	new_root->right = root;
	return new_root;
}

set_t *ccw_rot(set_t *root) {
	set_t *new_root = root->right;
	root->right = new_root->left;
	new_root->left = root;
	return new_root;
}

set_t *set_insert(set_t *root, const char *val) {
	if (root == NULL) {
		return set_create_node(val);
	}
	int cmp_val = strcmp(val, root->value);
	if (cmp_val == 0) {
		return root;
	} else if (cmp_val < 0) {
		root->left = set_insert(root->left, val);
		if (root->left->priority > root->priority) {
			root = cw_rot(root);
		}
		return root;
	} else {
		root->right = set_insert(root->right, val);
		if (root->right->priority > root->priority) {
			root = ccw_rot(root);
		}
		return root;
	}
}

void set_free(set_t *root) {
	if (root == NULL) {
		return;
	}
	set_free(root->right);
	set_free(root->left);
	api->godot_free(root->value);
	api->godot_free(root);
}

void set_print(set_t *root, int depth) {
	if (root == NULL)
		return;
	for (int i = 0; i < depth; i++) {
		printf(".");
	}
	printf("%s\n", root->value);
	set_print(root->left, depth + 1);
	set_print(root->right, depth + 1);
}

list_t set_create_list(set_t *root) {
	if (root == NULL) {
		list_t l;
		l.start = NULL;
		l.end = NULL;
		return l;
	}
	list_t left = set_create_list(root->left);
	list_t right = set_create_list(root->right);
	list_append(&left, root->value);
	list_join(&left, &right);
	return left;
}

#endif /* _STRING_SET_H */
