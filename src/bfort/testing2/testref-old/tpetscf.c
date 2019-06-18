/* tpetsc.c */
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

#include "petscksp.h"
#ifdef MPI_BUILD_PROFILING
#ifdef PETSC_HAVE_FORTRAN_CAPS
#define kspsetnormtype_ PKSPSETNORMTYPE
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define kspsetnormtype_ pkspsetnormtype
#else
#define kspsetnormtype_ pkspsetnormtype_
#endif
#else
#ifdef PETSC_HAVE_FORTRAN_CAPS
#define kspsetnormtype_ KSPSETNORMTYPE
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define kspsetnormtype_ kspsetnormtype
#endif
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef PETSC_HAVE_FORTRAN_CAPS
#define kspsetchecknormiteration_ PKSPSETCHECKNORMITERATION
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define kspsetchecknormiteration_ pkspsetchecknormiteration
#else
#define kspsetchecknormiteration_ pkspsetchecknormiteration_
#endif
#else
#ifdef PETSC_HAVE_FORTRAN_CAPS
#define kspsetchecknormiteration_ KSPSETCHECKNORMITERATION
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define kspsetchecknormiteration_ kspsetchecknormiteration
#endif
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef PETSC_HAVE_FORTRAN_CAPS
#define kspsetlagnorm_ PKSPSETLAGNORM
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define kspsetlagnorm_ pkspsetlagnorm
#else
#define kspsetlagnorm_ pkspsetlagnorm_
#endif
#else
#ifdef PETSC_HAVE_FORTRAN_CAPS
#define kspsetlagnorm_ KSPSETLAGNORM
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define kspsetlagnorm_ kspsetlagnorm
#endif
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef PETSC_HAVE_FORTRAN_CAPS
#define kspsetsupportednorm_ PKSPSETSUPPORTEDNORM
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define kspsetsupportednorm_ pkspsetsupportednorm
#else
#define kspsetsupportednorm_ pkspsetsupportednorm_
#endif
#else
#ifdef PETSC_HAVE_FORTRAN_CAPS
#define kspsetsupportednorm_ KSPSETSUPPORTEDNORM
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define kspsetsupportednorm_ kspsetsupportednorm
#endif
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef PETSC_HAVE_FORTRAN_CAPS
#define kspgetnormtype_ PKSPGETNORMTYPE
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define kspgetnormtype_ pkspgetnormtype
#else
#define kspgetnormtype_ pkspgetnormtype_
#endif
#else
#ifdef PETSC_HAVE_FORTRAN_CAPS
#define kspgetnormtype_ KSPGETNORMTYPE
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define kspgetnormtype_ kspgetnormtype
#endif
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef PETSC_HAVE_FORTRAN_CAPS
#define kspsetoperators_ PKSPSETOPERATORS
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define kspsetoperators_ pkspsetoperators
#else
#define kspsetoperators_ pkspsetoperators_
#endif
#else
#ifdef PETSC_HAVE_FORTRAN_CAPS
#define kspsetoperators_ KSPSETOPERATORS
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define kspsetoperators_ kspsetoperators
#endif
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef PETSC_HAVE_FORTRAN_CAPS
#define kspgetoperators_ PKSPGETOPERATORS
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define kspgetoperators_ pkspgetoperators
#else
#define kspgetoperators_ pkspgetoperators_
#endif
#else
#ifdef PETSC_HAVE_FORTRAN_CAPS
#define kspgetoperators_ KSPGETOPERATORS
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define kspgetoperators_ kspgetoperators
#endif
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef PETSC_HAVE_FORTRAN_CAPS
#define kspcreate_ PKSPCREATE
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define kspcreate_ pkspcreate
#else
#define kspcreate_ pkspcreate_
#endif
#else
#ifdef PETSC_HAVE_FORTRAN_CAPS
#define kspcreate_ KSPCREATE
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define kspcreate_ kspcreate
#endif
#endif



/* Definitions of Fortran Wrapper routines */
#if defined(__cplusplus)
extern "C" {
#endif
PetscErrorCode  kspsetnormtype_(KSP ksp,KSPNormType *normtype, int *__ierr)
{
*__ierr = KSPSetNormType(
	(KSP)PetscToPointer( *(int*)(ksp) ),*normtype);
}
PetscErrorCode  kspsetchecknormiteration_(KSP ksp,PetscInt *it, int *__ierr)
{
*__ierr = KSPSetCheckNormIteration(
	(KSP)PetscToPointer( *(int*)(ksp) ),*it);
}
PetscErrorCode  kspsetlagnorm_(KSP ksp,PetscBool *flg, int *__ierr)
{
*__ierr = KSPSetLagNorm(
	(KSP)PetscToPointer( *(int*)(ksp) ),*flg);
}
PetscErrorCode  kspsetsupportednorm_(KSP ksp,KSPNormType *normtype,PCSide *pcside,PetscInt *priority, int *__ierr)
{
*__ierr = KSPSetSupportedNorm(
	(KSP)PetscToPointer( *(int*)(ksp) ),*normtype,*pcside,*priority);
}
PetscErrorCode  kspgetnormtype_(KSP ksp,KSPNormType *normtype, int *__ierr)
{
*__ierr = KSPGetNormType(
	(KSP)PetscToPointer( *(int*)(ksp) ),
	(KSPNormType* )PetscToPointer( *(int*)(normtype) ));
}
PetscErrorCode  kspsetoperators_(KSP ksp,Mat Amat,Mat Pmat, int *__ierr)
{
*__ierr = KSPSetOperators(
	(KSP)PetscToPointer( *(int*)(ksp) ),
	(Mat)PetscToPointer( *(int*)(Amat) ),
	(Mat)PetscToPointer( *(int*)(Pmat) ));
}
PetscErrorCode  kspgetoperators_(KSP ksp,Mat *Amat,Mat *Pmat, int *__ierr)
{
*__ierr = KSPGetOperators(
	(KSP)PetscToPointer( *(int*)(ksp) ),Amat,Pmat);
}
PetscErrorCode  kspcreate_(MPI_Fint * comm,KSP *inksp, int *__ierr)
{
*__ierr = KSPCreate(
	MPI_Comm_f2c(*(comm)),inksp);
}
#if defined(__cplusplus)
}
#endif
