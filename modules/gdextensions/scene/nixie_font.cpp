#include "nixie_font.h"
#include "nixie_font_res.h"

#define MAKE_ABGR(r, g, b, a) ((uint32_t) (((uint32_t)(a) << 24) | ((uint32_t)(b) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(r)))
#define MAKE_RGBA(r, g, b, a) ((uint32_t) (((uint32_t)(r) << 24) | ((uint32_t)(g) << 16) | ((uint32_t)(b) << 8) | (uint32_t)(a)))

static PoolVector<Rect2> _build_tiles(Size2 grid_size, unsigned int total_frames, Point2 tex_origin=Point2(0,0), Size2 tex_size=Size2(1,1)) {

	PoolVector<Rect2> frames;
	for(int fr = 0; fr < grid_size.height /* rows */; fr++) {
		for(int fc = 0; fc < grid_size.width /* cols */; fc++) {
			const real_t cw = tex_size.width / grid_size.width, ch = tex_size.height / grid_size.height;
			frames.push_back(Rect2(tex_origin.x + fc * cw, tex_origin.y + fr * ch, cw, ch));
			if (frames.size() == total_frames) break;
		}
	}
	return frames;
}

enum LightColorMask {

    Mask00 = MAKE_ABGR(255, 255, 255, 00),
    Mask05 = MAKE_ABGR(255, 255, 255, 12),
    Mask10 = MAKE_ABGR(255, 255, 255, 24),
    Mask15 = MAKE_ABGR(255, 255, 255, 36),
    Mask20 = MAKE_ABGR(255, 255, 255, 48),
    Mask25 = MAKE_ABGR(255, 255, 255, 61),
    Mask30 = MAKE_ABGR(255, 255, 255, 73),
    Mask35 = MAKE_ABGR(255, 255, 255, 85),
    Mask40 = MAKE_ABGR(255, 255, 255, 97),
    Mask45 = MAKE_ABGR(255, 255, 255, 110),
    Mask50 = MAKE_ABGR(255, 255, 255, 122),
    Mask55 = MAKE_ABGR(255, 255, 255, 134),
    Mask60 = MAKE_ABGR(255, 255, 255, 146),
    Mask65 = MAKE_ABGR(255, 255, 255, 158),
    Mask70 = MAKE_ABGR(255, 255, 255, 170),
    Mask75 = MAKE_ABGR(255, 255, 255, 182),
    Mask80 = MAKE_ABGR(255, 255, 255, 194),
    Mask85 = MAKE_ABGR(255, 255, 255, 207),
    Mask90 = MAKE_ABGR(255, 255, 255, 219),
    Mask95 = MAKE_ABGR(255, 255, 255, 233),
    Mask100 = MAKE_ABGR(255, 255, 255, 255),
};

const int NixieFontCols = 8; // rows and cols
const int NixieFontRows = 12; // in font sheet
const int NixieFontChars = NixieFontCols * NixieFontRows; // might be less than AnimRows*AnimCols

