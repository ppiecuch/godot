/**************************************************************************/
/*  retrosfxvoice.cpp                                                     */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

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
	master_vol = 0.05;
	voice.sound_vol = 0.8;
	playing_sample = false;
	wav_samples_rendered = 0;

	ResetParams();
}

//////////////////////////////////////////////////////////////////////////

void RetroSFXVoice::ResetParams() {
	voice.waveform_shape = 0;

	voice.fx_morph_params.BaseFreq = voice.fx_base_params.BaseFreq = 0.3;
	voice.fx_morph_params.FreqLimit = voice.fx_base_params.FreqLimit = 0;
	voice.fx_morph_params.FreqRamp = voice.fx_base_params.FreqRamp = 0;
	voice.fx_morph_params.FreqDRamp = voice.fx_base_params.FreqDRamp = 0;
	voice.fx_morph_params.Duty = voice.fx_base_params.Duty = 0;
	voice.fx_morph_params.DutyRamp = voice.fx_base_params.DutyRamp = 0;

	voice.fx_morph_params.VibStrength = voice.fx_base_params.VibStrength = 0;
	voice.fx_morph_params.VibSpeed = voice.fx_base_params.VibSpeed = 0;
	voice.fx_morph_params.VibDelay = voice.fx_base_params.VibDelay = 0;

	voice.fx_morph_params.EnvAttack = voice.fx_base_params.EnvAttack = 0;
	voice.fx_morph_params.EnvSustain = voice.fx_base_params.EnvSustain = 0.3;
	voice.fx_morph_params.EnvDecay = voice.fx_base_params.EnvDecay = 0.4;
	voice.fx_morph_params.EnvPunch = voice.fx_base_params.EnvPunch = 0.0;

	voice.fx_morph_params.LPFResonance = voice.fx_base_params.LPFResonance = 0;
	voice.fx_morph_params.LPFFreq = voice.fx_base_params.LPFFreq = 1;
	voice.fx_morph_params.LPFRamp = voice.fx_base_params.LPFRamp = 0;
	voice.fx_morph_params.HPFFreq = voice.fx_base_params.HPFFreq = 0;
	voice.fx_morph_params.HPFRamp = voice.fx_base_params.HPFRamp = 0;
	voice.fx_morph_params.BitCrush = voice.fx_base_params.BitCrush = 0;
	voice.fx_morph_params.BitCrushSweep = voice.fx_base_params.BitCrushSweep = 0;
	voice.fx_morph_params.CompressionAmount = voice.fx_base_params.CompressionAmount = 0.3;

	voice.fx_morph_params.FlangerOffset = voice.fx_base_params.FlangerOffset = 0;
	voice.fx_morph_params.FlangerRamp = voice.fx_base_params.FlangerRamp = 0;

	voice.fx_morph_params.RepeatSpeed = voice.fx_base_params.RepeatSpeed = 0;

	voice.fx_morph_params.ArmRepeat = voice.fx_base_params.ArmRepeat = 0;
	voice.fx_morph_params.ArmSpeed = voice.fx_base_params.ArmSpeed = 1;
	voice.fx_morph_params.ArmMod = voice.fx_base_params.ArmMod = 0;
	voice.fx_morph_params.ArmSpeed2 = voice.fx_base_params.ArmSpeed2 = 1;
	voice.fx_morph_params.ArmMod2 = voice.fx_base_params.ArmMod2 = 0;

	voice.fx_morph_params.Overtones = voice.fx_base_params.Overtones = 0;
	voice.fx_morph_params.OvertoneRamp = voice.fx_base_params.OvertoneRamp = 0;

	//////////////////////////////////////////////////////////////////////////

	voice.morph_rate = 0;
}

//////////////////////////////////////////////////////////////////////////

int RetroSFXVoice::ReadData(void *dest, int size, int units, unsigned char *&data) {
	const int read_size = size * units;
	memcpy(dest, data, read_size);
	data += read_size;

	return read_size;
}

//////////////////////////////////////////////////////////////////////////

