/* -*- Mode: C; c-basic-offset:4 ; -*- */

#include "bfort2.h"
/*
 * Routines for the generation of a Fortran 9x interface file from
 * the doctext-headers used with C programs.  Called by bfort2
 */

static void OutputFortranToken(FILE *, int, const char *);
/*static void FixupArgNames(int nargs, ARG_LIST *args);*/
static const char *ArgToFortran(const char *typeName);

static int f90mod_skip_header = 1;
/* when converting C type to Fortran for F90 interfaces keep any unknown ones */
static int useUserTypes = 1;
/* If 0, do not generate Fortran 9x interface module */
static int F90Module = 0;
static FILE *fmodout = 0;
static const char *f90headerName = "f90header";
/* Use this for a module name to include in every interface spec */
static char *f90usemodule = 0;
static char modnamestr[256];

/* if 0, use original argument name. If 1, use a single character name
   instead.  Used in generating the Fortran interface module, if requested,
   and used for the error return if -ferr is selected. */
static int useShortNames = 0;

/* These control the Fortran output formatting */
static int curCol = 0;
static int maxOutputCol = 72;

/* Access the error count */
extern int ErrCnt;

int F90FuncCheck(func_t *fp);
int checkValidForF90(func_t *, int);
int f90ModuleOutput(FILE *fout, func_t *fp);

/*
 * Check for options to enable f90 output, and initialize if requested
 */
int f90ModuleInit(int *argc_ptr, char **argv)
{
    /* See if we should create the F90 Module file. */
    f90mod_skip_header = SYArgHasName(argc_ptr, argv, 1, "-f90mod_skip_header");
    if (!f90mod_skip_header) {
	/* Check for the more appropriate spelling */
	f90mod_skip_header =
	    SYArgHasName(argc_ptr, argv, 1, "-f90mod-skip-header");
    }
    if (f90mod_skip_header) {
	appendCmdLine("-f90mod-skip-header");
    }

    useShortNames          = SYArgHasName(argc_ptr, argv, 1, "-shortargname");
    if (useShortNames) {
	appendCmdLine("-shortargname");
    }

    /* This allows only one module to be added to the declaration of each
       interface */
    if (SYArgGetString(argc_ptr, argv, 1, "-f90usemodule", modnamestr,
		       sizeof(modnamestr))) {
	f90usemodule = modnamestr;
	appendCmdLine("-f90usemodule");
	appendCmdLine(modnamestr);
    }

    /* -shortargname overrides config file for the err arg name */
    if (errArgNameParm) {
	DBG2("bfortmod: errArgNameParm already set to %s\n", errArgNameParm);
    }
    if ( (! errArgNameParm) && useShortNames)
	errArgNameParm = "z";

    /* If an f90modfile argument is provided, then enable the f90module.
       Note that we do not remove it - we will get the value for this
       option below. */
    if (SYArgHasName(argc_ptr, argv, 0, "-f90modfile")) {
	F90Module = 1;
	appendCmdLine("-f90modfile");
    }
    /* If there is a f90 module file, open it now and add the header */
    if (F90Module) {
	char fmodfile[MAX_FILE_SIZE];
	if (!SYArgGetString(argc_ptr, argv, 1, "-f90modfile",
			     fmodfile, MAX_FILE_SIZE)) {
	    if (MPIU_Strncpy(fmodfile, "f90module.f90", sizeof(fmodfile))) {
		ABORT("Unable to set the name of the Fortran 90 module file");
	    }
	    /* Append the default module name */
	    appendCmdLine("f90module.f90");
	}
	else {
	    appendCmdLine(fmodfile);
	}

	fmodout = fopen(fmodfile, "w");
	if (!fmodout) {
	    ErrCnt++;
	    fprintf(stderr,
		    "Could not open file %s for Fortran 90 interface output\n",
		    fmodfile);
	    if (ErrCnt > MAX_ERR) ABORT("");
	    F90Module = 0;
	}
	else {
	    if (!f90mod_skip_header) {
		OutputFortranToken(fmodout, 0, "module");
		OutputFortranToken(fmodout, 1, f90headerName);
		OutputFortranToken(fmodout, 0,"\n");
		OutputFortranToken(fmodout, 0, "interface");
		OutputFortranToken(fmodout, 0, "\n");
	    }
	}
    }
    return 0;
}

