      module f90header
      interface
      subroutine simple(a,b,z)
       integer a ! int
       double precision b ! double
       integer z
       end subroutine simple
      subroutine foo1(a,b,c,z)
       integer a ! int
       character b ! char
       external c ! void function pointer
       integer z
       end subroutine foo1
      subroutine foo1a(a,b,c,z)
       integer a ! int
       character b ! char
       external c ! void function pointer
       integer z
       end subroutine foo1a
      subroutine foo2(a,b,z)
       MyType a ! MyType
       integer*2 b ! short
       integer z
       end subroutine foo2
      subroutine foo3(z)
       integer z
       end subroutine foo3
      subroutine foo3a(z)
       integer z
       end subroutine foo3a
      end interface
      end module f90header
