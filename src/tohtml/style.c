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

void TXStyleLatexInfo( SRList *myTeXlist, FILE *fin, FILE *fout )
{
    SCSetCommentChar( 0 );

    UsingLatexinfo = 1;
    /* printf ("Using Latexinfo Style\n" );  */
    TXInsertName( myTeXlist, "dfn", TXdfn, 0, (void *)0 );
    TXInsertName( myTeXlist, "var", TXdfn, 0, (void *)0 );
    TXInsertName( myTeXlist, "routine", TXroutine, 1, (void *)0 );
    TXInsertName( myTeXlist, "c",  TXcomment, 0, (void *)0 );
    TXInsertName( myTeXlist, "node", TXcomment, 0, (void *)0 );
    TXInsertName( myTeXlist, "xref",  TXxref, 1, (void *)0 );
    TXInsertName( myTeXlist, "pxref",  TXpxref, 1, (void *)0 );
    TXInsertName( myTeXlist, "cindex", TXcindex, 1, (void *)0 );
    TXInsertName( myTeXlist, "findex", TXfindex, 1, (void *)0 );
    TXInsertName( myTeXlist, "printindex", TXprintindex, 1, (void *)0 );
    TXInsertName( myTeXlist, "newindex", TXnop, 1, (void *)0 );
    TXInsertName( myTeXlist, "$", TXname, 0, TXCopy("$") );
    /* No comment character in TeXinfo style */
    SCSetCommentChar( (char) 0 );
    CommentChar = (char) 0;
}

void TXStyleFunclist( SRList *myTeXlist, FILE *fin, FILE *fout )
{
    TXInsertName( myTeXlist, "CDfont", TXnop, 1, (void *)0 );
    TXInsertName( myTeXlist, "CDw", TXnop, 1, (void *)0 );
    TXInsertName( myTeXlist, "CDl", TXnop, 1, (void *)0 );
    TXInsertName( myTeXlist, "CDr", TXnop, 1, (void *)0 );

    TXInsertName( myTeXlist, "k", TXnop, 3, (void *)0 );

    TXInsertName( myTeXlist, "CoDe", TXasisGrouped, 1, (void *)0 );
    TXInsertName( myTeXlist, "DeFn", TXasisGrouped, 1, (void *)0 );

/* \def\StartFunclist{\catcode`\_=13\def_{{\tt \char`\_}}} */
}

/* This is a fairly poor approximation */
#define MAX_TITLE 1000
#define MAX_AUTHOR 1000
static char TitleString[MAX_TITLE], AuthorString[MAX_AUTHOR];
void TXANLTitle( TeXEntry *e )
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

void TXStyleTpage( SRList *myTeXlist, FILE *fin, FILE *fout )
{
    TXInsertName( myTeXlist, "anltmfalse", TXnop, 0, (void *)0 );
    TXInsertName( myTeXlist, "anltmtrue", TXnop, 0, (void *)0 );
/*     TXInsertName( myTeXlist, "newif", TXnop, 0, (void *)0 ); */
    TXInsertName( myTeXlist, "ifanltm", TXnop, 0, (void *)0 );
    TXInsertName( myTeXlist, "ANLG", TXnop, 1, (void *)0 );
    TXInsertName( myTeXlist, "ANLEDITED", TXnop, 1, (void *)0 );
    TXInsertName( myTeXlist, "ANLTitle", TXANLTitle, 4, (void *)0 );
    TXInsertName( myTeXlist, "ANLTMTitle", TXANLTitle, 4, (void *)0 );
    TXInsertName( myTeXlist, "ANLPreprintTitle", TXANLTitle, 4, (void *)0 );
    TXInsertName( myTeXlist, "tpagetopskip", TXdimen, 0, (void *) 0 );
}

void TXStyleTools( SRList *myTeXlist, FILE *fin, FILE *fout )
{
    TXInsertName( myTeXlist, "Iter", TXname, 0, TXCopy("KSP") );
    TXInsertName( myTeXlist, "SMEIT", TXname, 0, TXCopy("SMEIT") );
    TXInsertName( myTeXlist, "SMEITP", TXname, 0, TXCopy("SMEIT Package") );
    TXInsertName( myTeXlist, "Solvers", TXname, 0, TXCopy("SLES") );
    TXInsertName( myTeXlist, "SolversP", TXname, 0, TXCopy("SLES Package") );
    TXInsertName( myTeXlist, "Tools", TXname, 0, TXCopy("PETSc") );
    TXInsertName( myTeXlist, "ToolsP", TXname, 0, TXCopy("PETSc Package") );

    TXInsertName( myTeXlist, "setmargin", TXnop, 1, (void *)0 );
    TXInsertName( myTeXlist, "esetmargin", TXnop, 0, (void *)0 );

    TXInsertName( myTeXlist, "ib", TXasisGrouped, 1, (void *)0 );
    TXInsertName( myTeXlist, "iba", TXasisGrouped, 1, (void *)0 );
    TXInsertName( myTeXlist, "ibc", TXasisGrouped, 1, (void *)0 );
    TXInsertName( myTeXlist, "ibd", TXasisGrouped, 1, (void *)0 );
    TXInsertName( myTeXlist, "ibamount", TXasisGrouped, 1, (void *)0 );
    TXInsertName( myTeXlist, "ibcamount", TXasisGrouped, 1, (void *)0 );
    TXInsertName( myTeXlist, "ibaamount", TXasisGrouped, 1, (void *)0 );
    TXInsertName( myTeXlist, "ibdamount", TXasisGrouped, 1, (void *)0 );

/*
  \def\usemath{\catcode`\^=7 \catcode`\_=8}
  \def\closer{\setlength{\parskip}{0pt}}
  */
    TXInsertName( myTeXlist, "gb", TXnop, 0, (void *)0 );

/* \def\lb{\hfil\break} */
}


void TXStyleEPSF( SRList *myTeXlist, FILE *fin, FILE *fout )
{
    TXInsertName( myTeXlist, "epsfxsize", TXdimen, 0, (void *)0 );
    TXInsertName( myTeXlist, "epsfysize", TXdimen, 0, (void *)0 );
    TXInsertName( myTeXlist, "epsfbox", TXepsfbox, 1, (void *)0 );
    TXInsertName( myTeXlist, "epsffile", TXepsfbox, 1, (void *)0 );
}

void TXStyleAnlhtext( SRList *myTeXlist, FILE *fin, FILE *fout )
{
    TXInsertName( myTeXlist, "animation", TXanimation, 3, (void *)0 );
    TXInsertName( myTeXlist, "more", TXmore, 1, (void *)0 );
}

/*
   This is for the "handout" style.
 */
static char handemail[256];
static char handphone[256];
static char handcontact[256];
void TXmakeinfo( TeXEntry *e )
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

void TXStyleANLHandpage( SRList *myTeXlist, FILE *fin, FILE *fout )
{
    handemail[0] = 0;
    handphone[0] = 0;
    handcontact[0] = 0;
    TXInsertName( myTeXlist, "contact", TXsavearg, 1, (void *)handcontact );
    TXInsertName( myTeXlist, "email", TXsavearg, 1, (void *)handemail );
    TXInsertName( myTeXlist, "phone", TXsavearg, 1, (void *)handphone );
    TXInsertName( myTeXlist, "makeinfo", TXmakeinfo, 0, (void *)0 );
}

void TXStylePsfig( SRList *myTeXlist, FILE *fin, FILE *fout )
{
    /* extern void TXpsfig(); */
    TXInsertName( myTeXlist, "psfig", TXpsfig, 1, (void *)0 );
}
