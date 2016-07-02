/*E
   Read_t - Enum describing read types
 E*/
typedef enum { foo, bar, last } Read_t;

/*E
   Write_t - Enum describing write types
 E*/
typedef enum { foo, bar=1, last = 3 } Write_t;

/*E
  Norm_t - Enum describing norm types
  E*/
typedef enum {
    NORM1 = -1 ,
    NORM2,       /* Some comment */
    NORM_1_OR_2, // Another comment
    NORM_I = +3,
    NORM_Inf,
} Norm_t;

/*S
  File_t - Structure describing files

+I foo1 - First struct item
.  buf1 - Second struct item
-  readfn - Last struct item


S*/
typedef struct { int foo1; char *buf1;
	int (*readfn)( int, void * );
	} File_t;
