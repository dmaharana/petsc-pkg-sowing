/*
   This file contains routines to handle the "newenvironment" command
 */

#include <stdio.h>
#include "sowing.h"
#include "tex.h"
#include "search.h"

typedef struct _NewEnv {
    int      nargs;
    char     *btext,      /* Text at begining */
             *etext;      /* Text at ending */
    } NewEnv;

static SRList *newenv = 0;
static int DebugEnv = 0;
static int LatexQuiet = 0;

void AddCodeDefn ( FILE * );

void TXSetLatexQuiet( flag )
int flag;
{
    LatexQuiet = flag;
}

/*
   A newenvironment command has the form

   \newenvironment{name}[nargs]{btext}{etext}

   where [nargs] is optional

   See also TXDoNewtheorem
 */
void TXDoNewenvironment( TeXEntry *e )
{
    char *nametok, *btext, *etext, args[10];
    int    nargs, d;
    LINK   *l;
    NewEnv *new;

/* Get name */
    nametok = (char *)MALLOC( MAX_TOKEN );  CHKPTR(nametok);
    TeXGetArg( fpin[curfile], nametok, MAX_TOKEN );

/* Check for [nargs] */
    nargs = TeXGetGenArg( fpin[curfile], args, 10, '[', ']', 1 );	
    if (nargs == -1) 
	TeXAbort( "TXDoNewenvironment", e->name );
    if (nargs) {
	nargs = atoi( args );
    }
/* Get begin text */
    btext = (char *)MALLOC( MAX_TOKEN );  CHKPTR(btext);
    if (TeXGetGenArg( fpin[curfile], btext, MAX_TOKEN, '{', '}', 0 ) == -1)
	TeXAbort( "TXDoNewenvironment", e->name );

/* Get End text */
    etext = (char *)MALLOC( MAX_TOKEN );  CHKPTR(etext);
    if (TeXGetGenArg( fpin[curfile], etext, MAX_TOKEN, '{', '}', 0 ) == -1)
	TeXAbort( "TXDoNewenvironment", e->name );

    if (!newenv) newenv = SRCreate();
    l       = SRInsert( newenv, nametok, 0, &d );
    new     = NEW(NewEnv);
    new->btext = btext;
    new->etext = etext;
    new->nargs = nargs;
    l->priv    = (void *)new;
}

/* Set an environment's begin and end text.  If btext or etext is null, 
   ignore it.  If name is not known, insert it */
void TXSetEnv( char *name, char *btext, char *etext )
{
    int    d;
    LINK   *l;
    NewEnv *new;

    if (!newenv) newenv = SRCreate();
    
    l       = SRLookup( newenv, name, (char *)0, &d );
    if (!l) {
      l          = SRInsert( newenv, name, 0, &d );
      new        = NEW(NewEnv);
      new->btext = 0;
      new->etext = 0;
      new->nargs = 0;
      l->priv    = (void *)new;
    }
    else 
      new = (NewEnv *)(l->priv);

    if (btext) new->btext = btext;
    if (etext) new->etext = etext;
}

/*
   \newtheorem{name}{title}[countername]

   as in

   \newtheorem{example}{Example}[chapter]
 
   which should generate
   Example 3.14
   for the 14th example in chapter 3.

   This creates a special environment
 */
void TXDoNewtheorem( TeXEntry *e )
{
    char   *nametok, *btext, *counter;
    int    d;
    LINK   *l;
    NewEnv *new;
    char tmpbuf[MAX_TOKEN];

/* Get name */
    nametok = (char *)MALLOC( MAX_TOKEN );  CHKPTR(nametok);
    TeXGetArg( fpin[curfile], nametok, MAX_TOKEN );

/* Get begin text */
    btext = (char *)MALLOC( MAX_TOKEN );  CHKPTR(btext);
    TeXGetGenArg( fpin[curfile], tmpbuf, MAX_TOKEN, '{', '}', 0 );
    strcpy( btext, "\\par{\\bf " );
    strcat( btext, tmpbuf );
/* really should do something here that allows us to process the counter */ 
    strcat( btext, "}" );

/* Get Counter name */
    counter = (char *)MALLOC( MAX_TOKEN );  CHKPTR(counter);
    TeXGetGenArg( fpin[curfile], counter, MAX_TOKEN, '[', ']', 0 );
    FREE(counter);

    if (!newenv) newenv = SRCreate();
    l       = SRInsert( newenv, nametok, 0, &d );
    new     = NEW(NewEnv);
    new->btext = btext;
    new->etext = 0;
    new->nargs = 0;
    l->priv    = (void *)new;
}

int LookupEnv( char *name, char **btext, char **etext, int *nargs )
{
    int    dummy;
    LINK   *l;
    NewEnv *env;

    l = SRLookup( newenv, name, (char *)0, &dummy );
    if (!l) return 0;

    env = (NewEnv *)l->priv;
    *btext = env->btext;
    *etext = env->etext;
    *nargs = env->nargs;
    return 1;
}

