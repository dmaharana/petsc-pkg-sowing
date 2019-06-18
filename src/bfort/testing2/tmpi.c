/* This is a test case for generating fortran wrappers.
   This version contains routines that return int, allowing tests of the
   option to return value as a parameter (-ferr option)

   This version tests handling of MPI objects, including the use of the
   MPI CtoF and FtoC routines for converting handles.
 */

/*@ Funcf_1 - some MPI-1 objects
  @*/
int Funcf_1(MPI_Comm c1, MPI_Comm *c_2, MPI_Request v1[], MPI_Op *ops)
{
}
/*@ FuncF_2 - more MPI-1 objects
  @*/
int FuncF_2(MPI_Datatype d1[], MPI_Errhandler ee)
{
}

/*@ Funcf3 - Some MPI-2 objects
  @*/
int Funcf3(MPI_File *f1, MPI_File f2, MPI_Info ii[], MPI_Win *win1)
{
}

/*@ Funcf4 - Some MPI-3 objects
  @*/
int Funcf4(MPI_Message *m1, MPI_Message m2)
{
}

/*@ Funcf5 - Some MPI special types
  @*/
int Funcf5(MPI_Aint ar1, MPI_Count cr1, MPI_Aint *arptr, MPI_Count *crptr,
       MPI_Offset of1, MPI_Offset *ofptr, MPI_Fint f1, MPI_Fint fv[])
{
}

/*@ Funcf6 - MPI Status
  @*/
int Funcf6(MPI_Status *s1, MPI_Status ar[], const MPI_Status *s2)
{
}
