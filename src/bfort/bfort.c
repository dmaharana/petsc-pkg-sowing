#include <ctype.h>
#include "sowing.h"
#include "doc.h"
#include "patchlevel.h"
#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/* 
   This is designed to work with comments in C programs.  
   It uses the standardized documentation to issue dummy routine
   definitions to allow the creation of a Fortran to C library.

   This version of "header.c" is a modification of the file taken 
   from "~gropp/tools.n/c2fort" on 10/7/93.  Modifications have been 
   introduced so that elements of type void * in the files "nlfunc_v.h", 
   and "nlspmat_v.h", and "nlsles_v.h" are translated as pointers to 
   structures in the Fortran version (instead of the default, which is
   no translation).  Note that pointers to void functions retain the 
   usual translation.  

   An additional flag (flag2) is used in the calling sequence of 
   ProcessArgDefs() to indicate the files which require the modified 
   translation.   Also, an additional element, void_function, is added 
   to the structure ARG_LIST to distinguish pointers to void functions 
   from pointers to void structures.  
 */
/* extern char GetSubClass(); */

static int NoFortMsgs = 1;
/* NoFortWarnings turns off messages about things not being available in
   Fortran */
static int NoFortWarnings = 1;

/* This says to convert char **a to int*a, and cast to (char **)*a */
static int MultipleIndirectAreInts    = 1;
static int MultipleIndirectsAreNative = 0;

/* Keep the file name to simplify finding files containing problems */
static char *CurrentFilename = 0;

/* Whether to replace pointers with indices to a mapping of pointers */
static int MapPointers = 0;
static char ptrprefix[32] = "__";

/* If this is set to true, "void *" are translated as pointers to structures */
static int TranslateVoidStar = 0;

/* If this is set to true, generate an "ifdef" block for the name of the
   Fortran interface to handle the case of no underscore and all caps
 */
static int IfdefFortranName = 0;

/* If true, add a last integer argument to int functions and return its
   value in the last parameter */
static int useFerr = 0;

/* Enable the MPI definitions */
static int isMPI = 0;

/* Enable and use MPI-2 conversion functions */
static int isMPI2 = 0;

/* Enable profiling name output */
static int DoProfileNames = 0;

/* If 1, output header immediately; otherwise, defer until declarations
   processed */
static int OutputImmed = 1;

/* If 1, generate ANSI style headers instead */
static int AnsiHeader = 0;

/* If 1, declarations are in ANSI prototype form */
static int AnsiForm = 0;

/* If 0, do not generate the ifndef DEBUG_ALL wrapper */
static int AddDebugAll = 1;

/* Catch serious problems */
#define MAX_ERR 100
static int ErrCnt = 0;

/* Debugging for development */
static int Debug = 0;
#define DBG(a) {if (Debug) printf(a);}
#define DBG2(a,b) {if (Debug)printf(a,b);}

/* Default #ifdef names for generated code */
static char FortranCaps[256] = "FORTRANCAPS";
static char FortranUscore[256] = "FORTRANUNDERSCORE";
static char FortranDblUscore[256] = "FORTRANDOUBLEUNDERSCORE";
static char Pointer64Bits[256] = "POINTER_64_BITS";
static char BuildProfiling[256] = "MPI_BUILD_PROFILING";

/* We also need to make some edits to the types occasionally.  First, note
   that double indirections are often bugs */
#define MAX_TYPE_NAME 60    /* Maximum length of a type name */
#define MAX_ARGS      128   /* Maximum number of args to a function */
#define MAX_TYPES      64   /* Maximum number of types to a function */

typedef struct {
    char *name;
    int  has_star, is_char, is_native, has_array,
	type,              /* Index into TYPE_LIST array */
	is_FILE, void_function;
    int  implied_star;
} ARG_LIST;
typedef struct {
    int is_char, is_native, implied_star, is_FILE, type_has_star, is_void;
    int is_mpi;
    char type[MAX_TYPE_NAME];
} TYPE_LIST;
typedef struct {
    char name[MAX_TYPE_NAME];
    int  num_stars;
} RETURN_TYPE;

/* Forward defs */
void OutputToken ANSI_ARGS(( FILE *, char *, int ));
void OutputRoutine ANSI_ARGS(( FILE *, FILE *, char *, char *, char ));
void SkipText ANSI_ARGS(( FILE *, char *, char *, char ));
int SkipToSynopsis ANSI_ARGS(( FILE *, char ));
int FindNextANToken ANSI_ARGS(( FILE *, char *, int * ));
void OutputBuf ANSI_ARGS(( FILE **, char *, char *, FILE *, char * ));
void OutputMacro ANSI_ARGS(( FILE *, FILE *, char *, char * ));
void ProcessFunctionType ANSI_ARGS(( FILE *, FILE *, char *, int *, 
				    char *, RETURN_TYPE *, int ));
void ProcessArgList ANSI_ARGS(( FILE *, FILE *, char *, int *, char *, 
				ARG_LIST[512], int *, RETURN_TYPE *, 
				int, TYPE_LIST *, int *, int ));
int ProcessArgDefs ANSI_ARGS(( FILE *, FILE *, ARG_LIST *, int, TYPE_LIST *,
			       int *, int *, int, char *, int ));
void PrintBody ANSI_ARGS(( FILE *, int, char *, int, int, ARG_LIST *,
			   TYPE_LIST *, RETURN_TYPE * ));
int NameHasUnderscore ANSI_ARGS(( char * ));
void OutputRoutineName ANSI_ARGS(( char *, FILE * ));
void OutputUniversalName ANSI_ARGS(( FILE *, char * ));
int GetTypeName ANSI_ARGS(( FILE *, FILE *, TYPE_LIST *, int, int, int ));
int GetArgName ANSI_ARGS(( FILE *, FILE *, ARG_LIST *, TYPE_LIST *, int ));
void OutputBalancedString ANSI_ARGS(( FILE *, FILE *, int, int ));
char *ToCPointer ANSI_ARGS(( char *, char *, int ));

void DoBfortHelp ANSI_ARGS(( char * ));

