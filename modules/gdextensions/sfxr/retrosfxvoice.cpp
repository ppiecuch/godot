/*************************************************************************/
/*  retrosfxvoice.cpp                                                    */
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

//////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS
#include "retrosfxvoice.h"

#include "core/math/math_funcs.h"
#include "core/os/dir_access.h"
#include "core/os/file_access.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory>

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
			case SEEK_SET:
				f->fa->seek(offset);
				break;
			case SEEK_CUR:
				f->fa->seek(f->fa->get_position() + offset);
				break;
			case SEEK_END:
				f->fa->seek_end(offset);
				break;
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
ssize_t g_read(void *buf, size_t len, size_t cnt, G_FILE *f) {
	if (f) {
		return f->fa->get_buffer((uint8_t *)buf, cnt * len);
	}
	return SUCCESS;
}
ssize_t g_write(const void *buf, size_t len, size_t cnt, G_FILE *f) {
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
ssize_t g_filesize(G_FILE *f) {
	if (f) {
		return f->fa->get_len();
	}
	return 0;
}

#include <math.h>
#include <string.h>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////

#define rnd(n) (rand() % (n + 1))

#define PI 3.14159265f

//////////////////////////////////////////////////////////////////////////

RetroSFXVoice::RetroSFXVoice() {
	m_fMasterVol = 0.05;
	m_Voice.fSoundVol = 0.5;
	m_bPlayingSample = false;
	m_WavSamplesRendered = 0;

	ResetParams();
}

//////////////////////////////////////////////////////////////////////////

RetroSFXVoice::~RetroSFXVoice() {
}

//////////////////////////////////////////////////////////////////////////

void RetroSFXVoice::ResetParams() {
	m_Voice.nWaveformType = 0;

	m_Voice.FXBaseParams.fBaseFreq = 0.3;
	m_Voice.FXBaseParams.fFreqLimit = 0;
	m_Voice.FXBaseParams.fFreqRamp = 0;
	m_Voice.FXBaseParams.fFreqDRamp = 0;
	m_Voice.FXBaseParams.fDuty = 0;
	m_Voice.FXBaseParams.fDutyRamp = 0;

	m_Voice.FXBaseParams.fVibStrength = 0;
	m_Voice.FXBaseParams.fVibSpeed = 0;
	m_Voice.FXBaseParams.fVibDelay = 0;

	m_Voice.FXBaseParams.fEnvAttack = 0;
	m_Voice.FXBaseParams.fEnvSustain = 0.3;
	m_Voice.FXBaseParams.fEnvDecay = 0.4;
	m_Voice.FXBaseParams.fEnvPunch = 0.0;

	m_Voice.FXBaseParams.fLPFResonance = 0;
	m_Voice.FXBaseParams.fLPFFreq = 1;
	m_Voice.FXBaseParams.fLPFRamp = 0;
	m_Voice.FXBaseParams.fHPFFreq = 0;
	m_Voice.FXBaseParams.fHPFRamp = 0;

	m_Voice.FXBaseParams.fPHAOffset = 0;
	m_Voice.FXBaseParams.fPHARamp = 0;

	m_Voice.FXBaseParams.fRepeatSpeed = 0;

	m_Voice.FXBaseParams.fArmSpeed = 1;
	m_Voice.FXBaseParams.fArmMod = 0;

	//////////////////////////////////////////////////////////////////////////

	m_Voice.fMorphRate = 0;

	//////////////////////////////////////////////////////////////////////////

	m_Voice.FXMorphParams.fBaseFreq = 0.3f;
	m_Voice.FXMorphParams.fFreqLimit = 0.0f;
	m_Voice.FXMorphParams.fFreqRamp = 0.0f;
	m_Voice.FXMorphParams.fFreqDRamp = 0.0f;
	m_Voice.FXMorphParams.fDuty = 0.0f;
	m_Voice.FXMorphParams.fDutyRamp = 0.0f;

	m_Voice.FXMorphParams.fVibStrength = 0.0f;
	m_Voice.FXMorphParams.fVibSpeed = 0.0f;
	m_Voice.FXMorphParams.fVibDelay = 0.0f;

	//	m_morph_env_attack = 0;
	//	m_morph_env_sustain = 0.3;
	//	m_morph_env_decay = 0.4;
	//	m_morph_env_punch = 0;

	m_Voice.FXMorphParams.fLPFResonance = 0;
	m_Voice.FXMorphParams.fLPFFreq = 1;
	m_Voice.FXMorphParams.fLPFRamp = 0;
	m_Voice.FXMorphParams.fHPFFreq = 0;
	m_Voice.FXMorphParams.fHPFRamp = 0;

	m_Voice.FXMorphParams.fPHAOffset = 0;
	m_Voice.FXMorphParams.fPHARamp = 0;

	m_Voice.FXMorphParams.fRepeatSpeed = 0;

	m_Voice.FXMorphParams.fArmSpeed = 1;
	m_Voice.FXMorphParams.fArmMod = 0;
}

//////////////////////////////////////////////////////////////////////////

int RetroSFXVoice::ReadData(void *pDest, int nSize, int nUnits, unsigned char *&pRAWData) {
	int read_size = nSize * nUnits;
	memcpy(pDest, pRAWData, nSize * nUnits);
	pRAWData += read_size;

	return read_size;
}

//////////////////////////////////////////////////////////////////////////

bool RetroSFXVoice::LoadSettings(unsigned char *pRAWData) {
	int version = 0;
	ReadData(&version, 1, sizeof(int), pRAWData);
	if (version != SFXR0100) {
		return false;
	}

	ReadData(&m_Voice.nWaveformType, 1, sizeof(int), pRAWData);

	m_Voice.fSoundVol = 0.5;
	ReadData(&m_Voice.fSoundVol, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.fMorphRate, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.nLengthInSamples, 1, sizeof(float), pRAWData);

	//////////////////////////////////////////////////////////////////////////

	ReadData(&m_Voice.FXBaseParams.fBaseFreq, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fFreqLimit, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fFreqRamp, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fFreqDRamp, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fDuty, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fDutyRamp, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.FXBaseParams.fVibStrength, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fVibSpeed, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fVibDelay, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.FXBaseParams.fEnvAttack, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fEnvSustain, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fEnvDecay, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fEnvPunch, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.FXBaseParams.fLPFResonance, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fLPFFreq, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fLPFRamp, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fHPFFreq, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fHPFRamp, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.FXBaseParams.fPHAOffset, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fPHARamp, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.FXBaseParams.fRepeatSpeed, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.FXBaseParams.fArmSpeed, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXBaseParams.fArmMod, 1, sizeof(float), pRAWData);

	//////////////////////////////////////////////////////////////////////////

	ReadData(&m_Voice.FXMorphParams.fBaseFreq, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fFreqLimit, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fFreqRamp, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fFreqDRamp, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fDuty, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fDutyRamp, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.FXMorphParams.fVibStrength, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fVibSpeed, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fVibDelay, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.FXMorphParams.fEnvAttack, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fEnvSustain, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fEnvDecay, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fEnvPunch, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.FXMorphParams.fLPFResonance, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fLPFFreq, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fLPFRamp, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fHPFFreq, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fHPFRamp, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.FXMorphParams.fPHAOffset, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fPHARamp, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.FXMorphParams.fRepeatSpeed, 1, sizeof(float), pRAWData);

	ReadData(&m_Voice.FXMorphParams.fArmSpeed, 1, sizeof(float), pRAWData);
	ReadData(&m_Voice.FXMorphParams.fArmMod, 1, sizeof(float), pRAWData);

	//////////////////////////////////////////////////////////////////////////

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool RetroSFXVoice::LoadSettings(const char *filename) {
	if (G_FILE *fp = g_open_read(filename)) {
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

bool RetroSFXVoice::SaveSettings(const char *filename) {
	G_FILE *file = g_open_write(filename);
	if (not file) {
		return false;
	}

	int version = SFXR0100;
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

	g_write(&m_Voice.FXBaseParams.fPHAOffset, 1, sizeof(float), file);
	g_write(&m_Voice.FXBaseParams.fPHARamp, 1, sizeof(float), file);

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

	g_write(&m_Voice.FXMorphParams.fPHAOffset, 1, sizeof(float), file);
	g_write(&m_Voice.FXMorphParams.fPHARamp, 1, sizeof(float), file);

	g_write(&m_Voice.FXMorphParams.fRepeatSpeed, 1, sizeof(float), file);

	g_write(&m_Voice.FXMorphParams.fArmSpeed, 1, sizeof(float), file);
	g_write(&m_Voice.FXMorphParams.fArmMod, 1, sizeof(float), file);

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

		m_FXWorkParams.fPHAOffset = m_Voice.FXBaseParams.fPHAOffset;
		m_FXWorkParams.fPHARamp = m_Voice.FXBaseParams.fPHARamp;

		m_FXWorkParams.fRepeatSpeed = m_Voice.FXBaseParams.fRepeatSpeed;

		m_FXWorkParams.fArmSpeed = m_Voice.FXBaseParams.fArmSpeed;
		m_FXWorkParams.fArmMod = m_Voice.FXBaseParams.fArmMod;
	}

	fperiod = 100.0 / (m_FXWorkParams.fBaseFreq * m_FXWorkParams.fBaseFreq + 0.001);
	period = (int)fperiod;
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
		fltw = pow(m_FXWorkParams.fLPFFreq, 3) * 0.1;
		fltw_d = 1 + m_FXWorkParams.fLPFRamp * 0.0001;
		fltdmp = 5 / (1 + pow(m_FXWorkParams.fLPFResonance, 2) * 20) * (0.01 + fltw);
		if (fltdmp > 0.8)
			fltdmp = 0.8;
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
		env_length[0] = (int)(m_FXWorkParams.fEnvAttack * m_FXWorkParams.fEnvAttack * 100000.0);
		env_length[1] = (int)(m_FXWorkParams.fEnvSustain * m_FXWorkParams.fEnvSustain * 100000.0);
		env_length[2] = (int)(m_FXWorkParams.fEnvDecay * m_FXWorkParams.fEnvDecay * 100000.0);

		fphase = pow(m_FXWorkParams.fPHAOffset, 2) * 1020.0;
		if (m_FXWorkParams.fPHAOffset < 0)
			fphase = -fphase;
		fdphase = pow(m_FXWorkParams.fPHARamp, 2) * 1;
		if (m_FXWorkParams.fPHARamp < 0)
			fdphase = -fdphase;
		iphase = Math::abs(fphase);
		ipp = 0;
		for (int i = 0; i < 1024; i++) {
			phaser_buffer[i] = 0;
		}
		for (int i = 0; i < 32; i++) {
			noise_buffer[i] = GenNoise();
		}
		rem_time = 0;
		rem_limit = (int)(pow(1 - m_FXWorkParams.fRepeatSpeed, 2) * 20000 + 32);
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

int RetroSFXVoice::GetVoiceLengthInSamples() {
	return m_Voice.nLengthInSamples;
}

//////////////////////////////////////////////////////////////////////////

int RetroSFXVoice::Render(int nSamples, short *pBuffer) {
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

				Morph(m_FXWorkParams.fPHAOffset, m_Voice.FXMorphParams.fPHAOffset);
				Morph(m_FXWorkParams.fPHARamp, m_Voice.FXMorphParams.fPHARamp);

				Morph(m_FXWorkParams.fRepeatSpeed, m_Voice.FXMorphParams.fRepeatSpeed);

				Morph(m_FXWorkParams.fArmSpeed, m_Voice.FXMorphParams.fArmSpeed);
				Morph(m_FXWorkParams.fArmMod, m_Voice.FXMorphParams.fArmMod);
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
		float rfperiod = (float)fperiod;
		if (vib_amp > 0) {
			vib_phase += vib_speed;
			rfperiod = (float)fperiod * (1 + Math::sin(vib_phase) * vib_amp);
		}
		period = (int)rfperiod;
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
		if (env_stage == 0)
			env_vol = (float)env_time / env_length[0];
		if (env_stage == 1)
			env_vol = 1 + pow(1 - (float)env_time / env_length[1], 1) * 2 * m_FXWorkParams.fEnvPunch;
		if (env_stage == 2)
			env_vol = 1 - (float)env_time / env_length[2];

		// phaser step
		fphase += fdphase;
		iphase = abs((int)fphase);
		if (iphase > 1023)
			iphase = 1023;

		if (flthm_d != 0) {
			flthp *= flthm_d;
			if (flthp < 0.00001)
				flthp = 0.00001;
			if (flthp > 0.1)
				flthp = 0.1;
		}

		float ssample = 0;
		for (int si = 0; si < 8; si++) { // 8x oversampling
			float sample = 0;
			phase++;
			if (phase >= period) {
				// phase=0;
				phase %= period;
				if (m_Voice.nWaveformType == 3)
					for (int i = 0; i < 32; i++) {
						noise_buffer[i] = GenNoise();
					}
			}
			// base waveform
			float fp = (float)phase / period;
			switch (m_Voice.nWaveformType) {
				case 0: // square
					if (fp < square_duty)
						sample = 0.5;
					else
						sample = -0.5;
					break;
				case 1: // sawtooth
					sample = 1 - fp * 2;
					break;
				case 2: // sine
					sample = Math::sin(fp * 2 * PI);
					break;
				case 3: // noise
					sample = noise_buffer[phase * 32 / period];
					break;
			}
			// lp filter
			float pp = fltp;
			fltw *= fltw_d;
			if (fltw < 0.0)
				fltw = 0;
			if (fltw > 0.1)
				fltw = 0.1;
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

		if (pBuffer != nullptr) {
			// Clamp
			if (ssample > 1)
				ssample = 1;
			if (ssample < -1)
				ssample = -1;

			*pBuffer += (short)(ssample * 32767.0f);
			pBuffer++;
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

float RetroSFXVoice::GenNoise() {
	const float range = 2;
	return ((float)rnd(10000) / 10000 * range) - 1;
}

//////////////////////////////////////////////////////////////////////////

void RetroSFXVoice::Morph(float &fMorphVar, float fMorphDest) {
	const float diff = fMorphDest - fMorphVar;

	if (diff != 0) {
		fMorphVar += diff * m_Voice.fMorphRate;
	}
}

//////////////////////////////////////////////////////////////////////////

bool RetroSFXVoice::IsActive() {
	if (m_bPlayingSample) {
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
