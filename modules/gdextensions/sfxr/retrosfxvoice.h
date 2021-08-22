/*************************************************************************/
/*  retrosfxvoice.h                                                      */
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

#define SFXR0100 (0x46580100) // FX 1.00

//////////////////////////////////////////////////////////////////////////

typedef struct _FXParams103 {
	float fBaseFreq;
	float fFreqLimit;
	float fFreqRamp;
	float fFreqDRamp;
	float fDuty;
	float fDutyRamp;

	float fVibStrength;
	float fVibSpeed;
	float fVibDelay;

	float fEnvAttack;
	float fEnvSustain;
	float fEnvDecay;
	float fEnvPunch;

	float fLPFResonance;
	float fLPFFreq;
	float fLPFRamp;

	float fHPFFreq;
	float fHPFRamp;

	float fPHAOffset;
	float fPHARamp;

	float fRepeatSpeed;

	float fArmSpeed;
	float fArmMod;
} FXParams103;

//////////////////////////////////////////////////////////////////////////

typedef struct _RetroVoice103 {
	int nVersion;
	int nWaveformType;
	float fSoundVol;
	float fMorphRate;
	FXParams103 FXBaseParams;
	FXParams103 FXMorphParams;
	int nLengthInSamples;
} RetroVoice103;

//////////////////////////////////////////////////////////////////////////

class RetroSFXVoice {
public:
	RetroSFXVoice();
	~RetroSFXVoice();

	void ResetParams();
	int ReadData(void *pDest, int nSize, int nUnits, unsigned char *&pRAWData);
	bool LoadSettings(unsigned char *pRAWData);
	bool LoadSettings(const char *filename);
	bool SaveSettings(const char *filename);
	bool CompareSettings(RetroSFXVoice *pOther);
	void Reset(bool restart);
	void Play(void *pData = 0);
	void Play(bool bCalculateLength);
	int GetVoiceLengthInSamples();
	int Render(int nSamples, short *pBuffer);
	float GenNoise();

	void Morph(float &fMorphVar, float fMorphDest);
	//////////////////////////////////////////////////////////////////////////

	bool IsActive();

	// Technically all of these should be private but the editor pokes these directly so we shall ignore this aside for now.

	RetroVoice103 m_Voice;
	FXParams103 m_FXWorkParams;

	int m_WavSamplesRendered;

	//////////////////////////////////////////////////////////////////////////

	float m_fMasterVol;
	bool m_bPlayingSample;

	int phase;
	double fperiod;
	double fmaxperiod;
	double fslide;
	double fdslide;
	int period;
	float square_duty;
	float square_slide;
	int env_stage;
	int env_time;
	int env_length[3];
	float env_vol;
	float fphase;
	float fdphase;
	int iphase;
	float phaser_buffer[1024];
	int ipp;
	float noise_buffer[32];
	float fltp;
	float fltdp;
	float fltw;
	float fltw_d;
	float fltdmp;
	float fltphp;
	float flthp;
	float flthm_d;
	float vib_phase;
	float vib_speed;
	float vib_amp;
	int rem_time;
	int rem_limit;
	int arm_time;
	int arm_limit;
	double arm_mod;
};

//////////////////////////////////////////////////////////////////////////