/*D
  bfort - program to extract short definitions for a Fortran to C interface

  Input:
. filenames - Names the files from which lint definitions are to be extracted
. -nomsgs   - Do not generate messages for routines that can not be converted
              to Fortran.
. -nofort   - Generate messages for all routines/macros without a Fortran
              counterpart.
. -dir name - Directory for output file
. -I name   - file that contains common includes
. -mapptr   - translate pointers to integer indices
. -ptrprefix - prefix for names of functions to convert to/from pointers
              (default is __).  The macro that selects the form based on the
              pointer size can be changed with -ptr64.
. -anyname   - Generate a single Fortran wrapper that works for almost all 
               systems, by adding C preprocessor names (see below).  These
               names can be changed with -fcaps, -fuscore, and -fduscore.
. -ferr     - Fortran versions return the value of the routine as the last
              argument (an integer).  This is used in MPI and is a not 
	      uncommon approach for handling error returns.
. -mpi      - Handle MPI datatypes (some things are pointers by definition)
. -mpi2     - Handle MPI datatypes using MPI2 converstion functions 
              (some things are pointers by definition)
. -no_pmpi  - Do not generate PMPI names
. -pmpi name - Change macro used to select MPI profiling version
. -noprofile - Turn off the generation of the profiling version
. -mnative  - Multiple indirects are native datatypes (no coercion)
. -voidisptr - Consider "void *" as a pointer to a structure.
. -ansi      - C routines use ANSI prototype form rather than K&R C form
. -ansiheader - Generate ANSI-C style headers instead of Fortran interfaces
  This will be useful for creating ANSI prototypes   without ANSI-fying the 
  code.  These use a trick to provide both ANSI and non-ANSI prototypes.
  The declarations are wrapped in "ANSI_ARGS", the definition of which 
  should be
.vb
  #ifdef ANSI_ARG
  #undef ANSI_ARG
  #endif
  #ifdef __STDC__
  #define ANSI_ARGS(a) a
  #else
  #define ANSI_ARGS(a) ()
  #endif
.ve
. -nodebug  - Do not add 
.vb
  #ifndef DEBUG_ALL
  #define DEBUG_ALL
  #endif
.ve
  to the wrapper file.
. -anyname   - Generate a single wrapper that can handle the three most common
               cases: trailing underscore, no underscore, and all caps.  The
               choice is based on whether 
.vb
     FORTRANCAPS:       Names are uppercase, no trailing underscore
     FORTRANUNDERSCORE: Names are lowercase, trailing underscore
              are defined.  
     FORTRANDOUBLEUNDERSCORE: Names are lowercase, with TWO trailing
.ve
      underscores.  This is needed when some versions of "f2c" are 
      used to generate C for Fortran routines.  Note that f2c uses two
      underscores ONLY when the name already contains an underscore
      (at least on the FreeBSD system that I use that uses f2c).
      To handle this case, the generated code contains the second
      underscore only when the name already contains one.

     If -mapptr is also chosen, then
.vb
     POINTER_64_BITS
.ve
               will also be used to determine if pointers are to long to
               fit in a 32-bit Fortran integer.  Routines that destroy
               a pointer will need to manually insert a call to 
               __RmPointer.  The routines for managing the pointers are 
	       in ptrcvt.c

	       In addition, if -mpi is used and -no_pmpi is not, the MPI 
               profiling names are also generated, surrounded by 
               MPI_BUILD_PROFILING.

  Note:
  We really need a way to specify a general type as a pointer, so that it
  will be handled as a pointer.  The -mpi option is a kludge for a pressing
  need.  Eventually should provide a "-ptr name" option and keep in a 
  search space when looking for known types.

  Author: 
  Bill Gropp
D*/
int main( argc, argv )
int  argc;
char **argv;
{
    char routine[MAX_ROUTINE_NAME];
    char *infilename;
    char outfilename[1024];
    char dirname[1024];
    char fname[1024], *p;
    FILE *fd, *fout, *incfd;
    char kind;
    char incfile[MAX_FILE_SIZE];
    char incbuffer[1024];
    int  n_in_file;

/* process all of the files */
    strcpy( dirname, "." );
    incfile[0]  = 0;
    SYArgGetString( &argc, argv, 1, "-dir", dirname, 1024 );
    SYArgGetString( &argc, argv, 1, "-I",   incfile, 1024 );
    NoFortMsgs		   = SYArgHasName( &argc, argv, 1, "-nomsgs" );
    MapPointers		   = SYArgHasName( &argc, argv, 1, "-mapptr" );
    if (MapPointers) {
	SYArgGetString( &argc, argv, 1, "-ptrprefix", ptrprefix, 32 );
    }
    useFerr		   = SYArgHasName( &argc, argv, 1, "-ferr" );
    isMPI		   = SYArgHasName( &argc, argv, 1, "-mpi" );
    isMPI2      	   = SYArgHasName( &argc, argv, 1, "-mpi2" );
    if (isMPI || isMPI2) DoProfileNames = 1;
    if (SYArgHasName( &argc, argv, 1, "-no_pmpi" ))
	DoProfileNames = 0;
    TranslateVoidStar	   = SYArgHasName( &argc, argv, 1, "-voidisptr" );
    MultipleIndirectsAreNative = SYArgHasName( &argc, argv, 1, "-mnative" );
    AnsiForm		   = SYArgHasName( &argc, argv, 1, "-ansi" );
    AnsiHeader		   = SYArgHasName( &argc, argv, 1, "-ansiheader" );
    AddDebugAll                = SYArgHasName( &argc, argv, 1, "-nodebug" );
    IfdefFortranName           = SYArgHasName( &argc, argv, 1, "-anyname" );
/* Get replacement names for ifdef items in generated code */
    SYArgGetString( &argc, argv, 1, "-fcaps", FortranCaps, 256 );
    SYArgGetString( &argc, argv, 1, "-fuscore", FortranUscore, 256 );
    SYArgGetString( &argc, argv, 1, "-fduscore", FortranDblUscore, 256 );
    SYArgGetString( &argc, argv, 1, "-ptr64", Pointer64Bits, 256 );
    SYArgGetString( &argc, argv, 1, "-pmpi", BuildProfiling, 256 );
    if (SYArgHasName( &argc, argv, 1, "-noprofile" )) DoProfileNames = 0;

    if (AnsiHeader) OutputImmed = 0;

    if (SYArgHasName( &argc, argv, 1, "-help" )) {
	DoBfortHelp( argv[0] );
	exit( 1 );
    }
    if (SYArgHasName( &argc, argv, 1, "-version" )) {
	printf( "bfort (sowing) release %d.%d.%d of %s\n",
		PATCHLEVEL, PATCHLEVEL_MINOR, PATCHLEVEL_SUBMINOR,
		PATCHLEVEL_RELEASE_DATE );
	exit( 1 );
    }
/* Open up the file of public includes */    
    if (incfile[0]) {
	incfd = fopen( incfile, "r" );
	if (!incfd) {
	    ErrCnt++;
	    fprintf( stderr, "Could not open file %s for -I option\n", incfile );
	    if (ErrCnt > MAX_ERR) exit(1);
	}
    }
    else
	incfd = 0;

    argc--; argv++;
    while (argc--) {
	/* Input filename */
	infilename = *argv++;
	fd = fopen( infilename, "r" );
	if (!fd) {
	    ErrCnt++;
	    fprintf( stderr, "Could not open file %s\n", infilename );
	    if (ErrCnt > MAX_ERR) exit(1);
	    continue;
        }
	n_in_file = 0;
	/* Remember file name */
	CurrentFilename = infilename;

	/* Set the output filename */
	SYGetRelativePath( infilename, fname, 1024 );
	/* Strip the trailer */
	p = fname + strlen(fname) - 1;
	while (p > fname && *p != '.') p--;
	*p = 0;
	/* Add an extra h to include files */
	if (p[1] == 'h') {
	    p[0] = 'h';
	    p[1] = 0;
	}
	if (AnsiHeader) 
	    sprintf( outfilename, "%s/%s.ansi", dirname, fname );
	else
	    sprintf( outfilename, "%s/%sf.c", dirname, fname );
	/* Don't open the filename yet (wait until we know that we'll have
	   some output for it) */
	fout = NULL;

	/* Pass 1: Generate the name mappings.
	   Eventually, we could store up the routine names and generate a single,
	   simpler block of definitions.
	   */
	while (FoundLeader( fd, routine, &kind )) {
	    if (!fout) {
		OutputBuf( &fout, infilename, outfilename, incfd, (char*)0 );
	    }
	    if (IfdefFortranName && fout && routine[0] && 
		(kind == ROUTINE || kind == MACRO)) {
		if (GetSubClass() != 'C') 
		    OutputUniversalName( fout, routine );
		SkipText( fd, routine, infilename, kind );
	    }
	    else if (kind == INCLUDE) {
		int guard_x = 0;
		strcpy( incbuffer, "#include " );
		/* Grumble.  We'll have to fix this eventually */
		if (routine[0] != '"' && routine[0] != '<') {
		    p = routine + strlen(routine) - 1;
		    if (*p == '>')
			strcat( incbuffer, "<" );
		    else if (*p == '"') 
			strcat( incbuffer, p );
		}
		/* Special case */
		/* fprintf( stderr, "include == |%s|\n", routine ); */
		if (strncmp( routine, "xtools/", 7 ) == 0) {
		    guard_x = 1;
		    OutputBuf( &fout, infilename, outfilename, incfd, 
			       "#ifndef TOOLSNOX11\n" );
		}
		strcat( incbuffer, routine );
		CopyIncludeName( fd, incbuffer + strlen(incbuffer) );
		strcat( incbuffer, "\n" );
		OutputBuf( &fout, infilename, outfilename, incfd, incbuffer );
		if (guard_x) {
		    OutputBuf( &fout, infilename, outfilename, incfd, 
			       "#endif\n" );
		    if (!fout) break;
		}
	    }
	    else SkipText( fd, routine, infilename, kind );

	}
	rewind( fd );
	if (fout) {
	    fprintf( fout, "\n\n/* Definitions of Fortran Wrapper routines */\n" );
/* BFS - next lines are to allow C++ code to be called from fortran */
	    fprintf( fout,"#if defined(__cplusplus)\n");
	    fprintf( fout,"extern \"C\" {\n");
	    fprintf( fout,"#endif\n");
/* BFS end of changes for C++ */
	    fflush( fout );
	}

	/* Pass 2: Generate the actual code */
	while (FoundLeader( fd, routine, &kind )) {
	    /* We need this test first to avoid creating an empty file, 
	       particularly for initf.c */
	    if ((kind == ROUTINE || kind == MACRO) && GetSubClass() == 'C') {
		if (!NoFortMsgs && !NoFortWarnings) {
		    fprintf( stderr, 
			     "%s %s(%s) can not be translated into Fortran\n",
			     (kind == ROUTINE) ? "Routine" : "Macro", 
			     routine, CurrentFilename );
		}
		SkipText( fd, routine, infilename, kind );
		continue;
	    }
	    if (GetIsX11Routine()) {
		OutputBuf( &fout, infilename, outfilename, incfd, 
			   "#ifndef TOOLSNOX11\n" );
	    }
	    if ((kind == ROUTINE || kind == MACRO) && fout == NULL) {
		OutputBuf( &fout, infilename, outfilename, incfd, (char *)0 );
		if (!fout) break;
	    }
#ifdef FOO
	    if (IfdefFortranName && fout && routine[0] && 
		(kind == ROUTINE || kind == MACRO)) {
		OutputUniversalName( fout, routine );
	    }
#endif
	    if (kind == ROUTINE) {
		n_in_file++;
		OutputRoutine( fd, fout, routine, infilename, kind );
	    }
	    else if (kind == MACRO) {
		/* Eventually we can handle this by using the Synopsis to
		   construct an equivalent definition */
		n_in_file ++;
		OutputMacro( fd, fout, routine, infilename );
	    }
/* moved include up BS */
	    if (GetIsX11Routine()) {
		OutputBuf( &fout, infilename, outfilename, incfd, 
			   "#endif\n" );
		if (!fout) break;
	    }
	}
	fclose( fd );
	if (fout) {
/* BFS added support for calling C++ from fortran */
	    fprintf(fout,"#if defined(__cplusplus)\n");
	    fprintf(fout,"}\n");
	    fprintf(fout,"#endif\n");
	    fclose( fout );
	    if (n_in_file == 0) {
		/* If all we put into the interface file was an include, we delete
		   it */
		unlink( outfilename );
	    }
	}
    }
    return 0;
}

