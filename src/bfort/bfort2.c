/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 * New version of bfort, which takes a C header file, with structured
 * documentation, and generates a Fortran interface, including generating
 * transformations of arguments as required.
 *
 * This is primarily used by PETSc, but can also be used by other codes.
 *
 * This version is a major rewrite of the original bfort code, which is over
 * 20 years old.  The original version attempted to process the input files
 * in a single pass fashion, which led to great complexity over time.  In
 * addition, that version handled both K&R and ANSI style declarations.
 *
 * This version reads and entire file, building a description of the routines
 * for which Fortran interfaces are generated, and then processes that
 * description to generate those interfaces.
 */

/*
 * A few simple todo notes:
 *
 * 1) Add info on the args that change behavior to the "This file" comment.
 * E.g., this include -ferr, -fstringxxxx. Could just add all args other than
 * filenames.  That does leave out the Environment variables, so another option
 * is to accumulate info on the run parameters from whatever source and then
 * output them,
 */
#include "bfort2.h"
#include <strings.h>  /* For rindex */

func_t *functions = 0;
func_t *functionsTail = 0;

/* Keep the file name to simplify finding files containing problems */
static char *CurrentFilename = 0;

/* Whether to replace pointers with indices to a mapping of pointers */
static int mapPointers = 0;
static char ptrPrefix[32] = "__";
static char ptr64Bits[256] = "POINTER_64_BITS";
static const char *ptrNameLocal = "_ptmp";

/* Catch serious problems */
#define MAX_ERR 100
int ErrCnt = 0;
#define ERRABORT \
    do { if (ErrCnt > MAX_ERR) { ABORT("Too many errors");} } while(0)
static int NoFortMsgs = 1;
/* NoFortWarnings turns off messages about things not being available in
   Fortran */
static int NoFortWarnings = 1;

/* Default #ifdef names for generated code */
static char FortranCaps[256] = "FORTRANCAPS";
static char FortranUscore[256] = "FORTRANUNDERSCORE";
static char FortranDblUscore[256] = "FORTRANDOUBLEUNDERSCORE";
/*static char BuildProfiling[256] = "MPI_BUILD_PROFILING";*/

/* Lists of type names */
static void *nativeList;
static void *nativePtrList;
static void *toptrList;
static void *fromptrList;
static void *declptrList;
static void *parmList;

/* Special global characteristics */

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

   stringTempName is used for any string temporary copy, and defaults to
   cltmp, and may be overridden in the config file.
 */
static const char *stringArgLenName = 0;
static const char *stringTempName   = 0;
static const char *mallocCheck = "if (!(%s)) {abort(1);}\n";
static enum { STRING_UNKNOWN, STRING_ABORT, STRING_LENEND }
    stringStyle = STRING_LENEND;

/* Enable the MPI definitions and conversion functions */
static int isMPI   = 0;
/* If true, add a last integer argument to int functions and return its
   value in the last parameter */
int useFerr = 0;
static const char *ferrFuncReturn = 0;
/* Defaults for these are "ierr" and "__ierr" */
const char *errArgNameParm = 0;
const char *errArgNameLocal = 0;

/* File for the include files (or other text) to insert at the beginning
   of the interface file */
static FILE *incfd=0;
static char *includeString=0;

/* This says to convert char **a to int*a, and cast to (char **)*a */
/*static int MultipleIndirectAreInts    = 1;*/
static int MultipleIndirectsAreNative = 0;

/* Debugging for development */
int Debug = 0;
#define DBG(a)    do {if (Debug) { fprintf(stderr, a);} }  while (0)
#define DBG2(a,b) do {if (Debug) { fprintf(stderr, a,b);} } while (0)

/* Forward references */
void DoBfortHelp(const char *pgm);
void SkipText (FILE *, const char *, const char *, char);
int SkipToSynopsis(FILE *, char);
int FindNextANToken (FILE *, char *, int, int *);
int ScanRoutine(FILE *fin, const char *routine, const char *infilename);
int ScanMacro(FILE *fin, const char *routine, const char *infilename);
int AppendInclude(FILE *fin, const char *routine, const char *infilename);
int GetFuncType(FILE *fin, const char *routine, const char *filename,
		func_t *nf);
int GetFuncArgs(FILE *fin, const char *routine, const char *filename,
		func_t *nf);
int GetSingleArg(FILE *fin, const char *routine, const char *filename,
		 arg_t **ap_ptr);
int cpyToken(char *p, int *plen, const char *token, int nsp);
int appendToken(char *p, int *plen, const char *token, int nsp);
char *dupToken(const char *token, int nsp);
int printFdefs(FILE *, func_t *nf);
int printOneFdef(FILE *, func_t *nf);

int applyToFuncs(FILE *fout, int (*single)(FILE *, func_t *, void *),
		 void *extra);
int routineNameMap(FILE *fout, func_t *fp, void *extra);
int outputUniversalName(FILE *fout);
int outputIfaceRoutine(FILE *fout, func_t *fp, void *extra);
int initFromConfig(void);
void setFromDefaults(void);
char *getOutputFilename(const char *infilename, const char *outdir);
int needPrePost(func_t *fp);
void copyFileFD(FILE *fd, FILE *fout);

void freeFuncDesc(func_t *fp);
int checkFunction(func_t *fp);

/* Argument processing routines.  It is important that no-op routines not
   but used - instead, provide null as the function pointer.  Parts of the
   code check for null function pointers to see if certain actions happen,
   such as post processing for transfering a C string to Fortran. */
static int nstring=0;
int stringIfaceInit(void);
int stringArglistDecl(FILE *, arg_t *);
int setupArgHandling(arg_t *ap);
void stringFtoCdecl(FILE *, arg_t *);
void stringFtoC(FILE *, arg_t *);
void stringCtoF(FILE *, arg_t *);
int getNstringsInArglist(arg_t *ap);

/* Pointer variables that need to be converted to/from C */
void pointerFtoCarg(FILE *, arg_t *);
void pointerFtoCdecl(FILE *, arg_t *);
void pointerFtoC(FILE *, arg_t *);
void pointerFtoCcall(FILE *, arg_t *);
void pointerCtoF(FILE *, arg_t *);

/* Native Pointer variables that need to be converted to/from C */
void nativeptrFtoCarg(FILE *, arg_t *);
void nativeptrFtoCdecl(FILE *, arg_t *);
void nativeptrFtoC(FILE *, arg_t *);
void nativeptrFtoCcall(FILE *, arg_t *);
void nativeptrInit(int *, char **);
void nativeptrHeader(FILE *, int);

/* Default handling for other variables */
void defaultFtoCarg(FILE *, arg_t *);
void defaultFtoCdecl(FILE *, arg_t *);
void defaultFtoC(FILE *, arg_t *);
void defaultFtoCcall(FILE *, arg_t *);

int substTypeinDecl(const char *orig, const char *oldtype, const char *newtype,
		    char **updated);

/* Save the commandline in a normalized format */
/* appendCmdLine prototype in bfort2.h */
char *getCmdLine(void);
void freeCmdLine(void);

/*
 * Algorithm for processing data:
 *
 * Step 0: Process commandline arguments and any configuration files
 *
 * Step 1: Read input file; find all qualifying functions based on their
 *   structured document.  These all start with a comment containing "@" or
 *   "M" right after the begin comment pair.
 *   Process each function (or macro) to create a new func_t structure for
 *   the routine.  This includes copying the function declared type and name,
 *   and building an arg_t list for all of the arguments.
 *
 * Step 2: Process the func_t structure to identify common needs; for example,
 *   are there any strings to process (which will require some special
 *   declarations).
 *
 * Step 3: Generate the output files. One file is the C to Fortran interface.
 *   Another optional output file is a Fortran 9x module.
 *
 */

