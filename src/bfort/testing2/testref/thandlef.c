/* thandle.c */
/* Fortran interface file */
/*
 * This file was generated automatically by bfort from the C source file
 * Major args: -mnative -mapptr -ptrptrfix Petsc -ptr64 PETSC_USE_POINTER_CONVERSION -anyname -usenativeptr -f90usemodule handledef -f90modfile thandle.f90
 */
#include "petscksp.h"
#include <stdlib.h>
#ifdef FORTRANCAPS
#define kspsetnormtype_ KSPSETNORMTYPE
#define kspsetsupportednorm_ KSPSETSUPPORTEDNORM
#define kspgetnormtype_ KSPGETNORMTYPE
#define kspsetoperators_ KSPSETOPERATORS
#define kspgetoperators_ KSPGETOPERATORS
#define kspcreate_ KSPCREATE
#define longarglist_ LONGARGLIST
#elif defined(FORTRANDOUBLEUNDERSCORE)
#elif !defined(FORTRANUNDERSCORE)
#define kspsetnormtype_ kspsetnormtype
#define kspsetsupportednorm_ kspsetsupportednorm
#define kspgetnormtype_ kspgetnormtype
#define kspsetoperators_ kspsetoperators
#define kspgetoperators_ kspgetoperators
#define kspcreate_ kspcreate
#define longarglist_ longarglist
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

PetscErrorCode kspsetnormtype_(KSP ksp, KSPNormType *normtype)
{
  return KSPSetNormType(
	(KSP)PetscToPointer(ksp), *normtype);
}
PetscErrorCode kspsetsupportednorm_(KSP ksp, KSPNormType *normtype, PCSide *pcside, PetscInt *priority)
{
  return KSPSetSupportedNorm(
	(KSP)PetscToPointer(ksp), *normtype, *pcside, *priority);
}
PetscErrorCode kspgetnormtype_(KSP ksp, KSPNormType *normtype)
{
  return KSPGetNormType(
	(KSP)PetscToPointer(ksp), normtype);
}
PetscErrorCode kspsetoperators_(KSP ksp, Mat Amat, Mat Pmat)
{
  return KSPSetOperators(
	(KSP)PetscToPointer(ksp), 
	(Mat)PetscToPointer(Amat), 
	(Mat)PetscToPointer(Pmat));
}
PetscErrorCode kspgetoperators_(KSP ksp, Mat *Amat, Mat *Pmat)
{
  return KSPGetOperators(
	(KSP)PetscToPointer(ksp),  Amat,  Pmat);
}
PetscErrorCode kspcreate_(MPI_Comm *comm, KSP *inksp)
{
  return KSPCreate(*comm,  inksp);
}
void longarglist_(KSP inksp, Mat thisisthefirstlongargument, Mat thisisthesecondlongargument, int *thisisusedforanoutput, double *alongnamedscalar, double *alongnamedvector, int *thefinallongargumentname)
{
  LongArgList(
	(KSP)PetscToPointer(inksp), 
	(Mat)PetscToPointer(thisisthefirstlongargument), 
	(Mat)PetscToPointer(thisisthesecondlongargument), thisisusedforanoutput, *alongnamedscalar, alongnamedvector, *thefinallongargumentname);
}
#if defined(__cplusplus)
 }
#endif
