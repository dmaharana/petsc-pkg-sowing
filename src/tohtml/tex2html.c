#include <stdio.h>
#include "sowing.h"
#include "search.h"
#include "tex.h"

/*
   This is a simple program to translate mostly ASCII text code from 
   a latexinfo file into another format.  Different formats can be
   generated by providing different versions of the file tex2....c
   For example, 
       tex2win generates Windows help output.
       tex2html generates HTML (WWW) output
   
 */

int DoGaudy = 0;       /* If true, generate more colorful output */
static char *(itemgifs[5]) = { "purple", "red", "blue", "green", "yellow" } ;
static int  itemlevel = -1;
static int IsPreformatted = 0;

/* 
   I'd like to push a token in every routine, but the PC version seems to
   run out of stack space (very, very tiny stacks!) 
 */

static FILE *ferr = 0;
static char buf[40];

static int InDocument = 0;


void TXDoGaudy( flag )
int flag;
{
    DoGaudy = flag;
}

void TXStartDoc( flag )
int flag;
{
    InDocument = flag;
}

/* Note that once preformatted is switched on, new lines become "real".  Thus,
   we must suppress the "extra" newlines when in preformatted mode */
void TXpreformated( fout, flag )
FILE *fout;
int  flag;
{
    IsPreformatted = flag;
    if (!InDocument || !InOutputBody) return;
    if (flag) 
	TeXoutcmd( fout, "<pre>" );
    else {
	TeXoutcmd( fout, "</pre>" );
        TeXoutstr( fpout, NewLineString );
    }
}

/*
    TeX processing:
    We need to manage things like \section{name}, \begin{tex}, \c (comment)

    Multiple blank lines must generate a \par in the rtf file.
    More specifically, a completely blank line must be changed into a par.
 */
    
/*
    This file contains the actions for processing a subset of LaTeXinfo
    files
 */

static int   Esp     = 0;
static char *(EndName[50]);
static char *LastFont = 0;

void TeXoutNewline( fout )
FILE *fout;
{
    if (!InDocument || !InOutputBody) return;
    if (DebugOutput) fprintf( stdout, "TeXoutNewline\n" );
/* Note that we change a newline into space-newline; TeX takes newline as a
   space; RTF does not */
    TeXoutcmd( fout, " " );
    TeXoutstr( fpout, NewLineString );

}

/* Handle \\ in TeX file */
void TXbw2( e )
TeXEntry *e;
{
/* Usually a linebreak */
    if (!InDocument || !InOutputBody) return;
    TeXoutcmd( fpout, "<BR>" );
}
/* I use \bw in LaTeXinfo to generate a \ */
void TXbw( e )
TeXEntry *e;
{
    if (!InDocument || !InOutputBody) return;
    TeXoutcmd( fpout, "\\" );
}

void TXoutbullet( e )
TeXEntry *e;
{
    if (!InDocument || !InOutputBody) return;
    if (DoGaudy && itemlevel >= 0) {
	char ctmp[256];
	if (NoBMCopy)
	    sprintf( ctmp, "<DT><IMG SRC=\"%s%sball.gif\" ALT=\"*\">%s", 
		     BMURL, itemgifs[itemlevel >= 5 ? 4 : itemlevel],
		     NewLineString);
	else
	    sprintf( ctmp, "<DT><IMG SRC=\"%sball.gif\" ALT=\"*\">%s", 
		     itemgifs[itemlevel >= 5 ? 4 : itemlevel], 
		     NewLineString);
	TeXoutcmd( fpout, ctmp );
    }
    else
	TeXoutcmd( fpout, "<li>" );
}

/* 
   Font changes may not be in groups.  For example, \tt ... \em ... \bf.
   In this case, we must end the previous font by undoing the last font.
 */
void TXbf( e )
TeXEntry *e;
{
/* output start of BoldFace */
    if (!InDocument || !InOutputBody) return;
    if (LastFont) {
	sprintf( buf, "</%s>", LastFont );
	TeXoutcmd( fpout, buf );
    }
    TeXoutcmd( fpout, "<b>" );
    EndName[Esp] = "b";
    LastFont     = "b";
}

void TXem( e )
TeXEntry *e;
{
    if (!InDocument || !InOutputBody) return;
/* output start of Emphasis (italics) */
    if (LastFont) {
	sprintf( buf, "</%s>", LastFont );
	TeXoutcmd( fpout, buf );
    }
    TeXoutcmd( fpout, "<em>" );
    EndName[Esp] = "em";
    LastFont     = "em";
}

void TXsf( e )
TeXEntry *e;
{
    if (!InDocument || !InOutputBody) return;
/* output start of Sans-serif (use rm for html) */
    if (LastFont) {
	sprintf( buf, "</%s>", LastFont );
	TeXoutcmd( fpout, buf );
    }
    EndName[Esp] = (char *)0;
    LastFont     = (char *)0;
}

/* HTML makes this particularly hard because there is NO roman font 
   command.  You have to move into and out of the current font

   What we'll do for now is issue an error message if you aren't in the
   correct font
 */
