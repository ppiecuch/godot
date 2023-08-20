/**************************************************************************/
/*  video_utils.cpp                                                       */
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

#include "video_utils.h"

#include "common/gd_core.h"
#include "core/error_macros.h"
#include "core/set.h"
#include "core/variant.h"
#include "servers/visual/rasterizer.h"

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

static VideoDriverInfo _default_none = {
	/* standard */ STANDARD_NONE,
	/* standard_detected */ STANDARD_NONE,
	/* version */ VIDEO_INVALID_VER,
	/* shading_lang_version */ VIDEO_INVALID_VER,
	/* vendor */ VENDOR_OTHER,
	/* renderer */ RENDERER_OTHER,
	/* driver */ DRIVER_UNKNOWN,
	/* driver_version */ DRIVER_UNKNOWN_VER,

	/* angle_backend */ ANGLE_UNKNOWN,
	/* angle_vendor */ VENDOR_OTHER,
	/* angle_renderer */ RENDERER_OTHER,
	/* angle_driver */ DRIVER_UNKNOWN,
	/* angle_driver_version */ DRIVER_UNKNOWN_VER,

	/* webgl_endor */ VENDOR_OTHER,
	/* webgl_renderer */ RENDERER_OTHER,
};

static VideoStandard get_standard_from_string(const String p_version_string) {
	ERR_FAIL_COND_V(p_version_string.empty(), STANDARD_NONE);

	const CharString version_string = p_version_string.utf8();
	int major, minor;

	// check for desktop
	int n = sscanf(version_string.c_str(), "%d.%d", &major, &minor);
	if (2 == n) {
		return STANDARD_GL;
	}

	// WebGL might look like "OpenGL ES 2.0 (WebGL 1.0 (OpenGL ES 2.0 Chromium))"
	int es_major, es_minor;
	n = sscanf(version_string.c_str(), "OpenGL ES %d.%d (WebGL %d.%d", &es_major, &es_minor, &major, &minor);
	if (4 == n) {
		return STANDARD_WEBGL;
	}

	// check for ES 1
	char profile[2];
	n = sscanf(version_string.c_str(), "OpenGL ES-%c%c %d.%d", profile, profile + 1, &major, &minor);
	if (4 == n) {
		return STANDARD_NONE; // no support for ES1.
	}

	// check for ES2
	n = sscanf(version_string.c_str(), "OpenGL ES %d.%d", &major, &minor);
	if (2 == n) {
		return STANDARD_GLES;
	}
	return STANDARD_NONE;
}

static VideoVersion get_version_from_string(const String &p_version_string) {
	ERR_FAIL_COND_V(p_version_string.empty(), VIDEO_INVALID_VER);

	const CharString version_string = p_version_string.utf8();
	int major, minor;

	// check for mesa
	int mesa_major, mesa_minor;
	int n = sscanf(version_string.c_str(), "%d.%d Mesa %d.%d", &major, &minor, &mesa_major, &mesa_minor);
	if (4 == n) {
		return VIDEO_VER(major, minor);
	}

	n = sscanf(version_string.c_str(), "%d.%d", &major, &minor);
	if (2 == n) {
		return VIDEO_VER(major, minor);
	}

	// WebGL might look like "OpenGL ES 2.0 (WebGL 1.0 (OpenGL ES 2.0 Chromium))"
	int es_major, es_minor;
	n = sscanf(version_string.c_str(), "OpenGL ES %d.%d (WebGL %d.%d", &es_major, &es_minor, &major, &minor);
	if (4 == n) {
		return VIDEO_VER(major, minor);
	}

	char profile[2];
	n = sscanf(version_string.c_str(), "OpenGL ES-%c%c %d.%d", profile, profile + 1, &major, &minor);
	if (4 == n) {
		return VIDEO_VER(major, minor);
	}

	n = sscanf(version_string.c_str(), "OpenGL ES %d.%d", &major, &minor);
	if (2 == n) {
		return VIDEO_VER(major, minor);
	}

	return VIDEO_INVALID_VER;
}

