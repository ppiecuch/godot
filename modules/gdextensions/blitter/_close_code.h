//  This file reverses the effects of begin_code.h and should be included
//after you finish any function and structure declarations in your headers

#ifndef _begin_code_h
#error close_code.h included without matching begin_code.h
#endif
#undef _begin_code_h

/* Reset structure packing at previous byte alignment */
#if defined(_MSC_VER) || defined(__MWERKS__) || defined(__BORLANDC__)
#ifdef __BORLANDC__
#pragma nopackwarning
#endif
#pragma pack(pop)
#endif /* Compiler needs structure packing set */
