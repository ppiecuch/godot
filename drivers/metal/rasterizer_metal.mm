/*************************************************************************/
/*  rasterizer_metal.mm                                                  */
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

#ifdef METAL_ENABLED

#include "rasterizer_metal.h"

#include <Availability.h>

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#ifdef __MACOSX__
#import <AppKit/NSView.h>
#endif

static inline Error is_metal_available() {
	// this checks a weak symbol.
#if (defined(__MACOSX__) && (MAC_OS_X_VERSION_MIN_REQUIRED < 101100))
	if (MTLCreateSystemDefaultDevice == nullptr) { // probably on 10.10 or lower.
		ERR_PRINT("Metal framework not available on this system");
		return ERR_UNAVAILABLE;
	}
#endif

	return OK;
}

static id<MTLDevice> get_default_metal_device() {
	id<MTLDevice> device = MTLCreateSystemDefaultDevice();
	if (!device) {
		// check manually for available devices
		NSArray<id<MTLDevice>> *devices = MTLCopyAllDevices();
		if(devices) {
			NSLog(@"   |                                |           | C           C|");
			NSLog(@"   |                                |           | o    A      a|");
			NSLog(@"   |                                |           | m    p      t|");
			NSLog(@"Nr | Name                           | Low power | m    p    M a|");
			NSLog(@"   |                                |           | o    l    a l|");
			NSLog(@"   |                                |           | n    e    c s|");
			NSLog(@"   |                                |           |12312345671212|");
			NSLog(@"---+--------------------------------+-----------+--------------+");
			for (int i = 0; i < devices.count; ++i) {
				if(devices[i]) {
					const int FamilyCapsEnums = 14;
					char family_str[FamilyCapsEnums+1] = "______________";
if (@available(macOS 10.15, *)) {
					static MTLGPUFamily family[FamilyCapsEnums] = {MTLGPUFamilyCommon1, MTLGPUFamilyCommon2, MTLGPUFamilyCommon3, MTLGPUFamilyApple1, MTLGPUFamilyApple2, MTLGPUFamilyApple3, MTLGPUFamilyApple4, MTLGPUFamilyApple5, MTLGPUFamilyApple6, MTLGPUFamilyApple7, MTLGPUFamilyMac1, MTLGPUFamilyMac2, MTLGPUFamilyMacCatalyst1, MTLGPUFamilyMacCatalyst2};
					for (int f = 0; f < FamilyCapsEnums; ++f) {
						if (family[f] == -1) {
							family_str[f] = '?';
						} else if ([devices[i] supportsFamily:family[f]]) {
							family_str[f] = '.';
						} else {
							family_str[f] = ' ';
						}
					}
}
					if (!device || !devices[i].lowPower) {
						device = devices[i];
					}
					NSLog(@"%2d | %30s |       %3s |%s|", i, [devices[i].name UTF8String], devices[i].lowPower ? "yes" : "no", family_str);
				}
			}
		}
	}
	return device;
}

void RasterizerMetal::set_boot_image(const Ref<Image> &p_image, const Color &p_color, bool p_scale, bool p_use_filter) {}
void RasterizerMetal::set_shader_time_scale(float p_scale) {}

void RasterizerMetal::initialize() {
	device = get_default_metal_device();
	if (!device) {
		ERR_PRINT("Cannot create default Metal device - not available or not supported");
		return;
	}
}
void RasterizerMetal::begin_frame(double frame_step) {}
void RasterizerMetal::set_current_render_target(RID p_render_target) {}
void RasterizerMetal::restore_render_target(bool p_3d_was_drawn) {}
void RasterizerMetal::clear_render_target(const Color &p_color) {}
void RasterizerMetal::blit_render_target_to_screen(RID p_render_target, const Rect2 &p_screen_rect, int p_screen) {}
void RasterizerMetal::output_lens_distorted_to_screen(RID p_render_target, const Rect2 &p_screen_rect, float p_k1, float p_k2, const Vector2 &p_eye_center, float p_oversample) {}
void RasterizerMetal::end_frame(bool p_swap_buffers) {}
void RasterizerMetal::finalize() {}

Error RasterizerMetal::is_viable() {
	return is_metal_available();
}

Rasterizer *RasterizerMetal::_create_current() {
	return memnew(RasterizerMetal);
}

void RasterizerMetal::make_current() {
	_create_func = _create_current;
}

bool RasterizerMetal::is_low_end() const {
	return true;
}

const char *gl_check_for_error(bool p_print_error) {
	return nullptr;
}

RasterizerMetal::RasterizerMetal() {}
RasterizerMetal::~RasterizerMetal() {}

#endif // METAL_ENABLED
