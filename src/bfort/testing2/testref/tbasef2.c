/* tbase.c */
/* Fortran interface file */
/*
 * This file was generated automatically by bfort from the C source file
 * Major args: -ferr -anyname
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
void funcf1_(int *z)
{
  *z = Funcf1();
}
void funcf2_(double *av, int *z)
{
  *z = Funcf2(*av);
}
void funcf3_(int *len, int *z)
{
  *z = Funcf3(len);
}
void funcf4_(int *len1, double *a_v2, int *v1, double *a4, int *z)
{
  *z = Funcf4(len1, a_v2, *v1, *a4);
}
void funcf5_(int *len, double a4[], int *z)
{
  *z = Funcf5(len, a4);
}
void funcf6_(void (*c)(int, long), int *z)
{
  *z = Funcf6(c);
}
void funcf7_(void (*c2)(), int *z)
{
  *z = Funcf7(c2);
}
void funcf8_(const int f[], const int *a, int *b, const double *const p1, int *z)
{
  *z = Funcf8(f, a, *b, p1);
}
void funcf9_(void *restrict carray, const int * restrict ptr_2, int *z)
{
  *z = Funcf9(carray, ptr_2);
}
void funcf10_(int *M, int *m, void *ptr, void *Ptr, int *z)
{
  *z = Funcf10(*M, *m, ptr, Ptr);
}
void funcf11_(const int aLongArrayName[], const int *alongArgumentName, double *anotherLongArgumentName, int *thisIsAnotherLongName, int *z)
{
  *z = Funcf11(aLongArrayName, alongArgumentName, anotherLongArgumentName, thisIsAnotherLongName);
}
void funcf12_(int *a, char *b, void (*c)(int, char, MyType *a), int *z, int cl0)
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
  *z = __z;
}
void funcf13_(int *z)
{
  *z = Funcf13();
}
#if defined(__cplusplus)
 }
#endif
