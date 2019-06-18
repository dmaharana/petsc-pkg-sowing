      module f90header
      interface
      subroutine simple(a, b ,ierr)
       integer a ! int
       double precision b ! double
       integer ierr
       end subroutine simple
      subroutine foo1(a, b, c ,ierr)
       integer a ! int
       character b ! char
       external c ! void function pointer
       integer ierr
       end subroutine foo1
      subroutine foo1a(a, b, c ,ierr)
       integer a ! int
       character b ! char
       external c ! void function pointer
       integer ierr
       end subroutine foo1a
      subroutine foo2(a, b ,ierr)
       MyType a ! MyType
       integer*2 b ! short
       integer ierr
       end subroutine foo2
      subroutine foo3(ierr)
       integer ierr
       end subroutine foo3
      subroutine foo3a(ierr)
       integer ierr
       end subroutine foo3a
      end interface
      end module f90header
