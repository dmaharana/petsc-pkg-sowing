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
