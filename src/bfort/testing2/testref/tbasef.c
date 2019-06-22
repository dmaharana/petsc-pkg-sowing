/* tbase.c */
/* Fortran interface file */
/*
 * This file was generated automatically by bfort from the C source file
 * Major args: -anyname
 */
#ifdef FORTRANCAPS
#define funcf1_ FUNCF1
#define funcf2_ FUNCF2
#define funcf3_ FUNCF3
#define funcf4_ FUNCF4
#define funcf5_ FUNCF5
#define funcf6_ FUNCF6
#define funcf7_ FUNCF7
#define funcf8_ FUNCF8
#define funcf9_ FUNCF9
#define funcf10_ FUNCF10
#define funcf11_ FUNCF11
#define funcf12_ FUNCF12
#define funcf13_ FUNCF13
#elif defined(FORTRANDOUBLEUNDERSCORE)
#elif !defined(FORTRANUNDERSCORE)
#define funcf1_ funcf1
#define funcf2_ funcf2
#define funcf3_ funcf3
#define funcf4_ funcf4
#define funcf5_ funcf5
#define funcf6_ funcf6
#define funcf7_ funcf7
#define funcf8_ funcf8
#define funcf9_ funcf9
#define funcf10_ funcf10
#define funcf11_ funcf11
#define funcf12_ funcf12
#define funcf13_ funcf13
#endif
/* Definitions of Fortran Wrapper routines */
#if defined(__cplusplus)
extern "C" {
#endif
#include <stdlib.h>
int funcf1_(void)
{
  return Funcf1();
}
int funcf2_(double *av)
{
  return Funcf2(*av);
}
int funcf3_(int *len)
{
  return Funcf3(len);
}
int funcf4_(int *len1, double *a_v2, int *v1, double *a4)
{
  return Funcf4(len1, a_v2, *v1, *a4);
}
int funcf5_(int *len, double a4[])
{
  return Funcf5(len, a4);
}
int funcf6_(void (*c)(int, long))
{
  return Funcf6(c);
}
int funcf7_(void (*c2)())
{
  return Funcf7(c2);
}
int funcf8_(const int f[], const int *a, int *b, const double *const p1)
{
  return Funcf8(f, a, *b, p1);
}
int funcf9_(void *restrict carray, const int * restrict ptr_2)
{
  return Funcf9(carray, ptr_2);
}
int funcf10_(int *M, int *m, void *ptr, void *Ptr)
{
  return Funcf10(*M, *m, ptr, Ptr);
}
int funcf11_(const int aLongArrayName[], const int *alongArgumentName, double *anotherLongArgumentName, int *thisIsAnotherLongName)
{
  return Funcf11(aLongArrayName, alongArgumentName, anotherLongArgumentName, thisIsAnotherLongName);
}
int funcf12_(int *a, char *b, void (*c)(int, char, MyType *a), int cl0)
{
  int __z;
  char *_cltmp0=0;
  /* insert Fortran-to-C conversion for b */
  _cltmp0 = (char*)malloc(cl0+1);
  if (_cltmp0) {
     int _i;
     for(_i=0; _i<cl0; _i++) _cltmp0[_i]=b[_i];
     _cltmp0[_i] = 0;
  }
  if (!(_cltmp0)) {abort(1);}
  __z = Funcf12(*a, *_cltmp0, c);
  /* insert C-to-Fortran conversion for b */
  if (_cltmp0) {
    int _i;
    for(_i=0; _i<cl0 && _cltmp0[_i]; _i++) b[_i]=_cltmp0[_i];
    while (_i<cl0) b[_i++] = ' ';
    free(_cltmp0);
  }
  return __z;
}
int funcf13_(void)
{
  return Funcf13();
}
#if defined(__cplusplus)
 }
#endif
