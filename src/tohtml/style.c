/*
   This file contains routines to process style files; this is done by
   recognizing style files in the \documentstyle command.
 */

#include <stdio.h>
#include "sowing.h"
#include "search.h"
#include "tex.h"

/*
extern void TXcode(), TXdfn(), TXroutine(), TXfile(), TXcomment(),
            TXxref(), TXpxref(), TXcindex(), TXfindex(), TXprintindex(),
            TXnop(), TXasis(), TXname(), TXdimen(), TXepsfbox(), 
            TXasisGrouped(), TXref(), TXanimation(), TXmore(), TXsavearg();
extern void *TXcopy();
*/
            
void TXStyleLatexInfo( TeXlist, fin, fout )
SRList *TeXlist;
FILE *fin, *fout;
{            
    SCSetCommentChar( 0 );

    UsingLatexinfo = 1;
    /* printf ("Using Latexinfo Style\n" );  */
    TXInsertName( TeXlist, "dfn", TXdfn, 0, (void *)0 );
    TXInsertName( TeXlist, "var", TXdfn, 0, (void *)0 );
    TXInsertName( TeXlist, "routine", TXroutine, 1, (void *)0 );
    TXInsertName( TeXlist, "c",  TXcomment, 0, (void *)0 );
    TXInsertName( TeXlist, "node", TXcomment, 0, (void *)0 );
    TXInsertName( TeXlist, "xref",  TXxref, 1, (void *)0 );
    TXInsertName( TeXlist, "pxref",  TXpxref, 1, (void *)0 );
    TXInsertName( TeXlist, "cindex", TXcindex, 1, (void *)0 );
    TXInsertName( TeXlist, "findex", TXfindex, 1, (void *)0 );
    TXInsertName( TeXlist, "printindex", TXprintindex, 1, (void *)0 );
    TXInsertName( TeXlist, "newindex", TXnop, 1, (void *)0 );
    TXInsertName( TeXlist, "$", TXname, 0, TXCopy("$") );
    /* No comment character in TeXinfo style */
    SCSetCommentChar( (char) 0 );
    CommentChar = (char) 0;
}    

void TXStyleFunclist( TeXlist, fin, fout )
SRList *TeXlist;
FILE *fin, *fout;
{            
    TXInsertName( TeXlist, "CDfont", TXnop, 1, (void *)0 );
    TXInsertName( TeXlist, "CDw", TXnop, 1, (void *)0 );
    TXInsertName( TeXlist, "CDl", TXnop, 1, (void *)0 );
    TXInsertName( TeXlist, "CDr", TXnop, 1, (void *)0 );

    TXInsertName( TeXlist, "k", TXnop, 3, (void *)0 );

    TXInsertName( TeXlist, "CoDe", TXasisGrouped, 1, (void *)0 );
    TXInsertName( TeXlist, "DeFn", TXasisGrouped, 1, (void *)0 );

/* \def\StartFunclist{\catcode`\_=13\def_{{\tt \char`\_}}} */
}

/* This is a fairly poor approximation */
#define MAX_TITLE 1000
#define MAX_AUTHOR 1000
static char TitleString[MAX_TITLE], AuthorString[MAX_AUTHOR];
void TXANLTitle( e )
TeXEntry *e;
{
    char ReportNumber[128];
    int  InBody = InOutputBody;
    /* Note that this evaluates the args.  If there are commands, we want
       them turned into the appropriate formatting commands
     */
    InOutputBody = 1;
    TeXGetArg( fpin[curfile], TitleString, MAX_TITLE );
    TeXGetArg( fpin[curfile], AuthorString, MAX_AUTHOR );
    TeXGetArg( fpin[curfile], ReportNumber, 128 );
    TeXGetArg( fpin[curfile], curtok, MAX_TOKEN );
    InOutputBody = InBody;
    TXmaketitle( e, TitleString, AuthorString );
}

void TXStyleTpage( SRList *TeXlist, FILE *fin, FILE *fout )
{
    TXInsertName( TeXlist, "anltmfalse", TXnop, 0, (void *)0 );
    TXInsertName( TeXlist, "anltmtrue", TXnop, 0, (void *)0 );
/*     TXInsertName( TeXlist, "newif", TXnop, 0, (void *)0 ); */
    TXInsertName( TeXlist, "ifanltm", TXnop, 0, (void *)0 );
    TXInsertName( TeXlist, "ANLG", TXnop, 1, (void *)0 );
    TXInsertName( TeXlist, "ANLEDITED", TXnop, 1, (void *)0 );
    TXInsertName( TeXlist, "ANLTitle", TXANLTitle, 4, (void *)0 );
    TXInsertName( TeXlist, "ANLTMTitle", TXANLTitle, 4, (void *)0 );
    TXInsertName( TeXlist, "ANLPreprintTitle", TXANLTitle, 4, (void *)0 );
    TXInsertName( TeXlist, "tpagetopskip", TXdimen, 0, (void *) 0 );
}

