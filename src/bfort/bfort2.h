#ifndef BFORT2_H_INCLUDED
#define BFORT2_H_INCLUDED

#include <ctype.h>
#include "sowing.h"
#include "doc.h"
#include "patchlevel.h"
#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifndef MAX_PATH_NAME
#define MAX_PATH_NAME 1024
#endif

typedef enum { ARG_INOUT, ARG_IN, ARG_OUT } arguse_t;
/* Typedefs for reading, processing functions */
typedef struct arg_t {
    char *typeStr;      /* String containing type */
    char *origName;     /* Original name of argument */
    char *origDecl;     /* Original declaration of argument - includes
		 	  modifiers such as [] or function args */
    char *baseType;     /* String containing the base type (e.g., without any
			   [] or * modifiers */
    char *actualarg;    /* If non-null, pointer to name to use when calling
			   C function (allows a substitute) */
    int  isAddress;     /* true if the C argument is an address (instead of
			   a value) */
    int  nIndirects;    /* Count of the number of * in the declaration -
                           the -mnative option (multiple-native-pointer)
                           uses this */
    arguse_t inout;     /* Whether arg is known to be only IN or only OUT */
    int  isNative;      /* The type in C is "native" in Fortran - no
			   transformation is required. */
    /* Routines that are applied for each use of the argument */
    const char *methodName;                    /* Name of the arg handling
						  approach; used for debugging*/
    void (*ftocarg)(FILE *, struct arg_t *);   /* Formal arg in definition */
    void (*ftocdecl)(FILE *, struct arg_t *);  /* Declaration for processing */
    void (*ftoc)(FILE *, struct arg_t *);      /* Processing before call */
    void (*ftoccall)(FILE *, struct arg_t *);  /* Output actual arg in call
						  to C routine */
    void (*ctof)(FILE *, struct arg_t *);      /* Processing after call */
    void (*free)(struct arg_t *);              /* Free for any routine args,
						  including argextra */
    /* Any special values needed by the functions above */
    void *argextra;
    int  varId;         /* Some values require a temp.  A non-negative value
			   should be used for temp variables. */
    struct arg_t *next; /* Pointer to next argument */
} arg_t;

typedef struct func_t {
    char *typeStr;       /* String containing type */
    char *origName;      /* Original name of function */
    arg_t *arglist;      /* Argument list */
    int   isFunc;        /* Set to false if no return value */
    struct func_t *next; /* Pointer to next function */
} func_t;

/* Catch serious problems */
#define MAX_ERR 100
extern int ErrCnt;
#define ERRABORT \
    do { if (ErrCnt > MAX_ERR) { ABORT("Too many errors");} } while(0)

#define ABORT(msg) Abort(msg,__FILE__,__LINE__)

/* Debugging for development */
extern int Debug;
#define DBG(a)    do {if (Debug) { fprintf(stderr, a);} }  while (0)
#define DBG2(a,b) do {if (Debug) { fprintf(stderr, a,b);} } while (0)

/* Defaults for these are "ierr" and "__ierr" */
extern const char *errArgNameParm;
extern const char *errArgNameLocal;
extern int useFerr;

int f90ModuleInit(int *argc_ptr, char **argv);
int f90OutputIfaces(func_t *fp);

void Abort(const char *msg, const char *file, int line);
int MPIU_Strncpy(char *, const char *, size_t);

#endif
