/*************************************************************************/
/*  debug_renderdoc.cpp                                                  */
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

#include "debug_renderdoc.h"

#if WINDOWS_ENABLED || X11_ENABLED

#include "common/gd_core.h"

#if WINDOWS_ENABLED
#include <psapi.h>
#endif
#include <renderdoc/renderdoc_app.h>

// References:
// - https://web.archive.org/web/20181115035420/http://cnicholson.net/2011/01/stupid-c-tricks-a-better-sizeof_array/
template <typename Ty, size_t Num>
char (&CountOfRequireArrayArgumentT(const Ty (&)[Num]))[Num];

#define COUNTOF(_x) sizeof(CountOfRequireArrayArgumentT(_x))
#define UNUSED(x) (void)(x)

void *GdFindModule(const char *_name) {
#if WINDOWS_ENABLED
	// NOTE: there was some reason to do it this way instead of simply calling GetModuleHandleA,
	// but not sure what it was.
	HANDLE process = GetCurrentProcess();
	DWORD size;
	BOOL result = EnumProcessModules(process, NULL, 0, &size);
	if (0 != result) {
		HMODULE *modules = (HMODULE *)alloca(size);
		result = EnumProcessModules(process, modules, size, &size);

		if (0 != result) {
			char moduleName[MAX_PATH];
			for (uint32_t ii = 0, num = uint32_t(size / sizeof(HMODULE)); ii < num; ++ii) {
				result = GetModuleBaseNameA(process, modules[ii], moduleName, COUNTOF(moduleName));
				if (0 != result && 0 == _stricmp(_name, moduleName)) {
					return (void *)modules[ii];
				}
			}
		}
	}
#else
	UNUSED(_name);
#endif // WINDOWS_ENABLED

	return nullptr;
}

pRENDERDOC_GetAPI RENDERDOC_GetAPI;
static RENDERDOC_API_1_1_2 *s_renderDoc = nullptr;
static void *s_renderDocDll = nullptr;

void *GdLoadRenderDoc() {
	if (nullptr != s_renderDoc) {
		return s_renderDocDll;
	}

	// Skip loading RenderDoc when IntelGPA is present to avoid RenderDoc crash.
	if (findModule(GD_ARCH_32BIT ? "shimloader32.dll" : "shimloader64.dll")) {
		return nullptr;
	}

	// If RenderDoc is already injected in the process then use the already present DLL
	void *renderDocDll = findModule("renderdoc.dll");
	if (nullptr == renderDocDll) {
		// TODO: try common installation paths before looking in current directory
		renderDocDll = bx::dlopen(
#if WINDOWS_ENABLED
				"renderdoc.dll"
#else
				"./librenderdoc.so"
#endif // WINDOWS_ENABLED
		);
	}

	if (nullptr != renderDocDll) {
		RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)bx::dlsym(renderDocDll, "RENDERDOC_GetAPI");

		if (nullptr != RENDERDOC_GetAPI && 1 == RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&s_renderDoc)) {
			s_renderDoc->SetCaptureFilePathTemplate(BGFX_CONFIG_RENDERDOC_LOG_FILEPATH);

			s_renderDoc->SetFocusToggleKeys(nullptr, 0);

			RENDERDOC_InputButton captureKeys[] = BGFX_CONFIG_RENDERDOC_CAPTURE_KEYS;
			s_renderDoc->SetCaptureKeys(captureKeys, BX_COUNTOF(captureKeys));

			s_renderDoc->SetCaptureOptionU32(eRENDERDOC_Option_AllowVSync, 1);
			s_renderDoc->SetCaptureOptionU32(eRENDERDOC_Option_SaveAllInitials, 1);

			s_renderDoc->MaskOverlayBits(eRENDERDOC_Overlay_None, eRENDERDOC_Overlay_None);

			s_renderDocDll = renderDocDll;
		} else {
			bx::dlclose(renderDocDll);
			renderDocDll = nullptr;
		}
	}

	return renderDocDll;
}

void GdUnloadRenderDoc(void *_renderdocdll) {
	if (nullptr != _renderdocdll) {
		// Once RenderDoc is loaded there shouldn't be calls
		// to Shutdown or unload RenderDoc DLL.
	}
}

void GdRenderDocTriggerCapture() {
	if (nullptr != s_renderDoc) {
		s_renderDoc->TriggerCapture();
	}
}

#else

void *GdLoadRenderDoc() {
	return nullptr;
}

void GdUnloadRenderDoc(void *) {}

void GdRenderDocTriggerCapture() {}

#endif // WINDOWS_ENABLED || X11_ENABLED
