/* This is a test case for generating fortran wrappers.
   This version contains routines that return int, allowing tests of the
   option to return value as a parameter (-ferr option)

   Function names are mixed case to avoid colision with the generated
   Fortran names.
 */

/* Basic functions with 0 to a few arguments */
/*@ Funcf1 - function with no args
  @*/
int Funcf1(void)
{
}

/*@ Funcf2 - function with one value arg
  @*/
int Funcf2(double av)
{
}

/*@ Funcf3 - function with one pointer arg
  @*/
int Funcf3(int *len)
{
}

/*@ Funcf4 - function with multiple basic arguments
  @*/
int Funcf4(int *len1, double *a_v2, int v1, double a4)
{
}

/*@ Funcf5 - functdion with array arguments
  @*/
int Funcf5(int *len, double a4[])
{
}

/*@ Funcf6 - function with function pointer arguments
  @*/
int Funcf6(void (*c)(int, long))
{
}

/*@ Funcf7 - function with function pointer arguments without parms
  @*/
int Funcf7(void (*c2)())
{
}

/*@ Funcf8 - function using const in parameters
 Note:
 Original bfort incorrectly handles `const double *const p1'
 This causes bfort to create incorrect code for this function.
  @*/
int Funcf8(const int f[], const int *a, const int b, const double *const p1)
{
}

/*@ Funcf9 - function using restrict
  @*/
int Funcf9(void *restrict carray, const int * restrict ptr_2)
{
}

/*@ Funcf10 - function with args that aren't distinct in case
  @*/
int Funcf10(const int M, const int m, void *ptr, void *Ptr)
{
}

/*@ Funcf11 - function with long names for arguments
  @*/
int Funcf11(const int aLongArrayName[], const int *alongArgumentName,
	double *anotherLongArgumentName, int *thisIsAnotherLongName)
{
}

/*@ Funcf12 - function with a function with args as an arg
 @*/
int Funcf12( int a, char b, void (*c)(int, char, MyType *a) )
{
}

/*@ Funcf13 - function with no arguments, returning int
@*/
int Funcf13()
{
}
