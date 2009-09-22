/* -*- Mode: C; c-basic-offset:4 ; -*- */

#include <stdio.h>
#include <stdlib.h>
#include "sowing.h"
#include "patchlevel.h"

#ifdef HAVE_UNISTD_H
/* For unlink */
#include <unistd.h>
#endif

#include "tex.h"
void ProcessFile ( int, char **, FILE *, FILE *, 
			     void (*)( int, char **, FILE *, FILE * ) );
char *SkipHTML ( char * );
void CopyImgFiles ( char * );
void PrintHelp ( char * );

/* void ProcessInfoFile ( int, char **, FILE *, FILE * ); */

void RemoveExtension ( char * );

/* void ProcessLatexFile ( int, char **, FILE *, FILE * );*/

void SaveCommandLine( int, char *[] );
char *GetCommandLine( void );

/* We make these globals in consideration of the DOS version of this code */
char infilename[256], outfilename[256], projfilename[256], auxfilename[256];
char latexerrfilename[256];

/* Linux defines a basename FUNCTION in string.h ! */
char basefilename[256];
char basedir[256];
char basedefs[256];
char userpath[1024];  /* Path to search for package definitions */

char ImageExt[4];     /* Choose the image type.  Used as both the
			 image name and the file extension.  */

char endpagefilename[256];
char beginpagefilename[256];
/* NavNames are the buttons at the bottom, TopNames are the buttons at the 
   top.  BottomNav is all of the stuff at the bottom.  */
int DoNavNames	= 1;
int DoTopNames	= 1;
int DoContents	= 1;
int DoBottomNav	= 1;
int IsGaudy     = 0;

/* Use level_offset to shift the header styles (e.g., use H2 instead of H1) */
int level_offset = 0;

/* If NoBMCopy is chosen, then the predefined bitmaps are not copied, and
   the path in BMURL is used instead */
int NoBMCopy = 0;

/* Change this to turn off output translations */
int  DoOutputTranslation = 1;

/* Change this to turn off active token handling */
int  DoActiveTokens = 1;

/* If true, add the <return> needed at the end of lines */
#if defined(WIN32) || defined(__MSDOS__)
int  DoDosFileNewlines = 1;
char NewLineString[4] = "\r\n";
#else
int  DoDosFileNewlines = 0;
char NewLineString[4] = "\n";
#endif

/* These are used to keep track of the state of HTML output for the current
   output file */
static int wrotehead=0;
static int wrotebody=0;


/*
    This program converts files to HTML

    I'm using a C program instead of a perl script because I don't have a
    version of perl for my PC and the translation that I intend to do is
    fairly simple.  Eventually, something like perl is probably a better
    choice.
 */
