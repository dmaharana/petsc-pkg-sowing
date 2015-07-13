/* tstpgma2.c */
/* Fortran interface file */

/*
* This file was generated automatically by bfort from the C source
* file.  
 */

#ifdef PETSC_USE_POINTER_CONVERSION
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

#ifdef PETSC_HAVE_FORTRAN_CAPS
#define simple_ SIMPLE
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define simple_ simple
#endif
#ifdef PETSC_HAVE_FORTRAN_CAPS
#define foo1_ FOO1
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define foo1_ foo1
#endif
#ifdef PETSC_HAVE_FORTRAN_CAPS
#define foo1a_ FOO1A
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define foo1a_ foo1a
#endif
#ifdef PETSC_HAVE_FORTRAN_CAPS
#define foo2_ FOO2
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define foo2_ foo2
#endif
#ifdef PETSC_HAVE_FORTRAN_CAPS
#define foo3_ FOO3
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define foo3_ foo3
#endif
#ifdef PETSC_HAVE_FORTRAN_CAPS
#define foo3a_ FOO3A
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define foo3a_ foo3a
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