static VideoVersion get_shading_lang_version(const String &p_version_string) {
	ERR_FAIL_COND_V(p_version_string.empty(), VIDEO_INVALID_VER);

	const CharString version_string = p_version_string.utf8();
	int major, minor;

	int n = sscanf(version_string.c_str(), "%d.%d", &major, &minor);
	if (2 == n) {
		return VIDEO_VER(major, minor);
	}

	n = sscanf(version_string.c_str(), "OpenGL ES GLSL ES %d.%d", &major, &minor);
	if (2 == n) {
		return VIDEO_VER(major, minor);
	}

#ifdef ANDROID_ENABLED
	// android hack until the gpu vender updates their drivers
	n = sscanf(version_string.c_str(), "OpenGL ES GLSL %d.%d", &major, &minor);
	if (2 == n) {
		return VIDEO_VER(major, minor);
	}
#endif

	return VIDEO_INVALID_VER;
}

// If this is detected as ANGLE then the ANGLE backend is returned along with renderer_string
// stripped of "ANGLE(" and ")" at the start and end, respectively.
static bool get_angle_backend(const String &p_renderer_string, VideoAngleBackend &r_backend, String &r_angle_renderer_string) {
	// crbug.com/1203705 ANGLE renderer will be "ANGLE (<gl-vendor>, <gl-renderer>, <gl-version>)"
	// on ANGLE's GL backend with related substitutions for the inner strings on other backends.
	static constexpr char kHeader[] = "ANGLE (";
	static constexpr size_t kHeaderLength = sizeof(kHeader) - 1;
	const int renderer_length = p_renderer_string.size();
	if (p_renderer_string.begins_with(kHeader) && p_renderer_string.ends_with(")")) {
		String inner_string = p_renderer_string.substr(kHeaderLength, renderer_length - kHeaderLength - 1);
		if (p_renderer_string.has("Direct3D11")) {
			r_backend = ANGLE_D3D11, r_angle_renderer_string = inner_string;
			return true;
		} else if (p_renderer_string.has("Direct3D9")) {
			r_backend = ANGLE_D3D9, r_angle_renderer_string = inner_string;
			return true;
		} else if (p_renderer_string.has("Metal")) {
			r_backend = ANGLE_METAL, r_angle_renderer_string = inner_string;
			return true;
		} else if (p_renderer_string.has("OpenGL")) {
			r_backend = ANGLE_OPENGL, r_angle_renderer_string = inner_string;
			return true;
		}
	}
	r_backend = ANGLE_UNKNOWN;
	r_angle_renderer_string = "";
	return false;
}

static VideoVendor get_vendor(const String &p_vendor_string) {
	ERR_FAIL_COND_V(p_vendor_string.empty(), VENDOR_OTHER);

	if (p_vendor_string == "ARM") {
		return VENDOR_ARM;
	}
	if (p_vendor_string == "Apple" || p_vendor_string == "Apple Inc.") {
		return VENDOR_APPLE;
	}
	if (p_vendor_string == "Google Inc.") {
		return VENDOR_GOOGLE;
	}
	if (p_vendor_string == "Imagination Technologies") {
		return VENDOR_IMAGINATION;
	}
	if (p_vendor_string.begins_with("Intel ") || p_vendor_string == "Intel" || p_vendor_string == "Tungsten Graphics, Inc") {
		return VENDOR_INTEL;
	}
	if (p_vendor_string == "Qualcomm" || p_vendor_string == "freedreno") {
		return VENDOR_QUALCOMM;
	}
	if (p_vendor_string == "Broadcom" || p_vendor_string.has("VideoCore")) {
		return VENDOR_BROADCOM;
	}
	if (p_vendor_string == "Vivante Corporation") {
		return VENDOR_VIVANTE;
	}
	if (p_vendor_string == "NVIDIA Corporation" || p_vendor_string == "Nouveau" || p_vendor_string == "nouveau") {
		return VENDOR_NVIDIA;
	}
	if (p_vendor_string == "ATI Technologies Inc." || p_vendor_string == "Advanced Micro Devices, Inc." || p_vendor_string == "AMD") {
		return VENDOR_ATI;
	}
	return VENDOR_OTHER;
}