int main( int argc, char *argv[] )
{
    FILE *fpin, *fpout;
    void (*process)( int, char **, FILE *, FILE * ) = 
	ProcessLatexFile;
    int  splitlevel = -1;
    char tmpstr[128];
    int DebugDef = 0;

    if (argc < 2 || SYArgHasName( &argc, argv, 1, "-help" )) {
	PrintHelp( argv[0] );
	return 1;
    }

    /* Check for any debugging options in the memory tracing code */
    TrInit();

    /* Initializations prior to checking the arguments */
    /* outfilename[0] = 0; */

    if (SYArgHasName( &argc, argv, 1, "-version" )) {
	printf( "tohtml from version %d.%d.%d %s of %s\n", 
		PATCHLEVEL, PATCHLEVEL_MINOR, PATCHLEVEL_SUBMINOR, 
		PATCHLEVEL_RELEASE_KIND, PATCHLEVEL_RELEASE_DATE );
	return 1;
    }

    /* Save the command line */
    SaveCommandLine( argc, argv );

    TXSetCitePrefix( "[" );
    TXSetCiteSuffix( "]" );

    if (SYArgHasName( &argc, argv, 1, "-info" )) {
	fprintf( stderr, "Latexinfo files not handled\n" );
	return 1;
/*	process = ProcessInfoFile; */
    }
    if (SYArgHasName( &argc, argv, 1, "-latex" )) {
	process = ProcessLatexFile;
    }
    if (SYArgHasName( &argc, argv, 1, "-nocopy" ))
	NoBMCopy = 1;
    if (SYArgHasName( &argc, argv, 1, "-numbers" ))
	IncludeSectionNumbers = 1;

    if (SYArgHasName( &argc, argv, 1, "-dosfiles" )) {
	DoDosFileNewlines = 1;
	strcpy( HTML_Suffix, "htm" );
	DirSep            = '\\';
	strcpy( DirSepString, "\\" );
    }

    if (SYArgHasName( &argc, argv, 1, "-dosnl" )) {
	DoDosFileNewlines = 1;
	strcpy ( NewLineString, "\r\n" );
	strcpy( HTML_Suffix, "htm" );
    }

    /*
      if (SYArgHasName( &argc, argv, 1, "-order")) {
      process = CreateOrderFile;
      }
      else {
      will want to read order file here
      }    
    */    

    /* FIXME: Add support for debug environment variables */

    if (SYArgHasName( &argc, argv, 1, "-default" )) {
	TXSetLatexUnknown( 1 );
	TXSetLatexTables( 1 );
	TXSetLatexMath( 1 );
	TXSetUseIfTex( 1 );
	splitlevel = 2;
	TXSetLatexAgain( 0 );
    }
    if (SYArgHasName( &argc, argv, 1, "-quietlatex")) {
	TXSetLatexQuiet( 1 );
    }

    if (SYArgHasName( &argc, argv, 1, "-debug")) {
	TXSetDebug( 1 );
    }
    if (SYArgHasName( &argc, argv, 1, "-debugfile" )) {
	TXSetDebugFile( 1 );
    }
    if (SYArgHasName( &argc, argv, 1, "-debugfont" )) {
	TXSetDebugFont( 1 );
    }
    if (SYArgHasName( &argc, argv, 1, "-debugscan" )) {
	SCSetDebug( 1 );
    }
    if (SYArgHasName( &argc, argv, 1, "-debugdef" )) {
	DebugDef = 1;
	TXDebugDef( 1 );
    }
    if (SYArgHasName( &argc, argv, 1, "-debugout" )) {
	DebugOutput = 1;
    }

    if (SYArgHasName( &argc, argv, 1, "-Wnoredef" )) {
	warnRedefinition = 0;
    }

    if (SYArgHasName( &argc, argv, 1, "-cvtlatex" ))
	TXSetLatexUnknown( 1 );
    if (SYArgHasName( &argc, argv, 1, "-cvttables" ))
	TXSetLatexTables( 1 );
    if (SYArgHasName( &argc, argv, 1, "-htables" )) {
	HandleAlign = 1;
	TXSetLatexTables( 0 );
    }
    if (SYArgHasName( &argc, argv, 1, "-cvtmath" ))
	TXSetLatexMath( 1 );

    if (SYArgHasName( &argc, argv, 1, "-simplemath" ))
	TXSetSimpleMath( 1 );

    if (SYArgHasName( &argc, argv, 1, "-gaudy" )) {
	TXDoGaudy( 1 );
	IsGaudy = 1;
    }

    if (SYArgHasName( &argc, argv, 1, "-nonavnames" )) 
	DoNavNames = 0;
    if (SYArgHasName( &argc, argv, 1, "-notopnames" )) 
	DoTopNames = 0;
    if (SYArgHasName( &argc, argv, 1, "-nobottomnav" ))
	DoBottomNav = 0;

    if (SYArgHasName( &argc, argv, 1, "-nocontents" )) {
	TeXNoContents();
	DoContents = 0;
    }
    if (SYArgGetString( &argc, argv, 1, "-citeprefix", tmpstr, 128 ))
	TXSetCitePrefix( tmpstr );
    if (SYArgGetString( &argc, argv, 1, "-citesuffix", tmpstr, 128 ))
	TXSetCiteSuffix( tmpstr );

    if (SYArgHasName( &argc, argv, 1, "-useimg" ))
	TXSetLatexAgain( 0 );

    if (SYArgGetString( &argc, argv, 1, "-indexname", tmpstr, 128 )) {
	if (UserIndexName) {
	    fprintf( stderr, "Only one index name allowed\n" );
	}
	UserIndexName = (char *)MALLOC( strlen(tmpstr) + 1 );
	strcpy( UserIndexName, tmpstr );
    }

    if (SYArgHasName( &argc, argv, 1, "-debugmalloc" ))
	trDebugLevel( 1 );
    if (SYArgHasName( &argc, argv, 1, "-malloctrace" ))
	trlevel( 3 );

    if (SYArgHasName( &argc, argv, 1, "-iftex" )) {
	/* Use iftex instead of ifinfo branches */
	TXSetUseIfTex( 1 );
    }
    /* This indicates at what level the output should be split into multiple 
       files
   if -split 0 is used, chapters are in separate files.
   -split 1        , chapters AND sections are in separate files.

   Note that in this case, ALL files are written into the split directory 
   */    
    SYArgGetInt( &argc, argv, 1, "-split", &splitlevel );

    SYArgGetInt( &argc, argv, 1, "-headeroffset", &level_offset );

/* Get the hypertext mappings */    
    while (SYArgGetString( &argc, argv, 1, "-mapref", infilename, 256 )) {
	RdRefMap( infilename );
    }

/* Read the simple TeX and LaTeX commands, along any user-defined names */
    RdBaseDef( BASEDEF );
    while (SYArgGetString( &argc, argv, 1, "-basedef", infilename, 256 )) {
	RdBaseDef( infilename );
    }

    while (SYArgGetString( &argc, argv, 1, "-mapman", infilename, 256 )) {
	RdRefMap( infilename );
	TXSetProcessManPageTokens( 1 );
    }

    basedir[0] = 0;    
    SYArgGetString( &argc, argv, 1, "-basedir", basedir, 256 );

    userpath[0] = 0;    
    SYArgGetString( &argc, argv, 1, "-userpath", userpath, 256 );

    /* Set the default image format*/
    strcpy( ImageExt, "xbm" );
    if (SYArgHasName( &argc, argv, 1, "-allgif" ))
        strcpy( ImageExt, "gif" );

    endpagefilename[0] = 0;
    SYArgGetString( &argc, argv, 1, "-endpage", endpagefilename, 256 );

    beginpagefilename[0] = 0;
    SYArgGetString( &argc, argv, 1, "-beginpage", beginpagefilename, 256 );

    outfilename[0] = 0;
    SYArgGetString( &argc, argv, 1, "-o", outfilename, sizeof(outfilename) );

    argv++;
    argc--;
    if (argc == 0 || !argv[0]) {
	fprintf( stderr, "Missing filename!\n" );
	return 1;
    }
    strcpy( infilename, argv[0] );
    if (outfilename[0] == 0) {
	strcpy( outfilename, argv[0] );
    }
    RemoveExtension( outfilename );
    strcpy( basefilename, outfilename );
    strcpy( auxfilename, outfilename );
    strcat( outfilename, "." );
    strcat( outfilename, HTML_Suffix );
    strcat( auxfilename, ".hux" );

    strcpy( latexerrfilename, outfilename );
    strcat( latexerrfilename, ".ler" );
    unlink( latexerrfilename );

    TXSetFiles( infilename, outfilename );

    if (splitlevel >= 0)  {
	strcpy( outfilename, basefilename );
	strcat( outfilename, DirSepString );
	strcat( outfilename, basefilename );
	strcat( outfilename, "." );
	strcat( outfilename, HTML_Suffix );
	TXSetSplitLevel( splitlevel, basefilename );
	strcat( basefilename, DirSepString );
	SYMakeAllDirs( basefilename, 0666 );
	if (!NoBMCopy)
	    CopyImgFiles( basefilename );
	/* remove the "/" we added to basefilename */
	basefilename[strlen(basefilename)-1] = 0;
    }    
    else {
	if (!NoBMCopy)
	    CopyImgFiles( "." );
    }

    fpin  = fopen( infilename, "r" );
    fpout = fopen( outfilename, "w" );
    if (!fpin || !fpout) {
	fprintf( stderr, "Could not open file %s and/or %s\n",
		 infilename, outfilename );
	exit(1);
    }

    OpenAuxFile( auxfilename );    

/* Generate the base dir line.  If multiple files are generated, this
   needs to be done for each file.  The name should look something
   like "http://www.mcs.anl.gov/foo/index.html" 
   */
    if (basedir[0]) 
	fprintf( fpout, "<base href=\"%s\">%s", basedir, NewLineString );
    
/* Process the input file */
    ProcessFile( argc, argv, fpin, fpout, process );

/* Close the files and write the project file */
    fclose( fpin );
    WriteEndPage( fpout );
    fclose( fpout );

    if (DebugDef) {
	fprintf( stdout, "User definitions in TeX format are:\n" );
	TXDumpUserDefs( stdout, 1 );
    }
    return 0;
}


