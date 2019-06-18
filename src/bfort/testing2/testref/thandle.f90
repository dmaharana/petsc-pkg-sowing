      module f90header
      interface
      function KSPSetNormType(ksp,normtype)
       use handledef
       KSP ksp ! KSP
       KSPNormType normtype ! KSPNormType
       PetscErrorCode KSPSetNormType ! PetscErrorCode
      end function KSPSetNormType
      function KSPSetSupportedNorm(ksp,normtype,pcside,priority)
       use handledef
       KSP ksp ! KSP
       KSPNormType normtype ! KSPNormType
       PCSide pcside ! PCSide
       PetscInt priority ! PetscInt
       PetscErrorCode KSPSetSupportedNorm ! PetscErrorCode
      end function KSPSetSupportedNorm
      function KSPGetNormType(ksp,normtype)
       use handledef
       KSP ksp ! KSP
       KSPNormType normtype ! KSPNormType
       PetscErrorCode KSPGetNormType ! PetscErrorCode
      end function KSPGetNormType
      function KSPSetOperators(ksp,Amat,Pmat)
       use handledef
       KSP ksp ! KSP
       Mat Amat ! Mat
       Mat Pmat ! Mat
       PetscErrorCode KSPSetOperators ! PetscErrorCode
      end function KSPSetOperators
      function KSPGetOperators(ksp,Amat,Pmat)
       use handledef
       KSP ksp ! KSP
       Mat Amat ! Mat
       Mat Pmat ! Mat
       PetscErrorCode KSPGetOperators ! PetscErrorCode
      end function KSPGetOperators
      function KSPCreate(comm,inksp)
       use handledef
       MPI_Comm comm ! MPI_Comm
       KSP inksp ! KSP
       PetscErrorCode KSPCreate ! PetscErrorCode
      end function KSPCreate
      subroutine LongArgList(inksp,thisisthefirstlongargument,          &
     &thisisthesecondlongargument,thisisusedforanoutput,alongnamedscalar&
     &,alongnamedvector,thefinallongargumentname)
       use handledef
       KSP inksp ! KSP
       Mat thisisthefirstlongargument ! Mat
       Mat thisisthesecondlongargument ! Mat
       integer thisisusedforanoutput ! int
       double precision alongnamedscalar ! double
       double precision alongnamedvector ! double
       integer thefinallongargumentname ! int
      
      end subroutine LongArgList
      end interface
      end module
