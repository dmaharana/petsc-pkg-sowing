/* tbase.c */
/* Fortran interface file */

/*
* This file was generated automatically by bfort from the C source
* file.  
 */
#ifdef FORTRANCAPS
#define funcf1_ FUNCF1
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define funcf1_ funcf1
#endif
#ifdef FORTRANCAPS
#define funcf2_ FUNCF2
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define funcf2_ funcf2
#endif
#ifdef FORTRANCAPS
#define funcf3_ FUNCF3
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define funcf3_ funcf3
#endif
#ifdef FORTRANCAPS
#define funcf4_ FUNCF4
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define funcf4_ funcf4
#endif
#ifdef FORTRANCAPS
#define funcf5_ FUNCF5
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define funcf5_ funcf5
#endif
#ifdef FORTRANCAPS
#define funcf6_ FUNCF6
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define funcf6_ funcf6
#endif
#ifdef FORTRANCAPS
#define funcf7_ FUNCF7
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define funcf7_ funcf7
#endif
#ifdef FORTRANCAPS
#define funcf8_ FUNCF8
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define funcf8_ funcf8
#endif
#ifdef FORTRANCAPS
#define funcf9_ FUNCF9
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define funcf9_ funcf9
#endif
#ifdef FORTRANCAPS
#define funcf10_ FUNCF10
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define funcf10_ funcf10
#endif
#ifdef FORTRANCAPS
#define funcf11_ FUNCF11
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define funcf11_ funcf11
#endif
#ifdef FORTRANCAPS
#define funcf12_ FUNCF12
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define funcf12_ funcf12
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
int  funcf1_(void)
{
return Funcf1();
}
int  funcf2_(double *av)
{
return Funcf2(*av);
}
int  funcf3_(int *len)
{
return Funcf3(len);
}
int  funcf4_(int *len1,double *a_v2,int *v1,double *a4)
{
return Funcf4(len1,a_v2,*v1,*a4);
}
int  funcf5_(int *len,double a4[])
{
return Funcf5(len,a4);
}
int  funcf6_(void (*c)(int, long))
{
return Funcf6(c);
}
int  funcf7_(void (*c2)())
{
return Funcf7(c2);
}
int  funcf8_( int f[], int *a, int *b, double *constp){
return Funcf8(f,a,*b,const);
}
int  funcf9_(void * carray, int * ptr_2)
{
return Funcf9(carray,ptr_2);
}
int  funcf10_( int *M, int *m,void *ptr,void *Ptr)
{
return Funcf10(*M,*m,ptr,Ptr);
}
int  funcf11_( int aLongArrayName[], int *alongArgumentName,
 double *anotherLongArgumentName,int *thisIsAnotherLongName)
{
return Funcf11(aLongArrayName,alongArgumentName,anotherLongArgumentName,thisIsAnotherLongName);
}
int  funcf12_(int *a,char *b,void (*c)(int, char, MyType *a), int cl0)
{
return Funcf12(*a,*b,c);
}
int  funcf13_()
{
return Funcf13();
}
#if defined(__cplusplus)
}
#endif
