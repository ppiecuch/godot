#include "debug_renderdoc.h"

#if WINDOWS_ENABLED || X11_ENABLED
#	if WINDOWS_ENABLED
#		include <psapi.h>
#	endif // WINDOWS_ENABLED
#	include <renderdoc/renderdoc_app.h>

// References:
// - https://web.archive.org/web/20181115035420/http://cnicholson.net/2011/01/stupid-c-tricks-a-better-sizeof_array/
template<typename Ty, size_t Num> char(&CountOfRequireArrayArgumentT(const Ty(&)[Num]))[Num];

#define COUNTOF(_x) sizeof(CountOfRequireArrayArgumentT(_x))
#define UNUSED(x) (void)(x)

void* GdFindModule(const char* _name) {
#if WINDOWS_ENABLED
	// NOTE: there was some reason to do it this way instead of simply calling GetModuleHandleA,
	// but not sure what it was.
	HANDLE process = GetCurrentProcess();
	DWORD size;
	BOOL result = EnumProcessModules(process
					, NULL
					, 0
					, &size
					);
	if (0 != result) {
		HMODULE* modules = (HMODULE*)alloca(size);
		result = EnumProcessModules(process
			, modules
			, size
			, &size
			);

		if (0 != result) {
			char moduleName[MAX_PATH];
			for (uint32_t ii = 0, num = uint32_t(size/sizeof(HMODULE) ); ii < num; ++ii) {
				result = GetModuleBaseNameA(process
							, modules[ii]
							, moduleName
							, COUNTOF(moduleName)
							);
				if (0 != result &&  0 == _stricmp(_name, moduleName) ) {
					return (void*)modules[ii];
				}
			}
		}
	}
#else
	UNUSED(_name);
#endif // WINDOWS_ENABLED

	return NULL;
}

pRENDERDOC_GetAPI RENDERDOC_GetAPI;
static RENDERDOC_API_1_1_2* s_renderDoc = NULL;
static void* s_renderDocDll = NULL;

void* GdLoadRenderDoc() {
	if (NULL != s_renderDoc) {
		return s_renderDocDll;
	}

	// Skip loading RenderDoc when IntelGPA is present to avoid RenderDoc crash.
	if (findModule(BX_ARCH_32BIT ? "shimloader32.dll" : "shimloader64.dll") ) {
		return NULL;
	}

	// If RenderDoc is already injected in the process then use the already present DLL
	void* renderDocDll = findModule("renderdoc.dll");
	if (NULL == renderDocDll) {
		// TODO: try common installation paths before looking in current directory
		renderDocDll = bx::dlopen(
#if WINDOWS_ENABLED
				"renderdoc.dll"
#else
				"./librenderdoc.so"
#endif // WINDOWS_ENABLED
				);
	}

	if (NULL != renderDocDll) {
		RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)bx::dlsym(renderDocDll, "RENDERDOC_GetAPI");

		if (NULL != RENDERDOC_GetAPI &&  1 == RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void**)&s_renderDoc) ) {
			s_renderDoc->SetCaptureFilePathTemplate(BGFX_CONFIG_RENDERDOC_LOG_FILEPATH);

			s_renderDoc->SetFocusToggleKeys(NULL, 0);

			RENDERDOC_InputButton captureKeys[] = BGFX_CONFIG_RENDERDOC_CAPTURE_KEYS;
			s_renderDoc->SetCaptureKeys(captureKeys, BX_COUNTOF(captureKeys) );

			s_renderDoc->SetCaptureOptionU32(eRENDERDOC_Option_AllowVSync,      1);
			s_renderDoc->SetCaptureOptionU32(eRENDERDOC_Option_SaveAllInitials, 1);

			s_renderDoc->MaskOverlayBits(eRENDERDOC_Overlay_None, eRENDERDOC_Overlay_None);

			s_renderDocDll = renderDocDll;
		} else {
			bx::dlclose(renderDocDll);
			renderDocDll = NULL;
		}
	}

	return renderDocDll;
}

void GdUnloadRenderDoc(void* _renderdocdll) {
	if (NULL != _renderdocdll) {
		// Once RenderDoc is loaded there shouldn't be calls
		// to Shutdown or unload RenderDoc DLL.
	}
}

void GdRenderDocTriggerCapture() {
	if (NULL != s_renderDoc) {
		s_renderDoc->TriggerCapture();
	}
}

#else

void* GdLoadRenderDoc() { return NULL; }

void GdUnloadRenderDoc(void*) { }

void GdRenderDocTriggerCapture() { }

#endif // WINDOWS_ENABLED || X11_ENABLED
