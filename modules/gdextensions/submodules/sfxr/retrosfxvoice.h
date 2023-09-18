/**************************************************************************/
/*  retrosfxvoice.h                                                       */
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

#pragma once

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

//////////////////////////////////////////////////////////////////////////

#define SFXR0100 (0x46580100) // FX 1.00
#define SFXR0140 (0x46580400) // FX 1.40

//////////////////////////////////////////////////////////////////////////

typedef struct _FXParams103 {
	int Overtones; // harmonics: overlays copies of the waveform with copies and multiples of its frequency. Good for bulking
				   // out or otherwise enriching the texture of the sounds (warning: this is the number 1 cause of sfxr slowdown!) (0 to 1)
	float OvertoneRamp; // Harmonics falloff: the rate at which higher overtones should decay (0 to 1)

	float BaseFreq; // base note of the sound (0 to 1)
	float FreqLimit; // if sliding, the sound will stop at this frequency, to prevent really low notes (0 to 1)
	float FreqRamp;
	float FreqDRamp;
	float Duty; // controls the ratio between the up and down states of the square wave, changing the tibre (0 to 1)
	float DutyRamp; // sweeps the duty up or down (-1 to 1)

	float VibStrength; // strength of the vibrato effect (0 to 1)
	float VibSpeed; // speed of the vibrato effect (i.e. frequency) (0 to 1)
	float VibDelay;

	float EnvAttack; // length of the volume envelope attack (0 to 1)
	float EnvSustain; // length of the volume envelope sustain (0 to 1)
	float EnvDecay; // length of the volume envelope decay (yes, I know it's called release) (0 to 1)
	float EnvPunch; // tilts the sustain envelope for more 'pop' (0 to 1)

	float LPFResonance;
	float LPFFreq;
	float LPFRamp;
	float HPFFreq;
	float HPFRamp;
	float BitCrush; // bit crush: resamples the audio at a lower frequency (0 to 1)
	float BitCrushSweep; // bit crush sweep: sweeps the Bit Crush filter up or down (-1 to 1)
	float CompressionAmount;

	float FlangerOffset;
	float FlangerRamp;

	float RepeatSpeed; // speed of the note repeating - certain variables are reset each time (0 to 1)

	float ArmRepeat;
	float ArmSpeed;
	float ArmMod;
	float ArmSpeed2;
	float ArmMod2;
} FXParams103;

//////////////////////////////////////////////////////////////////////////

typedef struct _RetroVoice103 {
	int version;
	int waveform_shape;
	float sound_vol;
	float morph_rate;
	FXParams103 fx_base_params;
	FXParams103 fx_morph_params;
	int length_in_samples;
} RetroVoice103;

//////////////////////////////////////////////////////////////////////////

struct BufferCallback {
	void operator()(float sample) { append_sample(sample); }
	virtual void append_sample(float sample) = 0;
};

class RetroSFXVoice {
	int wav_samples_rendered;

public:
	RetroVoice103 voice;
	FXParams103 fx_work_params;

	//////////////////////////////////////////////////////////////////////////

	float master_vol;
	bool playing_sample;

	int phase;
	double period;
	double maxperiod;
	double slide;
	double dslide;
	int iperiod;
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
	float noise_buffer[32], pink_noise_buffer[32];
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

	//////////////////////////////////////////////////////////////////////////

	void ResetParams();
	int ReadData(void *dest, int size, int units, unsigned char *&data);
	bool LoadSettings(unsigned char *data);
	bool LoadSettings(const char *filename);
	bool SaveSettings(const char *filename);
	bool CompareSettings(RetroSFXVoice *other);
	void Reset(bool restart);
	void Play(void *data = nullptr);
	void Play(bool calculate_length);
	int GetVoiceLengthInSamples() const;
	int Render(int nSamples, BufferCallback *callback);
	float GenNoise() const;
	float GenPinkNoise() const;

	void Mutate();
	void Randomize();

	void Morph(float &morph_var, float morph_dest);
	bool ExportWav(const char *filename, int wav_bits = 16, int wav_freq = 44100);

	bool IsActive();

	RetroSFXVoice();
};

//////////////////////////////////////////////////////////////////////////