int f90OutputIfaces(func_t *fp)
{

    if (! (F90Module && fmodout)) {
	return 0;
    }

    while (fp) {
	/* If the function cannot be translated into a Fortran 9x interface,
	   return */

	/* Check each function for valid defintion, including distinct
	   argument names if shortargnames is not selected */
	DBG2("Check if function %s is valid for a f90 interface\n",
	     fp->origName);
	if (F90FuncCheck(fp)) {
	    return 0;
	}
	f90ModuleOutput(fmodout, fp);

	fp = fp->next;

    }
    /* Output end-of-module */
    if (!f90mod_skip_header) {
	OutputFortranToken(fmodout, 0, "end interface");
	OutputFortranToken(fmodout, 0, "\n");
	OutputFortranToken(fmodout, 0, "end module");
	OutputFortranToken(fmodout, 0, "\n");
    }
    fclose(fmodout);
    fmodout = 0;
    return 0;
}

int F90FuncCheck(func_t *fp)
{
#if 0
    arg_t *ap = fp->arglist;
#endif

    /* If any argument is a void *, there is no corresponding Fortran
       interface (before F2013?) */
    /* We check for both void * arguments (for which there is no
       corresponding Fortran interface (before F2013?) and optionally
       for argument names that differ only in case */
    if (checkValidForF90(fp, !useShortNames)) {
	return 1;
    }
#if 0
    while (ap) {
	if (strcmp(ap->baseType, "void") == 1) {
	    /* FIXME: Message? */
	    return 1;
	}
	if (!useShortNames) {
	    /* Check that this name is unique if case-independant.
	       If not, do not process this function and return 1 */
	    if (checkValidForF90(fp)) {
		return 1;
	    }
	}
	ap = ap->next;
    }
#endif

    return 0;
}

