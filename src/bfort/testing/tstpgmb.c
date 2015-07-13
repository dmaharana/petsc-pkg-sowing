/* ISO-C  Test program for bfort, with arg names */
/*@ simple - test
 @*/
int simple(int nrows, double result)
{
}
/*@ foo1 - test
 @*/
void foo1( int nr, char nc, void (*func)() )
{
}
/*@ foo1a - test
 @*/
void foo1a( int nr, char nc, void (*func2)(int, char, MyType *a) )
{
}
/*@ foo2 - test
@*/
struct tm *foo2(MyType *input_type, short Cval )
{
}
/*@ foo3 - test
@*/
int foo3()
{
}
/*@ foo3a - test
  ISO-C code should use this for empty arg
@*/
int foo3a(void)
{
}