static VideoRenderer get_renderer(const String &p_renderer_string, const PoolStringArray &p_extensions) {
	ERR_FAIL_COND_V(p_renderer_string.empty(), RENDERER_OTHER);

	static const char kTegraStr[] = "NVIDIA Tegra";
	if (p_renderer_string.begins_with(kTegraStr)) {
		// Tegra strings are not very descriptive. We distinguish between the modern and legacy
		// architectures by the presence of NV_path_rendering.
		return p_extensions.has("GL_NV_path_rendering") ? RENDERER_TEGRA : RENDERER_TEGRA_PREK1;
	}

	const CharString renderer_string = p_renderer_string.utf8();

	int last_digit;
	int n = sscanf(renderer_string.c_str(), "PowerVR SGX 54%d", &last_digit);
	if (1 == n && last_digit >= 0 && last_digit <= 9) {
		return RENDERER_POWERVR54X;
	}
	if (p_renderer_string.has("PowerVR B-Series")) {
		return RENDERER_POWERVRBSERIES;
	}
	// certain iOS devices also use PowerVR54x GPUs
	static const char kAppleA4Str[] = "Apple A4";
	static const char kAppleA5Str[] = "Apple A5";
	static const char kAppleA6Str[] = "Apple A6";
	if (p_renderer_string.begins_with(kAppleA4Str) || p_renderer_string.begins_with(kAppleA5Str) || p_renderer_string.begins_with(kAppleA6Str)) {
		return RENDERER_POWERVR54X;
	}
	static const char kPowerVRRogueStr[] = "PowerVR Rogue";
	static const char kAppleA7Str[] = "Apple A7";
	static const char kAppleA8Str[] = "Apple A8";
	if (p_renderer_string.begins_with(kPowerVRRogueStr) || p_renderer_string.begins_with(kAppleA7Str) || p_renderer_string.begins_with(kAppleA8Str)) {
		return RENDERER_POWERVRROGUE;
	}
	int adreno_number;
	n = sscanf(renderer_string.c_str(), "Adreno (TM) %d", &adreno_number);
	if (n < 1) {
		// retry with freedreno driver
		n = sscanf(renderer_string.c_str(), "FD%d", &adreno_number);
	}
	if (1 == n) {
		if (adreno_number >= 300) {
			if (adreno_number < 400) {
				return RENDERER_ADRENO3XX;
			}
			if (adreno_number < 500) {
				return adreno_number >= 430 ? RENDERER_ADRENO430 : RENDERER_ADRENO4XX_OTHER;
			}
			if (adreno_number < 600) {
				return adreno_number == 530 ? RENDERER_ADRENO530 : RENDERER_ADRENO5XX_OTHER;
			}
			if (adreno_number < 700) {
				if (adreno_number == 615) {
					return RENDERER_ADRENO615;
				}
				if (adreno_number == 620) {
					return RENDERER_ADRENO620;
				}
				if (adreno_number == 630) {
					return RENDERER_ADRENO630;
				}
				if (adreno_number == 640) {
					return RENDERER_ADRENO640;
				}
				return RENDERER_ADRENO6XX_OTHER;
			}
		}
	}

	if (const char *intel_string = strstr(renderer_string.c_str(), "Intel")) {
		// These generic strings seem to always come from Haswell: Iris 5100 or Iris Pro 5200
		if (0 == strcmp(intel_string, "Intel Iris OpenGL Engine") || 0 == strcmp(intel_string, "Intel Iris Pro OpenGL Engine")) {
			return RENDERER_INTELHASWELL;
		}
		if (strstr(intel_string, "Sandybridge")) {
			return RENDERER_INTELSANDYBRIDGE;
		}
		if (strstr(intel_string, "Bay Trail")) {
			return RENDERER_INTELVALLEYVIEW;
		}
		// In Mesa, 'RKL' can be followed by 'Graphics', same for 'TGL' and 'ADL'.
		// Referenced from the following Mesa source code:
		// https://github.com/mesa3d/mesa/blob/master/include/pci_ids/iris_pci_ids.h
		if (strstr(intel_string, "RKL")) {
			return RENDERER_INTELROCKETLAKE;
		}
		if (strstr(intel_string, "TGL")) {
			return RENDERER_INTELTIGERLAKE;
		}
		// For Windows on ADL-S devices, 'AlderLake-S' might be followed by 'Intel(R)'.
		if (strstr(intel_string, "ADL") || strstr(intel_string, "AlderLake")) {
			return RENDERER_INTELALDERLAKE;
		}
		// For Windows on TGL or other ADL devices, we might only get 'Xe' from the string.
		// Since they are both 12th gen, we could temporarily use 'kIntelTigerLake' to cover
		// both TGL and ADL.
		if (strstr(intel_string, "Xe")) {
			return RENDERER_INTELTIGERLAKE;
		}
		// There are many possible intervening strings here:
		// 'Intel(R)' is a common prefix
		// 'Iris' may appear, followed by '(R)' or '(TM)'
		// 'Iris' can then be followed by 'Graphics', 'Pro Graphics', or 'Plus Graphics'
		// If 'Iris' isn't there, we might have 'HD Graphics' or 'UHD Graphics'
		//
		// In all cases, though, we end with 'Graphics ', an optional 'P', and a number,
		// so just skip to that and handle two cases:
		if (const char *intel_gfx_string = strstr(intel_string, "Graphics")) {
			int intel_number;
			if (sscanf(intel_gfx_string, "Graphics %d", &intel_number) ||
					sscanf(intel_gfx_string, "Graphics P%d", &intel_number)) {
				if (intel_number == 2000 || intel_number == 3000) {
					return RENDERER_INTELSANDYBRIDGE;
				}
				if (intel_number == 2500 || intel_number == 4000) {
					return RENDERER_INTELIVYBRIDGE;
				}
				if (intel_number >= 4200 && intel_number <= 5200) {
					return RENDERER_INTELHASWELL;
				}
				if (intel_number >= 400 && intel_number <= 405) {
					return RENDERER_INTELCHERRYVIEW;
				}
				if (intel_number >= 5300 && intel_number <= 6300) {
					return RENDERER_INTELBROADWELL;
				}
				if (intel_number >= 500 && intel_number <= 505) {
					return RENDERER_INTELAPOLLOLAKE;
				}
				if (intel_number >= 510 && intel_number <= 580) {
					return RENDERER_INTELSKYLAKE;
				}
				if (intel_number >= 600 && intel_number <= 605) {
					return RENDERER_INTELGEMINILAKE;
				}
				// 610 and 630 are reused from KabyLake to CoffeeLake. The CoffeeLake variants
				// are "UHD Graphics", while the KabyLake ones are "HD Graphics"
				if (intel_number == 610 || intel_number == 630) {
					return strstr(intel_string, "UHD") ? RENDERER_INTELCOFFEELAKE : RENDERER_INTELKABYLAKE;
				}
				if (intel_number >= 610 && intel_number <= 650) {
					return RENDERER_INTELKABYLAKE;
				}
				if (intel_number == 655) {
					return RENDERER_INTELCOFFEELAKE;
				}
				// 710/730/750/770 are all 12th gen UHD Graphics, but it's hard to distinguish
				// among RKL, TGL and ADL. We might temporarily use 'kIntelTigerLake' to cover all.
				if (intel_number >= 710 && intel_number <= 770) {
					return RENDERER_INTELTIGERLAKE;
				}
				if (intel_number >= 910 && intel_number <= 950) {
					return RENDERER_INTELICELAKE;
				}
			}
		}
	}

	// The AMD string can have a somewhat arbitrary preamble (see skbug.com/7195)
	static constexpr char kRadeonStr[] = "Radeon ";
	if (const char *amd_string = strstr(renderer_string.c_str(), kRadeonStr)) {
		amd_string += strlen(kRadeonStr);
		static constexpr char kTMStr[] = "(TM) "; // Sometimes there is a (TM) and sometimes not.
		if (!strncmp(amd_string, kTMStr, strlen(kTMStr))) {
			amd_string += strlen(kTMStr);
		}

		char amd0, amd1, amd2;
		int amd_model;
		n = sscanf(amd_string, "R9 M3%c%c", &amd0, &amd1);
		if (2 == n && isdigit(amd0) && isdigit(amd1)) {
			return RENDERER_AMDRADEONR9M3XX;
		}

		n = sscanf(amd_string, "R9 M4%c%c", &amd0, &amd1);
		if (2 == n && isdigit(amd0) && isdigit(amd1)) {
			return RENDERER_AMDRADEONR9M4XX;
		}

		n = sscanf(amd_string, "HD 7%c%c%c Series", &amd0, &amd1, &amd2);
		if (3 == n && isdigit(amd0) && isdigit(amd1) && isdigit(amd2)) {
			return RENDERER_AMDRADEONHD7XXX;
		}

		n = sscanf(amd_string, "Pro 5%c%c%c", &amd0, &amd1, &amd2);
		if (3 == n && isdigit(amd0) && isdigit(amd1) && isdigit(amd2)) {
			return RENDERER_AMDRADEONPRO5XXX;
		}

		n = sscanf(amd_string, "Pro Vega %i", &amd_model);
		if (1 == n) {
			return RENDERER_AMDRADEONPROVEGAXX;
		}
	}

	if (p_renderer_string.begins_with("Apple M")) {
		return RENDERER_APPLE_M;
	}

	if (p_renderer_string.has("llvmpipe")) {
		return RENDERER_GALLIUMLLVM;
	}
	if (p_renderer_string.begins_with("Mali-G")) {
		return RENDERER_MALIG;
	}
	if (p_renderer_string.begins_with("Mali-T")) {
		return RENDERER_MALIT;
	}
	int mali400num;
	if (1 == sscanf(renderer_string.c_str(), "Mali-%d", &mali400num) && mali400num >= 400 &&
			mali400num < 500) {
		return RENDERER_MALI4XX;
	}

	if (p_renderer_string.has("WebGL")) {
		return RENDERER_WEBGL;
	}

	return RENDERER_OTHER;
}

