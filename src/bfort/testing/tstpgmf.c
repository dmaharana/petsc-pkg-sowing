/* tstpgm.c */
/* Fortran interface file */

/*
* This file was generated automatically by bfort from the C source
* file.  
 */

#ifdef HAVE_64BIGS
#if defined(__cplusplus)
extern "C" { 
#endif 
extern void *PetscToPointer(int);
extern int PetscFromPointer(void *);
extern void PetscRmPointer(int);
#if defined(__cplusplus)
} 
#endif 

#else

#define PetscToPointer(a) (a)
#define PetscFromPointer(a) (int)(a)
#define PetscRmPointer(a)
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef HAVE_FORTRAN_CAPS
#define foo1_ PFOO1
#elif !defined(HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define foo1_ pfoo1
#else
#define foo1_ pfoo1_
#endif
#else
#ifdef HAVE_FORTRAN_CAPS
#define foo1_ FOO1
#elif !defined(HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define foo1_ foo1
#endif
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef HAVE_FORTRAN_CAPS
#define foo2_ PFOO2
#elif !defined(HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define foo2_ pfoo2
#else
#define foo2_ pfoo2_
#endif
#else
#ifdef HAVE_FORTRAN_CAPS
#define foo2_ FOO2
#elif !defined(HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define foo2_ foo2
#endif
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef HAVE_FORTRAN_CAPS
#define foo3_ PFOO3
#elif !defined(HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define foo3_ pfoo3
#else
#define foo3_ pfoo3_
#endif
#else
#ifdef HAVE_FORTRAN_CAPS
#define foo3_ FOO3
#elif !defined(HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define foo3_ foo3
#endif
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef HAVE_FORTRAN_CAPS
#define sygetarchtype_ PSYGETARCHTYPE
#elif !defined(HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define sygetarchtype_ psygetarchtype
#else
#define sygetarchtype_ psygetarchtype_
#endif
#else
#ifdef HAVE_FORTRAN_CAPS
#define sygetarchtype_ SYGETARCHTYPE
#elif !defined(HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define sygetarchtype_ sygetarchtype
#endif
#endif



/* Definitions of Fortran Wrapper routines */
#if defined(__cplusplus)
extern "C" {
#endif
void  foo1_(a *,b, *c, int *__ierr ){
foo1(*,,*c);
}
int foo2_(a *,b){
*__ierr = foo2(*,);
}
void foo3_(int *__ierr ){
*__ierr = foo3();
}
void  sygetarchtype_(str *,s){
SYGetArchType(*,);
}
#if defined(__cplusplus)
}
#endif