/*
    Write the help header to fp
 */
void WriteHeader( FILE *fp )
{
}

/* Write the trailer */
void WriteTrailer( FILE *fp )
{
}

/*
    Write the header for a section

    name = name (to be printed)
    entrylevel = chapter,section, ...
    number     = number of section
    keywords (optional) = keywords for section
 */
void WriteSectionHeader( FILE *fp, char *name, char *entrylevel, int number, 
			 char *keywords, int level )
{
    if (DebugOutput) fprintf( stdout, "WriteSectionHeader\n" );
/*
  fprintf( fp, "<A NAME=\"...\"><IMG SRC=\"/icons/previous.xbm\"></A>" );  
  fprintf( fp, "<A NAME=\"...\"><IMG SRC=\"/icons/up.xbm\"></A>" );  
  fprintf( fp, "<A NAME=\"...\"><IMG SRC=\"/icons/next.xbm\"></A>" );  
  fprintf( fp, "<b>Previous:</b><A NAME=\"...\">...</a> " );
  fprintf( fp, "<b>Up:</b><A NAME=\"...\">...</a> " );
  fprintf( fp, "<b>Next:</b><A NAME=\"...\">...</a> " );
  fprintf( fp, "<P>%s", NewLineString );
  */
/* fprintf( fp, "<H%d><A NAME=\"%s%d\">%s</a></H%d>",
   level+1, entrylevel, number, name, level+1 ); */
/* fprintf( fp, "<H%d>%s</H%d>",
   level+1, name, level+1 ); */
}

/* 
   We can't put an anchor in without there being actual TEXT.  GRUMBLE.
 */
void WriteSectionAnchor( FILE *fp, char *name, char *entrylevel, 
			 int number, int level )
{
    char tmpname[256];
/* FEATURE: font changes in headings don't work.  Rather than try and suppress 
   them in the translation phase, I'll remove them here... */
    RemoveFonts( name, tmpname );
/* Level must be >= 1 */
    if (DebugOutput) fprintf( stdout, "WriteSectionAnchor\n" );
    if (level < 1) level = 1;
    fprintf( fp, "<HR><H%d><A NAME=\"%s%d\">%s</a></H%d>%s", 
	     level + level_offset, entrylevel, number, tmpname,
	     level + level_offset, NewLineString );
}