int main(int argc, char *argv[])
{
    char *infilename = 0;
    FILE *fd;
    char routine[MAX_ROUTINE_NAME];
    char kind;
    char outfile[MAX_PATH_NAME];
    char *outfilep;
    char outdir[MAX_PATH_NAME];
    char incfile[MAX_FILE_SIZE];
    FILE *fout = 0;
    int  outperfile=0;
    int  nameMap = 1; /* Default is Double underscore default; -1 for
		         universal name */

    /* Process commandline arguments */
    if (SYArgHasName(&argc, argv, 1, "-help")) {
	DoBfortHelp(argv[0]);
	exit( 1 );
    }
    if (SYArgHasName(&argc, argv, 1, "-version")) {
	printf("bfort (sowing) release %d.%d.%d of %s\n",
		PATCHLEVEL, PATCHLEVEL_MINOR, PATCHLEVEL_SUBMINOR,
		PATCHLEVEL_RELEASE_DATE);
	exit(1);
    }

    outfile[0]  = 0;
    outdir[0]   = 0;
    incfile[0]  = 0;
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

    if (SYArgHasName(&argc, argv, 1, "-debug")) {
	Debug = 1;
    }

    SYArgGetString(&argc, argv, 1, "-o", outfile, MAX_PATH_NAME);
    SYArgGetString(&argc, argv, 1, "-dir", outdir, MAX_PATH_NAME);
    SYArgGetString(&argc, argv, 1, "-I",   incfile, MAX_PATH_NAME);
    NoFortMsgs		   = SYArgHasName(&argc, argv, 1, "-nomsgs");
    if (NoFortMsgs) appendCmdLine("-nomsgs");
    useFerr		   = SYArgHasName(&argc, argv, 1, "-ferr");
    if (useFerr) {
	appendCmdLine("-ferr");
	ferrFuncReturn = (const char *)malloc(128);
	if (!ferrFuncReturn) {
	    ABORT("Unable to allocate 128 bytes for ferrFuncReturn");
	}
	strncpy((char *)ferrFuncReturn, "void", 128);
	if (SYArgGetString(&argc, argv, 1, "-ferrfuncret",
			   (char *)ferrFuncReturn, 128)) {
	    appendCmdLine("-ferrfuncret");
	    appendCmdLine(ferrFuncReturn);
	}
    }
    MultipleIndirectsAreNative = SYArgHasName(&argc, argv, 1, "-mnative");
    if (MultipleIndirectsAreNative) appendCmdLine("-mnative");
    mapPointers		       = SYArgHasName(&argc, argv, 1, "-mapptr");
    if (mapPointers) {
	appendCmdLine("-mapptr");
	if (SYArgGetString(&argc, argv, 1, "-ptrprefix", ptrPrefix,
			   sizeof(ptrPrefix))) {
	    appendCmdLine("-ptrptrfix");
	    appendCmdLine(ptrPrefix);
	}
	if (SYArgGetString(&argc, argv, 1, "-ptr64", ptr64Bits,
			   sizeof(ptr64Bits))) {
	    appendCmdLine("-ptr64");
	    appendCmdLine(ptr64Bits);
	}
    }
    isMPI		   = SYArgHasName(&argc, argv, 1, "-mpi");
    if (isMPI) appendCmdLine("-mpi");

    /* Permit changing the ifdef names for the -anyname option */
    if (SYArgHasName(&argc, argv, 1, "-anyname")) {
	nameMap = -1;
	appendCmdLine("-anyname");
    }
    if (SYArgGetString(&argc, argv, 1, "-fcaps", FortranCaps, 256)) {
	appendCmdLine("-fcaps");
	appendCmdLine(FortranCaps);
    }
    if (SYArgGetString(&argc, argv, 1, "-fuscore", FortranUscore, 256)) {
	appendCmdLine("-fuscore");
	appendCmdLine(FortranUscore);
    }
    if (SYArgGetString(&argc, argv, 1, "-fduscore", FortranDblUscore, 256)) {
	appendCmdLine("-fduscore");
	appendCmdLine(FortranDblUscore);
    }

    /* String support */
    /* FIXME: These are only partly implemented!! */
    if (SYArgHasName(&argc, argv, 1, "-fstring")) {
	stringStyle = STRING_LENEND;
	appendCmdLine("-fstring");
    }
    if (SYArgHasName(&argc, argv, 1, "-fnostring")) {
	stringStyle = STRING_ABORT;
	appendCmdLine("-fnostring");
    }
    if (SYArgHasName(&argc, argv, 1, "-fstring-asis")) {
	stringStyle = STRING_UNKNOWN;
	appendCmdLine("-fstring-asis");
    }

    /* Initialize the native pointer options */
    nativeptrInit(&argc, argv);

    /* Check for f90 module options */
    f90ModuleInit(&argc, argv);

    /* Read the configuration files. This provides defaults that
       may be overridden by command-line arguments, so this routine
       only sets unset values. It should be called after all commandline
       options are processed. */
    initFromConfig();

    /* Update any remaining defaults */
    setFromDefaults();

    /* Do all setup that is needed before processing files */
    if (outdir[0] == 0) {
	if (MPIU_Strncpy(outdir, ".", sizeof(outdir))) {
	    ABORT("Unable to set dirname to \".\"");
	}
    }

    if (outfile[0]) {
	char *p, *p1;
	int ln = (int)strlen(outfile)+(int)strlen(outdir)+2;
	outfilep = (char *)malloc(ln);
	if (!outfilep) {
	    ABORT("Could not allocate space for outfile filename");
	}
	/* Copy the directory and file.  Since space allocated for
	   full string, we can just copy */
	p  = outfilep;
	p1 = outdir;
	while (*p1) *p++ = *p1++;
	*p++ = '/';
	p1 = outfile;
	while (*p1) *p++ = *p1++;
	*p = 0;

	fout = fopen(outfilep, "w");
	if (!fout) {
	    fprintf(stderr, "Could not open file %s\n", outfilep);
	    ABORT("");
	}
	free(outfilep);
    }
    else
	outperfile = 1;

    if (incfile[0]) {
	incfd = fopen(incfile, "r");
	if (!incfd) {
	    ErrCnt++;
	    fprintf(stderr, "Could not open file %s for -I option\n", incfile);
	    ERRABORT;
	}
    }

    /* Remaining arguments should be file names to process */
    argc--; argv++;
    while (argc--) {
	/* Skip null arguments */
	if (*argv == 0) { argv++; continue; }
	/* Input filename */
	infilename = *argv++;
	DBG2("Opening file %s\n", infilename);
	fd = fopen(infilename, "r");
	if (!fd) {
	    ErrCnt++;
	    fprintf(stderr, "Could not open file %s\n", infilename);
	    ERRABORT;
	    continue;
        }
	CurrentFilename = infilename;

	/* Scan file and build functions */
	DBG("Scanning file...\n");
	while (FoundLeader(fd, routine, &kind)) {
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
		    fprintf(stderr,
			    "%s %s(%s) cannot be translated into Fortran\n",
			    (kind == ROUTINE) ? "Routine" : "Macro",
			    routine, infilename);
		}
		SkipText(fd, routine, infilename, kind);
		continue;
	    }
	    /* Check for IsX11 routine ? */

	    if (kind == ROUTINE) {
		ScanRoutine(fd, routine, infilename);
	    }
	    else if (kind == MACRO) {
		/* Eventually we can handle this by using the Synopsis to
		   construct an equivalent definition */
		ScanMacro(fd, routine, infilename);
	    }
	    else if (kind == INCLUDE) {
		/* Save information on includes */
		AppendInclude(fd, routine, infilename);
	    }
	    /* else ignore */
	}
	fclose(fd);

	/* Step 2: Process the functions for the file */
	if (Debug) {
	    printf("All processed functions:\n");
	    printFdefs(stdout, functions);
	}

	/* Now the output processing */
	if (outperfile) {
	    /* Create an output file name */
	    outfilep = getOutputFilename(infilename, outdir);
	    fout     = fopen(outfilep, "w");
	}

	/* Print file header */
	fprintf(fout, "/* %s */\n", infilename);
	fprintf(fout, "/* Fortran interface file */\n");
	fprintf(fout, "/*\n\
 * This file was generated automatically by bfort from the C source file\n\
 * Major args:%s\n\
 */\n", getCmdLine());

	if (incfd) copyFileFD(incfd, fout);
	if (includeString) fprintf(fout, "%s", includeString);

	/* Generate the appropriate mapping for function names */
	if (nameMap >= 0) {
	    applyToFuncs(fout, routineNameMap, &nameMap);
	}
	else {
	    outputUniversalName(fout);
	}

	/* Generate the interface routines */
	fprintf(fout, "/* Definitions of Fortran Wrapper routines */\n");
	/* FIXME: Make this c++ stuff optional */
	fprintf(fout, "#if defined(__cplusplus)\nextern \"C\" {\n#endif\n");

	/* Add definitions for the pointer mapping routines if requested.
	 This is a no-op if pointer mapping has not been requested. */
	nativeptrHeader(fout, nameMap);

	/* Header files */
	if (needPrePost(functions)) {
	    fprintf(fout, "#include <stdlib.h>\n");
	}
	applyToFuncs(fout, outputIfaceRoutine, &nameMap);

	/* Finish off the C++ guard */
	fprintf(fout, "#if defined(__cplusplus)\n }\n#endif\n");

	if (outperfile) {
	    fclose(fout); fout = 0;
	    free(outfilep);
	}

	/* Generate the f90 interfaces if requested */
	f90OutputIfaces(functions);
    }


    if (fout)  { fclose(fout);  fout = 0;  }
    if (incfd) { fclose(incfd); incfd = 0; }

    return 0;
}

