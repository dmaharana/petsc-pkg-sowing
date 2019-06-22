/* tret.c */
/* Fortran interface file */
/*
 * This file was generated automatically by bfort from the C source file
 * Major args: -anyname
 */
#ifdef FORTRANCAPS
#define fa_ FA
#define fa_v_ FA_V
#define funcf12_ FUNCF12
#define s_ptr_ S_PTR
#define funcf13_ FUNCF13
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define fa_v_ fa_v__
#define s_ptr_ s_ptr__
#elif !defined(FORTRANUNDERSCORE)
#define fa_ fa
#define fa_v_ fa_v
#define funcf12_ funcf12
#define s_ptr_ s_ptr
#define funcf13_ funcf13
#endif
/* Definitions of Fortran Wrapper routines */
#if defined(__cplusplus)
extern "C" {
#endif
#include <stdlib.h>
void fa_(int *aptr)
{
  Fa(aptr);
}
void fa_v_(void)
{
  Fa_v();
}
void funcf12_(int *a, char *b, void (*c)(int, char, MyType *a), int cl0)
{
  char *_cltmp0=0;
  /* insert Fortran-to-C conversion for b */
  _cltmp0 = (char*)malloc(cl0+1);
  if (_cltmp0) {
     int _i;
     for(_i=0; _i<cl0; _i++) _cltmp0[_i]=b[_i];
     _cltmp0[_i] = 0;
  }
  if (!(_cltmp0)) {abort(1);}
  Funcf12(*a, *_cltmp0, c);
  /* insert C-to-Fortran conversion for b */
  if (_cltmp0) {
    int _i;
    for(_i=0; _i<cl0 && _cltmp0[_i]; _i++) b[_i]=_cltmp0[_i];
    while (_i<cl0) b[_i++] = ' ';
    free(_cltmp0);
  }
}
struct mine * s_ptr_(void *a, int *len)
{
  return S_ptr(a, *len);
}
struct tm * funcf13_(MyType *a, short *b)
{
  return Funcf13(a, *b);
}
#if defined(__cplusplus)
 }
#endif