void OutputToken( fout, p, nsp )
FILE *fout;
char *p;
int  nsp;
{
    int i;
    static int outcnt = 0;

    if (OutputImmed) {
	for (i=0; i<nsp; i++) putc( ' ', fout );
	fputs( p, fout );
	if (Debug) {
	    outcnt += nsp + strlen(p);
	    if (outcnt > 10000) {
		fprintf( stderr, "Exceeded output count limit!\n" );
		exit(1);
	    }
	}
    }
}

void OutputRoutine( FILE *fin, FILE *fout, char *name, char *filename,
		    char kind )
{
    int         is_function;
    ARG_LIST    args[MAX_ARGS];
    TYPE_LIST   types[MAX_TYPES];
    RETURN_TYPE rt;
    int         nargs, nstrings;
    int         ntypes;
    int         flag2 = 0;

/* Check to see if this is a C-only routine */
    if (GetSubClass() == 'C') {
	if (!NoFortMsgs && !NoFortWarnings) {
	    fprintf( stderr, "Routine %s(%s) can not be translated into Fortran\n",
		     name, CurrentFilename );
	}
	SkipText( fin, name, filename, kind );
	return;
    }

/* Skip to trailer */
    SkipText( fin, name, filename, kind );

/* Get the call to the routine, including finding the argument names */
    SkipWhite( fin );
    ProcessArgList( fin, fout, filename, &is_function, name, 
		    args, &nargs, &rt, 0, types, &ntypes, flag2 );

    if (!AnsiForm) {
	SkipWhite( fin );
	ProcessArgDefs( fin, fout, args, nargs, types, &ntypes, &nstrings, 0, 
			name, flag2 );
    }

    PrintBody( fout, is_function, name, nstrings, nargs, args, types, &rt );
}

/*
    This routine skips the text part of a text page.
 */        
void SkipText( FILE *fin, char *name, char *filename, char kind )
{
    int  c;
    char lineBuffer[MAX_LINE], *lp;
	
    lineBuffer[0] = '+';   /* Sentinal on lineBuffer */
    while (1) {
	lp = lineBuffer + 1;
	c  = getc( fin );
	if (c == EOF) break;
	if (c == ARGUMENT || c == VERBATIM)
	    SkipLine( fin );
	else if (c == '\n')
		;
	else {
	    if (isspace(c) && c != '\n')
		SkipWhite( fin );
	    else 
		*lp++ = c;
	    /* Copy to end of line; do NOT include the EOL */
	    while ((c = getc( fin )) != EOF && c != '\n') 
		*lp++ = c;
	    lp--;
	    while (isspace(*lp)) lp--;
	    lp[1] = '\0';    /* Add the trailing null */
	    if (lineBuffer[1] == kind && strcmp(lineBuffer+2,"*/") == 0)
		break;
        }
    }
}

int SkipToSynopsis( FILE *fin, char kind )
{
    int  c;
    char lineBuffer[MAX_LINE], *lp;
	
    lineBuffer[0] = '+';   /* Sentinal on lineBuffer */
    while (1) {
	lp = lineBuffer + 1;
	c  = getc( fin );
	if (c == EOF) break;
	if (c == ARGUMENT || c == VERBATIM)
	    SkipLine( fin );
	else if (c == '\n')
		;
	else {
	    if (isspace(c) && c != '\n')
		SkipWhite( fin );
	    else 
		*lp++ = c;
	    /* Copy to end of line; do NOT include the EOL */
	    while ((c = getc( fin )) != EOF && c != '\n') 
		*lp++ = c;
	    lp--;
	    while (isspace(*lp)) lp--;
	    lp[1] = '\0';    /* Add the trailing null */
	    if (lineBuffer[1] == kind && strcmp(lineBuffer+2,"*/") == 0)
		break;
	    if (lp[0] == ':') {
		lp = lineBuffer + 1;
		while (isspace(*lp)) lp++;
		LowerCase( lp );
		if (strcmp( lp, "synopsis:" ) == 0) 
		    return 1;
	    }
        }
    }
    return 0;
}

/* Find the next space delimited token; put the text into token.
   The number of leading spaces is kept in nsp.
   Alpha-numeric tokens are terminated by a non-alphanumeric character
   (_ is allowed in alpha-numeric tokens) */
int FindNextANToken( fd, token, nsp )
FILE *fd;
char *token;
int  *nsp;
{
    int fc, c, Nsp;

    Nsp = SkipWhite( fd );

    fc = c = getc( fd );
    if (fc == EOF) return fc;
    *token++ = c;
    if (isalnum(c) || c == '_') {
	while ((c = getc( fd )) != EOF) {
	    if (c != '\n' && (isalnum(c) || c == '_')) *token++ = c;
	    else break;
	}
	ungetc( (char)c, fd );
    }
    *token++ = '\0';
    *nsp     = Nsp;
    return fc;
}

void OutputBuf( fout, infilename, outfilename, incfd, buffer )
FILE **fout, *incfd;
char *infilename, *outfilename, *buffer;
{
    char arch[20];
    if (!*fout) {
	*fout = fopen( outfilename, "w" );
	if (!*fout) {
	    ErrCnt++;
	    fprintf( stderr, "Could not open file %s\n", outfilename );
	    if (ErrCnt > MAX_ERR) exit(1);
	    return;
	}
	fprintf( *fout, "/* %s */\n", infilename );
	if (!AnsiHeader) {
	    if (!IfdefFortranName) {
		SYGetArchType( arch, 20 );
		fprintf( *fout, "/* Fortran interface file for %s */\n", arch );
	    }
	    else {
		fprintf( *fout, "/* Fortran interface file */\n" );
	    }
	    fprintf( *fout, "\n/*\n\
* This file was generated automatically by bfort from the C source\n\
* file.  \n */\n" );
	    /* Turn on the base debugging */
	    if (AddDebugAll)
		fprintf( *fout, "#ifndef DEBUG_ALL\n#define DEBUG_ALL\n#endif\n" );
	    if (incfd) {
		int c;
		fseek( incfd, 0L, 0 );
		while ((c = getc( incfd )) != EOF) 
		    putc( (char)c, *fout );
	    }
	    if (MapPointers) {
/* BFS 3/5/96 code modified to support C++ on 64 bit machines */
		if (IfdefFortranName) {
		    if (AnsiHeader  || AnsiForm) {
			fprintf( *fout, "\n#ifdef %s\n\
#if defined(__cplusplus)\n\
extern \"C\" { \n\
#endif \n\
extern void *%sToPointer(int);\nextern int %sFromPointer(void *);\n\
extern void %sRmPointer(int);\n\
#if defined(__cplusplus)\n\
} \n\
#endif \n\
\n#else\n\
\n#define %sToPointer(a) (a)\n#define %sFromPointer(a) (int)(a)\
\n#define %sRmPointer(a)\n#endif\n\n", 
				 Pointer64Bits, 
				 ptrprefix, ptrprefix, ptrprefix, 
				 ptrprefix, ptrprefix, ptrprefix );
		    }
		    else {
			fprintf( *fout, "\n#ifdef %s\n\
#if defined(__cplusplus)\n\
extern \"C\" { \n\
#endif \n\
extern void *%sToPointer();\nextern int %sFromPointer();\n\
extern void %sRmPointer();\n\
#if defined(__cplusplus)\n\
} \n\
#endif \n\
\n#else\n\
\n#define %sToPointer(a) (a)\n#define %sFromPointer(a) (int)(a)\
\n#define %sRmPointer(a)\n#endif\n\n", 
				 Pointer64Bits, 
				 ptrprefix, ptrprefix, ptrprefix, 
				 ptrprefix, ptrprefix, ptrprefix );
		    }
		}
		else {
		    fprintf( *fout, 
			     "extern void *%sToPointer(); extern int %sFromPointer();\n",
			     ptrprefix, ptrprefix );
		}
	    }
	}
    }
    if (buffer) 
	fputs( buffer, *fout );
}


