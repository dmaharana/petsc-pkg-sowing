/* tchar.c */
/* Fortran interface file */
/*
 * This file was generated automatically by bfort from the C source file
 * Major args: -anyname
 */
/* Function prototypes */
int Getstr(int nc, char *a);
int Getstr2(int nc, char a[]);
int Putstr(int nc, const char *a);
int Mstring(void *buf, int len, char *n1, const char *n2, int l3, char *n_4);


#ifdef FORTRANCAPS
#define getstr_ GETSTR
#define getstr2_ GETSTR2
#define putstr_ PUTSTR
#define mstring_ MSTRING
#elif defined(FORTRANDOUBLEUNDERSCORE)
#elif !defined(FORTRANUNDERSCORE)
#define getstr_ getstr
#define getstr2_ getstr2
#define putstr_ putstr
#define mstring_ mstring
#endif
/* Definitions of Fortran Wrapper routines */
#if defined(__cplusplus)
extern "C" {
#endif
#include <stdlib.h>
int getstr_(int *nc, char *a, int cl0)
{
  int __z;
  char *_cltmp0=0;
  /* insert Fortran-to-C conversion for a */
  _cltmp0 = (char*)malloc(cl0+1);
  if (_cltmp0) {
     int _i;
     for(_i=0; _i<cl0; _i++) _cltmp0[_i]=a[_i];
     _cltmp0[_i] = 0;
  }
  if (!(_cltmp0)) {abort(1);}
  __z = Getstr(*nc, _cltmp0);
  /* insert C-to-Fortran conversion for a */
  if (_cltmp0) {
    int _i;
    for(_i=0; _i<cl0 && _cltmp0[_i]; _i++) a[_i]=_cltmp0[_i];
    while (_i<cl0) a[_i++] = ' ';
    free(_cltmp0);
  }
  return __z;
}
int getstr2_(int *nc, char a[], int cl0)
{
  int __z;
  char *_cltmp0=0;
  /* insert Fortran-to-C conversion for a */
  _cltmp0 = (char*)malloc(cl0+1);
  if (_cltmp0) {
     int _i;
     for(_i=0; _i<cl0; _i++) _cltmp0[_i]=a[_i];
     _cltmp0[_i] = 0;
  }
  if (!(_cltmp0)) {abort(1);}
  __z = Getstr2(*nc, _cltmp0);
  /* insert C-to-Fortran conversion for a */
  if (_cltmp0) {
    int _i;
    for(_i=0; _i<cl0 && _cltmp0[_i]; _i++) a[_i]=_cltmp0[_i];
    while (_i<cl0) a[_i++] = ' ';
    free(_cltmp0);
  }
  return __z;
}
int putstr_(int *nc, const char *a, int cl0)
{
  int __z;
  char *_cltmp0=0;
  /* insert Fortran-to-C conversion for a */
  _cltmp0 = (char*)malloc(cl0+1);
  if (_cltmp0) {
     int _i;
     for(_i=0; _i<cl0; _i++) _cltmp0[_i]=a[_i];
     _cltmp0[_i] = 0;
  }
  if (!(_cltmp0)) {abort(1);}
  __z = Putstr(*nc, _cltmp0);
  /* insert free for in string for a */
  if (_cltmp0) { free(_cltmp0); }
  return __z;
}
int mstring_(void *buf, int *len, char *n1, const char *n2, int *l3, char *n_4, int cl0, int cl1, int cl2)
{
  int __z;
  char *_cltmp0=0;
  char *_cltmp1=0;
  char *_cltmp2=0;
  /* insert Fortran-to-C conversion for n1 */
  _cltmp0 = (char*)malloc(cl0+1);
  if (_cltmp0) {
     int _i;
     for(_i=0; _i<cl0; _i++) _cltmp0[_i]=n1[_i];
     _cltmp0[_i] = 0;
  }
  if (!(_cltmp0)) {abort(1);}
  /* insert Fortran-to-C conversion for n2 */
  _cltmp1 = (char*)malloc(cl1+1);
  if (_cltmp1) {
     int _i;
     for(_i=0; _i<cl1; _i++) _cltmp1[_i]=n2[_i];
     _cltmp1[_i] = 0;
  }
  if (!(_cltmp1)) {abort(1);}
  /* insert Fortran-to-C conversion for n_4 */
  _cltmp2 = (char*)malloc(cl2+1);
  if (_cltmp2) {
     int _i;
     for(_i=0; _i<cl2; _i++) _cltmp2[_i]=n_4[_i];
     _cltmp2[_i] = 0;
  }
  if (!(_cltmp2)) {abort(1);}
  __z = Mstring(buf, *len, _cltmp0, _cltmp1, *l3, _cltmp2);
  /* insert C-to-Fortran conversion for n1 */
  if (_cltmp0) {
    int _i;
    for(_i=0; _i<cl0 && _cltmp0[_i]; _i++) n1[_i]=_cltmp0[_i];
    while (_i<cl0) n1[_i++] = ' ';
    free(_cltmp0);
  }
  /* insert free for in string for n2 */
  if (_cltmp1) { free(_cltmp1); }
  /* insert C-to-Fortran conversion for n_4 */
  if (_cltmp2) {
    int _i;
    for(_i=0; _i<cl2 && _cltmp2[_i]; _i++) n_4[_i]=_cltmp2[_i];
    while (_i<cl2) n_4[_i++] = ' ';
    free(_cltmp2);
  }
  return __z;
}
#if defined(__cplusplus)
 }
#endif
