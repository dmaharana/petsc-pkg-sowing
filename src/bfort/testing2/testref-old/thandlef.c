/* thandle.c */
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
#include <stdlib.h>
#ifdef FORTRANCAPS
#define kspsetnormtype_ KSPSETNORMTYPE
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define kspsetnormtype_ kspsetnormtype
#endif
#ifdef FORTRANCAPS
#define kspsetsupportednorm_ KSPSETSUPPORTEDNORM
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define kspsetsupportednorm_ kspsetsupportednorm
#endif
#ifdef FORTRANCAPS
#define kspgetnormtype_ KSPGETNORMTYPE
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define kspgetnormtype_ kspgetnormtype
#endif
#ifdef FORTRANCAPS
#define kspsetoperators_ KSPSETOPERATORS
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define kspsetoperators_ kspsetoperators
#endif
#ifdef FORTRANCAPS
#define kspgetoperators_ KSPGETOPERATORS
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define kspgetoperators_ kspgetoperators
#endif
#ifdef FORTRANCAPS
#define kspcreate_ KSPCREATE
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define kspcreate_ kspcreate
#endif
#ifdef FORTRANCAPS
#define longarglist_ LONGARGLIST
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define longarglist_ longarglist
#endif


/* Definitions of Fortran Wrapper routines */
#if defined(__cplusplus)
extern "C" {
#endif
PetscErrorCode  kspsetnormtype_(KSP ksp,KSPNormType *normtype)
{
return KSPSetNormType(
	(KSP)PetscToPointer( *(int*)(ksp) ),*normtype);
}
PetscErrorCode  kspsetsupportednorm_(KSP ksp,KSPNormType *normtype,PCSide *pcside,PetscInt *priority)
{
return KSPSetSupportedNorm(
	(KSP)PetscToPointer( *(int*)(ksp) ),*normtype,*pcside,*priority);
}
PetscErrorCode  kspgetnormtype_(KSP ksp,KSPNormType *normtype)
{
return KSPGetNormType(
	(KSP)PetscToPointer( *(int*)(ksp) ),
	(KSPNormType* )PetscToPointer( *(int*)(normtype) ));
}
PetscErrorCode  kspsetoperators_(KSP ksp,Mat Amat,Mat Pmat)
{
return KSPSetOperators(
	(KSP)PetscToPointer( *(int*)(ksp) ),
	(Mat)PetscToPointer( *(int*)(Amat) ),
	(Mat)PetscToPointer( *(int*)(Pmat) ));
}
PetscErrorCode  kspgetoperators_(KSP ksp,Mat *Amat,Mat *Pmat)
{
return KSPGetOperators(
	(KSP)PetscToPointer( *(int*)(ksp) ),Amat,Pmat);
}
PetscErrorCode  kspcreate_(MPI_Comm *comm,KSP *inksp)
{
return KSPCreate(*comm,inksp);
}
void  longarglist_(KSP inksp,Mat thisisthefirstlongargument,
   Mat thisisthesecondlongargument,int *thisisusedforanoutput,
   double *alongnamedscalar,double *alongnamedvector,
   int *thefinallongargumentname)
{
LongArgList(
	(KSP)PetscToPointer( *(int*)(inksp) ),
	(Mat)PetscToPointer( *(int*)(thisisthefirstlongargument) ),
	(Mat)PetscToPointer( *(int*)(thisisthesecondlongargument) ),thisisusedforanoutput,*alongnamedscalar,alongnamedvector,*thefinallongargumentname);
}
#if defined(__cplusplus)
}
#endif
