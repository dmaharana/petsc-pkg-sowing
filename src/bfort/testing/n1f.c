/* n1.c */
/* Fortran interface file */

/*
* This file was generated automatically by bfort from the C source
* file.  
 */
#include "mpi.h"
#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define foobar_ PFOOBAR
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define foobar_ pfoobar
#else
#define foobar_ pfoobar_
#endif
#else
#ifdef FORTRANCAPS
#define foobar_ FOOBAR
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define foobar_ foobar
#endif
#endif



/* Definitions of Fortran Wrapper routines */
#if defined(__cplusplus)
extern "C" {
#endif
void  foobar_(int *a,double *b, int *__ierr ){
foobar(*a,*b);
}
#if defined(__cplusplus)
}
#endif
