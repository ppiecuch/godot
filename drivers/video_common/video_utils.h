/**************************************************************************/
/*  video_utils.h                                                         */
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

#ifndef VIDEO_UTILS_H
#define VIDEO_UTILS_H

#include "core/variant.h"

// The Vendor and Renderer enum values are updated as required.
enum VideoVendor {
	VENDOR_ARM,
	VENDOR_APPLE,
	VENDOR_GOOGLE,
	VENDOR_IMAGINATION,
	VENDOR_INTEL,
	VENDOR_QUALCOMM,
	VENDOR_BROADCOM,
	VENDOR_VIVANTE,
	VENDOR_NVIDIA,
	VENDOR_ATI,
	VENDOR_MICROSOFT,
	VENDOR_SONY,
	VENDOR_NINTENDO,

	VENDOR_OTHER
};

enum VideoRenderer {
	RENDERER_TEGRA_PREK1, // Legacy Tegra architecture (pre-K1).
	RENDERER_TEGRA, // Tegra with the same architecture as NVIDIA desktop GPUs (K1+).

	RENDERER_POWERVR54X,
	RENDERER_POWERVRBSERIES,
	RENDERER_POWERVRROGUE,

	RENDERER_ADRENO3XX,
	RENDERER_ADRENO430,
	RENDERER_ADRENO4XX_OTHER,
	RENDERER_ADRENO530,
	RENDERER_ADRENO5XX_OTHER,
	RENDERER_ADRENO615, // Pixel3a
	RENDERER_ADRENO620, // Pixel5
	RENDERER_ADRENO630, // Pixel3
	RENDERER_ADRENO640, // Pixel4
	RENDERER_ADRENO6XX_OTHER,

	// Intel GPU families, ordered by generation

	// 6th gen
	RENDERER_INTELSANDYBRIDGE,

	// 7th gen
	RENDERER_INTELIVYBRIDGE,
	RENDERER_INTELVALLEYVIEW, // aka BayTrail
	RENDERER_INTELHASWELL,

	// 8th gen
	RENDERER_INTELCHERRYVIEW, // aka Braswell
	RENDERER_INTELBROADWELL,

	// 9th gen
	RENDERER_INTELAPOLLOLAKE,
	RENDERER_INTELSKYLAKE,
	RENDERER_INTELGEMINILAKE,
	RENDERER_INTELKABYLAKE,
	RENDERER_INTELCOFFEELAKE,

	// 11th gen
	RENDERER_INTELICELAKE,

	// 12th gen
	RENDERER_INTELROCKETLAKE,
	RENDERER_INTELTIGERLAKE,
	RENDERER_INTELALDERLAKE,

	RENDERER_GALLIUMLLVM,

	RENDERER_MALI4XX,
	RENDERER_MALIG, // G-3x, G-5x, or G-7x
	RENDERER_MALIT, // T-6xx, T-7xx, or T-8xx

	RENDERER_AMDRADEONHD7XXX, // AMD Radeon HD 7000 Series
	RENDERER_AMDRADEONR9M3XX, // AMD Radeon R9 M300 Series
	RENDERER_AMDRADEONR9M4XX, // AMD Radeon R9 M400 Series
	RENDERER_AMDRADEONPRO5XXX, // AMD Radeon Pro 5000 Series
	RENDERER_AMDRADEONPROVEGAXX, // AMD Radeon Pro Vega

	RENDERER_APPLE_M, // Apple M CPU

	RENDERER_WEBGL,

	RENDERER_OTHER,
};

enum VideoDriver {
	DRIVER_MESA,
	DRIVER_NVIDIA,
	DRIVER_INTEL,
	DRIVER_QUALCOMM,
	DRIVER_FREEDRENO,
	DRIVER_ANDROIDEMULATOR,
	DRIVER_IMAGINATION,
	DRIVER_ARM,
	DRIVER_APPLE,
	DRIVER_UNKNOWN
};

enum VideoAngleBackend {
	ANGLE_UNKNOWN,
	ANGLE_D3D9,
	ANGLE_D3D11,
	ANGLE_METAL,
	ANGLE_OPENGL
};

enum VideoStandard {
	STANDARD_NONE,
	STANDARD_GL,
	STANDARD_GLES,
	STANDARD_WEBGL,
};

#define VIDEO_VER(major, minor) ((static_cast<uint32_t>(major) << 16) | static_cast<uint32_t>(minor))
#define VIDEO_MAJOR_VER(version) (static_cast<uint32_t>(version) >> 16)
#define VIDEO_MINOR_VER(version) (static_cast<uint32_t>(version) & 0xFFFF)
#define VIDEO_INVALID_VER VIDEO_VER(0, 0)

#define DRIVER_VER(major, minor, point) ((static_cast<uint64_t>(major) << 32) | (static_cast<uint64_t>(minor) << 16) | static_cast<uint64_t>(point))
#define DRIVER_UNKNOWN_VER DRIVER_VER(0, 0, 0)

typedef uint32_t VideoVersion;
typedef uint64_t DriverVersion;

// The following allow certain interfaces to be turned off at compile time (for example, to lower code size).
#if VIDEO_ASSUME_GL_ES
#define VIDEO_IS_GL(standard) false
#define VIDEO_IS_GL_ES(standard) true
#define VIDEO_IS_WEBGL(standard) false
#elif VIDEO_ASSUME_GL
#define VIDEO_IS_GL(standard) true
#define VIDEO_IS_GL_ES(standard) false
#define VIDEO_IS_WEBGL(standard) false
#elif VIDEO_ASSUME_WEBGL
#define VIDEO_IS_GL(standard) false
#define VIDEO_IS_GL_ES(standard) false
#define VIDEO_IS_WEBGL(standard) true
#else
#define VIDEO_IS_GL(standard) (STANDARD_GL == standard)
#define VIDEO_IS_GL_ES(standard) (STANDARD_GLES == standard)
#define VIDEO_IS_WEBGL(standard) (STANDARD_WEBGL == standard)
#endif

struct VideoDriverInfo {
	VideoStandard standard;
	VideoStandard standard_detected;
	VideoVersion version;
	VideoVersion shading_lang_version;
	VideoVendor vendor;
	VideoRenderer renderer;
	VideoDriver driver;
	DriverVersion driver_version;

	VideoAngleBackend angle_backend;
	VideoVendor angle_vendor;
	VideoRenderer angle_renderer;
	VideoDriver angle_driver;
	DriverVersion angle_driver_version;

	VideoVendor webgl_vendor;
	VideoRenderer webgl_renderer;
};

VideoDriverInfo video_get_driver_info(const Dictionary &p_context_info);

String video_get_vendor_name(VideoVendor p_vendor);
String video_get_video_version_string(VideoVersion p_version);
String video_get_driver_version_string(DriverVersion p_version);

#endif // VIDEO_UTILS_H
