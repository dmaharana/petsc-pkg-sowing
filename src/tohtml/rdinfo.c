/*
    This file reads info files and generates the appropriate rtf output

    A complication is that Windows Help requires both a title and an
    identifying context string.  We handle the later by inserting the title
    into a table and generating a context string (which is just entry<number>)
    When that same string is seen again, we use that name for the the
    context string.
 */

#include <stdio.h>
/* #include "tools.h" */
#include "search.h"

/* Search context for section entries */
static SRList *infoctx = 0;

#define MAX_LINE 256
int ProcessInfoFile( argc, argv, fpin, fpout )
int  argc;
char **argv;
FILE *fpin, *fpout;    
{
char buffer[MAX_LINE];
int  maxlen=MAX_LINE-1, actlen, c;
int  InTopic = 0;

fprintf( stderr, "Starting rdinfo...\n" ); fflush( stderr );

infoctx = SRCreate();

/* Try to read info file */
RdAuxFile( infoctx );

/* Switch to writing a new aux file */
OpenWRAuxFile();

/* Skip the header */
SkipInfoHeader( fpin, buffer, maxlen );

while ((actlen = SYTxtGetLine( fpin, buffer, maxlen )) != EOF) {
    /* Look for the special cases */
    c = buffer[0];
    /* Skip the tag table at the end */
    if (strncmp( buffer, "Node: ", 6 ) == 0)
        continue;
    /* Check for * Menu: */
    if (c == '*') {
    	if (strncmp( buffer, "* Menu:", 7 ) == 0) {
	    WriteTextHeader( fpout );     /* Just in case */
    	    RdInfoMenu( fpin, fpout, buffer, maxlen );
    	    c = buffer[0];
    	    /* We may have found a topic ... */
    	    }
        }
    /* fprintf( stderr, "%xline is %s", buffer[0], buffer );         */
    if (c == 037) {
        while (c == 037) {
            /* NEXT LINE is File: ... with node information or menu
 	       information */
     	    if (InTopic)
     	        WriteEndofTopic( fpout );
    	    InTopic = 1;
    	    RdInfoTopic( fpin, fpout, buffer, maxlen );
    	    c = buffer[0];
            }
	}
    else {    
        /* A regular line */
        OutputTextLine( buffer, fpout );
        }
    }
WriteEndofTopic( fpout );

/* Dump error messages about dangling refs */
{ FILE *fp = fopen( "errors.txt", "w" );
SRDump( infoctx, fp );
SRDumpAll( infoctx, fp );
fclose( fp );
}
}    

/* An info menu is ALWAYS terminated by either a '\037' or an EOF */
int RdInfoMenu( fpin, fpout, buffer, maxlen )
FILE *fpin, *fpout;
char *buffer;
int  maxlen;
{
int  actlen;
char reftopic[MAX_LINE], name[MAX_LINE], entry[MAX_LINE];
int  refnumber;

while ((actlen = SYTxtGetLine( fpin, buffer, maxlen )) != EOF) {
    if (buffer[0] == 037) {
    	/* fprintf( stderr, "Found next topic in menu...\n" ); */
        return actlen;
        }
    if (buffer[0] != '*') continue;
    /* To do this, I need to know the topic reference */
    GetNameFromMenuEntry( buffer, actlen, name, entry );
    GetContextAndEntry( entry, reftopic, &refnumber );
    WritePointerText( fpout, name, reftopic, refnumber );
    WritePar( fpout );
    }
fprintf( stderr, "Done with menu\n" ); fflush( stderr );
return actlen;
}
    
static int  seqnum = 0;
int RdInfoTopic( fpin, fpout, buffer, maxlen )
FILE *fpin, *fpout;
char *buffer;
int  maxlen;
{
int actlen;
char *keywords = 0;
static int  number = 0;
static char *entrylevel = "Document";

/* This reads the "File... " line.  We skip it for now */
SYTxtGetLine( fpin, buffer, maxlen );
OutputUpButton( fpout, buffer );

if (strncmp( buffer, "End tag table", 13 ) == 0) {
    /* Actually, we are at the end of the file */
    return 0;
    }
actlen = SYTxtSkipBlankLines( fpin, buffer, maxlen );
/* Check for a menu line */
if (strncmp( buffer, "* Menu:", 7) == 0) {
    GetContextAndEntry( "Contents", entrylevel, &number );    
    WriteSectionHeader( fpout, "Contents", entrylevel, number, keywords );
    WriteTextHeader( fpout );
    RdInfoMenu( fpin, fpout, buffer, maxlen );
    WriteEndofTopic( fpout );
    return 0;
    }
WRtoauxfile( seqnum++, 0, buffer );
GetContextAndEntry( buffer, entrylevel, &number );    
WriteSectionHeader( fpout, buffer, entrylevel, number, keywords );
/* Skip the next line; it should contain some sort of underlining */
SYTxtGetLine( fpin, buffer, maxlen );

WriteTextHeader( fpout );
/* The rest of the text is handled by the default output code */
}

/*
   Info files start with some information that we want to discard
 */    
