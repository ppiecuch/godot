/*************************************************************************/
/*  retrosfxvoice.cpp                                                    */
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

//
// This code was originally part of a program called SFXR written by Dr Petter Circa 2007.
// http://www.drpetter.se/project_sfxr.html
// I took this code and modified it somewhat for Gilderos procedural sound effects. I also added
// a new sfx type so that sfx can morph between two sets of settings and produce evolving
// sfx that sound very much like a game called Robotron.
// Using this system means each sound effect is around 200 bytes

// The original code was released under the MIT license, and the modifications that I have made
// are also under the same license.
//
// Paul Carter 2018
//

// Reference:
// ----------
// 1. https://github.com/increpare/bfxr/blob/master/src/com/increpare/bfxr/synthesis/Synthesizer/SfxrSynth.as
// 2. https://github.com/hlywa/GM/blob/1a6fb9fc6a6c3ed10e733c6355d4a15286c2153c/Assets/Fungus/Thirdparty/Usfxr/Scripts/SfxrParams.cs

//////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS
#include "retrosfxvoice.h"

#include "core/math/math_funcs.h"
#include "core/os/dir_access.h"
#include "core/os/file_access.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // memcmp
#include <algorithm>
#include <memory>
#include <vector>

#define SUCCESS (0)
#define FAILURE (-1)

static DirAccess *_current_dir = nullptr;

struct G_FILE {
	FileAccess *fa;
	static String fixpath(const String &p_path) {
		if (_current_dir && !p_path.is_abs_path() && !FileAccess::exists(p_path)) {
			return _current_dir->get_current_dir().append_path(p_path);
		}
		return p_path;
	}
	static G_FILE *fopen(const String &p_path, FileAccess::ModeFlags p_mode) {
		if (!p_path.empty()) {
			const String real_path = fixpath(p_path);
			if (FileAccess *_fa = FileAccess::open(real_path, p_mode)) {
				return memnew(G_FILE(_fa));
			} else {
				return nullptr;
			}
		}
		return nullptr;
	}
	static G_FILE *fopen_read(const String &p_path) { return fopen(p_path, FileAccess::READ); }
	static G_FILE *fopen_write(const String &p_path) { return fopen(p_path, FileAccess::WRITE); }
	G_FILE(FileAccess *p_fa) { fa = p_fa; }
	G_FILE() { fa = nullptr; }
	~G_FILE() {
		if (fa) {
			memdelete(fa);
		}
	}
};

G_FILE *g_open_read(const char *name) {
	return G_FILE::fopen_read(name);
}

G_FILE *g_open_write(const char *name) {
	return G_FILE::fopen_write(name);
}

int g_close(G_FILE *f) {
	if (f) {
		if (f->fa) {
			f->fa->close();
			return SUCCESS;
		} else {
			return FAILURE;
		}
	}
	return FAILURE;
}

int g_seek(G_FILE *f, off_t offset, int whence) {
	if (f) {
		switch (whence) {
			case SEEK_SET: {
				f->fa->seek(offset);
			} break;
			case SEEK_CUR: {
				f->fa->seek(f->fa->get_position() + offset);
			} break;
			case SEEK_END: {
				f->fa->seek_end(offset);
			} break;
			default:
				return FAILURE;
		}
		return SUCCESS;
	}
	return FAILURE;
}

off_t g_tell(G_FILE *f) {
	if (f) {
		return f->fa->get_position();
	}
	return FAILURE;
}

size_t g_read(void *buf, size_t len, size_t cnt, G_FILE *f) {
	if (f) {
		return f->fa->get_buffer((uint8_t *)buf, cnt * len);
	}
	return 0;
}

size_t g_write(const void *buf, size_t len, size_t cnt, G_FILE *f) {
	if (f) {
		if (f->fa) {
			f->fa->store_buffer((const uint8_t *)buf, cnt * len);
		} else {
			WARN_PRINT("Undefined file access - information lost.");
			return 0;
		}
		return cnt * len;
	}
	return 0;
}