int ScanRoutine(FILE *fin, const char *routine, const char *infilename)
{
    func_t *nf = 0;

    DBG("Beginning ScanRoutine...\n");

    /* Create a new function entry */
    nf = (func_t *) malloc(sizeof(func_t));
    if (!nf) {
	ABORT("Unable to allocate function entry");
    }
    nf->next = 0;
    if (!functions) functions           = nf;
    else            functionsTail->next = nf;
    functionsTail = nf;

    /* Skip to trailer */
    SkipText(fin, routine, infilename, ROUTINE);

    /* Get the call to the routine, including finding the argument names */
    SkipWhite(fin);

    /* At this point, we have [type]* routine(arglist) */
    GetFuncType(fin, routine, infilename, nf);

    nf->origName = dupToken(routine, 0);

    /* Process the arguments */
    GetFuncArgs(fin, routine, infilename, nf);

    /* Temp for debugging: write out function definition */
    if (Debug) {
	printf("Function definition in scan routine:\n");
	printOneFdef(stdout, nf);
    }

    DBG("Ending ScanRoutine\n");
    return 0;
}

int ScanMacro(FILE *fin, const char *routine, const char *infilename)
{
    func_t *nf = 0;
    int    hasSynopsis;

    DBG("Beginning ScanMacro...\n");

    /* First check to see if this is a C-only macro.  If so, skip */
    /* Skip to the synopsis in the body */
    hasSynopsis = SkipToSynopsis(fin, MACRO);

    if (!hasSynopsis) {
	ErrCnt++;
	fprintf(stderr, "%s(%s) has no synopsis section\n",
		routine, infilename);
	if (ErrCnt > MAX_ERR) ABORT("");
	return -1;
    }

    /* Create a new function entry */
    nf = (func_t *) malloc(sizeof(func_t));
    if (!nf) {
	ABORT("Unable to allocate function entry");
    }
    nf->next = 0;
    if (!functions) functions           = nf;
    else            functionsTail->next = nf;
    functionsTail = nf;

    /* Get the call to the routine, including finding the argument names */
    SkipWhite(fin);

    /* At this point, we have [type]* routine(arglist) */
    GetFuncType(fin, routine, infilename, nf);

    nf->origName = dupToken(routine, 0);

    /* Process the arguments */
    GetFuncArgs(fin, routine, infilename, nf);

    /* Temp for debugging: write out function definition */
    if (Debug) {
	printf("Function definition in scan macro:\n");
	printOneFdef(stdout, nf);
    }

    /* finish up the section */
    SkipText(fin, routine, infilename, MACRO);

    DBG("Ending ScanMacro\n");
    return 0;
}

int AppendInclude(FILE *fin, const char *routine, const char *infilename)
{
    char *p;
    char firstChar;
    int  rlen, ln;

    DBG("Entering AppendInclude\n");
    DBG2("  input = '%s'\n", routine);

    /* Routine has the name, minus the first character (e.g., the " or <).
       Find the closing character and use the appropriate choice. */
    p = (char *)routine;
    while (*p) {
	if (*p == '"') {
	    firstChar = *p;
	    break;
	}
	else if (*p == '>') {
	    firstChar = '<';
	    break;
	}
	p++;
    }
    if (!*p) {
	ErrCnt++;
	fprintf(stderr, "Malformed INCLUDE spec %s in %s\n", routine,
		infilename);
	ERRABORT;
	return 1;
    }

    /* Append #include firstChar/routine to includeString */
    rlen = (int)strlen(routine);
    /* #include firstChar routine \n null */
    ln = 9 + 1 + rlen + 1 + 1;
    if (includeString) {
	int l1 = (int)strlen(includeString);
	includeString = (char *)realloc(includeString, l1+ln);
    }
    else {
	includeString = (char *)malloc(ln);
	if (includeString)
	    includeString[0] = 0;
    }
    if (!includeString) {
	ErrCnt++;
	fprintf(stderr,
		"Could not allocate additional include string of length %d\n",
		ln);
	ERRABORT;
	return 1;
    }
    p = includeString + strlen(includeString);
    strncpy(p, "#include ", 9);
    p += 9;
    *p++ = firstChar;
    strncpy(p, routine, rlen);
    p += rlen;
    *p++ = '\n';
    *p = 0;

    DBG("Exiting AppendInclude\n");
    return 0;
}

/* Internal routines */

/*
 * Get the return type of the function and save that value in nf.
 * Return 0 on success, non-zero on failure.
 *
 * Input file is positioned at the next character after the routine name.
 */
int GetFuncType(FILE *fin, const char *routine, const char *filename,
		func_t *nf)
{
    int  i, nsp, c;
    char *p=0;
    char *typename=0, *t;
    int  plen = 1024, tlen=1024;

    p = (char *)malloc(plen);
    if (!p) { ABORT("Unable to allocate token buffer in GetFuncType"); }
    t = (char *)malloc(tlen);
    if (!t) { ABORT("Unable to allocate typenanme buffer in GetFuncType"); }
    typename = t;

    SkipWhite(fin);

    /* Copy the type until we reach the routine name */
    while (1) {
	c = FindNextANToken(fin, p, plen, &nsp);
	if (strcmp(p, routine) == 0) break;
	if (c == '\n') {
	    /* Catch a likely error */
	    ErrCnt++;
	    fprintf(stderr, "Found newline before function name - likely error in structured comment for routine %s\n", routine);
	    ERRABORT;
	    return 1;
	}
	else if (c == '(') {
	    /* Definitely an error (unless we allow complex types in
	       function types) */
	    ErrCnt++;
	    fprintf(stderr, "Found open parenthesis before function name - likely error in structured comment for routine %s\n", routine);
	    ERRABORT;
	    return 1;
	}
	while (nsp-- && tlen > 0) {*t++ = ' '; tlen--; }
	for (i=0; p[i] && tlen > 0; i++) { *t++ = p[i]; tlen--; }
	*t = 0;
    }

    /* Make a copy of the typename in the function structure so that
       we can release the temp storage */
    nf->typeStr = (char *)malloc(strlen(typename)+1);
    if (!nf->typeStr) { ABORT("Unable to allocate typename"); }
    for (i=0; typename[i]; i++) nf->typeStr[i] = typename[i];
    nf->typeStr[i] = 0;

    DBG2("Type of function is %s\n", nf->typeStr);

    /* If the type is just void, the function does not return a value.
       Otherwise, we need to record the type of the return value */
    nf->isFunc = strcmp(nf->typeStr, "void") != 0;

    free(typename);
    free(p);

    return 0;
}

/*
 * Process the arguments
 */
int GetFuncArgs(FILE *fin, const char *routine, const char *filename,
		func_t *nf)
{
    int  c, rc;
    arg_t *ap, *aplast = 0;

    nf->arglist = 0;

    SkipWhite(fin);

    /* Next character must be an open parenthesis */
    c = getc(fin);
    if (c != '(') {
	ErrCnt++;
	fprintf(stderr, "Improper function declaration for %s\n", routine);
	ERRABORT;
	return 1;
    }

    DBG("Read opening paren in function\n");

    do {
	/* 0 = continue, 1 = done, -1 = error */
	rc = GetSingleArg(fin, routine, filename, &ap);
	if (rc < 0) {
	    /* Error count and message already generated in getSingleArg */
	}
	if (rc != 2) {
	    /* Add ap to function.  rc==2 was a (void) argument */
	    if (aplast) aplast->next = ap;
	    else nf->arglist = ap;
	    aplast = ap;
	}
    } while (rc == 0);

    return 0;
}

/*
 * Process a single argument declaration in a function definition
 * Return values: 0 for success, 1 for success and end of list, -1 for error,
 * 2 for success but a "(void)" argument.
 */
