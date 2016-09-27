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


/* Definitions of Fortran Wrapper routines */
#if defined(__cplusplus)
extern "C" {
#endif
void getstr_(int *nc,char *a, int *__z, cl0){
*__z = getstr(*nc,a);
}
void getstr2_(int *nc,char a[], int *__z, cl0){
*__z = getstr2(*nc,a);
}
#if defined(__cplusplus)
}
#endif