void TXStyleTools( TeXlist, fin, fout )
SRList *TeXlist;
FILE *fin, *fout;
{            
    TXInsertName( TeXlist, "Iter", TXname, 0, TXCopy("KSP") );
    TXInsertName( TeXlist, "SMEIT", TXname, 0, TXCopy("SMEIT") );
    TXInsertName( TeXlist, "SMEITP", TXname, 0, TXCopy("SMEIT Package") );
    TXInsertName( TeXlist, "Solvers", TXname, 0, TXCopy("SLES") );
    TXInsertName( TeXlist, "SolversP", TXname, 0, TXCopy("SLES Package") );
    TXInsertName( TeXlist, "Tools", TXname, 0, TXCopy("PETSc") );
    TXInsertName( TeXlist, "ToolsP", TXname, 0, TXCopy("PETSc Package") );

    TXInsertName( TeXlist, "setmargin", TXnop, 1, (void *)0 );
    TXInsertName( TeXlist, "esetmargin", TXnop, 0, (void *)0 );

    TXInsertName( TeXlist, "ib", TXasisGrouped, 1, (void *)0 );
    TXInsertName( TeXlist, "iba", TXasisGrouped, 1, (void *)0 );
    TXInsertName( TeXlist, "ibc", TXasisGrouped, 1, (void *)0 );
    TXInsertName( TeXlist, "ibd", TXasisGrouped, 1, (void *)0 );
    TXInsertName( TeXlist, "ibamount", TXasisGrouped, 1, (void *)0 );
    TXInsertName( TeXlist, "ibcamount", TXasisGrouped, 1, (void *)0 );
    TXInsertName( TeXlist, "ibaamount", TXasisGrouped, 1, (void *)0 );
    TXInsertName( TeXlist, "ibdamount", TXasisGrouped, 1, (void *)0 );

/*
  \def\usemath{\catcode`\^=7 \catcode`\_=8}
  \def\closer{\setlength{\parskip}{0pt}}
  */
    TXInsertName( TeXlist, "gb", TXnop, 0, (void *)0 );

/* \def\lb{\hfil\break} */
}


void TXStyleEPSF( TeXlist, fin, fout )
SRList *TeXlist;
FILE *fin, *fout;
{            
    TXInsertName( TeXlist, "epsfxsize", TXdimen, 0, (void *)0 );
    TXInsertName( TeXlist, "epsfysize", TXdimen, 0, (void *)0 );
    TXInsertName( TeXlist, "epsfbox", TXepsfbox, 1, (void *)0 );
    TXInsertName( TeXlist, "epsffile", TXepsfbox, 1, (void *)0 );
}

void TXStyleAnlhtext( TeXlist, fin, fout )
SRList *TeXlist;
FILE *fin, *fout;
{            
    TXInsertName( TeXlist, "animation", TXanimation, 3, (void *)0 );
    TXInsertName( TeXlist, "more", TXmore, 1, (void *)0 );
}

/*
   This is for the "handout" style.
 */
static char handemail[256];
static char handphone[256];
static char handcontact[256];
void TXmakeinfo( e )
TeXEntry *e;
{
    SCPushToken( "{Further Information}" );
    e->ctx = (void *)1;
    TXsection( e );
    if (handcontact[0]) {
	TeXoutstr( fpout, handcontact );
	TXWriteStartNewLine( fpout );
    }
    if (handemail[0]) {
	TeXoutstr( fpout, handemail );
	TXWriteStartNewLine( fpout );
    }
    if (handphone[0]) {
	TeXoutstr( fpout, handphone );
	TXWriteStartNewLine( fpout );
    }
}

void TXStyleANLHandpage( TeXlist, fin, fout )
SRList *TeXlist;
FILE *fin, *fout;
{            
    handemail[0] = 0;
    handphone[0] = 0;
    handcontact[0] = 0;
    TXInsertName( TeXlist, "contact", TXsavearg, 1, (void *)handcontact );
    TXInsertName( TeXlist, "email", TXsavearg, 1, (void *)handemail );
    TXInsertName( TeXlist, "phone", TXsavearg, 1, (void *)handphone );
    TXInsertName( TeXlist, "makeinfo", TXmakeinfo, 0, (void *)0 );
}

void TXStylePsfig( TeXlist, fin, fout )
SRList *TeXlist;
FILE *fin, *fout;
{
    /* extern void TXpsfig(); */
    TXInsertName( TeXlist, "psfig", TXpsfig, 1, (void *)0 );
}
