/* tstpgm.h */
/* Fortran interface file for sun4 */
void  foo1_( a, b, c)
int *a;
char b;
void (*c)();
{
foo1(*a,*b,c);
}
struct tm * foo2_( a, b)
MyType *a;
short *b;
{
return foo2(
	(MyType* )*((int*)a),*b);
}
int  foo3_()
{
return foo3();
}