/* Write out the name of the section; this will be the title for the file */
void WriteFileTitle( FILE *fp, char *name )
{
    /* Too late if we've written the body statement */
    if (DebugOutput) fprintf( stdout, "WriteFileTitle\n" );
    if (wrotebody) return;
    /* Make sure that we remove TOK_START and TOK_END from name */
    fprintf( fp, "<TITLE>" );
    WriteString( fp, name );
    fprintf( fp, "</TITLE>%s", NewLineString );
}

void WriteJumpDestination( FILE *fp, char *name, char *title )
{
    if (DebugOutput) fprintf( stdout, "WriteJumpDestination\n" );
    fprintf( fp, "<A NAME=\"%s\">%s</a>", name, title );
}

/*
    Write the text to set the formatting for the body of a topic
 */
void WriteTextHeader( FILE *fp )
{
}

/*
    Write out popup text.  popup text refers to a topic (entryname/number)
 */
void WritePopupTextReference( FILE *fp, char *text, char *reftopic, 
			      int refnumber )
{
    if (DebugOutput) fprintf( stdout, "WritePopupTextReference\n" );
    fprintf( fp, "<a href=\"%s%d\">%s</a>", reftopic, refnumber, text );
}

/*
    Write out pointers to other topics
    Since the topic may contain quoted commands, we need to process it
    as well
 */
void WritePointerText( FILE *fp, char *text, char *reftopic, int refnumber )
{
    if (DebugOutput) fprintf( stdout, "WritePointerText\n" );
    if (refnumber >= 0) 
	fprintf( fp, "<a href=\"%s%d\">", reftopic, refnumber );
    else
	fprintf( fp, "<a href=\"%s\">", reftopic );
    WriteString( fp, text );
    fprintf( fp, "</a>" );
}    
    
/*
    Write out the end of a topic
 */
void WriteEndofTopic( FILE *fp )
{
    if (!InDocument) return;
    if (DebugOutput) fprintf( stdout, "WriteEndofTopic\n" );
    fprintf( fp, "%s<P>%s", NewLineString, NewLineString );
    /* If there is text for the bottom, add a rule ... */
    if (DoNavNames && DoBottomNav) 
	fprintf( fp, "<HR>%s", NewLineString );
}	

/* This translation needs to happen on output in HTML, if it hasn't 
   already been processed */
int SCHTMLTranslate( char *token, int maxtoken )
{
/*
  if (token[0] == '<')      strcpy( token, "&lt;" );
  else if (token[0] == '>') strcpy( token, "&gt;" );
  else if (token[0] == '&') strcpy( token, "&amp;" );
  else 
  */ 
/* Even this is wrong; it breaks on \~ or ~ in a code/verbatim environment */
    if (!UsingLatexinfo && token[0] == '~') strcpy( token, " " );

    return token[0];
}

int SCHTMLTranslateTables( char *token, int maxtoken )
{
    if (token[0] == '<')      strcpy( token, "&lt;" );
    else if (token[0] == '>') strcpy( token, "&gt;" );
    else if (!UsingLatexinfo && token[0] == '~') strcpy( token, " " );

    return token[0];
}

/*
   We really want different versions of this depending on whether we are
   reading a raw info file or a latex file or ...

   In any event, the file represents a tree that has been flattened out.
   When a section is encountered, the actions are

   Get the section name, keywords.
       WriteSectionHeader
       WriteTextHeader
   write the text for the section until the next section is encountered.
   Then (recursively)
       remember the current position
       read forward, looking for additional sections
       
           Sections at the same level have
               WritePointerText
           Sections at a higher level terminate the search
           Sections at a lower level are skipped
       Finally, WriteEndofTopic

     In an info file, we don't need to do this because the file has already
     been processed.  See ProcessInfoFile().       
 */
void ProcessFile( int argc, char **argv, FILE *fpin, FILE *fpout, 
		  void (*process)( int, char **, FILE *, FILE *) )
{
    SCSetTranslate( SCHTMLTranslate );
    WriteHeader( fpout );
    (*process)( argc, argv, fpin, fpout );
    WriteTrailer( fpout );
}

void RemoveExtension( char *str )
{
    char *p;

    p = str + strlen( str ) - 1;
    while (p > str && *p != '.') p--;
    *p = '\0';
}


/* This isn't documented either in the "comprehensive" list of commands OR IN
   THE FORMAL GRAMMAR (but it IS in the description of grammar elements) */
void WriteStartNewLine( FILE *fp )
{
    fprintf( fp, "<BR>%s", NewLineString );
}	

