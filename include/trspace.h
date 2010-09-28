#ifndef _TR_SPACE
#define _TR_SPACE

#include <stdlib.h>

extern void trinit( int );
extern void trSummary( FILE * );
extern void trid( int );
extern void trlevel( int );
extern void trpush( int );
extern void trpop( void );
extern void trdumpGrouped( FILE * );
extern void TrSetMaxMem( int );
extern void TrInit( void );
extern void *trmalloc (unsigned int, int, char *), 
            trfree (void *, int, char *),
            trspace (int *, int *), 
            trdump (FILE *), 
            *trcalloc (unsigned int, unsigned int, int, char * ),
            *trrealloc (void *, int, int, char * );
extern char *trstrdup( const char *, int, const char * );
extern int  trvalid (char *);
extern void trDebugLevel ( int );

#ifdef MEMORY_TRACING
#define MALLOC(a)    trmalloc((unsigned)(a),__LINE__,__FILE__)
#define FREE(a)      trfree((char *)(a),__LINE__,__FILE__)
#define STRDUP(a)    trstrdup(a,__LINE__,__FILE__)

#if defined(MEMORY_TRACING_REQUIRED) && !defined(TRSPACE_SOURCE)
#define malloc ERROR("Use TR versions of malloc")
#define free   ERROR("Use TR versions of free")
#define calloc ERROR("Use TR versions of calloc")
#define strdup ERROR("Use TR versions of strdup")
#endif /* test on memory tracing required */

#else

#define MALLOC(a)    malloc((unsigned)(a))
#define FREE(a)      free((char *)(a))
#define STRDUP(a)    strdup(a)
#endif
#define NEW(a)    (a *)MALLOC(sizeof(a))
#define CHKPTR(p)        {if (!(p)) {fprintf(stderr, "Out of memory\n");return;}}
#define CHKPTRN(p)       {if (!(p)) {fprintf(stderr, "Out of memory\n");return 0;}}
#endif /* Test on memory tracing */
