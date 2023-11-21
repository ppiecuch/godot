#ifndef _akima_h_
#define _akima_h_

#include <stdbool.h>

/*
      .. Scalar Arguments ..
      INTEGER          IER,MD,NDP,NXI,NYI,NEAR(NDP),NEXT(NDP)
      ..
      .. Array Arguments ..
      DOUBLE PRECISION WK(NDP,17),XD(NDP),XI(NXI),YD(NDP),
                       YI(NYI),ZD(NDP),ZI(NXI,NYI),DIST(NDP)
      INTEGER          IWK(NDP,25)
      LOGICAL          EXTRPI(NXI,NYI)
*/

#ifdef __cplusplus
extern "C"
#endif
    void sdsf3p_(int *MD, int *NDP, double *XD, double *YD, double *ZD, int *NXI,
                 double *XI, int *NYI, double *YI, double *ZI, int *IER, double *WK,
                 int *IWK, bool *EXTRPI, int *NEAR, int *NEXT, double *DIST);

#endif // _akima_h_
