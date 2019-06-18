/* This is a test case for generating fortran wrappers.
   This version contains routines that return int, allowing tests of the
   option to return value as a parameter (-ferr option)

   These routines test the handling of character data

   Depending on the arguments to bfort, the generated code may also
   insert code to convert to/from C strings from the Fortran args.

   Note that const char [] data does not need to be converted from C back
   to Fortran
 */

/*@
Getstr -  A routine with a string
@*/
int Getstr(int nc, char *a)
{
}

/*@
 Getstr2 - Write as array instead of pointer
@*/
int Getstr2(int nc, char a[])
{
}

/*@
 Putstr - Put a string
@*/
int Putstr(int nc, const char *a)
{
}

/*@ Mstring - multiple strings
  @*/
int Mstring(void *buf, int len, char *n1, const char *n2, int l3, char *n_4)
{
}