int GetSingleArg(FILE *fin, const char *routine, const char *filename,
		 arg_t **ap_ptr)
{
    int nparen=1;
    int  nsp, c, rc;
    char *p=0;
    char *typename=0, *t;
    int  plen = 1024, tlen=1024;
    char *basetype=0, *name=0;
    arg_t *ap;
    int   isAddress, nIndirects;
    int   hasConst = 0;

    /* Accepted arguments may have these forms:
         [const|volatile] type [\*][const|volatile|restrict ][\*]
	       name [\[[n]\]]*
         type (*name([arg][,arg]*))()
       The later is for a function that returns a pointer to a function
       of the described type

       We also need to record:
          The basetype
	  The presense of * (indicating the arg is a pointer)
	  The presence of [] (also indicating the arg is a pointer)
    */

    p = (char *)malloc(plen);
    if (!p) { ABORT("Unable to allocate token buffer in GetSingleArg"); }
    t = (char *)malloc(tlen);
    if (!t) { ABORT("Unable to allocate typenanme buffer in GetSingleArg"); }
    typename = t;
    t[0] = 0;

    /* First, process any attributes */
    DBG("Looking for declaration modifiers...\n");
    do {
	c = FindNextANToken(fin, p, plen, &nsp);
	if (c == '\n') continue;
	if (t[0] == 0) nsp = 0;
	if (strcmp(p, "const") == 0) hasConst = 1;
        if (strcmp(p, "const") != 0 && strcmp(p, "volatile") != 0 &&
	    strcmp(p, "restrict") != 0) break;
	/* Ignore spaces on the first term */
	if (appendToken(t, &tlen, p, nsp) != 0) {
	    ErrCnt++;
	    fprintf(stderr, "Type declaration too long in argument list for %s\n", routine);
	    ERRABORT;
	    return -1;
	}
    } while (1);

    DBG("Looking for declaration base type...\n");
    /* Base type is the next name, unless at end of list */
    /* Note that we already have a token from the above search for
       modifiers */
    if (c == ')') {
	/* Empty argument list */
	ErrCnt++;
	fprintf(stderr, "Empty argument list for %s - (void) is preferred\n",
	    routine);
	ERRABORT;
	free(p);
	free(t);
	/* Act as if we have seen (void), since so many codes use ()
	   instead of (void) */
	return 2;
    }
    /* This name is the base type */
    basetype = dupToken(p, 0);

    /* Not at the end of the list.  Create a new argument entry */
    ap = (arg_t *)malloc(sizeof(arg_t));
    if (!ap) { ABORT("Unable to allocate argument entry"); }
    /* Set defaults; only changed if different */
    ap->methodName = "default";
    ap->ftocarg    = defaultFtoCarg;
    ap->ftocdecl   = defaultFtoCdecl;
    ap->ftoc       = defaultFtoC;
    ap->ftoccall   = defaultFtoCcall;
    ap->ctof       = 0; /* defaultCtoF; */
    ap->free       = 0;
    ap->argextra   = 0;
    ap->inout      = ARG_INOUT;
    ap->actualarg  = 0;

    isAddress  = 0;
    nIndirects = 0;

    if (appendToken(t, &tlen, p, nsp) != 0) {
	ErrCnt++;
	fprintf(stderr, "Argument too long\n");
	ERRABORT;
    }

    while (nparen > 0) {
	c = FindNextANToken(fin, p, plen, &nsp);
	if (nparen == 1 && c == ',') {
	    /* Found an argument separator */
	    rc = 0;
	    break;
	}
	else if (c == '(') {
	    nparen++;
	}
	else if (c == ')') {
	    nparen--;
	    /* Check for at the last argument */
	    if (nparen == 0) {
		rc = 1;
		break;
	    }
	}
	else if (c == '*' || c == '[') {
	    isAddress = 1;
	    nIndirects++;
	}
	/* Save the token in the typename */
	if (appendToken(t, &tlen, p, nsp) != 0) {
	    ErrCnt++;
	    fprintf(stderr, "Argument too long\n");
	    ERRABORT;
	}
	if (!name && isalpha(*p) && strcmp(p, "restrict") != 0 &&
	    strcmp(p, "const") != 0)
	    name = dupToken(p, 0);
    }

    /* Special case (void) */
    if (strcmp(t, "void") == 0) {
	DBG("Found (void)\n");
	free(basetype);
	free(name);
	free(typename);
	free(p);
	free(ap);
	return 2;
    }

    /* Save information in the arg, then return it */
    ap->origDecl   = dupToken(t, 0);
    ap->origName   = dupToken(name, 0);
    ap->baseType   = dupToken(basetype, 0);
    ap->nIndirects = nIndirects;
    ap->isAddress  = isAddress;
    ap->next       = 0;
    if (hasConst) ap->inout = ARG_IN;
    if (SYConfigDBLookup("native", basetype, NULL, nativeList) == 1) {
	ap->isNative = 1;
    }

    if (setupArgHandling(ap)) {
	DBG2("WARNING: unable to handle arg conversions for %s\n",
	     ap->origName);
	/* rc = -1; */
    }

    DBG2("Found arg type %s\n", ap->origDecl);
    DBG2("\tname %s\n", ap->origName);
    DBG2("\tbase type %s\n", ap->baseType);

    free(basetype);
    free(name);
    free(typename);
    free(p);

    *ap_ptr = ap;

    return rc;
}

/* OLD routines from bfort.c */
void DoBfortHelp(const char *pgm)
{
fprintf(stderr, "%s - write a Fortran interface to C routines with\n",
	 pgm);
fprintf(stderr, "routines documented in the `doctext' format\n");
fprintf(stderr, "Optional arguments:\n");
fprintf(stderr, "\
filenames - Names the files from which lint definitions are to be extracted\n\
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
-fstring  - Enable handling of Fortran character parameters, using the most\n\
            common call interface.  This is the default unless the\n\
            environment variable BFORT_STRINGHANDLING is set to IGNORE.\n\
-fnostring - Disable handling of Fortran character parameters.  Routines\n\
            with 'char' arguments in C will not have interface routines\n\
            generated for them.\n\
-fstring-asis - No special processing for string arguments.  Provided\n\
            for backward compatiblity to older versions of bfort that\n\
            did not handle strings\n\
-mpi      - Handle MPI types (some things are pointers by definition)\n\
            The macro used to determine whether the MPI profiling version\n\
            is being built can be changed with\n\
-mnative  - Multiple indirects are native datatypes (no coercion)\n");

/* Old items
XX-nomsgs   - Do not generate messages for routines that can not be converted\n\
            to Fortran.\n\
XX-nofort   - Generate messages for all routines/macros without a Fortran\n\
            counterpart.\n\
XX\t-no_pmpi\tDo not generate PMPI names\n\
XX\t-pmpi name\tReplace MPI_BUILD_PROFILING\n\
XX\t-noprofile\tTurn off the generation of the profiling version\n\
XX-voidisptr - Consider \"void *\" as a pointer to a structure.");
 */
}

void Abort(const char *msg, const char *file, int line)
{
    fprintf(stderr, "bfort terminating at %d: %s\n", line, msg);
    exit(1);
}

void SkipText(FILE *fin, const char *name, const char *filename, char kind)
{
    int  c;
    char lineBuffer[MAX_LINE], *lp;

    lineBuffer[0] = '+';   /* Sentinal on lineBuffer */
    while (1) {
	lp = lineBuffer + 1;
	c  = getc(fin);
	if (c == EOF) break;
	if (c == ARGUMENT || c == VERBATIM)
	    SkipLine(fin);
	else if (c == '\n')
		;
	else {
	    if (isspace(c) && c != '\n')
		SkipWhite(fin);
	    else
		*lp++ = c;
	    /* Copy to end of line; do NOT include the EOL */
	    while ((c = getc(fin)) != EOF && c != '\n')
		*lp++ = c;
	    lp--;
	    while (isspace(*lp)) lp--;
	    lp[1] = '\0';    /* Add the trailing null */
	    if (lineBuffer[1] == kind && strcmp(lineBuffer+2,"*/") == 0)
		break;
        }
    }
}

/* Find the next space delimited token; put the text into token.
   The number of leading spaces is kept in nsp.
   Alpha-numeric tokens are terminated by a non-alphanumeric character
   (_ is allowed in alpha-numeric tokens)
   Returns the first non-white character read.
*/
int FindNextANToken(FILE *fd, char *token, int tlen, int *nsp)
{
    int fc, c, Nsp, tleft = tlen;

    Nsp = SkipWhite(fd);

    fc = c = getc(fd);
    if (fc == EOF) return fc;
    *token++ = c; tleft--;
    if (isalnum(c) || c == '_') {
	while ((c = getc(fd)) != EOF && tleft > 0) {
	    if (c != '\n' && (isalnum(c) || c == '_')) {
		*token++ = c;
		tleft--;
	    }
	    else break;
	}
	ungetc((char)c, fd);
    }
    *token++ = '\0';
    *nsp     = Nsp;
    return fc;
}

/* Skip until either find Synopsis: or the matching end-of-structured-comment
   which has character "kind".
   Returns 1 if synopsis is found; 0 otherwise.
*/
int SkipToSynopsis(FILE *fin, char kind)
{
    int  c;
    char lineBuffer[MAX_LINE], *lp;

    DBG("In SkipToSynopsis...");

    lineBuffer[0] = '+';   /* Sentinal on lineBuffer */
    while (1) {
	lp = lineBuffer + 1;
	c  = getc(fin);
	if (c == EOF) break;
	if (c == ARGUMENT || c == VERBATIM)
	    SkipLine(fin);
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
	    DBG2("Read line '%s'\n", lineBuffer);
	    if (lineBuffer[1] == kind && strcmp(lineBuffer+2,"*/") == 0)
		break;
	    if (lp[0] == ':') {
		lp = lineBuffer + 1;
		while (isspace(*lp)) lp++;
		DBG2("Found section '%s'\n", lp);
		LowerCase(lp);
		if (strcmp(lp, "synopsis:") == 0) {
		    DBG("Found synopsis, exiting\n");
		    return 1;
		}
	    }
        }
    }
    DBG("Did not find synposis, exiting");
    return 0;
}

