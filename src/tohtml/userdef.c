/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
   This file contains routines to process user defined commands

   TeX uses a template macro processor; this means that we must be prepared 
   to skip over text in the macro definition.
 */

#include <stdio.h>
#include "sowing.h"
#include "search.h"
#include "tex.h"

static char  ltoken[MAX_TOKEN];
static char  ldef[MAX_TOKEN];
static int   DebugDef = 0;

typedef enum { TX_TEXCMD, TX_HTMLCMD } TX_Entry_t;
typedef struct {
    int      nargs;
    char     *replacement_text;
    char     *input_template;       /* for \def\MPI/{{\sf MPI}} etc */
    TX_Entry_t kind;      /* Identify html command replacments */
    } Definition;

void TXDebugDef( int flag )
{
    DebugDef = flag;
}

/* Processes a USE of a user-defined command */
/* Reduce mallocs by predefining at least the array of args */
#define MAX_ARGS 16

void TXDoUser( TeXEntry *e )
{
    Definition *def;
    char       *p, *pstart;
    int        j, nargs, argn, rc;
    int        ch;
    char       **args;
    char       *template;

    def = (Definition *)(e->ctx);

    nargs = def->nargs;
    if (nargs > 0) {
	args  = (char **)MALLOC( nargs * sizeof(char *) );  CHKPTR(args);
    }
    if (DebugDef) {
	printf( "Handling user-defined command with %d arguments\n", nargs );
    }
/* Don't evaluate the arguments until we push them back ... */
    template = def->input_template;
    if (template) {
	if (DebugDef && template[0]) {
	    printf( "Template string is %s\n", template );
	}
	while (template[0] && template[0] != ArgChar) {
	    ch = SCTxtGetChar( fpin[curfile] );
	    if (ch == template[0]) {
		if (DebugDef) printf( "Skipping character %c in template\n", ch );
		template++;
	    }
	    else break;
	}
    }
    for (j=0; j<nargs; j++) {
	while (template && template[0] && template[0] != ArgChar) {
	    ch = SCTxtGetChar( fpin[curfile] );
	    if (ch == template[0]) template++;
	    else break;
	}
	if (template && template[0] == ArgChar) {
	    /* skip over #n */
	    template += 2;
	}
	if (DebugDef) printf( "Reading arg %d\n", j );
	rc = TeXGetGenArg( fpin[curfile], ltoken, MAX_TOKEN, 
			   LbraceChar, RbraceChar, 0 );
	if (rc == -1) {
	    TeXAbort( "TXDoUser", e->name );
	}
	if (rc == 0) {
	    /* If there is no {...}, then read exactly one character */
	    ltoken[0] = SCTxtGetChar( fpin[curfile] );
	    ltoken[1] = 0;
	    if (DebugDef) printf( "Read a single %c character\n", ltoken[0] );
	}
	if (DebugDef) printf( "Argument %d is :%s:\n", j, ltoken );

	args[j] = (char *)MALLOC( strlen( ltoken ) + 1 );  CHKPTR(args[j]);
	strcpy( args[j], ltoken );
    }
    pstart = def->replacement_text;
/* We must push the characters back in inverse order */
    if (pstart) p = pstart + strlen( pstart ) - 1;
    else        p = 0;
    while (p && p >= pstart) {
	if (p[-1] == ArgChar && p > pstart) {
	    argn = p[0] - '1';
	    if (argn >= 0 && argn < nargs) {
		if (DebugDef) printf( "Pushing %s back in def eval(1)\n", 
				      args[argn] );
		SCPushToken( args[argn] );
		p--;
	    }
	    else {
		SCPushChar( *p );
		if (DebugDef) printf( "Pushing %c back in def eval(2)\n", *p );
	    }
	}
	else {
	    SCPushChar( *p );
	    if (DebugDef) printf( "Pushing %c back in def eval(3)\n", *p );
	}
	p--;
    }

    for (j=0; j<nargs; j++) {
	FREE( args[j] );
    }
    if (nargs > 0) {
	FREE( args );
    }
}

/* Add "name" as a user-defined command.  We've read the \def only */
static char name[128];