static int DebugDef  = 0;
void PushBeginEnv( char *btext, int nargs )
{
    char *(args[10]);
    int  i, argn;
    char *p;

    if (nargs > 0) {
	for (i=0; i<nargs; i++) {
	    args[i] = (char *)MALLOC( MAX_TOKEN );
	    TeXGetArg( fpin[curfile], args[i], MAX_TOKEN );
	}
    }


/* We must push the characters back in inverse order */
    if (btext) p = btext + strlen( btext ) - 1;
    else        p = 0;
    while (p && p >= btext) {
	if (p[-1] == '#' && p > btext) {
	    argn = p[0] - '1';
	    if (argn >= 0 && argn < nargs) {
		if (DebugDef) printf( "Pushing %s back in env eval(1)\n", 
				      args[argn] );
		SCPushToken( args[argn] );
		p--;
	    }
	    else {
		SCPushChar( *p );
		if (DebugDef) printf( "Pushing %c back in env eval(2)\n", *p );
	    }
	}
	else {
	    SCPushChar( *p );
	    if (DebugDef) printf( "Pushing %c back in environment eval(3)\n", *p );
	}
	p--;
    }

    for (i=0; i<nargs; i++) {
	FREE( args[i] );
    }
}

/*
   The intent of this routine is to run LaTeX on the given LaTeX code
   and include the output as an image in the file 

   The code is drawn from that in latex2html; it requires the ability to
   display gif files.

   This can handle either an environment (given by envname) or a short 
   command (given by string)
   
   Name is the basename of the file to use.

   Only run latex if runagain is true.  If runagain is FALSE, DO EAT the
   source code that would have been comsumed if runagain was true.
 */
