/* tmpi.c */
/* Fortran interface file */
/*
 * This file was generated automatically by bfort from the C source file
 * Major args: -ferr -mpi -anyname
 */
#include <mpi.h>
int Funcf_1(MPI_Comm c1, MPI_Comm *c_2, MPI_Request v1[], MPI_Op *ops);
int FuncF_2(MPI_Datatype d1[], MPI_Errhandler ee);
int Funcf3(MPI_File *f1, MPI_File f2, MPI_Info ii[], MPI_Win *win1);
int Funcf4(MPI_Message *m1, MPI_Message m2);
int Funcf5(MPI_Aint ar1, MPI_Count cr1, MPI_Aint *arptr, MPI_Count *crptr,
       MPI_Offset of1, MPI_Offset *ofptr, MPI_Fint f1, MPI_Fint fv[]);
int Funcf6(MPI_Status *s1, MPI_Status ar[], const MPI_Status *s2);
#ifdef FORTRANCAPS
#define funcf_1_ FUNCF_1
#define funcf_2_ FUNCF_2
#define funcf3_ FUNCF3
#define funcf4_ FUNCF4
#define funcf5_ FUNCF5
#define funcf6_ FUNCF6
#elif defined(FORTRANDOUBLEUNDERSCORE)
#define funcf_1_ funcf_1__
#define funcf_2_ funcf_2__
#elif !defined(FORTRANUNDERSCORE)
#define funcf_1_ funcf_1
#define funcf_2_ funcf_2
#define funcf3_ funcf3
#define funcf4_ funcf4
#define funcf5_ funcf5
#define funcf6_ funcf6
#endif
/* Definitions of Fortran Wrapper routines */
#if defined(__cplusplus)
extern "C" {
#endif
void funcf_1_(MPI_Fint *c1, MPI_Fint *c_2, MPI_Fint v1[], MPI_Fint *ops, int *ierr)
{
  int __ierr;
  MPI_Comm _ptmp0;
  MPI_Request _ptmp1;
  MPI_Op _ptmp2;
  __ierr = Funcf_1(MPI_Comm_f2c(*(c1)), &_ptmp0, &_ptmp1, &_ptmp2);
  *c_2 = MPI_Comm_c2f(_ptmp0);
  *v1 = MPI_Request_c2f(_ptmp1);
  *ops = MPI_Op_c2f(_ptmp2);
  *ierr = __ierr;
}
void funcf_2_(MPI_Fint d1[], MPI_Fint *ee, int *ierr)
{
  int __ierr;
  MPI_Datatype _ptmp0;
  __ierr = FuncF_2(&_ptmp0, MPI_Errhander_f2c(*(ee)));
  *d1 = MPI_Type_c2f(_ptmp0);
  *ierr = __ierr;
}
void funcf3_(MPI_Fint *f1, MPI_Fint *f2, MPI_Fint ii[], MPI_Fint *win1, int *ierr)
{
  int __ierr;
  MPI_File _ptmp0;
  MPI_Info _ptmp1;
  MPI_Win _ptmp2;
  __ierr = Funcf3(&_ptmp0, MPI_File_f2c(*(f2)), &_ptmp1, &_ptmp2);
  *f1 = MPI_File_c2f(_ptmp0);
  *ii = MPI_Info_c2f(_ptmp1);
  *win1 = MPI_Win_c2f(_ptmp2);
  *ierr = __ierr;
}
void funcf4_(MPI_Fint *m1, MPI_Fint *m2, int *ierr)
{
  int __ierr;
  MPI_Message _ptmp0;
  __ierr = Funcf4(&_ptmp0, MPI_Message_f2c(*(m2)));
  *m1 = MPI_Message_c2f(_ptmp0);
  *ierr = __ierr;
}
void funcf5_(MPI_Aint *ar1, MPI_Count *cr1, MPI_Aint *arptr, MPI_Count *crptr, MPI_Offset *of1, MPI_Offset *ofptr, MPI_Fint *f1, MPI_Fint fv[], int *ierr)
{
  *ierr = Funcf5(*ar1, *cr1, arptr, crptr, *of1, ofptr, *f1, fv);
}
void funcf6_(MPI_Status *s1, MPI_Status ar[], const MPI_Status *s2, int *ierr)
{
  *ierr = Funcf6(s1, ar, s2);
}
#if defined(__cplusplus)
 }
#endif
