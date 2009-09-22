#include <stdlib.h>
#include <stdio.h>
#include "sowing.h"
#include "search.h"

/*
   Each search entry will contain a pointer to another structure
   particular to that item.  In addition, we want to represent the
   fact that the input is organized in a tree, so symbols are linked
   together.

   2/11/97
   If there are multiple links with the same name but different levels, 
   we need to keep them straight.
 */

#define KEY_NAME_SIZE 256

#ifdef FOO
typedef struct _RTFNode {
    struct _RTFNode *sibling, *child, *parent;
    int    level;
    } RTFNode;
#endif
#define SR_ENTRY_SIZE 30    
    
/* #define DEBUG */

/* Forward defs */
static int SRHashFcn ( SRList *, char * );

static int SRHashFcn( SRList *lctx, char *name )
{
    int fcn = 0;
    while (*name) fcn += *name++;
    return fcn % lctx->hashsize;
}

SRList *SRCreate( void )
{
    SRList *new;
    int    i;

    new		= NEW(SRList);   CHKPTRN(new);
    new->head	= 0;
    new->tail	= 0;
    new->hashsize	= 511;
    new->HASH	= (LINK **) MALLOC( new->hashsize * sizeof(LINK *) );
    CHKPTRN(new->HASH);
    for (i = 0; i<new->hashsize; i++) new->HASH[i] = 0;
    new->NodeNumber	= 0;

    return new;
}

void SRDestroy( SRList *lctx )
{
    LINK *cur, *next;
    int  i;

    for (i=0; i<lctx->hashsize; i++) {
	cur = lctx->HASH[i];
	while (cur) {
	    next = cur->next;
	    FREE( cur );
	    cur = next;
	}
    }
    FREE( lctx->HASH );
    FREE( lctx );    
}

/* This needs to return a pointer to the link, OR have a routine to call
   to manage the private data area */
LINK *SRLookup( SRList *lctx, const char *topicname, char *entryname, 
		int *number )
{
    LINK *cur;
    int  match;
    char *p;
    char pcopy[KEY_NAME_SIZE];

    if (lctx == 0) return 0;

/* Remove leading, trailing blanks from topicname */
    while (*topicname == ' ') topicname++;
    strncpy( pcopy, topicname, sizeof(pcopy)-1 );
    p = pcopy + strlen(pcopy) - 1;
    while (p > pcopy && *p == ' ') p--;
    p[1] = 0;

/* First, see if the name is known */	
    cur = lctx->HASH[SRHashFcn( lctx, pcopy )];
#ifdef DEBUG
    fprintf( stderr, "Looking for topicname |%s|\n", pcopy );
#endif
    while (cur) {
#ifdef DEBUG
	fprintf( stderr, "Testing %s against %s\n", pcopy, cur->topicname );
#endif
	match = strcmp( pcopy, cur->topicname );
	if (match == 0) {
	    /* found */
#ifdef DEBUG
	    fprintf( stderr, "Found |%s| = |%s|\n", topicname, cur->entryname );
#endif
	    if (entryname)
		strcpy( entryname, cur->entryname );
	    cur->refcount++;
	    *number = cur->number;
	    return cur;
        }
	if (match > 0) {
	    /* Did not find */
	    break;
        }
	cur = cur->next;
    }
    return 0;
}

/* This needs to return a pointer to the link, OR have a routine to call
   to manage the private data area.
   In order to allow us to pass CONSTANTS in topicname, we need to 
   make a copy of it (so that we can elimiinate any potential blanks
   in the name) 
 */
LINK *SRInsert( SRList *lctx, const char *topicname, char *entryname, int *number )
{
    LINK *cur, *new, *prev;
    int  match;
    int  hidx;
    char *p;
    char pcopy[KEY_NAME_SIZE];

/* Remove leading, trailing blanks from topicname */
    while (*topicname == ' ') topicname++;
    strncpy( pcopy, topicname, sizeof(pcopy)-1 );
    p = pcopy + strlen(pcopy) - 1;
    while (p > pcopy && *p == ' ') p--;
    p[1] = 0;

/* First, see if the name is known */	
    prev = 0;
    hidx = SRHashFcn( lctx, pcopy );
    cur  = lctx->HASH[hidx];
#ifdef DEBUG
    fprintf( stderr, "Looking for topicname |%s|\n", pcopy );
#endif
    while (cur) {
#ifdef DEBUG
	fprintf( stderr, "Testing %s against %s\n", pcopy, cur->topicname );
#endif
	match = strcmp( pcopy, cur->topicname );
	if (match == 0) {
	    /* found */
#ifdef DEBUG
	    fprintf( stderr, "Found |%s| = |%s|\n", pcopy, cur->entryname );
#endif
	    if (entryname)
		strcpy( entryname, cur->entryname );
	    cur->refcount++;
	    *number = cur->number;
	    return cur;
        }
	if (match > 0) {
	    /* Did not find; insert */
	    break;
        }
	prev = cur;
	cur	 = cur->next;
    }
#ifdef DEBUG
    fprintf( stderr, "Did not find topic; inserting...\n" );
#endif    
    new = (LINK *)MALLOC( sizeof(LINK) );
    if (!new) {
	fprintf( stderr, "Out of memory in symbol table\n" );
	exit(1);
    }
    new->refcount = 0;    
    new->topicname = MALLOC( strlen(pcopy) + 1 );
    if (!new->topicname) {
	fprintf( stderr, "Out of memory in symbol table\n" );
	exit(1);
    }
    strcpy( new->topicname, pcopy );
    new->entryname = MALLOC( SR_ENTRY_SIZE );
    if (!new->entryname) {
	fprintf( stderr, "Out of memory in symbol table\n" );
	exit(1);
    }
    *number = lctx->NodeNumber;    
    new->number = lctx->NodeNumber;    
    sprintf( new->entryname, "Node" ); /* %d, lctx->NodeNumber ); */
    lctx->NodeNumber += 1;
    if (entryname)
	strcpy( entryname, new->entryname );
    new->next = cur;
    if (cur) {
#ifdef DEBUG
	fprintf( stderr, "Inserting %s ahead of %s\n",
		 new->topicname, cur->topicname );
#endif   
	new->prev = cur->prev;
	if (cur->prev) {
	    cur->prev->next = new;
        }
	else
	    lctx->HASH[hidx] = new;
	cur->prev = new;
    }
    else {
	if (!lctx->HASH[hidx]) {
#ifdef DEBUG    	
	    fprintf( stderr, "Initial insertion....\n" );
#endif
	    lctx->HASH[hidx] = new;
	    new->prev = new->next = 0;
        }
	else {
	    /* Insert at end */
#ifdef DEBUG
	    fprintf( stderr, "Inserting at end...\n" );
#endif    	
	    prev->next = new;
	    new->prev  = prev;
	    new->next  = 0;
        }
    }
    return new;    
}

