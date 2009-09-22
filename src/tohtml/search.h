#ifndef __SRLINK
#define __SRLINK

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

extern SRList *SRCreate (void);
extern LINK   *SRInsert ( SRList *, const char *, char *, int * ), 
              *SRLookup ( SRList *, const char *, char *, int * );
extern LINK   *SRInsertAllowDuplicates ( SRList *, const char *, char *,
					 int * );
extern void   SRDestroy ( SRList * );
extern void   SRDump    ( SRList *, FILE * );
extern void   SRDumpAll ( SRList *, FILE * );
#endif