const struct {

    real_t t; // timing
    uint32_t c; // color
} LightBlinkingPattern[] = {

    { 99, Mask10 }, // 10%
    { 01, Mask50 }, // 50%
    { 8, Mask80 }, // 80%
    { 02, Mask70 }, // 70%
    { 02, Mask20 }, // 20%
    { 01, Mask15 }, // 15%
    { 01, Mask70 }, // 70%
    { 02, Mask80 }, // 80%
    { 01, Mask75 }, // 75%
    { 01, Mask80 }, // 80%
    { 01, Mask15 }, // 15%
    { 01, Mask60 }, // 60%
    { 01, Mask65 }, // 65%
    { 01, Mask50 }, // 50%
    { 02, Mask55 }, // 121-122
    { 02, Mask60 }, // 123-124
    { 02, Mask55 }, // 125-126
    { 02, Mask60 }, // 127-128
    { 01, Mask40 }, // 129
    { 02, Mask45 }, // 130-131
    { 01, Mask20 }, // 132
    { 11, Mask30 }, // 133-143
    { 01, Mask20 }, // 144
    { 02, Mask05 }, // 145-146
    { 04, Mask30 }, // 147-150
    { 02, Mask20 }, // 151-152
    { 03, Mask25 }, // 153-155
    { 01, Mask15 }, // 156
    { 04, Mask25 }, // 157-160
    { 01, Mask15 }, // 161
    { 12, Mask25 }, // 162-173
    { 01, Mask15 }, // 174
    { 02, Mask05 }, // 175-176
    { 02, Mask20 }, // 177-178
    { 13, Mask35 }, // 179-191
    { 01, Mask25 }, // 192
    { 02, Mask15 }, // 193-194
    { 01, Mask45 }, // 195
    { 07, Mask65 }, // 196-202
    { 01, Mask25 }, // 203
    { 01, Mask10 }, // 204
    { 05, Mask75 }, // 205-210
    { 02, Mask15 }, // 211-212
    { 9, Mask65 }, // 213-221
    { 01, Mask15 }, // 222
    { 05, Mask25 }, // 223-227
    { 01, Mask15 }, // 228
    { 02, Mask20 }, // 229-230
    { 10, Mask65 }, // 231-240
    { 20, Mask75 }, // 241-260
    { 01, Mask25 }, // 261
    { 05, Mask75 }, // 262-266
    { 04, Mask85 }, // 267-269
    { 01, Mask25 }, // 270
    { 16, Mask75 }, // 271-284
    { 01, Mask25 }, // 285
    { 01, Mask35 }, // 286
    { 01, Mask75 }, // 287
    { 12, Mask85 }, // 288-298
    { 01, Mask35 }, // 299
    { 01, Mask45 }, // 300
    { 03, Mask75 }, // 301-303
    { 01, Mask55 }, // 304
    { 8, Mask85 }, // 305-311
    { 03, Mask25 }, // 312-314
    { 03, Mask85 }, // 315
    { 03, Mask85 }, // 315-317
    { 600, Mask100 }, // 318-417
    { 03, Mask25 }, // 418-420
    { -1, Mask100 }
};

void NixieFont::_draw() {

#if 0
    int cnt = 0; while(*msg) {
        const char ch = *msg++ - 32;
        const CGRect *coords = &m_texture.anim.frames[ch].coords;
        const CGRectangle *dimensions = &m_texture.anim.frames[ch].dimension;
        const int w = dimensions->size.width;
        const int h = dimensions->size.height;
        if (!ch) {
            x += w-1; continue; // space
        }
        glColor4ubv((GLubyte*)&kLightBlinkingPattern[_control[cnt].phase].c);        
        if (!blitQuad2D(x, y, w, h, dimensions))
            drawGlRect(w, h, coords, x, y);
        if (_control[cnt].alt_age > 0)
            --_control[cnt].alt_age;
        else {
            // display "broken" tube effect for 2 frames
            const char ch = _control[cnt].alt?_control[cnt].alt:0x10 + IRAND%10;
            const CGRect *coords = &m_texture.anim.frames[ch].coords;
            const CGRectangle *dimensions = &m_texture.anim.frames[ch].dimension;
            const int w = dimensions->size.width;
            const int h = dimensions->size.height;
            const uint32_t color = 0xffffff00 + (kLightBlinkingPattern[_control[cnt].phase].c&0xff)>>2;
            glColor4ubv((GLubyte*)&color); if (!blitQuad2D(x, y, w, h, dimensions))
                drawGlRect(w, h, coords, x, y);
            switch(_control[cnt].alt_age--) {
                case 0:
                    _control[cnt].alt = ch;
                    break;
                case -4:
                    _control[cnt].alt = 0;
                    _control[cnt].alt_age = 0x80 + (IRAND&0xff)<<1; // schedule next event
                    break;
            }
        }
        x += w-1;
        
        _control[cnt].age++; if (_control[cnt].age > kLightBlinkingPattern[_control[cnt].phase].t) {
            _control[cnt].phase++; if (kLightBlinkingPattern[_control[cnt].phase].t == -1) _control[cnt].phase = 0;
            _control[cnt].age = 0;    
        }
        
        ++cnt;
    }
#endif
}

void NixieFont::set_text(String p_text) {

	if (draw_text != p_text) {

		_control.resize(p_text.size());
		auto w = _control.write();
		for(int i = 0; i < _control.size(); ++i) {
			w[i].age = -(Math::rand() & 0x7f); // random initial delay
			w[i].alt_age = Math::rand() & 0x7f;
		}

		draw_text = p_text;
	}
}

NixieFont::NixieFont() {

	_age = 0;
	_control = PoolVector<CharControl>();
	_frames = _build_tiles(Size2(NixieFontCols, NixieFontRows), NixieFontChars);
}

NixieFont::~NixieFont() {

}
