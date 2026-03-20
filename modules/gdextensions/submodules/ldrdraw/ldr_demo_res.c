/* Embedded LDraw demo models for editor/tools builds */

#ifdef TOOLS_ENABLED

#undef INCBIN_PREFIX
#define INCBIN_PREFIX
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_SILENCE_BITCODE_WARNING
#include "misc/incbin.h"

INCBIN(ldr_demo_car, "submodules/ldrdraw/demo/Car.dat");
INCBIN(ldr_demo_ship, "submodules/ldrdraw/demo/Ship.mpd");
INCBIN(ldr_demo_viper, "submodules/ldrdraw/demo/Viper.mpd");
INCBIN(ldr_demo_millennium_falcon, "submodules/ldrdraw/demo/4488 - Millennium Falcon - Mini.mpd");
INCBIN(ldr_demo_at_at, "submodules/ldrdraw/demo/4489 - AT-AT - Mini.mpd");
INCBIN(ldr_demo_at_st, "submodules/ldrdraw/demo/30054 - AT-ST - Mini.mpd");
INCBIN(ldr_demo_vulture_droid, "submodules/ldrdraw/demo/30055 - Vulture Droid - Mini.mpd");
INCBIN(ldr_demo_attack_cruiser, "submodules/ldrdraw/demo/30053 - Republic Attack Cruiser - Mini.mpd");

#endif /* TOOLS_ENABLED */
