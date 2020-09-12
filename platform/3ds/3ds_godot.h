#ifndef GODOT_3DS_H
#define GODOT_3DS_H

extern "C" {
# define Thread Thread3DS
# include <3ds.h>
# include <tex3ds.h>
# undef Thread
}

#endif // GODOT_3DS_H