/* 
   There are a number of things to watch for.  One is that leading blanks are
   considered significant; since the text is being formated, we usually dont
   agree with that. 
 */
void OutputMacro( fin, fout, routine_name, filename )
FILE *fin, *fout;
char *routine_name, *filename;
{
    int         is_function;
    ARG_LIST    args[MAX_ARGS];
    TYPE_LIST   types[MAX_TYPES];
    RETURN_TYPE rt;
    int         nargs, nstrings;
    int         ntypes;
    int         has_synopsis;
    int         done;
    int         flag2 = 0;

/* Check to see if this is a C-only macro */
    if (GetSubClass() == 'C') {
	if (!NoFortMsgs && !NoFortWarnings) {
	    fprintf( stderr, "Macro %s(%s) can not be translated into Fortran\n",
		     routine_name, CurrentFilename );
	}
	SkipText( fin, routine_name, filename, MACRO );
	return;
    }

/* Skip to the synopsis in the body */
    has_synopsis = SkipToSynopsis( fin, MACRO );

    done = 0;
    if (has_synopsis) {
	/* Get the call to the routine, including finding the argument names */
	SkipWhite( fin );
	/* Process elements of type void * in the following files differently 
	   from the default */
	if ((strcmp( filename, "./nonlin/nlfunc_v.h") == 0) ||
	    (strcmp( filename, "./nonlin/nlspmat_v.h") == 0) ||
	    (strcmp( filename, "./nonlin/nlsles_v.h") == 0)  ||
	    TranslateVoidStar)
	    flag2 = 1;
	ProcessArgList( fin, fout, filename, &is_function, routine_name, 
			args, &nargs, &rt, 1, types, &ntypes, flag2 );
    
	if (!AnsiForm) {
	    SkipWhite( fin );
	    done = 
		ProcessArgDefs( fin, fout, args, nargs, types, &ntypes, &nstrings, 1, 
				routine_name, flag2 );
	}
	else
	    done = 1;
	PrintBody( fout, is_function, routine_name, nstrings, nargs, args, 
		   types, &rt );
    }
    else {
	ErrCnt++;
	fprintf( stderr, "%s(%s) has no synopsis section\n", 
		 routine_name, CurrentFilename );
	if (ErrCnt > MAX_ERR) exit(1);
    }
/* finish up the section */
    if (!done)
	SkipText( fin, routine_name, filename, MACRO );
}

/* Read the arg list and function type */

/* 
   This routine read the function type and name; that is, it processes
   things like "void *foo" and "void (*foo)()"
 */
void ProcessFunctionType( fin, fout, filename, is_function, name, rt, flag )
FILE        *fin, *fout;
char        *filename;
int         *is_function;
char        *name;
RETURN_TYPE *rt;
int         flag;
{
    static char rcall[1024];
    char *p, actname[1024];
    int  c, i;
    int  nsp;
    int  leadingm;
    int  in_args;
    int  found_name;

    SkipWhite( fin );
    in_args     = 0;
    p           = rcall;
    c           = FindNextANToken( fin, p, &nsp );
/* 
   We check for routines that return (functions) versus ones that don't
   by looking for "void".  A special case is functions that return 
   pointers to void; we check for these by looking at the first character
   of the first token after the void.

   We also want to defer generating the function type incase we need to 
   replace a pointer ref with an integer.
   */
    strcpy( rt->name, p );
    rt->num_stars = 0;
    *is_function          = strcmp( p, "void" );

    if (OutputImmed) {
	for (i=0; i<nsp; i++) putc( ' ', fout );
    }
/* fputs( p, fout ); */
    p += strlen( p );
    *p++ = ' ';
    leadingm = 0;    /* If a newline is encountered before this is one, AND
			this is a macro, insert one and exit */
    found_name = 0;
    actname[0] = 0;
    while (1) {
	c = FindNextANToken( fin, p, &nsp );
	if (c == EOF) {
	    ErrCnt++;
	    fprintf( stderr, "Unexpected EOF in %s\n", filename );
	    if (ErrCnt > MAX_ERR) exit(1);
	    return;
	}
	if (nsp > 0) strcat( rt->name, " " );
	if (strcmp( p, name ) != 0 && p[0] != '(')
	    strcat( rt->name, p );
	if (c == '*') {
	    *is_function = 1;
	    rt->num_stars++;
	}
	if (flag && c == '\n' && leadingm == 0) {
	    if (OutputImmed)
		fputs( "())", fout );
	    break;
	}
	if (c == '\n') leadingm = 1;
	if (c == '(') {
	    if (in_args == 0) {
		/* Output function type and name */
		if (rt->num_stars == 0 || !MapPointers) {
		    if (useFerr && strncmp( rt->name, "int", 3 ) == 0 ) {
			if (OutputImmed)
			    fputs( "void ", fout );
		    }
		    else {
			if (OutputImmed) {
			    fputs( rt->name, fout );
			    fputs( " ", fout );
			}
		    }
		}
		else {
		    if (OutputImmed) {
			fputs( "int ", fout );
		    }
		}
		if (OutputImmed) 
		    OutputRoutineName( name, fout );
	    }
	    ungetc( '(', fin );
	    break;
	}
	if (c == ')') {
	    in_args -= 1;
	    if (in_args == 0) {
		break;
	    }
	}
	strcpy( actname, p );
	if (in_args == 0) {
	    if (strcmp( p, name ) == 0) {
		/* Convert to Fortran name.  For now, this just does the
		   lowercase_ version */
		found_name = 1;
	    }
	    /* This test should be postponed until the end (e.g.,
	       struct tm *foo() */
	    /* 
	       else {
	       if (p[0] != '*') {
	       Errcnt++;
	       fprintf( stderr, "%s:Did not find matching name: %s != %s\n", 
	       filename, p, name );
	       if (ErrCnt > MAX_ERR) exit(1);
	       }
	       }
	       */
	}
    }
    if (!found_name) {
	ErrCnt++;
	fprintf( stderr, "%s:Did not find routine name (may be untyped): %s \n", 
		 filename, name );
	if (ErrCnt > MAX_ERR) exit(1);
    }
    else if (strcmp( name, actname ) != 0) {
	ErrCnt++;
	fprintf( stderr, "%s:Did not find matching name: %s != %s\n", 
		 filename, actname, name );
	if (ErrCnt > MAX_ERR) exit(1);
    }
}

/* We are moving to being able to suppress generating the output until the
   argument definitions are read.
   flag is 1 for C routines, 0 for macros (I think)  */
void ProcessArgList( fin, fout, filename, is_function, name, args, Nargs,
		     rt, flag, types, Ntypes, flag2 )
