/* n2.c */
/* Fortran interface file */

/*
* This file was generated automatically by bfort from the C source
* file.  
 */
#include "mpi.h"

#ifdef POINTER_64_BITS
#if defined(__cplusplus)
extern "C" { 
#endif 
extern void *__ToPointer(int);
extern int __FromPointer(void *);
extern void __RmPointer(int);
#if defined(__cplusplus)
} 
#endif 

#else

#define __ToPointer(a) (a)
#define __FromPointer(a) (int)(a)
#define __RmPointer(a)
#endif

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

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define barfoo_ PBARFOO
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define barfoo_ pbarfoo
#else
#define barfoo_ pbarfoo_
#endif
#else
#ifdef FORTRANCAPS
#define barfoo_ BARFOO
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define barfoo_ barfoo
#endif
#endif



/* Definitions of Fortran Wrapper routines */
#if defined(__cplusplus)
extern "C" {
#endif
void foobar_(MPI_File f,int *a,double *b, int *__ierr ){
*__ierr = foobar(
	(MPI_File)__ToPointer( *(int*)(f) ),*a,*b);
}
void barfoo_( int f[], int *a, int *b, int *__ierr ){
*__ierr = barfoo(f,a,*b);
}
#if defined(__cplusplus)
}
#endif