void DebugWriteString( FILE *fd, const char *str, int maxlen )
{
    const char *p = str;
    
    while (*p && maxlen-- >= 0) {
	if (*p == TOK_START) {
	    fputs( "<tok_start>", fd );
	}
	else if (*p == TOK_END) {
	    fputs( "<tok_end>", fd );
	}
	else {
	    fputc( *p, fd );
	}
	p++;
    }
}
void WriteString( FILE *fp, char *str )
{
    int  in_tok = 0;
    char thischar;

    if (DebugOutput) { 
	fprintf( stdout, "WriteString (" );
	DebugWriteString( stdout, str, 40 );
	fprintf( stdout, ") to %d\n", fileno(fp) );
    }

    if (fileno(fp) < 0) {
	abort();
    }
    while (*str) {
	thischar = *str;
	/* Skip the token start/end and don't process while within */
	if (thischar == TOK_START) {
	    if (DebugOutput) fprintf( stdout, "token-start\n" );
	    in_tok++;
	}
	else if (thischar == TOK_END) {
	    if (DebugOutput) fprintf( stdout, "token-end\n" );
	    in_tok--;
	}
	else if (in_tok == 0 && thischar == '>' && DoOutputTranslation) {
	    fputs( "&gt;", fp );
	}
	else if (in_tok == 0 && thischar == '<' && DoOutputTranslation) {
	    fputs( "&lt;", fp );
	}
	else if (in_tok == 0 && thischar == '&' && DoOutputTranslation) {
	    fputs( "&amp;", fp );
	}
	else if (thischar == '\r') {
	    fputc( ' ', fp );
	    fputc( thischar, fp );
	    if (str[1] == '\n') {
		str++;
		fputc( *str, fp );
	    }
		
	}
	else if (thischar == '\n') {
	    fputc( ' ', fp );
	    /* DOS style is \r\n */
	    if (DoDosFileNewlines)
		fputc( '\r', fp );
	    fputc( thischar, fp );
	}
	else {
	    fputc( thischar, fp );
	}
	str++;
    }
#ifdef FOO
    if (*str) {
	if (str[strlen(str)-1] == '\n') {
	    char *p;
	    p = str;
	    while (*p) {
		if (*p == '\n') fputc( ' ', fp );
		fputc( *p, fp );
		p++;
	    }
        }
	else 
	    fputs( str, fp );
    }
#endif
}

/* This handles escaping of special characters.  These are {}\ .
   An additional "feature" is that no spaces are silently inserted by
   rtf.  This means that <char>\n needs to be change to <char> \n or
   to <char>\n<space>  I'll use the former since we may not want
   the space after the newline.
   */
void WriteStringRaw( FILE *fp, char *str )
{
    char *p;

    if (DebugOutput) fprintf( stdout, "WriteStringRaw\n" );
    p = str;
    while (*p) {
	if (*p == '<') {
	    *p = 0;
	    if (p > str) fputs( str, fp );
	    fputs( "&lt;", fp );
	    str = p + 1;
        }
	else if (*p == '>') {
	    *p = 0;
	    if (p > str) fputs( str, fp );
	    fputs( "&gt;", fp );
	    str = p + 1;
        }
	else if (*p == '&') {
	    *p = 0;
	    if (p > str) fputs( str, fp );
	    fputs( "&amp;", fp );
	    str = p + 1;
        }
	p++;        
    }
/* Flush the remaining text */
    if (p > str) {
	fputs( str, fp );
    }
}

#include "search.h"
static char ParentTitle[128];
static char NextTitle[128];
static char PrevTitle[128];

/*
 * Sometime we want to write only one set of buttons per page, rather than
 * have the buttons move down the page.  However, to do this, the
 * prev/next/top links must point to PAGES, not to sections.
 * 
 * We'd also like to eliminate markers for empty sections.
 */
void WriteSectionButtons( FILE *fout, char *name, LINK *l )
{
    char contextParent[125], contextNext[125], contextPrev[125];
    int  did_output = 0;
    int  has_parent, has_next, has_prev;

    if (!l) return;

    if (DebugOutput) fprintf( stdout, "WriteSectionButtons\n" );
    /* We really need to pass the length of the third arg to the routines... */
    has_parent = GetParent( l, name, contextParent, ParentTitle );
    has_next   = GetNext( l, name, contextNext, NextTitle );
    has_prev   = GetPrevious( l, name, contextPrev, PrevTitle );

/* This is a horizontal rule for output */
/* fputs( "<HR>\n", fout ); */
    if (has_prev && DoTopNames) {
	SetPreviousButton( fout, contextPrev, PrevTitle );
	did_output++;
    }
/* The contents page should ALWAYS be the parent */
    if (has_parent && DoTopNames) {
	SetUpButton( fout, contextParent, ParentTitle );
	did_output++;
    }
    else {
	if (DoContents) {
	    /* extern char *ContentsLoc(); */
	    char *cnts;
	
	    cnts = ContentsLoc();
	    if (cnts) {
		strcpy( ParentTitle, "Contents" );
		strcpy( contextParent, cnts );
		has_parent = 1;
		if (DoTopNames) {
		    SetUpButton( fout, contextParent, ParentTitle );
		    did_output++;
		}
	    }
	}
    }
    if (has_next && DoTopNames) {
	SetNextButton( fout, contextNext, NextTitle );
	did_output++;
    }
    if (did_output) fprintf( fout, "<BR>%s", NewLineString );
    if (DoNavNames) {
	if (has_parent) {
	    OutJump( fout, contextParent, ParentTitle, "Up" );
	    did_output = 1;
	}
	if (has_next) {
	    OutJump( fout, contextNext, NextTitle, "Next" );
	    did_output = 1;
	}
	if (has_prev) {
	    OutJump( fout, contextPrev, PrevTitle, "Previous" );
	    did_output = 1;
	}
    }
    if (did_output)
	fprintf( fout, "<P>%s", NewLineString );
}

