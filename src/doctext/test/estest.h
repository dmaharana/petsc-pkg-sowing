/*E
   Read_t - Enum describing read types

 E*/
typedef enum { foo, bar, last } Read_t;

/*S
  File_t - Structure describing files

S*/
typedef struct { int foo; char *buf;
	int (*readfn)( int, void * );
	} File_t;
