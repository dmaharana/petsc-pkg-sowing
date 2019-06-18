/* tst3c.c */
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
int  getstr_(int *nc,char *a, int cl0)
{
return Getstr(*nc,a);
}
int  getstr2_(int *nc,char a[], int cl0)
{
return Getstr2(*nc,a);
}
int  putstr_(int *nc, char *a, int cl0)
{
return Putstr(*nc,a);
}
#if defined(__cplusplus)
}
#endif
