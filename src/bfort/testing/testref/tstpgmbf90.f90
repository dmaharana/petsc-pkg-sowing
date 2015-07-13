      module f90header
      interface
      subroutine simple(a,b,z)
       integer a ! int
       double precision b ! double
       integer z
       end subroutine
      subroutine foo1(a,b,c,z)
       integer a ! int
       character b ! char
       external c ! void function pointer
       integer z
       end subroutine
      subroutine foo1a(a,b,c,z)
       integer a ! int
       character b ! char
       external c ! void function pointer
       integer z
       end subroutine
      subroutine foo2(a,b,z)
       MyType a ! MyType
       integer*2 b ! short
       integer z
       end subroutine
      subroutine foo3(z)
       integer z
       end subroutine
      subroutine foo3a(z)
       integer z
       end subroutine
      end interface
      end module
