#include <stdio.h>
#include "sowing.h"
#include "search.h"
#include "tex.h"
#include <ctype.h>

/* extern TXdimen(), TXname(), TXasis() TXraw();*/

#define SKIPBLANK(bufp)\
	while (isspace(*bufp) && *bufp != '\n') bufp++;
#define SKIPNONBLANK(bufp) \
	while (*bufp && !isspace(*bufp)) bufp++;

/* 
   Convert a string that contains literal output quotes to the internal form
 */
char *TXConvertQuotes( char *value, char cmdchar )
{
  char *def, *p, *def_p;
  int  in_cmd = 0;

  def = (char *)MALLOC( strlen(value) + 1 );   CHKPTRN(def);
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
	p++;
      }
    }
    else 
      *def_p++ = *p++;
  }
  *def_p = 0;
  return def;
}


/*
    Read a file name containing definitions
 */
void RdBaseDef( infilename )
char *infilename;
{
    FILE *fp;
    char buf[200], *bufp, *name, *cmd, *value, *p;
    int  nargs;

    if (!TeXlist) 
	TeXlist = SRCreate();

    fp = fopen( infilename, "r" );
    if (!fp) {
	fprintf( stderr, "Could not open definition file %s\n", infilename );
	return;
    }
    while (fgets(buf, 199, fp)) {
	buf[199] = 0; /* Just in case */
	nargs = 0;   /* in case not set ... */
	name = 0; cmd = 0; value = 0;
	bufp = buf;
	/* We need to turn this into %s %s %d rest of line */
	/* sscanf( buf, "%s %s %d %s", cmd, name, &nargs, value ); */
	/* Get cmd */
	SKIPBLANK(bufp);
	if (*bufp == '\n' || *bufp == '#') continue;
	cmd = bufp;
	SKIPNONBLANK(bufp);
	if (*bufp == '\n') *bufp = 0;
	if (!*bufp) continue;  /* End of string before all found */
	*bufp++ = 0;  /* Null terminate cmd */

	/* Get name */
	SKIPBLANK(bufp);
	name = bufp;
	SKIPNONBLANK(bufp);
	if (*bufp == '\n') *bufp = 0;
	if (*bufp) *bufp++ = 0;

	/* Get nargs */
	SKIPBLANK(bufp);
	p = bufp;
	while (isdigit(*bufp)) bufp++;
	if (*bufp == '\n') *bufp = 0;
	if (*bufp) *bufp++ = 0;
	/* Only invoke atoi if there is an actual integer present */
	if (isdigit(*p)) 
	    nargs = atoi(p);

	/* Get value */
	SKIPBLANK(bufp);
	value = bufp;
	p     = value;
	while (*p && *p != '\n') p++;
	*p = 0;

	/* Skip null or blank lines */
	if (cmd[0] == 0 || cmd[0] == ' ') continue;
	
	/* If name is in quotes, remote the quotes */
	if (name[0] == '"') {
	    p = name+1;
	    while (*p && *p != '"') {
		p[-1] = p[0];
		p++;
	    }
	    *p = 0;
	}

	/* printf( "|%s| |%s| |%d| |%s|\n", cmd, name, nargs, value ); */

	/* Act on the command */
	if (strcmp( cmd, "nop" ) == 0) {
	    TXInsertName( TeXlist, name, TXnop, nargs, (void *)0 );
	}
	else if (strcmp( cmd, "dimen" ) == 0) {
	    TXInsertName( TeXlist, name, TXdimen, 0, (void *)0 );
	}
	else if (strcmp( cmd, "name" ) == 0) {
	    if (value[0] == '"') {
		p = value+1;
		while (*p && *p != '"') {
		    p[-1] = p[0];
		    p++;
		}
		*p = 0;
	    }
	    TXInsertName( TeXlist, name, TXname, nargs, 
			  (void *)TXCopy(value) );
	}
	else if (strcmp( cmd, "asis" ) == 0) {
	    TXInsertName( TeXlist, name, TXasis, nargs, (void *)0 );
	}
	else if (strcmp( cmd, "raw" ) == 0) {
	    if (value[0] == '"') {
		p = value+1;
		while (*p && *p != '"') {
		    p[-1] = p[0];
		    p++;
		}
		*p = 0;
	    }
	    TXInsertName( TeXlist, name, TXraw, nargs, 
			  (void *)TXCopy(value) );
	}
	else if (strcmp( cmd, "hdef" ) == 0) {
	  /* hdef for HTML-containing definition */
	  /* 4th argument is "cmdstart/end" character */
	  TXInsertName( TeXlist, name, TXDoUser, nargs, 
		TXCreateDefn( nargs, TXConvertQuotes( value, '\'' ) ) );
	}
	else if (strcmp( cmd, "begin-env" ) == 0) {
	  TXSetEnv( name, TXConvertQuotes( value, '\'' ), (char *)0 );
	}
	else if (strcmp( cmd, "end-env" ) == 0) {
	  TXSetEnv( name, (char *)0, TXConvertQuotes( value, '\'' ) );
	}
	else {
	    fprintf( stderr, "Unknown command %s in definitions file\n", cmd );
	}
    }
}