int f90ModuleOutput(FILE *fout, func_t *fp)
{
    arg_t *ap;
    const char *token;
    char sname[2];
    int  argnum;

    DBG2("Generate f90 module interface for %s\n", fp->origName);

    /* Generate the interface for this routine */
    ap = fp->arglist;

    curCol = 0;
    /* Output the function definition */
    if (useFerr) {
	token = "subroutine";
    } else {
	token = fp->isFunc ? "function" : "subroutine";
    }
    OutputFortranToken(fout, 6, token);
    OutputFortranToken(fout, 1, fp->origName);
    OutputFortranToken(fout, 0, "(");

    argnum   = 0;
    sname[1] = 0;
    while (ap) {
	if (useShortNames) {
	    sname[0] = 'a' + (char)argnum++;
	    OutputFortranToken(fout, 0, sname);
	}
	else {
	    OutputFortranToken(fout, 0, ap->origName);
	}
	if (ap->next)
	    OutputFortranToken(fout, 0, ",");
	ap = ap->next;
    }
    if (useFerr) {
	if (argnum > 0) OutputFortranToken(fout, 0, ",");
	OutputFortranToken(fout, 0, errArgNameParm);
    }
    OutputFortranToken(fout, 0, ")");
    OutputFortranToken(fout, 0, "\n");

    /* Output any "use module" statement */
    if (f90usemodule) {
	OutputFortranToken(fout, 7, "use");
	OutputFortranToken(fout, 1, f90usemodule);
	OutputFortranToken(fout, 0, "\n");
    }

    /* Output argument declarations */
    ap     = fp->arglist;
    argnum = 0;
    while (ap) {
	/* Output corresponding Fortran type */
	DBG2("F90 module: decl for type %s\n", ap->baseType);
	OutputFortranToken(fout, 7, ArgToFortran(ap->baseType));
	if (useShortNames) {
	    sname[0] = 'a' + (char)argnum++;
	    /* FIXME: Handle short name */
	    OutputFortranToken(fout, 1, sname);
	}
	else {
	    OutputFortranToken(fout, 1, ap->origName);
	}
	/* FIXME: Handle arrays - default is (*) */
	/* FiXME: Handle pointers to functions */
	OutputFortranToken(fout, 1, "!");
	OutputFortranToken(fout, 1, ap->baseType);
	OutputFortranToken(fout, 0, "\n");
	ap = ap->next;
    }

    /* Add a "decl/result(name) for functions */
    if (useFerr) {
	OutputFortranToken(fout, 7, "integer");
	OutputFortranToken(fout, 1, errArgNameParm);
    } else if (fp->isFunc) {
	OutputFortranToken(fout, 7, ArgToFortran(fp->typeStr));
	OutputFortranToken(fout, 1, fp->origName);
	OutputFortranToken(fout, 1, "!");
	OutputFortranToken(fout, 1, fp->typeStr);
    }
    OutputFortranToken(fout, 0, "\n");

    OutputFortranToken(fout, 6, "end");   /* 6 space to match function def */

    if (useFerr) {
	token = "subroutine";
    } else {
	token = fp->isFunc ? "function" : "subroutine";
    }
    OutputFortranToken(fout, 1, token);
    OutputFortranToken(fout, 1, fp->origName);
    OutputFortranToken(fout, 0, "\n");

    return 0;
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
static int inComment = 0;
static void OutputFortranToken(FILE *fout, int nsp, const char *token)
{
    int tokenLen = (int)strlen(token);
    int i;

    if (curCol + nsp > maxOutputCol) nsp = 0;
    for (i=0; i<nsp; i++) putc(' ', fout);
    curCol += nsp;
    if (curCol + tokenLen > maxOutputCol) {
	while (curCol < 72) {
	    putc(' ', fout);
	    curCol ++;
	}
	/* We continue a comment in a different way */
	if (inComment) {
	    putc('\n', fout);
	    putc('!', fout);
	    putc(' ', fout);
	    curCol = 2;
	}
	else {
	    putc('&', fout);
	    putc('\n', fout);
	    for (i=0; i<5; i++) putc(' ', fout);
	    putc('&', fout);
	    curCol = 6;
	}
    }
    if (curCol == 0 && (*token != 'C' || *token != 'c')) {
	/* Skip past column 6 to support free and fixed format */
	for (i=0; i<6; i++) putc(' ', fout);
	curCol = 7;
    }
    fputs(token, fout);
    curCol += tokenLen;
    if (*token == '\n') {
	curCol    = 0;
	inComment = 0;
    }
    else if (*token == '!') {
	inComment = 1;
    }

    /* To aid in debugging, flush the output to the file */
    if (Debug) fflush(fout);
}

# if 0
/* This routine ensures that all arguments are distinct, even when case is
   not considered.  Specifically, if both "m" and "M" are argument names,
   the "M" argument will be replaced with "M$1" */
static void FixupArgNames(int nargs, ARG_LIST *args)
{
    int i, j;
    char tmpbuf[MAX_LINE];
    char *c, *cout;

    for (i=0; i<nargs; i++) {
	int hasUpper = 0;
	/* printf("Trying to fix %s\n", args[i].name); */
	/* Produce a lower-case version of the name */
	c    = args[i].name;
	cout = tmpbuf;
	while (*c) {
	    *cout = tolower(*c);
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
		    /* printf("Matched %s to %s\n", args[i].name,
		       args[j].name); */
		    cout = tmpbuf;
		    while (*cout) cout++;
		    if (cout - tmpbuf > MAX_LINE - 6) {
			fprintf(stderr, "Argument name %s too long\n", tmpbuf);
			ABORT("");
		    }
		    *cout++ = 'u';
		    *cout++ = 'p';
		    *cout++ = 'p';
		    *cout++ = 'e';
		    *cout++ = 'r';
		    *cout++ = 0;
		    if (args[i].name) {
			FREE(args[i].name);
		    }
		    args[i].name = (char *)MALLOC(strlen(tmpbuf) + 1);
		    if (MPIU_Strncpy(args[i].name, tmpbuf, strlen(tmpbuf) + 1)) {
			ABORT("Cannot replace argument name");
		    }
		    break;
		}
	    }
	}
    }
}


/* **** OLD CODE **** */
/*
 * Create a Fortran 90 definition (module) for a function
 */