void WriteSectionButtonsBottom( FILE *fout, char *name, LINK *l )
{
    if (DoBottomNav)
	WriteSectionButtons( fout, name, l );
}


void OutJump( FILE *fp, char *context, char *name, char *label )
{
    if (DebugOutput) fprintf( stdout, "OutJump\n" );
    fprintf( fp, "<b>%s: </b><A HREF=\"%s\">", label, context );
    WriteString( fp, name );
    fprintf( fp, "</a>%s", NewLineString );
}
/*
   This changes the binding of the UP button to the given context
 */
void SetUpButton( FILE *fp, char *context, char *name )
{
    if (DebugOutput) fprintf( stdout, "SetUpButton\n" );
    fprintf( fp, "<A HREF=\"%s\"><IMG WIDTH=16 HEIGHT=16 SRC=\"%sup.%s\"></A>", 
	     context, NoBMCopy ? BMURL : "", ImageExt );  
/* fprintf( fp, "<b>Up: </b><A HREF=\"%s\">%s</a> ", context, name ); */
}   

void SetNextButton( FILE *fp, char *context, char *name )
{
    if (DebugOutput) fprintf( stdout, "SetNextButton\n" );
    fprintf( fp, "<A HREF=\"%s\"><IMG WIDTH=16 HEIGHT=16 SRC=\"%snext.%s\"></A>", 
	     context, NoBMCopy ? BMURL : "", ImageExt );  
/* fprintf( fp, "<b>Next: </b><A HREF=\"%s\">%s</a> ", context, name ); */
}   

void SetPreviousButton( FILE *fp, char *context, char *name )
{
    if (DebugOutput) fprintf( stdout, "SetPreviousButton\n" );
    fprintf( fp, "<A HREF=\"%s\"><IMG WIDTH=16 HEIGHT=16 SRC=\"%sprevious.%s\"></A>", 
	     context, NoBMCopy ? BMURL : "", ImageExt );  
/* fprintf( fp, "<b>Previous: </b><A HREF=\"%s\">%s</a> ", context, name ); */
}   

/* THIS SHOULD NOT BE USED (see "InDocument check in tex2html" ) */
void WritePar( FILE *fp )
{
    fprintf( stderr, "***BOGUS***\n" );
    fprintf( fp, "<P>%s", NewLineString );
}

void WriteBeginPointerMenu( FILE *fout )
{
    if (DebugOutput) fprintf( stdout, "WriteBeginPointerMenu\n" );
    fputs( "<li>", fout );
}
void WriteEndOfPointer( FILE *fout )
{
    if (DebugOutput) fprintf( stdout, "WriteEndOfPointer\n" );
    fputs( NewLineString, fout );
}

/* These next provide for paragraph justification */
void WriteCenter( FILE *fp )
{
}
void WriteLeftAligned( FILE *fp )
{
}
void WriteRightAligned( FILE *fp )
{
}
void WriteJustified( FILE *fp )
{
}
void WritePlain( FILE *fp )
{
}

/* Paragraph indentation */
void WriteRightIndent( FILE *fp, int n )
{
}
void WriteLeftIndent( FILE *fp, int n )
{
}
void WriteFirstLineIndent( FILE *fp, int n )
{
}

/* Return the name of the file (without the extension) */
void GetBaseName( char *str )
{
    strcpy( str, basefilename );
}
void GetMainInputFileName( char *str )
{
    strcpy( str, infilename );
}
/*
    Still to add...

    Support for bitmaps
    Support for ICONs (so that the icon represents the software)
    Support for DLL (for wav files etc)
    Support for more formatting options (tables)
 */



