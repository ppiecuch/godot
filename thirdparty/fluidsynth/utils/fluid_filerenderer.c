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

/*
 * Low-level routines for file output.
 */

#include "fluid_sys.h"
#include "fluid_synth.h"
#include "fluid_settings.h"

struct _fluid_file_renderer_t
{
    fluid_synth_t *synth;

    FILE *file;
    short *buf;

    int period_size;
    int buf_size;
};

void
fluid_file_renderer_settings(fluid_settings_t *settings)
{
    fluid_settings_register_str(settings, "audio.file.name", "fluidsynth.raw", 0);
    fluid_settings_register_str(settings, "audio.file.type", "raw", 0);
    fluid_settings_add_option(settings, "audio.file.type", "raw");
    fluid_settings_register_str(settings, "audio.file.format", "s16", 0);
    fluid_settings_add_option(settings, "audio.file.format", "s16");
    fluid_settings_register_str(settings, "audio.file.endian", "cpu", 0);
    fluid_settings_add_option(settings, "audio.file.endian", "cpu");
}

/**
 * Create a new file renderer and open the file.
 *
 * @param synth The synth that creates audio data.
 * @return the new object, or NULL on failure
 *
 * @note Available file types and formats depends on if libfluidsynth was
 * built with libsndfile support or not.  If not then only RAW 16 bit output is
 * supported.
 *
 * Uses the following settings from the synth object:
 *   - \ref settings_audio_file_name : Output filename
 *   - \ref settings_audio_file_type : File type, "auto" tries to determine type from filename
 *     extension with fallback to "wav".
 *   - \ref settings_audio_file_format : Audio format
 *   - \ref settings_audio_file_endian : Endian byte order, "auto" for file type's default byte order
 *   - \ref settings_audio_period-size : Size of audio blocks to process
 *   - \ref settings_synth_sample-rate : Sample rate to use
 *
 * @since 1.1.0
 */
fluid_file_renderer_t *
new_fluid_file_renderer(fluid_synth_t *synth)
{
    char *filename = NULL;
    fluid_file_renderer_t *dev;

    fluid_return_val_if_fail(synth != NULL, NULL);
    fluid_return_val_if_fail(synth->settings != NULL, NULL);

    dev = FLUID_NEW(fluid_file_renderer_t);

    if(dev == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(dev, 0, sizeof(fluid_file_renderer_t));

    dev->synth = synth;
    fluid_settings_getint(synth->settings, "audio.period-size", &dev->period_size);

    dev->buf_size = 2 * dev->period_size * sizeof(short);
    dev->buf = FLUID_ARRAY(short, 2 * dev->period_size);

    if(dev->buf == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        goto error_recovery;
    }

    fluid_settings_dupstr(synth->settings, "audio.file.name", &filename);

    if(filename == NULL)
    {
        FLUID_LOG(FLUID_ERR, "No file name specified");
        goto error_recovery;
    }

    dev->file = FLUID_FOPEN(filename, "wb");

    if(dev->file == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Failed to open the file '%s'", filename);
        goto error_recovery;
    }

    FLUID_FREE(filename);
    return dev;

error_recovery:

    FLUID_FREE(filename);
    delete_fluid_file_renderer(dev);
    return NULL;
}

/**
 * Set vbr encoding quality (only available with libsndfile support)
 * @param dev File renderer object.
 * @param q The encoding quality, see libsndfile documentation of \c SFC_SET_VBR_ENCODING_QUALITY
 * @return #FLUID_OK if the quality has been successfully set, #FLUID_FAILED otherwise
 * @since 1.1.7
 */
int
fluid_file_set_encoding_quality(fluid_file_renderer_t *dev, double q)
{
    return FLUID_FAILED;
}

/**
 * Close file and destroy a file renderer object.
 * @param dev File renderer object.
 * @since 1.1.0
 */
void delete_fluid_file_renderer(fluid_file_renderer_t *dev)
{
    fluid_return_if_fail(dev != NULL);

    if(dev->file != NULL)
    {
        fclose(dev->file);
    }

    FLUID_FREE(dev->buf);
    FLUID_FREE(dev);
}

/**
 * Write period_size samples to file.
 * @param dev File renderer instance
 * @return #FLUID_OK or #FLUID_FAILED if an error occurred
 * @since 1.1.0
 */
int
fluid_file_renderer_process_block(fluid_file_renderer_t *dev)
{
    size_t res, nmemb = dev->buf_size;

    fluid_synth_write_s16(dev->synth, dev->period_size, dev->buf, 0, 2, dev->buf, 1, 2);

    res = fwrite(dev->buf, 1, nmemb, dev->file);

    if(res < nmemb)
    {
        FLUID_LOG(FLUID_ERR, "Audio output file write error: %s",
                  strerror(errno));
        return FLUID_FAILED;
    }

    return FLUID_OK;
}
