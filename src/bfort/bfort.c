/* -*- Mode: C; c-basic-offset:4 ; -*- */

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

static int NoFortMsgs = 1;
/* NoFortWarnings turns off messages about things not being available in
   Fortran */
static int NoFortWarnings = 1;

/* when converting C type to Fortran for F90 interfaces keep any unknown ones */
static int useUserTypes = 1;

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
/* Defaults for these are "ierr" and "__ierr" */
static const char *errArgNameParm = 0;
static const char *errArgNameLocal = 0;

/* Handle character string arguments
   Note that there is no standard for how Fortran passing variables of
   type character*(*).  However, many Fortran implementations implement
   strings by passing a pointer to the character storage at the location
   of the argument in the argument list, and append a value int with
   the length at the end of the argument list.

   If stringStyle is STRING_LENEND, add an argument with the length of
   the string at the end of the argument list with the length of the
   string, as passed by Fortran.  This is the most common approach by
   Fortran compilers.

   Note that in addition to this, it may be necessary to copy the data
   either into or out of the Fortran storage (MPICH does this with its
   own Fortran interface generator).

   The default for stringArgLenName is cl, and may be overridden in the
   config file.
 */
static const char *stringArgLenName = 0;
static enum { STRING_UNKNOWN, STRING_ABORT, STRING_LENEND }
    stringStyle = STRING_UNKNOWN;

/* Enable the MPI definitions and conversion functions */
static int isMPI = 0;

/* Enable profiling name output */
static int DoProfileNames = 0;

/* If 0, do not generate the ifndef DEBUG_ALL wrapper */
static int AddDebugAll = 1;

/* If 0, do not generate Fortran 9x interface module */
static int F90Module = 0;
static FILE *fmodout = 0;
static const char *f90headerName = "f90header";

/* if 0, use original argument name. If 1, use a single character name
   instead*/
static int useShortNames = 0;

/* Catch serious problems */
static int MAX_ERR = 100;
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

/* Lists of type names */
static void *nativeList;
static void *nativePtrList;
static void *toptrList;
static void *parmList;
#define NUM_CONFIG_CMDS 6
static SYConfigCmds configcmds[NUM_CONFIG_CMDS+1];

/* We also need to make some edits to the types occasionally.  First, note
   that double indirections are often bugs */
#define MAX_TYPE_NAME 60    /* Maximum length of a type name */
#define MAX_ARGS      128   /* Maximum number of args to a function */
#define MAX_TYPES      64   /* Maximum number of types to a function */

#ifndef MAX_PATH_NAME
#define MAX_PATH_NAME 1024
#endif

typedef struct {
    char *name;
    int  is_const, has_star, is_char, is_native, has_array,
	type,              /* Index into TYPE_LIST array */
	is_FILE, void_function;
    int  implied_star;
} ARG_LIST;
typedef struct {
    int is_const, is_char, is_native, implied_star, is_FILE, type_has_star,
	is_void;
    int is_mpi;
    char type[MAX_TYPE_NAME];
} TYPE_LIST;
typedef struct {
    char name[MAX_TYPE_NAME];
    int  num_stars;
} RETURN_TYPE;

/* Forward defs */
void OutputToken ( FILE *, const char *, int );
void OutputRoutine ( FILE *, FILE *, char *, char *, char );
void OutputFortranToken( FILE *, int, const char *);
void SkipText ( FILE *, char *, char *, char );
int SkipToSynopsis ( FILE *, char );
int FindNextANToken ( FILE *, char *, int * );
void OutputBuf ( FILE **, const char *, const char *, FILE *, const char * );
void OutputMacro ( FILE *, FILE *, char *, char * );
void ProcessFunctionType ( FILE *, FILE *, char *, int *,
				    char *, RETURN_TYPE *, int );
void ProcessArgList ( FILE *, FILE *, char *, int *, char *,
				ARG_LIST[MAX_ARGS], int *, RETURN_TYPE *,
				int, TYPE_LIST *, int *, int );
void PrintBody ( FILE *, int, char *, int, int, ARG_LIST *,
			   TYPE_LIST *, RETURN_TYPE * );
void PrintDefinition ( FILE *, int, char *, int, int, ARG_LIST *,
			   TYPE_LIST *, RETURN_TYPE * );
int NameHasUnderscore ( char * );
void OutputRoutineName ( char *, FILE * );
void OutputUniversalName ( FILE *, char * );
int GetTypeName ( FILE *, FILE *, TYPE_LIST *, int, int, int );
int GetArgName ( FILE *, FILE *, ARG_LIST *, TYPE_LIST *, int );
void FixupArgNames( int, ARG_LIST * );
void OutputBalancedString ( FILE *, FILE *, int, int );
char *ToCPointer ( char *, char *, int );
const char *ArgToFortran( const char *typeName );
void FreeArgs( ARG_LIST *, int );
int MPIU_Strncpy( char *, const char *, size_t );
int MPIU_Strnapp( char *, const char *, size_t );
void Abort( const char *, const char *, int );
void DoBfortHelp ( char * );

int stringDecl(FILE *fout, int nargs, ARG_LIST args[]);
int stringFtoC(FILE *fout, int nargs, ARG_LIST args[]);
int stringCtoF(FILE *fout, int nargs, ARG_LIST args[]);

#define ABORT(msg) Abort(msg,__FILE__,__LINE__)

