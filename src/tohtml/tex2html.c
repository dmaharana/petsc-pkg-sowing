/* -*- Mode: C; c-basic-offset:4 ; -*- */
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

static int Debug_image_size = 0;
void TX_XBM_size( char *, int *, int * );
void TX_GIF_size( char *, int *, int * );

/* 
   I'd like to push a token in every routine, but the PC version seems to
   run out of stack space (very, very tiny stacks!) 
 */

static char buf[40];

/* FIXME: Is this a different variable than in tex.h? */
/* static int InDocument = 0; */


void TXDoGaudy( int flag )
{
    DoGaudy = flag;
}

void TXStartDoc( int flag )
{
    InDocument = flag;
    if (DebugOutput) printf( "Set InDocument to %d\n", flag );
}

/* Note that once preformatted is switched on, new lines become "real".  Thus,
   we must suppress the "extra" newlines when in preformatted mode */
void TXpreformated( FILE *fout, int flag )
{
    IsPreformatted = flag;
    if (!InDocument || !InOutputBody) return;
    if (flag) 
	TeXoutcmd( fout, "<pre>" );
    else {
	TeXoutcmd( fout, "</pre>" );
        TeXoutstr( fout, NewLineString );
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

/* We may need to add actions to be performed at the end of a group (e.g.,
   a change in character types, for obeylines, or for improved font handling).
   This structure allows us to add general actions */
typedef struct _action_t {
    void (*fcn)(void); /* Action to perform */
    struct _action_t *nextAction;
} action_t;

#define MAX_GROUP_STACK 50
static int   Esp     = 0;
static char *(EndName[MAX_GROUP_STACK]);
static action_t *(endActions[MAX_GROUP_STACK]);
static char *LastFont = 0;

void TeXoutNewline( FILE *fout )
{
    if (!InDocument || !InOutputBody) return;
    if (DebugOutput) fprintf( stdout, "TeXoutNewline\n" );
/* Note that we change a newline into space-newline; TeX takes newline as a
   space; RTF does not */
    TeXoutcmd( fout, " " );
    TeXoutstr( fout, NewLineString );
}

/* Handle \\ in TeX file */
void TXbw2( TeXEntry *e )
{
/* Usually a linebreak */
    if (!InDocument || !InOutputBody) return;
    TeXoutcmd( fpout, "<br>" );
}
/* I use \bw in LaTeXinfo to generate a \ */
void TXbw( TeXEntry *e )
{
    if (!InDocument || !InOutputBody) return;
    TeXoutcmd( fpout, "\\" );
}

void TXoutbullet( TeXEntry *e )
{
    if (!InDocument || !InOutputBody) return;
    if (DoGaudy && itemlevel >= 0) {
	char ctmp[256];
	if (NoBMCopy)
	    sprintf( ctmp, "<dt><img width=14 height=14 src=\"%s%sball.gif\" alt=\"*\">%s", 
		     IMAGEURL, itemgifs[itemlevel >= 5 ? 4 : itemlevel],
		     NewLineString);
	else
	    sprintf( ctmp, "<dt><img width=14 height=14 src=\"%sball.gif\" alt=\"*\">%s", 
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
void TXbf( TeXEntry *e )
{
  /* output start of BoldFace */
    if (!InDocument || !InOutputBody) return;
    if (LastFont && strcmp(LastFont,"b") == 0) {
	/* Same font, so no need to change */
	if (DebugFont) {
	    fprintf( stdout, "Already in bold, nothing to do\n" );
	}
	return;
    }
    if (LastFont) {
	if (DebugFont) {
	    fprintf( stdout, "Ended font %s (LastFont):", LastFont );
	    TXPrintLocation( stdout );
	}
	sprintf( buf, "</%s>", LastFont );
	TeXoutcmd( fpout, buf );
    }
    if (DebugFont) {
	fprintf( stdout, "TXbf: Starting font %s:", "b" );
	TXPrintLocation( stdout );
    }

    if (Esp >= 0) {
	TeXoutcmd( fpout, "<b>" );
	EndName[Esp] = "b";
	LastFont     = "b";
    }
    else {
	fprintf( ferr, "Group stack index = %d, <0:", Esp );
	TXPrintLocation( ferr );
    }

}

void TXem( TeXEntry *e )
{
    if (!InDocument || !InOutputBody) return;
/* output start of Emphasis (italics) */
    if (LastFont && strcmp(LastFont,"em") == 0) {
	/* Same font, so no need to change */
	if (DebugFont) {
	    fprintf( stdout, "Already in em, nothing to do\n" );
	}
	return;
    }
    if (LastFont) {
	if (DebugFont) {
	    fprintf( stdout, "Ended font %s (LastFont):", LastFont );
	    TXPrintLocation( stdout );
	}
	sprintf( buf, "</%s>", LastFont );
	TeXoutcmd( fpout, buf );
    }
    if (DebugFont) {
	fprintf( stdout, "Starting font %s:", "em" );
	TXPrintLocation( stdout );
    }
    if (Esp >= 0) {
	TeXoutcmd( fpout, "<em>" );
	EndName[Esp] = "em";
	LastFont     = "em";
    }
    else {
	fprintf( ferr, "Group stack index = %d, <0:", Esp );
	TXPrintLocation( ferr );
    }

}

void TXsf( TeXEntry *e )
{
    if (!InDocument || !InOutputBody) return;
    /* output start of Sans-serif (use rm for html) */
#if 0
    if (LastFont && strcmp(LastFont,"font") == 0) {
	/* Same font, so no need to change */
	if (DebugFont) {
	    fprintf( stdout, "Already in sans serif, nothing to do\n" );
	}
	return;
    }
#endif
    if (LastFont) {
	if (DebugFont) {
	    fprintf( stdout, "Ended font %s (LastFont):", LastFont );
	    TXPrintLocation( stdout );
	}
	sprintf( buf, "</%s>", LastFont );
	TeXoutcmd( fpout, buf );
    }
    if (Esp >= 0) {
	/* Should replace this with span style="font-something:..." */
	TeXoutcmd( fpout, "<font face=\"sans-serif\">" );
	EndName[Esp] = "font";
	LastFont     = "font";
    }
    else {
	fprintf( ferr, "Group stack index = %d, <0:", Esp );
	TXPrintLocation( ferr );
    }

}

/* 
   HTML makes this particularly hard because there is NO roman font 
   command (in the "classis" HTML).  
   You have to move into and out of the current font

   What we'll do for now is issue an error message if you aren't in the
   correct font
 */
void TXrm( TeXEntry *e )
{
    if (!InDocument || !InOutputBody) return;
    if (Esp >= 0) {
	if (EndName[Esp] && strcmp(EndName[Esp], "rm") != 0) {
	    if (!ferr) ferr=stderr;
	    fprintf( ferr, 
		     "Switching to Roman font within another font not supported\n" );
	    fprintf( ferr, "Current font is %s\n", EndName[Esp] );
	    TXPrintLocation( ferr );
	}
    }
/*
  EndName[Esp] = "rm";
  LastFont     = "rm";
  */
    if (Esp >= 0) {
	EndName[Esp] = (char *)0;
	LastFont     = (char *)0;
    }
    else {
	fprintf( ferr, "Group stack index = %d, <0:", Esp );
	TXPrintLocation( ferr );
    }

}

void TXbgroup( TeXEntry *e )
{
    if (!InDocument || !InOutputBody) return;
    Esp++;
    if (Esp >= MAX_GROUP_STACK) {
	fprintf( ferr, "Exceeded group stack; depth is %d, ", Esp );
	TXPrintLocation( ferr );
    }
    if (DebugFont) { 
	fprintf( stdout, "TXbgroup: Esp=%d:", Esp );
	TXPrintLocation( stdout );
    }
    if (Esp >= 0) {
	EndName[Esp]    = 0;
	endActions[Esp] = 0;
    }
    else {
	fprintf( ferr, "Mismatched groups - Esp < 0, = %d, ", Esp );
	TXPrintLocation( ferr );
    }
}

void TXegroup( TeXEntry *e )
{
    if (!InDocument || !InOutputBody) return;
    if (DebugFont) {
	fprintf( stdout, "TXegroup:" );
	if (Esp >= 0) {
	    fprintf( stdout, "(EspName[%d]= %s):", Esp, EndName[Esp] ? EndName[Esp] : "<null>" );
	}
	else {
	    fprintf( stdout, "(Esp = %d)", Esp );
	}
	TXPrintLocation( stdout );
    }

    if (Esp >= 0) {
	if (EndName[Esp]) {
	    if (DebugFont) {
		fprintf( stdout, "Ended font %s:", EndName[Esp] );
		TXPrintLocation( stdout );
	    }
	    sprintf( buf, "</%s>", EndName[Esp] );
	    TeXoutcmd( fpout, buf );
	    EndName[Esp] = 0;
	}
	if (endActions[Esp]) {
	    action_t *actions = endActions[Esp], *oldaction;
	    while (actions) {
		(actions->fcn)();
		oldaction = actions;
		actions   = actions->nextAction;
		FREE(oldaction);
	    }
	    endActions[Esp] = 0;
	}
    }
    Esp--;
    LastFont = 0;
    if (Esp < -1) { 
	fprintf( ferr, "Setting scan debug because group count < -1\n" );
	SCSetDebug( 1 );
    }
}
/* Use this to push a routine to be invoked when the group ends */
void TXgroupPushAction( void (*fcn)(void) )
{
    action_t *newaction, *tail;

    newaction = (action_t *)MALLOC(sizeof(action_t));
    newaction->fcn = fcn;
    newaction->nextAction = 0;
    tail = endActions[Esp];
    if (!tail) {
	endActions[Esp] = newaction;
    }
    else {
	while (tail->nextAction) {
	    tail = tail->nextAction;
	}
	tail->nextAction = newaction;
    }
}

void TXfont_tt( TeXEntry *e )
{
    /* output start of TT (Courier) in default face*/
    if (!InDocument || !InOutputBody) return;
    if (LastFont) {
	if (DebugFont) {
	    fprintf( stdout, "Ended font %s (LastFont):", LastFont );
	    TXPrintLocation( stdout );
	}

	sprintf( buf, "</%s>", LastFont );
	TeXoutcmd( fpout, buf );
    }
    if (DebugFont) {
	fprintf( stdout, "TXfont_tt: Starting font %s:", "tt" );
	TXPrintLocation( stdout );
    }

    if (Esp >= 0) {
	TeXoutcmd( fpout, "<tt>" );
	EndName[Esp] = "tt";
	LastFont     = "tt";
    }
    else {
	fprintf( ferr, "Group stack index = %d, <0:", Esp );
	TXPrintLocation( ferr );
    }
}

void TXfont_ss( TeXEntry *e )
{
    /* output start of Sans Serif in default face*/
    if (!InDocument || !InOutputBody) return;
    if (LastFont) {
	if (DebugFont) {
	    fprintf( stdout, "Ended font %s (LastFont):", LastFont );
	    TXPrintLocation( stdout );
	}

	sprintf( buf, "</%s>", LastFont );
	TeXoutcmd( fpout, buf );
    }
    if (DebugFont) {
	fprintf( stdout, "TXfont_ss: Starting font %s:", "em" );
	TXPrintLocation( stdout );
    }
    
    if (Esp >= 0) {
	TeXoutcmd( fpout, "<em>" );
	EndName[Esp] = "em";
	LastFont     = "em";
    }
    else {
	fprintf( ferr, "Group stack index = %d, <0:", Esp );
	TXPrintLocation( ferr );
    }

}

void TXtt( TeXEntry *e )
{
/* output start of TT (Courier) in default face*/
    if (!InDocument || !InOutputBody) return;
    if (LastFont) {
	if (DebugFont) {
	    fprintf( stdout, "Ended font %s (LastFont):", LastFont );
	    TXPrintLocation( stdout );
	}

	sprintf( buf, "</%s>", LastFont );
	TeXoutcmd( fpout, buf );
    }
    if (DebugFont) {
	fprintf( stdout, "TXtt: Starting font %s:", "tt" );
	TXPrintLocation( stdout );
    }

    if (Esp >= 0) {
	TeXoutcmd( fpout, "<tt>" );
	EndName[Esp] = "tt";
	LastFont     = "tt";
    }
    else {
	fprintf( ferr, "Group stack index = %d, <0:", Esp );
	TXPrintLocation( ferr );
    }

}

#ifdef FOO
void TXbitmap( TeXEntry *e, char *fname )
{
    if (!InDocument || !InOutputBody) return;
/* Process a bitmap reference */
    TeXoutcmd( fpout, "<img src=\""  );
    TeXoutstr( fpout, fname );
        TeXoutstr( fpout, "\" alt=\"Bitmap file\">" );
}
#endif

void TXWritePar( FILE *fp )
{
    if (!InDocument || !InOutputBody) return;
    if (DebugOutput) fprintf( stdout, "TXWritePar\n" );
    TeXoutcmd( fp, "<P>" );
    TeXoutstr( fp, NewLineString );
}

void TXimage( TeXEntry *e, char *fname )
{
    int height, width;
    if (!InDocument || !InOutputBody) return;
    if (DebugOutput) fprintf( stdout, "TXimage\n" );

    /* Try to get the width and height */
    width  = -1;
    height = -1;
    /* Note that most browsers no longer support xbm files at all,
       so generate a warning for xbm files */
    if (strstr( fname, ".xbm" )) {
	fprintf( stderr, "Warning: XBM file %s found; browsers no longer support these files\n", fname );
	TX_XBM_size( fname, &width, &height );
    }
    else if (strstr( fname, ".gif" )) {
	TX_GIF_size( fname, &width, &height );
    }

    /* Recent HTML requires an ALT field on IMG */
    /* FIXME: Provide a way to specify a string for the alt field */
    if (height == -1 || width == -1) {
	fprintf( fpout, "<P><img src=\"%s\" alt=\"Image file\"><P>%s", fname, NewLineString );
    }
    else {
	fprintf( fpout, "<P><img width=%d height=%d src=\"%s\" alt=\"Image file\"><P>%s", 
		 width, height, fname, NewLineString );
    }
/* This version makes the images external... */
/* fprintf( fpout, "<A HREF=\"%s\">push here for picture</a>"\n", fname ); */
}

void TXInlineImage( TeXEntry *e, char *fname )
{
    int width, height;

    if (!InDocument || !InOutputBody) return;

    /* Try to get the width and height */
    width  = -1;
    height = -1;
    /* Note that most browsers no longer support xbm files at all,
       so generate a warning for xbm files */
    if (strstr( fname, ".xbm" )) {
	fprintf( stderr, "Warning: XBM file %s found; browsers no longer support these files\n", fname);
	TX_XBM_size( fname, &width, &height );
    }
    else if (strstr( fname, ".gif" )) {
	TX_GIF_size( fname, &width, &height );
    }

    if (height == -1 || width == -1) {
	fprintf( fpout, "<img src=\"%s\" alt=\"Image file\">%s", fname, NewLineString );
    }
    else {
	fprintf( fpout, "<img width=%d height=%d src=\"%s\" alt=\"Image file\">%s", 
		 width, height, fname, NewLineString );
    }
/* This version makes the images external... */
/* fprintf( fpout, "<A HREF=\"%s\">push here for picture</a>"\n", fname ); */
}

void TXAnchoredImage( TeXEntry *e, char *anchorname, char *fname )
{
    int width, height;

    if (!InDocument || !InOutputBody) return;
    if (DebugOutput) fprintf( stdout, "TXAnchoredImage\n" );

    /* Try to get the width and height */
    width  = -1;
    height = -1;
    /* Note that most browsers no longer support xbm files at all,
       so generate a warning for xbm files */
    if (strstr( fname, ".xbm" )) {
	fprintf( stderr, "Warning: XBM file %s found; browsers no longer support these files\n", fname);
	TX_XBM_size( fname, &width, &height );
    }
    else if (strstr( fname, ".gif" )) {
	TX_GIF_size( fname, &width, &height );
    }

    if (height == -1 || width == -1) {
	fprintf( fpout, "<P><span id=\"%s\"><img src=\"%s\" alt=\"Image file\"></span><P>%s", 
		 anchorname, fname, NewLineString );
    }
    else {
	fprintf( fpout, 
	     "<P><span id=\"%s\"><img width=%d height=%d src=\"%s\" alt=\"Image file\"></span><P>%s", 
		 anchorname, width, height, fname, NewLineString );
    }
}

void TXAnchoredInlineImage( TeXEntry *e, char *anchorname, char *fname )
{
    int width, height;

    if (!InDocument || !InOutputBody) return;
    if (DebugOutput) fprintf( stdout, "TXAnchoredInlineImage\n" );

    /* Try to get the width and height */
    width  = -1;
    height = -1;
    /* Note that most browsers no longer support xbm files at all,
       so generate a warning for xbm files */
    if (strstr( fname, ".xbm" )) {
	fprintf( stderr, "Warning: XBM file %s found; browsers no longer support these files\n", fname);
	TX_XBM_size( fname, &width, &height );
    }
    else if (strstr( fname, ".gif" )) {
	TX_GIF_size( fname, &width, &height );
    }

    fprintf( fpout, "<img src=\"%s\" alt=\"Image file\">%s", fname, NewLineString );
/* This version makes the images external... */
    fprintf( fpout, "<span id=\"%s\"><img src=\"%s\" alt=\"Image file\"></span><P>%s", 
	     anchorname, fname, NewLineString );
}

void TXmovie( TeXEntry *e, char *movie, char *icon, char *text )
{
    if (!InDocument || !InOutputBody) return;
    fprintf( fpout, "<a href=\"%s\"><img align=top src=\"%s\" alt=\"Movie file\"></a>%s%s", 
	     movie, icon, text, NewLineString );
}

void TXbbrace( TeXEntry *e )
{
    if (!InDocument || !InOutputBody) return;	
    fputs( "{", fpout );
}

void TXebrace( TeXEntry *e )
{
    if (!InDocument || !InOutputBody) return;	
    fputs( "}", fpout );
}

void TXmath( TeXEntry *e )
{
    if (!InDocument || !InOutputBody) return;	
    TeXoutcmd( fpout, "<p><i>" );
}
void TXmathend( TeXEntry *e )
{
    if (!InDocument || !InOutputBody) return;	
    TeXoutcmd( fpout, "</i><p>" );
}
void TXinlinemath( TeXEntry *e )
{
    if (!InDocument || !InOutputBody) return;	
    TeXoutcmd( fpout, "<i>" );
}
void TXinlinemathend( TeXEntry *e )
{
    if (!InDocument || !InOutputBody) return;	
    TeXoutcmd( fpout, "</i>" );
}

void TXWriteStartNewLine( FILE *fout )
{
    if (!InDocument || !InOutputBody) return;
    if (!IsPreformatted) {
	TeXoutcmd( fout, "<br>" );
	TeXoutstr( fout, NewLineString );
    }
    else
	TeXoutcmd( fout, NewLineString );
}

void TXWriteStartItem( FILE *fout )
{
    if (!InDocument || !InOutputBody) return;
    TeXoutcmd( fout, NewLineString );
}

void TXmaketitle( TeXEntry *e, char *TitleString, char *AuthorString )
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
    TeXoutcmd( fpout, "<h1>" );
    TeXoutstr( fpout, tmpbuf );
    TeXoutcmd( fpout, "</h1>" );
    TeXoutstr( fpout, NewLineString );

    TeXoutcmd( fpout, "<p><b>" );
    TeXoutstr( fpout, AuthorString );
    TeXoutcmd( fpout, "</b></p>" );
    TXecenter( fpout );
    TeXoutstr( fpout, NewLineString );
}

/* Itemize environement */
void TXbitemize( TeXEntry *e )
{
    if (DoGaudy) {
	itemlevel++;
	TeXoutcmd( fpout, "<blockquote><dl>" );
    }
    else
	TeXoutcmd( fpout, "<ul>" );
    TeXoutstr( fpout, NewLineString );
}
void TXeitemize( TeXEntry *e )
{
    if (DoGaudy) {
	itemlevel--;
	TeXoutcmd( fpout, "</dl></blockquote>" );
    }
    else
	TeXoutcmd( fpout, "</ul>" );	
    TeXoutstr( fpout, NewLineString );
}	

/* Enumerate environment */
void TXbenumerate( TeXEntry *e )
{
    TeXoutcmd( fpout, "<ol>" );	
    TeXoutstr( fpout, NewLineString );
}
void TXeenumerate( TeXEntry *e )
{
    TeXoutcmd( fpout, "</ol>" );	
    TeXoutstr( fpout, NewLineString );
}
void TXbdescription( TeXEntry *e )
{
    TeXoutcmd( fpout, "<dl>" );
    TeXoutstr( fpout, NewLineString );
}
void TXedescription( TeXEntry *e )
{
    TeXoutcmd( fpout, "</dl>" );
    TeXoutstr( fpout, NewLineString );
}
/* Menus */
/* HTML is deprecating and then redefining menus.  For the simple passive
   list of elements, use an un-numbered list (ul) */
void TXbmenu( FILE *fout )
{
    TeXoutcmd( fout, "<ul>" );
    TeXoutstr( fout, NewLineString );
}
void TXemenu( FILE *fout )
{
    TeXoutcmd( fout, "</ul>" );
    TeXoutstr( fout, NewLineString );
}

/* Description Item */	
void TXbdesItem( TeXEntry *e )
{
    TeXoutcmd( fpout, "<dt>" );
    TeXoutstr( fpout, NewLineString );
}

void TXedesItem( TeXEntry *e )
{
    TeXoutcmd( fpout, "<dd>" );
    TeXoutstr( fpout, NewLineString );
}	

void TXWriteHyperLink( FILE *fout, char *token, char *url, int urltype )
{
    if (!InDocument || !InOutputBody) return;
    fprintf( fout, "<a href=\"%s\">%s</a>", url, token );	
}	

/* The <center> element has been deprecated.  Use this instead. */
void TXbcenter( FILE *fpout )
{
    TeXoutcmd( fpout, "<div style=\"text-align:center\">" );
}
void TXecenter( FILE *fpout )
{
    TeXoutcmd( fpout, "</div>" );
}

void TX_XBM_size( char *fname, int *width, int *height )
{
    FILE *f;
    char lbuf[257];
    char fullname[1024];
    /* To get the size, read the first line for the width and
       the second line for the height (#define name_width value,
       #define name_height value)
    */
    if (splitlevel >= 0) {
	strcpy( fullname, splitdir );
	strcat( fullname, "/" );
	strcat( fullname, fname );
	fname = fullname;
    }
    if (Debug_image_size) printf( "Opening %s\n", fname );
    if (! (f = fopen( fname, "r" )) ) return;
    fscanf( f, "#define %s %d\n", lbuf, width );
    /* We could check that lbuf ends in width */
    fscanf( f, "#define %s %d\n", lbuf, height );
    if (Debug_image_size) 
	printf( "Width = %d height = %d\n", *width, *height );
    fclose( f );
}

void TX_GIF_size( char *fname, int *width, int *height )
{
    FILE *f;
    int  w;
    char c;
    char fullname[1024];

    /* To get the size, do:
       giftopnm filename | head -2 | sed 1d 
       then the remaining text is '^width height$', where ^
       is the beginning of the line and $ is the end of the line
    */
    if (splitlevel >= 0) {
	strcpy( fullname, splitdir );
	strcat( fullname, "/" );
	strcat( fullname, fname );
	fname = fullname;
    }
    if(! (f = fopen( fname, "r" )) ) return;

    /* GIF8[79]a width height
       width and height are written with 
       int w;
       putc( w & 0xff ); putc( (w/256) & 0xff );
    */
    /* printf( "Looking at %s\n", fname ); */
    if (fscanf( f, "GIF8%ca", &c ) < 1 ) {
	/* We don't recognize the file.  Set the sizes to unknown */
	*width = -1;
	*height = -1;
	fclose( f );
	return;
    }
    
    /* printf( "Getting width\n" ); */
    w = fgetc( f );
    w += (fgetc( f ) * 256);
    *width = w;
    /* printf( "Width = %d, getting height\n", *width ); */
    w = fgetc( f );
    w += (fgetc( f ) * 256);
    *height = w;
    /* printf( "Height = %d\n" ); */
    fclose( f );
}
