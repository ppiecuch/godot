#ifndef UTILS_H
#define UTILS_H

#include "core/math_funcs.h"

#define min(a,b) MIN(a,b)
#define max(a,b) MAX(a,b)
#define decnz(x)  if ((x)>0) { (x)--; }
#define inctop(x,y) if ((x)<(y)) { (x)++; }
#define iff(cond,a,b)  ((cond) ? (a):(b))
#define sqr(x)   ((x)*(x))
#define cube(x)  ((x)*(x)*(x))

_FORCE_INLINE_ void swapint (int & a, int & b) { int t=a; a=b; b=t; }
_FORCE_INLINE_ void swapuint (unsigned int & a, unsigned int & b) { unsigned int t=a; a=b; b=t; }
_FORCE_INLINE_ void swapshort (short &a, short &b) { short t=a; a=b; b=t; }
_FORCE_INLINE_ void swapushort (unsigned short &a, unsigned short &b) {unsigned short t=a; a=b; b=t; }
_FORCE_INLINE_ void swapbyte ( char &a, char & b) { char t=a;a=b;b=t; }
_FORCE_INLINE_ void swapubyte (unsigned char &a,unsigned char & b) { unsigned char t=a; a=b; b=t; }
_FORCE_INLINE_ void swapmem( void *a, void *b, int c) { int t; for (t=0;  t<c; t++) { swapbyte(((char *)a)[t],((char *)b)[t]); } }

#endif // UTILS_H
