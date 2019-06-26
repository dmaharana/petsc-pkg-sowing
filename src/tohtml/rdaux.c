/* -*- Mode: C; c-basic-offset:4 ; -*- */
#include <stdio.h>
#include "tex.h"
#include "sowing.h"
#include "search.h"

static FILE *fpaux = 0;
static char fname[1024];
static int debugContents = 0;

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
    LINK     *Self;  /* Points to the master entry in the SRList */
    char     *name;  /* These could be gotten from Self, but this is easier */
    char     *fname; /* Filename that contains the reference */
    int      level;
    int      visitCount; /* Sanity check */
    } Contents;
extern void PrintContents ( FILE *, Contents * );
extern void WriteSubChildren ( FILE *, Contents *, int );
static Contents *NewContents( int, const char * );
void WriteSiblings( FILE *, Contents *, int );


#define MAX_DEPTH 7
static int  CurLevel = 0;
static int  RootLevel = -1;
static int  FirstLevel = -1;
static Contents *root = 0;
static Contents *(parents[MAX_DEPTH]);
static Contents *(lastchild[MAX_DEPTH]);
static Contents *lastnode = 0;

static int FirstNode = 0;   /* 0 For Chapter, 1 for Section */
static int FirstNodeSeen = 0;

void OpenAuxFile( const char *name )
{
    int i;
    if (debugContents) {
	fprintf( stderr, "Opening aux file %s\n", name );
    }
    fpaux = fopen( name, "r" );
    if (SafeStrncpy( fname, name, sizeof(fname) )) {
	TeXAbort( "OpenAuxFile", "file name too long" );
    }
    /* Just in case */
    for (i=0; i<MAX_DEPTH; i++) {
	parents[i] = lastchild[i] = NULL;
    }
}

void OpenWRAuxFile( void )
{
    if (debugContents) {
	fprintf( stderr, "Opening output aux file %s\n", fname );
    }
    if (fpaux) fclose( fpaux );
    fpaux = fopen( fname, "w" );
    if (fpaux) {
	fputs( "TEXAUX\n", fpaux );
#if 0
	/* This is a hack.  If, after reading the previous AUX file, we noticed
	   an out-of-range entry, we add a special command to set the initial
	   root */
	if (FirstLevel > RootLevel) {
	    fprintf( fpaux, "0#node1.htm %d Top\n", RootLevel-1 );
	}
#endif
    }
}

void RewindAuxFile( void )
{
    fseek( fpaux, 0l, 0 );
}

/*
 * Read the hux (aux) file.  This file has lines of the form
 *   number#file level title
 * i.e.,
 *   27#node27.htm 3 A Section title
 * or
 *   -1#node27.htm 4 label-name A Subsection title
 *
 * A -1 indicates that the item occurs with a file at a given anchor
 *    (label-name)
 *
 * The purpose of this code is to build a rooted tree of the sections.
 * A well formed list of sections has the following properties:
 *   a) All levels are greater than or equal to the first level encountered
 *   b) A level is either:
 *       i) the same as the previous level (a sibling)
 *       ii) one greater than the previous level (a child)
 *       iii) less than the previous level (a sibling of some previous level)
 * However, some files are not well formed - for example, a level may be
 * more than one greater than the previous level.  There are several
 * ways to handle that case. More below
 *
 * To ensure that there is a unique root, all levels are incremented by 1,
 * and the "rlevel" stores this value.  If there is no root yet, then
 * a root at level 0 is created.
 */
