      subroutine KSPSetNormType(a,b,z)
       KSP a ! KSP
       KSPNormType b ! KSPNormType
       integer z
       end subroutine KSPSetNormType
      subroutine KSPSetCheckNormIteration(a,b,z)
       KSP a ! KSP
       PetscInt b ! PetscInt
       integer z
       end subroutine KSPSetCheckNormIteration
      subroutine KSPSetLagNorm(a,b,z)
       KSP a ! KSP
       PetscBool b ! PetscBool
       integer z
       end subroutine KSPSetLagNorm
      subroutine KSPSetSupportedNorm(a,b,c,d,z)
       KSP a ! KSP
       KSPNormType b ! KSPNormType
       PCSide c ! PCSide
       PetscInt d ! PetscInt
       integer z
       end subroutine KSPSetSupportedNorm
      subroutine KSPGetNormType(a,b,z)
       KSP a ! KSP
       KSPNormType b ! KSPNormType
       integer z
       end subroutine KSPGetNormType
      subroutine KSPSetOperators(a,b,c,z)
       KSP a ! KSP
       Mat b ! Mat
       Mat c ! Mat
       integer z
       end subroutine KSPSetOperators
      subroutine KSPGetOperators(a,b,c,z)
       KSP a ! KSP
       Mat b ! Mat
       Mat c ! Mat
       integer z
       end subroutine KSPGetOperators
      subroutine KSPCreate(a,b,z)
       integer a ! MPI_Comm
       KSP b ! KSP
       integer z
       end subroutine KSPCreate
