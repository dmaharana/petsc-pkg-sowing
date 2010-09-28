#include <stdio.h>
#include "tex.h"
#include "sowing.h"

void ProcessInfoFile();
void ProcessLatexFile();

/* We make these globals in consideration of the DOS version of this code */
char infilename[256], outfilename[256], projfilename[256], auxfilename[256];
char basefilename[256];
char basedir[256];
char basedefs[256];
char endpagefilename[256];
char beginpagefilename[256];

int NoBMCopy = 0;

/*
    This program converts files to Microsoft RTF format for input to the
    Windows help compiler.

    I'm using a C program instead of a perl script because I don't have a
    version of perl for my PC and the translation that I intend to do is
    fairly simple.  Eventually, something like perl is probably a better
    choice.

    To get the right, we really need a two-pass algorithm.  The first pass
    should assign the numbers used for a linear reading of the document; the
    second pass can use those rather than assigning them on the fly (the
    presence of menus tends to distort the ordering).  Rather than make this
    a two-pass code with the information held in memory, I'll start by writing
    out a file in linear order (fname.hod), then reading it in to predefine
    the names of the sections.
 */
int main( argc, argv )
int  argc;
char **argv;
{
FILE *fpin, *fpout, *fpaux;
void (*process)() = ProcessInfoFile;

int DestIsHtml     = 0;

if (argc < 2) {
    fprintf( stderr, "%s [-latex] [-order] [-basedef=filename] filename\n", 
	     argv[0] );
    return 1;
    }
TXSetCitePrefix( "[" );
TXSetCiteSuffix( "]" );

/* Read the simple TeX and LaTeX commands, along any user-defined names */
RdBaseDef( BASEDEF );
while (SYArgGetString( &argc, argv, 1, "-basedef", infilename, 256 )) {
    RdBaseDef( infilename );
    }

/* identify and open the input, output files */
if (SYArgHasName( &argc, argv, 1, "-latex" )) {
    process = ProcessLatexFile;
    }
/*
if (strcmp( argv[0], "-order") == 0) {
    process = CreateOrderFile;
    argv++;
    }
else {
    will want to read order file here
    }    
 */    
endpagefilename[0] = 0;
SYArgGetString( &argc, argv, 1, "-endpage", endpagefilename, 256 );

argv++;
argc--;
if (argc == 0 || !argv[0]) {
    fprintf( stderr, "Missing filename!\n" );
    return 1;
    }

strcpy( infilename, argv[0] );
strcpy( outfilename, argv[0] );
RemoveExtension( outfilename );
strcpy( basefilename, outfilename );
strcpy( projfilename, outfilename );
strcpy( auxfilename, outfilename );
strcat( outfilename, ".rtf" );
strcat( projfilename, ".hpj" );
strcat( auxfilename, ".aux" );


WriteHelpProjectFile( outfilename, projfilename );

fpin  = fopen( infilename, "r" );
fpout = fopen( outfilename, "w" );
if (!fpin || !fpout) {
    fprintf( stderr, "Could not open file %s and/or %s\n",
	     infilename, outfilename );
    exit(1);
    }

OpenAuxFile( auxfilename );    

/* Process the input file */
ProcessFile( argc, argv, fpin, fpout, process );

/* Close the files and write the project file */
fclose( fpin );
fclose( fpout );

return 0;
}


/*
    Write the help header to fp
 */
int WriteHeader( fp )
FILE *fp;
{
fprintf( fp, "\
{\\rtf1\\ansi\\deff0\\deflang1024\n\
{\\fonttbl\n\
 {\\f0\\froman Times New Roman;}\n\
 {\\f1\\froman Symbol;}\n\
 {\\f2\\fswiss Arial;}\n\
 {\\f3\\froman MS Serif;}\n\
 {\\f4\\fswiss MS Sans Serif;}\n\
 {\\f5\\fmodern Courier;}\n\
 {\\f6\\fdecor ZapfDingbats;}\n\
 {\\f7\\fswiss HelveticaCondensed;}\n\
 {\\f8\\fswiss Helvetica;}\n\
}\n\
\n\
{\\colortbl;\n\
 \\red0\\green0\\blue0;\n\
 \\red0\\green0\\blue255;\n\
 \\red0\\green255\\blue255;\n\
 \\red0\\green255\\blue0;\n\
 \\red255\\green0\\blue255;\n\
 \\red255\\green0\\blue0;\n\
 \\red255\\green255\\blue0;\n\
 \\red255\\green255\\blue255;\n\
 \\red0\\green0\\blue127;\n\
 \\red0\\green127\\blue127;\n\
 \\red0\\green127\\blue0;\n\
 \\red127\\green0\\blue127;\n\
 \\red127\\green0\\blue0;\n\
 \\red127\\green127\\blue0;\n\
 \\red127\\green127\\blue127;\n\
 \\red192\\green192\\blue192;\n\
}\n\n" );
}

