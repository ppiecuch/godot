/* FluidSynth - A Software Synthesizer
*
* Copyright (C) 2003  Peter Hanappe and others.
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation; either version 2.1 of
* the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free
* Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301, USA
*/

/* fluid_godot.cpp
*
* Audio driver, outputs to the GodotEngine audio system
*
*/

#if GODOT_FLUIDDRIVER_ENABLED

#include "fluid_sys.h"
#include "fluid_adriver.h"
#include "fluid_settings.h"

#include "core/io/resource_importer.h"
#include "core/io/resource_saver.h"
#include "core/math/math_funcs.h"
#include "servers/audio/audio_stream.h"

class AudioStreamFluid : public AudioStream {
	GDCLASS(AudioStreamFluid, AudioStream);

	friend class AudioStreamPlaybackFluid;

public:
	AudioStreamFluid();
	~AudioStreamFluid();
};

class AudioStreamPlaybackFluid : public AudioStreamPlayback {
	GDCLASS(AudioStreamPlaybackFluid, AudioStreamPlayback);

	friend class ResourceImporterFluid;

private:
	bool active;
	Ref<AudioStreamFluid> fluid_stream;

	int loops;
	int sample_position;

protected:
	virtual void mix(AudioFrame *p_buffer, float p_rate_scale, int p_frames);

public:
	virtual void start(float p_from_pos = 0);
	virtual void stop();
	virtual bool is_playing() const;
	virtual int get_loop_count() const; // times it looped
	virtual float get_playback_position() const;
	virtual void seek(float p_time);
	virtual float get_length() const;

	AudioStreamPlaybackFluid();
};

class ResourceImporterFluid : public ResourceImporter {
	GDCLASS(ResourceImporterFluid, ResourceImporter);

public:
	virtual String get_importer_name() const;
	virtual String get_visible_name() const;
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual String get_save_extension() const;
	virtual String get_resource_type() const;

	virtual int get_preset_count() const;
	virtual String get_preset_name(int p_idx) const;

	virtual void get_import_options(List<ImportOption> *r_options, int p_preset = 0) const;
	virtual bool
	get_option_visibility(const String &p_option, const Map<StringName, Variant> &p_options) const;
	virtual Error import(const String &p_source_file, const String &p_save_path,
			const Map<StringName, Variant> &p_options,
			List<String> *r_platform_variants,
			List<String> *r_gen_files = nullptr,
			Variant *r_metadata = nullptr);

	ResourceImporterFluid() {}
	~ResourceImporterFluid() {}
};

typedef struct
{
	fluid_audio_driver_t driver;
	fluid_audio_func_t callback;
	void *data;
	fluid_file_renderer_t *renderer;
	int period_size;
	double sample_rate;
	fluid_timer_t *timer;
	unsigned int samples;
} fluid_godot_audio_driver_t;

static int fluid_godot_audio_run_s16(void *d, unsigned int msec);

/**************************************************************
*
*        GodotEngine audio driver
*
*/

fluid_audio_driver_t *
new_fluid_godot_audio_driver(fluid_settings_t *settings, fluid_synth_t *synth)
{
	fluid_godot_audio_driver_t *dev;
	int msec;

	dev = FLUID_NEW(fluid_godot_audio_driver_t);

	if(dev == NULL)
	{
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}

	FLUID_MEMSET(dev, 0, sizeof(fluid_godot_audio_driver_t));

	fluid_settings_getint(settings, "audio.period-size", &dev->period_size);
	fluid_settings_getnum(settings, "synth.sample-rate", &dev->sample_rate);

	dev->data = synth;
	dev->callback = (fluid_audio_func_t) fluid_synth_process;
	dev->samples = 0;

	dev->renderer = new_fluid_file_renderer(synth);

	if(dev->renderer == NULL)
	{
		goto error_recovery;
	}

	msec = (int)(0.5 + dev->period_size / dev->sample_rate * 1000.0);
	dev->timer = new_fluid_timer(msec, fluid_godot_audio_run_s16, (void *) dev, TRUE, FALSE, TRUE);

	if(dev->timer == NULL)
	{
		FLUID_LOG(FLUID_PANIC, "Couldn't create the audio thread.");
		goto error_recovery;
	}

	return (fluid_audio_driver_t *) dev;

error_recovery:
	delete_fluid_godot_audio_driver((fluid_audio_driver_t *) dev);
	return NULL;
}

void delete_fluid_godot_audio_driver(fluid_audio_driver_t *p)
{
	fluid_godot_audio_driver_t *dev = (fluid_godot_audio_driver_t *) p;
	fluid_return_if_fail(dev != NULL);

	delete_fluid_timer(dev->timer);
	delete_fluid_file_renderer(dev->renderer);

	FLUID_FREE(dev);
}

static int fluid_godot_audio_run_s16(void *d, unsigned int clock_time)
{
	fluid_godot_audio_driver_t *dev = (fluid_godot_audio_driver_t *) d;
	unsigned int sample_time;

	sample_time = (unsigned int)(dev->samples / dev->sample_rate * 1000.0);

	if(sample_time > clock_time)
	{
		return 1;
	}

	dev->samples += dev->period_size;

	return fluid_file_renderer_process_block(dev->renderer) == FLUID_OK ? 1 : 0;
}

#endif /* GODOT_FLUIDDRIVER_ENABLED */
