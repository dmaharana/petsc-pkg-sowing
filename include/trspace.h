#ifndef _TR_SPACE
#define _TR_SPACE

#include <stdlib.h>

#ifdef ANSI_ARGS
#undef ANSI_ARGS
#endif
#ifdef __STDC__
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif

#ifdef MEMORY_TRACING
extern void *trmalloc ANSI_ARGS((unsigned int, int, char *)), 
            trfree ANSI_ARGS((void *, int, char *)),
            trspace ANSI_ARGS((int *, int *)), 
            trdump ANSI_ARGS((FILE *)), 
            *trcalloc ANSI_ARGS((unsigned int, unsigned int, int, char * )),
            *trrealloc ANSI_ARGS((void *, int, int, char * ));
extern int  trvalid ANSI_ARGS((char *));
extern void trDebugLevel ANSI_ARGS(( int ));

#define MALLOC(a)    trmalloc((unsigned)(a),__LINE__,__FILE__)
#define FREE(a)      trfree((char *)(a),__LINE__,__FILE__)

#else

#define MALLOC(a)    malloc((unsigned)(a))
#define FREE(a)      free((char *)(a))
#endif
#define NEW(a)    (a *)MALLOC(sizeof(a))
#define CHKPTR(p)        {if (!(p)) {fprintf(stderr, "Out of memory\n");return;}}
#define CHKPTRN(p)       {if (!(p)) {fprintf(stderr, "Out of memory\n");return 0;}}
#endif
