/*************************************************************************/
/*  device_id.cpp                                                        */
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

#include "device_id.h"

#include "core/math/math_funcs.h"
#include "core/math/random_number_generator.h"
#include "core/os/os.h"

static String get_rand_id(int length = 4) {
	static RandomNumberGenerator _rnd;
	static String _allowed = "ABCDEFGHJKLMNPQRSTWXYZ123456789";
	ERR_FAIL_COND_V(length <= 0, "??");
	String ret;
	for (int i = 0; i < length; i++) {
		ret = ret + _allowed.substr(_rnd.randi() % _allowed.length(), 1);
	}
	return ret;
}

#if WINDOWS_ENABLED
#include <windows.h>
static String get_custom_name() {
	String nm;
	char buffer[256] = "";
	DWORD size = sizeof(buffer) - 1;
	if (GetComputerName(buffer, &size)) {
		if (size) {
			nm = buffer;
		}
	}
	if (nm.length() > 5) {
		return nm;
	}
	if (GetUserName(buffer, &size)) {
		if (size) {
			if (nm.empty()) {
				nm = buffer;
			} else {
				nm = nm + "-" + String(buffer);
			}
		}
	}
	return nm;
}
#elif OSX_ENABLED || LINUX_ENABLED || FRT_ENABLED
#include <limits.h>
#include <unistd.h>
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX _POSIX_HOST_NAME_MAX
#endif
static String get_custom_name() {
	String nm;
	char buffer[HOST_NAME_MAX + 1];
	if (gethostname(buffer, sizeof(buffer)) == 0) {
		// remove the .local or .lan from the computer name as reported by osx
		nm = String(buffer).trim_suffix(".lan").trim_suffix(".local");
	}
	if (nm.length() > 5) {
		return nm;
	}
	String login = getenv("LOGIN");
	if (!login.empty()) {
		if (nm.empty()) {
			nm = login;
		} else {
			nm = nm + "-" + login;
		}
	} else {
		String user = getenv("USER");
		if (!user.empty()) {
			if (nm.empty()) {
				nm = user;
			} else {
				nm = nm + "-" + user;
			}
		}
	}
	return nm;
}
#elif ANDROID_ENABLED
static String get_custom_name() {
	return OS::get_singleton()->get_custom_name();
}
#elif IPHONE_ENABLED
#import <objc/objc.h>
static String get_custom_name() {
	id uidevice_class = objc_getClass("UIDevice");
	ERR_FAIL_NULL_V(uidevice_class, "");
	SEL current_device_sel = sel_registerName("currentDevice");
	Method current_device_meth = class_getClassMethod((Class)uidevice_class, current_device_sel);
	ERR_FAIL_NULL_V(current_device_meth, "");
	IMP current_device_imp = method_getImplementation(current_device_meth);
	ERR_FAIL_NULL_V(current_device_imp, "");
	id uidevice = current_device_imp(uidevice_class, current_device_sel);
	ERR_FAIL_NULL_V(uidevice, "");
	id devicename = objc_msgSend(uidevice, sel_registerName("name"));
	ERR_FAIL_NULL_V(devicename, "");
	SEL utf8string = sel_registerName("UTF8String");
	ERR_FAIL_NULL_V(utf8string, "");
	return String(objc_msgSend(devicename, utf8string));
}
#endif

/// return unique but random name for this device/desktop/env.

String get_local_name() {
	String local_name = "??";

	String platform = OS::get_singleton()->get_name().to_upper();
	String device = OS::get_singleton()->get_model_name().to_upper();
	String unique_id = OS::get_singleton()->get_unique_id().to_upper();
	String name = get_custom_name().to_upper();
	String rand_postfix = get_rand_id().to_upper();

	if (platform == "WINDOWS") {
		platform = "WIN";
	} else if (platform == "ANDROID") {
		platform = "DROID";
	}
	if (device == "GENERICDEVICE") {
		device = "";
	}

	return local_name;
}