/*
   This is a better version of strncpy that does not zero out the entire
   array but does ensure that it is null-terminated, and returns a
   failure indication (value not 0) if the string did not fit.
*/
int MPIU_Strncpy(char *dest, const char *src, size_t n)
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
	/*printf("failure in copying %s with length %d\n", src, n); */
	return 1;
    }
}


/* NEW SERVICE ROUTINES */

/* Copy nsp blanks, then token to p, which has *plen remaining
   locations.  Update *plen with the number of ununsed characters.

   return 0 on success, nonzero on failure
*/
int cpyToken(char *p, int *plen, const char *token, int nsp)
{
    int i, left=*plen, rc=1;

    while (nsp-- && left > 0) { *p++ = ' '; left--; }
    for (i=0; token[i] && left>0; i++,left--) {
	p[i] = token[i];
    }
    if (left > 0) {
	p[i] = 0;
	left--;
	rc = 0;
    }
    *plen = left;
    return rc;
}

/* Copy nsp blanks, then token to the current end of p,
   which has *plen remaining
   locations.  Update *plen with the number of ununsed characters.

   return 0 on success, nonzero on failure
*/
int appendToken(char *p, int *plen, const char *token, int nsp)
{
    char *p1 = p;

    while (*p1) p1++;
    return cpyToken(p1, plen, token, nsp);
}

/* Duplicate a token, including any leading blanks, and return a pointer
   to the duplicate.  Return null on error */
char *dupToken(const char *token, int nsp)
{
    int  i, ln;
    char *pout;

    if (token) ln = (int)strlen(token);
    else return (char *)0;

    pout = (char *)malloc(ln + 1 + nsp);
    if (!pout) return 0;
    for (i=0; i<nsp; i++) pout[i] = ' ';
    for (i=0; i<ln+1; i++) pout[nsp+i] = token[i];

    return pout;
}

/* Used for debugging - print out the structure of recorded function */
int printOneFdef(FILE *fp, func_t *nf)
{
    arg_t *ap;
    fprintf(fp, "%s %s(", nf->typeStr, nf->origName);
    ap = nf->arglist;
    if (!ap) {
	fprintf(fp, "void");
    }
    else {
	fprintf(fp, "\n");
	while (ap) {
	    fprintf(fp, "\t%s : %s %s%s %s", ap->origDecl, ap->baseType,
		      ap->origName, ap->isAddress ? "(address)":"",
		      ap->methodName);
	    if (ap->nIndirects > 0) {
		fprintf(fp, ": %d indirects", ap->nIndirects);
	    }
	    fprintf(fp, "\n");
	    ap = ap->next;
	}
    }
    fprintf(fp, ")\n");
    return 0;
}

int printFdefs(FILE *fp, func_t *nf)
{
    while (nf) {
	printOneFdef(fp, nf);
	nf = nf->next;
    }
    return 0;
}

/* Apply a routine to every routine */
int applyToFuncs(FILE *fout, int (*single)(FILE *, func_t *, void *),
		 void *extra)
{
    func_t *fp = functions;
    int    rc;

    while (fp) {
	rc = (*single)(fout, fp, extra);
	if (rc != 0) break;
	fp = fp->next;
    }
    return rc;
}

/* Interface generation */
/* Generate the name maps.  The extra arg is used to control the output
   name. This prints a "#define old new" line to fout
*/
int routineNameMap(FILE *fout, func_t *fp, void *extra)
{
    const char *routine = fp->origName;
    int   nameChoice = *(int*)extra;
    int   ln = (int)strlen(routine)+1;
    char  *basename, *outname;

    basename = (char *)malloc(ln);
    outname  = (char *)malloc(ln);
    if (!outname || !basename) { ABORT("Out of memory"); }

    MPIU_Strncpy(basename, routine, ln);
    LowerCase(basename);
    switch (nameChoice) {
    case 0: /* Caps */
	MPIU_Strncpy(outname, routine, ln);
	UpperCase(outname);
	fprintf(fout, "#define %s_ %s\n", basename, outname);
	break;
    case 1: /* Double underscore if the name contains an underscore */
	if (rindex(basename, '_') != (char *)0) {
	    fprintf(fout, "#define %s_ %s__\n", basename, basename);
	}
	/* Otherwise it is a single underscore, so nothing to do */
	break;
    case 2: /* No Underscore */
	fprintf(fout, "#define %s_ %s\n", basename, basename);
	break;
    default: /* Error */
	break;
    }

    free(basename);
    free(outname);

    return 0;
}

/* The default is to output the name in a single form, using the single
 underscore as basename. */
int outputUniversalName(FILE *fout)
{
    int nameMap;
    fprintf(fout, "#ifdef %s\n", FortranCaps);
    nameMap = 0;
    applyToFuncs(fout, routineNameMap, &nameMap);
    fprintf(fout, "#elif defined(%s)\n", FortranDblUscore);
    nameMap = 1;
    applyToFuncs(fout, routineNameMap, &nameMap);
    fprintf(fout, "#elif !defined(%s)\n", FortranUscore);
    nameMap = 2;
    applyToFuncs(fout, routineNameMap, &nameMap);
    fprintf(fout, "#endif\n");

    return 0;
}

