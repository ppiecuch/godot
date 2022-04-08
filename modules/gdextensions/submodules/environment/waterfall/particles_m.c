#ifndef HAVE_PARTICLES_SIZE_M

const unsigned char *particles_size_m = 0;

#else

#undef INCBIN_PREFIX
#define INCBIN_PREFIX
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_SILENCE_BITCODE_WARNING

#include "misc/incbin.h"

#define RES_ROOT "submodules/environment/waterfall"

INCBIN(w1_m, RES_ROOT "/particles/M/w1.png");
INCBIN(w2_m, RES_ROOT "/particles/M/w2.png");
INCBIN(w3_m, RES_ROOT "/particles/M/w3.png");
INCBIN(w4_m, RES_ROOT "/particles/M/w4.png");
INCBIN(c1_m, RES_ROOT "/particles/M/c1.png");
INCBIN(c2_m, RES_ROOT "/particles/M/c2.png");

const unsigned char *particles_size_m[] = {
	w1_m_data, w2_m_data, w3_m_data, w4_m_data,
	c1_m_data, c1_m_data,
};

#endif // HAVE_PARTICLES_SIZE_M