/* Write the trailer */
int WriteTrailer( fp )
FILE *fp;
{
fprintf( fp, "}\n" );
}

int WriteFileTitle( fp, name )
FILE *fp;
char *name;
{
}

/*
    Write the header for a section

    name = name (to be printed)
    entrylevel = chapter,section, ...
    number     = number of section
    keywords (optional) = keywords for section
    level     = level of section (ignored for WINHELP)
 */
int WriteSectionHeader( fp, name, entrylevel, number, keywords, level )
FILE *fp;
char *name, *entrylevel, *keywords;
int  number, level;
{
fprintf( fp, "#{\\footnote %s%d}\n", entrylevel, number );
fprintf( fp, "${\\footnote %s}\n", name );
if (keywords) 
    fprintf( fp, "K{\\footnote %s}\n", keywords );
/* The + footnote puts topics into a specific order, allowing them to
   be read with the browse buttons.  Another approach would be to simply
   thread the document, start to finish, with these.

   An undocumented "feature" is that the "number" is sorted NOT numerically
   but lexigraphically.  Thus, 2 > 10. */
fprintf( fp, "+{\\footnote %s:%03d}\n", entrylevel, number );

fprintf( fp, "\\pard\\plain\\li120\\sb340\\sa120\\sl-320\\f3\\fs28\n%s\n\\par\n",
         name );
}

int WriteSectionAnchor( fp, name, entrylevel, number, level )
FILE *fp;
char *name, *entrylevel;
int  number, level;
{
}

int WriteJumpDestination( fp, name, title )
FILE *fp;
char *name, *title;
{
fprintf( fp, "#{\\footnote %s}\n", name );
fprintf( fp, "\\pard\\plain\n%s\n\\par\n", title );
}

/*
    Write the text to set the formatting for the body of a topic
 */
int WriteTextHeader( fp )
FILE *fp;
{
fprintf( fp, "\\par\\pard\\plain\\li120\\f4\\fs20\n" );
}

/*
    Write out popup text.  popup text refers to a topic (entryname/number)
 */
int WritePopupTextReference( fp, text, reftopic, refnumber )
FILE *fp;
char *text, *reftopic;
int  refnumber;
{
fprintf( fp, "{\\ul %s}{\\v %s%d}", text, reftopic, refnumber );
}

/*
    Write out pointers to other topics
 */
int WritePointerText( fp, text, reftopic, refnumber )
FILE *fp;
char *text, *reftopic;
int  refnumber;
{
fprintf( fp, "{\\uldb %s}{\\v %s%d}\n",
         text, reftopic, refnumber );
}    
    
/*
    Write out the end of a topic
 */
int WriteEndofTopic( fp )
FILE *fp;
{
fprintf( fp, "\\page\n" );
}	

void WriteLargeFont( fp )
FILE *fp;
{
fprintf( fp, "\\pard\\plain\\li120\\sb40\\sa40\\sl-80\\f3\\fs28\n" );
}