int outputIfaceRoutine(FILE *fout, func_t *fp, void *extra)
{
    arg_t *ap;
    int  ln, ns;
    char *basename;
    int  nameMap = *(int*)extra;
    int  hasPostcall = 0;

    /* FIXME: Need a general arg handling reinitialization.  Could use
     this as an opportunity to check to see if there is *any* special
    processing */
    nstring = 0;
    ns  = getNstringsInArglist(fp->arglist);
    /* Query: Move this check before any function processing, so that
       no application of ApplyRoutine works on this interface? */
    if (stringStyle == STRING_ABORT && ns > 0) {
	fprintf(stderr, "Routine %s(%s) contains a char argument and cannot be translated into Fortran\n",
		fp->origName, CurrentFilename);
	ErrCnt++;
	ERRABORT;
	return 0;
    }

    /* Output function type.  FIXME: May need to provide C type corresponding
       to Fortran type; e.g., replace int with MPI_Fint */
    /* FIXME: If the Fortran name mapping results in a name that is the
       same as the original name, then no valid name mapping exists.  This
       can happen when the C name is monocase, and the Fortran mapping
       is to the same case (i.e., upper to upper or lower to lower).  In
       this case, generate at least a warning.  Alternately, provide a
       more general name mapping option (e.g., add "f" to the end of the
       routine name.
    */

    ln = (int)strlen(fp->origName)+1;
    basename = (char *)malloc(ln);
    if (!basename) {
	ABORT("Unable to allocate memory for basename");
    }
    MPIU_Strncpy(basename, fp->origName, ln);
    LowerCase(basename);
    fprintf(fout, "%s %s%s", useFerr ? ferrFuncReturn : fp->typeStr, basename,
	    (nameMap == -1) ? "_" : "");
    free(basename);

    /* Output function declaration, using C types for Fortran values */
    ap = fp->arglist;
    fprintf(fout, "(");
    if (!ap) {
	if (useFerr) {
	    fprintf(fout, "int *%s", errArgNameParm);
	}
	else {
	    fprintf(fout, "void");
	}
    }
    else {
	while (ap) {
	    /* FIXME: Some types are intrinsically an address.  Will
	       need to fix that. */
	    /* FIXME: Use instead the original declaration (what does
	       bfort do?) */
	    /* FIXME: Option to remove const and restrict qualifiers
	       from the declarations */
	    /* FIXME: We may also need to replace the "C" types with
	       Fortran types, and we may need to transform the type */
	    (*ap->ftocarg)(fout, ap);

	    /* Keep track of whether there is anything to do after the
	       call.  May want to generalize for cases where the parameter
	       does not require post-processing in this particular case */
	    if (ap->ctof || ap->free) hasPostcall++;

	    ap = ap->next;
	    if (ap) fprintf(fout, ", ");
	}
	if (useFerr) {
	    /* Guaranteed at least one arg output already */
	    fprintf(fout, ", int *%s", errArgNameParm);
	}
	/* Special case: may need to add string args */
	if (stringStyle == STRING_LENEND)
	    stringArglistDecl(fout, fp->arglist);
    }
    fprintf(fout, ")\n");

    /* Output function body:
       Declarations
       Precall data transformations including allocations
       Call
       Postcall data transformations including deallocations
       Return value.
    */
    fprintf(fout, "{\n");
    if (fp->isFunc && hasPostcall) {
	fprintf(fout, "  %s %s;\n", fp->typeStr, errArgNameLocal);
    }

    /* Output any declaration handling */
    /* FIXME: The stringIfaceInit here and elsewhere needs to become
       a general "initArgProcessing" for any needed initializations.
    */
    stringIfaceInit();
    ap = fp->arglist;
    while (ap) {
	if (ap->ftocdecl) (*ap->ftocdecl)(fout, ap);
	ap = ap->next;
    }

    /* Output any pre-call transformations */
    stringIfaceInit();
    ap = fp->arglist;
    while (ap) {
	if (ap->ftoc) (*ap->ftoc)(fout, ap);
	ap = ap->next;
    }

    /* Capture return value if there is one */
    /* FIXME: Optimization: If no post-call operations, can simply return
       the function value */
    if (fp->isFunc) {
	if (hasPostcall) {
	    fprintf(fout, "  %s = ", errArgNameLocal);
	}
	else {
	    if (useFerr) {
		fprintf(fout, "  *%s = ", errArgNameParm);
	    }
	    else {
		fprintf(fout, "  return ");
	    }
	}
    }
    else {
	fprintf(fout, "  ");
    }
    fprintf(fout, "%s", fp->origName);

    /* Output function declaration, using C types for Fortran values */
    ap = fp->arglist;
    fprintf(fout, "(");
    while (ap) {
	(*ap->ftoccall)(fout, ap);
	ap = ap->next;
    }
    fprintf(fout, ");\n");

    /* Output any post-call transformations */
    stringIfaceInit();
    ap = fp->arglist;
    while (ap) {
	if (ap->ctof) (*ap->ctof)(fout, ap);
	ap = ap->next;
    }

    /* Handle any return value */
    if (fp->isFunc && hasPostcall) {
	if (useFerr) {
	    fprintf(fout, "  *%s = %s;\n", errArgNameParm, errArgNameLocal);
	}
	else {
	    fprintf(fout, "  return %s;\n", errArgNameLocal);
	}
    }
    fprintf(fout, "}\n");

    return 0;
}

/* Read the configuration file(s) and initialize the various variables
   and tables */
