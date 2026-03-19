#ifndef __MEMORY__
#define __MEMORY__

#ifdef _DEBUG
  void *GetMemory(long size);
  void FinalReport();
  void FreeMemory(void *p);
#else
  #define GetMemory(x) malloc((x))
  #define FreeMemory(x) free((x))
#endif

#endif // __MEMORY__
