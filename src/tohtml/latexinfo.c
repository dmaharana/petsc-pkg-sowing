#include <stdio.h>
#include <ctype.h>
#include "sowing.h"
#include "search.h"
#include "tex.h"

extern LINK *GetNextLink ( LINK * );
extern SRList *topicctx;
typedef enum { TXITEMIZE, TXDESCRIPTION, TXEXAMPLE, TXVERBATIM, TXENUMERATE,
	       TXLIST } 
        EnvType;
typedef struct {
    EnvType env;
    int    num;            /* relative number of an item in this
			      environment (needed for enumerate) */
    void    (*newline)( FILE *);   
                           /* routine to call for new-line handling */
    char   *p1, *p2;       /* Pointers to text for use by the environment
                              (some have code and replacement text; 
			      principly a user-defined list environment */
    /* These are currently unused */
    char   *label_node_name; /* Name of the node */
    char   *label_text;      /* Text to use in refering to the label */
    } LaTeXStack;
extern LaTeXStack lstack[];
extern int lSp;

/* 
   This file contains the latexinfo commands 
 */

/* latexinfo defines xref and pxref which use the section names rather 
   than labels.  */
void TXxref( e )
TeXEntry *e;
{
    LINK *RefedSection;
    int  dummy;
    char lfname[256];
    char *topicfile;

    if (!InDocument) return;	
    if (DebugCommands)
	fprintf( stdout, "Getting argument for %s\n", e->name );
    PUSHCURTOK;
    if (TeXGetArg( fpin[curfile], curtok, MAX_TOKEN ) == -1) 
	TeXAbort( "TXxref", e->name );
    ReplaceWhite( curtok );
    RefedSection = SRLookup( topicctx, curtok, (char *)0, &dummy );
    if (!RefedSection) {
	fprintf( stderr, "Could not find %s in topicctx\n", curtok );
	POPCURTOK;
	return;
    }
    TeXoutstr( fpout, "See section " );
    topicfile =  TopicFilename( RefedSection );
    if (topicfile) 
	sprintf( lfname, "%s#Node", topicfile );
    else
	strcpy( lfname, "Node" );
    WritePointerText( fpout, curtok, lfname, RefedSection->number );
    POPCURTOK;
}	

void TXpxref( e )
TeXEntry *e;
{
    LINK *RefedSection;
    int  dummy;
    char lfname[256];
    char *topicfile;

    if (!InDocument) return;	
    if (DebugCommands)
	fprintf( stdout, "Getting argument for %s\n", e->name );
    PUSHCURTOK;
    if (TeXGetArg( fpin[curfile], curtok, MAX_TOKEN ) == -1) 
	TeXAbort( "TXpxref", e->name );
    ReplaceWhite( curtok );
    RefedSection = SRLookup( topicctx, curtok, (char *)0, &dummy );
    if (!RefedSection) {
	fprintf( stderr, "Could not find %s in topicctx\n", curtok );
	POPCURTOK;
	return;
    }
    TeXoutstr( fpout, "see section " );
    topicfile = TopicFilename( RefedSection );
    if (topicfile)
	sprintf( lfname, "%s#Node", topicfile );
    else
	strcpy( lfname, "Node" );
    WritePointerText( fpout, curtok, lfname, RefedSection->number );
    POPCURTOK;
}	

/* This processes the LatexInfo "example" environment */
void TeXexample( e )
TeXEntry *e;
{
/* Skip code until find an \end{example} */
    TXWriteStartNewLine( fpout );
    TXpreformated( fpout, 1 );
    TXbgroup( e );
    TXfont_tt( e );
    lstack[++lSp].env	    = TXEXAMPLE;
    lstack[lSp].newline	    = TXWriteStartNewLine;
    lstack[lSp].label_node_name = 0;
    lstack[lSp].label_text	    = 0;

    /* Example is a verbatim environment, even the TeX comment character
       should be ignored (it isn't now) */
    PushCommentChar( '\0' );
    TeXskipEnv( e, "example", 1 );
    PopCommentChar();
    TXegroup( e );
    TXpreformated( fpout, 0 );
    lSp--;
}   

void TeXiftex( e )
TeXEntry *e;
{
/* Skip code until find an \end{iftex} */	
    TeXskipEnv( e, "iftex", 0 );
}   

void TeXtex( e )
TeXEntry *e;
{
/* Skip code until find an \end{tex} */	
    TeXskipEnv( e, "tex", 0 );
}   

void TeXmenu( e )
TeXEntry *e;
{
    TeXskipEnv( e, "menu", 0 );
}	

