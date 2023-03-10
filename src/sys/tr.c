#define TRSPACE_SOURCE
#include "sowing.h"
#ifndef MEMORY_TRACING
#define MEMORY_TRACING 1
#endif
#include <stdio.h>
#include <string.h>
#define _TR_SOURCE

#if HAVE_STDLIB_H
#include <stdlib.h>
#else
#ifdef __STDC__
extern void 	*calloc(/*size_t, size_t*/);
extern void	free(/*void * */);
extern void	*malloc(/*size_t*/);
#else
extern char *malloc();
extern char *calloc();
extern int free();
#endif
#endif

#undef TRSPACE

/*D
    trspace - Routines for tracing space usage

    Description:
    trmalloc replaces malloc and trfree replaces free.
    These routines
    have the same syntax and semantics as the routines that they replace,
    In addition, there are routines to report statistics on the memory
    usage, and to report the currently allocated space.  These routines
    are built on top of malloc and free, and can be used together with
    them as long as any space allocated with trmalloc is only freed with
    trfree.

    Note that the malloced data is scrubbed each time; you don't get
    random trash (or fortuitous zeros).  What you get is fc (bytes);
    this will usually create a "bad" value.

    As an aid in developing codes, a maximum memory threshold can
    be set with TrSetMaxMem.
 D*/

/* HEADER_DOUBLES is the number of doubles in a trSPACE header */
/* We have to be careful about alignment rules here */
#if defined(POINTER_64_BITS)
#define TR_ALIGN_BYTES 8
#define TR_ALIGN_MASK  0x7
#define TR_FNAME_LEN   16
#define HEADER_DOUBLES 12
#else
#define TR_ALIGN_BYTES 4
#define TR_ALIGN_MASK  0x3
#define TR_FNAME_LEN   12
#define HEADER_DOUBLES 8
#endif

#define COOKIE_VALUE   0xf0e0d0c9
#define ALREADY_FREED  0x0f0e0d9c

typedef struct _trSPACE {
    unsigned long   size;
    int             id;
    int             lineno;
    char            fname[TR_FNAME_LEN];
    int             freed_lineno;
    char            freed_fname[TR_FNAME_LEN];
    unsigned long   cookie;
    struct _trSPACE *next, *prev;
    } TRSPACE;
/* This union is used to insure that the block passed to the user is
   aligned on a double boundary */
typedef union {
    TRSPACE sp;
    double  v[HEADER_DOUBLES];
    } TrSPACE;

/*
 * This package maintains some state about itself.  These globals hold
 * this information.
 */
static int     world_rank = -1;
static long    allocated = 0, frags = 0;
static TRSPACE *TRhead = 0;
static int     TRid = 0;
static int     TRlevel = 0;
#define MAX_TR_STACK 20
static int     TRstack[MAX_TR_STACK];
static int     TRstackp = 0;
static int     TRdebugLevel = 0;
#define TR_MALLOC 0x1
#define TR_FREE   0x2

/* Used to keep track of allocations */
static long    TRMaxMem = 0;
static long    TRMaxMemId = 0;
/* Used to limit allocation */
static long    TRMaxMemAllow = 0;

/*+C
   trinit - Setup the space package.  Only needed for
   error messages and flags.
+*/
void trinit( int rank )
{
    world_rank = rank;
}

/*+C
    trmalloc - Malloc with tracing

    Input Parameters:
.   a   - number of bytes to allocate
.   lineno - line number where used.  Use __LINE__ for this
.   fname  - file name where used.  Use __FILE__ for this

    Returns:
    double aligned pointer to requested storage, or null if not
    available.
 +*/