static bool get_driver_and_version(VideoStandard p_standard, VideoVendor p_vendor, const String &p_vendor_string, const String &p_renderer_string, const String &p_version_string, VideoDriver &r_driver, DriverVersion &r_driver_version) {
	r_driver = DRIVER_UNKNOWN;
	r_driver_version = DRIVER_UNKNOWN_VER;

	ERR_FAIL_COND_V(p_renderer_string.empty(), false);
	ERR_FAIL_COND_V(p_version_string.empty(), false);

	const CharString version_string = p_version_string.utf8();

	VideoDriver driver = DRIVER_UNKNOWN;
	DriverVersion driver_version = DRIVER_UNKNOWN_VER;

	int major, minor, rev, driver_major, driver_minor, driver_point;
	// This is the same on ES and regular GL.
	if (p_vendor_string == "freedreno") {
		driver = DRIVER_FREEDRENO;
	} else if (p_vendor_string.begins_with("Apple")) {
		driver = DRIVER_APPLE;
		int n = sscanf(version_string.c_str(),
				"%d.%d Metal - %d.%d",
				&major,
				&minor,
				&driver_major,
				&driver_minor);
		if (n == 4) {
			driver_version = DRIVER_VER(driver_major, driver_minor, 0);
		}
	} else if (VIDEO_IS_GL(p_standard)) {
		if (p_vendor == VENDOR_NVIDIA) {
			driver = DRIVER_NVIDIA;
			int n = sscanf(version_string.c_str(),
					"%d.%d.%d NVIDIA %d.%d",
					&major,
					&minor,
					&rev,
					&driver_major,
					&driver_minor);
			// Some older NVIDIA drivers don't report the driver version.
			if (n == 5) {
				driver_version = DRIVER_VER(driver_major, driver_minor, 0);
			}
		} else {
			int n = sscanf(version_string.c_str(),
					"%d.%d Mesa %d.%d",
					&major,
					&minor,
					&driver_major,
					&driver_minor);
			if (n != 4) {
				n = sscanf(version_string.c_str(),
						"%d.%d (Core Profile) Mesa %d.%d",
						&major,
						&minor,
						&driver_major,
						&driver_minor);
			}
			if (n == 4) {
				driver = DRIVER_MESA;
				driver_version = DRIVER_VER(driver_major, driver_minor, 0);
			}
		}
	} else if (p_standard == STANDARD_GLES) {
		if (p_vendor == VENDOR_NVIDIA) {
			driver = DRIVER_NVIDIA;
			int n = sscanf(version_string.c_str(),
					"OpenGL ES %d.%d NVIDIA %d.%d",
					&major,
					&minor,
					&driver_major,
					&driver_minor);
			// Some older NVIDIA drivers don't report the driver version.
			if (n == 4) {
				driver_version = DRIVER_VER(driver_major, driver_minor, 0);
			}
		} else if (p_vendor == VENDOR_IMAGINATION) {
			int revision;
			int n = sscanf(version_string.c_str(),
					"OpenGL ES %d.%d build %d.%d@%d",
					&major,
					&minor,
					&driver_major,
					&driver_minor,
					&revision);
			if (n == 5) {
				driver = DRIVER_IMAGINATION;
				driver_version = DRIVER_VER(driver_major, driver_minor, 0);
			}
		} else {
			int n = sscanf(version_string.c_str(),
					"OpenGL ES %d.%d Mesa %d.%d",
					&major,
					&minor,
					&driver_major,
					&driver_minor);
			if (n == 4) {
				driver = DRIVER_MESA;
				driver_version = DRIVER_VER(driver_major, driver_minor, 0);
			}
		}
	}

	if (driver == DRIVER_UNKNOWN) {
		if (p_vendor == VENDOR_INTEL) {
			// We presume we're on the Intel driver since it hasn't identified itself as Mesa.
			driver = DRIVER_INTEL;

			// This is how the macOS version strings are structured. This might be different on different OSes.
			int n = sscanf(version_string.c_str(),
					"%d.%d INTEL-%d.%d.%d",
					&major,
					&minor,
					&driver_major,
					&driver_minor,
					&driver_point);
			if (n == 5) {
				driver_version = DRIVER_VER(driver_major, driver_minor, driver_point);
			}
		} else if (p_vendor == VENDOR_QUALCOMM) {
			driver = DRIVER_QUALCOMM;
			int n = sscanf(version_string.c_str(),
					"OpenGL ES %d.%d V@%d.%d",
					&major,
					&minor,
					&driver_major,
					&driver_minor);
			if (n == 4) {
				driver_version = DRIVER_VER(driver_major, driver_minor, 0);
			}
		} else if (p_vendor == VENDOR_IMAGINATION) {
			int revision;
			int n = sscanf(version_string.c_str(),
					"OpenGL ES %d.%d build %d.%d@%d",
					&major,
					&minor,
					&driver_major,
					&driver_minor,
					&revision);
			if (n == 5) {
				// Revision is a large number (looks like a source control revision number) that
				// doesn't fit into the 'patch' bits, so omit it until we need it.
				driver_version = DRIVER_VER(driver_major, driver_minor, 0);
			}
		} else if (p_vendor == VENDOR_ARM) {
			// Example:
			// OpenGL ES 3.2 v1.r26p0-01rel0.217d2597f6bd19b169343737782e56e3
			// It's unclear how to interpret what comes between "p" and "rel". Every string we've
			// seen so far has "0-01" there. We ignore it for now.
			int ignored0;
			int ignored1;
			int n = sscanf(version_string.c_str(),
					"OpenGL ES %d.%d v%d.r%dp%d-%drel",
					&major,
					&minor,
					&driver_major,
					&driver_minor,
					&ignored0,
					&ignored1);
			if (n == 6) {
				driver = DRIVER_ARM;
				driver_version = DRIVER_VER(driver_major, driver_minor, 0);
			}
		} else {
			static constexpr char kEmulatorPrefix[] = "Android Emulator OpenGL ES Translator";
			if (p_renderer_string.begins_with(kEmulatorPrefix)) {
				driver = DRIVER_ANDROIDEMULATOR;
			}
		}
	}

	r_driver = driver, r_driver_version = driver_version;
	return true;
}