FILE        *fin, *fout;
char        *filename, *name;
ARG_LIST    args[512];
RETURN_TYPE *rt;
int         *Nargs;
int         *is_function, flag;
TYPE_LIST   *types;
int         *Ntypes, flag2;
{
    int             c, ntypes;
    char            *p;
    int             nsp, leadingm;
    static char     rcall[1024];
    int             nargs, in_args;
    TYPE_LIST       *curtype;
    int             outparen;

    ProcessFunctionType( fin, fout, filename, is_function, name, rt, flag );

    nargs       = 0;
    in_args     = 0;
    p           = rcall;

    leadingm = 0;    /* If a newline is encountered before this is one, AND
			this is a macro, insert one and exit */
    curtype = (TYPE_LIST *)0;
    ntypes  = 0;
/* Get the opening ( */
    c = FindNextANToken( fin, p, &nsp );
    if (c != '(') {
	ErrCnt++;
	fprintf( stderr, "Expected a (, found %s\n", p );
	if (ErrCnt > MAX_ERR) exit(1);
	return;
    }
    OutputToken( fout, p, nsp );
    while (1) {
	/* First, get the type name.  Note that there might not be one */
	if (AnsiForm) {
	    curtype = &types[ntypes];
	    outparen = ntypes > 0;
	    if ((c = GetTypeName( fin, fout, &types[ntypes], flag, flag2, 
			     outparen ))) {
		if (ntypes == 0 && AnsiForm && c == ')') {
		    fprintf( stderr, 
		     "Empty argument list in -ansi mode (use (void))\n");
		    /* For this to work, gettypename can't output the last
		       closing paren */
		    if ( useFerr ) {
			fprintf( fout, "int *__ierr ");
		    }
		}
		if (c != 1) {
		    char cstring[2];
		    cstring[0] = c; cstring[1] = 0;
		    OutputToken( fout, cstring, 0 );
		}
		break;
	    }
	    ntypes++;
	}
	/* Now, get the variable names until the arg terminator.  They are of the
	   form [(\*]*name[(\*\[]*
	   */
	if (GetArgName( fin, fout, &args[nargs], curtype, AnsiForm )) {
	    break;
	}
	args[nargs].type = ntypes-1;
	if (curtype && curtype->type_has_star)
	    args[nargs].has_star++;
	if (curtype) {
/* added by BS */
	    if (curtype->implied_star)
		args[nargs].implied_star++;
	    args[nargs].is_native = curtype->is_native;
	}
	if (nargs > 511) {
	    ErrCnt++;
	    fprintf( stderr, "Too many arguments to function\n" );
	    exit(1);
	}
	nargs++;
	/* Get between-arg character */
	c = FindNextANToken( fin, p, &nsp );
	if (c == '(') {
	    /* Need to skip to corresponding ')' */
	    OutputBalancedString( fin, fout, '(', ')' );
	    c = FindNextANToken( fin, p, &nsp );
	    /* This is a function */
	    args[nargs-1].void_function = 1;
	}
	if (c == ')') {
	    if (OutputImmed) {
		if (useFerr) {
/* added AnsiForm BS Aug 20, 1995 */
		    if (AnsiForm) {
			fprintf( fout, "%sint *__ierr ", (nargs > 0) ? ", " : "" );
		    }
		    else {
			fprintf( fout, "%s__ierr ", (nargs > 0) ? ", " : "" );
		    }
		}
		fputs( ")", fout ); 
	    }
	    break;
	}
	OutputToken( fout, p, nsp );
    }

/* Handle definitions of the form "type (*Name( args, ... ))()" (this is
   function returns pointer to function returning type). */
    SkipWhite( fin );
    c = getc( fin );
    if (c == '(') {
	SkipWhite( fin );
	c = getc(fin);
	if (c == ')') 
	    fputs( "()", fout );
	else 
	    ungetc( (char)c, fin );
    }
    else 
	ungetc( (char)c, fin );

    if (AnsiForm) {
	/* Handle declaration of form int foo(void) */
	if (ntypes == 1 && nargs == 0 && strcmp("void",types[0].type) == 0) {
	    ntypes = 0;
	}
    }
    *Nargs  = nargs;
    *Ntypes = ntypes;
/* If being called from Fortran, we need to append dummy ints for the strings
   passed in.  This requires that we defer to the end of the argument 
   passing the printing of the function declaration line */
/* for (i=0; i<nstrings; i++) fprintf( fout, ",d%d", i ); */

}

/* Read the arg list and function type */

/* if flag == 1, stop on empty line rather than { */
/* This needs to distinguish between pointers and values, since all
   parameters are passed by reference in Fortran.  Just to keep things
   lively, there are two ways to indicate a pointer in C:
     type *foo;
     type foo[];

   Needed to add a change that definitions are terminated by ;, not by \n.

   ANSI version has no args predefined, types separated by comma, and 
   the same default type (int) as for K&R form.
   
   Returns 1 if it saw the end of a macro, 0 otherwise (see OutputMacro)
 */
int ProcessArgDefs( fin, fout, args, nargs, types, Ntypes, Nstrings, flag, 
		    name, flag2 )
