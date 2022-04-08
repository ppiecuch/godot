#ifndef HAVE_PARTICLES_SIZE_L

const unsigned char *particles_size_l = 0;

#else

#undef INCBIN_PREFIX
#define INCBIN_PREFIX
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_SILENCE_BITCODE_WARNING

#include "misc/incbin.h"

#define RES_ROOT "submodules/environment/waterfall"

INCBIN(w1_l, RES_ROOT "/particles/L/w1.png");
INCBIN(w2_l, RES_ROOT "/particles/L/w2.png");
INCBIN(w3_l, RES_ROOT "/particles/L/w3.png");
INCBIN(w4_l, RES_ROOT "/particles/L/w4.png");
INCBIN(c1_l, RES_ROOT "/particles/L/c1.png");
INCBIN(c2_l, RES_ROOT "/particles/L/c2.png");

const unsigned char *particles_size_l[] = {
	w1_l_data, w2_l_data, w3_l_data, w4_l_data,
	c1_l_data, c1_l_data,
};

#endif // HAVE_PARTICLES_SIZE_L
