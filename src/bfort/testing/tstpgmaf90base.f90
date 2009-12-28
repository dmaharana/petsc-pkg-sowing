      module f90header
      interface
        subroutine simple(a, b ,ierr)
       integer a ! int
       double precision b ! double
       integer ierr
       end subroutine
        subroutine foo1(a, b, c ,ierr)
       integer a ! int
       character b ! char
       external c ! void function pointer
       integer ierr
       end subroutine
        subroutine foo1a(a, b, c ,ierr)
       integer a ! int
       character b ! char
       external c ! void function pointer
       integer ierr
       end subroutine
        subroutine foo2(a, b ,ierr)
       MyType a ! MyType
       integer*2 b ! short
       integer ierr
       end subroutine
        subroutine foo3(ierr)
       integer ierr
       end subroutine
        subroutine foo3a(ierr)
       integer ierr
       end subroutine
      end interface
      end module
