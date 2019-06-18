/* tbase.c */
/* Fortran interface file for x86_64 */

/*
* This file was generated automatically by bfort from the C source
* file.  
 */


/* Definitions of Fortran Wrapper routines */
#if defined(__cplusplus)
extern "C" {
#endif
void funcf1(int *__z)
{
*__z = Funcf1();
}
void funcf2(double *av, int *__z)
{
*__z = Funcf2(*av);
}
void funcf3(int *len, int *__z)
{
*__z = Funcf3(len);
}
void funcf4(int *len1,double *a_v2,int *v1,double *a4, int *__z)
{
*__z = Funcf4(len1,a_v2,*v1,*a4);
}
void funcf5(int *len,double a4[], int *__z)
{
*__z = Funcf5(len,a4);
}
void funcf6(void(*c)(int, long), int *__z)
{
*__z = Funcf6(c);
}
void funcf7(void(*c2)(), int *__z)
{
*__z = Funcf7(c2);
}
void funcf8( int f[], int *a, int *b, double *constp){
*__z = Funcf8(f,a,*b,const);
}
void funcf9(void* carray, int * ptr_2, int *__z)
{
*__z = Funcf9(carray,ptr_2);
}
void funcf10( int *M, int *m,void*ptr,void*Ptr, int *__z)
{
*__z = Funcf10(*M,*m,ptr,Ptr);
}
void funcf11( int aLongArrayName[], int *alongArgumentName,
 double *anotherLongArgumentName,int *thisIsAnotherLongName, int *__z)
{
*__z = Funcf11(aLongArrayName,alongArgumentName,anotherLongArgumentName,thisIsAnotherLongName);
}
void funcf12(int *a,char *b,void(*c)(int, char, MyType *a), int *__z, int cl0)
{
*__z = Funcf12(*a,*b,c);
}
void funcf13(int *__z)
{
*__z = Funcf13();
}
#if defined(__cplusplus)
}
#endif