/* This is just SRInsert without the lookup */
LINK *SRInsertAllowDuplicates( SRList *lctx, const char *topicname, 
			       char *entryname, int *number )
{
    LINK *cur, *new, *prev;
    int  match;
    int  hidx;
    char *p;
    char pcopy[KEY_NAME_SIZE];

/* Remove leading, trailing blanks from topicname */
    while (*topicname == ' ') topicname++;
    strncpy( pcopy, topicname, sizeof(pcopy)-1 );
    p = pcopy + strlen(pcopy) - 1;
    while (p > pcopy && *p == ' ') p--;
    p[1] = 0;

/* Add to end of list of matching found values. */
    prev = 0;
    hidx = SRHashFcn( lctx, pcopy );
    cur  = lctx->HASH[hidx];
#ifdef DEBUG
    fprintf( stderr, "Looking for topicname |%s|\n", pcopy );
#endif
    while (cur) {
#ifdef DEBUG
	fprintf( stderr, "Testing %s against %s\n", pcopy, cur->topicname );
#endif
	match = strcmp( pcopy, cur->topicname );
	if (match == 0) {
	    /* found */
#ifdef DEBUG
	    fprintf( stderr, "Found |%s| = |%s|\n", pcopy, cur->entryname );
#endif
	    /* Skip until we're at the first entry that is different */
	    prev = cur;
	    cur = cur->next;
	    while (cur && strcmp( pcopy, cur->topicname ) == 0) {
		prev = cur;
		cur  = cur->next;
	    }
	    break;
        }
	if (match > 0) {
	    /* Did not find; insert */
	    break;
        }
	prev = cur;
	cur	 = cur->next;
    }
#ifdef DEBUG
    fprintf( stderr, "Did not find topic; inserting...\n" );
#endif    
    new = (LINK *)MALLOC( sizeof(LINK) );
    if (!new) {
	fprintf( stderr, "Out of memory in symbol table\n" );
	exit(1);
    }
    new->refcount = 0;    
    new->topicname = MALLOC( strlen(pcopy) + 1 );
    if (!new->topicname) {
	fprintf( stderr, "Out of memory in symbol table\n" );
	exit(1);
    }
    strcpy( new->topicname, pcopy );
    new->entryname = MALLOC( SR_ENTRY_SIZE );
    if (!new->entryname) {
	fprintf( stderr, "Out of memory in symbol table\n" );
	exit(1);
    }
    *number = lctx->NodeNumber;    
    new->number = lctx->NodeNumber;    
    sprintf( new->entryname, "Node" ); /* %d, lctx->NodeNumber ); */
    lctx->NodeNumber += 1;
    if (entryname)
	strcpy( entryname, new->entryname );
    new->next = cur;
    if (cur) {
#ifdef DEBUG
	fprintf( stderr, "Inserting %s ahead of %s\n",
		 new->topicname, cur->topicname );
#endif   
	new->prev = cur->prev;
	if (cur->prev) {
	    cur->prev->next = new;
        }
	else
	    lctx->HASH[hidx] = new;
	cur->prev = new;
    }
    else {
	if (!lctx->HASH[hidx]) {
#ifdef DEBUG    	
	    fprintf( stderr, "Initial insertion....\n" );
#endif
	    lctx->HASH[hidx] = new;
	    new->prev = new->next = 0;
        }
	else {
	    /* Insert at end */
#ifdef DEBUG
	    fprintf( stderr, "Inserting at end...\n" );
#endif    	
	    prev->next = new;
	    new->prev        = prev;
	    new->next        = 0;
        }
    }
    return new;    
}


/* Dump the names of all objects referenced only once */
void SRDump( SRList *lctx, FILE *fp )
{
    LINK *cur;
    int  i;

    for (i=0; i<lctx->hashsize; i++) {
	cur = lctx->HASH[i];
    
	while (cur) {
	    if (cur->refcount < 2) {
		fprintf( fp, "Entry |%s| only ref'ed once\n", cur->topicname );
	    }	
	    cur = cur->next;
	}
    }
}

/* Dump the names of all objects */
void SRDumpAll( SRList *lctx, FILE *fp )
{
    LINK *cur;
    int  i;

    for (i=0; i<lctx->hashsize; i++) {
	cur = lctx->HASH[i];
	while (cur) {
	    fprintf( fp, "Key %s, entry %s, number %d\n",
		     cur->topicname, cur->entryname, cur->number );
	    cur = cur->next;
	}
    }
}