void WriteDefaultFont( fp )
FILE *fp;
{
fprintf( fp, "\\pard\\plain\n" );
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
ProcessFile( argc, argv, fpin, fpout, process )
int  argc;
char **argv;
FILE *fpin, *fpout;
void (*process)();
{
WriteHeader( fpout );
(*process)( argc, argv, fpin, fpout );
WriteTrailer( fpout );
}

int RemoveExtension( str )
char *str;
{
char *p;

p = str + strlen( str ) - 1;
while (p > str && *p != '.') p--;
*p = '\0';
}

/*
   This is a fairly simple help project file.  I've added OPTCDROM=yes
   which is supposed to optimize the layout of the help file for
   CDROM access.  Change to no or remove if not building for CDROMs.
 */
int WriteHelpProjectFile( name, outfilename )
char *name, *outfilename;
{
FILE *fpprj;
char curdir[128];

SYGetwd( curdir, 128 );
fpprj = fopen( outfilename, "w" );
if (!fpprj) {
    fprintf( stderr, "Could not open help project file %s\n", outfilename );
    return 0;
    }
fprintf( fpprj, "\
; This is a sample help project file\n\
[OPTIONS]\n\
WARNING=3\n\
REPORT=ON\n\
ROOT=%s\n\
COMPRESS=MEDIUM\n\
OPTCDROM=yes\n\
\n\
[FILES]\n\
%s\n\
\n\
[CONFIG]\n\
CreateButton( \"btn_Top\", \"&Top\", \"JumpId(`',`Node1')\")\n\
CreateButton( \"btn_Up\", \"&Up\", \"JumpId(`',`Node1')\")\n\
BrowseButtons()\n", curdir, name );
fclose( fpprj );
}

int WriteStartNewLine( fp )
FILE *fp;
{
fputs( "\\line\n", fp );
}	


/* This handles escaping of special characters.  These are {}\ .
   An additional "feature" is that no spaces are silently inserted by
   rtf.  This means that <char>\n needs to be change to <char> \n or
   to <char>\n<space>  I'll use the former since we may not want
   the space after the newline.
   */
int WriteString( fp, str )
FILE *fp;
char *str;
{
char *p;
char buffer[256];

#ifdef FOO
p = str;
while (*p) {
    if (*p == '{') {
    	/* *p = 0; */
    	if (p > str) {
	    strncpy( buffer, str, (int)(p-str) );
	    buffer[(int)(p-str)] = 0;
	    fputs( buffer, fp );
	    }
    	fputs( "\\{", fp );
    	str = p + 1;
        }
    else if (*p == '}') {
    	/* *p = 0; */
    	if (p > str) {
	    strncpy( buffer, str, (int)(p-str) );
	    buffer[(int)(p-str)] = 0;
	    fputs( buffer, fp );
	    }
    	fputs( "\\}", fp );
    	str = p + 1;
        }
    else if (*p == '\\') {
    	/* *p = 0; */
    	if (p > str) {
	    strncpy( buffer, str, (int)(p-str) );
	    buffer[(int)(p-str)] = 0;
	    fputs( buffer, fp );
	    }
    	fputs( "\\\\", fp );
    	str = p + 1;
        }
    p++;        
    }
if (*str) {
    if (str[strlen(str)-1] == '\n') {
	strcpy( buffer, str );
    	buffer[strlen(str)-1] = '\0';
    	fputs( buffer, fp );
    	fputs( " \n", fp );
        }
    else 
        fputs( str, fp );
    }
#endif
while (*str) {
    /* Eventually, we may need to replace some of the translations above ... */
    if (*str == TOK_START || *str == TOK_END) {
	/* Skip the token start/end */
	;
	}
    else if (*str == '\n') {
	fputc( ' ', fp );
	fputc( *str, fp );
	}
    else
	fputc( *str, fp );
    str++;
    }
}
int WriteStringRaw( fp, str )
FILE *fp;
char *str;
{
return WriteString( fp, str );
}

#include "search.h"
WriteSectionButtons( fout, name, l )
FILE *fout;
char *name;
LINK *l;
{
}

WriteSectionButtonsBottom( fout, name, l )
FILE *fout;
char *name;
LINK *l;
{
}

/*
   This changes the binding of the UP button to the given context
 */
int SetUpButton( fp, context )
FILE *fp;
char *context;
{
fprintf( fp,
"!{\\footnote ChangeButtonBinding(\"btn_Up\", \"JumpId(`',`%s')\")}\n",
   context );
}   

int WritePar( fp )
FILE *fp;
{
fputs( "\\par\n", fp );
}

int WriteBeginPointerMenu( fout )
FILE *fout;
{
}
int WriteEndOfPointer( fout )
FILE *fout;
{
fputs( "\\par\n", fout );
}

/* These next provide for paragraph justification */
WriteCenter( fp )
FILE *fp;
{
fputs( "\\qc ", fp );
}
WriteLeftAligned( fp )
FILE *fp;
{
fputs( "\\ql ", fp );
}
WriteRightAligned( fp )
FILE *fp;
{
fputs( "\\qr ", fp );
}
WriteJustified( fp )
FILE *fp;
{
fputs( "\\qj ", fp );
}
WritePlain( fp )
FILE *fp;
{
fputs( "\\pard\n", fp );
}

/* Paragraph indentation */
WriteRightIndent( fp, n )
FILE *fp;
int  n;
{
fprintf( fp, "\\ri%d ", n );
}
WriteLeftIndent( fp, n )
FILE *fp;
int  n;
{
fprintf( fp, "\\li%d ", n );
}
WriteFirstLineIndent( fp, n )
FILE *fp;
int  n;
{
fprintf( fp, "\\fi%d ", n );
}

/* Return the name of the file (without the extension) */
void GetBaseName( str )
char *str;
{
strcpy( str, basefilename );
}

/*
    Still to add...

    Support for bitmaps
    Support for ICONs (so that the icon represents the software)
    Support for DLL (for wav files etc)
    Support for more formatting options (tables, bold, italics)
 */

/*
   A quick summary of some rtf commands:

   \sa...   Space after a paragraph
   \sb...   Space before a paragraph
   \sl...   Line spaceing
       Sizes are in 1/20 pt
   \fs..    Font size in half points (fs28 is 14point)
   \pard    Set default paragraph properties
 */

static FILE *eofpage = 0;
int WriteEndPage( fpout )
FILE *fpout;
{
int c;
if (!eofpage) {
    if (endpagefilename[0]) {
	eofpage = fopen( endpagefilename, "r" );
	if (!eofpage) {
	    fprintf( stderr, "Could not open endpage %s\n", endpagefilename );
	    return;
	    }
	}
    else
	return;
    }
rewind( eofpage );
while ((c = getc( eofpage )) != EOF) 
    putc( c, fpout );
}

static FILE *bofpage = 0;
int WriteBeginPage( fpout )
FILE *fpout;
{
int c;
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
int WriteHeadPage( fpout )
FILE *fpout;
{
    return 0;
}
