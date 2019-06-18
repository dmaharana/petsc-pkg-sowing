/* tmpi.c */
/* Fortran interface file */

/*
* This file was generated automatically by bfort from the C source
* file.  
 */
#include <mpi.h>
int Funcf_1(MPI_Comm c1, MPI_Comm *c_2, MPI_Request v1[], MPI_Op *ops);
int FuncF_2(MPI_Datatype d1[], MPI_Errhandler ee);
int Funcf3(MPI_File *f1, MPI_File f2, MPI_Info ii[], MPI_Win *win1);
int Funcf4(MPI_Message *m1, MPI_Message m2);
int Funcf5(MPI_Aint ar1, MPI_Count cr1, MPI_Aint *arptr, MPI_Count *crptr,
       MPI_Offset of1, MPI_Offset *ofptr, MPI_Fint f1, MPI_Fint fv[]);
int Funcf6(MPI_Status *s1, MPI_Status ar[], const MPI_Status *s2);
#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define funcf_1_ PFUNCF_1
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define funcf_1_ pfuncf_1__
#elif !defined(FORTRANUNDERSCORE)
#define funcf_1_ pfuncf_1
#else
#define funcf_1_ pfuncf_1_
#endif
#else
#ifdef FORTRANCAPS
#define funcf_1_ FUNCF_1
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define funcf_1_ funcf_1__
#elif !defined(FORTRANUNDERSCORE)
#define funcf_1_ funcf_1
#endif
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define funcf_2_ PFUNCF_2
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define funcf_2_ pfuncf_2__
#elif !defined(FORTRANUNDERSCORE)
#define funcf_2_ pfuncf_2
#else
#define funcf_2_ pfuncf_2_
#endif
#else
#ifdef FORTRANCAPS
#define funcf_2_ FUNCF_2
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define funcf_2_ funcf_2__
#elif !defined(FORTRANUNDERSCORE)
#define funcf_2_ funcf_2
#endif
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define funcf3_ PFUNCF3
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define funcf3_ pfuncf3
#else
#define funcf3_ pfuncf3_
#endif
#else
#ifdef FORTRANCAPS
#define funcf3_ FUNCF3
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define funcf3_ funcf3
#endif
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define funcf4_ PFUNCF4
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define funcf4_ pfuncf4
#else
#define funcf4_ pfuncf4_
#endif
#else
#ifdef FORTRANCAPS
#define funcf4_ FUNCF4
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define funcf4_ funcf4
#endif
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define funcf5_ PFUNCF5
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define funcf5_ pfuncf5
#else
#define funcf5_ pfuncf5_
#endif
#else
#ifdef FORTRANCAPS
#define funcf5_ FUNCF5
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define funcf5_ funcf5
#endif
#endif

#ifdef MPI_BUILD_PROFILING
#ifdef FORTRANCAPS
#define funcf6_ PFUNCF6
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define funcf6_ pfuncf6
#else
#define funcf6_ pfuncf6_
#endif
#else
#ifdef FORTRANCAPS
#define funcf6_ FUNCF6
#elif !defined(FORTRANUNDERSCORE) && !defined(FORTRANDOUBLEUNDERSCORE)
#define funcf6_ funcf6
#endif
#endif



/* Definitions of Fortran Wrapper routines */
#if defined(__cplusplus)
extern "C" {
#endif
void funcf_1_(MPI_Fint * c1,MPI_Fint * *c_2,MPI_Fint * v1[],MPI_Fint * *ops, int *__ierr)
{
*__ierr = Funcf_1(
	MPI_Comm_f2c(*(c1)),(MPI_Comm )*((int *)c_2),(MPI_Request )*((int *)v1),(MPI_Op )*((int *)ops));
}
void funcf_2_(MPI_Fint * d1[],MPI_Fint * ee, int *__ierr)
{
*__ierr = FuncF_2((MPI_Datatype )*((int *)d1),
	MPI_Errhander_f2c(*(ee)));
}
void funcf3_(MPI_Fint * *f1,MPI_Fint * f2,MPI_Fint * ii[],MPI_Fint * *win1, int *__ierr)
{
*__ierr = Funcf3((MPI_File )*((int *)f1),
	MPI_File_f2c(*(f2)),(MPI_Info )*((int *)ii),(MPI_Win )*((int *)win1));
}
void funcf4_(MPI_Fint * *m1,MPI_Fint * m2, int *__ierr)
{
*__ierr = Funcf4((MPI_Message )*((int *)m1),
	MPI_Message_f2c(*(m2)));
}
void funcf5_(MPI_Aint *ar1,MPI_Count *cr1,MPI_Aint *arptr,MPI_Count *crptr,
       MPI_Offset *of1,MPI_Offset *ofptr,MPI_Fint *f1,MPI_Fint fv[], int *__ierr)
{
*__ierr = Funcf5(*ar1,*cr1,arptr,crptr,*of1,ofptr,*f1,fv);
}
void funcf6_(MPI_Status *s1,MPI_Status ar[], MPI_Status *s2, int *__ierr)
{
*__ierr = Funcf6(s1,ar,s2);
}
#if defined(__cplusplus)
}
#endif