/* kind is xbm or gif */
void RunLatex( char *envname, char *string, char *name, char *mathmode, 
	       char *kind, int runagain )
{
    TeXEntry E;
    char *p;
#ifndef __MSDOS__
    char pgm[256];
    char fname[100];
    char fdviname[256];
    char ext[10];
    char latex_errname[300];
    char *latex_pgm;
    FILE  *fp, *foutsave;
    int  problem_with_file = 0;
#endif

/* Set dummy values for E */
    E.name	 = envname;
    E.action = 0;
    E.ctx	 = 0;
    E.nargs	 = 0;

#ifndef __MSDOS__	
/* Write out header (documentstyle, header) */
    if (runagain) {
	sprintf( fname, "%s.tex", name );
	fp = fopen( fname, "w" );
	if (!fp) {
	    fprintf( ferr, "Could not open file %s for LaTeX processing\n",
		     fname );
	    return;
	}
	fprintf( fp, "%% %s near line %d\n",
		 InFName[curfile] ? InFName[curfile]: "",
		 LineNo[curfile] );

	/* Make the page long enough that it will generate a single output page */
	fprintf( fp, "\
\\%s%s\n\
\\pagestyle{empty}\n\
\\thispagestyle{empty}\n\
\\vsize=20in\\setlength{\\textheight}{20in}\n\
\\begin{document}\n", 
documentcmd, preamble ? preamble : "{article}" );
/* Add the definition of \code and \file, just in case.  This really should
   be set if format is -slide, along with other slide items. */
	AddCodeDefn( fp );

	fprintf( fp, "\\setcounter{section}{1}\n\
\\setcounter{figure}{%d}\n\
\\setcounter{table}{%d}\n\
\\setcounter{equation}{%d}\n", FigureNumber, TableNumber, EquationNumber-1 );

	if (predoc && predoc[0]) 
	    fputs( predoc, fp );
	fprintf( fp, "%% User definitions begin here\n" );
	TXDumpUserDefs( fp, 0 );
	fprintf( fp, "%% User definitions end here\n" );

	/* Disable things like \index */
	fprintf( fp, "%% Disable other commands\n" );
	fprintf( fp, "\\def\\index#1{\\relax}\n" );

	if (string) {
	    fprintf( fp, "% User command to run\n" );
	    fputs( string, fp );
	    fprintf( fp, "% End of user command to run\n" );
	}
    }

    if (!runagain) {
	if (envname) {
	    p = (char *) MALLOC( strlen(envname) + 1 );  CHKPTR(p);
	    strcpy( p, envname );
	    TeXskipRaw( &E, p, 0 );
	}
	else if (mathmode) {
	    p = (char *) MALLOC( strlen(mathmode) + 1 );  CHKPTR(p);
	    strcpy( p, mathmode );
	    TeXskipMath( &E, p, 0 );
	}
	FREE( p );
    }
    else {
	if (envname) {
	    int SaveInArg;
	    fprintf( fp, "%% Beginning of Environment to be handled\n" );
	    fprintf( fp, "\\begin{%s}\n", envname );
	    /* We'll start by using SkipEnv ... */
	    foutsave = fpout;
	    fpout    = fp;
	    /* Make sure that envname is private */
	    p = (char *) MALLOC( strlen(envname) + 1 );  CHKPTR(p);
	    strcpy( p, envname );
	    /* Turn off HTML translations */
	    DoOutputTranslation = 0;
	    /* Turn off active tokens */
	    DoActiveTokens = 0;
	    /* Turn off "In arg" processing */
	    SaveInArg = InArg;
	    InArg     = 0;
	    TeXskipRaw( &E, p, 1 );
	    InArg = SaveInArg;
	    /* Restore them */
	    DoOutputTranslation = 1;
	    DoActiveTokens = 1;
	    FREE( p );
	    fpout    = foutsave;
	    fprintf( fp, "\\end{%s}\n", envname );
	    fprintf( fp, "%% End of Environment to be handled\n" );
	}
	if (mathmode) {
	    fprintf( fp, "%s\n", mathmode );
	    /* We'll start by using SkipEnv ... */
	    foutsave = fpout;
	    fpout    = fp;
	    /* Make sure that mathmode is private */
	    p = (char *) MALLOC( strlen(mathmode) + 1 );  CHKPTR(p);
	    strcpy( p, mathmode );
	    /* Turn off HTML translations */
	    DoOutputTranslation = 0;
	    /* Turn off active tokens */
	    DoActiveTokens = 0;
	    TeXskipMath( &E, p, 1 );
	    /* Restore them */
	    DoOutputTranslation = 1;
	    DoActiveTokens = 1;
	    FREE( p );
	    fpout    = foutsave;
	    if (mathmode[1] == '[') fputs( "\\]\n", fp );
	    else if (mathmode[1] == '(') fputs( "\\)\n", fp );
	    else fputs( "$\n", fp );
	}

	fprintf( fp, "\\end{document}\n" );
	fclose( fp );

	/* Run Latex */
	/* Unfortunately, there are now TWO things named latex, and
	   they are incompatible.  Rather than force the path to
	   be correct, allow the user to override with the environment
	   variable TOHTML_LATEX
	   */
	latex_pgm = "latex";
	if ((p = getenv( "TOHTML_LATEX" ))) latex_pgm = p;
	GetBaseName( latex_errname );
	strcat( latex_errname, ".ler" );
	/* An alternative is 
	   strcpy( latex_errname, "/dev/null" ); */
	if (LatexQuiet) {
	    sprintf( pgm, "%s %s.tex >>%s 2>&1 </dev/null", 
		     latex_pgm, name, latex_errname );
	}
	else {
	    sprintf( pgm, "%s %s.tex", latex_pgm, name );
	}

	/* Should check return status */
	if (system( pgm )) {
	    /* Something went wrong */
	    fprintf( ferr, "Latex command %s %s.tex returned nonzero\n", 
		     latex_pgm, name );
	    problem_with_file = 1;
	}
	/* Some dvips send to the PRINTER by default! */
	/* Add -q to make quiet */
	
	if (LatexQuiet) {
	    sprintf( pgm, "dvips %s -o %s.ps >>%s 2>&1", 
		     name, name, latex_errname );
	}
	else {
	    sprintf( pgm, "dvips %s -o %s.ps", name, name );
	}
	strcpy( fdviname, name );
	strcat( fdviname, ".dvi" );
	if (!SYiFileExists( fdviname, 'r' )) {
	    /* Could not find the dvi file */
	    problem_with_file = 1;
	    fprintf( ferr, "No DVI file created for %s.tex\n", name );
	}
        else {
	    system( pgm );
	    /* Add "figure" as third argument to get color output */
	    /* Could use pstoxbm instead */
	    if (strcmp( kind, "gif" ) == 0) {
		if (LatexQuiet)
		    sprintf( pgm, "%spstogif %s.ps %s.gif >>%s 2>&1", 
			     PSPATH, name, name, latex_errname );
		else
		    sprintf( pgm, "%spstogif %s.ps %s.gif", PSPATH, name, name );
		strcpy( ext, "gif" );
	    }
	    else {
		if (LatexQuiet) 
		    sprintf( pgm, "%spstoxbm %s.ps %s.xbm >>%s 2>&1", 
			     PSPATH, name, name, latex_errname );
		else
		    sprintf( pgm, "%spstoxbm %s.ps %s.xbm", PSPATH, name, name );
		strcpy( ext, "xbm" );
	    }
	    system( pgm );
	    /* Note that the result of the run might be name%d.ext, 
	       for example img2101.xbm and img2102.xbm, instead of img201.xbm
	    */
	    if (splitlevel >= 0) {
		sprintf( pgm, "/bin/mv %s.%s %s", name, ext, splitdir );
		/* The mv sometimes fails... */
		fprintf( stdout, "%s\n", pgm );
		system ( pgm );
	    }
	}
	sprintf( pgm, "/bin/rm -f %s.dvi %s.ps %s.aux %s.log", 
		 name, name, name, name );
	system( pgm );
	if (!problem_with_file) {
	    sprintf( pgm, "/bin/rm -f %s.tex", name );
	    system( pgm );
	}
    }
#else
/* Just skip the environment */
    if (envname) {
	p = (char *) MALLOC( strlen(envname) + 1 );  CHKPTR(p);
	strcpy( p, envname );
	TeXskipRaw( &E, p, 0 );
    }
    else if (mathmode) {
	p = (char *) MALLOC( strlen(mathmode) + 1 );  CHKPTR(p);
	strcpy( p, mathmode );
	TeXskipMath( &E, p, 0 );
    }
    FREE( p );
#endif
}