void *trmalloc( unsigned int a, int lineno, char *fname )
{
    TRSPACE          *head;
    char             *new;
    unsigned long    *nend;
    unsigned int     nsize;
    int              l;

    if (TRdebugLevel > 0) {
	char buf[256];
	sprintf( buf, "Invalid MALLOC arena detected at line %d in %s\n",
		 lineno, fname );
	if (trvalid( buf )) return 0;
    }

    nsize = a;
    if (nsize & TR_ALIGN_MASK)
	nsize += (TR_ALIGN_BYTES - (nsize & TR_ALIGN_MASK));
    if ((long)(allocated + nsize) > TRMaxMemAllow && TRMaxMemAllow) {
	/* Return a null when memory would be exhausted */
	fprintf( stderr, "Exceeded allowed memory! \n" );
	return 0;
    }

    new = malloc( (unsigned)( nsize + sizeof(TrSPACE) + sizeof(unsigned long) ) );
    if (!new) return 0;

    memset( new, 0xfc, nsize + sizeof(TrSPACE) + sizeof(unsigned long) );
    head = (TRSPACE *)new;
    new  += sizeof(TrSPACE);

    if (TRhead)
	TRhead->prev = head;
    head->next     = TRhead;
    TRhead         = head;
    head->prev     = 0;
    head->size     = nsize;
    head->id       = TRid;
    head->lineno   = lineno;
    if ((l = (int)strlen( fname )) > TR_FNAME_LEN-1 )
	fname += (l - (TR_FNAME_LEN-1));
    strncpy( head->fname, fname, (TR_FNAME_LEN-1) );
    head->fname[TR_FNAME_LEN-1]= 0;
    head->cookie   = COOKIE_VALUE;
    nend           = (unsigned long *)(new + nsize);
    nend[0]        = COOKIE_VALUE;

    allocated += nsize;
    if (allocated > TRMaxMem) {
	TRMaxMem   = allocated;
	TRMaxMemId = TRid;
    }
    frags     ++;

    if (TRlevel & TR_MALLOC)
	fprintf( stderr, "[%d] Allocating %d bytes at %lx in %s:%d\n",
		 world_rank, a, (long)new, fname, lineno );
    return (void *)new;
}

/*+C
   trfree - Free with tracing

   Input Parameters:
.  a    - pointer to a block allocated with trmalloc
.  line - line in file where called
.  file - Name of file where called
 +*/
void trfree( void *a_ptr, int line, char *file )
{
    TRSPACE  *head;
    char     *ahead;
    char     *a = (char *)a_ptr;
    unsigned long *nend;
    int      l, nset;

/* Don't try to handle empty blocks */
    if (!a) return;

    if (TRdebugLevel > 0) {
	if (trvalid( "Invalid MALLOC arena detected by FREE" )) return;
    }

    ahead = a;
    a     = a - sizeof(TrSPACE);
    head  = (TRSPACE *)a;
    if (head->cookie != COOKIE_VALUE) {
	/* Damaged header */
	fprintf( stderr, "[%d] Block at address %lx is corrupted; cannot free;\n\
may be block not allocated with trmalloc or MALLOC\n\
called in %s at line %d\n", world_rank, (long)a, file, line );
	return;
    }
    nend = (unsigned long *)(ahead + head->size);
/* Check that nend is properly aligned */
    if ((sizeof(long) == 4 && ((long)nend & 0x3) != 0) ||
	(sizeof(long) == 8 && ((long)nend & 0x7) != 0)) {
	fprintf( stderr,
 "[%d] Block at address %lx is corrupted (invalid address or header)\n\
called in %s at line %d\n", world_rank, (long)a + sizeof(TrSPACE),
		 file, line );
	return;
    }
    if (*nend != COOKIE_VALUE) {
	if (*nend == ALREADY_FREED) {
	    fprintf( stderr,
		     "[%d] Block [id=%d(%lu)] at address %lx was already freed\n",
		     world_rank, head->id, head->size, (long)a + sizeof(TrSPACE) );
	    head->fname[TR_FNAME_LEN-1]	  = 0;  /* Just in case */
	    head->freed_fname[TR_FNAME_LEN-1] = 0;  /* Just in case */
	    fprintf( stderr,
		     "[%d] Block freed in %s[%d]\n", world_rank, head->freed_fname,
		     head->freed_lineno );
	    fprintf( stderr,
		     "[%d] Block allocated at %s[%d]\n",
		     world_rank, head->fname, head->lineno );
	    return;
	}
	else {
	    /* Damaged tail */
	    fprintf( stderr,
		     "[%d] Block [id=%d(%lu)] at address %lx is corrupted (probably write past end)\n",
		     world_rank, head->id, head->size, (long)a );
	    head->fname[TR_FNAME_LEN-1]= 0;  /* Just in case */
	    fprintf( stderr,
		     "[%d] Block allocated in %s[%d]\n", world_rank,
		     head->fname, head->lineno );
	}
    }
/* Mark the location freed */
    *nend		   = ALREADY_FREED;
    head->freed_lineno = line;
    if ((l = (int)strlen( file )) > TR_FNAME_LEN-1 )
	file += (l - (TR_FNAME_LEN-1));
    strncpy( head->freed_fname, file, (TR_FNAME_LEN-1) );

    allocated -= head->size;
    frags     --;
    if (head->prev)
	head->prev->next = head->next;
    else
	TRhead = head->next;

    if (head->next)
	head->next->prev = head->prev;
    if (TRlevel & TR_FREE)
	fprintf( stderr, "[%d] Freeing %lu bytes at %lx in %s:%d\n",
		 world_rank, head->size, (long)a + sizeof(TrSPACE),
		 file, line );

    /*
       Now, scrub the data (except possibly the first few ints) to
       help catch access to already freed data
     */
    nset = head->size -  2 * sizeof(int);
    if (nset > 0)
	memset( ahead + 2 * sizeof(int), 0xda, nset );
    free( a );
}

