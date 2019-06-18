/* tret.c */
/* Fortran interface file */

/*
* This file was generated automatically by bfort from the C source
* file.  
 */
#ifdef FORTRANCAPS
#define fa_ FA
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define fa_ fa
#endif
#ifdef FORTRANCAPS
#define fa_v_ FA_V
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define fa_v_ fa_v__
#elif !defined(FORTRANUNDERSCORE)
#define fa_v_ fa_v
#endif
#ifdef FORTRANCAPS
#define funcf12_ FUNCF12
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define funcf12_ funcf12
#endif
#ifdef FORTRANCAPS
#define s_ptr_ S_PTR
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define s_ptr_ s_ptr__
#elif !defined(FORTRANUNDERSCORE)
#define s_ptr_ s_ptr
#endif
#ifdef FORTRANCAPS
#define funcf13_ FUNCF13
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define funcf13_ funcf13
#endif


/* Definitions of Fortran Wrapper routines */
#if defined(__cplusplus)
extern "C" {
#endif
void  fa_(int *aptr)
{
Fa(aptr);
}
void  fa_v_(void)
{
Fa_v();
}
void  funcf12_(int *a,char *b,void (*c)(int, char, MyType *a), int cl0)
{
Funcf12(*a,*b,c);
}
struct mine * s_ptr_(void *a,int *len)
{
return S_ptr(a,*len);
}
struct tm * funcf13_(MyType *a,short *b)
{
return Funcf13(
	(MyType* )*((int*)a),*b);
}
#if defined(__cplusplus)
}
#endif
