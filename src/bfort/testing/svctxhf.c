/* ../solvers/svctx.h */
/* Fortran interface file for sun4 */
void  svsetprecondmat_( ctx, bmat)
    SVctx *ctx;
    SpMat *bmat;
{
SVSetPrecondMat(
	(SVctx* )*((int*)ctx),
	(SpMat* )*((int*)bmat));
}
void  svsetup_(ctx)
     SVctx *ctx;
{
SVSetUp(
	(SVctx* )*((int*)ctx));
}
void  svdestroy_(ctx)
     SVctx *ctx;
{
SVDestroy(
	(SVctx* )*((int*)ctx));
}
void  svsetluthreshold_(ctx, threshold)
     SVctx *ctx;
     int   *threshold;
{
SVSetLUThreshold(
	(SVctx* )*((int*)ctx),*threshold);
}
void  svsetlupivoting_(ctx, pivoting)
     SVctx *ctx;
     int   *pivoting;
{
SVSetLUPivoting(
	(SVctx* )*((int*)ctx),*pivoting);
}
void  svsetaccelerator_(ctx, type)
     SVctx    *ctx;
     ITMETHOD type;
{
SVSetAccelerator(
	(SVctx* )*((int*)ctx),*type);
}
void  svsaveresidualhistory_( ctx, p, nmax)
     SVctx  *ctx;
     double *p;
     int    *nmax;
{
SVSaveResidualHistory(
	(SVctx* )*((int*)ctx),p,*nmax);
}
void  svsetssoromega_(ctx,omega)
     SVctx  *ctx;
     double *omega;
{
SVSetSSOROmega(
	(SVctx* )*((int*)ctx),*omega);
}
void  svsetilufill_(ctx, fill)
     SVctx  *ctx;
     int    *fill;
{
SVSetILUFill(
	(SVctx* )*((int*)ctx),*fill);
}
void  svsetiludroptol_(ctx, tol)
     SVctx  *ctx;
     double *tol;
{
SVSetILUDropTol(
	(SVctx* )*((int*)ctx),*tol);
}
void  svsetilupivoting_(ctx, pivoting)
     SVctx *ctx;
     int   *pivoting;
{
SVSetILUPivoting(
	(SVctx* )*((int*)ctx),*pivoting);
}
void  svsetits_(ctx, max_its)
     SVctx  *ctx;
     int    *max_its;
{
SVSetIts(
	(SVctx* )*((int*)ctx),*max_its);
}
void  svsetrelativetol_(ctx, tol)
     SVctx  *ctx;
     double *tol;
{
SVSetRelativeTol(
	(SVctx* )*((int*)ctx),*tol);
}
void  svsetabsolutetol_(ctx, tol)
     SVctx  *ctx;
     double *tol;
{
SVSetAbsoluteTol(
	(SVctx* )*((int*)ctx),*tol);
}
void  svsetgmresorthog_(ctx, fcn)
     SVctx  *ctx;
     int    (*fcn)();
{
SVSetGMRESOrthog(
	(SVctx* )*((int*)ctx),fcn);
}
void  svsetflopszero_(ctx)
     SVctx  *ctx;
{
SVSetFlopsZero(
	(SVctx* )*((int*)ctx));
}
double  svgettimesetup_( ctx)
     SVctx  *ctx;
{
return SVGetTimeSetup(
	(SVctx* )*((int*)ctx));
}
void  svresettimes_( ctx)
     SVctx *ctx;
{
SVResetTimes(
	(SVctx* )*((int*)ctx));
}
void  svsetissymmetric_( ctx, flag)
    SVctx *ctx;
    int   *flag;
{
SVSetIsSymmetric(
	(SVctx* )*((int*)ctx),*flag);
}