void TXAddUserDef( SRList *TeXlist, TeXEntry *e )
{
    int        nargs, ch, nsp, nbrace, j, i;
    char       *p;
    Definition *def;
    char       *ltoken;
    char       *ldef;
    char       lname[40];
    char       template_buf[100];

    ltoken    = (char *)MALLOC( MAX_TOKEN ); CHKPTR(ltoken);
    ldef      = (char *)MALLOC( MAX_TOKEN ); CHKPTR(ldef);

    ch = TeXReadToken( name, &nsp );
    if (ch != CommandChar) {
	fprintf( ferr, "Expected a %c (TeX command char)\n", CommandChar );
	SCPushToken( name );
	FREE( ltoken );
	FREE( ldef );
	return;
    }

    if (DebugDef) {
	printf( "Defining %s\n", name );
    }
/* Get the arguments by looking for #n upto brace */

    nargs = 0;

    template_buf[0] = 0;
    while ( (ch = SCTxtFindNextANToken( fpin[curfile], ltoken, MAX_TOKEN, &nsp ))
	    != EOF ) {
	if (ch == LbraceChar) break;
	if (ch == ArgChar) nargs++;
	for (i=0; i<nsp; i++) 
	    strncat( template_buf, " ", 100 );
	strncat( template_buf, ltoken, 100 );
    }
    nbrace = 1;
    p      = ldef;
    while (nbrace > 0) {
	ch = SCTxtFindNextANToken( fpin[curfile], ltoken, MAX_TOKEN, &nsp );
	if (ch == EOF) break;
	for (j=0; j<nsp; j++) *p++ = ' ';
	if (ch == CommentChar) continue;
	if (ch == '\n') { ch = ' '; ltoken[0] = ' '; }
	if (ch == LbraceChar) nbrace++;
	else if (ch == RbraceChar) nbrace--;
	j = 0;
	while (ltoken[j]) *p++ = ltoken[j++];
    }
/* The last (p-1) should have been a closing right-brace */
    if (p > ldef && p[-1] == RbraceChar)
	p[-1] = 0;
    if (SRLookup( TeXlist, name+1, lname, &j )) {
	if (warnRedefinition)  {
	    fprintf( ferr, "Attempt to redefine %s; new definition discarded\n", name );
	    fprintf( ferr, "  (Redefinitions may cause problems with the translator)\n" );
	}
    }
    else {
	/* Add to known commands */
	def = NEW(Definition);  CHKPTR(def);
	def->nargs = nargs;
	if (DebugDef) {
	    printf( "User provided defintion of %s is:\n\ttemplate = %s\n\tdef = %s\n", 
		    name+1, *template_buf ? template_buf : "<null>", 
		    *ldef ? ldef : "<null>" );
        }
	def->replacement_text = (char *)MALLOC( strlen( ldef ) + 1 );
	CHKPTR(def->replacement_text);
	strcpy( def->replacement_text, ldef );

	def->input_template = (char *)MALLOC( strlen( template_buf ) + 1 );
	CHKPTR(def->input_template);
	strcpy( def->input_template, template_buf );

	TXInsertName( TeXlist, name+1, TXDoUser, nargs, (void *)def );
    }
    FREE( ltoken );
    FREE( ldef );
}

/* 
   This is for \renewcommand and \newcommand; these have a different format
   than \def but need to have the same effect.  Their format is

   \renewcommand{name}[nargs]{replacement-text}

   if [nargs] is missing, it means [0].
 */
