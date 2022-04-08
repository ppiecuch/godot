#ifndef HAVE_PARTICLES_SIZE_S

const unsigned char *particles_size_s = 0;

#else

#undef INCBIN_PREFIX
#define INCBIN_PREFIX
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_SILENCE_BITCODE_WARNING

#include "misc/incbin.h"

#define RES_ROOT "submodules/environment/waterfall"

INCBIN(w1_s, RES_ROOT "/particles/S/w1.png");
INCBIN(w2_s, RES_ROOT "/particles/S/w2.png");
INCBIN(w3_s, RES_ROOT "/particles/S/w3.png");
INCBIN(w4_s, RES_ROOT "/particles/S/w4.png");
INCBIN(c1_s, RES_ROOT "/particles/S/c1.png");
INCBIN(c2_s, RES_ROOT "/particles/S/c2.png");

const unsigned char *particles_size_s[] = {
	w1_s_data, w2_s_data, w3_s_data, w4_s_data,
	c1_s_data, c1_s_data,
};

#endif // HAVE_PARTICLES_SIZE_S
