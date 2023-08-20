/**************************************************************************/
/*  enum_string.h                                                         */
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

#ifndef ENUM_STRING_H
#define ENUM_STRING_H

#if 0
	EnumString - A utility to provide stringizing support for C++ enums
	Author: Francis Xavier Joseph Pulikotil

	Usage example:
	--------------

	// WeekEnd enumeration
	enum WeekEnd {
		Sunday = 1,
		Saturday = 7
	};

	// String support for WeekEnd
	BeginEnumString( WeekEnd ) {
		EnumString( Sunday );
		EnumString( Saturday );
	}
	EndEnumString;

	// Convert from WeekEnd to string
	const string &str = EnumString<WeekEnd>::From(Saturday);
	// str should now be "Saturday"

	// Convert from string to WeekEnd
	WeekEnd w;
	EnumString<WeekEnd>::To(w, "Sunday");
	// w should now be Sunday
#endif

#include "core/error_macros.h"

#include "core/map.h"
#include "core/ustring.h"

// Helper macros

#define BeginEnumString(EnumerationName)                                                                       \
	template <>                                                                                                \
	struct EnumString<EnumerationName> : public EnumStringBase<EnumString<EnumerationName>, EnumerationName> { \
		static void RegisterEnumerators()

#define EnumString(EnumeratorName) RegisterEnumerator(EnumeratorName, #EnumeratorName);

#define EndEnumString }

// The EnumString base class
template <class DerivedType, class EnumType>
class EnumStringBase {
protected:
	typedef Map<String, EnumType> AssocMap;

protected:
	explicit EnumStringBase();
	~EnumStringBase();

private:
	EnumStringBase(const EnumStringBase &);
	const EnumStringBase &operator=(const EnumStringBase &);

private:
	static AssocMap &GetMap();

protected:
	// use this helper function to register each enumerator
	// and its string representation
	static void RegisterEnumerator(const EnumType e, const String &eStr);

public:
	// converts from an enumerator to a string
	// returns an empty string if the enumerator was not registered
	static const String &From(const EnumType e);

	// converts from a string to an enumerator
	// returns true if the conversion is successful; false otherwise
	static const bool To(EnumType &e, const String &str);
};

// The EnumString class
// Note: Specialize this class for each enumeration
// and implement the RegisterEnumerators() function.
template <class EnumType>
struct EnumString : public EnumStringBase<EnumString<EnumType>, EnumType> {
	static void RegisterEnumerators();
};

// Function definitions

template <class D, class E>
typename EnumStringBase<D, E>::AssocMap &EnumStringBase<D, E>::GetMap() {
	// a static map of associations from strings to enumerators
	static AssocMap assocMap;
	static bool firstAccess = true;
	// if this is the first time we're accessing the map, then populate it
	if (firstAccess) {
		firstAccess = false;
		D::RegisterEnumerators();
		DEV_ASSERT(!assocMap.empty());
	}
	return assocMap;
}

template <class D, class E>
void EnumStringBase<D, E>::RegisterEnumerator(const E e, const String &eStr) {
	DEV_ASSERT(!GetMap().has(eStr));
	GetMap().insert(eStr, e);
}

template <class D, class E>
const String &EnumStringBase<D, E>::From(const E e) {
	for (;;) { // code block
		typename AssocMap::Element *i = nullptr; // search for the enumerator in our map
		for (i = GetMap().front(); i; i = i->next()) {
			if (i->value() == e) {
				break;
			}
		}
		if (i == nullptr) { // if we didn't find it, we can't do this conversion
			break;
		}
		typename AssocMap::Element *j = i; // keep searching and see if we find another one with the same value
		for (j = j->next(); j; j = j->next()) {
			if (j->value() == e) {
				break;
			}
		}
		if (j != nullptr) { // if we found another one with the same value, we can't do this conversion
			break;
		}
		return i->key(); // we found exactly one string which matches the required enumerator
	}

	static const String _empty;
	return _empty; // we couldn't do this conversion; return an empty string
}

template <class D, class E>
const bool EnumStringBase<D, E>::To(E &e, const String &str) {
	if (const typename AssocMap::Element *found = GetMap().find(str)) { // if we have it, then return the associated enumerator
		e = found->value();
		return true;
	}
	return false; // we don't have it; the conversion failed
}
#endif // ENUM_STRING_H
