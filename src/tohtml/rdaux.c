#include <stdio.h>
#include "tex.h"
#include "sowing.h"
#include "search.h"

static FILE *fpaux = 0;
static char fname[100];

/* 
    This file contains code for reading an aux file and inserting it into
    the search space

    The format is
        number#file level name
    #file is optional; this is used for documents split across multiple
    files.
    If number is < 0, then the entry is
    	-1 nodenumber name topicname
    where name is the the value of a \label command, topicname is the
    corresponding topic, and nodenumber is the corresponding node

        -2 name bibreference

    is reserved for handling bibliographies.

    In order to distinguish between other aux files and ours, the first line
    is TEXAUX\n.

    We also insert links for the children and siblings of a node.
 */

/* We keep track of the current link and last child at a level

   parents[i]   = most recent node at level i
   lastchild[i] = most recent CHILD of parents[i]
   lastnode     = most recent node

   The algorithm is:
   if (level == cur_level) set lastchild (add link)
   elseif (level < cur_level) pop until at previous level, add as last child,
                              push as parent
   elseif (level == cur_level+1) set as last child, push as parent
   else   error (level > cur_level + 1)

   One problem is that some documents start with chapters, others with 
   sections.  If the FIRST node is a section, this is the base node.
 */
typedef struct _Contents {
    struct _Contents *Parent, *Sibling, *Child, *Next, *Prev;
    LINK     *Self;
    char     *name;  /* These could be gotten from Self, but this is easier */
    char     *fname; /* Filename that contains the reference */
    int      level;
    } Contents;
extern void PrintContents ( FILE *, Contents * );
extern void WriteSubChildren ( FILE *, Contents *, int );

#define MAX_DEPTH 7
static int  CurLevel = 0;
static Contents *root = 0;
static Contents *(parents[MAX_DEPTH]);
static Contents *(lastchild[MAX_DEPTH]);
static Contents *lastnode = 0;

static int FirstNode = 0;    /* 0 For Chapter, 1 For Section */
static int FirstNodeSeen = 0;

void OpenAuxFile( name )
char *name;
{
    fpaux = fopen( name, "r" );
    strcpy( fname, name );
}

void OpenWRAuxFile()
{
    if (fpaux) fclose( fpaux );
    fpaux = fopen( fname, "w" );
    if (fpaux) 
	fputs( "TEXAUX\n", fpaux );
}

void RewindAuxFile()
{
    fseek( fpaux, 0l, 0 );
}
 