/*+C
   trvalid - test the allocated blocks for validity.  This can be used to
   check for memory overwrites.

   Input Parameter:
.  str - string to write out only if an error is detected.

   Return value:
   The number of errors detected.

   Output Effect:
   Error messages are written to stdout.  These have the form of either

$   Block [id=%d(%d)] at address %lx is corrupted (probably write past end)
$   Block allocated in <filename>[<linenumber>]

   if the sentinal at the end of the block has been corrupted, and

$   Block at address %lx is corrupted

   if the sentinal at the begining of the block has been corrupted.

   The address is the actual address of the block.  The id is the
   value of TRID.

   No output is generated if there are no problems detected.
+*/
int trvalid( const char *str )
{
    TRSPACE *head;
    char    *a;
    unsigned long *nend;
    int     errs = 0;

    head = TRhead;
    while (head) {
	if (head->cookie != COOKIE_VALUE) {
	    if (!errs) fprintf( stderr, "%s\n", str );
	    errs++;
	    fprintf( stderr, "[%d] Block at address %lx is corrupted\n",
		     world_rank, (long)head );
	    /* Must stop because if head is invalid, then the data in the
	       head is probably also invalid, and using could lead to SEGV
	       or BUS
	    */
	    return errs;
	}
	a    = (char *)(((TrSPACE*)head) + 1);
	nend = (unsigned long *)(a + head->size);
	if (nend[0] != COOKIE_VALUE) {
	    if (!errs) fprintf( stderr, "%s\n", str );
	    errs++;
	    head->fname[TR_FNAME_LEN-1]= 0;  /* Just in case */
	    fprintf( stderr,
		     "[%d] Block [id=%d(%lu)] at address %lx is corrupted (probably write past end)\n",
		     world_rank, head->id, head->size, (long)a );
	    fprintf( stderr,
		     "[%d] Block allocated in %s[%d]\n",
		     world_rank, head->fname, head->lineno );
	    fprintf( stderr,
		     "[%d] Expected %x, read %lx\n",
		     world_rank, COOKIE_VALUE, nend[0] );
	}
	head = head->next;
    }
    return errs;
}

/*+C
   trspace - Return space statistics

   Output parameters:
.   space - number of bytes currently allocated
.   frags - number of blocks currently allocated
 +*/
void trspace( int *space, int *fr )
{
    /* We use ints because systems without prototypes will usually
       allow calls with ints instead of longs, leading to unexpected
       behavior */
    *space = (int)allocated;
    *fr    = (int)frags;
}