/*D
  tohtml - Create HTML from LaTeX input

Synopsis:

  tohtml [-latex] [-info] [-mapref filename] [-basedir dirname]
         [-userpath path]
         [-iftex] [-split n] [-headeroffset n ]
         [-mapman filename ] [-cvtlatex] [-cvttables]
         [-cvtmath] [-simplemath] [-useimg] [-gaudy] [-iftex] [-default]
         [-nonavnames] [-notopnames] [-nobottomnav]
         [-basedef filename] [-endpage filename ] [-beginpage filename]
         [-citeprefix str] [-citesuffix str] [-o outname ] filename

Input Parameters:
+ -iftex - Include text in begin{iftex}...end{iftex} mode
. -cvtlatex - Convert LaTeX that can not be represented as text into
              bitmaps
. -cvttables - Convert tabular environments into bitmaps
. -useimg - When used with -cvtlatex, use existing bitmaps from a
            previous run
. -gaudy - Use color images for itemize bullets
. -nocopy - Do not copy the pre-defined bitmaps
. -dosnl  - Generate Windows/DOS-style newlines (newline return)
. -split n - Split the document down to sections at level n
  -split -1 prohibits splitting; 0 = chapter,
  1 = section, etc.
. -headeroffset n - Offset the HTML headers generated by section.  For example,
   to use H2 instead of H1 for top level sections, use -headeroffset 1
. -nonavnames - Do not output the 'navigation' buttons
. -notopnames - Do not output the names on the top of the page
. -nobottomnav - Do not output the names or buttons at the bottom of a
  section
. -nocontents - Do not generate the 'contents' page
. -mapref file - Convert references in \\cite into hyperlinks by using the
 specified file
. -mapman file - Convert occurances of tokens into hyperlinks to files
. -default -Pick relatively nice defaults
. -citeprefix str - Set the prefix for citations ([ by default)
. -citesuffix str - Set the suffix for citations (] by default)
. -basedef name - Use file name to define TeX commands
. -beginpage filename - Prepend the html in filename to the end of
 each page
. -endpage filename - Append the html in filename to the end of
 each page
- -o outname - Specify the name to be used as the output file name (instead
    of the basing the output file name on the input file name).  This
    name should include the natural extension; e.g., for HTML output, 
    use something like tohtml -latex -o myout.htm myfile.tex .
D*/
void PrintHelp( char *pgm )
{
    fprintf( stderr,
	     "%s [-latex] [-info] [-mapref filename] [-basedir dirname]\n\
[-iftex] [-split n] [-mapman filename ] [-cvtlatex] [-cvttables]\n\
[-cvtmath] [-simplemath] [-useimg] [-gaudy] [-iftex] [-default]\n\
[-nonavnames] [-notopnames] [-nobottomnav]\n\
[-basedef filename] [-endpage filename ] [-beginpage filename]\n\
[-citeprefix str] [-citesuffix str] filename\n", pgm );
    fprintf( stderr, "\n\
\t-iftex\t\tInclude text in begin{iftex}...end{iftex} mode\n\
\t-cvtlatex\tConvert LaTeX that can not be represented as text into\n\
\t\t\tbitmaps\n\
\t-cvttables\tConvert tabular environments into bitmaps\n\
\t-useimg\t\tWhen used with -cvtlatex, use existing bitmaps from a\n\
\t\t\tprevious run\n\
\t-gaudy\t\tUse color images for itemize bullets\n\
\t-nocopy\t\tDo not copy the pre-defined bitmaps\n\
\t-split n\tSplit the document down to sections at level n.\n\
\t\t\t-split -1 prohibits splitting; 0 = chapter,\n\
\t\t\t1 = section, etc.\n\
\t-nonavnames\tDo not output the 'navigation' buttons\n\
\t-notopnames\tDo not output the names on the top of the page\n\
\t-nobottomnav\tDo not output the names or buttons at the bottom of a \n\
\t\t\tsection\n\
\t-nocontents\tDo not generate the 'contents' page\n\
\t-mapref file\tConvert references in \\cite into hyperlinks by using the\n\
\t\t\tspecified file\n\
\t-mapman file\tConvert occurances of tokens into hyperlinks to files\n\
\t-default\tPick relatively nice defaults\n\
\t-citeprefix str\tSet the prefix for citations ([ by default)\n\
\t-citesuffix str\tSet the suffix for citations (] by default)\n\
\t-basedef name\tUse file name to define TeX commands\n\
\t-beginpage filename\tPrepend the html in filename to the end of\n\
\t\t\t\teach page\n\
\t-endpage filename\tAppend the html in filename to the end of\n\
\t\t\t\teach page\n\
\t-o outname\tSpecify the name to be used as the output file name (instead\n\
\t\t\t\tof the basing the output file name on the input file name).  This\n\
\t\t\t\tname should include the natural extension; e.g., for HTML output, \n\
\t\t\t\tuse something like tohtml -latex -o myout.htm myfile.tex ." );
    fprintf( stderr, "(for wizards): [-debug] [-debugscan] [-debugdef]\n" );
    return;
}

/*
 * These provide special begin/end of page information.  Note that
 * the page <TITLE> is part of the HEAD, not the BODY, so we need to 
 * have two bop commands.
 */
static FILE *eofpage = 0;
void WriteEndPage( FILE *fpout )
{
    int c;

    if (DebugOutput) fprintf( stdout, "WriteEndPage\n" );
    if (!eofpage) {
	if (endpagefilename[0]) {
	    eofpage = fopen( endpagefilename, "r" );
	    if (!eofpage) {
		fprintf( stderr, 
			 "Could not open endpage %s\n", endpagefilename );
		return;
	    }
	}
    }
    if (eofpage) {
	rewind( eofpage );
	while ((c = getc( eofpage )) != EOF) 
	    putc( c, fpout );
    }
    if (wrotebody)
	fprintf( fpout, "</BODY>%s", NewLineString );
    if (wrotehead)
	fprintf( fpout, "</HTML>%s", NewLineString );
    wrotebody = 0;
    InOutputBody = 0;
    if (DebugOutput) printf( "Set InOutputBody and wrotebody to 0\n" );
    
    wrotehead = 0;
}

static FILE *bofpage = 0;

