#ifndef lint
static char vcid[] = "$Id: leftover.c,v 1.1.1.1 2000-01-05 16:39:15 gropp Exp $";
#endif

/* 
   This file contains those routines whose translation into Fortran
   could not be handled by the automatic translator.  Some of them
   are not supported in Fortran.
 */
#include "tools.h"
#include "solvers/svctx.h"
/* this is an ugly hack but there is no good way. */
#if defined(intelnx) || defined(p4) || defined(pvm) || defined(picl) || \
    defined(eui)
#include "comm/comm.h"
#endif

#if !defined(FORTRANUNDERSCORE)
#define svgeticcalpha_ svgeticcalpha
#define svgetflops_    svgetflops
#define numnodes_      numnodes
#define myprocid_      myprocid
#define msgdiameter_   msgdiameter
#define spcreate_      spcreate
#define synull_        synull
#endif

void svgeticcalpha_(C,a)
SVctx **C;
int   *a;
{
int b;
SVGetICCAlpha(*C,b);
*a = b;
}
void svgetflops( C, a ) 
SVctx **C;
int   *a;
{
int b;
SVGetFlops(*C,b);
*a = b;
}

#if defined(__commh)

void msgallocsend_(a,b,c)
int *a,*b,*c;
{
SETERRC(1,"MSGALLOCSEND has no Fortran counterpart");
}
void msgallocrecv_(a,b,c)
{
SETERRC(1,"MSGALLOCRECV has no Fortran counterpart");
}
int numnodes_()
{
  return NUMNODES;
}
int myprocid_()
{
  return MYPROCID;
}
int msgdiameter_()
{
  return MSGDIAMETER;
}
#endif

#include "sparse/spmat.h"
int spcreate_( N, M, NMAX )
int *N, *M, *NMAX;
{
return (int)SpCreate( *N, *M, *NMAX );
}

#ifndef _HAS_SYNULL
#define _HAS_SYNULL
void synull_()
{
}
#endif
