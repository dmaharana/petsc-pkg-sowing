#ifndef lint
static char vcid1[] = "$Id: lftcomm.c,v 1.1.1.1 2000-01-05 16:39:15 gropp Exp $";
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
#include "comm/comm.h"

#if defined(FORTRANCAPS)
#define pinumtids_      PINUMTIDS
#define pimytid_        PIMYTID
#define pidiameter_     PIDIAMETER
#define synull_         SYNULL

#elif !defined(FORTRANUNDERSCORE)
#define pinumtids_      pinumtids
#define pimytid_        pimytid
#define pidiameter_     pidiameter
#define synull_         synull
#endif

void msgallocsend_(a,b,c)
int *a,*b,*c;
{
SETERRC(1,"MSGALLOCSEND has no Fortran counterpart");
}
void msgallocrecv_(a,b,c)
{
SETERRC(1,"MSGALLOCRECV has no Fortran counterpart");
}
int pinumtids_()
{
  return PInumtids;
}
int pimytid_()
{
  return PImytid;
}
int pidiameter_()
{
  return PIdiameter;
}
#ifndef _HAS_SYNULL
#define _HAS_SYNULL
void synull_()
{
}
#endif
