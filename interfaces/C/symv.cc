#include <algorithm>
#include <interfaces/C/config.h>
#include <interfaces/C/xerbla.h>
#include <ulmblas/level1/copy.h>
#include <ulmblas/level1extensions/gecopy.h>
#include <ulmblas/level2/sylmv.h>

//#define SCATTER

#ifdef SCATTER
#define   SCATTER_INCROWA   2
#define   SCATTER_INCCOLA   3
#define   SCATTER_INCX      4
#define   SCATTER_INCY      5
#endif


extern "C" {

void
ULMBLAS(dsymv)(enum UpLo     upLo,
               int           n,
               double        alpha,
               const double  *A,
               int           ldA,
               const double  *x,
               int           incX,
               double        beta,
               double        *y,
               int           incY)
{

//
//  Test the input parameters
//
    int info = 0;
    if (upLo!=Upper && upLo!=Lower) {
        info = 1;
    } else if (n<0) {
        info = 2;
    } else if (ldA<std::max(1,n)) {
        info = 5;
    } else if (incX==0) {
        info = 7;
    } else if (incY==0) {
        info = 10;
    }

    if (info!=0) {
        ULMBLAS(xerbla)("DSYMV ", &info);
    }

#ifndef SCATTER
//
//  Start the operations.
//
    if (incX<0) {
        x -= incX*(n-1);
    }
    if (incY<0) {
        y -= incY*(n-1);
    }
    if (upLo==Lower) {
        ulmBLAS::sylmv(n, alpha, A, 1, ldA, x, incX, beta, y, incY);
    } else {
        ulmBLAS::sylmv(n, alpha, A, ldA, 1, x, incX, beta, y, incY);
    }
#else
//
//  Scatter operands
//
    double *_A = new double[ldA*n*SCATTER_INCROWA*SCATTER_INCCOLA];
    double *_x = new double[n*incX*SCATTER_INCX];
    double *_y = new double[n*incY*SCATTER_INCY];

    ulmBLAS::gecopy(n, n,
                    A, 1, ldA,
                    _A, SCATTER_INCROWA, ldA*SCATTER_INCCOLA);
    ulmBLAS::copy(n, x, incX, _x, incX*SCATTER_INCX);
    ulmBLAS::copy(n, y, incY, _y, incY*SCATTER_INCY);

//
//      Start the operations.
//
    if (upLo==Lower) {
        ulmBLAS::sylmv(n, alpha,
                       _A, SCATTER_INCROWA, ldA*SCATTER_INCCOLA,
                       _x, incX*SCATTER_INCX,
                       beta,
                       _y, incY*SCATTER_INCY);
    } else {
        ulmBLAS::sylmv(n, alpha,
                       _A, ldA*SCATTER_INCCOLA, SCATTER_INCROWA,
                       _x, incX*SCATTER_INCX,
                       beta,
                       _y, incY*SCATTER_INCY);
    }
    ulmBLAS::copy(n, _y, incY*SCATTER_INCY, y, incY);

//
//      Gather result
//
    delete [] _A;
    delete [] _x;
    delete [] _y;
#endif
}

} // extern "C"