void RdAuxFile( SRList *topicctx )
{
    int         err;
    char        buffer[1024];
    char        lfname[1024];
    int         seqnum, level, rlevel, dummy;
    char        entrylevel[1024];
    LINK        *l;
    Contents    *c;
    Contents    *LastRead = 0;

    if (debugContents) {
	fprintf( stderr, "Reading auxfile\n" );
    }

/* Default value of entrylevel */
    if (SafeStrncpy( entrylevel, "Document", sizeof(entrylevel) )) {
	TeXAbort( "RdAuxFile", "Problem setting Document name" );
    }
    if (!fpaux) return;
    fgets( buffer, sizeof(buffer), fpaux );
    if (strcmp( buffer, "TEXAUX\n" ) != 0) {
	/* Invalid aux file */
	fprintf( stderr, "AUX file missing TEXAUX header\n" );
	return;
    }
    while (1) {
	err = fscanf( fpaux, "%d#%s %d", &seqnum, lfname, &level );
	if (err < 0) {
	    /* This means end-of-file, which is not an error */
	    break;
	}
	err = SYTxtGetLine( fpaux, buffer, sizeof(buffer) );
	if (err < 0) {
	    fprintf( stderr, "Error getting text line from buffer=%s\n",
		     buffer );
	    break;
	}
	/* labels and bib entries are intermixed within the aux file with
	   the chapter/section structure */
	if (seqnum == -1) {
	    InsertLabel( level, buffer, lfname );
        }
	else if (seqnum == -2) {
	    InsertBib( level, buffer );
	}
	else {
	    /* Ensure buffer is null-terminated */
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
	    c = NewContents( level, lfname );
	    l->priv    = (void *)c;
	    c->Self    = l;
	    c->name    = STRDUP(buffer);
	    CHKPTR(c->name);

	    /* Keep track of items in the order in which they were read */
	    c->Prev    = LastRead;
	    if (LastRead) LastRead->Next = c;
	    LastRead   = c;

	    rlevel = level + 1;    /* We need to make all levels greater than 0
				      so that multiple 0 levels can be handled */
	    if (rlevel < RootLevel) {
		int i;
		fprintf( stderr, "AUX file contains out-of-order elements\n" );
		fprintf( stderr, "rlevel = %d, RootLevel = %d\n",
			 rlevel, RootLevel );
		fprintf( stderr, "Line: %s\n", buffer );
		/* Update the root.  Create one if there isn't one already */
		if (!root) {
		    Contents    *cr;
		    if (debugContents)
			fprintf( stderr, "Creating a new root\n" );
		    cr = NewContents( rlevel, lfname );
		    l->priv     = 0;
		    cr->Self    = l;
		    cr->name    = STRDUP( "**ROOT**" ); /* temp */
		    CHKPTR(cr->name);
		    root        = cr;
		}
		/* */
		for (i=rlevel;i<RootLevel;i++) {
		    /* */
		    if (parents[i] == 0) {
			fprintf( stderr, "PANIC! Level %d has no parent\n", i );
			exit(1);
		    }
		    if (parents[i-1] == 0) {
			fprintf( stderr, "PANIC! level %d-1 has no parent\n", i );
			exit(1);
		    }
		    parents[i-1]->Child = parents[i];
		    lastchild[i-1]      = parents[i];
		}
		RootLevel = rlevel;
		/* The FIRST sibling of the new root must be the old root */
		parents[rlevel-1]->Sibling = root;
		root      = parents[rlevel-1];
	    }
	    /* FIXME: This originally had a level != 0 here.  Why?  */
	    if (!FirstNodeSeen && level != 0) {
		/* This code attempts to determine the "top level"
		   by using the first value seen.  If there are higher
		   (lower-numbered) levels, then this will give an
		   incorrect result.  To guard against that, we check
		   against the root level*/
		if (debugContents) {
		    fprintf( stderr, "Setting first node to level %d\n",
			     level );
		}
		FirstNode     = level;
		FirstNodeSeen = 1;
		CurLevel      = level;
		FirstLevel    = level;
		RootLevel     = level;
	    }
	    if (level == FirstNode && !root) {
		int i;
		if (debugContents) {
		    fprintf( stderr, "Adding root for level %d\n", level );
		}
		/* Fill in the parent array in case there are higher-level
		   entries */
		for (i=0; i<=level; i++) {
		    root = NewContents( -1, lfname );
		    CHKPTR( root );
		    parents[i]   = root;
		    lastchild[i] = 0;
		    if (i > 0 && i < level) {
			lastchild[i-1] = parents[i];
		    }
		}
	    }
	    if (debugContents) {
		fprintf( stderr, "rlevel = %d, CurLevel = %d, name = %s\n",
			 rlevel, CurLevel, c->name );
	    }

	    /* This is the code that sets the sibling and child links of
	     *previous* nodes to this node as appropriate */
	    if (rlevel == CurLevel) {
		/* This node is on the current level */
		parents[CurLevel] = c; 		/* parents is always the most
						   recent node at the given
						   level */
		if (lastchild[rlevel-1])
		    (lastchild[rlevel-1])->Sibling = c;
		else
		    (parents[rlevel-1])->Child = c;
		lastchild[rlevel-1] = c;
	    }
	    else if (rlevel < CurLevel) {
		/* This node is a parent of the current level */
		if (parents[rlevel-1]) {
		    /* ??? is this code correct? */
		    if (parents[rlevel-1]->Child) {
			if (lastchild[rlevel-1])
			    lastchild[rlevel-1]->Sibling = c;
		    }
		    else
			parents[rlevel-1]->Child = c;
		}
		parents[rlevel]     = c;
		lastchild[rlevel]   = 0;
		lastchild[rlevel-1] = c;
	    }
	    else if (rlevel == CurLevel + 1) {
		/* This cnode is a child of the current level */
		if (parents[rlevel-1]) {
		    if (!parents[rlevel-1]->Child)
			parents[rlevel-1]->Child = c;
		    else {
			if (lastchild[rlevel-1]) {
			    lastchild[rlevel-1]->Sibling = c;
			}
		    }
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

void WRtoauxfile( int seqnum, char *lfname, int level, char *text )
{
    if (!fpaux) return;
    fprintf( fpaux, "%d#%s %d %s", seqnum, lfname, level, text );
    if (text[strlen(text)-1] != '\n') fputs( "\n", fpaux );
}

/* Write a label to the aux file */
void WriteLabeltoauxfile( int seqnum, char *lfname, char *labelname,
			  char *nodename )
{
    if (!fpaux) return;
    fprintf( fpaux, "-1#%s %d %s %s\n", lfname, seqnum, labelname, nodename );
}

/* Write a bib entry to the aux file */
void WriteBibtoauxfile( int seqnum, char *labelname, char *nodename )
{
    if (!fpaux) return;
    fprintf( fpaux, "-2# %d %s %s\n", seqnum, labelname, nodename );
}


/* Write FROM the aux file, using all names of the given level.
   Write as a "jump" table */
void WRfromauxfile( FILE *fout, int level )
{
    int  err;
    char buffer[1024];
    int  seqnum, ilevel;
    char fullname[1024];
    char lfname[1024];

    if (!fpaux) return;
    RewindAuxFile();
    while (1) {
	err = fscanf( fpaux, "%d#%s %d", &seqnum, lfname, &ilevel );
	if (err < 0) break;
	err = SYTxtGetLine( fpaux, buffer, sizeof(buffer) );
	if (err < 0) break;
	if (level == ilevel) {
	    if (buffer[strlen(buffer)-1] == '\n')
		buffer[strlen(buffer)-1] = 0;
	    if (lfname[0])
		sprintf( fullname, "%s#Node",
			 (lfname[0] == '#') ? lfname + 1 : lfname );
	    else {
		if (SafeStrncpy( fullname, "Node", sizeof(fullname) )) {
		    TeXAbort( "WRfromauxfile", "Unable to set fullname" );
		}
	    }
	    WritePointerText( fout, buffer, fullname, seqnum );
	    WritePar( fout );
        }
    }

}

/*
  For debugging, this routine follows the tree of Contents and prints
  them out.
 */
void PrintContents( FILE *fp, Contents *r )
{
    int indent, i;
    static char indentString[100];

    if (!r) { r = root; indent = 0; }

    indent = r->level;
    for (i=0; i<indent; i++) {
	indentString[i] = ' ';
    }
    indentString[i] = 0;

    if (r->level >= 0)
	if (r->name)
	    fprintf( fp, "%s%3d: %s\n", indentString, r->level, r->name );
    if (r->Child) {
	fputs( indentString, fp );
	fputs( "Children...\n", fp );
	PrintContents( fp, r->Child );
    }
    if (r->Sibling) {
	fputs( indentString, fp );
	fputs( "Siblings...\n", fp );
	PrintContents( fp, r->Sibling );
    }
}
void PrintAllContents(FILE *fp)
{
    PrintContents(fp, 0);
}

/* A debugging routine to print just the siblings, not the children
   of the siblings */
static void PrintSiblings( FILE *, Contents * );

static void PrintSiblings( FILE *fp, Contents *r )
{
    int maxcount = 256;
    fprintf( fp, "Sibling list:\n" );
    while (r && maxcount--) {
	fprintf( fp, "\t%d:%s\n", r->level, r->name ? r->name : "NULL" );
	r = r->Sibling;
    }
    if (maxcount == 0)
	fprintf( fp, "\t**exceeded max sibling count\n" );
}
/* Get the NUMBER of childen of the given node */
int  NumChildren( LINK *l )
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

/* Write a menu for the SIBLINGS and their CHILDREN of a given contents
   pointer
 */
#ifndef MAX_CONTENTS_ENTRIES
#define MAX_CONTENTS_ENTRIES 2048
#endif
static int elementCount = 0;
void WriteSiblings( FILE *fout, Contents *r, int depth )
{
    if (!r) return;

    if (debugContents) {
	PrintSiblings( stderr, r );
    }
    TXbmenu( fout );
    while (r) {
	if (debugContents) {
	    fprintf( stderr, "Push sibling entry %s (level %d)\n",
		     r->name, r->level );
	}
	if (r->level <= depth) {
	    /* Handle the case where the Self entry is empty */
	    if (r->Self) {
		char fullname[1024];
		elementCount ++;
		if (elementCount > MAX_CONTENTS_ENTRIES) {
		    fprintf( stderr, "Too many contents elements (%d)\n",
			     elementCount );
		    exit(1);
		}
		WriteBeginPointerMenu( fout );
		if (r->fname)
		    sprintf( fullname, "%s#Node", r->fname );
		else {
		    if (SafeStrncpy( fullname, "Node", sizeof(fullname) )) {
			TeXAbort( "WriteSiblings", "Unable to set fullname" );
		    }
		}
		WritePointerText( fout, r->name, fullname, r->Self->number );
		WriteEndOfPointer( fout );
	    }
	    WriteSubChildren( fout, r, depth );
        }
	r = r->Sibling;
    }
    TXemenu( fout );
}

/* Write a menu for the CHILDREN of a given contents pointer */
void WriteSubChildren( FILE *fout, Contents *r, int depth )
{
    if (!r || !r->Child) return;
    r = r->Child;
    if (debugContents) {
	fprintf( stderr, "Push entry %s (level %d)\n",
		 r->name, r->level );
    }
    WriteSiblings( fout, r, depth );
}

/* Given a link from SRLookup, write pointer text for the immediate children
   (if depth == -1) or children to level depth (depth >= 0) or ALL children
   (depth == 100) */
void WriteChildren( FILE *fout, LINK *l, int depth )
{
    Contents *r;

    if (l == 0) {
	r = root;
	if (!r) {
	    fprintf( stderr, "No Root!\n" );
	    return;
	}
	elementCount = 0;
	/* Note that the root *can* now have siblings - in the case where
	   the document has parts.  However, in this case we must first
	   write out the siblings */
	/* if (r->Sibling) fprintf( stderr, "Root can't have siblings!\n" ); */
	if (r->Sibling) {
	    WriteSiblings( fout, r->Sibling, depth );
	}
    }
    else        r = (Contents *) l->priv;

    if (!r) {
	fprintf( stderr, "Could not get contents for %s\n", l->topicname );
	return;
    }

    WriteSubChildren( fout, r, depth );
}

/* Look up name; if found, return the name in context-ref of the PARENT.  */
 int GetParent( LINK *l, const char *name, char *context, char *title,
		int titlelen )
{
    Contents *c = (Contents *)(l->priv);
    Contents *parent;

    if (debugContents) {
	fprintf( stderr, "Looking for parent of link %s\n", l->topicname );
    }
    if (!c) return 0;
    parent = c->Parent;
    if (!parent) return 0;
    if (!parent->name || !parent->Self) return 0;

    if (SafeStrncpy( title, parent->name, titlelen )) {
	TeXAbort( "GetParent", "title too long" );
    }
    if (parent->fname)
	sprintf( context, "%s#Node%d", parent->fname, parent->Self->number );
    else
	sprintf( context, "Node%d", parent->Self->number );

    if (debugContents) {
	fprintf( stderr, "Found parent %s\n", parent->name );
    }
    return 1;
}

 int GetNext( LINK *l, const char *name, char *context, char *title,
	      int titlelen )
{
    Contents *c = (Contents *)(l->priv);
    Contents *nbr;

    nbr = c->Next;
    if (!nbr) return 0;
    if (!nbr->name || !nbr->Self) return 0;

    if (SafeStrncpy( title, nbr->name, titlelen )) {
	TeXAbort( "GetNext", "Title too long" );
    }
    if (nbr->fname)
	sprintf( context, "%s#Node%d", nbr->fname, nbr->Self->number );
    else
	sprintf( context, "Node%d", nbr->Self->number );

    return 1;
}

/*
 * This routine can be used to move sequentially through the list of sections.
 * Note that some link are placeholders and have no "self".  In that case,
 * we take in order:
 *   The next sibling
 *   The next child
 */
LINK *GetNextLink( LINK *l )
{
    Contents *c;

    if (l) {
	c = (Contents *)l->priv;
	if (!c || !c->Next) return 0;
	if (c->Next->Self) return c->Next->Self;
    }
    /* If the link is empth, start at the root */
    if (root) {
	Contents *r = root;
	while (r) {
	    if (r->Self) return r->Self;
	    if (r->Sibling) r = r->Sibling;
	    else if (r->Child) r = r->Child;
	    else break;
	}
    }
    return 0;
}

int GetPrevious( LINK *l, const char *name, char *context, char *title,
		 int titlelen )
{
    Contents *c = (Contents *)(l->priv);
    Contents *nbr;

    nbr = c->Prev;
    if (!nbr) return 0;
    if (!nbr->name || !nbr->Self) return 0;

    if (SafeStrncpy( title, nbr->name, titlelen) ) {
	TeXAbort( "GetPrevious", "title too long" );
    }
    sprintf( context, "%s#Node%d", nbr->fname, nbr->Self->number );

    return 1;
}

/* Given a link (returned by SRLookup), give the filename for that topic */
char *TopicFilename( LINK *l )
{
    Contents *c;

    c = (Contents *)(l->priv);
    return c->fname;
}

/* Create and safely initialize a contents element.
   Generally initialized with null/zero elements
 */
static Contents *NewContents( int level, const char *lfname )
{
    Contents *c;
    c             = NEW(Contents);   CHKPTRN(c);
    /* Link for contents */
    c->Parent     = 0;
    c->Sibling    = 0;
    c->Child      = 0;
    c->Self       = 0;

    /* Name of node */
    c->name       = 0;

    /* File where found */
    c->fname      = (char *)MALLOC( strlen( lfname ) + 1 );
    CHKPTRN(c->fname);
    if (SafeStrncpy( c->fname,
		     (lfname[0] == '#') ? lfname + 1 : lfname,
		     strlen(lfname) + 1 )) {
	TeXAbort( "NewContents", "filename too long" );
    }

    /* Level in contents */
    c->level      = level;

    /* Linear list of entries */
    c->Prev       = 0;
    c->Next       = 0;

    /* Sanity count */
    c->visitCount = 1;

    return c;
}