/*+C
  trdump - Dump the allocated memory blocks to a file

  Input Parameter:
.  fp  - file pointer.  If fp is NULL, stderr is assumed.
 +*/
void trdump( FILE *fp )
{
    TRSPACE *head;
    int     id;

    if (fp == 0) fp = stderr;
    head = TRhead;
    while (head) {
	fprintf( fp, "[%d] %lu at [%lx], id = ",
		 world_rank, head->size, (long)head + sizeof(TrSPACE) );
	if (head->id >= 0) {
	    head->fname[TR_FNAME_LEN-1] = 0;
	    fprintf( fp, "%d %s[%d]\n",
		     head->id, head->fname, head->lineno );
	}
	else {
	    /* Decode the package values */
	    head->fname[TR_FNAME_LEN-1] = 0;
	    id = head->id;
	    fprintf( fp, "%d %s[%d]\n",
		     id, head->fname, head->lineno );
	}
	head = head->next;
    }
/*
    fprintf( fp, "# [%d] The maximum space allocated was %ld bytes [%ld]\n",
	     world_rank, TRMaxMem, TRMaxMemId );
 */
}

/* Configure will set HAVE_SEARCH for these systems.  We assume that
   the system does NOT have search.h unless otherwise noted.
   The otherwise noted lets the non-configure approach work on our
   two major systems */
#if defined(HAVE_SEARCH_H)

/*
   Old test ...
   (!defined(__MSDOS__) && !defined(fx2800) && !defined(tc2000) && \
    !defined(NeXT) && !defined(c2mp) && !defined(intelnx) && !defined(BSD386))
 */
/* The following routine uses the tsearch routines to summarize the
   memory heap by id */
/* rs6000 and paragon needs _XOPEN_SOURCE to use tsearch */
#if (defined(rs6000) || defined(paragon)) && !defined(_XOPEN_SOURCE)
#define _NO_PROTO
#define _XOPEN_SOURCE
#endif
#if defined(HPUX) && !defined(_INCLUDE_XOPEN_SOURCE)
#define _INCLUDE_XOPEN_SOURCE
#endif
#include <search.h>
typedef struct { int id, size, lineno; char *fname; } TRINFO;
static int IntCompare( TRINFO *a, TRINFO *b )
{
    return a->id - b->id;
}
static FILE *TRFP;
/*ARGSUSED*/
static void PrintSum( TRINFO **a, VISIT order, int level )
{
    if (order == postorder || order == leaf)
	fprintf( TRFP, "[%d]%s[%d] has %d\n",
		 (*a)->id, (*a)->fname, (*a)->lineno, (*a)->size );
}

/*+C
  trSummary - Summarize the allocate memory blocks by id

  Input Parameter:
.  fp  - file pointer

  Note:
  This routine is the same as trDump on those systems that do not include
  /usr/include/search.h .
 +*/
void trSummary( FILE *fp )
{
TRSPACE *head;
TRINFO  *root, *key, **fnd;
TRINFO  nspace[1000];

root = 0;
head = TRhead;
key  = nspace;
while (head) {
    key->id     = head->id;
    key->size   = 0;
    key->lineno = head->lineno;
    key->fname  = head->fname;
#if !defined(IRIX) && !defined(solaris) && !defined(HPUX) && !defined(rs6000)
    fnd    = (TRINFO **)tsearch( (char *) key, (char **) &root, IntCompare );
#else
    fnd    = (TRINFO **)tsearch( (void *) key, (void **) &root,
				 (int (*)())IntCompare );
#endif
    if (*fnd == key) {
	key->size = 0;
	key++;
	}
    (*fnd)->size += head->size;
    head = head->next;
    }

/* Print the data */
TRFP = fp;
twalk( (char *)root, (void (*)())PrintSum );
/*
fprintf( fp, "# [%d] The maximum space allocated was %d bytes [%d]\n",
	 world_rank, TRMaxMem, TRMaxMemId );
	 */
}
#else
void trSummary( FILE *fp )
{
fprintf( fp, "# [%d] The maximum space allocated was %ld bytes [%ld]\n",
	 world_rank, TRMaxMem, TRMaxMemId );
}
#endif

