/* /home/bsmith/petsc/src/snes/src/snes.c */
/* Fortran interface file for sun4 */
int  snessetfromoptions_(SNES snes){
return SNESSetFromOptions(*snes);
}
int  snesprinthelp_(SNES snes){
return SNESPrintHelp(*snes);
}
int  snessetapplicationcontext_(SNES snes,void *usrP){
return SNESSetApplicationContext(*snes,
	(void* )*((int*)usrP));
}
int  snesgetapplicationcontext_(SNES snes,void **usrP){
return SNESGetApplicationContext(*snes,
	(void* )*((int*)usrP));
}
int  snesgetiterationnumber_(SNES snes,int* iter){
return SNESGetIterationNumber(*snes,
	(int* )*((int*)iter));
}
int  snesgetfunctionnorm_(SNES snes,Scalar *fnorm){
return SNESGetFunctionNorm(*snes,
	(Scalar* )*((int*)fnorm));
}
int  snesgetnumberunsuccessfulsteps_(SNES snes,int* nfails){
return SNESGetNumberUnsuccessfulSteps(*snes,
	(int* )*((int*)nfails));
}
int  snesgetsles_(SNES snes,SLES *sles){
return SNESGetSLES(*snes,
	(SLES* )*((int*)sles));
}
int  snescreate_(MPI_Comm comm,SNES *outsnes){
return SNESCreate(*comm,
	(SNES* )*((int*)outsnes));
}
int  snessetjacobian_(SNES snes,Mat A,Mat B,int (*func)(SNESS,Vec,Mat*,M*,
MatStructure*,v*),v*ctx){
return SNESSetJacobian(*snes,*A,*B,
	(int* )*((int*)func),*Vec,
	(Mat* )*((int*),),*,,
	(MatStructure* )*((int*),),*,,*ctx);
}
int  snessetup_(SNES snes){
return SNESSetUp(*snes);
}
int  snesdestroy_(SNES snes){
return SNESDestroy(*snes);
}
int  snessetmaxiterations_(SNES snes,int *maxits){
return SNESSetMaxIterations(*snes,*maxits);
}
int  snessetmaxfunctionevaluations_(SNES snes,int *maxf){
return SNESSetMaxFunctionEvaluations(*snes,*maxf);
}
int  snessetrelativetolerance_(SNES snes,double *rtol){
return SNESSetRelativeTolerance(*snes,*rtol);
}
int  snessetabsolutetolerance_(SNES snes,double *atol){
return SNESSetAbsoluteTolerance(*snes,*atol);
}
int  snessettruncationtolerance_(SNES snes,double *tol){
return SNESSetTruncationTolerance(*snes,*tol);
}
int  snessetsolutiontolerance_(SNES snes,double *tol){
return SNESSetSolutionTolerance(*snes,*tol);
}
int  snessetsolution_(SNES snes,Vec x,int (*func)(SNESS,Vec,void*),v*ctx){
return SNESSetSolution(*snes,*x,
	(int* )*((int*)func),*Vec,
	(void* )*((int*),),*ctx);
}
int  snesscalestep_(SNES snes,Vec y,double *fnorm,double *delta,
                  double *gpnorm,double *ynorm){
return SNESScaleStep(*snes,*y,
	(double* )*((int*)fnorm),
	(double* )*((int*)delta),
	(double* )*((int*)gpnorm),
	(double* )*((int*)ynorm));
}
int  snessolve_(SNES snes,int *its){
return SNESSolve(*snes,
	(int* )*((int*)its));
}
int  snessetmethod_(SNES snes,SNESMethod method){
return SNESSetMethod(*snes,*method);
}
int  snesregister_(int *name,char *sname,int (*create)(SNESS))
{
  int *ierr;
  if (!__NLList_) {i= NRCreate(&&__NLList);Cierr){
return SNESRegister(*name,
	(char* )*((int*)sname),
	(int* )*((int*)create),*
,*ierr,*!,*{,*NRCreate,*;,*ierr);
}
int  snesregisterdestroy_()
{
if (__NLList){
    NRDestroy( __NLList );
    __NLList =0;
  }
return 0;
}

/*
   SNESGetMethodFromOptions_Private -Sthe selectedmfrom the
options database.

   Input parameters:
.  ctx-the SNESc

   Output Parameter:
.  method-solver method

   Returns:
Returns 1ithe methodifound;0otherwise.

   Options DatabaseK:
$-snes_method  method
*/
int *SNESGetMethodFromOptions_Private(SNESSctx,S*method){
return SNESRegisterDestroy(*{,*__NLList,*__NLList,*=,*},*0,*/,*-,*selected,*the,*database,*parameters,*ctx,*SNES,*Parameter,*method,*method,*:,*1,*method,*;,*.,*Database,*$,*method,*/,*SNESGetMethodFromOptions_Private,*,,*method);
}
int  snesgetsolution_(SNES snes,Vec *x){
return SNESGetSolution(*snes,
	(Vec* )*((int*)x));
}
int  snesgetsolutionupdate_(SNES snes,Vec *x){
return SNESGetSolutionUpdate(*snes,
	(Vec* )*((int*)x));
}
int  snesgetfunction_(SNES snes,Vec *r){
return SNESGetFunction(*snes,
	(Vec* )*((int*)r));
}