void RdAuxFile( topicctx )
SRList *topicctx;
{
    int         err;
    char        buffer[256];
    char        lfname[256];
    int         seqnum, level, rlevel, dummy;
    char entrylevel[256];
    LINK        *l;
    Contents    *c;
    Contents    *LastRead = 0;

/* Default value of entrylevel */
    strcpy( entrylevel, "Document");
    if (!fpaux) return;	
    fgets( buffer, 256, fpaux );
    if (strcmp( buffer, "TEXAUX\n" ) != 0) {
	/* Invalid aux file */
	return;
    }
    while (1) {
	err = fscanf( fpaux, "%d#%s %d", &seqnum, lfname, &level );
	if (err < 0) break;
	err = SYTxtGetLine( fpaux, buffer, 256 );
	if (err < 0) break;
	if (seqnum == -1) {
	    InsertLabel( level, buffer, lfname );
        }
	else if (seqnum == -2) {
	    InsertBib( level, buffer );
	}
	else {
	    if (buffer[strlen(buffer)-1] == '\n')
		buffer[strlen(buffer)-1] = 0;
	    /* We must NOT do a lookup when we insert this into the
	       table, because if we have multiple items, they must be
	       inserted separately.  Thus, if a value already exists, 
	       we still create a new one.
	       */
	    l = SRInsertAllowDuplicates( topicctx, buffer, entrylevel, &dummy );
	    l->number = seqnum;
	    /* fprintf( stdout, "Inserting |%s|\n", buffer ); */
	    /* We want to add links to the neighbors ... */
	    c = NEW(Contents);   CHKPTR(c);
	    l->priv = (void *)c;
	    c->Parent  = 0;
	    c->Sibling = 0;
	    c->Child   = 0;
	    c->Self    = l;
	    c->name    = (char *)MALLOC( strlen(buffer) + 1 );
	    CHKPTR(c->name);
	    c->fname   = (char *)MALLOC( strlen( lfname ) + 1 );
	    CHKPTR(c->fname);
	    strcpy( c->fname, (lfname[0] == '#') ? lfname + 1 : lfname );
	    c->level   = level;
	    c->Prev    = LastRead;
	    c->Next    = 0;
	    if (LastRead) LastRead->Next = c;
	    LastRead   = c;
	    strcpy( c->name, buffer );
	    rlevel = level + 1;    /* We need to make all levels greater than 0
				      so that multiple 0 levels can be handled */
	    if (!FirstNodeSeen && level != 0) {
		FirstNode = level;
		FirstNodeSeen = 1;
		CurLevel  = level;
	    }
	    if (level == FirstNode && !root) {
		root = NEW(Contents);
		parents[level]   = root;
		lastchild[level] = 0;
		root->Sibling = 0;
		root->Parent  = 0;
		root->Child   = 0;
		root->Next    = 0;
		root->Prev    = 0;
		root->name    = 0;
		root->level   = -1;
		root->Self    = 0;
		root->fname   = 0;
	    }
	    if (rlevel == CurLevel) {
		parents[CurLevel] = c;
		if (lastchild[rlevel-1])
		    (lastchild[rlevel-1])->Sibling = c;
		else
		    (parents[rlevel-1])->Child = c;
		lastchild[rlevel-1] = c;
	    }
	    else if (rlevel < CurLevel) {
		if (parents[rlevel-1]) {
		    /* ??? is this code correct? */
		    if (parents[rlevel-1]->Child)
			lastchild[rlevel-1]->Sibling = c;
		    else
			parents[rlevel-1]->Child = c;
		}
		parents[rlevel]   = c;
		lastchild[rlevel] = 0;
		lastchild[rlevel-1] = c;
	    }
	    else if (rlevel == CurLevel + 1) {
		if (parents[rlevel-1]) {
		    if (!parents[rlevel-1]->Child)
			parents[rlevel-1]->Child = c;
		    else 
			lastchild[rlevel-1]->Sibling = c;
		}
		lastchild[rlevel-1] = c;
		parents[rlevel]     = c;
	    }
	    else {
		fprintf( stderr, "Section %s not properly nested\n", buffer );
	    }
	    CurLevel = rlevel;
/*	printf( "adding node at %d, parents[%d] = %d\n", level, CurLevel, 
	parents[CurLevel-1] );
	*/
	    c->Parent = (CurLevel > FirstNode) ? parents[CurLevel-1] : 0;
/* 	printf( "c->Parent = %d\n", c->Parent ); */
	    lastnode = c;
        }
    }    
}

void WRtoauxfile( seqnum, fname, level, text )
int  seqnum, level;
char *fname;
char *text;
{
    if (!fpaux) return;
    fprintf( fpaux, "%d#%s %d %s", seqnum, fname, level, text );
    if (text[strlen(text)-1] != '\n') fputs( "\n", fpaux );
}
/* Write a label to the aux file */
void WriteLabeltoauxfile( seqnum, fname, labelname, nodename )
int  seqnum;
char *labelname, *nodename;
char *fname;
{
    if (!fpaux) return;
    fprintf( fpaux, "-1#%s %d %s %s\n", fname, seqnum, labelname, nodename );
}

/* Write a bib entry to the aux file */
void WriteBibtoauxfile( seqnum, labelname, nodename )
int  seqnum;
char *labelname, *nodename;
{
    if (!fpaux) return;
    fprintf( fpaux, "-2# %d %s %s\n", seqnum, labelname, nodename );
}
	

/* Write FROM the aux file, using all names of the given level.
   Write as a "jump" table */
void WRfromauxfile( fout, level )
FILE *fout;
int  level;
{
    int  err;
    char buffer[256];
    int  seqnum, ilevel;
    char fullname[256];
    char lfname[256];

    if (!fpaux) return;
    RewindAuxFile();
    while (1) {
	err = fscanf( fpaux, "%d#%s %d", &seqnum, lfname, &ilevel );
	if (err < 0) break;
	err = SYTxtGetLine( fpaux, buffer, 256 );
	if (err < 0) break;
	if (level == ilevel) {
	    if (buffer[strlen(buffer)-1] == '\n')
		buffer[strlen(buffer)-1] = 0;
	    if (lfname)
		sprintf( fullname, "%s#Node", 
			 (lfname[0] == '#') ? lfname + 1 : lfname );
	    else
		strcpy( fullname, "Node" );
	    WritePointerText( fout, buffer, fullname, seqnum );
	    WritePar( fout );
        }
    }    

}

/*
  For debugging, this routine follows the tree of Contents and prints 
  them out.
 */
