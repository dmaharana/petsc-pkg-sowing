#include <stdio.h>
#include <ctype.h>
#include "sowing.h"
#include "search.h"
#include "tex.h"

extern LINK *GetNextLink ( LINK * );
extern SRList *topicctx;
extern LaTeXStack lstack[];
extern int lSp;

/* 
   This file contains the latexinfo commands 
 */

/* latexinfo defines xref and pxref which use the section names rather 
   than labels.  */
void TXxref( TeXEntry *e )
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

void TXpxref( TeXEntry *e )
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
void TeXexample( TeXEntry *e )
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
    lstack[lSp].line_num = LineNo[curfile];

    /* Example is a verbatim-like environment, even the TeX comment character
       should be ignored (it isn't now) */
    /* However, some operations should be processed, including macros and
       {} */
    PushCommentChar( '\0' );
    TeXskipEnv( e, "example", 1 );
    PopCommentChar();
    TXegroup( e );
    TXpreformated( fpout, 0 );
    lSp--;
}   

void TeXiftex( TeXEntry *e )
{
/* Skip code until find an \end{iftex} */	
    TeXskipEnv( e, "iftex", 0 );
}   

void TeXtex( TeXEntry *e )
{
/* Skip code until find an \end{tex} */	
    TeXskipEnv( e, "tex", 0 );
}   

void TeXmenu( TeXEntry *e )
{
    TeXskipEnv( e, "menu", 0 );
}	

