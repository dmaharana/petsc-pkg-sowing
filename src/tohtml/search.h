#ifndef __SRLINK
#define __SRLINK

#ifdef ANSI_ARGS
#undef ANSI_ARGS
#endif
#ifdef __STDC__
#define ANSI_ARGS(a) a
#else
#define ANSI_ARGS(a) ()
#endif

typedef struct _link {
	char *topicname, *entryname;
	int  number;
	int  refcount;
	void *priv;                 /* Pointer to user-specified data */
	struct _link *next, *prev;
    } LINK;

typedef struct {
    LINK *head, *tail;
    LINK **HASH;
    int hashsize;
    int NodeNumber;		/* Current number of nodes */
    } SRList;

extern SRList *SRCreate ANSI_ARGS((void));
extern LINK   *SRInsert ANSI_ARGS(( SRList *, char *, char *, int * )), 
              *SRLookup ANSI_ARGS(( SRList *, char *, char *, int * ));
extern LINK   *SRInsertAllowDuplicates ANSI_ARGS(( SRList *, char *, 
						   char *, int * ));
extern void   SRDestroy ANSI_ARGS(( SRList * ));
extern void   SRDump    ANSI_ARGS(( SRList *, FILE * ));
extern void   SRDumpAll ANSI_ARGS(( SRList *, FILE * ));
#endif
