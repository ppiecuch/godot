/*************************************************************************/
/*  hooks.c                                                              */
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

//
// Copyright (c) 2020 by SonicMastr <sonicmastr@gmail.com>
//
// This file is part of Pigs In A Blanket
//
// Pigs in a Blanket is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//

#include "hooks.h"
#include "debug.h"
#include "patches.h"
#include "shacccgpatch.h"
#include "sysmodepatch.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

tai_hook_ref_t hookRef[NUM_HOOKS];
SceUID hook[NUM_HOOKS];
int customResolutionMode;
tai_module_info_t modInfo;
int systemMode, msaaEnabled = 0;

void loadHooks(PibOptions options) {
	modInfo.size = sizeof(modInfo);
	taiGetModuleInfo("libScePiglet", &modInfo);
	if (options & PIB_SHACCCG) {
		hook[0] = taiHookFunctionOffset(&hookRef[0], modInfo.modid, 0, 0x32BB4, 1, pglPlatformShaderCompiler_CustomPatch);
		hook[1] = taiHookFunctionExport(&hookRef[1], modInfo.name, 0xB4FE1ABB, 0x919FBCB7, glGetBooleanv_shaderCompilerPatch);
		LOG("ShaccCg Patch: 0x%08x\nEnabled Shader Compiler Response: 0x%08x\n", hook[0], hook[1]);
	}
	hook[2] = taiHookFunctionOffset(&hookRef[2], modInfo.modid, 0, 0x39770, 1, _pglPlatformTextureUploadParams_patch);
	hook[3] = taiHookFunctionExport(&hookRef[3], modInfo.name, 0xB4FE1ABB, 0x4B86317A, eglCreateWindowSurface_resolutionPatch);
	LOG("Texture Upload Params Patch: 0x%08x\n", hook[2]);
	LOG("Resolution Patch: 0x%08x\n", hook[3]);
	if (options & PIB_GET_PROC_ADDR_CORE) {
		hook[4] = taiHookFunctionExport(&hookRef[4], modInfo.name, 0xB4FE1ABB, 0x249A431A, eglGetProcAddress_functionNamePatch);
		LOG("eglGetProcAddress Function Name Patch: 0x%08x\n", hook[4]);
	}
	hook[5] = taiHookFunctionExport(&hookRef[5], modInfo.name, 0xB4FE1ABB, 0x33A55EAB, eglGetConfigAttrib_intervalPatch);
	hook[6] = taiHookFunctionOffset(&hookRef[6], modInfo.modid, 0, 0x158F8, 1, pglDisplaySetSwapInterval_intervalPatch);
	hook[7] = taiHookFunctionImport(&hookRef[7], modInfo.name, 0x5ED8F994, 0x5795E898, sceDisplayWaitVblankStart_intervalPatch);
	LOG("Swap interval Patch: 0x%08x\nWaitVblankStart Patch: 0x%08x\n", hook[6], hook[7]);
	if ((options & PIB_SYSTEM_MODE) || (options & PIB_ENABLE_MSAA)) {
		hook[8] = taiHookFunctionOffset(&hookRef[8], modInfo.modid, 0, 0x17d24, 1, pglMemoryAllocAlign_patch);
		hook[10] = taiHookFunctionOffset(&hookRef[10], modInfo.modid, 0, 0x33074, 1, pglPlatformSurfaceCreateWindow_detect);
	}
	if (options & PIB_SYSTEM_MODE) {
		systemMode = 1;
		uint8_t cbnz_opcode = 0xB9;
		uint8_t mem_mode_two = 0x2;
		taiInjectData(modInfo.modid, 0, 0x33219, &cbnz_opcode, sizeof(cbnz_opcode));
		taiInjectData(modInfo.modid, 0, 0x2D2C0, &mem_mode_two, sizeof(mem_mode_two)); // Patch pglVitaMemoryAlloc to always use MAIN memblock
		taiInjectData(modInfo.modid, 0, 0x2D1DC, &mem_mode_two, sizeof(mem_mode_two)); //
		hook[9] = taiHookFunctionImport(&hookRef[9], modInfo.name, 0xF76B66BD, 0xB0F1E4EC, sceGxmInitialize_patch);
		hook[11] = taiHookFunctionImport(&hookRef[11], modInfo.name, 0xF76B66BD, 0x6A6013E1, sceGxmSyncObjectCreate_patch);
		hook[12] = taiHookFunctionOffset(&hookRef[12], modInfo.modid, 0, 0x2A85A, 1, pglPlatformContextBeginFrame_patch);
		hook[13] = taiHookFunctionOffset(&hookRef[13], modInfo.modid, 0, 0x33902, 1, pglPlatformSurfaceSwap_patch);
		hook[14] = taiHookFunctionOffset(&hookRef[14], modInfo.modid, 0, 0x337A6, 1, pglPlatformSurfaceDestroy_detect);
		hook[15] = taiHookFunctionImport(&hookRef[15], modInfo.name, 0xF76B66BD, 0x889AE88C, sceGxmSyncObjectDestroy_patch);
	}
	if (options & PIB_ENABLE_MSAA) {
		msaaEnabled = 1;
		hook[16] = taiHookFunctionImport(&hookRef[16], modInfo.name, 0xF76B66BD, 0xED0F6E25, sceGxmColorSurfaceInit_msaaPatch);
		hook[17] = taiHookFunctionImport(&hookRef[17], modInfo.name, 0xF76B66BD, 0x207AF96B, sceGxmCreateRenderTarget_msaaPatch);
		hook[18] = taiHookFunctionImport(&hookRef[18], modInfo.name, 0xF76B66BD, 0xCA9D41D1, sceGxmDepthStencilSurfaceInit_msaaPatch);
		hook[19] = taiHookFunctionImport(&hookRef[19], modInfo.name, 0xF76B66BD, 0x4ED2E49D, sceGxmShaderPatcherCreateFragmentProgram_msaaPatch);
		LOG("MSAA ENABLED!\n");
	}

	hook[20] = taiHookFunctionExport(&hookRef[20], modInfo.name, 0xB4FE1ABB, 0xFD616E54, glClear_loadPatch);
	hook[21] = taiHookFunctionImport(&hookRef[21], modInfo.name, 0xF76B66BD, 0x8734FF4E, sceGxmBeginScene_loadPatch);
	hook[22] = taiHookFunctionImport(&hookRef[22], modInfo.name, 0xCAE9ACE6, 0x1D8D7945, pglPlatformCriticalSectionEnter_patch);
	hook[23] = taiHookFunctionImport(&hookRef[23], modInfo.name, 0x859A24B1, 0x1A372EC8, pglPlatformCriticalSectionLeave_patch);
	hook[24] = taiHookFunctionImport(&hookRef[24], modInfo.name, 0xCAE9ACE6, 0xED53334A, pglPlatformCriticalSectionCreate_patch);
	hook[25] = taiHookFunctionImport(&hookRef[25], modInfo.name, 0x859A24B1, 0xCB78710D, pglPlatformCriticalSectionDestroy_patch);

	LOG("Hooked pglPlatformCriticalSectionEnter: 0x%08x\nHooked pglPlatformCriticalSectionLeave: 0x%08x\n", hook[22], hook[23]);
}

void releaseHooks(void) {
	for (int i = 0; i < NUM_HOOKS; i++) {
		taiHookRelease(hook[i], hookRef[i]);
	}
}