void TXrm( e )
TeXEntry *e;
{
    if (!InDocument || !InOutputBody) return;
    if (Esp >= 0) {
	if (EndName[Esp] && strcmp(EndName[Esp], "rm") != 0) {
	    if (!ferr) ferr=stderr;
	    fprintf( ferr, 
		     "Switching to Roman font within another font not supported\n" );
	}
    }
/*
  EndName[Esp] = "rm";
  LastFont     = "rm";
  */
    EndName[Esp] = (char *)0;
    LastFont     = (char *)0;
}

void TXbgroup( e )
TeXEntry *e;
{
    if (!InDocument || !InOutputBody) return;
    Esp++;
}

void TXegroup( e )
TeXEntry *e;
{
    if (!InDocument || !InOutputBody) return;
    if (Esp >= 0 && EndName[Esp]) {
	sprintf( buf, "</%s>", EndName[Esp] );
	TeXoutcmd( fpout, buf );
	EndName[Esp] = 0;
    }
    Esp--;
    LastFont = 0;
}

void TXfont_tt( e )
TeXEntry *e;
{
/* output start of TT (Courier) in default face*/
    if (!InDocument || !InOutputBody) return;
    if (LastFont) {
	sprintf( buf, "</%s>", LastFont );
	TeXoutcmd( fpout, buf );
    }
    TeXoutcmd( fpout, "<tt>" );
    EndName[Esp] = "tt";
    LastFont     = "tt";
}

void TXfont_ss( e )
TeXEntry *e;
{
/* output start of Sans Serif in default face*/
    if (!InDocument || !InOutputBody) return;
    if (LastFont) {
	sprintf( buf, "</%s>", LastFont );
	TeXoutcmd( fpout, buf );
    }
    TeXoutcmd( fpout, "<em>" );
    EndName[Esp] = "em";
    LastFont     = "em";
}

void TXtt( e )
TeXEntry *e;
{
/* output start of TT (Courier) in default face*/
    if (!InDocument || !InOutputBody) return;
    if (LastFont) {
	sprintf( buf, "</%s>", LastFont );
	TeXoutcmd( fpout, buf );
    }
    TeXoutcmd( fpout, "<tt>" );
    EndName[Esp] = "tt";
    LastFont     = "tt";
}

#ifdef FOO
void TXbitmap( e, fname )
TeXEntry *e;
char     *fname;
{
    if (!InDocument || !InOutputBody) return;
/* Process a bitmap reference */	
    TeXoutcmd( fpout, "<IMG SRC=\"" );
    TeXoutstr( fpout, fname );
    TeXoutstr( fpout, "\">" );
}
#endif

void TXWritePar( fp )
FILE *fp;
{
    if (!InDocument || !InOutputBody) return;
    if (DebugOutput) fprintf( stdout, "TXWritePar\n" );
    TeXoutcmd( fp, "<P>" );
    TeXoutstr( fp, NewLineString );
}

void TXimage( e, fname )
TeXEntry *e;
char     *fname;
{
    if (!InDocument || !InOutputBody) return;
    if (DebugOutput) fprintf( stdout, "TXimage\n" );
    fprintf( fpout, "<P><IMG SRC=\"%s\"><P>%s", fname, NewLineString );
/* This version makes the images external... */
/* fprintf( fpout, "<A HREF=\"%s\">push here for picture</a>"\n", fname ); */
}

void TXInlineImage( e, fname )
TeXEntry *e;
char     *fname;
{
    if (!InDocument || !InOutputBody) return;
    fprintf( fpout, "<IMG SRC=\"%s\">%s", fname, NewLineString );
/* This version makes the images external... */
/* fprintf( fpout, "<A HREF=\"%s\">push here for picture</a>"\n", fname ); */
}

void TXAnchoredImage( e, anchorname, fname )
TeXEntry *e;
char     *anchorname, *fname;
{
    if (!InDocument || !InOutputBody) return;
    if (DebugOutput) fprintf( stdout, "TXAnchoredImage\n" );
    fprintf( fpout, "<P><A NAME=\"%s\"><IMG SRC=\"%s\"></a><P>%s", 
	     anchorname, fname, NewLineString );
}

void TXAnchoredInlineImage( e, anchorname, fname )
TeXEntry *e;
char     *anchorname, *fname;
{
    if (!InDocument || !InOutputBody) return;
    if (DebugOutput) fprintf( stdout, "TXAnchoredInlineImage\n" );
    fprintf( fpout, "<IMG SRC=\"%s\">%s", fname, NewLineString );
/* This version makes the images external... */
    fprintf( fpout, "<A NAME=\"%s\"><IMG SRC=\"%s\"></a><P>%s", 
	     anchorname, fname, NewLineString );
}

void TXmovie( e, movie, icon, text )
TeXEntry *e;
char     *movie, *icon, *text;
{
    if (!InDocument || !InOutputBody) return;
    fprintf( fpout, "<A HREF=\"%s\"><IMG align=top src=\"%s\"></A>%s%s", 
	     movie, icon, text, NewLineString );
}

void TXbbrace( e )
TeXEntry *e;
{
    if (!InDocument || !InOutputBody) return;	
    fputs( "{", fpout );
}