void PrintDefinition(FILE *fout, int is_function, char *name, int nstrings,
		      int nargs, ARG_LIST *args, TYPE_LIST *types,
		      RETURN_TYPE *rt)
{
    int  i;
    char *token = 0;
    char sname[2];      /* Use for short argnames, if enabled */

    sname[1] = 0;       /* sname is a single character name */

    /*
     * Initial setup.  Fortran is case-insensitive and C is case-sensitive
     * Check that the case-insensitive argument names are distinct, and
     * if not, replace them with ones that are.  The rule is to
     * take a lowercase name and add "$1" to it.  We warn if a variable name
     * includes $1 ($ is permitted in Fortran names (check))
     */
    FixupArgNames(nargs, args);

    /* Check for void * args.  If found, we don't generate an interface,
       since we can't (at least until F2008+extensions for MPI) */
    for (i=0; i<nargs; i++) {
	/* Look for problems types */
	if (types[args[i].type].is_void ||
	    (strcmp(types[args[i].type].type,"void") == 0 &&
	     args[i].void_function == 0)) {
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
    OutputFortranToken(fout, 6, token);
    OutputFortranToken(fout, 1, name);
    OutputFortranToken(fout, 0, "(");
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
	    OutputFortranToken(fout, 0, args[i].name);
	    OutputFortranToken(fout, 0, ", ");
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
	    OutputFortranToken(fout, 0, sname);
	}
	else {
	    OutputFortranToken(fout, 0, args[nargs-1].name);
	    OutputFortranToken(fout, 0, " ");
	}
    }
    if (useFerr) {
	if (nargs > 0) OutputFortranToken(fout, 0, ",");
	OutputFortranToken(fout, 0, errArgNameParm);
    }
    OutputFortranToken(fout, 0, ")");
    OutputFortranToken(fout, 0, "\n");

    for (i=0; i<nargs; i++) {
	/* Figure out the corresponding Fortran type */
	if (types[args[i].type].is_mpi) {
	    OutputFortranToken(fout, 7, "integer");
	}
	else if (args[i].void_function) {
	    OutputFortranToken(fout, 7, "external");
	}
	else {
	    OutputFortranToken(fout, 7,
				ArgToFortran(types[args[i].type].type));
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
	    OutputFortranToken(fout, 1, sname);  /* space needed here */
	}
	else {
	    OutputFortranToken(fout, 1, args[i].name);
	}
	if (args[i].has_array && !args[i].void_function) {
	    OutputFortranToken(fout, 1, "(*)");
	}
	OutputFortranToken(fout, 1, "!");
	if (args[i].void_function) {
	    OutputFortranToken(fout, 1, "void function pointer");
	}
	else {
	    OutputFortranToken(fout, 1, types[args[i].type].type);
	}
	OutputFortranToken(fout, 0, "\n");
# if 0
	if (args[i].is_FILE) {
	    OutputFortranToken(fout, 0, "integer");
	    OutputFortranToken(fout, 1, args[i].name);
	}
	else if (!args[i].is_native && args[i].has_star
		 && !args[i].void_function) {
	    if (args[i].has_star == 1 || !MultipleIndirectAreInts)
		OutputFortranToken(fout, 0,
			 ToCPointer(types[args[i].type].type, args[i].name,
				     args[i].implied_star));
	    else {
		if (MultipleIndirectsAreNative) {
		    OutputFortranToken(fout, 0, args[i].name);
		}
		else {
		    OutputFortranToken(fout, 0, "(");
		    OutputFortranToken(fout, 0, types[args[i].type].type);
		    OutputFortranToken(fout, 0, " ");
		    if (!args[i].implied_star)
			for (j = 0; j<args[i].has_star; j++) {
			    OutputFortranToken(fout, 0, "*");
			}
		    OutputFortranToken(fout, 0, ")*((int *)");
		    OutputFortranToken(fout, 0, args[i].name);
		    OutputFortranToken(fout, 0, ")");
		}
	    }
	}
	else {
	    /* if args[i].has_star, the argument often has intent OUT
	       or INOUT */
	    OutputFortranToken(fout, 0, args[i].name);
	}
#endif
    }

    /* Add a "decl/result(name) for functions */
    if (useFerr) {
	OutputFortranToken(fout, 7, "integer");
	OutputFortranToken(fout, 1, errArgNameParm);
    } else if (is_function) {
	OutputFortranToken(fout, 7, ArgToFortran(rt->name));
	OutputFortranToken(fout, 1, name);
	OutputFortranToken(fout, 1, "!");
	OutputFortranToken(fout, 1, rt->name);
    }
    OutputFortranToken(fout, 0, "\n");
    OutputFortranToken(fout, 7, "end");

    if (useFerr) {
	token = "subroutine";
    } else {
	token = is_function ? "function" : "subroutine";
    }
    OutputFortranToken(fout, 1, token);
    OutputFortranToken(fout, 0, "\n");
}