static bool get_angle_gl_vendor_and_renderer(const String &p_inner_string, const PoolStringArray &p_extensions, VideoVendor &r_angle_vendor, VideoRenderer &r_angle_renderer, VideoDriver &r_angle_driver, DriverVersion &r_angle_driver_version) {
	Vector<String> parts = p_inner_string.split(",");
	// This would need some fixing if we have substrings that contain commas.
	if (parts.size() != 3) {
		r_angle_vendor = VENDOR_OTHER, r_angle_renderer = RENDERER_OTHER, r_angle_driver = DRIVER_UNKNOWN, r_angle_driver_version = DRIVER_UNKNOWN_VER;
		return false;
	}

	const String angle_vendor_string = parts[0];
	const String angle_renderer_string = parts[1].right(-1); // skip initial space
	const String angle_version_string = parts[2].right(-1); // skip initial space

	r_angle_vendor = get_vendor(angle_vendor_string);
	r_angle_renderer = get_renderer(angle_renderer_string, p_extensions);
	get_driver_and_version(STANDARD_GLES, r_angle_vendor, angle_vendor_string, angle_renderer_string, angle_version_string, r_angle_driver, r_angle_driver_version);

	return true;
}

static bool get_angle_d3d_vendor_and_renderer(const String &p_inner_string, VideoVendor &r_angle_vendor, VideoRenderer &r_angle_renderer, VideoDriver &r_angle_driver, DriverVersion &r_angle_driver_version) {
	VideoVendor vendor = VENDOR_OTHER;
	VideoRenderer renderer = RENDERER_OTHER;

	if (p_inner_string.has("Intel")) {
		vendor = VENDOR_INTEL;

		const CharString inner_string = p_inner_string.utf8();

		const char *model_str;
		int model_number;
		if ((model_str = strstr(inner_string.c_str(), "HD Graphics")) &&
				(1 == sscanf(model_str, "HD Graphics %i", &model_number) ||
						1 == sscanf(model_str, "HD Graphics P%i", &model_number))) {
			switch (model_number) {
				case 2000:
				case 3000:
					renderer = RENDERER_INTELSANDYBRIDGE;
					break;
				case 4000:
				case 2500:
					renderer = RENDERER_INTELSANDYBRIDGE;
					break;
				case 510:
				case 515:
				case 520:
				case 530:
					renderer = RENDERER_INTELSKYLAKE;
					break;
			}
		} else if ((model_str = strstr(inner_string.c_str(), "Iris")) &&
				(1 == sscanf(model_str, "Iris(TM) Graphics %i", &model_number) || 1 == sscanf(model_str, "Iris(TM) Pro Graphics %i", &model_number) || 1 == sscanf(model_str, "Iris(TM) Pro Graphics P%i", &model_number))) {
			switch (model_number) {
				case 540:
				case 550:
				case 555:
				case 580:
					renderer = RENDERER_INTELSKYLAKE;
					break;
			}
		}
	} else if (p_inner_string.has("NVIDIA")) {
		vendor = VENDOR_NVIDIA;
	} else if (p_inner_string.has("Radeon")) {
		vendor = VENDOR_ATI;
	}
	// We haven't had a need yet to parse the D3D driver string.
	r_angle_vendor = vendor, r_angle_renderer = renderer, r_angle_driver = DRIVER_UNKNOWN, r_angle_driver_version = DRIVER_UNKNOWN_VER;
	return true;
}

