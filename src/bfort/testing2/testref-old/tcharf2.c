/* tchar.c */
/* Fortran interface file */

/*
* This file was generated automatically by bfort from the C source
* file.  
 */
/* Function prototypes */
int Getstr(int nc, char *a);
int Getstr2(int nc, char a[]);
int Putstr(int nc, const char *a);
int Mstring(void *buf, int len, char *n1, const char *n2, int l3, char *n_4);


#ifdef FORTRANCAPS
#define getstr_ GETSTR
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define getstr_ getstr
#endif
#ifdef FORTRANCAPS
#define getstr2_ GETSTR2
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define getstr2_ getstr2
#endif
#ifdef FORTRANCAPS
#define putstr_ PUTSTR
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define putstr_ putstr
#endif
#ifdef FORTRANCAPS
#define mstring_ MSTRING
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define mstring_ mstring
#endif


/* Definitions of Fortran Wrapper routines */
#if defined(__cplusplus)
extern "C" {
#endif
void getstr_(int *nc,char *a, int *__z, int cl0)
{
*__z = Getstr(*nc,a);
}
void getstr2_(int *nc,char a[], int *__z, int cl0)
{
*__z = Getstr2(*nc,a);
}
void putstr_(int *nc, char *a, int *__z, int cl0)
{
*__z = Putstr(*nc,a);
}
void mstring_(void*buf,int *len,char *n1, char *n2,int *l3,char *n_4, int *__z, int cl0, int cl1, int cl2)
{
*__z = Mstring(buf,*len,n1,n2,*l3,n_4);
}
#if defined(__cplusplus)
}
#endif