int SkipInfoHeader( fpin, buffer, maxlen )
FILE *fpin;
char *buffer;
int  maxlen;
{
int  actlen;
/* We read until we find a blank line */
	
while ( (actlen = SYTxtGetLine( fpin, buffer, maxlen )) != EOF ) {
    if (SYTxtLineIsBlank( buffer, actlen )) break;
    }
}    

/* Given a name in buffer, get the entrylevel and number to use */
/* MS C provides both lsearch and bsearch, but they do not correspond with the
   Unix or POSIX definitions */
int GetContextAndEntry( buffer, entrylevel, number )
char *buffer, *entrylevel;
int  *number;
{
char *p;

p = buffer + strlen(buffer) - 1;
if (*p == '\n') *p = '\0';
/* We really need to try and look name up and if it isn't found, generate a
   unique name */
*number = -1;   
SRInsert( infoctx, buffer, entrylevel, number );
}

/* Menu entries can have two forms:

    * nodename::

   or

    * searchname: nodename.

  In the first case, name and entry are the same.
  In the second, name==searchname.
*/    
int GetNameFromMenuEntry( buffer, actlen, name, entry )
char *buffer, *name, *entry;
int  actlen;
{
char *p;

strcpy( name, buffer + 2 );
p = name + strlen(name) - 1;
if (*p == '\n') p--;
while (p > name && *p == ' ' ) p--;
if (*p == ':') {
    /* Skip over the :: */
    p -= 1;
    *p = '\0';
    strcpy( entry, name );
    }
else if (*p == '.') {
    /* skip over the . */
    *p = '\0';
    /* look for the : */
    while( p > name && *p != ':' ) p--;
    strcpy( entry, p+1 );
    *p = '\0';
    }
else
    fprintf( stderr, "Could not parse menu line %s\n", buffer );    
}


/*
    Eventually, we need to convert "* ... ::" into WritePointerText
 */
static int LastWasIndented = 0;
static int LastWasPar      = 0;

int OutputTextLine( buffer, fpout )
char *buffer;
FILE *fpout;
{
char *p = buffer;
char *starloc;
char schar;

/* A completely blank line needs to be turned into a \par */
if (SYTxtLineIsBlank( buffer, strlen(buffer) )) {
    if (!LastWasPar) 
        WritePar( fpout );
    LastWasPar = 1;
    return 0;
    }
LastWasPar = 0;
/* A line that starts with a blank needs to be formatted as starting a new
   line */
if (*p == ' ') {
    WriteStartNewLine( fpout );
    LastWasIndented = 1;
    }
else if (LastWasIndented) {
    WriteStartNewLine( fpout );
    LastWasIndented = 0;
    }
    
while (*p) {
    if (*p == '*' && strncmp( p, "*Note ", 6) == 0) {
	starloc = p;
	p += 6;
	while (*p && *p != ':') p++;
	if (p[1] == ':') {
	    int refnumber, actlen = strlen(buffer);
	    char entry[MAX_LINE], reftopic[MAX_LINE], name[MAX_LINE];
	    /* Found a interior jump point */
	    *starloc = 0;
	    schar = p[2];
	    p[2]  = 0;
	    WriteString( fpout, buffer );
            GetNameFromMenuEntry( starloc+4, actlen, name, entry );
            GetContextAndEntry( entry, reftopic, &refnumber );
	    WritePointerText( fpout, name, reftopic, refnumber );
	    p[2] = schar;
	    p += 2;
	    WriteString( fpout, p );
	    return 0; 
	    }
        }
    else
        p++;
    }        
WriteString( fpout, buffer );
return 0;
}

/*
   This routine searches through a "file" line for the "Up: topicname,"
   phrase and adds the appropriate rebinding of Up to that topic.
 */
int OutputUpButton( fpout, buffer )
FILE *fpout;
char *buffer;
{
char *p, *p2;
char reftopic[MAX_LINE], context[MAX_LINE];
int  refnumber;

if (strncmp( buffer, "File: ", 6 ) != 0) {
    SetUpButton( fpout, "Node1", "Contents" );
    return 0;
    }
p = buffer;
while (*p) {
    while (*p && *p != 'U') p++;
    if (strncmp( p, "Up: ", 4 ) == 0) {
    	/* Found the "up" value */
    	p += 4;
    	/* Look for the comma */
    	p2 = p + 1;
    	while (*p2 && *p2 != ',') p2++;
    	if (!*p2) {
    	    SetUpButton( fpout, "Node0", "Contents" );
    	    return 0;
    	    }
	*p2 = 0;
	/* Top is a synonym for the top, (dir) for the info directory that
	   has no Windows counterpart */
	if (strcmp( p, "Top" ) == 0 || strcmp( p, "(dir)" ) == 0) {
    	    SetUpButton( fpout, "Node000", "Contents" );
    	    return 0;
	    }
        GetContextAndEntry( p, reftopic, &refnumber );
        sprintf( context, "%s%.3d", reftopic, refnumber );
	SetUpButton( fpout, context, p );
	return 0;
        }
    p++;
    }
}
