/*************************************************************************/
/*  gdsfx.cpp                                                            */
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

#include <stdio.h>

#define GAUDIO_NO_ERROR 0
#define GAUDIO_CANNOT_OPEN_FILE -1
#define GAUDIO_UNRECOGNIZED_FORMAT -2
#define GAUDIO_ERROR_WHILE_READING -3
#define GAUDIO_UNSUPPORTED_FORMAT -4

#define g_id unsigned int
#define gaudio_Error int

#include <iostream>

#define LOG1(a) std::cout << a << "\n";
#define LOG2(a, b) std::cout << a << b << "\n";
#define LOG3(a, b, c) std::cout << a << b << c << "\n";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <map>

#include "gdsfx.h"
#include "retrosfxvoice.h"

struct GGSFXHandle {
	int nBytesRead;
	RetroSFXVoice *pSFXVoice;
};
static std::map<g_id, GGSFXHandle *> ctxmap;
static g_id s_nextgid = 1;

g_id g_NextId() {
	return s_nextgid++;
}

g_id gaudio_SFXOpen(const char *fileName, int *numChannels, int *sampleRate, int *bitsPerSample, int *numSamples, gaudio_Error *error) {
	GGSFXHandle *handle = new GGSFXHandle();

	handle->nBytesRead = 0;
	handle->pSFXVoice = new RetroSFXVoice();

	if (handle->pSFXVoice->LoadSettings(fileName) == false) {
		if (error) {
			*error = GAUDIO_ERROR_WHILE_READING;
			delete handle;
			return 0;
		}
	}

	handle->pSFXVoice->Play();

	if (numChannels) {
		*numChannels = 1;
	}
	if (sampleRate) {
		*sampleRate = 44100;
	}
	if (bitsPerSample) {
		*bitsPerSample = 16;
	}
	if (numSamples) {
		*numSamples = handle->pSFXVoice->GetVoiceLengthInSamples();
	}
	if (error) {
		*error = GAUDIO_NO_ERROR;
	}

	g_id gid = g_NextId();
	ctxmap[gid] = handle;

	return gid;
}

void gaudio_SFXClose(g_id id) {
	GGSFXHandle *handle = ctxmap[id];
	ctxmap.erase(id);
	if (handle) {
		if (handle->pSFXVoice) {
			delete handle->pSFXVoice;
		}
		delete handle;
	}
}

int gaudio_SFXSeek(g_id id, long int offset, int whence) {
	GGSFXHandle *handle = ctxmap[id];

	if ((offset == 0) && (whence == SEEK_SET)) {
		handle->pSFXVoice->Play();
		handle->nBytesRead = 0;
	} else if (whence == SEEK_CUR) {
		handle->nBytesRead += offset;
	} else if ((offset == 0) && (whence == SEEK_END)) {
		handle->pSFXVoice->Play();
		handle->nBytesRead = handle->pSFXVoice->GetVoiceLengthInSamples() * 2;
	}

	return handle->nBytesRead;
}

long int gaudio_SFXTell(g_id id) {
	GGSFXHandle *handle = ctxmap[id];
	return handle->nBytesRead;
}

size_t gaudio_SFXRead(g_id id, size_t size, void *data) {
	GGSFXHandle *handle = ctxmap[id];

	// clamp the size we will be rendering
	int nActualSize = handle->pSFXVoice->GetVoiceLengthInSamples() * 2;
	if ((handle->nBytesRead + size) > nActualSize) {
		size = (nActualSize - handle->nBytesRead);
	}

	memset(data, 0, size);

	// i hope size is a multiple of two here....
	int nLocalBytesRead = handle->pSFXVoice->Render(size / 2, (short *)data) * 2;
	handle->nBytesRead += nLocalBytesRead;

	return nLocalBytesRead;
}
