/* Test program for bfort */
/*@ foo1 - test 
 @*/
void foo1( a, b, c )
int a;
char b;
void (*c)();
{
}
/*@ foo2 - test 
@*/
struct tm *foo2( a, b )
MyType *a;
short b;
{
}
/*@ foo3 - test
@*/
int foo3()
{
}
/*@
     SYGetArchType - Return a standardized architecture type for the machine
     that is executing this routine.  This uses uname where possible,
     but may modify the name (for example, sun4 is returned for all
     sun4 types).

     Input Parameter:
     slen - length of string buffer
     Output Parameter:
.    str - string area to contain architecture name.  Should be at least 
           10 characters long.
@*/
void SYGetArchType( str, slen )
char *str;
int  slen;
{
#if defined(HAS_UNAME)
#include <sys/utsname.h>
struct utsname un;
(void)uname( &un );
#endif
}
/*@C
    foobar - test program C only

    This is used as an example of C - only (should be ignored)
@*/
int foobar( a )
void **a;
{
*a = 0;
}