void TXebrace( e )
TeXEntry *e;
{
    if (!InDocument || !InOutputBody) return;	
    fputs( "}", fpout );
}

void TXmath( e )
TeXEntry *e;
{
    if (!InDocument || !InOutputBody) return;	
    TeXoutcmd( fpout, "<p><I>" );
}
void TXmathend( e )
TeXEntry *e;
{
    if (!InDocument || !InOutputBody) return;	
    TeXoutcmd( fpout, "</I><p>" );
}
void TXinlinemath( e )
TeXEntry *e;
{
    if (!InDocument || !InOutputBody) return;	
    TeXoutcmd( fpout, "<I>" );
}
void TXinlinemathend( e )
TeXEntry *e;
{
    if (!InDocument || !InOutputBody) return;	
    TeXoutcmd( fpout, "</I>" );
}

void TXWriteStartNewLine( fout )
FILE *fout;
{
    if (!InDocument || !InOutputBody) return;
    if (!IsPreformatted) {
	TeXoutcmd( fout, "<BR>" );
	TeXoutstr( fout, NewLineString );
    }
    else
	TeXoutcmd( fout, NewLineString );
}

void TXWriteStartItem( fout )
FILE *fout;
{
    if (!InDocument || !InOutputBody) return;
    TeXoutcmd( fout, NewLineString );
}

void TXmaketitle( e, TitleString, AuthorString )
TeXEntry *e;
char *TitleString, *AuthorString;
{
    char tmpbuf[1024];

    if (DebugOutput) fprintf( stdout, "TXmaketitle\n" );
    RemoveFonts( TitleString, tmpbuf );
    WriteHeadPage( fpout );
    WriteFileTitle( fpout, tmpbuf );
    WriteBeginPage( fpout );
/* TeXoutcmd( fpout, "<TITLE>" );
   TeXoutstr( fpout, tmpbuf );
   TeXoutcmd( fpout, "</TITLE>\n" );
   */

    /* Add centering commands to title body */
    TXbcenter( fpout );
    TeXoutcmd( fpout, "<H1>" );
    TeXoutstr( fpout, tmpbuf );
    TeXoutcmd( fpout, "</H1>" );
    TeXoutstr( fpout, NewLineString );

    TeXoutcmd( fpout, "<b>" );
    TeXoutstr( fpout, AuthorString );
    TeXoutcmd( fpout, "</b><P>" );
    TXecenter( fpout );
    TeXoutstr( fpout, NewLineString );
}

/* Itemize environement */
void TXbitemize( e )
TeXEntry *e;
{
    if (DoGaudy) {
	itemlevel++;
	TeXoutcmd( fpout, "<BLOCKQUOTE><DL>" );
    }
    else 
	TeXoutcmd( fpout, "<ul>" );	
    TeXoutstr( fpout, NewLineString );
}	
void TXeitemize( e )
TeXEntry *e;
{
    if (DoGaudy) {
	itemlevel--;
	TeXoutcmd( fpout, "</DL></BLOCKQUOTE>" );
    }
    else
	TeXoutcmd( fpout, "</ul>" );	
    TeXoutstr( fpout, NewLineString );
}	

/* Enumerate environment */
void TXbenumerate( e )
TeXEntry *e;
{
    TeXoutcmd( fpout, "<ol>" );	
    TeXoutstr( fpout, NewLineString );
}
void TXeenumerate( e )
TeXEntry *e;
{
    TeXoutcmd( fpout, "</ol>" );	
    TeXoutstr( fpout, NewLineString );
}
void TXbdescription( e )
TeXEntry *e;
{
    TeXoutcmd( fpout, "<dl>" );
    TeXoutstr( fpout, NewLineString );
}
void TXedescription( e )
TeXEntry *e;
{
    TeXoutcmd( fpout, "</dl>" );
    TeXoutstr( fpout, NewLineString );
}
/* Menus */	
void TXbmenu( fout )
FILE *fout;
{
    TeXoutcmd( fout, "<menu>" );
    TeXoutstr( fpout, NewLineString );
}
void TXemenu( fout )
FILE *fout;
{
    TeXoutcmd( fout, "</menu>" );
    TeXoutstr( fpout, NewLineString );
}

/* Description Item */	
void TXbdesItem( e )
TeXEntry *e;
{
    TeXoutcmd( fpout, "<dt>" );
    TeXoutstr( fpout, NewLineString );
}

void TXedesItem( e )
TeXEntry *e;
{
    TeXoutcmd( fpout, "<dd>" );
    TeXoutstr( fpout, NewLineString );
}	

void TXWriteHyperLink( fout, token, url, urltype )
FILE *fout;
char *token, *url;
int  urltype;
{
    if (!InDocument || !InOutputBody) return;
    fprintf( fout, "<a href=\"%s\">%s</a>", url, token );	
}	

void TXbcenter( FILE *fpout )
{
    TeXoutcmd( fpout, "<center>" );
}
void TXecenter( FILE *fpout )
{
    TeXoutcmd( fpout, "</center>" );
}
