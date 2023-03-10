
/*   Utility routines used by the document preparation system.     */

#include "sowing.h"
#include <string.h>
#include "doc.h"

/* SubClass is the character after the KIND specifier */
static char SubClass    = ' ';
static int IsX11Routine = 0;

/*************************************************************************/

static int LineNo = 1;

/* This is a getc that keeps track of line numbers */
/* It gives TOTAL lines read, not per file */

/*
   altbuf allows us to provide a simple, one-level redirection of
   input for processing .N commands
 */
static char *altbuf = 0;

int DocGetChar( FILE *fd )
{
    int c;

    if (altbuf) {
	if (*altbuf) c = *altbuf++;
	else {
	    altbuf = 0;
	    c = getc( fd );
	}
    }
    else {
	c = getc( fd );
    }
    if (c == '\n') LineNo++;
    return c;
}

/* "Backwards" order matches ungetc */
void DocUnGetChar( char c, FILE *fd )
{
    if (c == '\n') LineNo--;
    if (altbuf) {
	--altbuf;
	if (*altbuf != c) {
	    fprintf( stderr, "Unexpected char %c in pushback\n", c );
	}
    }
    else {
	ungetc( c, fd );
    }
}

void DocSetPushback( char *str )
{
    if (altbuf) {
	fprintf( stderr, "Only one level of pushback string allowed\n" );
	return;
    }
    altbuf = str;
}

void ResetLineNo(void)
{
    LineNo = 1;
}

int GetLineNo(void)
{
    return LineNo;
}


/*************************************************************************/
/* Return the character after the entry kind.  This character must be
   either a space or the letter 'C'.  If it is not, set ierr to 1, otherwise
   set to zero. */
char GetSubClass(int *ierr)
{
    *ierr = 0;
    if (SubClass != 'C' && !isspace(SubClass)) *ierr = 1;
    return SubClass;
}
int GetIsX11Routine(void)
{
    return IsX11Routine;
}

/* Find a (non-alphanumeric) delimited token ----------------------*/
/* (After finding / * <char>, look for additional chars */
void FindToken( FILE *fd, char *token )
{
    int c;

    IsX11Routine = 0;
    c            = DocGetChar( fd );
    SubClass     = c;
    if (c == 'C') {
	c = DocGetChar( fd );
    }
    if (c == 'X') IsX11Routine = 1;

    while ((c = DocGetChar( fd )) != EOF)
	if (isalpha(c)) break;
    *token++ = c;
    while ((c = DocGetChar( fd )) != EOF)
	if (!isspace(c)) *token++ = c;
	else break;
    *token++ = '\0';
}

/* read chars until we find a leader (/ * <char>) and a matching character.
   Then find the routine name (<name> - )
   Note that this routine does NOT use DocGetChar; this routine is
   the one that skips most of the text and needs to be faster.  */
int FoundLeader( FILE *fd, char *routine, char *kind )
{
    int c;

    while ((c = getc( fd )) != EOF) {
	if (c == '/') {
	    if (MatchLeader( fd, MATCH_CHARS, kind )) {
		FindToken( fd, routine );
		return 1;
	    }
        }
	else if (c == '\n') LineNo++;
    }
    return 0;
}

/* Match a leader that starts with / * and then any of the selected
   characters.  Discards characters that don't match.  If we have
   entered this routine, we have already seen the first character (/) */
int MatchLeader( FILE *fd, const char *Poss, char *kind )
{
    int c;
    c = DocGetChar( fd );
    if (c == '*') {
	/* In a comment.  We should really be prepared to skip this
	   comment if we don't find that it is a documentation block */
	c = DocGetChar( fd );
	if (strchr( Poss, c )) {
	    *kind = c;
	    return 1;
        }
    }
    return 0;
}

/*
  Copy an "include" to a buffer.  The form is / *I include-file-spec I* /
  Only one per line.
 */
void CopyIncludeName( FILE *fin, char *buffer )
{
    char *p;
    int  c;

    SkipWhite( fin );
    p = buffer;

    while ((c = DocGetChar(fin)) != EOF)  {
	if (c == '\n') break;
	if (c == 'I') {
	    p[0] = 'I';
	    c = DocGetChar(fin);
	    if (c == '*') {
		p[1] = '*';
		c = DocGetChar(fin);
		if (c == '/') { p--; break; }
		p += 1;
	    }
	    p += 1;
	}
	*p++ = c;
    }
    while (p > buffer && *p == ' ') p--;
    *++p = 0;
}

/* modifies the filename by converting it to the fullpath name and
   then removing the piece TOOLSDIR */
void ExpandFileName(char *infilename, int rlen)
{
    char tmp[MAX_FILE_SIZE],*name;
    name = tmp + rlen;
    SYGetFullPath(infilename,tmp,MAX_FILE_SIZE);
    if (rlen > 0) {
	strcpy(infilename,"$TOOLSDIR");
	strncat(infilename,name,MAX_FILE_SIZE-rlen-9);
    }
    else {
	strncpy(infilename,name,MAX_FILE_SIZE);
    }
}

#if !defined(MSDOS) && !defined(WIN32)
#include <sys/param.h>
#endif
/* extern char *getenv(); */
#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

/* returns the length of the full path name of the tools directory */
int GetToolsDirLength(void)
{
    char *toolsdir, truepath[MAXPATHLEN];
    int  rlen;

    toolsdir = getenv("TOOLSDIR");
    if (!toolsdir) return 0;
    else {
	SYGetRealpath(toolsdir,truepath);
	rlen = (int)strlen(truepath);
    }
    return rlen;
}


/*
   This is a comparision routine that is independent of case
   str2 should be upper case; str1 need not be.
   Returns 0 if they match, != 0 otherwise
 */
int MatchTokens( char *str1, char *str2 )
{
    char buf[256];

    strcpy( buf, str1 );
    UpperCase( buf );
    return strcmp( buf, str2 );
}
