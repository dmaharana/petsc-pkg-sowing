/* Test of support for other handle options, using the PETSc examples.
 */

#include <petsc/private/kspimpl.h>      /*I "petscksp.h" I*/
#include <stdlib.h>                     /*I <stdlib.h> I*/

/*@
   KSPSetNormType - Sets the norm that is used for convergence testing.

$ Here are some verbatim comments to skip
Args:
+ foo
. bar
- foobar

@*/
PetscErrorCode  KSPSetNormType(KSP ksp, KSPNormType normtype)
{}

/*@
   KSPSetSupportedNorm - Sets a norm and preconditioner side supported by a KSP

@*/
PetscErrorCode KSPSetSupportedNorm(KSP ksp,KSPNormType normtype,PCSide pcside,PetscInt priority)
{}

/*@
   KSPGetNormType - Gets the norm that is used for convergence testing.
@*/
PetscErrorCode  KSPGetNormType(KSP ksp, KSPNormType *normtype)
{}

/*@
   KSPSetOperators - Sets the matrix associated with the linear system
   and a (possibly) different one associated with the preconditioner.
@*/
PetscErrorCode  KSPSetOperators(KSP ksp,Mat Amat,Mat Pmat)
{}

/*@
   KSPGetOperators - Gets the matrix associated with the linear system
   and a (possibly) different one associated with the preconditioner.
@*/
PetscErrorCode  KSPGetOperators(KSP ksp,Mat *Amat,Mat *Pmat)
{}

/*@
   KSPCreate - Creates the default KSP context.
@*/
PetscErrorCode  KSPCreate(MPI_Comm comm,KSP *inksp)
{}

/*@
   LongArgList - Routine with a long argument list to test Fortran 90
   output routines

  @*/
void LongArgList(KSP inksp, Mat thisisthefirstlongargument,
		 Mat thisisthesecondlongargument, int *thisisusedforanoutput,
		 double alongnamedscalar, double *alongnamedvector,
		 int thefinallongargumentname)
{
}