static bool get_webgl_vendor_and_renderer(const Dictionary &p_context_info, VideoVendor &r_vendor, VideoRenderer &r_renderer) {
	if (!p_context_info.has(VideoContextInfoWebglInfo)) {
		r_vendor = VENDOR_OTHER, r_renderer = RENDERER_OTHER;
		return false;
	}

	const String webgl_vendor_string = Array(p_context_info[VideoContextInfoWebglInfo])[0];
	const String webgl_renderer_string = Array(p_context_info[VideoContextInfoWebglInfo])[1];

	r_vendor = get_vendor(webgl_vendor_string);
	r_renderer = get_renderer(webgl_renderer_string, p_context_info[VideoContextInfoVideoExtensions]);

	return true;
}

template <typename T>
T as_enum(const Variant &v) {
	return (T)(v.operator int());
}

VideoDriverInfo video_get_driver_info(const Dictionary &p_context_info) {
	const String version = p_context_info[VideoContextInfoVideoVersion];
	const String shading_version = p_context_info[VideoContextInfoVideoShadingLanguageVersion];
	const String renderer = p_context_info[VideoContextInfoVideoRenderer];
	const String vendor = p_context_info[VideoContextInfoVideoVendor];

	ERR_FAIL_COND_V(as_enum<VideoStandard>(p_context_info[VideoContextInfoVideoStandard]) == STANDARD_NONE, _default_none);

	VideoDriverInfo info = _default_none;
	info.standard = as_enum<VideoStandard>(p_context_info[VideoContextInfoVideoStandard]);
	info.standard_detected = get_standard_from_string(version);
	info.version = get_version_from_string(version);
	info.shading_lang_version = get_shading_lang_version(shading_version);
	info.vendor = get_vendor(vendor);
	info.renderer = get_renderer(renderer, p_context_info[VideoContextInfoVideoExtensions]);

	get_driver_and_version(info.standard, info.vendor, vendor, renderer, version, info.driver, info.driver_version);

	String inner_angle_renderer_string;
	get_angle_backend(renderer, info.angle_backend, inner_angle_renderer_string);

	if (info.angle_backend == ANGLE_D3D9 || info.angle_backend == ANGLE_D3D11) {
		get_angle_d3d_vendor_and_renderer(inner_angle_renderer_string, info.angle_vendor, info.angle_renderer, info.angle_driver, info.angle_driver_version);
	} else if (info.angle_backend == ANGLE_OPENGL) {
		get_angle_gl_vendor_and_renderer(inner_angle_renderer_string, p_context_info[VideoContextInfoVideoExtensions], info.angle_vendor, info.angle_renderer, info.angle_driver, info.angle_driver_version);
	}

	if (info.renderer == RENDERER_WEBGL) {
		get_webgl_vendor_and_renderer(p_context_info, info.webgl_vendor, info.webgl_renderer);
	}

	return info;
}

