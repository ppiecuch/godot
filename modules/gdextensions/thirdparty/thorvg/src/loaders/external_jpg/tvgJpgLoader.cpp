/*
 * Copyright (c) 2021 - 2024 the ThorVG project. All rights reserved.

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// Patched for Godot: uses jpgd (thirdparty/jpeg-compressor) instead of turbojpeg.
// jpgd is a public-domain single-file JPEG decoder already bundled with Godot.

#include <memory.h>
#include <stdlib.h>
#include "jpgd.h"
#include "tvgJpgLoader.h"

/************************************************************************/
/* Internal Class Implementation                                        */
/************************************************************************/

void JpgLoader::clear()
{
    if (freeData) free(data);
    data = nullptr;
    size = 0;
    freeData = false;
}


/************************************************************************/
/* External Class Implementation                                        */
/************************************************************************/

JpgLoader::JpgLoader() : ImageLoader(FileType::Jpg)
{
}


JpgLoader::~JpgLoader()
{
    clear();
    free(surface.buf8);
}


bool JpgLoader::open(const string& path)
{
#ifdef THORVG_FILE_IO_SUPPORT
    auto jpegFile = fopen(path.c_str(), "rb");
    if (!jpegFile) return false;

    auto ret = false;

    //determine size
    if (fseek(jpegFile, 0, SEEK_END) < 0) goto finalize;
    if (((size = ftell(jpegFile)) < 1)) goto finalize;
    if (fseek(jpegFile, 0, SEEK_SET)) goto finalize;

    data = (unsigned char *) malloc(size);
    if (!data) goto finalize;

    freeData = true;

    if (fread(data, size, 1, jpegFile) < 1) goto failure;

    {
        int width, height, actual_comps;
        auto probe = jpgd::decompress_jpeg_image_from_memory(data, size, &width, &height, &actual_comps, 4);
        if (!probe) goto failure;
        free(probe);

        w = static_cast<float>(width);
        h = static_cast<float>(height);
        ret = true;
    }

    goto finalize;

failure:
    clear();

finalize:
    fclose(jpegFile);
    return ret;
#else
    return false;
#endif
}


bool JpgLoader::open(const char* data, uint32_t size, bool copy)
{
    int width, height, actual_comps;
    auto probe = jpgd::decompress_jpeg_image_from_memory((const unsigned char *)data, size, &width, &height, &actual_comps, 4);
    if (!probe) return false;
    free(probe);

    if (copy) {
        this->data = (unsigned char *) malloc(size);
        if (!this->data) return false;
        memcpy((unsigned char *)this->data, data, size);
        freeData = true;
    } else {
        this->data = (unsigned char *) data;
        freeData = false;
    }

    w = static_cast<float>(width);
    h = static_cast<float>(height);
    this->size = size;

    return true;
}


bool JpgLoader::read()
{
    if (!LoadModule::read()) return true;

    if (w == 0 || h == 0) return false;

    // Decompress using jpgd — always request 4 components (RGBX)
    int dw, dh, actual_comps;
    auto image = jpgd::decompress_jpeg_image_from_memory(data, size, &dw, &dh, &actual_comps, 4);
    if (!image) return false;

    // jpgd returns RGBX layout — convert to match expected color space
    if (ImageLoader::cs == ColorSpace::ARGB8888 || ImageLoader::cs == ColorSpace::ARGB8888S) {
        // Swap R<->B for BGRA layout
        auto px = (uint32_t *)image;
        for (int i = 0; i < dw * dh; i++) {
            uint32_t c = px[i];
            px[i] = (c & 0xFF00FF00) | ((c >> 16) & 0xFF) | ((c & 0xFF) << 16);
        }
        surface.cs = ColorSpace::ARGB8888;
    } else {
        surface.cs = ColorSpace::ABGR8888;
    }

    //setup the surface
    surface.buf8 = image;
    surface.stride = w;
    surface.w = w;
    surface.h = h;
    surface.channelSize = sizeof(uint32_t);
    surface.premultiplied = true;

    clear();
    return true;
}
