/* ANSI Test program for bfort */
/*@ simple - test
 @*/
int simple(int a, double b)
{
}
/*@ foo1 - test 
 @*/
void foo1( int a, char b, void (*c)() )
{
}
/*@ foo1a - test 
 @*/
void foo1a( int a, char b, void (*c)(int, char, MyType *a) )
{
}
/*@ foo2 - test 
@*/
struct tm *foo2( MyType *a, short b )
{
}
/*@ foo3 - test
@*/
int foo3()
{
}
/*@ foo3a - test
  ANSI should use this for empty arg
@*/
int foo3a(void)
{
}
