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
#include "core/print_string.h"

#if REDERDOC_APP_ENABLED

#if !WINDOWS_ENABLED && !X11_ENABLED
#error "Only Windows and Linux builds are supported"
#endif

#include "common/gd_core.h"

#if WINDOWS_ENABLED
#include <psapi.h>
#endif
#include <renderdoc/renderdoc_app.h>

#include <string.h>

// References:
// -----------
//  - https://web.archive.org/web/20181115035420/http://cnicholson.net/2011/01/stupid-c-tricks-a-better-sizeof_array/

template <typename Ty, size_t Num>
char (&CountOfRequireArrayArgumentT(const Ty (&)[Num]))[Num];

#define COUNTOF(_x) sizeof(CountOfRequireArrayArgumentT(_x))
#define UNUSED(x) (void)(x)

#ifndef CONFIG_RENDERDOC_CAPTURE_KEYS
#define CONFIG_RENDERDOC_CAPTURE_KEYS { eRENDERDOC_Key_F11 }
#endif // CONFIG_RENDERDOC_CAPTURE_KEYS

void* _dlopen(const char *p_file_path) {
#if WINDOWS_ENABLED
	return (void*)::LoadLibraryA(p_file_path);
#elif  X11_ENABLED
	void* so = ::dlopen(p_file_path, RTLD_LOCAL|RTLD_LAZY);
	WARN_PRINT(nullptr != so, "dlopen failed: \"%s\".", ::dlerror() );
	return so;
#else
	UNUSED(p_file_path);
	return nullptr;
#endif
}

void _dlclose(void* p_handle) {
	if (nullptr == p_handle) {
		return;
	}
#if WINDOWS_ENABLED
	::FreeLibrary( (HMODULE)p_handle);
#elif  X11_ENABLED
	::dlclose(p_handle);
#else
	UNUSED(p_handle);
#endif
}

void* _dlsym(void* p_handle, const const *p_symbol) {
	const int32_t symbol_max = strlen(p_symbol) + 1;
	char* symbol = (char*)alloca(symbol_max);
	strncpy(symbol, symbol_max, p_symbol);

#if WINDOWS_ENABLED
	return (void*)::GetProcAddress( (HMODULE)p_handle, symbol);
#elif  X11_ENABLED
	return ::dlsym(p_handle, symbol);
#else
	UNUSED(p_handle, symbol);
	return nullptr;
#endif
}


static void *_gdFindModule(const char *p_name) {
#if WINDOWS_ENABLED
	// NOTE: there was some reason to do it this way instead of simply calling GetModuleHandleA,
	// but not sure what it was.
	HANDLE process = GetCurrentProcess();
	DWORD size;
	BOOL result = EnumProcessModules(process, nullptr, 0, &size);
	if (0 != result) {
		HMODULE *modules = (HMODULE *)alloca(size);
		result = EnumProcessModules(process, modules, size, &size);

		if (0 != result) {
			char module_name[MAX_PATH];
			for (uint32_t ii = 0, num = uint32_t(size / sizeof(HMODULE)); ii < num; ++ii) {
				result = GetModuleBaseNameA(process, modules[ii], module_name, COUNTOF(module_name));
				if (0 != result && 0 == _stricmp(p_name, module_name)) {
					return (void *)modules[ii];
				}
			}
		}
	}
#else
	UNUSED(p_name);
#endif // WINDOWS_ENABLED
	return nullptr;
}

pRENDERDOC_GetAPI RENDERDOC_GetAPI;
static RENDERDOC_API_1_1_2 *s_render_doc = nullptr;
static void *s_render_doc_dll = nullptr;

bool gdIsRenderDocModuleCompiled() {
	return true;
}

void *gdLoadRenderDoc() {
	if (nullptr != s_render_doc) {
		return s_render_doc_dll;
	}

	// Skip loading RenderDoc when IntelGPA is present to avoid RenderDoc crash.
	if (_gdFindModule(GD_ARCH_32BIT ? "shimloader32.dll" : "shimloader64.dll")) {
		return nullptr;
	}

	// If RenderDoc is already injected in the process then use the already present DLL
	void *render_doc_dll = _gdFindModule("renderdoc.dll");
	if (nullptr == render_doc_dll) {
		// TODO: try common installation paths before looking in current directory
		render_doc_dll = _dlopen(
#if WINDOWS_ENABLED
				"renderdoc.dll"
#else
				"./librenderdoc.so"
#endif // WINDOWS_ENABLED
		);
	}

	if (nullptr != render_doc_dll) {
		RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)_dlsym(render_doc_dll, "RENDERDOC_GetAPI");

		if (nullptr != RENDERDOC_GetAPI && 1 == RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&s_render_doc)) {
			s_render_doc->SetCaptureFilePathTemplate(BGFX_CONFIG_RENDERDOC_LOG_FILEPATH);

			s_render_doc->SetFocusToggleKeys(nullptr, 0);

			RENDERDOC_InputButton capture_keys[] = BGFX_CONFIG_RENDERDOC_CAPTURE_KEYS;
			s_render_doc->SetCaptureKeys(capture_keys, COUNTOF(capture_keys));

			s_render_doc->SetCaptureOptionU32(eRENDERDOC_Option_AllowVSync, 1);
			s_render_doc->SetCaptureOptionU32(eRENDERDOC_Option_SaveAllInitials, 1);

			s_render_doc->MaskOverlayBits(eRENDERDOC_Overlay_None, eRENDERDOC_Overlay_None);

			s_render_doc_dll = render_doc_dll;
		} else {
			dlclose(render_doc_dll);
			render_doc_dll = nullptr;
		}
	}

	return render_doc_dll;
}

void gdUnloadRenderDoc(void *p_renderdocdll) {
	if (nullptr != p_renderdocdll) {
		// Once RenderDoc is loaded there shouldn't be calls
		// to Shutdown or unload RenderDoc DLL.
	}
}

void gdRenderDocTriggerCapture() {
	if (nullptr != s_render_doc) {
		s_render_doc->TriggerCapture();
	}
}

#else // REDERDOC_APP_ENABLED

bool gdIsRenderDocModuleCompiled() {
	return false;
}

void *gdLoadRenderDoc() {
	return nullptr;
}

void gdUnloadRenderDoc(void *) {}

void gdRenderDocTriggerCapture() {
	print_verbose("RenderDoc module not compiled.");
}

#endif // REDERDOC_APP_ENABLED