void TXDoNewCommand( SRList *TeXlist, TeXEntry *e )
{
    int        nargs, ch, nsp, nbrace, j;
    char       *p;
    Definition *def;
    char       lname[40];

/* We can't use GetArg to get the arg, because we need to skip over the 
   \ in the name.  Fortunately, the argument MUST be a simple name */
    ch = SCTxtGetChar( fpin[curfile] );
    if (ch != LbraceChar) {
	fprintf( ferr, "Expected a command name in {...}\n" );
	return;
    }
    nbrace = 1;
    p      = name;
    while (nbrace > 0) {
	ch = SCTxtFindNextANToken( fpin[curfile], ltoken, MAX_TOKEN, &nsp );
	if (ch == EOF) break;
	for (j=0; j<nsp; j++) *p++ = ' ';
	if (ch == LbraceChar) nbrace++;
	else if (ch == RbraceChar) nbrace--;
	if (nbrace == 0) { 
	    *p = 0;
	    break;
	}
	j = 0;
	while (ltoken[j]) *p++ = ltoken[j++];
    }
/* The last (p-1) should have been a closing right-brace */
    if (p > ldef && p[-1] == RbraceChar)
	p[-1] = 0;

/*
    if (DebugDef) {
	printf( "Defining %s with newcommand or renewcommand\n", name );
    }
 */
    nargs = TeXGetGenArg( fpin[curfile], ltoken, MAX_TOKEN, '[', ']', 1 );	
    if (nargs == -1) 
	TeXAbort( "TXDoUser", e->name );
    if (nargs) {
	nargs = atoi( ltoken );
    }
/* We get the definition without processing it */
    nbrace = 0;
    ch = SCTxtFindNextANToken( fpin[curfile], ltoken, MAX_TOKEN, &nsp );
    if (ch == LbraceChar) 
	nbrace++;
    else {
	SCPushToken( ltoken );
    }
    p      = ldef;
    while (nbrace > 0) {
	ch = SCTxtFindNextANToken( fpin[curfile], ltoken, MAX_TOKEN, &nsp );
	if (ch == EOF) break;
	for (j=0; j<nsp; j++) *p++ = ' ';
	if (ch == LbraceChar) nbrace++;
	else if (ch == RbraceChar) nbrace--;
	if (ch == CommentChar) continue;
	if (ch == '\n') { ch = ' '; ltoken[0] = ' '; }
	j = 0;
	while (ltoken[j]) *p++ = ltoken[j++];
    }
/* The last (p-1) should have been a closing right-brace */
    if (p > ldef && p[-1] == RbraceChar)
	p[-1] = 0;
/* Add to known commands */

    if (SRLookup( TeXlist, name+1, lname, &j )) {
	if (warnRedefinition)  {
	    fprintf( ferr, "Attempt to redefine %s; new definition discarded\n", name );
	    fprintf( ferr, "  (Redefinitions may cause problems with the translator)\n" );
	}
    }
    else {
	def = NEW(Definition);  CHKPTR(def);
	def->nargs = nargs;
	def->replacement_text = (char *)MALLOC( strlen( ldef ) + 1 );
	CHKPTR(def->replacement_text);
	strcpy( def->replacement_text, ldef );

	def->input_template = 0;
	if (DebugDef) {
	    printf( "Defining %s with newcommand or renewcommand\n", name );
	    printf( "Definition text is %s\n", ldef ? ldef : "<null>" );
	}
	
	TXInsertName( TeXlist, name+1, TXDoUser, nargs, (void *)def );
    }
}

/* Create a definition context suitable for TXInsertName */
void *TXCreateDefn( int nargs, char *ldef, int is_html )
{
    Definition *def;
    /* Add to known commands */
    def = NEW(Definition);  CHKPTRN(def);
    def->nargs = nargs;
    def->replacement_text = ldef;
    def->input_template = 0;
    def->kind = is_html ? TX_HTMLCMD : TX_TEXCMD;

    return (void *)def;
}

void TXDoNewLength( SRList *TeXlist, TeXEntry *e )
{
    int        ch, nsp, nbrace, j;
    char       *p;
    Definition *def;
    static char null_string[1] = {0};

/* We can't use GetArg to get the arg, because we need to skip over the 
   \ in the name.  Fortunately, the argument MUST be a simple name */
    ch = SCTxtGetChar( fpin[curfile] );
    if (ch != LbraceChar) {
	fprintf( ferr, "Expected a command name in {...}\n" );
	return;
    }
    nbrace = 1;
    p      = name;
    while (nbrace > 0) {
	ch = SCTxtFindNextANToken( fpin[curfile], ltoken, MAX_TOKEN, &nsp );
	if (ch == EOF) break;
	for (j=0; j<nsp; j++) *p++ = ' ';
	if (ch == LbraceChar) nbrace++;
	else if (ch == RbraceChar) nbrace--;
	j = 0;
	while (ltoken[j]) *p++ = ltoken[j++];
    }
/* The last (p-1) should have been a closing right-brace */
    if (p > ldef && p[-1] == RbraceChar)
	p[-1] = 0;

    def		      = NEW(Definition);  CHKPTR(def);
    def->nargs	      = 0;
    /* We need a pointer to null */
    def->replacement_text = null_string;
    def->input_template   = 0;
    TXInsertName( TeXlist, name+1, TXDoUser, 0, (void *)def );
}

/*
 * TeX allows you to capture a definition with \let<newcommand>=<oldcommand>
 * or \let<newcommand><oldcommand>
 */
void TXlet( TeXEntry *e )
{
    char       *ltoken;
    int        nsp, i, ch;
    TeXEntry   *old;
    LINK       *l;
    
    ltoken    = (char *)MALLOC( MAX_TOKEN ); CHKPTR(ltoken);

    /* Read command */
    ch = TeXReadToken( name, &nsp );
    if (ch != CommandChar) {
	fprintf( ferr, "Expected a %c (TeX command char)\n", CommandChar );
	SCPushToken( name );
	FREE( ltoken );
	return;
    }
    
    /* Read oldvalue */
    ch = TeXReadToken( ltoken, &nsp );
    if (ch == '=') 
	ch = TeXReadToken( ltoken, &nsp );

    /* Insert newcommand into list with old meaning */
    l = SRLookup( TeXlist, ltoken+1, ltoken+1, &i );
    if (l) {
	old = (TeXEntry *)(l->priv);
	TXInsertName( TeXlist, name + 1, old->action, old->nargs, old->ctx );
    }
    else {
	/* Insert as nop */
	TXInsertName( TeXlist, name + 1, TXnop, 0, (void *)0 );
    }
    FREE( ltoken );
    
    return;
}

