/* tstpgm.c */
/* Fortran interface file for i386 */

/*
* This file was generated automatically by bfort from the C source
* file.  
 */


/* Definitions of Fortran Wrapper routines */
#if defined(__cplusplus)
extern "C" {
#endif
void  foo1( a, b, c)
int *a;
char *b;
void (*c)();
{
foo1(*a,*b,c);
}
struct tm * foo2( a, b)
MyType *a;
short *b;
{
return foo2(
	(MyType* )*((int*)a),*b);
}
int  foo3()
{
return foo3();
}
void  sygetarchtype( str, slen)
char *str;
int  *slen;
{
SYGetArchType(str,*slen);
}
#if defined(__cplusplus)
}
#endif