size_t g_filesize(G_FILE *f) {
	if (f) {
		return f->fa->get_len();
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////

#define rnd(n) (rand() % (n + 1))
#define frnd(r) (((rand() % (unsigned(RAND_MAX) + 1)) / float(RAND_MAX)) * r)

#define PI 3.14159265f

/*
 * Pink Number
 * -----------
 * From BFXR
 * Class taken from http://www.firstpr.com.au/dsp/pink-noise/#Filtering
 */
struct PinkNumber {
	// Properties
	int max_key, key;
	unsigned white_values[5];
	unsigned range;

	// Temp
	float _range;

#define _rnd1() ((rand() % (unsigned(RAND_MAX) + 1)) / float(RAND_MAX) * 2 - 1)

	PinkNumber() {
		max_key = 1; // Five bits set
		range = 128;
		_range = range / 5.0;
		key = 0;
		for (int i = 0; i < 5; i++) {
			white_values[i] = _rnd1() * _range;
		}
	}

	float getNextValue() {
		// Returns a number between -1 and 1
		int last_key = key;
		key++;
		if (key > max_key)
			key = 0;
		// Exclusive-Or previous value with current value. This gives
		// a list of bits that have changed.
		int diff = last_key ^ key;
		unsigned sum = 0;
		for (int i = 0; i < 5; i++) {
			// If bit changed get new random number for corresponding
			// white_value
			if ((diff & (1 << i)) > 0) {
				white_values[i] = _rnd1() * _range;
			}
			sum += white_values[i];
		}
		return sum / 64.0 - 1;
	}
};

//////////////////////////////////////////////////////////////////////////

RetroSFXVoice::RetroSFXVoice() {
	m_fMasterVol = 0.05;
	m_Voice.fSoundVol = 0.8;
	m_bPlayingSample = false;
	m_WavSamplesRendered = 0;

	ResetParams();
}

//////////////////////////////////////////////////////////////////////////

void RetroSFXVoice::ResetParams() {
	m_Voice.nWaveformType = 0;

	m_Voice.FXMorphParams.fBaseFreq = m_Voice.FXBaseParams.fBaseFreq = 0.3;
	m_Voice.FXMorphParams.fFreqLimit = m_Voice.FXBaseParams.fFreqLimit = 0;
	m_Voice.FXMorphParams.fFreqRamp = m_Voice.FXBaseParams.fFreqRamp = 0;
	m_Voice.FXMorphParams.fFreqDRamp = m_Voice.FXBaseParams.fFreqDRamp = 0;
	m_Voice.FXMorphParams.fDuty = m_Voice.FXBaseParams.fDuty = 0;
	m_Voice.FXMorphParams.fDutyRamp = m_Voice.FXBaseParams.fDutyRamp = 0;

	m_Voice.FXMorphParams.fVibStrength = m_Voice.FXBaseParams.fVibStrength = 0;
	m_Voice.FXMorphParams.fVibSpeed = m_Voice.FXBaseParams.fVibSpeed = 0;
	m_Voice.FXMorphParams.fVibDelay = m_Voice.FXBaseParams.fVibDelay = 0;

	m_Voice.FXMorphParams.fEnvAttack = m_Voice.FXBaseParams.fEnvAttack = 0;
	m_Voice.FXMorphParams.fEnvSustain = m_Voice.FXBaseParams.fEnvSustain = 0.3;
	m_Voice.FXMorphParams.fEnvDecay = m_Voice.FXBaseParams.fEnvDecay = 0.4;
	m_Voice.FXMorphParams.fEnvPunch = m_Voice.FXBaseParams.fEnvPunch = 0.0;

	m_Voice.FXMorphParams.fLPFResonance = m_Voice.FXBaseParams.fLPFResonance = 0;
	m_Voice.FXMorphParams.fLPFFreq = m_Voice.FXBaseParams.fLPFFreq = 1;
	m_Voice.FXMorphParams.fLPFRamp = m_Voice.FXBaseParams.fLPFRamp = 0;
	m_Voice.FXMorphParams.fHPFFreq = m_Voice.FXBaseParams.fHPFFreq = 0;
	m_Voice.FXMorphParams.fHPFRamp = m_Voice.FXBaseParams.fHPFRamp = 0;
	m_Voice.FXMorphParams.fBitCrush = m_Voice.FXBaseParams.fBitCrush = 0;
	m_Voice.FXMorphParams.fBitCrushSweep = m_Voice.FXBaseParams.fBitCrushSweep = 0;
	m_Voice.FXMorphParams.fCompressionAmount = m_Voice.FXBaseParams.fCompressionAmount = 0.3;

	m_Voice.FXMorphParams.fFlangerOffset = m_Voice.FXBaseParams.fFlangerOffset = 0;
	m_Voice.FXMorphParams.fFlangerRamp = m_Voice.FXBaseParams.fFlangerRamp = 0;

	m_Voice.FXMorphParams.fRepeatSpeed = m_Voice.FXBaseParams.fRepeatSpeed = 0;

	m_Voice.FXMorphParams.fArmRepeat = m_Voice.FXBaseParams.fArmRepeat = 0;
	m_Voice.FXMorphParams.fArmSpeed = m_Voice.FXBaseParams.fArmSpeed = 1;
	m_Voice.FXMorphParams.fArmMod = m_Voice.FXBaseParams.fArmMod = 0;
	m_Voice.FXMorphParams.fArmSpeed2 = m_Voice.FXBaseParams.fArmSpeed2 = 1;
	m_Voice.FXMorphParams.fArmMod2 = m_Voice.FXBaseParams.fArmMod2 = 0;

	m_Voice.FXMorphParams.fOvertones = m_Voice.FXBaseParams.fOvertones = 0;
	m_Voice.FXMorphParams.fOvertoneRamp = m_Voice.FXBaseParams.fOvertoneRamp = 0;

	//////////////////////////////////////////////////////////////////////////

	m_Voice.fMorphRate = 0;
}

//////////////////////////////////////////////////////////////////////////

int RetroSFXVoice::ReadData(void *pDest, int nSize, int nUnits, unsigned char *&pData) {
	int read_size = nSize * nUnits;
	memcpy(pDest, pData, nSize * nUnits);
	pData += read_size;

	return read_size;
}

//////////////////////////////////////////////////////////////////////////

bool RetroSFXVoice::LoadSettings(unsigned char *pData) {
	int version = 0;
	ReadData(&version, 1, sizeof(int), pData);
	if (version != SFXR0100 && version != SFXR0140) {
		WARN_PRINT("Cannot load - unknown version tag.");
		return false;
	}

	ReadData(&m_Voice.nWaveformType, 1, sizeof(int), pData);

	ReadData(&m_Voice.fSoundVol, 1, sizeof(float), pData);
	ReadData(&m_Voice.fMorphRate, 1, sizeof(float), pData);
	ReadData(&m_Voice.nLengthInSamples, 1, sizeof(float), pData);

	//////////////////////////////////////////////////////////////////////////

	ReadData(&m_Voice.FXBaseParams.fBaseFreq, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXBaseParams.fFreqLimit, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXBaseParams.fFreqRamp, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXBaseParams.fFreqDRamp, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXBaseParams.fDuty, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXBaseParams.fDutyRamp, 1, sizeof(float), pData);

	ReadData(&m_Voice.FXBaseParams.fVibStrength, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXBaseParams.fVibSpeed, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXBaseParams.fVibDelay, 1, sizeof(float), pData);

	ReadData(&m_Voice.FXBaseParams.fEnvAttack, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXBaseParams.fEnvSustain, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXBaseParams.fEnvDecay, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXBaseParams.fEnvPunch, 1, sizeof(float), pData);

	ReadData(&m_Voice.FXBaseParams.fLPFResonance, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXBaseParams.fLPFFreq, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXBaseParams.fLPFRamp, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXBaseParams.fHPFFreq, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXBaseParams.fHPFRamp, 1, sizeof(float), pData);

	ReadData(&m_Voice.FXBaseParams.fFlangerOffset, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXBaseParams.fFlangerRamp, 1, sizeof(float), pData);

	ReadData(&m_Voice.FXBaseParams.fRepeatSpeed, 1, sizeof(float), pData);

	ReadData(&m_Voice.FXBaseParams.fArmSpeed, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXBaseParams.fArmMod, 1, sizeof(float), pData);

	//////////////////////////////////////////////////////////////////////////

	ReadData(&m_Voice.FXMorphParams.fBaseFreq, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXMorphParams.fFreqLimit, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXMorphParams.fFreqRamp, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXMorphParams.fFreqDRamp, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXMorphParams.fDuty, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXMorphParams.fDutyRamp, 1, sizeof(float), pData);

	ReadData(&m_Voice.FXMorphParams.fVibStrength, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXMorphParams.fVibSpeed, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXMorphParams.fVibDelay, 1, sizeof(float), pData);

	ReadData(&m_Voice.FXMorphParams.fEnvAttack, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXMorphParams.fEnvSustain, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXMorphParams.fEnvDecay, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXMorphParams.fEnvPunch, 1, sizeof(float), pData);

	ReadData(&m_Voice.FXMorphParams.fLPFResonance, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXMorphParams.fLPFFreq, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXMorphParams.fLPFRamp, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXMorphParams.fHPFFreq, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXMorphParams.fHPFRamp, 1, sizeof(float), pData);

	ReadData(&m_Voice.FXMorphParams.fFlangerOffset, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXMorphParams.fFlangerRamp, 1, sizeof(float), pData);

	ReadData(&m_Voice.FXMorphParams.fRepeatSpeed, 1, sizeof(float), pData);

	ReadData(&m_Voice.FXMorphParams.fArmSpeed, 1, sizeof(float), pData);
	ReadData(&m_Voice.FXMorphParams.fArmMod, 1, sizeof(float), pData);

	if (version == SFXR0140) {
		ReadData(&m_Voice.FXBaseParams.fOvertones, 1, sizeof(int), pData);
		ReadData(&m_Voice.FXBaseParams.fOvertoneRamp, 1, sizeof(float), pData);

		ReadData(&m_Voice.FXBaseParams.fBitCrush, 1, sizeof(float), pData);
		ReadData(&m_Voice.FXBaseParams.fBitCrushSweep, 1, sizeof(float), pData);
		ReadData(&m_Voice.FXBaseParams.fCompressionAmount, 1, sizeof(float), pData);

		ReadData(&m_Voice.FXBaseParams.fArmRepeat, 1, sizeof(float), pData);
		ReadData(&m_Voice.FXBaseParams.fArmSpeed2, 1, sizeof(float), pData);
		ReadData(&m_Voice.FXBaseParams.fArmMod2, 1, sizeof(float), pData);

		ReadData(&m_Voice.FXMorphParams.fOvertones, 1, sizeof(int), pData);
		ReadData(&m_Voice.FXMorphParams.fOvertoneRamp, 1, sizeof(float), pData);

		ReadData(&m_Voice.FXMorphParams.fBitCrush, 1, sizeof(float), pData);
		ReadData(&m_Voice.FXMorphParams.fBitCrushSweep, 1, sizeof(float), pData);
		ReadData(&m_Voice.FXMorphParams.fCompressionAmount, 1, sizeof(float), pData);

		ReadData(&m_Voice.FXMorphParams.fArmRepeat, 1, sizeof(float), pData);
		ReadData(&m_Voice.FXMorphParams.fArmSpeed2, 1, sizeof(float), pData);
		ReadData(&m_Voice.FXMorphParams.fArmMod2, 1, sizeof(float), pData);
	} else {
		m_Voice.FXBaseParams.fOvertones = m_Voice.FXMorphParams.fOvertones = 0;
		m_Voice.FXBaseParams.fOvertoneRamp = m_Voice.FXMorphParams.fOvertoneRamp = 0;

		m_Voice.FXBaseParams.fBitCrush = m_Voice.FXMorphParams.fBitCrush = 0;
		m_Voice.FXBaseParams.fBitCrushSweep = m_Voice.FXMorphParams.fBitCrushSweep = 0;
		m_Voice.FXBaseParams.fCompressionAmount = m_Voice.FXMorphParams.fCompressionAmount = 0;

		m_Voice.FXBaseParams.fArmRepeat = m_Voice.FXMorphParams.fArmRepeat = 0;
		m_Voice.FXBaseParams.fArmSpeed2 = m_Voice.FXMorphParams.fArmSpeed2 = 0;
		m_Voice.FXBaseParams.fArmMod2 = m_Voice.FXMorphParams.fArmMod2 = 0;
	}

	//////////////////////////////////////////////////////////////////////////

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool RetroSFXVoice::LoadSettings(const char *pFilename) {
	if (G_FILE *fp = g_open_read(pFilename)) {
		const int file_size = g_filesize(fp);
		std::unique_ptr<char[]> ptr(new char[file_size]);
		const int bytes_read = g_read(ptr.get(), file_size, 1, fp);
		g_close(fp);

		ERR_FAIL_COND_V_MSG(bytes_read != file_size, false, "Short read from file");

		LoadSettings(ptr.get());

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////

bool RetroSFXVoice::SaveSettings(const char *pFilename) {
	G_FILE *file = g_open_write(pFilename);
	if (!file) {
		return false;
	}

	int version = SFXR0140;
	g_write(&version, 1, sizeof(int), file);

	g_write(&m_Voice.nWaveformType, 1, sizeof(int), file);

	g_write(&m_Voice.fSoundVol, 1, sizeof(float), file);
	g_write(&m_Voice.fMorphRate, 1, sizeof(float), file);

	g_write(&m_Voice.nLengthInSamples, 1, sizeof(float), file);

	g_write(&m_Voice.FXBaseParams.fBaseFreq, 1, sizeof(float), file);
	g_write(&m_Voice.FXBaseParams.fFreqLimit, 1, sizeof(float), file);
	g_write(&m_Voice.FXBaseParams.fFreqRamp, 1, sizeof(float), file);
	g_write(&m_Voice.FXBaseParams.fFreqDRamp, 1, sizeof(float), file);
	g_write(&m_Voice.FXBaseParams.fDuty, 1, sizeof(float), file);
	g_write(&m_Voice.FXBaseParams.fDutyRamp, 1, sizeof(float), file);

	g_write(&m_Voice.FXBaseParams.fVibStrength, 1, sizeof(float), file);
	g_write(&m_Voice.FXBaseParams.fVibSpeed, 1, sizeof(float), file);
	g_write(&m_Voice.FXBaseParams.fVibDelay, 1, sizeof(float), file);

	g_write(&m_Voice.FXBaseParams.fEnvAttack, 1, sizeof(float), file);
	g_write(&m_Voice.FXBaseParams.fEnvSustain, 1, sizeof(float), file);
	g_write(&m_Voice.FXBaseParams.fEnvDecay, 1, sizeof(float), file);
	g_write(&m_Voice.FXBaseParams.fEnvPunch, 1, sizeof(float), file);

	g_write(&m_Voice.FXBaseParams.fLPFResonance, 1, sizeof(float), file);
	g_write(&m_Voice.FXBaseParams.fLPFFreq, 1, sizeof(float), file);
	g_write(&m_Voice.FXBaseParams.fLPFRamp, 1, sizeof(float), file);
	g_write(&m_Voice.FXBaseParams.fHPFFreq, 1, sizeof(float), file);
	g_write(&m_Voice.FXBaseParams.fHPFRamp, 1, sizeof(float), file);

	g_write(&m_Voice.FXBaseParams.fFlangerOffset, 1, sizeof(float), file);
	g_write(&m_Voice.FXBaseParams.fFlangerRamp, 1, sizeof(float), file);

	g_write(&m_Voice.FXBaseParams.fRepeatSpeed, 1, sizeof(float), file);

	g_write(&m_Voice.FXBaseParams.fArmSpeed, 1, sizeof(float), file);
	g_write(&m_Voice.FXBaseParams.fArmMod, 1, sizeof(float), file);

	//////////////////////////////////////////////////////////////////////////

	g_write(&m_Voice.FXMorphParams.fBaseFreq, 1, sizeof(float), file);
	g_write(&m_Voice.FXMorphParams.fFreqLimit, 1, sizeof(float), file);
	g_write(&m_Voice.FXMorphParams.fFreqRamp, 1, sizeof(float), file);
	g_write(&m_Voice.FXMorphParams.fFreqDRamp, 1, sizeof(float), file);
	g_write(&m_Voice.FXMorphParams.fDuty, 1, sizeof(float), file);
	g_write(&m_Voice.FXMorphParams.fDutyRamp, 1, sizeof(float), file);

	g_write(&m_Voice.FXMorphParams.fVibStrength, 1, sizeof(float), file);
	g_write(&m_Voice.FXMorphParams.fVibSpeed, 1, sizeof(float), file);
	g_write(&m_Voice.FXMorphParams.fVibDelay, 1, sizeof(float), file);

	g_write(&m_Voice.FXMorphParams.fEnvAttack, 1, sizeof(float), file);
	g_write(&m_Voice.FXMorphParams.fEnvSustain, 1, sizeof(float), file);
	g_write(&m_Voice.FXMorphParams.fEnvDecay, 1, sizeof(float), file);
	g_write(&m_Voice.FXMorphParams.fEnvPunch, 1, sizeof(float), file);

	g_write(&m_Voice.FXMorphParams.fLPFResonance, 1, sizeof(float), file);
	g_write(&m_Voice.FXMorphParams.fLPFFreq, 1, sizeof(float), file);
	g_write(&m_Voice.FXMorphParams.fLPFRamp, 1, sizeof(float), file);
	g_write(&m_Voice.FXMorphParams.fHPFFreq, 1, sizeof(float), file);
	g_write(&m_Voice.FXMorphParams.fHPFRamp, 1, sizeof(float), file);

	g_write(&m_Voice.FXMorphParams.fFlangerOffset, 1, sizeof(float), file);
	g_write(&m_Voice.FXMorphParams.fFlangerRamp, 1, sizeof(float), file);

	g_write(&m_Voice.FXMorphParams.fRepeatSpeed, 1, sizeof(float), file);

	g_write(&m_Voice.FXMorphParams.fArmSpeed, 1, sizeof(float), file);
	g_write(&m_Voice.FXMorphParams.fArmMod, 1, sizeof(float), file);

	//////////////////////////////////////////////////////////////////////////

	g_write(&m_Voice.FXBaseParams.fOvertones, 1, sizeof(int), file);
	g_write(&m_Voice.FXBaseParams.fOvertoneRamp, 1, sizeof(float), file);

	g_write(&m_Voice.FXBaseParams.fBitCrush, 1, sizeof(float), file);
	g_write(&m_Voice.FXBaseParams.fBitCrushSweep, 1, sizeof(float), file);
	g_write(&m_Voice.FXBaseParams.fCompressionAmount, 1, sizeof(float), file);

	g_write(&m_Voice.FXBaseParams.fArmRepeat, 1, sizeof(float), file);
	g_write(&m_Voice.FXBaseParams.fArmSpeed2, 1, sizeof(float), file);
	g_write(&m_Voice.FXBaseParams.fArmMod2, 1, sizeof(float), file);

	g_write(&m_Voice.FXMorphParams.fOvertones, 1, sizeof(int), file);
	g_write(&m_Voice.FXMorphParams.fOvertoneRamp, 1, sizeof(float), file);

	g_write(&m_Voice.FXMorphParams.fBitCrush, 1, sizeof(float), file);
	g_write(&m_Voice.FXMorphParams.fBitCrushSweep, 1, sizeof(float), file);
	g_write(&m_Voice.FXMorphParams.fCompressionAmount, 1, sizeof(float), file);

	g_write(&m_Voice.FXMorphParams.fArmRepeat, 1, sizeof(float), file);
	g_write(&m_Voice.FXMorphParams.fArmSpeed2, 1, sizeof(float), file);
	g_write(&m_Voice.FXMorphParams.fArmMod2, 1, sizeof(float), file);

	//////////////////////////////////////////////////////////////////////////

	g_close(file);
	return true;
}

//////////////////////////////////////////////////////////////////////////

bool RetroSFXVoice::CompareSettings(RetroSFXVoice *pOther) {
	if (memcmp(&this->m_Voice, &pOther->m_Voice, sizeof(this->m_Voice)) == 0) {
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////

void RetroSFXVoice::Reset(bool restart) {
	if (!restart) {
		phase = 0;

		m_FXWorkParams.fBaseFreq = m_Voice.FXBaseParams.fBaseFreq;
		m_FXWorkParams.fFreqLimit = m_Voice.FXBaseParams.fFreqLimit;
		m_FXWorkParams.fFreqRamp = m_Voice.FXBaseParams.fFreqRamp;
		m_FXWorkParams.fFreqDRamp = m_Voice.FXBaseParams.fFreqDRamp;
		m_FXWorkParams.fDuty = m_Voice.FXBaseParams.fDuty;
		m_FXWorkParams.fDutyRamp = m_Voice.FXBaseParams.fDutyRamp;

		m_FXWorkParams.fVibStrength = m_Voice.FXBaseParams.fVibStrength;
		m_FXWorkParams.fVibSpeed = m_Voice.FXBaseParams.fVibSpeed;
		m_FXWorkParams.fVibDelay = m_Voice.FXBaseParams.fVibDelay;

		m_FXWorkParams.fEnvAttack = m_Voice.FXBaseParams.fEnvAttack;
		m_FXWorkParams.fEnvSustain = m_Voice.FXBaseParams.fEnvSustain;
		m_FXWorkParams.fEnvDecay = m_Voice.FXBaseParams.fEnvDecay;
		m_FXWorkParams.fEnvPunch = m_Voice.FXBaseParams.fEnvPunch;

		m_FXWorkParams.fLPFResonance = m_Voice.FXBaseParams.fLPFResonance;
		m_FXWorkParams.fLPFFreq = m_Voice.FXBaseParams.fLPFFreq;
		m_FXWorkParams.fLPFRamp = m_Voice.FXBaseParams.fLPFRamp;
		m_FXWorkParams.fHPFFreq = m_Voice.FXBaseParams.fHPFFreq;
		m_FXWorkParams.fHPFRamp = m_Voice.FXBaseParams.fHPFRamp;

		m_FXWorkParams.fFlangerOffset = m_Voice.FXBaseParams.fFlangerOffset;
		m_FXWorkParams.fFlangerRamp = m_Voice.FXBaseParams.fFlangerRamp;

		m_FXWorkParams.fRepeatSpeed = m_Voice.FXBaseParams.fRepeatSpeed;

		m_FXWorkParams.fArmSpeed = m_Voice.FXBaseParams.fArmSpeed;
		m_FXWorkParams.fArmMod = m_Voice.FXBaseParams.fArmMod;

		m_FXWorkParams.fOvertones = m_Voice.FXMorphParams.fOvertones;
		m_FXWorkParams.fOvertoneRamp = m_Voice.FXMorphParams.fOvertoneRamp;

		m_FXWorkParams.fBitCrush = m_Voice.FXMorphParams.fBitCrush;
		m_FXWorkParams.fBitCrushSweep = m_Voice.FXMorphParams.fBitCrushSweep;
		m_FXWorkParams.fCompressionAmount = m_Voice.FXMorphParams.fCompressionAmount;

		m_FXWorkParams.fArmRepeat = m_Voice.FXMorphParams.fArmRepeat;
		m_FXWorkParams.fArmSpeed2 = m_Voice.FXMorphParams.fArmSpeed2;
		m_FXWorkParams.fArmMod2 = m_Voice.FXMorphParams.fArmMod2;
	}

	fperiod = 100.0 / (m_FXWorkParams.fBaseFreq * m_FXWorkParams.fBaseFreq + 0.001);
	period = fperiod;
	fmaxperiod = 100.0 / (m_FXWorkParams.fFreqLimit * m_FXWorkParams.fFreqLimit + 0.001);
	fslide = 1 - Math::pow(m_FXWorkParams.fFreqRamp, 3) * 0.01;
	fdslide = -Math::pow(m_FXWorkParams.fFreqDRamp, 3) * 0.000001;
	square_duty = 0.5 - m_FXWorkParams.fDuty * 0.5;
	square_slide = -m_FXWorkParams.fDutyRamp * 0.00005;
	if (m_FXWorkParams.fArmMod >= 0) {
		arm_mod = 1.0 - Math::pow(m_FXWorkParams.fArmMod, 2) * 0.9;
	} else {
		arm_mod = 1.0 + Math::pow(m_FXWorkParams.fArmMod, 2) * 10.0;
	}
	arm_time = 0;
	arm_limit = (int)(Math::pow(1 - m_FXWorkParams.fArmSpeed, 2) * 20000 + 32);
	if (m_FXWorkParams.fArmSpeed == 1) {
		arm_limit = 0;
	}

	if (!restart) {
		// reset filter
		fltp = 0;
		fltdp = 0;
		fltw = Math::pow(m_FXWorkParams.fLPFFreq, 3) * 0.1;
		fltw_d = 1 + m_FXWorkParams.fLPFRamp * 0.0001;
		fltdmp = 5 / (1 + Math::pow(m_FXWorkParams.fLPFResonance, 2) * 20) * (0.01 + fltw);
		if (fltdmp > 0.8) {
			fltdmp = 0.8;
		}
		fltphp = 0;
		flthp = Math::pow(m_FXWorkParams.fHPFFreq, 2) * 0.1;
		flthm_d = 1 + m_FXWorkParams.fHPFRamp * 0.0003;
		// reset vibrato
		vib_phase = 0;
		vib_speed = Math::pow(m_Voice.FXBaseParams.fVibSpeed, 2) * 0.01;
		vib_amp = m_FXWorkParams.fVibStrength * 0.5;
		// reset envelope
		env_vol = 0;
		env_stage = 0;
		env_time = 0;
		env_length[0] = m_FXWorkParams.fEnvAttack * m_FXWorkParams.fEnvAttack * 100000;
		env_length[1] = m_FXWorkParams.fEnvSustain * m_FXWorkParams.fEnvSustain * 100000;
		env_length[2] = m_FXWorkParams.fEnvDecay * m_FXWorkParams.fEnvDecay * 100000;

		fphase = Math::pow(m_FXWorkParams.fFlangerOffset, 2) * 1020;
		if (m_FXWorkParams.fFlangerOffset < 0) {
			fphase = -fphase;
		}
		fdphase = Math::pow(m_FXWorkParams.fFlangerRamp, 2) * 1;
		if (m_FXWorkParams.fFlangerRamp < 0) {
			fdphase = -fdphase;
		}
		iphase = Math::abs(fphase);
		ipp = 0;
		for (int i = 0; i < 1024; i++) {
			phaser_buffer[i] = 0;
		}
		for (int i = 0; i < 32; i++) {
			noise_buffer[i] = GenNoise();
			pink_noise_buffer[i] = GenPinkNoise();
		}
		rem_time = 0;
		rem_limit = Math::pow(1 - m_FXWorkParams.fRepeatSpeed, 2) * 20000 + 32;
		if (m_FXWorkParams.fRepeatSpeed == 0) {
			rem_limit = 0;
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void RetroSFXVoice::Play(void *pData) {
	if (pData) {
		RetroVoice103 *pVoice103 = (RetroVoice103 *)pData;
		if (pVoice103->nVersion == SFXR0100) {
			memcpy(&m_Voice, pData, sizeof(RetroVoice103));
		}
	}
	m_WavSamplesRendered = 0;
	Reset(false);
	m_bPlayingSample = true;
}

//////////////////////////////////////////////////////////////////////////

void RetroSFXVoice::Play(bool bCalculateLength) {
	if (bCalculateLength) {
		m_Voice.nLengthInSamples = 0;
	}
	m_WavSamplesRendered = 0;
	Reset(false);
	m_bPlayingSample = true;
}

//////////////////////////////////////////////////////////////////////////

int RetroSFXVoice::GetVoiceLengthInSamples() const {
	return m_Voice.nLengthInSamples;
}

//////////////////////////////////////////////////////////////////////////

int RetroSFXVoice::Render(int nSamples, BufferCallback *pCallback) {
	int nSamplesRendered = 0;

	for (int i = 0; i < nSamples; i++) {
		if (!m_bPlayingSample) {
			break;
		}
		rem_time++;
		if (rem_limit != 0 && rem_time >= rem_limit) {
			rem_time = 0;
			Reset(true);

			if (m_Voice.fMorphRate != 0) {
				Morph(m_FXWorkParams.fBaseFreq, m_Voice.FXMorphParams.fBaseFreq);
				Morph(m_FXWorkParams.fFreqLimit, m_Voice.FXMorphParams.fFreqLimit);
				Morph(m_FXWorkParams.fFreqRamp, m_Voice.FXMorphParams.fFreqRamp);
				Morph(m_FXWorkParams.fFreqDRamp, m_Voice.FXMorphParams.fFreqDRamp);
				Morph(m_FXWorkParams.fDuty, m_Voice.FXMorphParams.fDuty);
				Morph(m_FXWorkParams.fDutyRamp, m_Voice.FXMorphParams.fDutyRamp);

				Morph(m_FXWorkParams.fVibStrength, m_Voice.FXMorphParams.fVibStrength);
				Morph(m_FXWorkParams.fVibSpeed, m_Voice.FXMorphParams.fVibSpeed);
				Morph(m_FXWorkParams.fVibDelay, m_Voice.FXMorphParams.fVibDelay);

				Morph(m_FXWorkParams.fEnvAttack, m_Voice.FXMorphParams.fEnvAttack);
				Morph(m_FXWorkParams.fEnvSustain, m_Voice.FXMorphParams.fEnvSustain);
				Morph(m_FXWorkParams.fEnvDecay, m_Voice.FXMorphParams.fEnvDecay);
				Morph(m_FXWorkParams.fEnvPunch, m_Voice.FXMorphParams.fEnvPunch);

				Morph(m_FXWorkParams.fLPFResonance, m_Voice.FXMorphParams.fLPFResonance);
				Morph(m_FXWorkParams.fLPFFreq, m_Voice.FXMorphParams.fLPFFreq);
				Morph(m_FXWorkParams.fLPFRamp, m_Voice.FXMorphParams.fLPFRamp);
				Morph(m_FXWorkParams.fHPFFreq, m_Voice.FXMorphParams.fHPFFreq);
				Morph(m_FXWorkParams.fHPFRamp, m_Voice.FXMorphParams.fHPFRamp);

				Morph(m_FXWorkParams.fFlangerOffset, m_Voice.FXMorphParams.fFlangerOffset);
				Morph(m_FXWorkParams.fFlangerRamp, m_Voice.FXMorphParams.fFlangerRamp);

				Morph(m_FXWorkParams.fRepeatSpeed, m_Voice.FXMorphParams.fRepeatSpeed);

				Morph(m_FXWorkParams.fArmSpeed, m_Voice.FXMorphParams.fArmSpeed);
				Morph(m_FXWorkParams.fArmMod, m_Voice.FXMorphParams.fArmMod);

				// Morph(m_FXWorkParams.fOvertones, m_Voice.FXMorphParams.fOvertones); // int ?
				Morph(m_FXWorkParams.fOvertoneRamp, m_Voice.FXMorphParams.fOvertoneRamp);

				Morph(m_FXWorkParams.fBitCrush, m_Voice.FXMorphParams.fBitCrush);
				Morph(m_FXWorkParams.fBitCrushSweep, m_Voice.FXMorphParams.fBitCrushSweep);
				Morph(m_FXWorkParams.fCompressionAmount, m_Voice.FXMorphParams.fCompressionAmount);

				Morph(m_FXWorkParams.fArmRepeat, m_Voice.FXMorphParams.fArmRepeat);
				Morph(m_FXWorkParams.fArmSpeed2, m_Voice.FXMorphParams.fArmSpeed2);
				Morph(m_FXWorkParams.fArmMod2, m_Voice.FXMorphParams.fArmMod2);
			}
		}

		// frequency envelopes/arpeggios
		arm_time++;
		if (arm_limit != 0 && arm_time >= arm_limit) {
			arm_time = 0;
			fperiod *= arm_mod;
		}
		fslide += fdslide;
		fperiod *= fslide;
		if (fperiod > fmaxperiod) {
			fperiod = fmaxperiod;
			if (m_FXWorkParams.fFreqLimit > 0) {
				m_bPlayingSample = false;
			}
		}
		float rfperiod = fperiod;
		if (vib_amp > 0) {
			vib_phase += vib_speed;
			rfperiod = fperiod * (1 + Math::sin(vib_phase) * vib_amp);
		}
		period = rfperiod;
		if (period < 8)
			period = 8;

		square_duty += square_slide;
		if (square_duty < 0)
			square_duty = 0;
		if (square_duty > 0.5)
			square_duty = 0.5;

		// volume envelope
		env_time++;
		if (env_time > env_length[env_stage]) {
			env_time = 0;
			env_stage++;
			if (env_stage == 3) {
				m_bPlayingSample = false;
			}
		}
		if (env_stage == 0) {
			env_vol = float(env_time) / env_length[0];
		}
		if (env_stage == 1) {
			env_vol = 1 + Math::pow(1 - float(env_time) / env_length[1], 1) * 2 * m_FXWorkParams.fEnvPunch;
		}
		if (env_stage == 2) {
			env_vol = 1 - float(env_time) / env_length[2];
		}

		// phaser step
		fphase += fdphase;
		iphase = Math::abs(fphase);
		if (iphase > 1023) {
			iphase = 1023;
		}
		if (flthm_d != 0) {
			flthp *= flthm_d;
			if (flthp < 0.00001)
				flthp = 0.00001;
			if (flthp > 0.1)
				flthp = 0.1;
		}

		float ssample = 0;
		for (int si = 0; si < 8; si++) { // 8x oversampling
			phase++;
			if (phase >= period) {
				// phase = 0;
				phase %= period;
				if (m_Voice.nWaveformType == 3) {
					for (int n = 0; n < 32; n++) {
						noise_buffer[n] = GenNoise();
					}
				} else if (m_Voice.nWaveformType == 5) {
					for (int n = 0; n < 32; n++) {
						pink_noise_buffer[n] = GenPinkNoise();
					}
				}
			}
			// base waveform
			float fp = (float)phase / period;
			float ots = 1; // overtonestrength
			float sample = 0;
			for (int k = 0; k <= m_Voice.FXBaseParams.fOvertones; k++) {
				float _sample = 0;
				switch (m_Voice.nWaveformType) {
					case 0: { // square
						_sample = (fp < square_duty) ? 0.5 : -0.5;
					} break;
					case 1: { // sawtooth
						_sample = 1 - fp * 2;
					} break;
					case 2: { // sine
						_sample = Math::sin(fp * 2 * PI);
					} break;
					case 3: { // noise
						_sample = noise_buffer[phase * 32 / period];
					} break;
					case 4: { // triangle
						_sample = Math::abs(1 - fp * 2) - 1;
					} break;
					case 5: { // pink noise
						_sample = pink_noise_buffer[(phase * 32 / period) % 32];
					} break;
					case 6: { // tan
						_sample = Math::tan(PI * fp);
					} break;
					case 7: { // whistle
						// sine wave code
						float pos = fp;
						pos = pos > 0.5 ? (pos - 1.0) * 6.28318531 : pos * 6.28318531;
						sample = pos < 0 ? 1.27323954 * pos + 0.405284735 * pos * pos : 1.27323954 * pos - 0.405284735 * pos * pos;
						sample = 0.75 * (sample < 0 ? 0.225 * (sample * -sample - sample) + sample : 0.225 * (sample * sample - sample) + sample);
						// then whistle (essentially an overtone with frequency x20 and amplitude 0.25
						pos = ((phase * 20) % period) / period;
						pos = pos > 0.5 ? (pos - 1.0) * 6.28318531 : pos * 6.28318531;
						const float sample2 = pos < 0 ? 1.27323954 * pos + 0.405284735 * pos * pos : 1.27323954 * pos - 0.405284735 * pos * pos;
						_sample = 0.25 * (sample2 < 0 ? 0.225 * (sample2 * -sample2 - sample2) + sample2 : 0.225 * (sample2 * sample2 - sample2) + sample2);
					} break;
					case 8: { // breaker
						_sample = Math::abs(1 - fp * fp * 2) - 1;
					} break;
				}
				sample += _sample * ots;
				ots *= (1 - m_Voice.FXBaseParams.fOvertoneRamp);
			}
			// lp filter
			float pp = fltp;
			fltw *= fltw_d;
			if (fltw < 0) {
				fltw = 0;
			}
			if (fltw > 0.1) {
				fltw = 0.1;
			}
			if (m_FXWorkParams.fLPFFreq != 1) {
				fltdp += (sample - fltp) * fltw;
				fltdp -= fltdp * fltdmp;
			} else {
				fltp = sample;
				fltdp = 0;
			}
			fltp += fltdp;
			// hp filter
			fltphp += fltp - pp;
			fltphp -= fltphp * flthp;
			sample = fltphp;
			// phaser
			phaser_buffer[ipp & 1023] = sample;
			sample += phaser_buffer[(ipp - iphase + 1024) & 1023];
			ipp = (ipp + 1) & 1023;
			// final accumulation and envelope application
			ssample += sample * env_vol;
		}
		ssample = ssample / 8 * m_fMasterVol;

		ssample *= 2 * m_Voice.fSoundVol;

		if (pCallback != nullptr) {
			// clamp
			if (ssample > 1) {
				ssample = 1;
			}
			if (ssample < -1) {
				ssample = -1;
			}
			(*pCallback)(ssample);
			nSamplesRendered++;
		}
	}

	// Calculating length here
	m_WavSamplesRendered += nSamplesRendered;
	if (m_bPlayingSample == false) {
		m_Voice.nLengthInSamples = m_WavSamplesRendered;
	}

	return nSamplesRendered;
}

//////////////////////////////////////////////////////////////////////////

float RetroSFXVoice::GenNoise() const {
	const float range = 2;
	return ((float)rnd(10000) / 10000 * range) - 1;
}

float RetroSFXVoice::GenPinkNoise() const {
	static PinkNumber _pn;

	return _pn.getNextValue();
}

//////////////////////////////////////////////////////////////////////////

void RetroSFXVoice::Morph(float &fMorphVar, float fMorphDest) {
	const float diff = fMorphDest - fMorphVar;

	if (diff != 0) {
		fMorphVar += diff * m_Voice.fMorphRate;
	}
}

//////////////////////////////////////////////////////////////////////////

void RetroSFXVoice::Mutate() {
}

void RetroSFXVoice::Randomize() {
	m_Voice.nWaveformType = rnd(8);

	m_Voice.FXBaseParams.fBaseFreq = Math::pow(frnd(1), 2);
	if (rnd(1)) {
		m_Voice.FXBaseParams.fBaseFreq = Math::pow(frnd(1), 3) + 0.5;
	}
	m_Voice.FXBaseParams.fFreqLimit = 0;
	m_Voice.FXBaseParams.fFreqRamp = pow(frnd(1), 5);
	if (m_Voice.FXBaseParams.fBaseFreq > 0.7 && m_Voice.FXBaseParams.fFreqRamp > 0.2) {
		m_Voice.FXBaseParams.fFreqRamp = -m_Voice.FXBaseParams.fFreqRamp;
	}
	if (m_Voice.FXBaseParams.fBaseFreq < 0.2 && m_Voice.FXBaseParams.fFreqRamp < -0.05) {
		m_Voice.FXBaseParams.fFreqRamp = -m_Voice.FXBaseParams.fFreqRamp;
	}
	m_Voice.FXBaseParams.fFreqDRamp = Math::pow(frnd(1), 3);
	m_Voice.FXBaseParams.fDuty = frnd(1);
	m_Voice.FXBaseParams.fDutyRamp = Math::pow(frnd(1), 3);
	m_Voice.FXBaseParams.fVibStrength = Math::pow(frnd(1), 3);
	m_Voice.FXBaseParams.fVibSpeed = frnd(1);
	m_Voice.FXBaseParams.fVibDelay = frnd(1);
	m_Voice.FXBaseParams.fEnvAttack = Math::pow(frnd(1), 3);
	m_Voice.FXBaseParams.fEnvSustain = Math::pow(frnd(1), 2);
	m_Voice.FXBaseParams.fEnvDecay = ((frnd(1)) / 2) + 0.5;
	m_Voice.FXBaseParams.fEnvPunch = Math::pow(frnd(0.8), 2);
	if (m_Voice.FXBaseParams.fEnvAttack + m_Voice.FXBaseParams.fEnvSustain + m_Voice.FXBaseParams.fEnvDecay < 0.2) {
		m_Voice.FXBaseParams.fEnvSustain += 0.2 + frnd(0.3);
		m_Voice.FXBaseParams.fEnvDecay += 0.2 + frnd(0.3);
	}
	m_Voice.FXBaseParams.fLPFResonance = frnd(1);
	m_Voice.FXBaseParams.fLPFFreq = 1.0f - Math::pow(frnd(1.0f), 3);
	m_Voice.FXBaseParams.fLPFRamp = Math::pow(frnd(1.0f), 3.0f);
	if (m_Voice.FXBaseParams.fLPFFreq < 0.1f && m_Voice.FXBaseParams.fLPFRamp < -0.05) {
		m_Voice.FXBaseParams.fLPFRamp = -m_Voice.FXBaseParams.fLPFRamp;
	}
	m_Voice.FXBaseParams.fHPFFreq = Math::pow(frnd(1), 5);
	m_Voice.FXBaseParams.fHPFRamp = Math::pow(frnd(1), 5);
	m_Voice.FXBaseParams.fFlangerOffset = Math::pow(frnd(1), 3);
	m_Voice.FXBaseParams.fFlangerRamp = Math::pow(frnd(1), 3);
	m_Voice.FXBaseParams.fRepeatSpeed = frnd(1);
	m_Voice.FXBaseParams.fArmSpeed = frnd(1);
	m_Voice.FXBaseParams.fArmMod = frnd(1);

	//////////////////////////////////////////////////////////////////////////
	// for morph

	if (m_Voice.FXBaseParams.fRepeatSpeed == 0) {
		// If we don't have a repeat then we don't need a morph
		m_Voice.fMorphRate = 0;
	} else {
		m_Voice.fMorphRate = frnd(2);
		m_Voice.fMorphRate -= 1;
		if (m_Voice.fMorphRate < 0) {
			m_Voice.fMorphRate = 0; // 50% chance of morph
		}
	}

	m_Voice.FXMorphParams.fBaseFreq = Math::pow(frnd(1), 2);
	if (rnd(1)) {
		m_Voice.FXMorphParams.fBaseFreq = Math::pow(frnd(1), 3) + 0.5;
	}
	m_Voice.FXMorphParams.fFreqLimit = 0;
	m_Voice.FXMorphParams.fFreqRamp = Math::pow(frnd(1), 5);
	if (m_Voice.FXMorphParams.fBaseFreq > 0.7 && m_Voice.FXMorphParams.fFreqRamp > 0.2) {
		m_Voice.FXMorphParams.fFreqRamp = -m_Voice.FXMorphParams.fFreqRamp;
	}
	if (m_Voice.FXMorphParams.fBaseFreq < 0.2 && m_Voice.FXMorphParams.fFreqRamp < -0.05) {
		m_Voice.FXMorphParams.fFreqRamp = -m_Voice.FXMorphParams.fFreqRamp;
	}
	m_Voice.FXMorphParams.fFreqDRamp = Math::pow(frnd(1), 3);
	m_Voice.FXMorphParams.fDuty = frnd(1);
	m_Voice.FXMorphParams.fDutyRamp = Math::pow(frnd(1), 3);
	m_Voice.FXMorphParams.fVibStrength = Math::pow(frnd(1), 3);
	m_Voice.FXMorphParams.fVibSpeed = frnd(1);
	m_Voice.FXMorphParams.fVibDelay = frnd(1);
	m_Voice.FXMorphParams.fEnvAttack = Math::pow(frnd(1), 3);
	m_Voice.FXMorphParams.fEnvSustain = Math::pow(frnd(1), 2);
	m_Voice.FXMorphParams.fEnvDecay = frnd(1);
	m_Voice.FXMorphParams.fEnvPunch = Math::pow(frnd(0.8), 2);
	if (m_Voice.FXMorphParams.fEnvAttack + m_Voice.FXMorphParams.fEnvSustain + m_Voice.FXMorphParams.fEnvDecay < 0.2) {
		m_Voice.FXMorphParams.fEnvSustain += 0.2 + frnd(0.3);
		m_Voice.FXMorphParams.fEnvDecay += 0.2 + frnd(0.3);
	}
	m_Voice.FXMorphParams.fLPFResonance = frnd(1);
	m_Voice.FXMorphParams.fLPFFreq = 1 - Math::pow(frnd(1), 3);
	m_Voice.FXMorphParams.fLPFRamp = Math::pow(frnd(1), 3);
	if (m_Voice.FXMorphParams.fLPFFreq < 0.1 && m_Voice.FXMorphParams.fLPFRamp < -0.05) {
		m_Voice.FXMorphParams.fLPFRamp = -m_Voice.FXMorphParams.fLPFRamp;
	}
	m_Voice.FXMorphParams.fHPFFreq = Math::pow(frnd(1), 5);
	m_Voice.FXMorphParams.fHPFRamp = Math::pow(frnd(1), 5);
	m_Voice.FXMorphParams.fFlangerOffset = Math::pow(frnd(1), 3);
	m_Voice.FXMorphParams.fFlangerRamp = Math::pow(frnd(1), 3);
	m_Voice.FXMorphParams.fRepeatSpeed = frnd(1);
	m_Voice.FXMorphParams.fArmSpeed = frnd(1);
	m_Voice.FXMorphParams.fArmMod = frnd(1);
}

//////////////////////////////////////////////////////////////////////////

bool RetroSFXVoice::ExportWav(const char *pFilename, int pWavBits, int pWavFreq) {
	auto write_little_endian = [](unsigned word, int num_bytes, G_FILE *wav_file) {
		unsigned buf;
		while (num_bytes > 0) {
			buf = word & 0xff;
			g_write(&buf, 1, 1, wav_file);
			num_bytes--;
			word >>= 8;
		}
	};

	struct SampleBuffer : public BufferCallback {
		int wav_bits, wav_freq;
		float file_sample = 0;
		int file_acc_key = 0, file_acc = 0;
		std::vector<uint8_t> data;
		virtual void append_sample(float ssample) {
			// quantize depending on format
			ssample *= 4; // arbitrary gain to get reasonable output volume...
			if (ssample > 1) {
				ssample = 1;
			}
			if (ssample < -1) {
				ssample = -1;
			}
			file_sample += ssample;
			file_acc++;
			if (wav_freq == 44100 || file_acc == file_acc_key) {
				file_sample /= file_acc;
				file_acc = 0;
				if (wav_bits == 16) {
					const short isample = file_sample * 32767;
					data.push_back(isample & 0xff);
					data.push_back((isample >> 8) & 0xff);
				} else if (wav_bits == 8) {
					const unsigned char isample = file_sample * 127 + 128;
					data.push_back(isample);
				}
				file_sample = 0;
			}
		}
		const uint8_t *ptr() const { return &data[0]; }
		size_t size() const { return data.size(); }
		SampleBuffer(int wav_bits, int wav_freq) :
				wav_bits(wav_bits), wav_freq(wav_freq) {
			data.reserve(1024);
			switch (wav_freq) {
				case 44100: {
					file_acc_key = 1;
				} break;
				case 22050: {
					file_acc_key = 2;
				} break;
				case 11025: {
					file_acc_key = 4;
				} break;
			}
		}
	};

	SampleBuffer buffer(pWavBits, pWavFreq);
	Play();
	while (IsActive()) {
		Render(256, &buffer);
	}

	const int num_channels = 1; // monoaural
	const int num_samples = GetVoiceLengthInSamples();
	const int bytes_per_sample = pWavBits / 2;
	const int sample_rate = pWavFreq;
	const int byte_rate = sample_rate * num_channels * bytes_per_sample;

	G_FILE *wav_file = g_open_write(pFilename);
	if (wav_file == 0) {
		WARN_PRINT("Cannot open wav file to write.");
		return false;
	}

	// write RIFF header
	g_write("RIFF", 1, 4, wav_file);
	write_little_endian(36 + bytes_per_sample * num_samples * num_channels, 4, wav_file);
	g_write("WAVE", 1, 4, wav_file);

	g_write("fmt ", 1, 4, wav_file); // write fmt  subchunk
	write_little_endian(16, 4, wav_file); // SubChunk1Size is 16
	write_little_endian(1, 2, wav_file); // PCM is format 1
	write_little_endian(num_channels, 2, wav_file);
	write_little_endian(sample_rate, 4, wav_file);
	write_little_endian(byte_rate, 4, wav_file);
	write_little_endian(num_channels * bytes_per_sample, 2, wav_file); // block align
	write_little_endian(8 * bytes_per_sample, 2, wav_file); // bits/sample

	// write data subchunk
	g_write("data", 1, 4, wav_file);
	g_write(buffer.ptr(), 1, buffer.size(), wav_file);

	g_close(wav_file);
	return true;
}

//////////////////////////////////////////////////////////////////////////

bool RetroSFXVoice::IsActive() {
	return m_bPlayingSample;
}
