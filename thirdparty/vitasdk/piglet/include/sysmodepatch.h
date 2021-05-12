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

#ifndef SYSMODEPATCH_H_
#define SYSMODEPATCH_H_

#include <psp2/appmgr.h>
#include <psp2/gxm.h>

SceGxmErrorCode sceGxmInitialize_patch(const SceGxmInitializeParams *params);
unsigned int pglMemoryAllocAlign_patch(int memoryType, int size, int unused, unsigned int *memory);
void *pglPlatformSurfaceCreateWindow_detect(int a1, int a2, int a3, int a4, int *a5);
SceGxmErrorCode sceGxmSyncObjectCreate_patch(SceGxmSyncObject **syncObject);
int pglPlatformContextBeginFrame_patch(int context, int framebuffer);
int pglPlatformSurfaceSwap_patch(int surface);
void pglPlatformSurfaceDestroy_detect(int surface);
SceGxmErrorCode sceGxmSyncObjectDestroy_patch(SceGxmSyncObject *syncObject);

#endif /* SYSMODEPATCH_H_ */
