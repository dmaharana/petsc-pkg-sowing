/* This is a test case for generating fortran wrappers.

   This file contains routines that return non-int values, or are void.
   This file must not be processed with the -ferr option.
*/
/*@ Fa - subroutine
  @*/
void Fa(int *aptr)
{
}

/*@ Fa_v - subroutine with void
  @*/
void Fa_v(void)
{
}


/*@ Funcf12 - subroutine with a function with args as an arg
 @*/
void Funcf12( int a, char b, void (*c)(int, char, MyType *a) )
{
}


/*@ S_ptr - return pointer to struct
  @*/
struct mine *S_ptr(void *a, int len)
{
}

/*@ Funcf13 - function with an unknown type as an arg, and returning a
    pointer to arg.
@*/
struct tm *Funcf13( MyType *a, short b )
{
}


