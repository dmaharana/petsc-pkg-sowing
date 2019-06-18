/* This is a test case for generating fortran wrappers.
   This version contains Macros that return int, allowing tests of the
   option to return value as a parameter (-ferr option)

   Function names are mixed case to avoid colision with the generated
   Fortran names.
 */

/*I "thead.h" I*/

/* Basic functions with 0 to a few arguments */
/*M Funcf1 - function with no args

Synopsis:
int Funcf1(void)

M*/

/*M Funcf2 - function with one value arg

 Synopsis:
int Funcf2(double av)

M*/


/*M Funcf3 - function with one pointer arg

 synopsis:
 int Funcf3(int *len)

 M*/

/*M Funcf4 - function with multiple basic arguments

SYNOPSIS:
int Funcf4(int *len1, double *a_v2, int v1, double a4)

M*/


/*M Funcf5 - functdion with array arguments

 sYnOpSiS:
int Funcf5(int *len, double a4[])
M*/

/*M Funcf6 - function with function pointer arguments

  synopsis:
  int Funcf6(void (*c)(int, long))

  M*/

/*M Funcf7 - function with function pointer arguments without parms

 some other text

 synopsis:
int Funcf7(void (*c2)())

M*/


/*M Funcf8 - function using const ni parameters

Other text

synopsis:
int Funcf8(const int f[], const int *a, const int b, const double const *p1)

M*/

/*M Funcf9 - function using restrict

 synopsis:
 int Funcf9(void *restrict carray, const int * restrict ptr_2)

 M*/


/*M Funcf10 - function with args that aren't distinct in case

  Synopsis:
      int Funcf10(const int M, const int m, void *ptr, void *Ptr)
  M*/

/*M Funcf11 - function with long names for arguments

Notes:

Synopsis:
int Funcf11(const int aLongArrayName[], const int *alongArgumentName,
	double *anotherLongArgumentName, int *thisIsAnotherLongName)

aM*/