/*D
  bfort - program to extract short definitions for a Fortran to C interface

  Input:
+ filenames - Names the files from which lint definitions are to be extracted
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
. -shortargname - Use short (single character) argument names instead of the
              name in the C definition.
. -fstring  - Enable handling of Fortran character parameters, using the most
              common call interface.  This is the default unless the
	      environment variable BFORT_STRINGHANDLING is set to IGNORE.
. -fnostring - Disable handling of Fortran character parameters.  Routines
               with 'char' arguments in C will not have interface routines
               generated for them.
. -fstring-asis - No special processing for string arguments.  Provided
               for backward compatiblity to older versions of bfort that
               did not handle strings
. -mpi      - Handle MPI types (some things are pointers by definition)
. -no_pmpi  - Do not generate PMPI names
. -pmpi name - Change macro used to select MPI profiling version
. -noprofile - Turn off the generation of the profiling version
. -mnative  - Multiple indirects are native datatypes (no coercion)
. -voidisptr - Consider "void *" as a pointer to a structure.
. -nodebug  - Do not add
.vb
  #ifndef DEBUG_ALL
  #define DEBUG_ALL
  #endif
.ve
  to the wrapper file.
- -anyname   - Generate a single wrapper that can handle the three most common
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
int main( int argc, char **argv )
{
    char routine[MAX_ROUTINE_NAME];
    char *infilename;
    char outfilename[MAX_PATH_NAME];
    char defoutfilename[MAX_PATH_NAME];
    char dirname[MAX_PATH_NAME];
    char fname[MAX_PATH_NAME], *p;
    FILE *fd, *fout, *incfd;
    char kind;
    char incfile[MAX_FILE_SIZE];
    char incbuffer[MAX_PATH_NAME];
    int  n_in_file;
    int  f90mod_skip_header = 1;


    /* Initialize setup for config files */
    SYConfigDBInit("native", &nativeList);
    SYConfigDBInit("nativeptr", &nativePtrList);
    SYConfigDBInit("toptr", &toptrList);
    SYConfigDBInit("parm", &parmList);
    configcmds[0].name     = "native";
    configcmds[0].docmd    = SYConfigDBInsert;
    configcmds[0].cmdextra = nativeList;
    configcmds[1].name     = "nativeptr";
    configcmds[1].docmd    = SYConfigDBInsert;
    configcmds[1].cmdextra = nativePtrList;
    configcmds[2].name     = "toptr";
    configcmds[2].docmd    = SYConfigDBInsert;
    configcmds[2].cmdextra = toptrList;
    configcmds[3].name     = "parm";
    configcmds[3].docmd    = SYConfigDBInsert;
    configcmds[3].cmdextra = parmList;
    /* These next two are used by bfort2 but ignore by bfort */
    configcmds[4].name     = "fromptr";
    configcmds[4].docmd    = SYConfigDBIgnore;
    configcmds[4].cmdextra = 0;
    configcmds[5].name     = "declptr";
    configcmds[5].docmd    = SYConfigDBIgnore;
    configcmds[5].cmdextra = 0;

    configcmds[NUM_CONFIG_CMDS].name     = 0;

    /* Some more defaults */
    defoutfilename[0] = 0;

    /* Special handling of defaults.
       The default handling of strings changed in this version of bfort
       from version 1.1.25.  The environment variable BFORT_STRINGHANDLING
       can be set to IGNORE to use the old (oblivious) approach.
    */
    {
	char *ep;
	ep = getenv("BFORT_STRINGHANDLING");
	if (ep && (strcmp(ep, "IGNORE") == 0 || strcmp(ep, "ignore") == 0))
	    stringStyle = STRING_UNKNOWN;
    }

    /* process all of the files */
    /* Set a default for the directory name */
    if (MPIU_Strncpy( dirname, ".", sizeof(dirname) )) {
	ABORT( "Unable to set dirname to \".\"" );
    }
    incfile[0]  = 0;
    SYArgGetString( &argc, argv, 1, "-dir", dirname, MAX_PATH_NAME );
    SYArgGetString( &argc, argv, 1, "-I",   incfile, MAX_PATH_NAME );
    SYArgGetString( &argc, argv, 1, "-o", defoutfilename, MAX_PATH_NAME);

    NoFortMsgs		   = SYArgHasName( &argc, argv, 1, "-nomsgs" );
    MapPointers		   = SYArgHasName( &argc, argv, 1, "-mapptr" );
    if (MapPointers) {
	SYArgGetString( &argc, argv, 1, "-ptrprefix", ptrprefix, 32 );
    }
    useFerr		   = SYArgHasName( &argc, argv, 1, "-ferr" );
    isMPI		   = SYArgHasName( &argc, argv, 1, "-mpi" );
    /* For backward compatibility, allow -mpi2 */
    if (SYArgHasName( &argc, argv, 1, "-mpi2" )) isMPI = 1;
    if (isMPI) DoProfileNames = 1;
    if (SYArgHasName( &argc, argv, 1, "-no_pmpi" ))
	DoProfileNames = 0;
    TranslateVoidStar	   = SYArgHasName( &argc, argv, 1, "-voidisptr" );
    MultipleIndirectsAreNative = SYArgHasName( &argc, argv, 1, "-mnative" );

    (void) SYArgHasName( &argc, argv, 1, "-ansi" );
    AddDebugAll                = SYArgHasName( &argc, argv, 1, "-nodebug" );
    IfdefFortranName           = SYArgHasName( &argc, argv, 1, "-anyname" );
    useShortNames              = SYArgHasName( &argc, argv, 1, "-shortargname");

    /* No longer support -noansi or -ansiheader */
    if (SYArgHasName(&argc, argv, 1, "-ansiheader")) {
	ABORT("-ansiheader no longer supported\n");
    }
    if (SYArgHasName(&argc, argv, 1, "-noansi")) {
	ABORT("-noansi no longer supported\n");
    }

    /* Get replacement names for ifdef items in generated code */
    SYArgGetString( &argc, argv, 1, "-fcaps", FortranCaps, 256 );
    SYArgGetString( &argc, argv, 1, "-fuscore", FortranUscore, 256 );
    SYArgGetString( &argc, argv, 1, "-fduscore", FortranDblUscore, 256 );
    SYArgGetString( &argc, argv, 1, "-ptr64", Pointer64Bits, 256 );
    SYArgGetString( &argc, argv, 1, "-pmpi", BuildProfiling, 256 );
    if (SYArgHasName( &argc, argv, 1, "-noprofile" )) DoProfileNames = 0;

    if (SYArgHasName( &argc, argv, 1, "-on_error_abort" )) MAX_ERR = 0;

    /* String support */
    if (SYArgHasName(&argc, argv, 1, "-fstring"))   stringStyle = STRING_LENEND;
    if (SYArgHasName(&argc, argv, 1, "-fnostring")) stringStyle = STRING_ABORT;
    if (SYArgHasName(&argc, argv, 1, "-fstring-asis"))
	stringStyle = STRING_UNKNOWN;

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

    /* Read the basics, such as predefined C types */
    if (SYGetFileFromPathEnv(BASEPATH, "BFORT_CONFIG_PATH", NULL,
			     "bfort-base.txt", fname, 'r')) {
	if (!SYReadConfigFile(fname, ' ', '#', configcmds, NUM_CONFIG_CMDS)) {
	    fprintf(stderr, "Unable to read configure file bfort-base.txt");
	    exit(1);
	}
    }
    else {
	fprintf(stderr, "Unable to find bfort-base.txt config file in %s\n",
		BASEPATH);
	exit(1);
    }
    /* Based on options such as isMPI, load the appropriate
       config files */
    if (isMPI) {
	if (SYGetFileFromPathEnv(BASEPATH, "BFORT_CONFIG_PATH", NULL,
				 "bfort-mpi.txt", fname, 'r')) {
	    if (!SYReadConfigFile(fname, ' ', '#',
				  configcmds, NUM_CONFIG_CMDS)) {
		fprintf(stderr, "Unable to read configure file bfort-mpi.txt");
		exit(1);
	    }
	}
	else {
	    fprintf(stderr, "Unable to read MPI config file bfort-mpi.txt in %s\n",
		    BASEPATH);
	    exit(1);
	}
    }

    /* Allow the user to override the variable name used for the error
       parameter. */
    if (useFerr) {
	/* -shortargname overrides config file for the err arg name */
	if ((! errArgNameParm) && useShortNames)
	    errArgNameParm = "z";
	if (!errArgNameParm) {
	    if (SYConfigDBLookup("parm", "errparm",
				 &errArgNameParm, parmList) != 1) {
		errArgNameParm = "ierr";
	    }
	}
	if (!errArgNameLocal) {
	    if (SYConfigDBLookup("parm", "errparmlocal",
				 &errArgNameLocal, parmList) != 1) {
		errArgNameLocal = "__ierr";
	    }
	}
    }

    /* Get the name to use for the length of the string argument */
    if (SYConfigDBLookup("parm", "stringargname",
			 &stringArgLenName, parmList) != 1) {
	stringArgLenName = "cl";
    }

    /* Open up the file of public includes */
    if (incfile[0]) {
	incfd = fopen( incfile, "r" );
	if (!incfd) {
	    ErrCnt++;
	    fprintf( stderr, "Could not open file %s for -I option\n", incfile );
	    if (ErrCnt > MAX_ERR) ABORT( "" );
	}
    }
    else
	incfd = 0;

    /* See if we should create the F90 Module file. */
    f90mod_skip_header = SYArgHasName( &argc, argv, 1, "-f90mod_skip_header" );
    if (!f90mod_skip_header) {
	/* Check for the more appropriate spelling */
	f90mod_skip_header =
	    SYArgHasName( &argc, argv, 1, "-f90mod-skip-header" );
    }
    /* If an f90modfile argument is provided, then enable the f90module */
    if (SYArgHasName( &argc, argv, 0, "-f90modfile" )) {
	F90Module = 1;
    }

    /* If there is a f90 module file, open it now */
    if (F90Module) {
	char fmodfile[MAX_FILE_SIZE];
	if (!SYArgGetString( &argc, argv, 1, "-f90modfile",
			     fmodfile, MAX_FILE_SIZE )) {
	    if (MPIU_Strncpy( fmodfile, "f90module.f90", sizeof(fmodfile) )) {
		ABORT( "Unable to set the name of the Fortran 90 module file" );
	    }
	}

	fmodout = fopen( fmodfile, "w" );
	if (!fmodout) {
	    ErrCnt++;
	    fprintf( stderr, "Could not open file %s for Fortran 90 interface output\n", fmodfile );
	    if (ErrCnt > MAX_ERR) ABORT( "" );
	    F90Module = 0;
	}
	else {
	  if (!f90mod_skip_header) {
	      OutputFortranToken( fmodout, 0, "module" );
	      OutputFortranToken( fmodout, 1, f90headerName );
	      OutputFortranToken( fmodout, 0,"\n" );
	      OutputFortranToken( fmodout, 0, "interface" );
	      OutputFortranToken( fmodout, 0, "\n" );
          }
	}
    }

    argc--; argv++;
    while (argc--) {
	/* Input filename */
	infilename = *argv++;
	fd = fopen( infilename, "r" );
	if (!fd) {
	    ErrCnt++;
	    fprintf( stderr, "Could not open file %s\n", infilename );
	    if (ErrCnt > MAX_ERR) ABORT( "" );
	    continue;
        }
	n_in_file = 0;
	/* Remember file name */
	CurrentFilename = infilename;

	if (defoutfilename[0] == 0) {
	    /* Set the output filename */
	    SYGetRelativePath( infilename, fname, MAX_PATH_NAME );
	    /* Strip the trailer */
	    p = fname + strlen(fname) - 1;
	    while (p > fname && *p != '.') p--;
	    *p = 0;
	    /* Add an extra h to include files */
	    if (p[1] == 'h') {
		p[0] = 'h';
		p[1] = 0;
	    }
	    snprintf( outfilename, MAX_PATH_NAME, "%s/%sf.c", dirname, fname );
	}
	else {
	    snprintf( outfilename, MAX_PATH_NAME, "%s/%s",
		      dirname, defoutfilename);
	}

	/* Don't open the filename yet (wait until we know that we'll have
	   some output for it) */
	fout = NULL;

	/* Pass 1: Generate the name mappings.
	   Eventually, we could store up the routine names and generate a
	   single, simpler block of definitions.
	   */
	while (FoundLeader( fd, routine, &kind )) {
	    /* Check for a valid leader first */
	    int cerr;
	    char subclass = GetSubClass(&cerr);
	    if (cerr != 0) {
		fprintf(stderr, "Invalid structured comment.  Did you forget to leave a blank after /* in %s:%d?  Unexpected character is \"%c\"\n",
			infilename, GetLineNo(), subclass);
		continue;
	    }
	    if (!fout) {
		OutputBuf( &fout, infilename, outfilename, incfd, (char*)0 );
	    }
	    if (IfdefFortranName && fout && routine[0] &&
		(kind == ROUTINE || kind == MACRO)) {
		if (subclass != 'C')
		    OutputUniversalName( fout, routine );
		SkipText( fd, routine, infilename, kind );
	    }
	    else if (kind == INCLUDE) {
		int guard_x = 0;
		if (MPIU_Strncpy( incbuffer, "#include ", sizeof(incbuffer) )) {
		    ABORT( "Unable to set the name of the include buffer" );
		}
		/* Grumble.  We'll have to fix this eventually */
		if (routine[0] != '"' && routine[0] != '<') {
		    p = routine + strlen(routine) - 1;
		    if (*p == '>') {
			if (MPIU_Strnapp( incbuffer, "<", sizeof(incbuffer) )){
			    ABORT( "Cannot add < to include buffer" );
			}
		    }
		    else if (*p == '"') {
			if (MPIU_Strnapp( incbuffer, p, sizeof(incbuffer) ) ){
			    ABORT( "Cannot append file name to include buffer");
			}
		    }
		}
		/* Special case */
		/* fprintf( stderr, "include == |%s|\n", routine ); */
		if (strncmp( routine, "xtools/", 7 ) == 0) {
		    guard_x = 1;
		    OutputBuf( &fout, infilename, outfilename, incfd,
			       "#ifndef TOOLSNOX11\n" );
		}
		if (MPIU_Strnapp( incbuffer, routine, sizeof(incbuffer) )) {
		    ABORT( "Cannot add routine name to include buffer" );
		}
		CopyIncludeName( fd, incbuffer + strlen(incbuffer) );
		if (MPIU_Strnapp( incbuffer, "\n", sizeof(incbuffer) )) {
		    ABORT( "Cannot add newline to include buffer" );
		}
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
	ResetLineNo();

	/* if generating code for Fortran character arguments, add stdlib */
	if (stringStyle == STRING_LENEND && fout) {
	    fprintf(fout, "/* Provide declarations for malloc/free if needed for strings */\n#include <stdlib.h>\n");
	}
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
	    /* Check for a valid leader first */
	    int cerr;
	    char subclass = GetSubClass(&cerr);
	    if (cerr != 0) {
		fprintf(stderr, "Invalid structured comment.  Did you forget to leave a blank after /* in %s:%d?  Unexpected character is \"%c\"\n",
			infilename, GetLineNo(), subclass);
		continue;
	    }
	    /* We need this test first to avoid creating an empty file,
	       particularly for initf.c */
	    if ((kind == ROUTINE || kind == MACRO) && subclass == 'C') {
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
	    /* FIXME: For a default output file, need to more this eof
	       handling to end - along with not duplicating bof handling */
	    fclose(fout);
	    if (n_in_file == 0) {
		/* If all we put into the interface file was an include, we delete
		   it */
		unlink( outfilename );
	    }
	}
    }
    if (F90Module && fmodout) {
	if (!f90mod_skip_header) {
	    OutputFortranToken( fmodout, 0, "end interface" );
	    OutputFortranToken( fmodout, 0, "\n" );
	    OutputFortranToken( fmodout, 0, "end module" );
	    OutputFortranToken( fmodout, 1, f90headerName );
	    OutputFortranToken( fmodout, 0, "\n" );
        }
	fclose( fmodout );
	fmodout = 0;
    }

    return 0;
}

/*
 * Output routines
 */
void OutputToken( FILE *fout, const char *p, int nsp )
{
    int i;
    static int outcnt = 0;

    for (i=0; i<nsp; i++) putc( ' ', fout );
    fputs( p, fout );
    if (Debug) {
	outcnt += nsp + (int)strlen(p);
	if (outcnt > 10000) {
	    ABORT( "Exceeded output count limit!" );
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
    int         nargs, nstrings = 0;
    int         ntypes;
    int         flag2 = 0, cerr;

    /* Check to see if this is a C-only routine */
    if (GetSubClass(&cerr) == 'C' && cerr == 0) {
	if (!NoFortMsgs && !NoFortWarnings) {
	    fprintf( stderr, "Routine %s(%s) cannot be translated into Fortran\n",
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

    /* Check for the case that there are strings in argument list and
       strings are not handled */
    if (stringStyle == STRING_ABORT) {
	int i;
	for (i=0; i<nargs; i++) {
	    if (args[i].is_char) {
		printf("Routine %s(%s) contains a char argument and cannot be translated into Fortran\n",
		       name, CurrentFilename);
		SkipText(fin, name, filename, kind);
		/* At this point, we've output the routine argument lists
		   (in ProcessArgList) but not the body. We add an
		   empty body here. */
		fputs("{}\n", fout);
		return;
	    }
	}
    }

    PrintBody( fout, is_function, name, nstrings, nargs, args, types, &rt );
    if (F90Module) {
	PrintDefinition( fmodout, is_function, name, nstrings, nargs,
			 args, types, &rt );
    }
    /* Free the created space */
    FreeArgs( args, nargs );
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
int FindNextANToken( FILE *fd, char *token, int *nsp )
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

void OutputBuf( FILE **fout, const char *infilename, const char *outfilename,
		FILE *incfd, const char *buffer )
{
    char arch[20];

    if (!*fout) {
	*fout = fopen( outfilename, "w" );
	if (!*fout) {
	    ErrCnt++;
	    fprintf( stderr, "Could not open file %s\n", outfilename );
	    if (ErrCnt > MAX_ERR) ABORT( "" );
	    return;
	}
	fprintf( *fout, "/* %s */\n", infilename );
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
		fprintf( *fout,
			 "extern void *%sToPointer(); extern int %sFromPointer();\n",
			 ptrprefix, ptrprefix );
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
void OutputMacro( FILE *fin, FILE *fout, char *routine_name, char *filename )
{
    int         is_function;
    ARG_LIST    args[MAX_ARGS];
    TYPE_LIST   types[MAX_TYPES];
    RETURN_TYPE rt;
    int         nargs, nstrings = 0;
    int         ntypes;
    int         has_synopsis;
    int         done;
    int         flag2 = 0, cerr;

/* Check to see if this is a C-only macro */
    if (GetSubClass(&cerr) == 'C' && cerr == 0) {
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
	done = 1;
	PrintBody( fout, is_function, routine_name, nstrings, nargs, args,
		   types, &rt );
    }
    else {
	ErrCnt++;
	fprintf( stderr, "%s(%s) has no synopsis section\n",
		 routine_name, CurrentFilename );
	if (ErrCnt > MAX_ERR) ABORT( "" );
    }
/* finish up the section */
    if (!done)
	SkipText( fin, routine_name, filename, MACRO );
}

/* Read the arg list and function type */

/*
   This routine reads the function type and name; that is, it processes
   things like "void *foo" and "void (*foo)()"
 */
void ProcessFunctionType( FILE *fin, FILE *fout, char *filename,
			  int *is_function, char *name, RETURN_TYPE *rt,
			  int flag )
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

   We also want to defer generating the function type in case we need to
   replace a pointer ref with an integer.
   */
    if (MPIU_Strncpy( rt->name, p, sizeof(rt->name) )) {
	ABORT( "Cannot copy return type name" );
    }
    rt->num_stars = 0;
    *is_function          = strcmp( p, "void" );

    for (i=0; i<nsp; i++) putc( ' ', fout );

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
	    if (ErrCnt > MAX_ERR) ABORT( "" );
	    return;
	}
	if (nsp > 0) {
	    if (MPIU_Strnapp( rt->name, " ", sizeof(rt->name) )) {
		ABORT( "Cannot add space to return type name" );
	    }
	}
	if (strcmp( p, name ) != 0 && p[0] != '(') {
	    if (MPIU_Strnapp( rt->name, p, sizeof(rt->name) )) {
		ABORT( "Cannot append name to return type" );
	    }
	}
	if (c == '*') {
	    *is_function = 1;
	    rt->num_stars++;
	}
	if (flag && c == '\n' && leadingm == 0) {
	    fputs( "())", fout );
	    break;
	}
	if (c == '\n') leadingm = 1;
	if (c == '(') {
	    if (in_args == 0) {
		/* Output function type and name */
		if (rt->num_stars == 0 || !MapPointers) {
		    if (useFerr && strncmp( rt->name, "int", 3 ) == 0 ) {
			fputs( "void ", fout );
		    }
		    else {
			fputs( rt->name, fout );
			fputs( " ", fout );
		    }
		}
		else {
		    fputs( "int ", fout );
		}
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
	if (MPIU_Strncpy( actname, p, sizeof(actname) )) {
	    ABORT( "Cannot copy actual name" );
	}
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
	       if (ErrCnt > MAX_ERR) ABORT( "" );
	       }
	       }
	       */
	}
    }
    if (!found_name) {
	ErrCnt++;
	fprintf( stderr, "%s:Did not find routine name (may be untyped): %s \n",
		 filename, name );
	if (ErrCnt > MAX_ERR) ABORT( "" );
    }
    else if (strcmp( name, actname ) != 0) {
	ErrCnt++;
	fprintf( stderr, "%s:Did not find matching name: %s != %s\n",
		 filename, actname, name );
	if (ErrCnt > MAX_ERR) ABORT( "" );
    }
}

/* We are moving to being able to suppress generating the output until the
   argument definitions are read.
   flag is 1 for C routines, 0 for macros */
void ProcessArgList( FILE *fin, FILE *fout, char *filename, int *is_function,
		     char *name, ARG_LIST args[MAX_ARGS], int *Nargs,
		     RETURN_TYPE *rt, int flag, TYPE_LIST *types, int *Ntypes,
		     int flag2 )
{
    int             c, ntypes;
    char            *p;
    int             nsp, leadingm;
    static char     rcall[1024];
    int             nargs, in_args;
    TYPE_LIST       *curtype;
    int             outparen;
    int             nstrings = 0;

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
	if (ErrCnt > MAX_ERR) ABORT( "" );
	return;
    }
    OutputToken( fout, p, nsp );
    while (1) {
	/* First, get the type name.  Note that there might not be one */
	curtype = &types[ntypes];
	outparen = ntypes > 0;
	if ((c = GetTypeName( fin, fout, &types[ntypes], flag, flag2,
			      outparen ))) {
	    if (ntypes == 0 && c == ')') {
		fprintf(stderr,
			 "Empty argument list in -ansi mode (use (void))\n");
		/* For this to work, gettypename can't output the last
		   closing paren */
		if ( useFerr ) {
		    fprintf( fout, "int *%s)\n", errArgNameLocal);
		}
		else {
		    fputs(")\n", fout);
		}
	    }
	    else if (c != 1) {
		char cstring[2];
		cstring[0] = c; cstring[1] = 0;
		OutputToken( fout, cstring, 0 );
	    }
	    break;
	}
	ntypes++;

	/* Now, get the variable names until the arg terminator.
	   They are of the form [(\*]*name[(\*\[]*
	   */
	if (GetArgName( fin, fout, &args[nargs], curtype, 1 )) {
	    break;
	}
	args[nargs].type = ntypes-1;
	if (curtype) {
	    /* Propagate information about the type to the specific argument */
	    if (curtype->type_has_star)
		args[nargs].has_star++;
	    if (curtype->implied_star)
		args[nargs].implied_star++;
	    args[nargs].is_native = curtype->is_native;
	    args[nargs].is_char   = curtype->is_char;
	    args[nargs].is_const  = curtype->is_const;
	    args[nargs].is_FILE   = curtype->is_FILE;
	    if (args[nargs].is_char) {
		DBG("Found a string (is_char)\n");
		nstrings++;
	    }
	}
	if (nargs >= MAX_ARGS) {
	    ErrCnt++;
	    fprintf( stderr, "Too many arguments to function %s; only %d supported and function has %d\n",
		     name, MAX_ARGS, nargs );
	    ABORT( "Aborting: Too many arguments to function" );
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
	    if (useFerr) {
		fprintf( fout, "%sint *%s",
			 (nargs > 0) ? ", " : "", errArgNameLocal );
	    }
	    /* Add string length arguments */
	    if (nstrings && (stringStyle == STRING_LENEND ||
			     stringStyle == STRING_UNKNOWN)) {
		int i;
		for (i=0; i<nstrings; i++) {
		    fprintf(fout, ", int %s%d", stringArgLenName, i);
		}
	    }
	    fputs( ")\n", fout );
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

    /* Handle declaration of form int foo(void) */
    if (ntypes == 1 && nargs == 0 && strcmp("void",types[0].type) == 0) {
	ntypes = 0;
    }
    *Nargs  = nargs;
    *Ntypes = ntypes;
}

#ifdef FOO
/* This is a replacement version for ProcessArgList that only reads
   the args and creates the necessary data structures.  Output is
   handled separately (Question: Can we do it just from the data structures?)
*/
void ProcessArgList( FILE *fin, FILE *fout, char *filename, int *is_function,
		     char *name, ARG_LIST args[MAX_ARGS], int *Nargs,
		     RETURN_TYPE *rt, int flag, TYPE_LIST *types, int *Ntypes,
		     int flag2 )
{
    int             c, ntypes;
    char            *p;
    int             nsp, leadingm;
    static char     rcall[1024];
    int             nargs, in_args;
    TYPE_LIST       *curtype;
    int             outparen;
    int             nstrings = 0;

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
	if (ErrCnt > MAX_ERR) ABORT( "" );
	return;
    }
    OutputToken( fout, p, nsp );
    while (1) {
	/* First, get the type name.  Note that there might not be one */
	curtype = &types[ntypes];
	outparen = ntypes > 0;
	if ((c = GetTypeName( fin, fout, &types[ntypes], flag, flag2,
			      outparen ))) {
	    if (ntypes == 0 && c == ')') {
		fprintf( stderr,
			 "Empty argument list in -ansi mode (use (void))\n");
		/* For this to work, gettypename can't output the last
		   closing paren */
		if ( useFerr ) {
		    fprintf( fout, "int *%s ", errArgNameLocal);
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

	/* Now, get the variable names until the arg terminator.
	   They are of the form [(\*]*name[(\*\[]*
	   */
	if (GetArgName( fin, fout, &args[nargs], curtype, 1 )) {
	    break;
	}
	args[nargs].type = ntypes-1;
	if (curtype) {
	    /* Propagate information about the type to the specific argument */
	    if (curtype->type_has_star)
		args[nargs].has_star++;
	    if (curtype->implied_star)
		args[nargs].implied_star++;
	    args[nargs].is_native = curtype->is_native;
	    args[nargs].is_char   = curtype->is_char;
	    args[nargs].is_const  = curtype->is_const;
	    args[nargs].is_FILE   = curtype->is_FILE;
	    if (args[nargs].is_char) {
		DBG("Found a string (is_char)\n");
		nstrings++;
	    }
	}
	if (nargs >= MAX_ARGS) {
	    ErrCnt++;
	    fprintf( stderr, "Too many arguments to function %s; only %d supported and function has %d\n",
		     name, MAX_ARGS, nargs );
	    ABORT( "Aborting: Too many arguments to function" );
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
	    if (useFerr) {
		fprintf( fout, "%sint *%s",
			 (nargs > 0) ? ", " : "", errArgNameLocal );
	    }
	    /* Add string length arguments */
	    if (nstrings && (stringStyle == STRING_LENEND ||
			     stringStyle == STRING_UNKNOWN)) {
		int i;
		for (i=0; i<nstrings; i++) {
		    fprintf(fout, ", int %s%d", stringArgLenName, i);
		}
	    }
	    fputs( ")\n", fout );
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

    /* Handle declaration of form int foo(void) */
    if (ntypes == 1 && nargs == 0 && strcmp("void",types[0].type) == 0) {
	ntypes = 0;
    }
    *Nargs  = nargs;
    *Ntypes = ntypes;
}

#endif
/* Read the arg list and function type */

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
char *ToCPointer( char *type, char *name, int implied_star )
{
    static char buf[300];
    const char *outstr = 0;
    if (SYConfigDBLookup("toptr", type, &outstr, toptrList) == 1 && outstr) {
	buf[0] = '\n'; buf[1] = '\t';
	snprintf(&buf[2], 300-2, outstr, name);
	return buf;
    }
    if (MapPointers)
	snprintf( buf, 300, "\n\t(%s%s)%sToPointer( *(int*)(%s) )",
		  type, !implied_star ? "* " : "", ptrprefix, name );
    else
	snprintf( buf, 300, "\n\t(%s%s)*((int*)%s)",
		  type, !implied_star ? "* " : "", name );

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
void PrintBody( FILE *fout, int is_function, char *name, int nstrings,
		int nargs, ARG_LIST *args, TYPE_LIST *types, RETURN_TYPE *rt )
{
    int  i, j, nstr;

    fputs( "{\n", fout );
    /* Look for special-cases (FILE and declarations for string processing) */
    for (i=0; i<nargs; i++) {
	if (args[i].is_FILE) {
	    fprintf( fout, "FILE *_fp%d = stdout;\n", i );
	}
    }
    stringDecl(fout, nargs, args);

    /* Add code for the string processing */
    stringFtoC(fout, nargs, args);

    /* Generate the routine call with the return */
    if (is_function) {
	/* FIXME: Must also use a local variable if there is any
	   post processing of arguments, e.g., free string storage */
	if (useFerr) {
	    fprintf(fout, "*%s = ", errArgNameLocal);
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

    nstr = 0;
    fputs( name, fout );
    fputs( "(", fout );
    for (i=0; i<nargs; i++) {
	if (args[i].is_FILE)
	    fprintf( fout, "_fp%d", i );
	else if (args[i].is_char && stringStyle == STRING_LENEND) {
	    fprintf(fout, "_%stmp%d", stringArgLenName, nstr);
	    nstr++;
	}
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

    if (is_function && MapPointers && rt->num_stars > 0 && !useFerr) {
	fprintf( fout, ") )" );
    }
    fputs( ");\n", fout);
    /* May need to process strings on exit if argument was not const */
    stringCtoF(fout, nargs, args);
    fputs("}\n", fout );
}

/*
 * In support for Fortran 9x/20xx, print a Fortran module interface definition.
 *
 * These definitions are of the form
 * {SUBROUTINE|FUNCTION} name( arg-list )
 * arg-decls
 * result-type name ! if function
 * end {SUBROUTINE|FUNCTION} name
 *
 * These are within an interface - end interface block, which is itself
 * within a module mod-name -- end module
 */
static int curCol = 0;
static int maxOutputCol = 72;
static int inComment = 0;
void OutputFortranToken( FILE *fout, int nsp, const char *token )
{
    int tokenLen = (int)strlen( token );
    int i;

    if (curCol + nsp > maxOutputCol) nsp = 0;
    for (i=0; i<nsp; i++) putc( ' ', fout );
    curCol += nsp;
    if (curCol + tokenLen > maxOutputCol) {
	while (curCol < 72) {
	    putc( ' ', fout );
	    curCol ++;
	}
	/* We continue a comment in a different way */
	if (inComment) {
	    putc( '\n', fout );
	    putc( '!', fout );
	    putc( ' ', fout );
	    curCol = 2;
	}
	else {
	    putc( '&', fout );
	    putc( '\n', fout );
	    for (i=0; i<5; i++) putc( ' ', fout );
	    putc( '&', fout );
	    curCol = 6;
	}
    }
    if (curCol == 0 && (*token != 'C' || *token != 'c') ) {
	/* Skip past column 6 to support free and fixed format */
	for (i=0; i<6; i++) putc( ' ', fout );
	curCol = 7;
    }
    fputs( token, fout );
    curCol += tokenLen;
    if (*token == '\n' ) {
	curCol    = 0;
	inComment = 0;
    }
    else if (*token == '!') {
	inComment = 1;
    }
}

/* This routine ensures that all arguments are distinct, even when case is
   not considered.  Specifically, if both "m" and "M" are argument names,
   the "M" argument will be replaced with "M$1" */
void FixupArgNames( int nargs, ARG_LIST *args )
{
    int i, j;
    char tmpbuf[MAX_LINE];
    char *c, *cout;

    for (i=0; i<nargs; i++) {
	int hasUpper = 0;
	/* printf( "Trying to fix %s\n", args[i].name ); */
	/* Produce a lower-case version of the name */
	c    = args[i].name;
	cout = tmpbuf;
	while (*c) {
	    *cout = tolower( *c );
	    if (*cout != *c) hasUpper = 1;
	    c++; cout++;
	}
	*cout = 0;
	if (hasUpper) {
	    /* Compare with the other arguments.  tmpbuf has the
	       lower-case version of the current name. */
	    /* Q: can we just use j<i? */
	    for (j=0; j<nargs; j++) {
		if (j == i) continue;
		c = args[j].name;
		cout = tmpbuf;
		while (*c && *cout) {
		    char mychar = tolower(*c);
		    if (mychar != *cout) break;
		    c++; cout++;
		}
		if (!*c && !*cout) {
		    /* Problem - matched lower case version.  Replace
		       current name with name + Upper */
		    /* printf( "Matched %s to %s\n", args[i].name,
		       args[j].name ); */
		    cout = tmpbuf;
		    while (*cout) cout++;
		    if (cout - tmpbuf > MAX_LINE - 6) {
			fprintf( stderr, "Argument name %s too long\n", tmpbuf );
			ABORT( "" );
		    }
		    *cout++ = 'u';
		    *cout++ = 'p';
		    *cout++ = 'p';
		    *cout++ = 'e';
		    *cout++ = 'r';
		    *cout++ = 0;
		    if (args[i].name) {
			FREE( args[i].name );
		    }
		    args[i].name = (char *)MALLOC( strlen( tmpbuf ) + 1 );
		    if (MPIU_Strncpy( args[i].name, tmpbuf, strlen( tmpbuf ) + 1 )) {
			ABORT( "Cannot replace argument name" );
		    }
		    break;
		}
	    }
	}
    }
}

/*
 * Create a Fortran 90 definition (module) for a function
 */
void PrintDefinition( FILE *fout, int is_function, char *name, int nstrings,
		      int nargs, ARG_LIST *args, TYPE_LIST *types,
		      RETURN_TYPE *rt )
{
    int  i;
    const char *token = 0;
    char sname[2];      /* Use for short argnames, if enabled */

    sname[1] = 0;       /* sname is a single character name */

    /*
     * Initial setup.  Fortran is case-insensitive and C is case-sensitive
     * Check that the case-insensitive argument names are distinct, and
     * if not, replace them with ones that are.  The rule is to
     * take a lowercase name and add "$1" to it.  We warn if a variable name
     * includes $1 ($ is permitted in Fortran names (check))
     */
    FixupArgNames( nargs, args );

    /* Check for void * args.  If found, we don't generate an interface,
       since we can't (at least until F2008+extensions for MPI) */
    for (i=0; i<nargs; i++) {
	/* Look for problems types */
	if (types[args[i].type].is_void ||
	    (strcmp(types[args[i].type].type,"void") == 0 &&
	     args[i].void_function == 0) ) {
	    break;
	}
    }
    if (i != nargs) {
	if (Debug)
	    fprintf(stderr,"Found void arg in routine.  Skipping f90 definition\n");
	/* Found a void argument.  Just return */
	return;
    }

    curCol = 0;
    /* Output the function definition */
    if (useFerr) {
	token = "subroutine";
    } else {
	token = is_function ? "function" : "subroutine";
    }
    OutputFortranToken( fout, 6, token );
    OutputFortranToken( fout, 1, name );
    OutputFortranToken( fout, 0, "(" );
    for (i=0; i<nargs-1; i++) {
	if (useShortNames) {
	    sname[0] = 'a' + (char)i;
	    if (i >=25) {
		ErrCnt++;
		fprintf(stderr,
		"Too many arguments in %s for -shortargname option\n", name);
		if (ErrCnt > MAX_ERR) ABORT("");
		/* Use a likely to still be valid name just to continue */
		sname[0] = 'A'+(char)(i-25);
	    }
	    OutputFortranToken(fout, 0, sname);
	    OutputFortranToken(fout, 0, ",");
	}
	else {
	    OutputFortranToken( fout, 0, args[i].name );
	    OutputFortranToken( fout, 0, ", " );
	}
    }
    if (nargs > 0) {
	/* Do the last arg, if any */
	if (useShortNames) {
	    sname[0] = 'a' + (char)(nargs-1);
	    if (nargs-1 >=25) {
		ErrCnt++;
		fprintf(stderr,
		"Too many arguments in %s for -shortargname option\n", name);
		if (ErrCnt > MAX_ERR) ABORT("");
		/* Use a likely to still be valid name just to continue */
		sname[0] = 'A'+nargs-1-25;
	    }
	    OutputFortranToken( fout, 0, sname );
	}
	else {
	    OutputFortranToken( fout, 0, args[nargs-1].name );
	    OutputFortranToken( fout, 0, " " );
	}
    }
    if (useFerr) {
	if (nargs > 0) OutputFortranToken( fout, 0, "," );
	OutputFortranToken( fout, 0, errArgNameParm );
    }
    OutputFortranToken( fout, 0, ")" );
    OutputFortranToken( fout, 0, "\n" );

    for (i=0; i<nargs; i++) {
	/* Figure out the corresponding Fortran type */
	if (types[args[i].type].is_mpi) {
	    OutputFortranToken( fout, 7, "integer" );
	}
	else if (args[i].void_function) {
	    OutputFortranToken( fout, 7, "external" );
	}
	else {
	    OutputFortranToken( fout, 7,
				ArgToFortran( types[args[i].type].type ) );
	}
	if (useShortNames) {
	    sname[0] = 'a' + (char)(i);
	    if (i >=25) {
		ErrCnt++;
		fprintf(stderr,
		"Too many arguments in %s for -shortargname option\n", name);
		if (ErrCnt > MAX_ERR) ABORT("");
		/* Use a likely to still be valid name just to continue */
		sname[0] = 'A'+(char)(i-25);
	    }
	    OutputFortranToken( fout, 1, sname );  /* space needed here */
	}
	else {
	    OutputFortranToken( fout, 1, args[i].name );
	}
	if (args[i].has_array && !args[i].void_function) {
	    OutputFortranToken( fout, 1, "(*)" );
	}
	OutputFortranToken( fout, 1, "!" );
	if (args[i].void_function) {
	    OutputFortranToken( fout, 1, "void function pointer" );
	}
	else {
	    OutputFortranToken( fout, 1, types[args[i].type].type );
	}
	OutputFortranToken( fout, 0, "\n" );
# if 0
	if (args[i].is_FILE) {
	    OutputFortranToken( fout, 0, "integer" );
	    OutputFortranToken( fout, 1, args[i].name );
	}
	else if (!args[i].is_native && args[i].has_star
		 && !args[i].void_function) {
	    if (args[i].has_star == 1 || !MultipleIndirectAreInts)
		OutputFortranToken( fout, 0,
			 ToCPointer( types[args[i].type].type, args[i].name,
				     args[i].implied_star ) );
	    else {
		if (MultipleIndirectsAreNative) {
		    OutputFortranToken( fout, 0, args[i].name );
		}
		else {
		    OutputFortranToken( fout, 0, "(" );
		    OutputFortranToken( fout, 0, types[args[i].type].type );
		    OutputFortranToken( fout, 0, " " );
		    if (!args[i].implied_star)
			for (j = 0; j<args[i].has_star; j++) {
			    OutputFortranToken( fout, 0, "*" );
			}
		    OutputFortranToken( fout, 0, ")*((int *)" );
		    OutputFortranToken( fout, 0, args[i].name );
		    OutputFortranToken( fout, 0, ")" );
		}
	    }
	}
	else {
	    /* if args[i].has_star, the argument often has intent OUT
	       or INOUT */
	    OutputFortranToken( fout, 0, args[i].name );
	}
#endif
    }

    /* Add a "decl/result(name) for functions */
    if (useFerr) {
	OutputFortranToken( fout, 7, "integer" );
	OutputFortranToken( fout, 1, errArgNameParm);
    } else if (is_function) {
	OutputFortranToken( fout, 7, ArgToFortran( rt->name ) );
	OutputFortranToken( fout, 1, name );
	OutputFortranToken( fout, 1, "!" );
	OutputFortranToken( fout, 1, rt->name );
    }
    OutputFortranToken( fout, 0, "\n" );
    OutputFortranToken( fout, 7, "end" );

    if (useFerr) {
	token = "subroutine";
    } else {
	token = is_function ? "function" : "subroutine";
    }
    OutputFortranToken( fout, 1, token );
    OutputFortranToken( fout, 1, name );
    OutputFortranToken( fout, 0, "\n" );
}

int NameHasUnderscore( char *p )
{
    while (*p)
	if (*p++ == '_') return 1;
    return 0;
}

void OutputRoutineName( char *name, FILE *fout )
{
    char buf[256], *p;
    int  ln;

    p = buf;
    if (MPIU_Strncpy( buf, name, sizeof(buf) )) {
	ABORT( "Cannot copy name to buf" );
    }
    if (IfdefFortranName) {
	LowerCase( p );
	ln = (int)strlen( p );
	p[ln] = '_';
	p[ln+1] = 0;
    }
    else {
#if defined(FORTRANCAPS)
	UpperCase( p );
#elif defined(FORTRANUNDERSCORE)
	LowerCase( p );
	ln	= (int)strlen( p );
	p[ln]	= '_';
	p[ln+1]	= 0;
#elif defined(FORTRANDOUBLEUNDERSCORE)
	LowerCase( p );
	ln	= (int)strlen( p );
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
    fputs( buf, fout );
}

void OutputUniversalName( FILE *fout, char *routine )
{
    char basename[MAX_ROUTINE_NAME], capsname[MAX_ROUTINE_NAME],
	nouscorename[MAX_ROUTINE_NAME];
    if (MPIU_Strncpy( basename, routine, sizeof(basename) )) {
	ABORT( "Cannot set basename" );
    }
    LowerCase( basename );
    if (MPIU_Strnapp( basename, "_", sizeof(basename) )) {
	ABORT( "Cannot append underscore to basename" );
    }
    if (MPIU_Strncpy( capsname, routine, sizeof(capsname) )) {
	ABORT( "Cannot set capsname" );
    }
    UpperCase( capsname );
    if (MPIU_Strncpy( nouscorename, routine, sizeof(nouscorename) )) {
	ABORT( "Cannot set nouscorename" );
    }
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

   If useFerr is true and the list is (void), return 0.

 */
int GetTypeName( FILE *fin, FILE *fout, TYPE_LIST *type, int is_macro,
		 int flag2, int outparen )
{
    int             c, nsp;
    char            token[1024];
    char            *typename = type->type;
    int             last_was_newline = 0;
    int             typenamelen = sizeof(type->type);

    typename[0]	        = 0;
    type->is_const      = 0;
    type->is_char       = 0;
    type->is_native     = 0;
    type->is_FILE       = 0;
    type->implied_star  = 0;
    type->type_has_star = 0;
    type->is_void       = 0;
    type->is_mpi        = 0;

    DBG("Looking for type...\n");
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
       ) in ANSI, or M * / in a Macro defn */
    if (c == EOF) return 1;
    if (c == '{') {
	/* We don't output the initial brace here (see printbody) */
	return 1;
    }
    if (c == '(' || c == ')') {
	if (outparen)
	    OutputToken( fout, token, nsp );
	else
	    return c;
	return 1;
    }
    /* The macro form should stop at a newline as well */
    if (is_macro && c == MACRO) {
	DBG("Checking for macro\n");
	c = getc( fin );
	if (c == '*') {
	    c = getc( fin );
	    if (c == '/') {
		DBG("Found end of macro defn\n");
		return 2;
	    }
	    else {
		/* This won't work on all systems (some only allow 1
		   pushback). */
		ungetc( '*', fin );
		ungetc( (char)c, fin );
	    }
	}
	else {
	    ungetc( (char)c, fin );
	}
    }

    /* Ignore qualifiers register/volatile/const */
    if (strcmp( token, "register" ) == 0) {
	c = FindNextANToken( fin, token, &nsp );
	if (c == EOF || c == '{' || c == '(') return 1;
    }

    if (strcmp( token, "volatile" ) == 0) {
	c = FindNextANToken( fin, token, &nsp );
	if (c == EOF || c == '{' || c == '(') return 1;
    }

    /* const is special, as it provides semantic information useful to
       bfort.  In particular, if the argument is const, then it will
       be unchanged by the call to the routine, and any transformation
       need only be made before input. */
    if (strcmp( token, "const" ) == 0) {
	c = FindNextANToken( fin, token, &nsp );
	if (c == EOF || c == '{' || c == '(') return 1;
	type->is_const = 1;
    }

    /* Read type declaration: struct name or [ unsigned ] type */
    if (strcmp( token, "struct" ) == 0) {
	if (MPIU_Strnapp( typename, token, typenamelen )) {
	    ABORT( "Cannot append token to typename" );
	}
	if (MPIU_Strnapp( typename, " ", typenamelen) ) {
	    ABORT( "Cannot append space to typename" );
	}
	OutputToken( fout, token, nsp );
	c = FindNextANToken( fin, token, &nsp );
	if (MPIU_Strnapp( typename, token, typenamelen )) {
	    ABORT( "Cannot append token to typename" );
	}
    }
    else {
	if (strcmp( token, "unsigned" ) == 0) {
	    if (MPIU_Strnapp( typename, token, typenamelen )) {
		ABORT( "Cannot append unsigned to typename" );
	    }
	    if (MPIU_Strnapp( typename, " ", typenamelen )) {
		ABORT( "Cannot append space to typename" );
	    }
	    OutputToken( fout, token, nsp );
	    c = FindNextANToken( fin, token, &nsp );
	}
	if (MPIU_Strnapp( typename, token, typenamelen )) {
	    ABORT( "Cannot append token to typename" );
	}
	/* Look for known names */
	if (strcmp( token, "char" ) == 0) {
	    DBG("Saw char\n");
	    type->is_char = 1;
	}
	if (strcmp( token, "FILE" ) == 0) {
	    DBG("Saw FILE\n");
	    type->is_FILE = 1;
	}
	/* FIXME: We should put these names in an array, and provide
	   a way to add to that array from a configuration file,
	   to make it easier to customize and extend this code */
	/* Note that we might want special processing for short and long */
	/* Some of these are NOT C types (complex, BCArrayPart)! */
#if 1
	if (SYConfigDBLookup("native", token, NULL, nativeList) == 1) {
	    type->is_native = 1;
	}
#else
	if (
	    strcmp( token, "double" ) == 0 ||
	    strcmp( token, "int"    ) == 0 ||
	    strcmp( token, "short"  ) == 0 ||
	    strcmp( token, "long"   ) == 0 ||
	    strcmp( token, "size_t" ) == 0 ||
	    strcmp( token, "float"  ) == 0 ||
	    strcmp( token, "char"   ) == 0 ||
	    strcmp( token, "complex") == 0 ||
	    strcmp( token, "dcomplex")== 0 ||
	    strcmp( token, "MPI_Status") == 0 ||
	    strcmp( token, "PetscScalar")== 0 ||
	    strcmp( token, "PetscReal")  == 0 ||
	    strcmp( token, "PetscTruth") == 0 ||
	    strcmp( token, "PetscSizeT") == 0 ||
	    strcmp( token, "MatStructure") == 0 ||
	    strcmp( token, "KSPConvergedReason") == 0 ||
	    strcmp( token, "BCArrayPart") == 0 ||
	    strcmp( token, "PetscLogDouble") == 0 ||
	    strcmp( token, "PetscInt") == 0 ||
	    strcmp( token, "SNESConvergedReason") == 0 ||
	    strcmp( token, "PetscMPIInt") == 0 ||
	    strcmp( token, "PetscErrorCode") == 0 ||
	    strcmp( token, "PetscCookie") == 0 ||
	    strcmp( token, "PetscEvent") == 0 ||
	    strcmp( token, "PetscBLASInt") == 0 ||
	    strcmp( token, "ISColoringValue") == 0 ||
	    strcmp( token, "MatReal") == 0 ||
	    strcmp( token, "PetscSysInt") == 0 ||
	    /* some structures - that are like arrays */
	    strcmp(token,"MatInfo") == 0 ||
	    strcmp(token,"MatStencil") == 0 ||
	    strcmp(token,"DALocalInfo") == 0 ||
	    strcmp(token,"MatFactorInfo") == 0 ||
	    0)
	    type->is_native = 1;
#endif
	/* PETSc types that are implicitly pointers are specified here */
	/* This really needs to take the types from a file, so that
	   it can be configured for each package.  See the search code in
	   info2rtf (but do a better job of it) */
#if 1
	if (SYConfigDBLookup("nativeptr", token, NULL, nativePtrList) == 1) {
	    type->type_has_star = 1;
	    type->implied_star  = 1;
	}
#else
	if (
	    strcmp(token,"AO") == 0 ||
	    strcmp(token,"AOData") == 0 ||
	    strcmp(token,"AOData2dGrid") == 0 ||
	    strcmp(token,"ClassPerfLog") == 0 ||
	    strcmp(token,"ClassRegLog") == 0 ||
	    strcmp(token,"DA") == 0 ||
	    strcmp(token,"DM") == 0 ||
	    strcmp(token,"DMMG") == 0 ||
	    strcmp(token,"EventPerfLog") == 0 ||
	    strcmp(token,"EventRegLog") == 0 ||
	    strcmp(token,"IntStack") == 0 ||
	    strcmp(token,"IS") == 0 ||
	    strcmp(token,"ISColoring") == 0 ||
	    strcmp(token,"ISLocalToGlobalMapping") == 0 ||
            strcmp(token,"Characteristic") == 0 ||
	    strcmp(token,"KSP") == 0 ||
	    strcmp(token,"Mat") == 0 ||
	    strcmp(token,"MatFDColoring") == 0 ||
	    strcmp(token,"MatNullSpace") == 0 ||
	    strcmp(token,"MatPartitioning") == 0 ||
	    strcmp(token,"MatSNESMFCtx") == 0 ||
	    strcmp(token,"PC") == 0 ||
	    strcmp(token,"PetscADICFunction") == 0 ||
	    strcmp(token,"PetscBag") == 0 ||
	    strcmp(token,"PetscBagItem") == 0 ||
	    strcmp(token,"PetscDLLibraryList") == 0 ||
	    strcmp(token,"PetscDraw") == 0 ||
	    strcmp(token,"PetscDrawAxis") == 0 ||
	    strcmp(token,"PetscDrawHG") == 0 ||
	    strcmp(token,"PetscDrawLG") == 0 ||
	    strcmp(token,"PetscDrawSP") == 0 ||
	    strcmp(token,"PetscFList") == 0 ||
	    strcmp(token,"PetscMap") == 0 ||
	    strcmp(token,"PetscMatlabEngine") == 0 ||
	    strcmp(token,"PetscObject") == 0 ||
	    strcmp(token,"PetscContainer") == 0 ||
	    strcmp(token,"PetscOList") == 0 ||
	    strcmp(token,"PetscRandom") == 0 ||
	    strcmp(token,"PetscTable") == 0 ||
	    strcmp(token,"PetscViewer") == 0 ||
	    strcmp(token,"PetscViewers") == 0 ||
	    strcmp(token,"PF") == 0 ||
	    strcmp(token,"SDA") == 0 ||
	    strcmp(token,"SNES") == 0 ||
	    strcmp(token,"StageLog") == 0 ||
	    strcmp(token,"TS") == 0 ||
	    strcmp(token,"Vec") == 0 ||
	    strcmp(token,"VecPack") == 0 ||
	    strcmp(token,"Vecs") == 0 ||
	    strcmp(token,"VecScatter") == 0 ||
	    /* the following are old stuff - might be requird for older versions
	       of PETSc */
	    strcmp(token,"PetscObjectContainer") == 0 ||
	    strcmp(token,"DF") == 0 ||
	    strcmp(token,"Discretization") == 0 ||
	    strcmp(token,"Draw") == 0 ||
	    strcmp(token,"DrawAxis") == 0 ||
	    strcmp(token,"DrawLG") == 0 ||
	    strcmp(token,"ElementMat") == 0 ||
	    strcmp(token,"ElementVec") == 0 ||
	    strcmp(token,"FieldClassMap") == 0 ||
	    strcmp(token,"GMat") == 0 ||
	    strcmp(token,"Grid") == 0 ||
	    strcmp(token,"GSNES") == 0 ||
	    strcmp(token,"GTS") == 0 ||
	    strcmp(token,"GVec") == 0 ||
	    strcmp(token,"Mesh") == 0 ||
	    strcmp(token,"Partition") == 0 ||
	    strcmp(token,"PetscDrawMesh") == 0 ||
	    strcmp(token,"SLES") == 0 ||
	    strcmp(token,"Stencil") == 0 ||
	    strcmp(token,"VarOrdering") == 0 ||
	    strcmp(token,"Viewer") == 0 ||
	    strcmp(token,"XBWindow") == 0 ||
	    0 )  {
	    type->type_has_star = 1;
	    type->implied_star  = 1;
	}
#endif

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
	    if (strcmp( token, "MPI_Comm" ) == 0       ||
		strcmp( token, "MPI_Request" ) == 0    ||
		strcmp( token, "MPI_Group" ) == 0      ||
		strcmp( token, "MPI_Op" ) == 0         ||
		strcmp( token, "MPI_Uop" ) == 0        ||
		strcmp( token, "MPI_File" ) == 0       ||
		strcmp( token, "MPI_Win"  ) == 0       ||
		strcmp( token, "MPI_Datatype" ) == 0   ||
		strcmp( token, "MPI_Errhandler" ) == 0 ||
		strcmp( token, "MPI_Info" ) == 0       ||
		strcmp( token, "MPI_Message" ) == 0    ||
		0) {
		type->is_mpi = 1;
		type->type_has_star = 1;
		type->implied_star  = 1;
	    }
#if 0
	    if (strcmp( token, "MPI_Aint" ) == 0) {
		/* For most systems, MPI_Aint is just long */
		type->type_has_star = 0;
		type->implied_star  = 0;
		type->is_native     = 1;
	    }
	    if (strcmp( token, "MPI_Offset" ) == 0) {
		/* For most systems, MPI_Offset is long long */
		type->type_has_star = 0;
		type->implied_star  = 0;
		type->is_native     = 1;
	    }
#endif
	}
	if (strcmp( token, "void" ) == 0) {
	    /* Activate set_void only for the files specified by flag2 */
	    if (!flag2) type->is_native = 1;
	    else type->is_void = 1;
	}
    }
    DBG2("Found type %s\n",token);
    if (useFerr && strcmp(token, "void") == 0) {
	/* Special case for (void) when we replace with an argument */
	while ( (c = SYTxtGetChar( fin )) != EOF && isspace(c)) ;
	ungetc( c, fin );
	if (c == ')') return 0;
    }
    if (type->is_mpi && isMPI) {
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
int GetArgName( FILE *fin, FILE *fout, ARG_LIST *arg, TYPE_LIST *type,
		int has_extra_chars )
{
    int c, nsp, nsp1, nparen, nbrack, nstar;
    char *p, token[1024];

    DBG("Looking for arg...\n")
/* This should really use a better parser.
   Names are
   [ *( ]* [restrict] varname [(*\[]*
   separated by , and terminated by ')'

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
    arg->has_star      = 0;
    arg->is_const      = 0;
    arg->is_char       = 0;
    arg->is_native     = 0;
    arg->has_array     = 0;
    arg->is_FILE       = 0;
    arg->void_function = 0;
    arg->implied_star  = 0;
    arg->name	       = 0;

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
	    fprintf( fout, "int *%s", errArgNameLocal );
	}
	OutputToken( fout, token, nsp );
	fputs("\n", fout);
	return 1;
    }
/* We don't want to output the token when we reach the name incase
   we need to generate a "*" for it */
    while (c != EOF) {
	if (strcmp( p, "restrict" ) == 0) {
	    c = FindNextANToken( fin, p, &nsp );
	    continue;
	}
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
    if (MPIU_Strncpy( arg->name, p, strlen(p) + 1) ) {
	ABORT( "Cannot set argument name" );
    }

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

    if (type && !arg->has_star && !type->implied_star &&
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
	    if (ErrCnt > MAX_ERR) ABORT( "" );
	}
    ungetc( (char)c, fin );
    return 0;
}

/* Output a balanced string.  The first character (cs) has been read */
void OutputBalancedString( FILE *fin, FILE *fout, int cs, int ce )
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

void DoBfortHelp( char *pgm )
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
-o filename - file to use for output.  The default is to create the output\n\
            filename from the input filename.\n\
-mapptr   - translate pointers to integer indices\n\
            The macro used to determine whether pointers are 64 bits can be\n\
            changed with\n\
\t-ptr64 name\tReplace POINTER_64_BITS\n\
-ptrprefix prefix - Prepend this name to the routines to map pointers\n");
fprintf(stderr, "\
-anyname  - generate Fortran names for a variety of systems\n\
            The macros used to select the form can be set with\n\
\t-fcaps name\tReplace FORTRANCAPS\n\
\t-fuscore name\tReplace FORTRANUNDERSCORE\n\
\t-fduscore name\tReplace FORTRANDOUBLEUNDERSCORE\n\
-ferr     - Fortran versions return the value of the routine as the last\n\
            argument (an integer).  This is used in MPI and is a not\n\
            uncommon approach for handling error returns.\n\
-shortargname - Use short (single character) argument names instead of the\n\
            name in the C definition.\n\
-mpi      - Handle MPI types (some things are pointers by definition)\n\
            The macro used to determine whether the MPI profiling version\n\
            is being built can be changed with\n\
\t-no_pmpi\tDo not generate PMPI names\n\
\t-pmpi name\tReplace MPI_BUILD_PROFILING\n\
\t-noprofile\tTurn off the generation of the profiling version\n\
-mnative  - Multiple indirects are native datatypes (no coercion)\n\
-on_error_abort  - End with nonzero error code when the first error is detected in the formatted comments\n\
-voidisptr - Consider \"void *\" as a pointer to a structure." );
}

void Abort( const char *msg, const char *file, int line )
{
    fprintf( stderr, "bfort terminating at %d: %s\n", line, msg );
    exit( 1 );
}

/*
 * Mapping of types between C and Fortran.
 *
 * These routines implement a translation between C and Fortran types.
 * For each C type that may be used in the source code, we need to know
 * the following:
 *    What is the corresponding Fortran datatype?
 *    What is the C type corresponding to that Fortran datatype?
 *    Are special steps required in translating between the C and Fortran
 *       types?  Two cases are MPI handles and character strings
 *
 *
 */

/*
 * This is a simple, temporary version of a routine to take a C type
 * (by name) and return the Fortran equivalent.
 */
const char *ArgToFortran( const char *typeName )
{
    const char *outName = 0;
    if (strcmp( typeName, "int") == 0) outName = "integer";
    else if (strcmp( typeName, "char" ) == 0) outName   = "character";
    else if (strcmp( typeName, "double" ) == 0) outName = "double precision";
    else if (strcmp( typeName, "float" ) == 0) outName  = "real";
    else if (strcmp( typeName, "short" ) == 0) outName  = "integer*2";
    /* The following is a temporary hack */
    else if (strcmp( typeName, "void" ) == 0) outName = "PetscVoid";
    else {
      if (!useUserTypes) {
	outName = "integer";
      } else {
	outName = typeName;
      }
    }
    return outName;
}

#if 0
/*
   Define the mapping a C to Fortran types, along with properties of the
   C types that are needed in generating the Fortran wrappers.
   Because the list is static (once created), we use a simple array, sorted
   by type type name in C, to improve search performance
*/
/* Properties */
#define CTYPE_IS_POINTER 0x1
#define CTYPE_IS_MPI_HANDLE 0x2
#define CTYPE_IS_CHARACTER 0x4

typedef struct {
    char cName[MAX_TYPE_NAME];   /* Name of the type in C */
    char fName[MAX_TYPE_NAME];   /* Corresponding name in Fortran */
    int flags;                   /* Each bit is used with a flag (CTYPE_IS_xxx)*/
} TypeInfo;

static TypeInfo *typeArray = 0;
static int typeArrayLen = 0;

/* Given a C type name, return the corresponding TypeInfo record */
TypeInfo *FindCType( const char *cName )
{
    int i=typeArrayLen/2, first = 0, last = typeArrayLen-1;
    int cmp;

    do {
	i = (last + first) / 2;
	cmp = strcmp( cName, typeArray[i].cName );
	if (cmp == 0) return typeArray + i;
	if (cmp > 0) {
	    first = i+1;
	}
	else {
	    last = i-1;
	}
    } while (first < last);

    return 0;
}

/* Sort the typeArray so that it can be used by the find routine above */
int typeCompare( const void *a, const void *b )
{
    const char *astr = ((TypeInfo *)a)->cName;
    const char *bstr = ((TypeInfo *)b)->cName;
    return strcmp( astr, bstr );
}
void sortTypeArray( void )
{
    qsort( typeArray, typeArrayLen, sizeof(TypeInfo), typeCompare );
}

/* Add to/from Fortran option in description? */
/* MPI_Comm_f2c( *(%s) ) */
{ "MPI_Comm", "integer", CTYPE_IS_MPI_HANDLE }, /* Add handle to int? */
{ "MPI_Request", "integer", CTYPE_IS_MPI_HANDLE },
{ "MPI_Group", "integer", CTYPE_IS_MPI_HANDLE },
{ "MPI_Op", "integer", CTYPE_IS_MPI_HANDLE },
{ "MPI_Datatype", "integer", CTYPE_IS_MPI_HANDLE },
{ "MPI_Win", "integer", CTYPE_IS_MPI_HANDLE },
{ "MPI_File", "integer", CTYPE_IS_MPI_HANDLE },
{ "MPI_Info", "integer", CTYPE_IS_MPI_HANDLE },
{ "MPI_Errhandler", "integer", CTYPE_IS_MPI_HANDLE },
/* MPI types */
{ "MPI_Aint", "integer (kind=MPI_ADDRESS_KIND)", 0 },
{ "MPI_Offset", "integer (kind=MPI_OFFSET_KIND)", 0 },

#endif

/*
   This is a better version of strncpy that does not zero out the entire
   array but does ensure that it is null-terminated, and returns a
   failure indication (value not 0) if the string did not fit.
*/
int MPIU_Strncpy( char *dest, const char *src, size_t n )
{
    char * restrict d_ptr = dest;
    const char * restrict s_ptr = src;
    register int i;

    if (n == 0) return 0;

    i = (int)n;
    while (*s_ptr && i-- > 0) {
	*d_ptr++ = *s_ptr++;
    }

    if (i > 0) {
	*d_ptr = 0;
	return 0;
    }
    else {
	/* Force a null at the end of the string (gives better safety
	   in case the user fails to check the error code) */
	dest[n-1] = 0;
	/* We may want to force an error message here, at least in the
	   debugging version */
	/*printf( "failure in copying %s with length %d\n", src, n ); */
	return 1;
    }
}

/* This is like strncat, but does an append and the size is the
   size of the dest buffer.  Return 0 on success. */
int MPIU_Strnapp( char *dest, const char *src, size_t n )
{
    char * restrict d_ptr = dest;
    const char * restrict s_ptr = src;
    register int i;

    /* Get to the end of dest */
    i = (int)n;
    while (i-- > 0 && *d_ptr) d_ptr++;
    if (i <= 0) return 1;

    /* Append.  d_ptr points at first null and i is remaining space. */
    while (*s_ptr && i-- > 0) {
	*d_ptr++ = *s_ptr++;
    }

    /* We allow i >= (not just >) here because the first while decrements
       i by one more than there are characters, leaving room for the null */
    if (i >= 0) {
	*d_ptr = 0;
	return 0;
    }
    else {
	/* Force the null at the end */
	*--d_ptr = 0;

	/* We may want to force an error message here, at least in the
	   debugging version */
	return 1;
    }
}

void FreeArgs( ARG_LIST *args, int nargs )
{
    int i;
    for (i=0; i<nargs; i++) {
	if (args[i].name) {
	    FREE( args[i].name );
	    args[i].name = 0;
	}
    }
}

/* Character string handling.  Converting to/from Fortran strings may involve
   making a copy of the data, in particular because Fortran strings are not
   NULL-terminated; instead, the length of the string storage is passed with
   the pointer to the storage (and there is no standard for this, though
   there is an approach used by most compilers now), and strings are
   blank-padded to the declared length of the string.

   These routines implement the most common approach.  In addition, if the
   string is declared const, then the string is only copied on input.  They
   process all of the arguments in a group; this allows them to handle
   any operations or declarations needed only once for all strings.
*/
int stringDecl(FILE *fout, int nargs, ARG_LIST args[])
{
    int i, nstring;

    /* For now, only implement the most common string style */
    if (stringStyle != STRING_LENEND) return 0;

    nstring        = 0;
    for (i=0; i<nargs; i++) {
	if (args[i].is_char) {
	    fprintf(fout,"  char *_%stmp%d=0;\n", stringArgLenName, nstring);
	    nstring++;
	}
    }
    return 0;
}

int stringFtoC(FILE *fout, int nargs, ARG_LIST args[])
{
    int i, nstring;

    /* For now, only implement the most common string style */
    if (stringStyle != STRING_LENEND) return 0;

    nstring = 0;     /* reset the string counter */
    for (i=0; i<nargs; i++) {
	if (args[i].is_char) {
	    /* For each character string, copy to a C string.
	       Note that we could define the end-of-string as the
	       point where the string is blank padded (this is what
	       MPICH does).  But in the general case, those trailing
	       blanks may be meaningful, so we copy the entire string */
	    fprintf(fout,"/* insert Fortran-to-C conversion for %s */\n",
		    args[i].name);
	    fprintf(fout,"\
  _%stmp%d = (char*)malloc(%s%d+1);\n\
  if (_%stmp%d) {\n\
     int _i;\n\
     for(_i=0; _i<%s%d; _i++) _%stmp%d[_i]=%s[_i];\n\
     _%stmp%d[_i] = 0;\n\
  }\n",
		    stringArgLenName, nstring, stringArgLenName, nstring,
		    stringArgLenName, nstring,
		    stringArgLenName, nstring, stringArgLenName, nstring, args[i].name,
		    stringArgLenName, nstring);
	    nstring++;
	}
    }
    return 0;
}

int stringCtoF(FILE *fout, int nargs, ARG_LIST args[])
{
    int i, nstring;

    /* For now, only implement the most common string style */
    if (stringStyle != STRING_LENEND) return 0;

    nstring = 0;
    for (i=0; i<nargs; i++) {
	if (args[i].is_char) {
	    if (!args[i].is_const) {
		/* This is more complex than the Fortran-to-C copy,
		   since a null must be replaced with blank pads */
		fprintf(fout,"/* insert C-to-Fortran conversion for %s */\n",
			args[i].name);
		fprintf(fout,"\
  if (_%stmp%d) {\n\
    int _i;\n\
    for(_i=0; _i<%s%d && _%stmp%d[_i]; _i++) %s[_i]=_%stmp%d[_i];\n\
    while (_i<%s%d) %s[_i++] = ' ';\n\
    free(_%stmp%d);\n\
  }\n",
			stringArgLenName, nstring,
			stringArgLenName, nstring, stringArgLenName, nstring, args[i].name, stringArgLenName, nstring,
			stringArgLenName, nstring, args[i].name,
			stringArgLenName, nstring);
	    }
	    else {
		fprintf(fout, "  if (_%stmp%d) { free(_%stmp%d); }\n",
			stringArgLenName, nstring, stringArgLenName, nstring);
	    }
	    nstring++;
	}
    }
    return 0;
}
