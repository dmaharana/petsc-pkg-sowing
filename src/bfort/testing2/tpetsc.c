/* Test of support for PETSc.
   Drawn from itcreate.c; includes routines marked as C only to test that
   handling (and reporting).
 */

#include <petsc/private/kspimpl.h>      /*I "petscksp.h" I*/

/*@C
  KSPLoad - Loads a KSP that has been stored in binary  with KSPView().
@*/
PetscErrorCode  KSPLoad(KSP newdm, PetscViewer viewer)
{}

/*@C
   KSPView - Prints the KSP data structure.

@*/
PetscErrorCode  KSPView(KSP ksp,PetscViewer viewer)
{}

/*@
   KSPSetNormType - Sets the norm that is used for convergence testing.
@*/
PetscErrorCode  KSPSetNormType(KSP ksp,KSPNormType normtype)
{}

/*@
   KSPSetCheckNormIteration - Sets the first iteration at which the norm of the residual will be
     computed and used in the convergence test.

@*/
PetscErrorCode  KSPSetCheckNormIteration(KSP ksp,PetscInt it)
{}

/*@
   KSPSetLagNorm - Lags the residual norm calculation so that it is computed as part of the MPI_Allreduce() for
   computing the inner products for the next iteration.  This can reduce communication costs at the expense of doing
   one additional iteration.

@*/
PetscErrorCode  KSPSetLagNorm(KSP ksp,PetscBool flg)
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

/*@C
   KSPGetOperatorsSet - Determines if the matrix associated with the linear system and
   possibly a different one associated with the preconditioner have been set in the KSP.
@*/
PetscErrorCode  KSPGetOperatorsSet(KSP ksp,PetscBool  *mat,PetscBool  *pmat)
{}

/*@C
   KSPSetPreSolve - Sets a function that is called before every KSPSolve() is started
@*/
PetscErrorCode  KSPSetPreSolve(KSP ksp,PetscErrorCode (*presolve)(KSP,Vec,Vec,void*),void *prectx)
{}

/*@C
   KSPSetPostSolve - Sets a function that is called after every KSPSolve() completes (whether it converges or not)
@*/
PetscErrorCode  KSPSetPostSolve(KSP ksp,PetscErrorCode (*postsolve)(KSP,Vec,Vec,void*),void *postctx)
{}

/*@
   KSPCreate - Creates the default KSP context.
@*/
PetscErrorCode  KSPCreate(MPI_Comm comm,KSP *inksp)
{}

/*@C
   KSPSetType - Builds KSP for a particular solver.
@*/
PetscErrorCode  KSPSetType(KSP ksp, KSPType type)
{}

/*@C
   KSPGetType - Gets the KSP type as a string from the KSP object.
@*/
PetscErrorCode  KSPGetType(KSP ksp,KSPType *type)
{}

/*@C
  KSPRegister -  Adds a method to the Krylov subspace solver package.
@*/
PetscErrorCode  KSPRegister(const char sname[],PetscErrorCode (*function)(KSP))
{}