/*+
  trid - set an "id" field to be used with each fragment
 +*/
void trid( int id )
{
    TRid = id;
}

/*+C
  trlevel - Set the level of output to be used by the tracing routines

  Input Parameters:
. level = 0 - notracing
. level = 1 - trace mallocs
. level = 2 - trace frees

  Note:
  You can add levels together to get combined tracing.
 +*/
void trlevel( int level )
{
    TRlevel = level;
}

/*+C
   trpush - Push an "id" value for the tracing space routines

   Input Parameters:
.  a      - value to push
+*/
void trpush( int a )
{
if (TRstackp < MAX_TR_STACK - 1)
    TRstack[++TRstackp] = a;
TRid = a;
}

/*+C
  trpop - Pop an "id" value for the tracing space routines
+*/
void trpop( void )
{
if (TRstackp > 1) {
    TRstackp--;
    TRid = TRstack[TRstackp];
    }
else
    TRid = 0;
}

/*+C
    trDebugLevel - set the level of debugging for the space management routines

    Input Parameter:
.   level - level of debugging.  Currently, either 0 (no checking) or 1
    (use trvalid at each trmalloc or trfree).
+*/
void trDebugLevel( int level )
{
    TRdebugLevel = level;
}

/*+C
    trcalloc - Calloc with tracing

    Input Parameters:
.   nelem  - number of elements to allocate
.   elsize - size of each element
.   lineno - line number where used.  Use __LINE__ for this
.   fname  - file name where used.  Use __FILE__ for this

    Returns:
    Double aligned pointer to requested storage, or null if not
    available.
 +*/
void *trcalloc( unsigned nelem, unsigned elsize, int lineno, char *fname )
{
void *p;

p = trmalloc( (unsigned)(nelem*elsize), lineno, fname );
if (p) {
    memset(p,0,nelem*elsize);
    }
return p;
}

/*+C
    trrealloc - Realloc with tracing

    Input Parameters:
.   p      - pointer to old storage
.   size   - number of bytes to allocate
.   lineno - line number where used.  Use __LINE__ for this
.   fname  - file name where used.  Use __FILE__ for this

    Returns:
    Double aligned pointer to requested storage, or null if not
    available.  This implementation ALWAYS allocates new space and copies
    the contents into the new space.
 +*/
void *trrealloc( void *p, int size, int lineno, char *fname )
{
    void    *pnew;
    char    *pa;
    int     nsize;
    TRSPACE *head;

/* We should really use the size of the old block... */
    pa   = (char *)p;
    head = (TRSPACE *)(pa - sizeof(TrSPACE));
    if (head->cookie != COOKIE_VALUE) {
	/* Damaged header */
	fprintf( stderr, "[%d] Block at address %lx is corrupted; cannot realloc;\n\
may be block not allocated with trmalloc or MALLOC\n",
		 world_rank, (long)pa );
	return 0;
    }

    pnew = trmalloc( (unsigned)size, lineno, fname );
    if (!pnew) return p;

    nsize = size;
    if (head->size < (unsigned long)nsize) nsize = (int)(head->size);
    memcpy( pnew, p, nsize );
    trfree( p, lineno, fname );
    return pnew;
}

/*+C
    trstrdup - Strdup with tracing

    Input Parameters:
.   str    - string to duplicate
.   lineno - line number where used.  Use __LINE__ for this
.   fname  - file name where used.  Use __FILE__ for this

    Returns:
    Pointer to copy of the input string.
 +*/
char *trstrdup( const char *str, int lineno, const char *fname )
{
    char *p;
    unsigned len = (unsigned)strlen( str ) + 1;

    p = (char *)trmalloc( len, lineno, (char *)fname );
    if (p) {
	memcpy( p, str, len );
	trvalid( "memcpy broke string in strdup!" );
    }
    return p;
}