FILE      *fin, *fout;
ARG_LIST  *args;
int       nargs;
TYPE_LIST *types;
int       *Ntypes, *Nstrings, flag, flag2;
char      *name;
{
    int      c;
    char     token[1024];
    int      i, nsp, newline, newstmt;
    int      in_function;
    int      nstrings;
    int      ntypes, set_void, void_function;
    int      done = 0;         /* set to 1 if ate end-of-definition */
    TYPE_LIST *curtype;
    ARG_LIST  narg;

    newline		  = 1;
    newstmt		  = 1;
    if (flag) newline = 0;
    nstrings	  = 0;
/* The default type is int */
    ntypes		  = 1;
    strcpy( types[0].type, "int" );
    in_function	  = 0;
    set_void	  = 0;
    void_function	  = 0;

/* This should really use a better parser. 
   Types are
   [register] [const] [struct] typename [ *( ]* varname [(*\[]* 
   separated by , (ANSI) or ; (K&R), and terminated by
   ')' (ANSI) or '{' (K&R)

   A modification is that in K&R form, after a ',', the 
   [register] [struct] typename is carried forward until a ';'

   The known typenames (and optional [register][const][struct] are 
   processed by GetTypeName;
 */
    while (1) {
	curtype = &types[ntypes];
	if ((done = GetTypeName( fin, fout, &types[ntypes++], flag, flag2, 1 )))
	    break;
	while (1) {
	    if (GetArgName( fin, fout, &narg, curtype, 1 )) break;
	    /* match arg to input argument */
	    for (i=0; i<nargs; i++) {
		if (strcmp( narg.name, args[i].name ) == 0) break;
	    }
	    if (i >= nargs) {
		ErrCnt++;
		fprintf( stderr, "Could not find argument %s\n", narg.name );
		if (ErrCnt > MAX_ERR) exit(1);
	    }
	    args[i]		     = narg;
	    args[i].type	     = ntypes-1;
	    args[i].implied_star = curtype->implied_star;
	    args[i].is_char	     = curtype->is_char;
	    args[i].is_FILE	     = curtype->is_FILE;
	    args[i].is_native    = curtype->is_native;
	    if (curtype->type_has_star)
		args[i].has_star++;
	    if (args[i].is_char)
		nstrings++;
	    c = FindNextANToken( fin, token, &nsp );
	    OutputToken( fout, token, nsp );
	    if (c == ';') break;
	}

#ifdef FOO
	/* Handle various argument features */
	if (c == '*')                  has_star++;
	else if (c == '(') {
	    in_function = 1;
	    /* If set_void is activated, set the void function indicator */
	    if (set_void) {        
		set_void = 0;
		void_function = 1;
            }
	}
	else if (c == ')' && in_function) {
	    is_function = 1;
	}
	else if (c == '\n') {
	    /* New lines have little meaning in declarations.
	       However, they are necessary to handle blanks lines */
	    newline = 1;
	}
	else if (newstmt) {
#endif
#ifdef FOO
	    if (!has_star) {
		/* This makes it look nicer */
		nsp = 0;
		OutputToken( fout, "*", nsp );
	    }
	    if (has_array) OutputToken( fout, "[]", 0 );
#endif
	}
/* Add the final error return definition */
	if (useFerr && OutputImmed) {
	    fputs( "int *__ierr;\n", fout );
	}
	*Ntypes   = ntypes;
	*Nstrings = nstrings;
	return done == 2;
    }

/*
    Pointer mashing.  There are two kinds of pointer mashing available.
    For systems for which a pointer will fit into an int, we simply
    use the ints to store the pointers.  In this case, a pointer is
    passed to a C routine by using:

    (type *)*(int *)varname

    That is, we convert the varname to an address of an int and deref it.

    On systems with pointers that are longer than ints, we have to do more.
    
    The first step is to convert pointers to indices on input and output
    from the routines.

    The routine __ToPointer converts an index into a pointer.
    The routine __FromPointer converts from a pointer to an index
    The routine __RmPointer   removes a pointer from the table of pointers

    __FromPointer always allocates a new pointer item.
 */
char *ToCPointer( type, name, implied_star )
char *type, *name;
int  implied_star;
{
    static char buf[300];
    
    if (isMPI2) {
	/* If the type is an MPI type, use the MPI conversion 
	   function */
	buf[0] = 0;
	if (strcmp("MPI_Comm",type) == 0) {
	    sprintf( buf, "\n\tMPI_Comm_f2c( *(%s) )", name );
	}
	else if (strcmp( "MPI_Request",type) == 0) {
	    sprintf( buf, "\n\tMPI_Request_f2c( *(%s) )", name );
	}
	else if (strcmp( "MPI_Group", type) == 0) {
	    sprintf( buf, "\n\tMPI_Group_f2c( *(%s) )", name );
	}
	else if (strcmp( "MPI_Op", type ) == 0) {
	    sprintf( buf, "\n\tMPI_Op_f2c( *(%s) )", name );
	}
	else if (strcmp( "MPI_Datatype", type ) == 0) {
	    sprintf( buf, "\n\tMPI_Datatype_f2c( *(%s) )", name );
	}
	else if (strcmp( "MPI_Win", type ) == 0) {
	    sprintf( buf, "\n\tMPI_Win_f2c( *(%s) )", name );
	}
	else if (strcmp( "MPI_File", type ) == 0) {
	    sprintf( buf, "\n\tMPI_File_f2c( *(%s) )", name );
	}
	if (buf[0]) return buf;
    }
    if (MapPointers) 
	sprintf( buf, "\n\t(%s%s)%sToPointer( *(int*)(%s) )", 
		 type, !implied_star ? "* " : "", ptrprefix, name );
    else
	sprintf( buf, "\n\t(%s%s)*((int*)%s)", type, !implied_star ? "* " : "", 
		 name );
    
    return buf;
}
/*
   A major question is whether "void *" should be considered the actual
   pointer or an address containing the value of the pointer (the usual "int"
   trick). 

   Since "void *" is used heavily in the communications routines to refer
   to any one of the type double*, int*, ..., I'm going to add void * to
   the list of types that are not translated
 */
void PrintBody( fout, is_function, name, nstrings, nargs, args, types, rt )
FILE        *fout;
char        *name;
int         is_function, nstrings, nargs;
ARG_LIST    *args;
TYPE_LIST   *types;
RETURN_TYPE *rt;
{
    int  i, j;
    
/* Known bugs in ansiheader:
   Definitions like     void (*fcn)() fail
   Multiple indirection (char **argv) fail
   */  
    if (!OutputImmed) {
	/* Output the function definition */
	if (AnsiHeader) fputs( "extern ", fout );
	fputs( rt->name, fout );
	fputs( " ", fout );
	OutputRoutineName( name, fout );
	if (AnsiHeader) fputs( " ANSI_ARGS(", fout );
	fprintf( fout, "(" );
	for (i=0; i<nargs-1; i++) {
	    if (AnsiHeader) {
		fprintf( fout, "%s", types[args[i].type].type );
		if (args[i].has_star > 0) {
		    for (j=0; j<args[i].has_star; j++) 
			fputs( "*", fout );
		}
		fputs( ", ", fout );
	    }
	    else 
		fprintf( fout, "%s, ", args[i].name );
	}
	if (nargs > 0) {
	    /* Do the last arg, if any */
	    if (AnsiHeader) {
		fprintf( fout, "%s ", types[args[nargs-1].type].type );
		if (args[nargs-1].has_star > 0) {
		    for (j=0; j<args[nargs-1].has_star; j++) 
			fputs( "*", fout );
		}
	    }
	    else 
		fprintf( fout, "%s ", args[nargs-1].name );
	}
	else {
	    if (AnsiHeader)
		/* A routine with no arguments gets a 'void' as the argument 
		   name */
		fputs( "void", fout );
	}
	if (nstrings && !AnsiHeader) {
	    for (i=1; i<nstrings; i++) fprintf( fout, ",d%d", i );
	    /* Undefined variables are int's by default */
	    /* fprintf( fout, "int d0" );
	       for (i=1; i<nstrings; i++) fprintf( fout, ",d%d", i );
	       fputs( ";\n", fout );
	       */
	}
	fprintf( fout, ")" );
	if (AnsiHeader) {
	    /* No more to do */
	    fputs( ");\n", fout );
	    return;
	}
	else
	    fputs( "\n", fout );
    }
    fputs( "{\n", fout );
/* Look for special-case translations (currently, "FILE") */
    for (i=0; i<nargs; i++) {
	if (args[i].is_FILE) {
	    fprintf( fout, "FILE *_fp%d = stdout;\n", i );
	}
    }

/* Generate the routine call with the return */
    if (is_function) {
	if (useFerr) {
	    fputs( "*__ierr = ", fout );
	}
	else {
	    fputs( "return ", fout );
	    /* May have to convert type */
	    if (MapPointers && rt->num_stars > 0) {
		/* In this case, we return an integer */
		fprintf( fout, "%sFromPointer( (void *)( ", ptrprefix );
	    }
	}
    }
    fputs( name, fout );
    fputs( "(", fout );
    for (i=0; i<nargs; i++) {
	if (args[i].is_FILE) 
	    fprintf( fout, "_fp%d", i );
	else if (!args[i].is_native && args[i].has_star 
		 && !args[i].void_function) {
	    if (args[i].has_star == 1 || !MultipleIndirectAreInts) 
		fprintf( fout, "%s", 
			 ToCPointer( types[args[i].type].type, args[i].name,
				     args[i].implied_star ) );
	    else {
		if (MultipleIndirectsAreNative) {
		    fprintf( fout, "%s", args[i].name );
		}
		else {
		    fprintf( fout, "(%s ", types[args[i].type].type );
		    if (!args[i].implied_star)
			for (j = 0; j<args[i].has_star; j++) fputs( "*", fout );
		    fprintf( fout, ")*((int *)%s)", args[i].name );
		}
	    }
	}
	else {
	    if (!args[i].has_star) 
		fputs( "*", fout );
	    fputs( args[i].name, fout );
	}
	if (i < nargs-1) fputs( ",", fout );
    }
/* fputs( rcall, fout ); */

    if (is_function && MapPointers && rt->num_stars > 0 && !useFerr) {
	fprintf( fout, ") )" );
    }
    fputs( ");\n}\n", fout );
}

int NameHasUnderscore( p )
char *p;
{
    while (*p) 
	if (*p++ == '_') return 1;
    return 0;
}

void OutputRoutineName( name, fout )
char *name;
FILE *fout;
{
    char buf[256], *p;
    int  ln;

    p = buf;
    strcpy( buf, name );
    if (!AnsiHeader) {
	if (IfdefFortranName) {
	    LowerCase( p );
	    ln = strlen( p );
	    p[ln] = '_';
	    p[ln+1] = 0;
	}
	else {
#if defined(FORTRANCAPS)
	    UpperCase( p );
#elif defined(FORTRANUNDERSCORE)	    
	    LowerCase( p );
	    ln	= strlen( p );
	    p[ln]	= '_';
	    p[ln+1]	= 0;
#elif defined(FORTRANDOUBLEUNDERSCORE)
	    LowerCase( p );
	    ln	= strlen( p );
	    if (NameHasUnderscore( p )) {
		p[ln]	= '_';
		p[ln+1]	= '_';
		p[ln+2]	= 0;
	    }
	    else {
		p[ln]	= '_';
		p[ln+1]	= 0;
	    }
#else
	    LowerCase( p );
#endif
	}
    }
    fputs( buf, fout );
}

void OutputUniversalName( fout, routine )
FILE *fout;
char *routine;
{
    char basename[MAX_ROUTINE_NAME], capsname[MAX_ROUTINE_NAME],
	nouscorename[MAX_ROUTINE_NAME];
    strcpy( basename, routine );
    LowerCase( basename );
    strcat( basename, "_" );
    strcpy( capsname, routine );
    UpperCase( capsname );
    strcpy( nouscorename, routine );
    LowerCase( nouscorename );
    if (isMPI && DoProfileNames) {
	if (NameHasUnderscore(nouscorename)) {
	    fprintf( fout, "\
#ifdef %s\n\
#ifdef %s\n\
#define %s P%s\n\
#elif defined(%s)\n\
#define %s p%s_\n\
#elif !defined(%s)\n\
#define %s p%s\n\
#else\n\
#define %s p%s\n\
#endif\n\
#else\n\
#ifdef %s\n\
#define %s %s\n\
#elif defined(%s)\n\
#define %s %s_\n\
#elif !defined(%s)\n\
#define %s %s\n\
#endif\n\
#endif\n\n", BuildProfiling,
	     FortranCaps, basename, capsname, 
	     FortranDblUscore, basename, basename, 
	     FortranUscore, basename, nouscorename, 
	     basename, basename, 
	    
	     FortranCaps, basename, capsname, 
	     FortranDblUscore, basename, basename, 
	     FortranUscore, basename, nouscorename );
	}
    else {
	fprintf( fout, "\
#ifdef %s\n\
#ifdef %s\n\
#define %s P%s\n\
#elif !defined(%s) && !defined(%s)\n\
#define %s p%s\n\
#else\n\
#define %s p%s\n\
#endif\n\
#else\n\
#ifdef %s\n\
#define %s %s\n\
#elif !defined(%s) && !defined(%s)\n\
#define %s %s\n\
#endif\n\
#endif\n\n", BuildProfiling, 
	     FortranCaps, basename, capsname, 
	     FortranUscore, FortranDblUscore, basename, nouscorename, 
	     basename, basename, 
	    
	     FortranCaps, basename, capsname, 
	     FortranUscore, FortranDblUscore, basename, nouscorename );
	}
    }
else {
    if (NameHasUnderscore(nouscorename)) {
	fprintf( fout, "\
#ifdef %s\n\
#define %s %s\n\
#elif defined(%s)\n\
#define %s %s_\n\
#elif !defined(%s)\n\
#define %s %s\n\
#endif\n",  FortranCaps, basename, capsname, 
	    FortranDblUscore, basename, basename, 
	    FortranUscore, basename, nouscorename );
	}
    else {
	fprintf( fout, "\
#ifdef %s\n\
#define %s %s\n\
#elif !defined(%s) && !defined(%s)\n\
#define %s %s\n\
#endif\n",  FortranCaps, basename, capsname, 
	    FortranUscore, FortranDblUscore, basename, nouscorename );
	}
    }
}

/*
   Read the type name.  Handles the known types and user-defined types

   Special case (void) must not generate output IF useFerr is set.

   Return non-zero if a non-type name is encountered.  

   Activate set_void only for the files specified by flag2 
   
   The flag outparen is true if paren characters should be output;
   false otherwise.  If outparen is false, the character will be returned.
   Effective only if AnsiForm is true.
   
 */
int GetTypeName( fin, fout, type, is_macro, flag2, outparen )
FILE     *fin, *fout;
TYPE_LIST *type;
int       is_macro, flag2, outparen;
{
int             c, nsp;
char            token[1024];
char            *typename = type->type;
int             last_was_newline = 0;

typename[0]	    = 0;
type->is_char	    = 0;
type->is_native	    = 0;
type->is_FILE	    = 0;
type->implied_star  = 0;
type->type_has_star = 0;
type->is_void       = 0;
type->is_mpi        = 0;

DBG("Looking for type...\n")
/* Skip register */
SkipWhite( fin );
c = FindNextANToken( fin, token, &nsp );
while (c != EOF && c == '\n') {
    /* Macro typedefs end with a blank line */
    if (is_macro && last_was_newline) return 1;
    last_was_newline = 1;
    OutputToken( fout, token, nsp );
    c = FindNextANToken( fin, token, &nsp );
    }
/* Now we check for end of type definitions.  This is either a
   { in K&R, ) in ANSI, or M * / in a Macro defn */
if (c == EOF) return 1;
if (c == '{') {
    /* We don't output the initial brace here (see printbody) */
    return 1;
    }
if (AnsiForm && (c == '(' || c == ')')) {
    if (outparen)
	OutputToken( fout, token, nsp );
    else 
	return c;
    return 1;
    }
/* The macro form should stop at a newline as well */
if (is_macro && c == MACRO) {
    DBG("Checking for macro\n")
    c = getc( fin );
    if (c == '*') {
	c = getc( fin );
	if (c == '/') {
	    DBG("Found end of macro defn\n")
	    return 2;
	    }
	else { 
	    /* This won't work on all systems. */
	    ungetc( '*', fin );
	    ungetc( (char)c, fin );
	    }
	}
    else 
	ungetc( (char)c, fin );
    }

if (strcmp( token, "register" ) == 0) {
    c = FindNextANToken( fin, token, &nsp );
    if (c == EOF || c == '{' || (AnsiForm && c == '(')) return 1;
    }

if (strcmp( token, "const" ) == 0) {
    c = FindNextANToken( fin, token, &nsp );
    if (c == EOF || c == '{' || (AnsiForm && c == '(')) return 1;
    }

/* Read type declaration: struct name or [ unsigned ] type */
if (strcmp( token, "struct" ) == 0) {
    strcat( typename, token );
    strcat( typename, " " );
    OutputToken( fout, token, nsp );
    c = FindNextANToken( fin, token, &nsp );
    strcat( typename, token );
    }
else {
    if (strcmp( token, "unsigned" ) == 0) {
	strcat( typename, token );
	strcat( typename, " " );
	OutputToken( fout, token, nsp );
	c = FindNextANToken( fin, token, &nsp );
	}
    strcat( typename, token );
    /* Look for known names */
    if (strcmp( token, "char" ) == 0) type->is_char = 1;
    if (strcmp( token, "FILE" ) == 0) type->is_FILE = 1;
    /* Note that we might want special processing for short and long */
    /* Some of these are NOT C types (complex, BCArrayPart)! */
    if (strcmp( token, "double" ) == 0 ||
	strcmp( token, "int"    ) == 0 ||
	strcmp( token, "short"  ) == 0 ||
	strcmp( token, "long"   ) == 0 ||
	strcmp( token, "float"  ) == 0 ||
	strcmp( token, "char"   ) == 0 ||
	strcmp( token, "complex") == 0 ||
	strcmp( token, "dcomplex")== 0 ||
	strcmp( token, "MPI_Status") == 0 ||
	strcmp( token, "Scalar")     == 0 ||
	strcmp( token, "PetscTruth") == 0 ||
	strcmp( token, "BCArrayPart") == 0 ||
	strcmp( token, "PLogDouble") == 0)
        type->is_native = 1;
    /* PETSc types that are implicitly pointers are specified here */
    /* This really needs to take the types from a file, so that
       it can be configured for each package.  See the search code in 
       info2rtf (but do a better job of it) */
    if (strcmp(token,"XBWindow") == 0 || strcmp(token,"KSP") == 0 ||
        strcmp(token,"Vec") == 0 || strcmp(token,"IS") == 0 ||
        strcmp(token,"Mat") == 0 || strcmp(token,"SLES") == 0 ||
        strcmp(token,"PC") == 0 || strcmp(token,"Draw") == 0 ||
        strcmp(token,"DrawLG") == 0 || strcmp(token,"SNES") == 0 ||
        strcmp(token,"DA") == 0 || strcmp(token,"Grid") == 0 ||
        strcmp(token,"Stencil") == 0 || strcmp(token,"DrawAxis") == 0 ||
        strcmp(token,"PetscObject") == 0 || strcmp(token,"TS") == 0 ||
        strcmp(token,"DF") == 0 || strcmp(token,"PetscRandom") == 0 ||
        strcmp(token,"VecScatter") == 0 ||
        strcmp(token,"AO") == 0 ||
        strcmp(token,"GVec") == 0 ||
        strcmp(token,"Viewer") == 0) {
	/* type->has_star      = 1; */
	type->type_has_star = 1;
	type->implied_star  = 1;
	}
    
    /* This should be an "mpi defs file" rather than just -mpi */
    if (isMPI) {
	/* Some things need to be considered ints in the declarations.
	   That is, these are "implicit" pointer objects; often they
	   are pointers to be returned from the calling routine. 
	   These tests set these up as being pointer objects 
	   In many recent implementations, they are now ints.
	   There are also the MPI-2 functions for converting, which
	   we should use (actually, we should have a table that 
	   we can read in).
	 */
	if (strcmp( token, "MPI_Comm" ) == 0    ||
	    strcmp( token, "MPI_Request" ) == 0 ||
	    strcmp( token, "MPI_Group" ) == 0   || 
	    strcmp( token, "MPI_Op" ) == 0      ||    
	    strcmp( token, "MPI_Uop" ) == 0     ||    
	    strcmp( token, "MPI_File" ) == 0    ||
            strcmp( token, "MPI_Win"  ) == 0    || 
	    strcmp( token, "MPI_Datatype" ) == 0) {
	    type->is_mpi = 1;
	    /* type->has_star      = 1; */
	    type->type_has_star = 1;
	    type->implied_star  = 1;
	    }
	if (strcmp( token, "MPI_Aint" ) == 0) {
	    /* For most systems, MPI_Aint is just long */
	    /* type->has_star      = 0; */
	    type->type_has_star = 0;
	    type->implied_star  = 0;
	    type->is_native     = 1;
	    }
	}
    if (strcmp( token, "void"   ) == 0) {
	/* Activate set_void only for the files specified by flag2 */
	if (!flag2) type->is_native = 1;
	else type->is_void = 1;    
	}
    }
DBG2("Found type %s\n",token)
if (AnsiForm && useFerr && strcmp( token, "void") == 0) {
    /* Special case for (void) when we replace with an argument */
    while ( (c = SYTxtGetChar( fin )) != EOF && isspace(c)) ;
    ungetc( c, fin );
    if (c == ')') return 0;
    }
if (type->is_mpi && isMPI2) {
    OutputToken( fout, "MPI_Fint *", nsp );
}
else {
    OutputToken( fout, token, nsp );
}
return 0;
}

/* 
   Read an argument.  If it is not a pointer type, add the "*" to the
   definition.
 */
int GetArgName( fin, fout, arg, type, has_extra_chars )
FILE     *fin, *fout;
ARG_LIST *arg;
TYPE_LIST *type;
int has_extra_chars;
{
    int c, nsp, nsp1, nparen, nbrack, nstar;
    char *p, token[1024];

    DBG("Looking for arg...\n")
/* This should really use a better parser. 
   Names are
   [ *( ]* varname [(*\[]* 
   separated by , (ANSI) or ; (K&R), and terminated by
   ')' (ANSI) or '{' (K&R)

   A modification is that in K&R form, after a ',', the 
   [register] [struct] typename is carried forward until a ';'

   The known typenames (and optional [register][const][struct] are 
   processed by GetTypeName;

   If the type is NOT a pointer, we must put a '*' in front of it (since
   Fortran always passes pointers).
   */
    nparen = 0;
    nbrack = 0;
    nstar  = 0;
/* Many of these fields are set from the base type */
    arg->has_star	   = 0;
    arg->is_char	   = 0;
    arg->is_native	   = 0;
    arg->has_array	   = 0;
    arg->is_FILE	   = 0;
    arg->void_function = 0;
    arg->implied_star  = 0;
    arg->name	   = 0;

    p = token;
    c = FindNextANToken( fin, p, &nsp );
    while (c != EOF && c == '\n') {
	OutputToken( fout, token, nsp );
	c = FindNextANToken( fin, token, &nsp );
    }
    if (c == ')') {
	/* No argument to get (while reading function declaration) 
	   (may be (void) or () in ANSI) */
	if (useFerr) {
	    if (AnsiForm) {
		fprintf( fout, "int *__ierr " );
	    }
	    else {
		fprintf( fout, "__ierr " );
	    }
	}
	OutputToken( fout, token, nsp );
	return 1;
    }
/* We don't want to output the token when we reach the name incase
   we need to generate a "*" for it */
    while (c != EOF) {
	if (c == '(') 
	    nparen++;
	else if (c == '*')
	    nstar++;
	else if (c == ')')
	    nparen--;
	else
	    break;
	OutputToken( fout, p, nsp );
	c = FindNextANToken( fin, p, &nsp );
    }

    /* Current token is name */
    arg->has_star = (nstar > 0);
    arg->name     = (char *)MALLOC( strlen(p) + 1 );
    strcpy( arg->name, p );

    /* We can't output the name just yet, because if it is
       int foo[], we don't want to generate int *foo[].  As a short cut,
       we could eliminate the first [] if we've already output a * 
     */
/* if (type && (type->is_native && !type->is_char && !arg->has_star)) { */
/* 
   We use "type" to determine if we should generate any special "*"s,
   for example, in the declaration. 
 */

/* Read rest of definition if necessary.  May need to read () or [] */
/* In ANSI form, it needs to skip (....) where this is part of a 
   function declaration (ie., void (f)(int,char)).  Note that in this
   case, we need to peek at the next character to see if it is a (.
   Eventually, we'll want to use this information to generate automatic
   stubs for the Fortran versions of the routines.

   Note that the next token should be a single character except when scanning
   tokens that we want to process (that is, any token to be pushed back 
   should be a single character).
   */
    /* Peak at the next character */
    c = FindNextANToken( fin, p, &nsp1 );

    if (type && !arg->has_star && !type->implied_star && !isMPI2 &&
	!type->is_mpi && c != '[') {
	if (nsp == 0) nsp++;
	OutputToken( fout, "*", nsp );
	nsp = 0;
    }
    if (nstar > 0) {
	if (!NoFortMsgs && /* !is_function &&  */
	    nstar > type->implied_star + 1) {
	    fprintf( stderr, 
		     "%s(%s) has multiple indirection for %s\n",
		     "routine"/*name*/, CurrentFilename, arg->name );
	}
	if (!MultipleIndirectsAreNative) arg->is_native = 0;
    }

    /* Here we output the variable name, which has been saved in arg->name */
    OutputToken( fout, arg->name, nsp );

    /* Now, get the value from the peak above */
    nsp = nsp1;
    while (has_extra_chars && (c == '(' || (nparen > 0 && c == ')') || c == '[')) {
	OutputToken( fout, p, nsp );
	if (c == '(') { OutputBalancedString( fin, fout, '(', ')' );
	arg->void_function = 1; }
	else if (c == ')') nparen--;
	else if (c == '[') { OutputBalancedString( fin, fout, '[', ']' );
	arg->has_array = 1; arg->has_star++; }
	else if (c == ']') nbrack--;
	else if (c == '*') ;
	else
	    break;
	c = FindNextANToken( fin, p, &nsp );
    }
    DBG2("Found arg %s\n",arg->name)
/* Really need to unget entire token */
	if (strlen(p) > 1) {
	    ErrCnt++;
	    fprintf( stderr, "Unexpected token (%s) while reading argument name\n",p);
	    if (ErrCnt > MAX_ERR) exit(1);
	}
    ungetc( (char)c, fin );
    return 0;
}

/* Output a balanced string.  The first character (cs) has been read */
void OutputBalancedString( fin, fout, cs, ce )
FILE *fin, *fout;
int  cs, ce;
{
    int c, bcount;
    bcount = 1;
    while (bcount) {
	c = getc( fin );
	if (c == cs) bcount++;
	else if (c == ce) bcount--;
	putc( (char)c, fout );
    }
}

void DoBfortHelp( pgm )
char *pgm;
{
fprintf( stderr, "%s - write a Fortran interface to C routines with\n", 
	 pgm );
fprintf( stderr, "routines documented in the `doctext' format\n" );
fprintf( stderr, "Optional arguments:\n" );
fprintf( stderr, "\
filenames - Names the files from which lint definitions are to be extracted\n\
-nomsgs   - Do not generate messages for routines that can not be converted\n\
            to Fortran.\n\
-nofort   - Generate messages for all routines/macros without a Fortran\n\
            counterpart.\n\
-dir name - Directory for output file\n\
-I name   - file that contains common includes\n\
-mapptr   - translate pointers to integer indices\n\
            The macro used to determine whether pointers are 64 bits can be\n\
            changed with\n\
\t-ptr64 name\tReplace POINTER_64_BITS\n\
-ptrprefix prefix - Prepend this name to the routines to map pointers\n" );
fprintf( stderr, "\
-anyname  - generate Fortran names for a variety of systems\n\
            The macros used to select the form can be set with\n\
\t-fcaps name\tReplace FORTRANCAPS\n\
\t-fuscore name\tReplace FORTRANUNDERSCORE\n\
\t-fduscore name\tReplace FORTRANDOUBLEUNDERSCORE\n\
-ferr     - Fortran versions return the value of the routine as the last\n\
            argument (an integer).  This is used in MPI and is a not\n\
            uncommon approach for handling error returns.\n\
-mpi      - Handle MPI datatypes (some things are pointers by definition)\n\
            The macro used to determine whether the MPI profiling version\n\
            is being built can be changed with\n\
\t-pmpi name\tReplace MPI_BUILD_PROFILING\n\
-mnative  - Multiple indirects are native datatypes (no coercion)\n\
-voidisptr - Consider \"void *\" as a pointer to a structure.\n\
-ansi      - Input files use ANSI-C prototype form instead of K&R\n\
-ansiheader - Generate ANSI-C style headers instead of Fortran interfaces\n\
This will be useful for creating ANSI prototypes without ANSI-fying the\n\
code.  The output is in <filename>.ansi .\n\
" );
}
