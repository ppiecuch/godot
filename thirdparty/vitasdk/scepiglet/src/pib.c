/*************************************************************************/
/*  pib.c                                                                */
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <psp2/io/fcntl.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/shacccg.h>

#define sceShaccCgSetMemAllocator sceShaccCgSetDefaultAllocator
#define SCE_NULL NULL

#include "taihen.h"

#include "common.h"
#include "debug.h"
#include "hooks.h"
#include "sha1.h"

static SceUID modID[4];
static SceBool pibIsInit = SCE_FALSE;
static char searchDir[200] = "ur0:data/external/"; // Max Path Length
static char modulePath[200];

static char *getModulePath(const char *moduleName) {
	memset(modulePath, '\0', sizeof(modulePath));
	strncpy(modulePath, searchDir, strlen(modulePath));
	strncat(modulePath, moduleName, strlen(modulePath));
	return modulePath;
}

static PibError loadModules(PibOptions options) {
	if (!(options & PIB_NOSTDLIB)) {
		if (modID[3] = sceKernelLoadStartModule("vs0:sys/external/libfios2.suprx", 0, SCE_NULL, 0, SCE_NULL, 0), modID[3] < 0 && modID[3] != 0x8002D014 && modID[3] != 0x8002D013)
			return PIB_ERROR_FIOS2_FAILED;
		if (modID[2] = sceKernelLoadStartModule("vs0:sys/external/libc.suprx", 0, SCE_NULL, 0, SCE_NULL, 0), modID[2] < 0 && modID[2] != 0x8002D014 && modID[2] != 0x8002D013)
			return PIB_ERROR_LIBC_FAILED;
	}
	if (modID[1] = sceKernelLoadStartModule(getModulePath("libScePiglet.suprx"), 0, SCE_NULL, 0, SCE_NULL, 0), modID[1] < 0)
		return PIB_ERROR_PIGLET_FAILED;
	if (options & PIB_SHACCCG) {
		if (modID[0] = sceKernelLoadStartModule(getModulePath("libshacccg.suprx"), 0, SCE_NULL, 0, SCE_NULL, 0), modID[0] < 0)
			return PIB_ERROR_SHACCCG_FAILED;
		sceShaccCgSetMemAllocator(malloc, free);
	}
	return PIB_SUCCESS;
}

static void unloadModules(void) {
	for (int i = 0; i < 4; i++) {
		if (modID[i])
			sceKernelStopUnloadModule(modID[i], 0, NULL, 0, NULL, 0);
	}
}

static void getResolutionConfig() {
	SceUID fd;
	fd = sceIoOpen("ur0:data/external/resolution.bin", SCE_O_RDONLY, 0666);
	if (fd < 0) {
		LOG("Error: File Open Error, 0x%08X.\nNo custom resolution set\n", fd);
		customResolutionMode = 0;
		return;
	}
	sceIoRead(fd, &customResolutionMode, sizeof(int));
	sceIoClose(fd);
}

PibError pibInit(PibOptions options) {
	if (pibIsInit) {
		LOG("PIB is already Initialized!\n");
		return PIB_ERROR_ALREADY_INIT; // PIB already Initialized
	}
	PibError ret = loadModules(options);
	if (ret)
		return ret;

	if (!(options & PIB_SYSTEM_MODE))
		getResolutionConfig();
	else
		customResolutionMode = 7; // Custom System Mode Resolution Configuration

	loadHooks(options);
	pibIsInit = SCE_TRUE;
	return PIB_SUCCESS;
}

PibError pibTerm(void) {
	if (!pibIsInit) {
		LOG("PIB is not Initialized!\n");
		return PIB_ERROR_NOT_INIT; // PIB isn't Initialized
	}
	unloadModules();
	releaseHooks();
	pibIsInit = SCE_FALSE;
	return PIB_SUCCESS;
}

#ifdef PIB_PRX
int module_stop(SceSize argc, const void *args) {
	LOG("PIB module stop\n");
	return SCE_KERNEL_STOP_SUCCESS;
}

int module_start(SceSize argc, void *args) {
	if (argc) {
		memset(modulePath, '\0', sizeof((char)args));
		strncpy(modulePath, (char)args, strlen((char)args));
	}
	LOG("PIB module start\n");
	return SCE_KERNEL_START_SUCCESS;
}
#endif