/*
   Dump user-definitions in TeX format.  Use this (a) for debugging and (b)
   for generating LaTeX img files

   If the name contains an @, bracket with 
   \makeatletter ... \makeatother
   
   We must be careful to remove the quoting characters.

   We also need the option to skip definitions that provide TeX->HTML
   mappings.  These should not be output when generating LaTeX output
   to be used when creating gif files for unknown LaTeX environments
   and commands.
 */
void TXDumpUserDefs( FILE *fout, int include_hdef )
{
    LINK       *cur;
    TeXEntry   *e;
    Definition *d;
    int        i, j;

    for (i=0; i<TeXlist->hashsize; i++) {
	cur = TeXlist->HASH[i];
	while (cur) {
	    e = (TeXEntry *)(cur->priv);
	    if (e->action == TXDoUser) {
		d = (Definition *)(e->ctx);
		if (d->kind != TX_HTMLCMD || include_hdef) {
		    if (strchr( cur->topicname, '@' )) 
			fprintf( fout, "\\makeatletter\n" );
		    fprintf( fout, "\\def\\%s", cur->topicname );
		    if (d->input_template) 
			WriteString( fout, d->input_template );
		    else {
			for (j=0; j<d->nargs; j++) 
			    fprintf( fout, "#%d", j + 1 );
		    }
		    fputs( "{", fout );
		    WriteString( fout, d->replacement_text );
		    fputs( "}\n", fout );
		    if (strchr( cur->topicname, '@' )) 
			fprintf( fout, "\\makeatother\n" );
		}
	    }
	    cur = cur->next;
	}
    }
}

/* This code allows us to keep track of include files that we either want to
   skip or want to replace with a different file. */
static SRList *skipFileList = 0;
void TXAddSkipFile( const char *value )
{
    int n;
    LINK *elm;

    if (!skipFileList) skipFileList = SRCreate();
    elm = SRInsert( skipFileList, value, (char *)0, &n );
    if (elm) {
	FREE( elm->entryname );
	elm->entryname = 0;
    }
}
void TXAddReplaceFile( const char *value, const char *newname )
{
    int n;
    LINK *elm;
    if (!skipFileList) skipFileList = SRCreate();
    elm = SRInsert( skipFileList, value, (char *)0, &n );
    if (elm) {
	FREE( elm->entryname );
	elm->entryname = MALLOC( sizeof(newname)+1 );
	strcpy( elm->entryname, newname );
    }
}

int  TXIsSkipFile( const char *name )
{
    int n, rc = 0;
    LINK *elm;
    if (!skipFileList) return 0;
    elm = SRLookup( skipFileList, name, (char *)0, &n );
    if (elm) {
	if (elm->entryname && elm->entryname[0]) rc = 0;
	else rc = 1;
    }
    return rc;
}

int  TXIsReplaceFile( const char *name, char *newname )
{
    int n, rc = 0;
    LINK *elm;
    if (!skipFileList) return 0;
    newname[0] = 0;
    elm = SRLookup( skipFileList, name, (char *)0, &n );
    if (elm) {
	strcpy( newname, elm->entryname );
	rc = 1;
    }
    return rc;
}

#ifdef FOO
/* 
   Add a definition that may include commands (e.g., may contain raw html)
 */
void TXAddUserDefn( char *name, int nargs, char *value, char cmdchar )
{
  char *def, *p, *def_p;
  int  in_cmd = 0;

  def = (char *)malloc( strlen(value) + 1 );   CHKPTR(def);
  def_p = def;

  p = value;
  while (*p) {
    if (*p == cmdchar) {
      if (p[1] == cmdchar) {
	/* doubled cmdchars are treated as a single char */
	*def_p++ = *p;
	p        += 2;
      }
      else {
	*def_p++ = (in_cmd ? TOK_END : TOK_START);
	in_cmd = ! in_cmd;
      }
    }
    else 
      *def_p++ = *p++;
  }
  *def_p = 0;
  TXInsertName( TeXlist, name, TXDoUser, nargs, (void *)def );
}

#endif /* FOO */