#define TR_MAX_DUMP 100
/*
   The following routine attempts to give useful information about the
   memory usage when an "out-of-memory" error is encountered.  The rules are:
   If there are less than TR_MAX_DUMP blocks, output those.
   Otherwise, try to find multiple instances of the same routine/line #, and
   print a summary by number:
   file line number-of-blocks total-number-of-blocks

   We have to do a sort-in-place for this
 */

/*
  Sort by file/line number.  Do this without calling a system routine or
  allocating ANY space (space is being optimized here).

  We do this by first recursively sorting halves of the list and then
  merging them.
 */
/* Forward refs for these local routines */
TRSPACE *trImerge(TRSPACE *, TRSPACE *);
TRSPACE *trIsort(TRSPACE *, int);
void trSortBlocks(void);

/* Merge two lists, returning the head of the merged list */
TRSPACE *trImerge( TRSPACE *l1, TRSPACE *l2 )
{
TRSPACE *head = 0, *tail = 0;
int     sign;
while (l1 && l2) {
    sign = strcmp(l1->fname, l2->fname);
    if (sign > 0 || (sign == 0 && l1->lineno >= l2->lineno)) {
	if (head) tail->next = l1;
	else      head = tail = l1;
	tail = l1;
	l1   = l1->next;
	}
    else {
	if (head) tail->next = l2;
	else      head = tail = l2;
	tail = l2;
	l2   = l2->next;
	}
    }
/* Add the remaining elements to the end */
if (l1) tail->next = l1;
if (l2) tail->next = l2;

return head;
}
/* Sort head with n elements, returning the head */
TRSPACE *trIsort( TRSPACE *head, int n )
{
TRSPACE *p, *l1, *l2;
int     m, i;

if (n <= 1) return head;

/* This guarentees that m, n are both > 0 */
m = n / 2;
p = head;
for (i=0; i<m-1; i++) p = p->next;
/* p now points to the END of the first list */
l2 = p->next;
p->next = 0;
l1 = trIsort( head, m );
l2 = trIsort( l2,   n - m );
return trImerge( l1, l2 );
}

void trSortBlocks( void )
{
TRSPACE *head;
int     cnt;

head = TRhead;
cnt  = 0;
while (head) {
    cnt ++;
    head = head->next;
    }
TRhead = trIsort( TRhead, cnt );
}

/* Takes sorted input and dumps as an aggregate */
void trdumpGrouped( FILE *fp )
{
TRSPACE *head, *cur;
int     nblocks, nbytes;

if (fp == 0) fp = stderr;

trSortBlocks();
head = TRhead;
cur  = 0;
while (head) {
    cur     = head->next;
    nblocks = 1;
    nbytes  = (int)head->size;
    while (cur && strcmp(cur->fname,head->fname) == 0 &&
	   cur->lineno == head->lineno ) {
	nblocks++;
	nbytes += (int)cur->size;
	cur    = cur->next;
	}
    fprintf( fp, "[%d] File %13s line %5d: %d bytes in %d allocation%c\n",
	     world_rank, head->fname, head->lineno, nbytes, nblocks,
	     (nblocks > 1) ? 's' : ' ' );
    head = cur;
    }
fflush( fp );
}

void TrSetMaxMem( int size )
{
    TRMaxMemAllow = size;
}

void TrInit( void )
{
    char *p;
    char *v;

    /* This is a special entry point that checks an environment variable
       for the debug level.  This routine does not *need* to be called,
       but it can help */

    v = getenv("TR_VERBOSE");
    if (getenv("TR_DEBUG")) {
	trDebugLevel( 1 );
	if (v && strcmp(v,"yes")) printf( "Setting trDebugLevel to 1\n" );
    }
    p = getenv( "TR_LEVEL" );
    if (p && *p) {
	int level = atoi(p);
	if (level >= 0 && level <= 3) {
	    trlevel( level );
	    if (v && strcmp(v,"yes")) printf( "Setting trlevel to 1\n" );
	}
    }
}
