/**************************************************************************/
/*  utils_arrays.h                                                        */
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

/**
 * Copyright (C) 2015-2018 Think Silicon S.A. (https://think-silicon.com/)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public v3
 * License as published by the Free Software Foundation;
 */

/// A simple interface is provided for handling all the accesses to
/// the arrays of classes needed in GLOVE using the map container.

#ifndef UTILS_ARRAYS_H
#define UTILS_ARRAYS_H

#include <map>

/**
 * @brief A templated class for handling the memory allocation, indexing and
 * searching of all the different arrays of classes.
 *
 * A separate map container is created for every class that the GLOVE supports.
 * This map is used later for the creation of the various new objects,
 * their indexing and searching. The key value of the map is used as the GL
 * handle, therefore the 0 value is not permitted. The mapped value is the
 * generated object.
 */
template <class ELEMENT>
class ObjectArray {
private:
	uint32_t mCounter; /**< The id (key-value of the map)
						  reserved during the creation of a new
						  object. */
	std::map<uint32_t, ELEMENT *> mObjects; /**< The templated map container (one
										  for each different class that maps
										  id to a specific object). */
public:
	/**
	 * @brief The constructor initializes the key value to a non-usable value in
	 * order to be assigned later to a correct one, through the allocate method.
	 */
	ObjectArray() :
			mCounter(0) {
	}

	/**
	 * @brief The destructor removes all elements from the map container (which
	 * are destroyed), leaving the container with a size of 0.
	 */
	~ObjectArray() {
		typename std::map<uint32_t, ELEMENT *>::iterator it;
		for (it = mObjects.begin(); it != mObjects.end(); it++) {
			delete it->second;
		}
		mObjects.clear();
	}

	/**
	 * @brief Returns the GL handle and reserves this as the new key value.
	 * @return The GL handle.
	 */
	uint32_t Allocate() {
		return ++mCounter;
	}

	/**
	 * @brief Removes from the map container a single element with the given
	 * key value (element is  destroyed).
	 * @param index: The GL handle of the element to be destroyed.
	 */
	bool Deallocate(uint32_t index) {
		typename std::map<uint32_t, ELEMENT *>::iterator it = mObjects.find(index);
		if (it != mObjects.end()) {
			delete it->second;
			mObjects.erase(it);

			return true;
		}

		return false;
	}

	/**
	 * @brief Removes from the map container a single element with the given
	 * key value (element is NOT destroyed).
	 */
	bool RemoveFromList(uint32_t index) {
		typename std::map<uint32_t, ELEMENT *>::iterator it = mObjects.find(index);
		if (it != mObjects.end()) {
			mObjects.erase(it);
			return true;
		}
		return false;
	}

	/**
	 * @brief Searches the container for an element with a key equivalent to
	 * index and returns it.
	 * @param index: The GL handle of the element to be found or to be created.
	 * @return A pointer to the element in the map.
	 *
	 * In case the key value is not found (thus, the element does not exist)
	 * a new object is created. Consequently this method is the only way to
	 * insert a new element in the map.
	 */
	ELEMENT *GetObject(uint32_t index) {
		if (mCounter < index) {
			mCounter = index;
		}
		typename std::map<uint32_t, ELEMENT *>::iterator it = mObjects.find(index);
		if (it != mObjects.end()) {
			return it->second;
		} else {
			return mObjects[index] = new ELEMENT();
		}
	}

	/**
	 * @brief Searches the container for an element with a key equivalent to
	 * index.
	 * @param index: The GL handle of the element to be found.
	 * @return The decision whether the element exists or not.
	 */
	bool ObjectExists(uint32_t index) const {
		typename std::map<uint32_t, ELEMENT *>::const_iterator it = mObjects.find(index);
		return it == mObjects.end() ? false : true;
	}

	/**
	 * @brief Returns the GL handle of a specific element of the container.
	 * @param *element: The element to be searched in the container.
	 * @return The GL handle of the element.
	 *
	 * The container is traversed using the mapped value as the search value.
	 * The GL handle is returned in case the wanted element exists, else the
	 * returned value is 0.
	 */
	uint32_t GetObjectId(const ELEMENT *element) const {
		typename std::map<uint32_t, ELEMENT *>::const_iterator it;
		for (it = mObjects.begin(); it != mObjects.end(); it++) {
			if (it->second == element) {
				return it->first;
			}
		}

		return ~0;
	}

	/**
	 * @brief Returns the map container for a specific class.
	 * @return The map container.
	 */
	std::map<uint32_t, ELEMENT *> *GetObjects(void) {
		return &mObjects;
	}
};

#endif // UTILS_ARRAYS_H