#endif

/* FIXME: Replace with lookup in yet another table, with defaults in
 * the bfort-base.txt file.
 */
/*
 * This is a simple, temporary version of a routine to take a C type
 * (by name) and return the Fortran equivalent.
 */
static const char *ArgToFortran(const char *typeName)
{
    const char *outName = 0;

    DBG2("ArgToFortran for '%s'\n", typeName);

    if (strcmp(typeName, "int") == 0) outName = "integer";
    else if (strcmp(typeName, "char") == 0) outName   = "character";
    else if (strcmp(typeName, "double") == 0) outName = "double precision";
    else if (strcmp(typeName, "float") == 0) outName  = "real";
    else if (strcmp(typeName, "short") == 0) outName  = "integer*2";
    /* The following is a temporary hack */
    else if (strcmp(typeName, "void") == 0) outName = "PetscVoid";
    else {
	if (!useUserTypes) {
	    outName = "integer";
	} else {
	    outName = typeName;
	}
    }
    return outName;
}

typedef struct arghash_t {
    arg_t *ap;
    struct arghash_t *next;
} arghash_t;

#define HASH_SIZE 512
static arghash_t *arghash[HASH_SIZE];

/*
 * Routine to check that the function can be processed
 */
int checkValidForF90(func_t *fp, int checkNames)
{
    arg_t *ap = fp->arglist;
    int i;
    char *p;
    arghash_t *hptr, *hnew;
    int       rc = 0;

    DBG("Entering checkValidForF90\n");

    for (i=0; i<HASH_SIZE; i++) {
	arghash[i] = 0;
    }

    while (ap) {
	int hf = 0;

	DBG2("Checking arg %s for void *\n", ap->origName);
	/* Check for void * type */
	if (strcmp(ap->baseType, "void") == 0 && ap->isAddress) {
	    fprintf(stderr, "Function %s has a void * argument and no Fortran interface can be generated\n", fp->origName);
	    rc = 1;
	    goto cleanup;
	}

	if (checkNames) {
	    DBG2("Checking that arg %s is unique regardless of case\n",
		 ap->origName);
	    /* compute hash of argument name */
	    p = ap->origName;
	    while (*p) {
		if (*p < ' ') {
		    /* Skip this character */
		    fprintf(stderr, "Name has non-printing character\n");
		}
		else
		    hf += *p++ - ' ';
	    }
	    hf = hf % HASH_SIZE;

	    DBG2("Hash index = %d\n", hf);

	    /* Look for a match (independent of case) in the hash table.
	       If found, return with failure.  Otherwise, add to hash table.
	    */
	    hptr = arghash[hf];
	    while (hptr) {
		char *p1 = ap->origName, *p2 = hptr->ap->origName;

		DBG2("Found entry in hash table for %s\n", p2);
		while (*p1 && *p2) {
		    if (tolower(*p1) != tolower(*p2)) break;
		    p1++; p2++;
		}
		if (*p1 == 0 && *p2 == 0) {
		    fprintf(stderr, "Function %s has arguments %s and %s\n", fp->origName,
			    ap->origName, hptr->ap->origName);
		    fprintf(stderr, "Fortran is case-insensitive, so a valid Fortran interface for this function cannot be generated\n");
		    rc = -1;
		    goto cleanup;
		}
		hptr = hptr->next;
	    }

	    /* Insert at the beginning */
	    hnew        = (arghash_t *)malloc(sizeof(arghash_t));
	    hnew->ap    = ap;
	    hnew->next  = arghash[hf];
	    arghash[hf] = hnew;
	    DBG2("Inserting entry for arg %s\n", ap->origName);
	}
	ap = ap->next;
    }

cleanup:
    /* Free the hash entries */
    for (i=0; i<HASH_SIZE; i++) {
	hptr = arghash[i];
	while (hptr) {
	    hnew = hptr->next;
	    free(hptr);
	    hptr = hnew;
	}
    }
    DBG("Exiting checkValidForF90\n");
    return rc;
}