bool RetroSFXVoice::LoadSettings(unsigned char *data) {
	int version = 0;
	ReadData(&version, 1, sizeof(int), data);
	if (version != SFXR0100 && version != SFXR0140) {
		WARN_PRINT("Cannot load - unknown version tag.");
		return false;
	}

	ReadData(&voice.waveform_shape, 1, sizeof(int), data);

	ReadData(&voice.sound_vol, 1, sizeof(float), data);
	ReadData(&voice.morph_rate, 1, sizeof(float), data);
	ReadData(&voice.length_in_samples, 1, sizeof(float), data);

	//////////////////////////////////////////////////////////////////////////

	ReadData(&voice.fx_base_params.BaseFreq, 1, sizeof(float), data);
	ReadData(&voice.fx_base_params.FreqLimit, 1, sizeof(float), data);
	ReadData(&voice.fx_base_params.FreqRamp, 1, sizeof(float), data);
	ReadData(&voice.fx_base_params.FreqDRamp, 1, sizeof(float), data);
	ReadData(&voice.fx_base_params.Duty, 1, sizeof(float), data);
	ReadData(&voice.fx_base_params.DutyRamp, 1, sizeof(float), data);

	ReadData(&voice.fx_base_params.VibStrength, 1, sizeof(float), data);
	ReadData(&voice.fx_base_params.VibSpeed, 1, sizeof(float), data);
	ReadData(&voice.fx_base_params.VibDelay, 1, sizeof(float), data);

	ReadData(&voice.fx_base_params.EnvAttack, 1, sizeof(float), data);
	ReadData(&voice.fx_base_params.EnvSustain, 1, sizeof(float), data);
	ReadData(&voice.fx_base_params.EnvDecay, 1, sizeof(float), data);
	ReadData(&voice.fx_base_params.EnvPunch, 1, sizeof(float), data);

	ReadData(&voice.fx_base_params.LPFResonance, 1, sizeof(float), data);
	ReadData(&voice.fx_base_params.LPFFreq, 1, sizeof(float), data);
	ReadData(&voice.fx_base_params.LPFRamp, 1, sizeof(float), data);
	ReadData(&voice.fx_base_params.HPFFreq, 1, sizeof(float), data);
	ReadData(&voice.fx_base_params.HPFRamp, 1, sizeof(float), data);

	ReadData(&voice.fx_base_params.FlangerOffset, 1, sizeof(float), data);
	ReadData(&voice.fx_base_params.FlangerRamp, 1, sizeof(float), data);

	ReadData(&voice.fx_base_params.RepeatSpeed, 1, sizeof(float), data);

	ReadData(&voice.fx_base_params.ArmSpeed, 1, sizeof(float), data);
	ReadData(&voice.fx_base_params.ArmMod, 1, sizeof(float), data);

	//////////////////////////////////////////////////////////////////////////

	ReadData(&voice.fx_morph_params.BaseFreq, 1, sizeof(float), data);
	ReadData(&voice.fx_morph_params.FreqLimit, 1, sizeof(float), data);
	ReadData(&voice.fx_morph_params.FreqRamp, 1, sizeof(float), data);
	ReadData(&voice.fx_morph_params.FreqDRamp, 1, sizeof(float), data);
	ReadData(&voice.fx_morph_params.Duty, 1, sizeof(float), data);
	ReadData(&voice.fx_morph_params.DutyRamp, 1, sizeof(float), data);

	ReadData(&voice.fx_morph_params.VibStrength, 1, sizeof(float), data);
	ReadData(&voice.fx_morph_params.VibSpeed, 1, sizeof(float), data);
	ReadData(&voice.fx_morph_params.VibDelay, 1, sizeof(float), data);

	ReadData(&voice.fx_morph_params.EnvAttack, 1, sizeof(float), data);
	ReadData(&voice.fx_morph_params.EnvSustain, 1, sizeof(float), data);
	ReadData(&voice.fx_morph_params.EnvDecay, 1, sizeof(float), data);
	ReadData(&voice.fx_morph_params.EnvPunch, 1, sizeof(float), data);

	ReadData(&voice.fx_morph_params.LPFResonance, 1, sizeof(float), data);
	ReadData(&voice.fx_morph_params.LPFFreq, 1, sizeof(float), data);
	ReadData(&voice.fx_morph_params.LPFRamp, 1, sizeof(float), data);
	ReadData(&voice.fx_morph_params.HPFFreq, 1, sizeof(float), data);
	ReadData(&voice.fx_morph_params.HPFRamp, 1, sizeof(float), data);

	ReadData(&voice.fx_morph_params.FlangerOffset, 1, sizeof(float), data);
	ReadData(&voice.fx_morph_params.FlangerRamp, 1, sizeof(float), data);

	ReadData(&voice.fx_morph_params.RepeatSpeed, 1, sizeof(float), data);

	ReadData(&voice.fx_morph_params.ArmSpeed, 1, sizeof(float), data);
	ReadData(&voice.fx_morph_params.ArmMod, 1, sizeof(float), data);

	if (version == SFXR0140) {
		ReadData(&voice.fx_base_params.Overtones, 1, sizeof(int), data);
		ReadData(&voice.fx_base_params.OvertoneRamp, 1, sizeof(float), data);

		ReadData(&voice.fx_base_params.BitCrush, 1, sizeof(float), data);
		ReadData(&voice.fx_base_params.BitCrushSweep, 1, sizeof(float), data);
		ReadData(&voice.fx_base_params.CompressionAmount, 1, sizeof(float), data);

		ReadData(&voice.fx_base_params.ArmRepeat, 1, sizeof(float), data);
		ReadData(&voice.fx_base_params.ArmSpeed2, 1, sizeof(float), data);
		ReadData(&voice.fx_base_params.ArmMod2, 1, sizeof(float), data);

		ReadData(&voice.fx_morph_params.Overtones, 1, sizeof(int), data);
		ReadData(&voice.fx_morph_params.OvertoneRamp, 1, sizeof(float), data);

		ReadData(&voice.fx_morph_params.BitCrush, 1, sizeof(float), data);
		ReadData(&voice.fx_morph_params.BitCrushSweep, 1, sizeof(float), data);
		ReadData(&voice.fx_morph_params.CompressionAmount, 1, sizeof(float), data);

		ReadData(&voice.fx_morph_params.ArmRepeat, 1, sizeof(float), data);
		ReadData(&voice.fx_morph_params.ArmSpeed2, 1, sizeof(float), data);
		ReadData(&voice.fx_morph_params.ArmMod2, 1, sizeof(float), data);
	} else {
		voice.fx_base_params.Overtones = voice.fx_morph_params.Overtones = 0;
		voice.fx_base_params.OvertoneRamp = voice.fx_morph_params.OvertoneRamp = 0;

		voice.fx_base_params.BitCrush = voice.fx_morph_params.BitCrush = 0;
		voice.fx_base_params.BitCrushSweep = voice.fx_morph_params.BitCrushSweep = 0;
		voice.fx_base_params.CompressionAmount = voice.fx_morph_params.CompressionAmount = 0;

		voice.fx_base_params.ArmRepeat = voice.fx_morph_params.ArmRepeat = 0;
		voice.fx_base_params.ArmSpeed2 = voice.fx_morph_params.ArmSpeed2 = 0;
		voice.fx_base_params.ArmMod2 = voice.fx_morph_params.ArmMod2 = 0;
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

	g_write(&voice.waveform_shape, 1, sizeof(int), file);

	g_write(&voice.sound_vol, 1, sizeof(float), file);
	g_write(&voice.morph_rate, 1, sizeof(float), file);

	g_write(&voice.length_in_samples, 1, sizeof(float), file);

	g_write(&voice.fx_base_params.BaseFreq, 1, sizeof(float), file);
	g_write(&voice.fx_base_params.FreqLimit, 1, sizeof(float), file);
	g_write(&voice.fx_base_params.FreqRamp, 1, sizeof(float), file);
	g_write(&voice.fx_base_params.FreqDRamp, 1, sizeof(float), file);
	g_write(&voice.fx_base_params.Duty, 1, sizeof(float), file);
	g_write(&voice.fx_base_params.DutyRamp, 1, sizeof(float), file);

	g_write(&voice.fx_base_params.VibStrength, 1, sizeof(float), file);
	g_write(&voice.fx_base_params.VibSpeed, 1, sizeof(float), file);
	g_write(&voice.fx_base_params.VibDelay, 1, sizeof(float), file);

	g_write(&voice.fx_base_params.EnvAttack, 1, sizeof(float), file);
	g_write(&voice.fx_base_params.EnvSustain, 1, sizeof(float), file);
	g_write(&voice.fx_base_params.EnvDecay, 1, sizeof(float), file);
	g_write(&voice.fx_base_params.EnvPunch, 1, sizeof(float), file);

	g_write(&voice.fx_base_params.LPFResonance, 1, sizeof(float), file);
	g_write(&voice.fx_base_params.LPFFreq, 1, sizeof(float), file);
	g_write(&voice.fx_base_params.LPFRamp, 1, sizeof(float), file);
	g_write(&voice.fx_base_params.HPFFreq, 1, sizeof(float), file);
	g_write(&voice.fx_base_params.HPFRamp, 1, sizeof(float), file);

	g_write(&voice.fx_base_params.FlangerOffset, 1, sizeof(float), file);
	g_write(&voice.fx_base_params.FlangerRamp, 1, sizeof(float), file);

	g_write(&voice.fx_base_params.RepeatSpeed, 1, sizeof(float), file);

	g_write(&voice.fx_base_params.ArmSpeed, 1, sizeof(float), file);
	g_write(&voice.fx_base_params.ArmMod, 1, sizeof(float), file);

	//////////////////////////////////////////////////////////////////////////

	g_write(&voice.fx_morph_params.BaseFreq, 1, sizeof(float), file);
	g_write(&voice.fx_morph_params.FreqLimit, 1, sizeof(float), file);
	g_write(&voice.fx_morph_params.FreqRamp, 1, sizeof(float), file);
	g_write(&voice.fx_morph_params.FreqDRamp, 1, sizeof(float), file);
	g_write(&voice.fx_morph_params.Duty, 1, sizeof(float), file);
	g_write(&voice.fx_morph_params.DutyRamp, 1, sizeof(float), file);

	g_write(&voice.fx_morph_params.VibStrength, 1, sizeof(float), file);
	g_write(&voice.fx_morph_params.VibSpeed, 1, sizeof(float), file);
	g_write(&voice.fx_morph_params.VibDelay, 1, sizeof(float), file);

	g_write(&voice.fx_morph_params.EnvAttack, 1, sizeof(float), file);
	g_write(&voice.fx_morph_params.EnvSustain, 1, sizeof(float), file);
	g_write(&voice.fx_morph_params.EnvDecay, 1, sizeof(float), file);
	g_write(&voice.fx_morph_params.EnvPunch, 1, sizeof(float), file);

	g_write(&voice.fx_morph_params.LPFResonance, 1, sizeof(float), file);
	g_write(&voice.fx_morph_params.LPFFreq, 1, sizeof(float), file);
	g_write(&voice.fx_morph_params.LPFRamp, 1, sizeof(float), file);
	g_write(&voice.fx_morph_params.HPFFreq, 1, sizeof(float), file);
	g_write(&voice.fx_morph_params.HPFRamp, 1, sizeof(float), file);

	g_write(&voice.fx_morph_params.FlangerOffset, 1, sizeof(float), file);
	g_write(&voice.fx_morph_params.FlangerRamp, 1, sizeof(float), file);

	g_write(&voice.fx_morph_params.RepeatSpeed, 1, sizeof(float), file);

	g_write(&voice.fx_morph_params.ArmSpeed, 1, sizeof(float), file);
	g_write(&voice.fx_morph_params.ArmMod, 1, sizeof(float), file);

	//////////////////////////////////////////////////////////////////////////

	g_write(&voice.fx_base_params.Overtones, 1, sizeof(int), file);
	g_write(&voice.fx_base_params.OvertoneRamp, 1, sizeof(float), file);

	g_write(&voice.fx_base_params.BitCrush, 1, sizeof(float), file);
	g_write(&voice.fx_base_params.BitCrushSweep, 1, sizeof(float), file);
	g_write(&voice.fx_base_params.CompressionAmount, 1, sizeof(float), file);

	g_write(&voice.fx_base_params.ArmRepeat, 1, sizeof(float), file);
	g_write(&voice.fx_base_params.ArmSpeed2, 1, sizeof(float), file);
	g_write(&voice.fx_base_params.ArmMod2, 1, sizeof(float), file);

	g_write(&voice.fx_morph_params.Overtones, 1, sizeof(int), file);
	g_write(&voice.fx_morph_params.OvertoneRamp, 1, sizeof(float), file);

	g_write(&voice.fx_morph_params.BitCrush, 1, sizeof(float), file);
	g_write(&voice.fx_morph_params.BitCrushSweep, 1, sizeof(float), file);
	g_write(&voice.fx_morph_params.CompressionAmount, 1, sizeof(float), file);

	g_write(&voice.fx_morph_params.ArmRepeat, 1, sizeof(float), file);
	g_write(&voice.fx_morph_params.ArmSpeed2, 1, sizeof(float), file);
	g_write(&voice.fx_morph_params.ArmMod2, 1, sizeof(float), file);

	//////////////////////////////////////////////////////////////////////////

	g_close(file);
	return true;
}

//////////////////////////////////////////////////////////////////////////

bool RetroSFXVoice::CompareSettings(RetroSFXVoice *pOther) {
	if (memcmp(&this->voice, &pOther->voice, sizeof(this->voice)) == 0) {
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////

void RetroSFXVoice::Reset(bool restart) {
	if (!restart) {
		phase = 0;

		fx_work_params.BaseFreq = voice.fx_base_params.BaseFreq;
		fx_work_params.FreqLimit = voice.fx_base_params.FreqLimit;
		fx_work_params.FreqRamp = voice.fx_base_params.FreqRamp;
		fx_work_params.FreqDRamp = voice.fx_base_params.FreqDRamp;
		fx_work_params.Duty = voice.fx_base_params.Duty;
		fx_work_params.DutyRamp = voice.fx_base_params.DutyRamp;

		fx_work_params.VibStrength = voice.fx_base_params.VibStrength;
		fx_work_params.VibSpeed = voice.fx_base_params.VibSpeed;
		fx_work_params.VibDelay = voice.fx_base_params.VibDelay;

		fx_work_params.EnvAttack = voice.fx_base_params.EnvAttack;
		fx_work_params.EnvSustain = voice.fx_base_params.EnvSustain;
		fx_work_params.EnvDecay = voice.fx_base_params.EnvDecay;
		fx_work_params.EnvPunch = voice.fx_base_params.EnvPunch;

		fx_work_params.LPFResonance = voice.fx_base_params.LPFResonance;
		fx_work_params.LPFFreq = voice.fx_base_params.LPFFreq;
		fx_work_params.LPFRamp = voice.fx_base_params.LPFRamp;
		fx_work_params.HPFFreq = voice.fx_base_params.HPFFreq;
		fx_work_params.HPFRamp = voice.fx_base_params.HPFRamp;

		fx_work_params.FlangerOffset = voice.fx_base_params.FlangerOffset;
		fx_work_params.FlangerRamp = voice.fx_base_params.FlangerRamp;

		fx_work_params.RepeatSpeed = voice.fx_base_params.RepeatSpeed;

		fx_work_params.ArmSpeed = voice.fx_base_params.ArmSpeed;
		fx_work_params.ArmMod = voice.fx_base_params.ArmMod;

		fx_work_params.Overtones = voice.fx_morph_params.Overtones;
		fx_work_params.OvertoneRamp = voice.fx_morph_params.OvertoneRamp;

		fx_work_params.BitCrush = voice.fx_morph_params.BitCrush;
		fx_work_params.BitCrushSweep = voice.fx_morph_params.BitCrushSweep;
		fx_work_params.CompressionAmount = voice.fx_morph_params.CompressionAmount;

		fx_work_params.ArmRepeat = voice.fx_morph_params.ArmRepeat;
		fx_work_params.ArmSpeed2 = voice.fx_morph_params.ArmSpeed2;
		fx_work_params.ArmMod2 = voice.fx_morph_params.ArmMod2;
	}

	period = 100.0 / (fx_work_params.BaseFreq * fx_work_params.BaseFreq + 0.001);
	iperiod = period;
	maxperiod = 100.0 / (fx_work_params.FreqLimit * fx_work_params.FreqLimit + 0.001);
	slide = 1 - Math::pow(fx_work_params.FreqRamp, 3) * 0.01;
	dslide = -Math::pow(fx_work_params.FreqDRamp, 3) * 0.000001;
	square_duty = 0.5 - fx_work_params.Duty * 0.5;
	square_slide = -fx_work_params.DutyRamp * 0.00005;
	if (fx_work_params.ArmMod >= 0) {
		arm_mod = 1.0 - Math::pow(fx_work_params.ArmMod, 2) * 0.9;
	} else {
		arm_mod = 1.0 + Math::pow(fx_work_params.ArmMod, 2) * 10.0;
	}
	arm_time = 0;
	arm_limit = (int)(Math::pow(1 - fx_work_params.ArmSpeed, 2) * 20000 + 32);
	if (fx_work_params.ArmSpeed == 1) {
		arm_limit = 0;
	}

	if (!restart) {
		// reset filter
		fltp = 0;
		fltdp = 0;
		fltw = Math::pow(fx_work_params.LPFFreq, 3) * 0.1;
		fltw_d = 1 + fx_work_params.LPFRamp * 0.0001;
		fltdmp = 5 / (1 + Math::pow(fx_work_params.LPFResonance, 2) * 20) * (0.01 + fltw);
		if (fltdmp > 0.8) {
			fltdmp = 0.8;
		}
		fltphp = 0;
		flthp = Math::pow(fx_work_params.HPFFreq, 2) * 0.1;
		flthm_d = 1 + fx_work_params.HPFRamp * 0.0003;
		// reset vibrato
		vib_phase = 0;
		vib_speed = Math::pow(voice.fx_base_params.VibSpeed, 2) * 0.01;
		vib_amp = fx_work_params.VibStrength * 0.5;
		// reset envelope
		env_vol = 0;
		env_stage = 0;
		env_time = 0;
		env_length[0] = fx_work_params.EnvAttack * fx_work_params.EnvAttack * 100000;
		env_length[1] = fx_work_params.EnvSustain * fx_work_params.EnvSustain * 100000;
		env_length[2] = fx_work_params.EnvDecay * fx_work_params.EnvDecay * 100000;

		fphase = Math::pow(fx_work_params.FlangerOffset, 2) * 1020;
		if (fx_work_params.FlangerOffset < 0) {
			fphase = -fphase;
		}
		fdphase = Math::pow(fx_work_params.FlangerRamp, 2) * 1;
		if (fx_work_params.FlangerRamp < 0) {
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
		rem_limit = Math::pow(1 - fx_work_params.RepeatSpeed, 2) * 20000 + 32;
		if (fx_work_params.RepeatSpeed == 0) {
			rem_limit = 0;
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void RetroSFXVoice::Play(void *data) {
	if (data) {
		RetroVoice103 *pVoice103 = (RetroVoice103 *)data;
		if (pVoice103->version == SFXR0100) {
			memcpy(&voice, data, sizeof(RetroVoice103));
		}
	}
	wav_samples_rendered = 0;
	Reset(false);
	playing_sample = true;
}

//////////////////////////////////////////////////////////////////////////

void RetroSFXVoice::Play(bool calculate_length) {
	if (calculate_length) {
		voice.length_in_samples = 0;
	}
	wav_samples_rendered = 0;
	Reset(false);
	playing_sample = true;
}

//////////////////////////////////////////////////////////////////////////

int RetroSFXVoice::GetVoiceLengthInSamples() const {
	return voice.length_in_samples;
}

//////////////////////////////////////////////////////////////////////////

int RetroSFXVoice::Render(int samples, BufferCallback *callback) {
	int samples_rendered = 0;

	for (int i = 0; i < samples; i++) {
		if (!playing_sample) {
			break;
		}
		rem_time++;
		if (rem_limit != 0 && rem_time >= rem_limit) {
			rem_time = 0;
			Reset(true);

			if (voice.morph_rate != 0) {
				Morph(fx_work_params.BaseFreq, voice.fx_morph_params.BaseFreq);
				Morph(fx_work_params.FreqLimit, voice.fx_morph_params.FreqLimit);
				Morph(fx_work_params.FreqRamp, voice.fx_morph_params.FreqRamp);
				Morph(fx_work_params.FreqDRamp, voice.fx_morph_params.FreqDRamp);
				Morph(fx_work_params.Duty, voice.fx_morph_params.Duty);
				Morph(fx_work_params.DutyRamp, voice.fx_morph_params.DutyRamp);

				Morph(fx_work_params.VibStrength, voice.fx_morph_params.VibStrength);
				Morph(fx_work_params.VibSpeed, voice.fx_morph_params.VibSpeed);
				Morph(fx_work_params.VibDelay, voice.fx_morph_params.VibDelay);

				Morph(fx_work_params.EnvAttack, voice.fx_morph_params.EnvAttack);
				Morph(fx_work_params.EnvSustain, voice.fx_morph_params.EnvSustain);
				Morph(fx_work_params.EnvDecay, voice.fx_morph_params.EnvDecay);
				Morph(fx_work_params.EnvPunch, voice.fx_morph_params.EnvPunch);

				Morph(fx_work_params.LPFResonance, voice.fx_morph_params.LPFResonance);
				Morph(fx_work_params.LPFFreq, voice.fx_morph_params.LPFFreq);
				Morph(fx_work_params.LPFRamp, voice.fx_morph_params.LPFRamp);
				Morph(fx_work_params.HPFFreq, voice.fx_morph_params.HPFFreq);
				Morph(fx_work_params.HPFRamp, voice.fx_morph_params.HPFRamp);

				Morph(fx_work_params.FlangerOffset, voice.fx_morph_params.FlangerOffset);
				Morph(fx_work_params.FlangerRamp, voice.fx_morph_params.FlangerRamp);

				Morph(fx_work_params.RepeatSpeed, voice.fx_morph_params.RepeatSpeed);

				Morph(fx_work_params.ArmSpeed, voice.fx_morph_params.ArmSpeed);
				Morph(fx_work_params.ArmMod, voice.fx_morph_params.ArmMod);

				// Morph(fx_work_params.Overtones, voice.fx_morph_params.Overtones); // int ?
				Morph(fx_work_params.OvertoneRamp, voice.fx_morph_params.OvertoneRamp);

				Morph(fx_work_params.BitCrush, voice.fx_morph_params.BitCrush);
				Morph(fx_work_params.BitCrushSweep, voice.fx_morph_params.BitCrushSweep);
				Morph(fx_work_params.CompressionAmount, voice.fx_morph_params.CompressionAmount);

				Morph(fx_work_params.ArmRepeat, voice.fx_morph_params.ArmRepeat);
				Morph(fx_work_params.ArmSpeed2, voice.fx_morph_params.ArmSpeed2);
				Morph(fx_work_params.ArmMod2, voice.fx_morph_params.ArmMod2);
			}
		}

		// frequency envelopes/arpeggios
		arm_time++;
		if (arm_limit != 0 && arm_time >= arm_limit) {
			arm_time = 0;
			period *= arm_mod;
		}
		slide += dslide;
		period *= slide;
		if (period > maxperiod) {
			period = maxperiod;
			if (fx_work_params.FreqLimit > 0) {
				playing_sample = false;
			}
		}
		float rperiod = period;
		if (vib_amp > 0) {
			vib_phase += vib_speed;
			rperiod = period * (1 + Math::sin(vib_phase) * vib_amp);
		}
		iperiod = rperiod;
		if (iperiod < 8) {
			iperiod = 8;
		}
		square_duty += square_slide;
		if (square_duty < 0) {
			square_duty = 0;
		}
		if (square_duty > 0.5) {
			square_duty = 0.5;
		}
		// volume envelope
		env_time++;
		if (env_time > env_length[env_stage]) {
			env_time = 0;
			env_stage++;
			if (env_stage == 3) {
				playing_sample = false;
			}
		}
		if (env_stage == 0) {
			env_vol = float(env_time) / env_length[0];
		}
		if (env_stage == 1) {
			env_vol = 1 + Math::pow(1 - float(env_time) / env_length[1], 1) * 2 * fx_work_params.EnvPunch;
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
			if (phase >= iperiod) {
				// phase = 0;
				phase %= iperiod;
				if (voice.waveform_shape == 3) {
					for (int n = 0; n < 32; n++) {
						noise_buffer[n] = GenNoise();
					}
				} else if (voice.waveform_shape == 5) {
					for (int n = 0; n < 32; n++) {
						pink_noise_buffer[n] = GenPinkNoise();
					}
				}
			}
			// base waveform
			float fp = float(phase) / iperiod;
			float ots = 1; // overtonestrength
			float sample = 0;
			for (int k = 0; k <= voice.fx_base_params.Overtones; k++) {
				float _sample = 0;
				switch (voice.waveform_shape) {
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
						_sample = noise_buffer[phase * 32 / iperiod];
					} break;
					case 4: { // triangle
						_sample = Math::abs(1 - fp * 2) - 1;
					} break;
					case 5: { // pink noise
						_sample = pink_noise_buffer[(phase * 32 / iperiod) % 32];
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
						pos = ((phase * 20) % iperiod) / iperiod;
						pos = pos > 0.5 ? (pos - 1.0) * 6.28318531 : pos * 6.28318531;
						const float sample2 = pos < 0 ? 1.27323954 * pos + 0.405284735 * pos * pos : 1.27323954 * pos - 0.405284735 * pos * pos;
						_sample = 0.25 * (sample2 < 0 ? 0.225 * (sample2 * -sample2 - sample2) + sample2 : 0.225 * (sample2 * sample2 - sample2) + sample2);
					} break;
					case 8: { // breaker
						_sample = Math::abs(1 - fp * fp * 2) - 1;
					} break;
				}
				sample += _sample * ots;
				ots *= (1 - voice.fx_base_params.OvertoneRamp);
			}
			// lp filter
			const float pp = fltp;
			fltw *= fltw_d;
			if (fltw < 0) {
				fltw = 0;
			}
			if (fltw > 0.1) {
				fltw = 0.1;
			}
			if (fx_work_params.LPFFreq != 1) {
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
		ssample = ssample / 8 * master_vol;

		ssample *= 2 * voice.sound_vol;

		if (callback != nullptr) {
			// clamp
			if (ssample > 1) {
				ssample = 1;
			}
			if (ssample < -1) {
				ssample = -1;
			}
			(*callback)(ssample);
			samples_rendered++;
		}
	}

	// Calculating length here
	wav_samples_rendered += samples_rendered;
	if (playing_sample == false) {
		voice.length_in_samples = wav_samples_rendered;
	}

	return samples_rendered;
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

void RetroSFXVoice::Morph(float &morph_var, float morph_dest) {
	const float diff = morph_dest - morph_var;
	if (diff != 0) {
		morph_var += diff * voice.morph_rate;
	}
}

//////////////////////////////////////////////////////////////////////////

void RetroSFXVoice::Mutate() {
}

void RetroSFXVoice::Randomize() {
	voice.waveform_shape = rnd(8);

	voice.fx_base_params.BaseFreq = Math::pow(frnd(1), 2);
	if (rnd(1)) {
		voice.fx_base_params.BaseFreq = Math::pow(frnd(1), 3) + 0.5;
	}
	voice.fx_base_params.FreqLimit = 0;
	voice.fx_base_params.FreqRamp = pow(frnd(1), 5);
	if (voice.fx_base_params.BaseFreq > 0.7 && voice.fx_base_params.FreqRamp > 0.2) {
		voice.fx_base_params.FreqRamp = -voice.fx_base_params.FreqRamp;
	}
	if (voice.fx_base_params.BaseFreq < 0.2 && voice.fx_base_params.FreqRamp < -0.05) {
		voice.fx_base_params.FreqRamp = -voice.fx_base_params.FreqRamp;
	}
	voice.fx_base_params.FreqDRamp = Math::pow(frnd(1), 3);
	voice.fx_base_params.Duty = frnd(1);
	voice.fx_base_params.DutyRamp = Math::pow(frnd(1), 3);
	voice.fx_base_params.VibStrength = Math::pow(frnd(1), 3);
	voice.fx_base_params.VibSpeed = frnd(1);
	voice.fx_base_params.VibDelay = frnd(1);
	voice.fx_base_params.EnvAttack = Math::pow(frnd(1), 3);
	voice.fx_base_params.EnvSustain = Math::pow(frnd(1), 2);
	voice.fx_base_params.EnvDecay = ((frnd(1)) / 2) + 0.5;
	voice.fx_base_params.EnvPunch = Math::pow(frnd(0.8), 2);
	if (voice.fx_base_params.EnvAttack + voice.fx_base_params.EnvSustain + voice.fx_base_params.EnvDecay < 0.2) {
		voice.fx_base_params.EnvSustain += 0.2 + frnd(0.3);
		voice.fx_base_params.EnvDecay += 0.2 + frnd(0.3);
	}
	voice.fx_base_params.LPFResonance = frnd(1);
	voice.fx_base_params.LPFFreq = 1.0f - Math::pow(frnd(1.0f), 3);
	voice.fx_base_params.LPFRamp = Math::pow(frnd(1.0f), 3.0f);
	if (voice.fx_base_params.LPFFreq < 0.1f && voice.fx_base_params.LPFRamp < -0.05) {
		voice.fx_base_params.LPFRamp = -voice.fx_base_params.LPFRamp;
	}
	voice.fx_base_params.HPFFreq = Math::pow(frnd(1), 5);
	voice.fx_base_params.HPFRamp = Math::pow(frnd(1), 5);
	voice.fx_base_params.FlangerOffset = Math::pow(frnd(1), 3);
	voice.fx_base_params.FlangerRamp = Math::pow(frnd(1), 3);
	voice.fx_base_params.RepeatSpeed = frnd(1);
	voice.fx_base_params.ArmSpeed = frnd(1);
	voice.fx_base_params.ArmMod = frnd(1);

	//////////////////////////////////////////////////////////////////////////
	// for morph

	if (voice.fx_base_params.RepeatSpeed == 0) {
		// If we don't have a repeat then we don't need a morph
		voice.morph_rate = 0;
	} else {
		voice.morph_rate = frnd(2);
		voice.morph_rate -= 1;
		if (voice.morph_rate < 0) {
			voice.morph_rate = 0; // 50% chance of morph
		}
	}

	voice.fx_morph_params.BaseFreq = Math::pow(frnd(1), 2);
	if (rnd(1)) {
		voice.fx_morph_params.BaseFreq = Math::pow(frnd(1), 3) + 0.5;
	}
	voice.fx_morph_params.FreqLimit = 0;
	voice.fx_morph_params.FreqRamp = Math::pow(frnd(1), 5);
	if (voice.fx_morph_params.BaseFreq > 0.7 && voice.fx_morph_params.FreqRamp > 0.2) {
		voice.fx_morph_params.FreqRamp = -voice.fx_morph_params.FreqRamp;
	}
	if (voice.fx_morph_params.BaseFreq < 0.2 && voice.fx_morph_params.FreqRamp < -0.05) {
		voice.fx_morph_params.FreqRamp = -voice.fx_morph_params.FreqRamp;
	}
	voice.fx_morph_params.FreqDRamp = Math::pow(frnd(1), 3);
	voice.fx_morph_params.Duty = frnd(1);
	voice.fx_morph_params.DutyRamp = Math::pow(frnd(1), 3);
	voice.fx_morph_params.VibStrength = Math::pow(frnd(1), 3);
	voice.fx_morph_params.VibSpeed = frnd(1);
	voice.fx_morph_params.VibDelay = frnd(1);
	voice.fx_morph_params.EnvAttack = Math::pow(frnd(1), 3);
	voice.fx_morph_params.EnvSustain = Math::pow(frnd(1), 2);
	voice.fx_morph_params.EnvDecay = frnd(1);
	voice.fx_morph_params.EnvPunch = Math::pow(frnd(0.8), 2);
	if (voice.fx_morph_params.EnvAttack + voice.fx_morph_params.EnvSustain + voice.fx_morph_params.EnvDecay < 0.2) {
		voice.fx_morph_params.EnvSustain += 0.2 + frnd(0.3);
		voice.fx_morph_params.EnvDecay += 0.2 + frnd(0.3);
	}
	voice.fx_morph_params.LPFResonance = frnd(1);
	voice.fx_morph_params.LPFFreq = 1 - Math::pow(frnd(1), 3);
	voice.fx_morph_params.LPFRamp = Math::pow(frnd(1), 3);
	if (voice.fx_morph_params.LPFFreq < 0.1 && voice.fx_morph_params.LPFRamp < -0.05) {
		voice.fx_morph_params.LPFRamp = -voice.fx_morph_params.LPFRamp;
	}
	voice.fx_morph_params.HPFFreq = Math::pow(frnd(1), 5);
	voice.fx_morph_params.HPFRamp = Math::pow(frnd(1), 5);
	voice.fx_morph_params.FlangerOffset = Math::pow(frnd(1), 3);
	voice.fx_morph_params.FlangerRamp = Math::pow(frnd(1), 3);
	voice.fx_morph_params.RepeatSpeed = frnd(1);
	voice.fx_morph_params.ArmSpeed = frnd(1);
	voice.fx_morph_params.ArmMod = frnd(1);
}

//////////////////////////////////////////////////////////////////////////

bool RetroSFXVoice::ExportWav(const char *filename, int wav_bits, int wav_freq) {
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
		virtual void append_sample(float s) {
			s *= 4; // arbitrary gain to get reasonable output volume ...
			if (s > 1) {
				s = 1;
			}
			if (s < -1) {
				s = -1;
			}
			file_sample += s;
			file_acc++;
			// quantize depending on format
			if (wav_freq == 44100 || file_acc == file_acc_key) {
				file_sample /= file_acc;
				file_acc = 0;
				if (wav_bits == 16) {
					const int16_t isample = file_sample * 0x7fff;
					data.push_back(isample & 0xff);
					data.push_back((isample >> 8) & 0xff);
				} else if (wav_bits == 8) {
					const uint8_t isample = file_sample * 0x7f + 0x80;
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

	SampleBuffer buffer(wav_bits, wav_freq);
	Play();
	while (IsActive()) {
		Render(256, &buffer);
	}

	const int num_channels = 1; // monoaural
	const int num_samples = GetVoiceLengthInSamples();
	const int bytes_per_sample = wav_bits / 2;
	const int sample_rate = wav_freq;
	const int byte_rate = sample_rate * num_channels * bytes_per_sample;

	G_FILE *wav_file = g_open_write(filename);
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
	return playing_sample;
}
