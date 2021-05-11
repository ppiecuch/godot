/*************************************************************************/
/*  sysmodepatch.c                                                       */
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

 *
 *  Copyright (c) 2020 by SonicMastr <sonicmastr@gmail.com>
 *
 *  This file is part of Pigs In A Blanket
 *
 *  Pigs in a Blanket is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <psp2/kernel/threadmgr.h>
#include <psp2/sharedfb.h>

#include "debug.h"
#include "hooks.h"
#include "sysmodepatch.h"

// vitasdk lacks prototypes for these right now
SceGxmErrorCode sceGxmVshSyncObjectOpen(uint32_t key, SceGxmSyncObject **syncObject);
SceGxmErrorCode sceGxmVshSyncObjectClose(uint32_t key, SceGxmSyncObject *syncObject);

// sceGxmVsh stuff also has different names in vitasdk/dolcesdk
#define sceGxmInitializeInternal sceGxmVshInitialize
#define sceGxmSyncObjectOpenShared sceGxmVshSyncObjectOpen
#define sceGxmSyncObjectCloseShared sceGxmVshSyncObjectClose

// this struct is already defined in vitasdk; use a unique name here
typedef struct SceSharedFbInfoPib { // size is 0x58
	void *base1; // cdram base
	int memsize;
	void *base2; // cdram base
	int unk_0C;
	void *unk_10;
	int unk_14;
	int unk_18;
	int unk_1C;
	int unk_20;
	int unk_24; // 960
	int unk_28; // 960
	int unk_2C; // 544
	int unk_30;
	int curbuf;
	int unk_38;
	int unk_3C;
	int unk_40;
	int unk_44;
	int owner;
	int unk_4C;
	int unk_50;
	int unk_54;
} SceSharedFbInfoPib;

static SceSharedFbInfoPib info;
static SceUID shfb_id;
static void *displayBufferData[2];
static int isDestroyingSurface, currentContext, framebegun = 0;
static unsigned int bufferDataIndex;
int isCreatingSurface;

SceGxmErrorCode sceGxmInitialize_patch(const SceGxmInitializeParams *params) {
	SceGxmInitializeParams gxm_init_params_internal;
	memset(&gxm_init_params_internal, 0, sizeof(SceGxmInitializeParams));
	gxm_init_params_internal.flags = 0x0A;
	gxm_init_params_internal.displayQueueMaxPendingCount = 2;
	gxm_init_params_internal.parameterBufferSize = 0x200000;

	SceGxmErrorCode error = sceGxmInitializeInternal(&gxm_init_params_internal);
	if (error != 0)
		return error;

	while (1) {
		shfb_id = sceSharedFbOpen(1);
		sceSharedFbGetInfo(shfb_id, (SceSharedFbInfo *)&info); // cast to proper struct; they're identical
		sceKernelDelayThread(40);
		if (info.curbuf == 1)
			sceSharedFbClose(shfb_id);
		else
			break;
	}

	sceGxmMapMemory(info.base1, info.memsize, SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE);

	displayBufferData[0] = info.base1;
	displayBufferData[1] = info.base2;

	return error;
}

unsigned int pglMemoryAllocAlign_patch(int memoryType, int size, int unused, unsigned int *memory) {
	if (systemMode && memoryType == 4 && isCreatingSurface) // ColorSurface/Framebuffer Allocation. We want to skip this and replace with SharedFb Framebuffer
	{
		memory[0] = (unsigned int)displayBufferData[bufferDataIndex];
		return 0;
	}
	if (msaaEnabled && memoryType == 5 && isCreatingSurface) {
		size *= 4; // For MSAA
	}
	return TAI_CONTINUE(unsigned int, hookRef[8], memoryType, size, unused, memory);
}

void *pglPlatformSurfaceCreateWindow_detect(int a1, int a2, int a3, int a4, int *a5) {
	isCreatingSurface = 1;
	bufferDataIndex = 0;
	void *ret = TAI_CONTINUE(void *, hookRef[10], a1, a2, a3, a4, a5);
	isCreatingSurface = 0;
	return ret;
}

SceGxmErrorCode sceGxmSyncObjectCreate_patch(SceGxmSyncObject **syncObject) {
	if (isCreatingSurface) {
		bufferDataIndex++;
		LOG("Patching Sync Object #%d\n", bufferDataIndex);
		SceGxmErrorCode ret = sceGxmSyncObjectOpenShared(bufferDataIndex, syncObject);
		return ret;
	}
	TAI_CONTINUE(SceGxmErrorCode, hookRef[11], syncObject);
	return 0;
}

int pglPlatformContextBeginFrame_patch(int context, int framebuffer) {
	if (!*(int *)(context + 0x12bf0) && !framebegun) { // GL FrameBuffer Object ID
		sceSharedFbBegin(shfb_id, (SceSharedFbInfo *)&info);
		info.owner = 1;
		framebegun = 1;
	}
	currentContext = context;
	return TAI_CONTINUE(int, hookRef[12], context, framebuffer);
}

int pglPlatformSurfaceSwap_patch(int surface) // Completely rewritten for System Mode
{
	int bufferIndexNew, bufferIndexOld, framebufferAddress, var4;
	sceSharedFbEnd(shfb_id);
	if (info.curbuf == 1)
		bufferIndexOld = 1;
	else
		bufferIndexOld = 0;
	bufferIndexNew = *(int *)(surface + 0x7c); // Prepare to swap Shared FrameBuffer
	LOG("Buffer Old: %d\n", bufferIndexNew);
	*(int *)(surface + 0x78) = bufferIndexNew;
	*(int *)(surface + 0x7c) = bufferIndexOld;
	*(int *)(surface + 0x60) = surface + bufferIndexOld * 0x30 + 0xa8;
	bufferIndexNew = bufferIndexOld * 4 + surface;
	bufferIndexOld = 0;
	if (*(int *)(surface + 0x128) != 0)
		bufferIndexOld = surface + 0x124;
	framebufferAddress = *(int *)(bufferIndexNew + 0x108);
	*(int *)(surface + 100) = bufferIndexOld;
	*(int *)(surface + 0x68) = *(int *)(surface + 0x94);
	var4 = *(int *)(bufferIndexNew + 0x98);
	*(int *)(surface + 0x6c) = framebufferAddress;
	*(int *)(surface + 0x70) = var4;
	LOG("Buffer to write: %d\n", *(int *)(surface + 0x7c));
	framebegun = 0; // Reset drawing status
	return 1;
}

void pglPlatformSurfaceDestroy_detect(int surface) {
	isDestroyingSurface = 1;
	TAI_CONTINUE(void, hookRef[14], surface);
	isDestroyingSurface = 0;
}

SceGxmErrorCode sceGxmSyncObjectDestroy_patch(SceGxmSyncObject *syncObject) {
	if (isDestroyingSurface) {
		SceGxmErrorCode ret = sceGxmSyncObjectCloseShared(bufferDataIndex, syncObject);
		bufferDataIndex--;
		return ret;
	}
	return TAI_CONTINUE(SceGxmErrorCode, hookRef[15], syncObject);
}
