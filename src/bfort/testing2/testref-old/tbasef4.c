/* tbase.c */
/* Fortran interface file */

/*
* This file was generated automatically by bfort from the C source
* file.  
 */
#ifdef USECAPS
#define funcf1_ FUNCF1
#elif !defined(USEUSCORE) && !defined(USEDOUBLEUSCORE)
#define funcf1_ funcf1
#endif
#ifdef USECAPS
#define funcf2_ FUNCF2
#elif !defined(USEUSCORE) && !defined(USEDOUBLEUSCORE)
#define funcf2_ funcf2
#endif
#ifdef USECAPS
#define funcf3_ FUNCF3
#elif !defined(USEUSCORE) && !defined(USEDOUBLEUSCORE)
#define funcf3_ funcf3
#endif
#ifdef USECAPS
#define funcf4_ FUNCF4
#elif !defined(USEUSCORE) && !defined(USEDOUBLEUSCORE)
#define funcf4_ funcf4
#endif
#ifdef USECAPS
#define funcf5_ FUNCF5
#elif !defined(USEUSCORE) && !defined(USEDOUBLEUSCORE)
#define funcf5_ funcf5
#endif
#ifdef USECAPS
#define funcf6_ FUNCF6
#elif !defined(USEUSCORE) && !defined(USEDOUBLEUSCORE)
#define funcf6_ funcf6
#endif
#ifdef USECAPS
#define funcf7_ FUNCF7
#elif !defined(USEUSCORE) && !defined(USEDOUBLEUSCORE)
#define funcf7_ funcf7
#endif
#ifdef USECAPS
#define funcf8_ FUNCF8
#elif !defined(USEUSCORE) && !defined(USEDOUBLEUSCORE)
#define funcf8_ funcf8
#endif
#ifdef USECAPS
#define funcf9_ FUNCF9
#elif !defined(USEUSCORE) && !defined(USEDOUBLEUSCORE)
#define funcf9_ funcf9
#endif
#ifdef USECAPS
#define funcf10_ FUNCF10
#elif !defined(USEUSCORE) && !defined(USEDOUBLEUSCORE)
#define funcf10_ funcf10
#endif
#ifdef USECAPS
#define funcf11_ FUNCF11
#elif !defined(USEUSCORE) && !defined(USEDOUBLEUSCORE)
#define funcf11_ funcf11
#endif
#ifdef USECAPS
#define funcf12_ FUNCF12
#elif !defined(USEUSCORE) && !defined(USEDOUBLEUSCORE)
#define funcf12_ funcf12
#endif
#ifdef USECAPS
#define funcf13_ FUNCF13
#elif !defined(USEUSCORE) && !defined(USEDOUBLEUSCORE)
#define funcf13_ funcf13
#endif


/* Definitions of Fortran Wrapper routines */
#if defined(__cplusplus)
extern "C" {
#endif
void funcf1_(int *__z)
{
*__z = Funcf1();
}
void funcf2_(double *av, int *__z)
{
*__z = Funcf2(*av);
}
void funcf3_(int *len, int *__z)
{
*__z = Funcf3(len);
}
void funcf4_(int *len1,double *a_v2,int *v1,double *a4, int *__z)
{
*__z = Funcf4(len1,a_v2,*v1,*a4);
}
void funcf5_(int *len,double a4[], int *__z)
{
*__z = Funcf5(len,a4);
}
void funcf6_(void(*c)(int, long), int *__z)
{
*__z = Funcf6(c);
}
void funcf7_(void(*c2)(), int *__z)
{
*__z = Funcf7(c2);
}
void funcf8_( int f[], int *a, int *b, double *constp){
*__z = Funcf8(f,a,*b,const);
}
void funcf9_(void* carray, int * ptr_2, int *__z)
{
*__z = Funcf9(carray,ptr_2);
}
void funcf10_( int *M, int *m,void*ptr,void*Ptr, int *__z)
{
*__z = Funcf10(*M,*m,ptr,Ptr);
}
void funcf11_( int aLongArrayName[], int *alongArgumentName,
 double *anotherLongArgumentName,int *thisIsAnotherLongName, int *__z)
{
*__z = Funcf11(aLongArrayName,alongArgumentName,anotherLongArgumentName,thisIsAnotherLongName);
}
void funcf12_(int *a,char *b,void(*c)(int, char, MyType *a), int *__z, int cl0)
{
*__z = Funcf12(*a,*b,c);
}
void funcf13_(int *__z)
{
*__z = Funcf13();
}
#if defined(__cplusplus)
}
#endif