String video_get_video_version_string(VideoVersion p_version) { return string_format("%d.%d", VIDEO_MAJOR_VER(p_version), VIDEO_MINOR_VER(p_version)); }

#ifdef DOCTEST
TEST_CASE("Parsing driver informations") {
	SUBCASE("From Vendor string") {
		REQUIRE(get_vendor("Apple") == VENDOR_APPLE);
		REQUIRE(get_vendor("Intel Inc.") == VENDOR_INTEL);
	}
	SUBCASE("From Renderer string") {
		REQUIRE(get_renderer("Apple M1", PoolStringArray()) == RENDERER_APPLE_M);
		REQUIRE(get_renderer("Apple M1 Pro", PoolStringArray()) == RENDERER_APPLE_M);
		REQUIRE(get_renderer("Apple M1 Max", PoolStringArray()) == RENDERER_APPLE_M);
		REQUIRE(get_renderer("Intel(R) Iris(TM) Plus Graphics 655", PoolStringArray()) == RENDERER_INTELCOFFEELAKE);
		REQUIRE(get_renderer("Intel HD Graphics 4000 OpenGL Engine", PoolStringArray()) == RENDERER_INTELIVYBRIDGE);
	}
	SUBCASE("From Version string") {
		VideoDriver driver;
		DriverVersion driver_version;
		REQUIRE(get_driver_and_version(STANDARD_GL, get_vendor("Apple"), "Apple", "Apple M1 Pro", "4.1 Metal - 83.1", driver, driver_version));
		REQUIRE(driver == DRIVER_APPLE);
		REQUIRE(driver_version == 0x5300010000);
		REQUIRE(driver_version == DRIVER_VER(83, 1, 0));
		REQUIRE(get_driver_and_version(STANDARD_GL, get_vendor("Intel Inc."), "Intel Inc.", "Intel HD Graphics 4000 OpenGL Engine", "2.1 INTEL-14.7.28", driver, driver_version));
		REQUIRE(driver == DRIVER_INTEL);
		REQUIRE(driver_version == 0xE0007001C);
		REQUIRE(driver_version == DRIVER_VER(14, 7, 28));
	}
}
#endif // DOCTEST