#define NUM_CONFIG_CMDS 6
int initFromConfig(void)
{
    char fname[MAX_PATH_NAME];
    SYConfigCmds configcmds[NUM_CONFIG_CMDS+1];

    /* Initialize setup for config files */
    SYConfigDBInit("native", &nativeList);
    SYConfigDBInit("nativeptr", &nativePtrList);
    SYConfigDBInit("toptr", &toptrList);
    SYConfigDBInit("fromptr", &fromptrList);
    SYConfigDBInit("declptr", &declptrList);
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
    configcmds[4].name     = "fromptr";
    configcmds[4].docmd    = SYConfigDBInsert;
    configcmds[4].cmdextra = fromptrList;
    configcmds[5].name     = "declptr";
    configcmds[5].docmd    = SYConfigDBInsert;
    configcmds[5].cmdextra = declptrList;
    configcmds[NUM_CONFIG_CMDS].name     = 0;

    /* Read the basics, such as predefined C types */
    if (SYGetFileFromPathEnv(BASEPATH, "BFORT_CONFIG_PATH", NULL,
			     "bfort-base.txt", fname, 'r')) {
	DBG2("Reading config file %s\n", fname);
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
	    DBG2("Reading mpi config file %s\n", fname);
	    if (!SYReadConfigFile(fname, ' ', '#', configcmds,
				  NUM_CONFIG_CMDS)) {
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
	if (!errArgNameParm) {
	    SYConfigDBLookup("parm", "errparm",
			     &errArgNameParm, parmList);
	}
    }

    /* We use this for the return value of the routine, even if it isn't
       an error return */
    if (!errArgNameLocal) {
	/* Ignore if not set */
	SYConfigDBLookup("parm", "errparmlocal",
			 &errArgNameLocal, parmList);
    }

    /* Get the name to use for the length of the string argument.  Ignore
       if not set */
    SYConfigDBLookup("parm", "stringarglenname",
		     &stringArgLenName, parmList);
    SYConfigDBLookup("parm", "stringtempname",
		     &stringTempName, parmList);

    return 0;
}

/* For all values that have defaualts, set them if not already set */
void setFromDefaults(void)
{
    if (!errArgNameLocal)  errArgNameLocal  = "__ierr";
    if (!errArgNameParm)   errArgNameParm   = "ierr";
    if (!stringArgLenName) stringArgLenName = "cl";
    if (!stringTempName)   stringTempName   = "cltmp";
}

/*
 * Allocate and return the output files name from the input filename.
 * This includes stripping the directories and the filename extension
 * and adding [h]f.c
 *
 * The user must free the space when no-longer needed.
 */
char *getOutputFilename(const char *infilename, const char *outdir)
{
    char fname[MAX_PATH_NAME];
    char *p;
    char *outfilename;
    int  outlen;

    /* Set the output filename */
    SYGetRelativePath(infilename, fname, MAX_PATH_NAME);
    /* Strip the trailer */
    p = fname + strlen(fname) - 1;
    while (p > fname && *p != '.') p--;
    *p = 0;
    /* Add an extra h to include files */
    if (p[1] == 'h') {
	p[0] = 'h';
	p[1] = 0;
    }

    outlen = (int)strlen(outdir) + 1 + (int)strlen(fname) + 4;
    outfilename = (char *)malloc(outlen);
    if (!outfilename) {
	fprintf(stderr, "Unable to allocate %d bytes for output file name\n",
		outlen);
	ErrCnt++;
	ERRABORT;
	return 0;
    }
    snprintf(outfilename, outlen, "%s/%sf.c", outdir, fname);

    return outfilename;
}

/* String handling (special case of handling special argument processing */
static int argId = -1;
int stringIfaceInit(void)
{
    argId = 0;
    return 0;
}

void stringFtoCdecl(FILE *fout, arg_t *ap)
{
    int ln;
    ln = 1 + (int)strlen(stringArgLenName) + 4 + 1;  /* Allows 9999+1 args */
    ap->actualarg = (char *)malloc(ln);
    if (!ap->actualarg) {
	ABORT("Unable to allocate memory for actualarg");
    }
    ap->varId     = argId++;
    snprintf(ap->actualarg, ln, "_%s%d", stringTempName, ap->varId);
    fprintf(fout, "  char *%s=0;\n", ap->actualarg);
}
void stringFtoC(FILE *fout, arg_t *ap)
{
    int id = ap->varId;
    /* ap->actualarg should be the name to use, from the stringTempName */
    fprintf(fout, "  /* insert Fortran-to-C conversion for %s */\n",
	    ap->origName);
    fprintf(fout, "\
  %s = (char*)malloc(%s%d+1);\n\
  if (%s) {\n\
     int _i;\n\
     for(_i=0; _i<%s%d; _i++) %s[_i]=%s[_i];\n\
     %s[_i] = 0;\n\
  }\n",
	    ap->actualarg /* stringTempName, id */, stringArgLenName, id,
	    ap->actualarg /* stringTempName, id */,
	    stringArgLenName, id, ap->actualarg /* stringTempName, id*/,
	    ap->origName, ap->actualarg /*stringTempName, id*/);
    if (mallocCheck) {
	fputs("  ", fout);
	fprintf(fout, mallocCheck, ap->actualarg /*stringTempName, id*/);
    }
    nstring++;
}

void stringCtoF(FILE *fout, arg_t *ap)
{
    int id = ap->varId;
    if (ap->inout != ARG_IN) {
	/* This is more complex than the Fortran-to-C copy,
	   since a null must be replaced with blank pads */
	fprintf(fout, "  /* insert C-to-Fortran conversion for %s */\n",
		ap->origName);
	fprintf(fout, "\
  if (%s) {\n\
    int _i;\n\
    for(_i=0; _i<%s%d && %s[_i]; _i++) %s[_i]=%s[_i];\n\
    while (_i<%s%d) %s[_i++] = ' ';\n\
    free(%s);\n\
  }\n",
		ap->actualarg /*stringTempName, id*/,
		stringArgLenName, id, ap->actualarg /*stringTempName, id*/,
		ap->origName, ap->actualarg /*stringTempName, id*/,
		stringArgLenName, id, ap->origName,
		ap->actualarg /*stringTempName, id*/);
    }
    else {
	fprintf(fout, "  /* insert free for in string for %s */\n",
		ap->origName);
	fprintf(fout, "  if (%s) { free(%s); }\n",
		ap->actualarg /*stringTempName, id*/,
		ap->actualarg /*stringTempName, id*/);
    }
}

/* FIXME: This routine should also check configuration for more general,
   user-provided setup */
int setupArgHandling(arg_t *ap)
{
    const char *outstr;

    DBG2("Setup arg handling with basetype %s\n", ap->baseType);
    if (strcmp(ap->baseType, "char") == 0 && stringStyle != STRING_UNKNOWN) {
	DBG2("Arg %s is a string\n", ap->origName);
	ap->methodName = "string";
 	ap->ftocdecl   = stringFtoCdecl;
	ap->ftoc       = stringFtoC;
	ap->ctof       = stringCtoF;
    }
    else if (ap->isNative) {
	/* Do nothing - use default functions - already set */
	DBG2("Arg %s is native\n", ap->origName);
    }
    else if (SYConfigDBLookup("toptr",
			      ap->baseType, &outstr, toptrList) == 1 &&
	     outstr) {
	DBG2("Arg %s has pointer processing\n", ap->origName);
	ap->methodName = "pointer";
	ap->ftocarg    = pointerFtoCarg;
	ap->ftocdecl   = pointerFtoCdecl;
	ap->ftoc       = pointerFtoC;
	ap->ftoccall   = pointerFtoCcall;
	ap->ctof       = pointerCtoF;
	/* Could save outstr in ap */
    }
    else if (SYConfigDBLookup("nativeptr",
			      ap->baseType, &outstr, nativePtrList) == 1 &&
	     outstr) {
	DBG2("Arg %s has native pointer processing\n", ap->origName);
	ap->methodName = "nativePtr";
	ap->ftocarg    = nativeptrFtoCarg;
	ap->ftocdecl   = nativeptrFtoCdecl;
	ap->ftoc       = nativeptrFtoC;
	ap->ftoccall   = nativeptrFtoCcall;
	ap->ctof       = 0; /* nativeptrCtoF; */
	/* Could save outstr in ap */
    }
    else {
	/* Report error - unknown type ? */
	DBG2("Arg %s is of unknown type!\n", ap->origName);
	return 1;
    }

    return 0;
}

/* Return the number of strings in the function declaration */
int getNstringsInArglist(arg_t *ap)
{
    int   ns = 0;

    /* Determine the number of strings to handle */
    while (ap) {
	if (strcmp(ap->baseType, "char") == 0) ns++;
	ap = ap->next;
    }
    return ns;
}

int stringArglistDecl(FILE *fout, arg_t *ap_list)
{
    arg_t *ap;
    int   ns = 0;

    ap = ap_list;

    while (ap) {
	if (strcmp(ap->baseType, "char") == 0) {
	    fprintf(fout, ", int %s%d", stringArgLenName, ns++);
	}
	ap = ap->next;
    }

    return 0;
}

/* Check *all* functions for any arguments that require pre/post processing
   Also determine is special headers are required (e.g., stdlib)
 */
int needPrePost(func_t *fp)
{
    arg_t *ap;
    int   foundChar=0;

    while (fp) {
	ap = fp->arglist;
	while (ap) {
	    if (strcmp(ap->baseType, "char") == 0) {
		foundChar = 1;
		break;
	    }
	    ap = ap->next;
	}
	fp = fp->next;
    }

    return foundChar;
}

void copyFileFD(FILE *fd, FILE *fout)
{
    int c;
    fseek(fd, 0L, 0);
    while ((c = getc(fd)) != EOF)
	putc((char)c, fout);
}

/*
 * Pointer Mapping.  Some interfaces may use pointers in C; in all but
 * the most recent versions of Fortran, there are no equivalent types,
 * and pointers must be mapped to and from Fortran integers.
 *
 * Unfortunately, there isn't enough information in a standard argument
 * list to fully describe any transformation that might be needed.  Thus,
 * this set of routines only approximates the transformations needed.
 * In lieu of this information, we use the following:
 * If the type is a value (no explicit address indicator, i.e., no * or []),
 * it is an IN value and is converted and passed to the function.
 * It the type is an address (with *), and is not const, it is converted on
 * output (that is, assumed as OUT).
 * Otherwise, issue a warning message (and optionally do not generate an
 * interface).  This handles cases where an array of objects is passed; these
 * can't be converted without knowing the length of the array.
 *
 * Specifically,
 * 1) if the formal argument is a value, output a pointer to the replacement
 *    type (provided by the fromptr descriptions in the configuration files).
 * 2) if the format argument is a pointer to a value but not an array,
 *    will output the same as in the value case (1 above).
 *    However, it will then use ftocdecl to add a declaration for the
 *    C type, and pass that variable by address.  ctofPostcall will use
 *    from?? to convert back to Fortran, and store in the correct output
 *    variable
 *
 * These routines provide support for these operations:
 *
 * isPointerArg - Indicates if an argument is pointer that must be
 *                converted
 *
 * toDo - should use the arg routines!!!
 *
 */

/*
 * There are two supported ways to handle the pointer types.
 * The original bfort approach replqced them with INTEGER, and
 * assumes the values are passed by address as usual.
 *
 * An alternative, used by PETSc, is to define types (possibly using
 * macro replacement), and using a ToPointer routine that takes the
 * address of the pointer directly.
 */


/*
 * Substitute newtype for old type in a copy of orig, returned through updated.
 * updated must be freed when no longer needed with free.
 */
int substTypeinDecl(const char *orig, const char *oldtype, const char *newtype,
		    char **updated)
{
    char *p, *newp;
    const char *p1, *p2;
    int  i, len;

    p = strstr(orig, oldtype);
    if (!p) {
    }
    len = (int)strlen(orig) - (int)strlen(oldtype) + (int)strlen(newtype);
    newp = (char *)malloc(len + 1);
    if (!newp) {
	ABORT("Unable to allocate memory for new type name");
    }

    p1 = orig;
    i  = 0;
    while (p1 != p) newp[i++] = *p1++;
    p2 = newtype;
    while (*p2) newp[i++] = *p2++;
    p1 += strlen(oldtype);
    while (*p1) newp[i++] = *p1++;
    newp[i] = 0;

    *updated = newp;
    return 0;
}

/*
 * Check that a function (or macro) can be properly processed.
 *
 * This checks that the argument list doesn't contain anything that
 * will prohibit processing, such as pointer arrays to be mapped or
 * character strings with -fnostring (stringStyle == STRING_ABORT)
 */
int checkFunction(func_t *fp)
{
    arg_t *ap;

    ap = fp->arglist;
    while (ap) {
        if (stringStyle == STRING_ABORT && strcmp(ap->baseType, "char") == 0)
	    return 1;
        ap = ap->next;
    }
    return 0;
}
/*
 * Free a function description
 */
void freeFuncDesc(func_t *fp)
{
    arg_t *ap, *apold;

    ap = fp->arglist;
    while (ap) {
        apold = ap;
        ap    = ap->next;
        free(apold);
    }
    free(fp);
}

/* Transformations for pointer */
void pointerFtoCarg(FILE *fout, arg_t *ap)
{
    const char *outstr=0;
    const char *basetype;

    if (SYConfigDBLookup("declptr",
	ap->baseType, &outstr, declptrList) == 1 && outstr) {
        /* outstr is the replacement type */
        basetype = outstr;
	DBG2("decl: type %s replaced\n", basetype);
    }
    else {
        /* Error - missing definition */
        DBG2("decl: Missing declptr for %s\n", ap->baseType);
	basetype = ap->baseType;
    }

    if (ap->isAddress && outstr) {
        char *p1;
	substTypeinDecl(ap->origDecl, ap->baseType, outstr, &p1);
	fprintf(fout, "%s", p1);
	free(p1);
    }
    else {
	fprintf(fout, "%s %s%s", basetype, "*", ap->origName);
    }
}
void pointerFtoCdecl(FILE *fout, arg_t *ap)
{
    if (ap->isAddress) {
        ap->varId = argId++;
        fprintf(fout, "  %s %s%d;\n", ap->baseType, ptrNameLocal, ap->varId);
    }
}

void pointerFtoC(FILE *fout, arg_t *ap)
{
}

void pointerFtoCcall(FILE *fout, arg_t *ap)
{
    const char *outstr;
    char *argname  = ap->actualarg ? ap->actualarg : ap->origName;
    char       *buf=0;

    if (ap->isAddress) {
	fprintf(fout, "&%s%d%s", ptrNameLocal, ap->varId, ap->next ? ", ":"");
    }
    else {
	if (SYConfigDBLookup("toptr", ap->baseType, &outstr, toptrList) == 1 &&
	    outstr) {
	    /* If the type requires a conversion, extract that here */
	    buf = (char *)malloc(1024);
	    if (!buf) {
		ABORT("Unable to allocate memory in FtoC call");
	    }
	    snprintf(buf, 1024, outstr, argname);
	}
	else {
	    /* FIXME: Warning or error */
	    DBG2("Expected %s to have toptr info\n",ap->origName);
	}
	/* Note that this is a value, but we convert the address passed
	   in into a value in the toptr call */
	fprintf(fout, "%s%s", buf ? buf : argname, ap->next ? ", " : "");
    }

    if (buf) free(buf);

}

void pointerCtoF(FILE *fout, arg_t *ap)
{
    const char *outstr = 0;
/*    char       *argname  = ap->actualarg ? ap->actualarg : ap->origName; */
    char       *buf=0;
    char       *largname=0;

    if (ap->isAddress) {
	int ln = (int)strlen(ptrNameLocal)+10;
	largname = (char *)malloc(ln+1);
	if (!largname) {
	    ABORT("Unable to allocate memory for CtoF");
	}
	snprintf(largname, ln, "%s%d", ptrNameLocal, ap->varId);

        /* Needs a fromcall transformation */
	if (SYConfigDBLookup("fromptr", ap->baseType,
			     &outstr, fromptrList) == 1 && outstr) {
	    /* If the type requires a conversion, extract that here */
	    buf = (char *)malloc(1024);
	    if (!buf) {
		ABORT("Unable to allocate memory for largname");
	    }
	    snprintf(buf, 1024, outstr, largname);
	    DBG2("CtoF found fromptr type %s\n", outstr);
	    fprintf(fout, "  *%s = %s;\n", ap->origName, buf);
	    free(largname);
	    free(buf);
	}
	else {
	    /* FIXME: Warning or error */
	    DBG2("Expected %s to have toptr info\n",ap->origName);
	}
    }
}

/* Transformations for native pointer (These use a common transformation
 approach) */
static int nativeUseInt = 1;
void nativeptrInit(int *argc_ptr, char **argv)
{
    if (SYArgHasName(argc_ptr, argv, 1, "-usenativeptr")) {
	nativeUseInt = 0;
	DBG("Set nativeUseInt to 0\n");
	appendCmdLine("-usenativeptr");
    }
}

void nativeptrHeader(FILE *fout, int nameMap)
{
    /* Note that RmPointer is unused, so has been removed
         extern void %sRmPointer(int);\n\
         #define %sRmPointer(a)\n
        Fill %s with ptrPrefix
       FromPointer is used in bfort-old, and is retained here.
     */
    if (mapPointers) {
	if (nameMap) {
	    fprintf(fout, "\n\
/* Mapping of pointers in C to and from integers in Fortran */\n\
#ifdef %s\n\
extern void *%sToPointer(int);\nextern int %sFromPointer(void *);\n\
#else\n\
#define %sToPointer(a) (a)\n#define %sFromPointer(a) (int)(a)\n\
#endif\n\n",
		    ptr64Bits,
		    ptrPrefix, ptrPrefix,
		    ptrPrefix, ptrPrefix);
	}
	else {
	    fprintf(fout,
		    "extern void *%sToPointer(); extern int %sFromPointer();\n",
		    ptrPrefix, ptrPrefix );
	}
    }
}

void nativeptrFtoCarg(FILE *fout, arg_t *ap)
{
    DBG2("Adding arg %s\n", ap->origDecl);
    if (ap->isAddress || !nativeUseInt) {
        fprintf(fout, "%s", ap->origDecl);
    }
    else {
        fprintf(fout, "%s %s%s", ap->baseType, "", ap->origName);
    }
}
void nativeptrFtoCdecl(FILE *fout, arg_t *ap)
{
}

void nativeptrFtoC(FILE *fout, arg_t *ap)
{
}

void nativeptrFtoCcall(FILE *fout, arg_t *ap)
{
    if (MultipleIndirectsAreNative && ap->nIndirects > 0) {
        fprintf(fout, " %s%s", ap->origName, ap->next ? ", " : "");
    }
    else if (nativeUseInt) {
	fprintf(fout, "\n\t(%s)%sToPointer(*(int*)(%s))%s", ap->baseType,
		ptrPrefix, ap->origName, ap->next ? ", " : "");
    }
    else {
	fprintf(fout, "\n\t(%s)%sToPointer(%s)%s", ap->baseType,
		ptrPrefix, ap->origName, ap->next ? ", " : "");
    }
}


/* Default processing routines for arguments */
void defaultFtoCarg(FILE *fout, arg_t *ap)
{
    if (ap->isAddress) {
        fprintf(fout, "%s", ap->origDecl);
    }
    else {
        fprintf(fout, "%s %s%s", ap->baseType, "*", ap->origName);
    }
}

void defaultFtoCdecl(FILE *fout, arg_t *ap)
{
    /* Nothing required */
}

void defaultFtoC(FILE *fout, arg_t *ap)
{
    /* Nothing required */
}

void defaultFtoCcall(FILE *fout, arg_t *ap)
{
    fprintf(fout, "%s%s%s", ap->isAddress ? "" : "*",
	ap->actualarg ? ap->actualarg : ap->origName,
	ap->next ? ", " : "");
}

static char *normalizedCmdLine = 0;
static int   nCmdLineLen = 0;
static char *curptr = 0;

void appendCmdLine(const char *str)
{
    int len = (int)strlen(str) + 1;  /* Include the preceeding space */

    if (nCmdLineLen == 0) {
	/* Initial allocation */
	nCmdLineLen = 1024;
	normalizedCmdLine = (char *)malloc(nCmdLineLen);
	if (!normalizedCmdLine) {
	    fprintf(stderr, "Unable to allocated %d bytes for ncmdline\n",
		    nCmdLineLen);
	    return;
	}
	curptr = normalizedCmdLine;
    }

    while (len + (curptr-normalizedCmdLine) + 1 >= nCmdLineLen) {
	int nlen= 2 * nCmdLineLen;
	int offset = (int)(curptr - normalizedCmdLine);
	normalizedCmdLine = realloc(normalizedCmdLine, nlen);
	curptr = normalizedCmdLine + offset;
	nCmdLineLen = nlen;
    }

    strncpy(curptr, " ", 2);
    curptr++;
    strncpy(curptr, str, len+1);
    curptr += strlen(str);
}

char *getCmdLine(void)
{
    return normalizedCmdLine;
}

void freeCmdLine(void)
{
    if (normalizedCmdLine) {
	free(normalizedCmdLine);
    }
}
