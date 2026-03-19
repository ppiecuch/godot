#ifndef __SCRIPTLIB__
#define __SCRIPTLIB__

#include "cmdlib.h"

#define MAXTOKEN 1024

extern char token[MAXTOKEN];
extern char *scriptbuffer,*script_p,*scriptend_p;
extern int grabbed;
extern int scriptline;
extern qboolean endofscript;

extern char brush_info[2000];
void MarkBrushBegin();

void LoadScriptFile (char *filename);
void ParseFromMemory (char *buffer, int size);

qboolean GetToken (qboolean crossline);
void UnGetToken (void);
qboolean TokenAvailable (void);

#endif // __SCRIPTLIB__
