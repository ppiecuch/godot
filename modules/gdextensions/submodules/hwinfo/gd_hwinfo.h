/**************************************************************************/
/*  gd_hwinfo.h                                                           */
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

#ifndef GD_HWINFO_H
#define GD_HWINFO_H

#include "core/reference.h"
#include "core/variant.h"

class HWInfo : public Reference {
	GDCLASS(HWInfo, Reference);

protected:
	static void _bind_methods();

public:
	// --- CPU ---
	Array get_cpu_sockets() const;
	Dictionary get_cpu_info() const; // convenience: first socket's CPU

	// --- GPU ---
	Array get_gpus() const;
	Dictionary get_gpu_info() const; // convenience: first GPU

	// --- RAM ---
	Dictionary get_ram_info() const;
	Array get_ram_modules() const;

	// --- Disk ---
	Array get_disks() const;

	// --- Battery ---
	Array get_batteries() const;
	Dictionary get_battery_info() const; // convenience: first battery

	// --- OS ---
	Dictionary get_os_info() const;

	// --- System ---
	Dictionary get_system_info() const;
	Array get_cpu_usage() const;

	// --- MainBoard ---
	Dictionary get_mainboard_info() const;

	// --- Summary ---
	String get_summary() const;

	HWInfo();
};

#endif // GD_HWINFO_H
