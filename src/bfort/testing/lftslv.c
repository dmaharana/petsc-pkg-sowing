#ifndef lint
static char vcid2[] = "$Id: lftslv.c,v 1.1.1.1 2000-01-05 16:39:15 gropp Exp $";
#endif

#ifndef DEBUG_ALL
#define DEBUG_ALL
#endif

/* 
   This file contains those routines whose translation into Fortran
   could not be handled by the automatic translator.  Some of them
   are not supported in Fortran.
 */
#include "tools.h"
#include "solvers/svctx.h"

#if defined(FORTRANCAPS)
#define svgeticcalpha_ SVGETICCALPHA
#define svgetflops_    SVGETFLOPS
#define spcreate_      SPCREATE
#define svsetmonitor_  SVSETMONITOR
#define svsetconvergencetest_ SVSETCONVERGENCETEST
#define cvttovoid_     CVTTOVOID
#define spcompress_    SPCOMPRESS
#define spcompressifwasteful_ SPCOMPRESSIFWASTEFUL
#define itdefaultmonitor_ ITDEFAULTMONITOR
#define synull_           SYNULL

#elif !defined(FORTRANUNDERSCORE)
#define svgeticcalpha_ svgeticcalpha
#define svgetflops_    svgetflops
#define spcreate_      spcreate
#define svsetmonitor_  svsetmonitor
#define svsetconvergencetest_ svsetconvergencetest
#define cvttovoid_     cvttovoid
#define spcompress_    spcompress
#define spcompressifwasteful_ spcompressifwasteful
#define itdefaultmonitor_ itdefaultmonitor
#define synull_           synull
#endif

void svgeticcalpha_(C,a)
SVctx **C;
int   *a;
{
int b;
SVGetICCAlpha(*C,b);
*a = b;
}
void svgetflops_( C, a ) 
SVctx **C;
int   *a;
{
int b;
SVGetFlops(*C,b);
*a = b;
}
void svgetmemory_( C, a ) 
SVctx **C;
int   *a;
{
int b;
SVGetMemory(*C,b);
*a = b;
}

/* These need to stash the user function away and call a special stub routine 
 */
static struct { int (*converge)(); } LocalConverge;
int SViFtoCConverge( itP, it, res )
void *itP;
int  it;
double res;
{
int itp     = it;
double resp = res;
return (*LocalConverge.converge)( itP, &itp, &resp );
}
void svsetconvergencetest_( C, a, b )
SVctx **C;
int   (*a)();
void   *b;
{
LocalConverge.converge = a;
SVSetConvergenceTest( *C, SViFtoCConverge, (void *)&LocalConverge );
}

static struct { void (*monitor)(); } LocalMonitor;
void SViFtoCMonitor( itP, it, res )
void *itP;
int  it;
double res;
{
int itp     = it;
double resp = res;
(*LocalMonitor.monitor)( itP, &itp, &resp );
}
void svsetmonitor_( C, a, b )
SVctx **C;
void  (*a)(), *b;
{
/* Handle the null pointer */
if ((int)(a) == 0) {
    SVSetMonitor( *C, (void (*)())0, (void *)0 );
    }
else {
    LocalMonitor.monitor = a;
    SVSetMonitor( *C, SViFtoCMonitor, (void *)&LocalMonitor );
    }
}

#include "sparse/spmat.h"
int spcreate_( N, M, NMAX )
int *N, *M, *NMAX;
{
return (int)SpCreate( *N, *M, *NMAX );
}

/* This routine takes an address and dereferences it once */
void *cvttovoid_( n )
int **n;
{
return (void *)**n;
}

/* Because these are passed the addresses of the SpMat pointers, we don't
   need to de-ref them */
int spcompressifwasteful_(AA, ratio)
SpMat  **AA;
double *ratio;
{
return SpCompressIfWasteful(AA,*ratio);
}
int spcompress_(AA, tol)
SpMat  **AA;
double *tol;
{
return SpCompress(AA,*tol);
}

void itdefaultmonitor_(itP,n,rnorm)
void   *itP;
int    n;
double rnorm;
{
  ITDefaultMonitor(itP,n,rnorm);
}
#ifndef _HAS_SYNULL
#define _HAS_SYNULL
void synull_()
{
}
#endif