void WriteBeginPage( FILE *fpout )
{
    int c;

    if (DebugOutput && wrotebody) 
	printf( "Skipping WriteBeginPage because writebody is true\n" );
    if (wrotebody) return;
    wrotebody    = 1;
    InOutputBody = 1;
    if (DebugOutput) printf( "Set InOutputBody and wrotebody to 1\n" );

    if (DebugOutput) fprintf( stdout, "WriteBeginPage\n" );
    /* Should parameterize this - command in basedefs? */
    fprintf( fpout, "</HEAD>%s<BODY BGCOLOR=\"#FFFFFF\">%s", 
	     NewLineString, NewLineString );
    if (!bofpage) {
	if (beginpagefilename[0]) {
	    bofpage = fopen( beginpagefilename, "r" );
	    if (!bofpage) {
		fprintf( stderr, "Could not open beginpage %s\n", 
			 beginpagefilename );
		return;
	    }
	}
	else
	    return;
    }
    rewind( bofpage );
    while ((c = getc( bofpage )) != EOF) 
	putc( c, fpout );
}

void WriteHeadPage( FILE *fpout )
{
    if (wrotehead) return;
    wrotehead = 1;
    if (DebugOutput) fprintf( stdout, "WriteHeadPage\n" );
    fprintf( fpout, "<HTML>%s<HEAD>%s", NewLineString, NewLineString );
    fprintf( fpout, "<!-- This file was generated by tohtml from %s -->%s",
	     InFName[curfile] ? InFName[curfile] : "unknown", NewLineString );
    fprintf( fpout, "<!-- with the command%stohtml %s%s-->%s", 
	     NewLineString, GetCommandLine(), NewLineString, NewLineString );
}

/* This routine copies the bitmap and gif files that might be used */
void CopyImgFiles( char *basefilename )
{
    char pgm[256];

#ifdef __MSDOS__
    char c1 = basefilename[strlen(basefilename)-1];

    if (c1 == '\\')
	basefilename[strlen(basefilename)-1] = 0;
    sprintf( pgm, "copy \"%s\\next.%s\" %s", BMSOURCE, ImageExt, basefilename );
    system( pgm );
    sprintf( pgm, "copy \"%s\\previous.%s\" %s", BMSOURCE, ImageExt, 
	     basefilename );
    system( pgm );
    sprintf( pgm, "copy \"%s\\up.%s\" %s", BMSOURCE, ImageExt, basefilename );
    system( pgm );
    if (IsGaudy) {
	sprintf( pgm, "copy \"%s\\purpleball.gif\" %s", BMSOURCE, basefilename );
	system( pgm );
	sprintf( pgm, "copy \"%s\\redball.gif\" %s", BMSOURCE, basefilename );
	system( pgm );
	sprintf( pgm, "copy \"%s\\blueball.gif\" %s", BMSOURCE, basefilename );
	system( pgm );
	sprintf( pgm, "copy \"%s\\greenball.gif\" %s", BMSOURCE, basefilename );
	system( pgm );
	sprintf( pgm, "copy \"%s\\yellowball.gif\" %s", BMSOURCE, basefilename );
	system( pgm );
    }
    if (c1 == '\\')
	basefilename[strlen(basefilename)] = c1;
#else
    sprintf( pgm, "/bin/cp %s/next.%s %s", BMSOURCE, ImageExt, basefilename );
    system( pgm );
    sprintf( pgm, "/bin/cp %s/previous.%s %s", BMSOURCE, ImageExt, 
	     basefilename );
    system( pgm );
    sprintf( pgm, "/bin/cp %s/up.%s %s", BMSOURCE, ImageExt, basefilename );
    system( pgm );
    if (IsGaudy) {
	sprintf( pgm, "/bin/cp %s/purpleball.gif %s", BMSOURCE, basefilename );
	system( pgm );
	sprintf( pgm, "/bin/cp %s/redball.gif %s", BMSOURCE, basefilename );
	system( pgm );
	sprintf( pgm, "/bin/cp %s/blueball.gif %s", BMSOURCE, basefilename );
	system( pgm );
	sprintf( pgm, "/bin/cp %s/greenball.gif %s", BMSOURCE, basefilename );
	system( pgm );
	sprintf( pgm, "/bin/cp %s/yellowball.gif %s", BMSOURCE, basefilename );
	system( pgm );
    }
#endif
}

/* Skip to the end of an HTML expression; return pointer to first char
   after expression */
char *SkipHTML( char *str )
{
    while (str && *str) {
	if (*str == '<')
	    str = SkipHTML( ++str );
	else if (*str == '>') return ++str;
	else
	    str++;
    }
    return 0;
}

void RemoveFonts( const char *instr, char *outstr )
{
    const char *pin = instr;
    char *pout = outstr;

    while (pin && *pin) {
	if (*pin == '<') 
	    pin = SkipHTML( (char *)++pin );
	else if (*pin == TOK_START || *pin == TOK_END)
	    pin++;
	else {
	    *pout++ = *pin++;
	}
    }
    *pout = 0;
}

static char cmdlin[1024];

void SaveCommandLine( int argc, char *argv[] )
{
    int totlen = 0, i;

    cmdlin[0] = 0;

    for (i=1; i<argc; i++) {
	if (!argv[i]) continue;
	totlen += strlen( argv[i] );
	if (totlen >= 1024) break;
	strcat( cmdlin, argv[i] );
	strcat( cmdlin, " " );
    }
}

char *GetCommandLine( void )
{
    return cmdlin;
}
