/* tstc.c */
/* Fortran interface file */

/*
* This file was generated automatically by bfort from the C source
* file.  
 */
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
#if defined(__cplusplus)
}
#endif
