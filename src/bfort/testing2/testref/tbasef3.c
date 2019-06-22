/* tbase.c */
/* Fortran interface file */
/*
 * This file was generated automatically by bfort from the C source file
 * Major args: -ferr -shortargname
 */
/* Definitions of Fortran Wrapper routines */
#if defined(__cplusplus)
extern "C" {
#endif
#include <stdlib.h>
void funcf1(int *z)
{
  *z = Funcf1();
}
void funcf2(double *av, int *z)
{
  *z = Funcf2(*av);
}
void funcf3(int *len, int *z)
{
  *z = Funcf3(len);
}
void funcf4(int *len1, double *a_v2, int *v1, double *a4, int *z)
{
  *z = Funcf4(len1, a_v2, *v1, *a4);
}
void funcf5(int *len, double a4[], int *z)
{
  *z = Funcf5(len, a4);
}
void funcf6(void (*c)(int, long), int *z)
{
  *z = Funcf6(c);
}
void funcf7(void (*c2)(), int *z)
{
  *z = Funcf7(c2);
}
void funcf8(const int f[], const int *a, int *b, const double *const p1, int *z)
{
  *z = Funcf8(f, a, *b, p1);
}
void funcf9(void *restrict carray, const int * restrict ptr_2, int *z)
{
  *z = Funcf9(carray, ptr_2);
}
void funcf10(int *M, int *m, void *ptr, void *Ptr, int *z)
{
  *z = Funcf10(*M, *m, ptr, Ptr);
}
void funcf11(const int aLongArrayName[], const int *alongArgumentName, double *anotherLongArgumentName, int *thisIsAnotherLongName, int *z)
{
  *z = Funcf11(aLongArrayName, alongArgumentName, anotherLongArgumentName, thisIsAnotherLongName);
}
void funcf12(int *a, char *b, void (*c)(int, char, MyType *a), int *z, int cl0)
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
void funcf13(int *z)
{
  *z = Funcf13();
}
#if defined(__cplusplus)
 }
#endif
