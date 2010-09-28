/*@
    foobar - Simple routine
@*/
int foobar( MPI_File f, int a, double b )
{
/* do stuff */
}
/*@
  barfoo - Using const
@*/
int barfoo( const int f[], const int *a, const int b )
{
/* do more stuff */
}
/*@
  barlong - a routine with many, long arguments
  @*/
int barlong( const int aLongArrayName[], const int *alongArgumentName, 
	     double *anotherLongArgumentName, int *thisIsAnotherLongName )
{
  /* do more stuff */
}
/*@
  mixedargs - a routine with arguments that are not distinct in type
  and ones that use void types
  @*/
double mixedargs( const int M, const int m, void *p, void * restrict c )
{
  /* do yet more stuff */
}
/*@
  mixedargs2 - a routine with arguments that are not distinct in type
  and ones that use void types
  @*/
double mixedargs2( int Mlonger, const int M, float m1, const int m, void *p, 
		   void * restrict c )
{
  /* do yet more stuff */
}
