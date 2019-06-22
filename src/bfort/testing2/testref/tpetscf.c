/* tpetsc.c */
/* Fortran interface file */
/*
 * This file was generated automatically by bfort from the C source file
 * Major args: -ferr -ferrfuncret PETSC_EXTERN void PETSC_STDCALL -mnative -mapptr -ptrptrfix Petsc -ptr64 PETSC_USE_POINTER_CONVERSION -mpi -anyname -fcaps PETSC_HAVE_FORTRAN_CAPS -fuscore PETSC_HAVE_FORTRAN_UNDERSCORE -usenativeptr -f90mod-skip-header -shortargname -f90modfile tpetsc.f90
 */
#include "petscksp.h"
#ifdef PETSC_HAVE_FORTRAN_CAPS
#define kspsetnormtype_ KSPSETNORMTYPE
#define kspsetchecknormiteration_ KSPSETCHECKNORMITERATION
#define kspsetlagnorm_ KSPSETLAGNORM
#define kspsetsupportednorm_ KSPSETSUPPORTEDNORM
#define kspgetnormtype_ KSPGETNORMTYPE
#define kspsetoperators_ KSPSETOPERATORS
#define kspgetoperators_ KSPGETOPERATORS
#define kspcreate_ KSPCREATE
#elif defined(FORTRANDOUBLEUNDERSCORE)
#elif !defined(PETSC_HAVE_FORTRAN_UNDERSCORE)
#define kspsetnormtype_ kspsetnormtype
#define kspsetchecknormiteration_ kspsetchecknormiteration
#define kspsetlagnorm_ kspsetlagnorm
#define kspsetsupportednorm_ kspsetsupportednorm
#define kspgetnormtype_ kspgetnormtype
#define kspsetoperators_ kspsetoperators
#define kspgetoperators_ kspgetoperators
#define kspcreate_ kspcreate
#endif
/* Definitions of Fortran Wrapper routines */
#if defined(__cplusplus)
extern "C" {
#endif

/* Mapping of pointers in C to and from integers in Fortran */
#ifdef PETSC_USE_POINTER_CONVERSION
extern void *PetscToPointer(int);
extern int PetscFromPointer(void *);
#else
#define PetscToPointer(a) (a)
#define PetscFromPointer(a) (int)(a)
#endif

PETSC_EXTERN void PETSC_STDCALL kspsetnormtype_(KSP ksp, KSPNormType *normtype, int *z)
{
  *z = KSPSetNormType(
	(KSP)PetscToPointer(ksp), *normtype);
}
PETSC_EXTERN void PETSC_STDCALL kspsetchecknormiteration_(KSP ksp, PetscInt *it, int *z)
{
  *z = KSPSetCheckNormIteration(
	(KSP)PetscToPointer(ksp), *it);
}
PETSC_EXTERN void PETSC_STDCALL kspsetlagnorm_(KSP ksp, PetscBool *flg, int *z)
{
  *z = KSPSetLagNorm(
	(KSP)PetscToPointer(ksp), *flg);
}
PETSC_EXTERN void PETSC_STDCALL kspsetsupportednorm_(KSP ksp, KSPNormType *normtype, PCSide *pcside, PetscInt *priority, int *z)
{
  *z = KSPSetSupportedNorm(
	(KSP)PetscToPointer(ksp), *normtype, *pcside, *priority);
}
PETSC_EXTERN void PETSC_STDCALL kspgetnormtype_(KSP ksp, KSPNormType *normtype, int *z)
{
  *z = KSPGetNormType(
	(KSP)PetscToPointer(ksp), normtype);
}
PETSC_EXTERN void PETSC_STDCALL kspsetoperators_(KSP ksp, Mat Amat, Mat Pmat, int *z)
{
  *z = KSPSetOperators(
	(KSP)PetscToPointer(ksp), 
	(Mat)PetscToPointer(Amat), 
	(Mat)PetscToPointer(Pmat));
}
PETSC_EXTERN void PETSC_STDCALL kspgetoperators_(KSP ksp, Mat *Amat, Mat *Pmat, int *z)
{
  *z = KSPGetOperators(
	(KSP)PetscToPointer(ksp),  Amat,  Pmat);
}
PETSC_EXTERN void PETSC_STDCALL kspcreate_(MPI_Fint *comm, KSP *inksp, int *z)
{
  PetscErrorCode __ierr;
  __ierr = KSPCreate(MPI_Comm_f2c(*(comm)),  inksp);
  *z = __ierr;
}
#if defined(__cplusplus)
 }
#endif