void PrintContents( fp, r )
FILE     *fp;
Contents *r;
{
    if (!r) r = root;

    if (r->level >= 0)
	fprintf( fp, "%d: %s\n", r->level, r->name );
    if (r->Child) {
	fputs( "Children...\n", fp );
	PrintContents( fp, r->Child );
    }
    if (r->Sibling) {
	fputs( "Siblings...\n", fp );
	PrintContents( fp, r->Sibling );
    }
}

/* Get the NUMBER of childen of the given node */
int  NumChildren( l )
LINK *l;
{
    Contents *r;
    int      cnt = 0;

    if (l == 0) {
	r = root;
	if (!r) {
	    return 0;
	}
    }
    else        r = (Contents *) l->priv;
    if (r) {
	r = r->Child;
	while (r) {
	    cnt++;
	    r = r->Sibling;
	}
    }
    return cnt;
}

/* Write a menu for the CHILDREN of a given contents pointer */
void WriteSubChildren( fout, r, depth )
FILE     *fout;
Contents *r;
int      depth;
{
    if (!r || !r->Child) return;	
    TXbmenu( fout );
    r = r->Child;
    while (r) {
	if (r->level <= depth) {
	    WriteBeginPointerMenu( fout );
	    {
		char fullname[256];
		if (r->fname)
		    sprintf( fullname, "%s#Node", r->fname );
		else
		    strcpy( fullname, "Node" );
		WritePointerText( fout, r->name, fullname, r->Self->number );
	    }
	    WriteEndOfPointer( fout );
	    WriteSubChildren( fout, r, depth );
        }
	r = r->Sibling;
    }
    TXemenu( fout );    
}    	    

/* Given a link from SRLookup, write pointer text for the immediate children
   (if depth == -1) or children to level depth (depth >= 0) or ALL children
   (depth == 100) */
void WriteChildren( fout, l, depth )
FILE *fout;
LINK *l;
int  depth;
{
    Contents *r;

    if (l == 0) {
	r = root;
	if (!r) {
	    fprintf( stderr, "No Root!\n" );
	    return;
	}
	if (r->Sibling) fprintf( stderr, "Root can't have siblings!\n" );
    }
    else        r = (Contents *) l->priv;
    if (!r) {
	fprintf( stderr, "Could not get contents for %s\n", l->topicname );
	return;
    }

    WriteSubChildren( fout, r, depth );    
}

/* Look up name; if found, return the name in context-ref of the PARENT.  */
int GetParent( l, name, context, title )
LINK *l;
char *name, *context, *title;
{
    Contents *c = (Contents *)(l->priv);
    Contents *parent;

    if (!c) return 0;
    parent = c->Parent;
    if (!parent) return 0;
    if (!parent->name || !parent->Self) return 0;

    strcpy( title, parent->name );
    if (parent->fname)
	sprintf( context, "%s#Node%d", parent->fname, parent->Self->number );
    else
	sprintf( context, "Node%d", parent->Self->number );

    return 1;
}

int GetNext( l, name, context, title )
LINK *l;
char *name, *context, *title;
{
    Contents *c = (Contents *)(l->priv);
    Contents *nbr;

    nbr = c->Next;
    if (!nbr) return 0;
    if (!nbr->name || !nbr->Self) return 0;

    strcpy( title, nbr->name );
    if (nbr->fname)
	sprintf( context, "%s#Node%d", nbr->fname, nbr->Self->number );
    else
	sprintf( context, "Node%d", nbr->Self->number );

    return 1;
}

/* 
 * This routine can be used to move sequentially through the list of sections.
 */
LINK *GetNextLink( l )
LINK *l;
{
    Contents *c;

    if (l) {
	c = (Contents *)l->priv;
	if (!c || !c->Next) return 0;
	return c->Next->Self;
    }
    if (root) {
	if (root->Self) return root->Self;
	return root->Child->Self;
    }
    return 0;
}

int GetPrevious( l, name, context, title )
LINK *l;
char *name, *context, *title;
{
    Contents *c = (Contents *)(l->priv);
    Contents *nbr;

    nbr = c->Prev;
    if (!nbr) return 0;
    if (!nbr->name || !nbr->Self) return 0;

    strcpy( title, nbr->name );
    sprintf( context, "%s#Node%d", nbr->fname, nbr->Self->number );

    return 1;
}

/* Given a link (returned by SRLookup), give the filename for that topic */
char *TopicFilename( l )
LINK *l;
{
    Contents *c;

    c = (Contents *)(l->priv);
    return c->fname;
}
