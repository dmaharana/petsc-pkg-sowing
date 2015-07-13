/* tstpgma.c */
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
#define simple_ PSIMPLE
#elif !defined(HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define simple_ psimple
#else
#define simple_ psimple_
#endif
#else
#ifdef HAVE_FORTRAN_CAPS
#define simple_ SIMPLE
#elif !defined(HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define simple_ simple
#endif
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
#define foo1a_ PFOO1A
#elif !defined(HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define foo1a_ pfoo1a
#else
#define foo1a_ pfoo1a_
#endif
#else
#ifdef HAVE_FORTRAN_CAPS
#define foo1a_ FOO1A
#elif !defined(HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define foo1a_ foo1a
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
#define foo3a_ PFOO3A
#elif !defined(HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define foo3a_ pfoo3a
#else
#define foo3a_ pfoo3a_
#endif
#else
#ifdef HAVE_FORTRAN_CAPS
#define foo3a_ FOO3A
#elif !defined(HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define foo3a_ foo3a
#endif
#endif



/* Definitions of Fortran Wrapper routines */
#if defined(__cplusplus)
extern "C" {
#endif
void simple_(int *a,double *b, int *__ierr ){
*__ierr = simple(*a,*b);
}
void  foo1_(int *a,char *b,void(*c)(), int *__ierr ){
foo1(*a,*b,c);
}
void  foo1a_(int *a,char *b,void(*c)(int, char, MyType *a), int *__ierr ){
foo1a(*a,*b,c);
}
int foo2_(MyType *a,short *b, int *__ierr ){
*__ierr = foo2(
	(MyType* )PetscToPointer( *(int*)(a) ),*b);
}
void foo3_(int *__ierr ){
*__ierr = foo3();
}
void foo3a_(int *__ierr ){
*__ierr = foo3a();
}
#if defined(__cplusplus)
}
#endif