/*
   When entering a numbered environment, set the type and number. 

   These are in the global variable envJumpNum and in the returned string
   envname.
 */
void TXStartNumberedEnv( envname )
char *envname;
{
    int  envnum;

    envname[0] = 0;
    switch (NumberedEnvironmentType) {
    case ENV_NONE: break;
    case ENV_TABLE:
        envnum = ++TableNumber;
        strcpy( envname, "Table" );
        break;
    case ENV_FIGURE:
        envnum = ++FigureNumber;
        strcpy( envname, "Figure" );
        break;
    case ENV_EQUATION:
        envnum = ++EquationNumber;
        strcpy( envname, "Equation" );
        break;
    }

    envJumpNum = envnum;
}

/*
   For figures and tables, a caption command should

   Increment the figure etc number

   For figure and table environments handled by using Latexmode, I need
   to capture the \label command, if any, and keep the info.
 */
void TXcaptionHandling( e )
TeXEntry *e;
{
    char envname[20];

    envname[0] = 0;
    TXStartNumberedEnv( envname );
    if (InDocument && envname[0]) {
	if (!envjumpname) 
	    envjumpname = (char *)MALLOC( 1024 );
	CHKPTR(envjumpname);
	sprintf( envjumpname, "%s#%s%d", outfile, envname, envJumpNum );
    }
}

void TXcaption( e )
TeXEntry *e;
{
    char *caption;
    char envname[20];

    caption = (char *)MALLOC( MAX_TOKEN );  CHKPTR(caption);
    envname[0] = 0;
    TXStartNumberedEnv( envname );
    TXbgroup( e );
    TeXGetArg( fpin[curfile], caption, MAX_TOKEN );
    if (InDocument) {
	TXWriteStartNewLine( fpout );
	if (envname[0]) {
	    char buf[40];
	    sprintf( buf, "%s %d: ", envname, envJumpNum );
	    TXbgroup( e );
	    TXbf( e );
	    TeXoutstr( fpout, buf );
	    TXegroup( e );
	    if (!envjumpname) 
		envjumpname = (char *)MALLOC( 1024 );
	    CHKPTR(envjumpname);
	    sprintf( envjumpname, "%s%d", envname, envJumpNum );
	    WriteJumpDestination( fpout, envjumpname, caption );
	    sprintf( envjumpname, "%s#%s%d", outfile, envname, envJumpNum );
	}
	else 
	    TeXoutstr( fpout, caption );
    }
    TXWritePar( fpout );
    TXegroup( e );
    FREE( caption );
/* End the anchor */
}

void TeXSetEnvJump( envname )
char *envname;
{
    if (envname[0]) {
	if (!envjumpname) 
	    envjumpname = (char *)MALLOC( 1024 );
	CHKPTR(envjumpname);
	sprintf( envjumpname, "%s#%s%d", outfile, envname, envJumpNum );
    }
}

void AddCodeDefn( fout )
FILE *fout;
{
    fprintf( fout, "{\\catcode`\\_=\\active\\gdef_{{\\tt\\char`\\_}}}\n\
{\\catcode`\\_=\\active\\gdef\\makeustext{\\def_{{\\tt\\char`\\_}}}}\n\
{\\catcode`\\&=\\active\\gdef\\makeamptext{\\catcode`\\&=\\active\\def&{{\\tt\\char`\\&}}}}\n\
\\def\\eatcode{\\catcode`\\_=\\active\\makeustext\\makeamptext\\eatnext}\n\
\\def\\eatfile{\\catcode`\\_=\\active\\makeustext\\makeamptext\\eatnextfile}\n\
\\def\\eatnext#1{{\\tt #1}\\endgroup}\n\
\\def\\eatnextfile#1{`{\\tt #1}'\\endgroup}\n\
\\def\\file{\\begingroup\\eatfile}\n\
\\def\\code{\\begingroup\\eatcode}\n" );
}
