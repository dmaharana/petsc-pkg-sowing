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

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define barlong_ PBARLONG
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define barlong_ pbarlong
#else
#define barlong_ pbarlong_
#endif
#else
#ifdef FORTRANCAPS
#define barlong_ BARLONG
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define barlong_ barlong
#endif
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mixedargs_ PMIXEDARGS
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define mixedargs_ pmixedargs
#else
#define mixedargs_ pmixedargs_
#endif
#else
#ifdef FORTRANCAPS
#define mixedargs_ MIXEDARGS
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define mixedargs_ mixedargs
#endif
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define mixedargs2_ PMIXEDARGS2
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define mixedargs2_ pmixedargs2
#else
#define mixedargs2_ pmixedargs2_
#endif
#else
#ifdef FORTRANCAPS
#define mixedargs2_ MIXEDARGS2
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define mixedargs2_ mixedargs2
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
void barlong_( int aLongArrayName[], int *alongArgumentName,
      double *anotherLongArgumentName,int *thisIsAnotherLongName, int *__ierr ){
*__ierr = barlong(aLongArrayName,alongArgumentName,anotherLongArgumentName,thisIsAnotherLongName);
}
double  mixedargs_( int *M, int *m,void*p,void* c, int *__ierr ){
*__ierr = mixedargs(*M,*m,p,c);
}
double  mixedargs2_(int *Mlonger, int *M,float *m1, int *m,void*p,
     void* c, int *__ierr ){
*__ierr = mixedargs2(*Mlonger,*M,*m1,*m,p,c);
}
#if defined(__cplusplus)
}
#endif
