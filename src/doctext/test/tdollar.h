/*MC

    UsingFortran - To use PETSc with Fortran you must use both PETSc include files and modules. At the beginning
      of every function and module definition you need something like

$
$#include "petsc/finclude/petscXXX.h"
$         use petscXXXX

     You can delcare PETSc variables using either 

$    XXX variablename
$    type(tXXX) variablename

    for example

$#include "petsc/finclude/petscvec.h"
$         use petscvec
$
$    Vec  A
$    type(tVec) A

    Level: beginner

M*/
