/*
    This file contains routines for providing a mapping from names to
    hypertext links

    The format of an entry is

    tagtype:%cname%ckind%ctext%c%cicon%cscheme://host.domain[.port]/path/filename

    where
    tagtype is one of cite, see, man, demo, manual, activetoken

    %c stands for ANY character (matched by the corresponding char)

    name is the name to match with

    kind is one of bib (bibliography entry), file, ...

    text is the text to be used to label the link

    icon is the name of an icon file to use (may be null)
    
    The rest is a standard URL (see html documentation)
 */

#include <stdio.h>
#include "sowing.h"
#include "tex.h"
#include "search.h"

typedef struct _MapRef {
    int      tagtype, ikind;
    char     *url,        /* Uniform Resource Locator field */
             *text,       /* Short text */
             *icon;       /* icon file */
    } MapRef;

static SRList *refmap = 0;
static int DebugMap = 0;
/* These routines process the name buffer */

/* Forward refs */
void GetMapTagName ( char **, char * );
char *GetMapName ( char ** );
char *GetMapURL ( char ** );
char *CpyString ( char * );
int MapKind ( char * );

void GetMapTagName( P, namep )
char **P, *namep;
{
    char *p = *P, c;

    while (*p && *p != ':') 
	*namep++ = *p++;
    p++;
    c = *p++;
    while (*p && *p != c) {
	*namep++ = *p++;
    }
    *namep++ = 0;    
    *P = p + 1;    
}
	
/* Get the name (%cname%c).  Return ptr to name (null if empty) */
char *GetMapName( P )
char **P;
{
    char c, *p, *ptr;

    p = *P;
    c = *p++;
    if (*p == c) {
	*P = p + 1;
	return 0;
    }
    ptr = p;
    while (*p && *p != c) p++;

    *p = 0;
    p++;
    *P = p;
    return ptr;
}

char *GetMapURL( P )
char **P;
{
    char *ptr = *P, *p;

    p = ptr;
    while (*p && *p != '\n') p++;
    if (*p) *p = 0;   /* Remove trailing newline */
    return ptr;	
}

char *CpyString( str )
char *str;
{
    char *p;
	
    if (!str) return 0;

    p = (char *)MALLOC( strlen( str ) + 1 );   CHKPTRN(p);

    strcpy( p, str );
    return p;
}

int MapKind( kind )
char *kind;
{
    if (strcmp( kind, "cite" ) == 0 ) return 1;
    if (strcmp( kind, "see" ) == 0 ) return 2;
    if (strcmp( kind, "man" ) == 0 ) return 3;
    if (strcmp( kind, "demo" ) == 0 ) return 4;
    if (strcmp( kind, "manual" ) == 0 ) return 5;
    if (strcmp( kind, "active" ) == 0 ) return 6;
    return -1;	
}

void RdRefMap( name )
char *name;
{
    FILE   *fp;
    LINK   *l;
    int    d;
    MapRef *c;
    char   sbuf[256], *p, *url, namep[128], *kind, *icon, *text;

    fp = fopen( name, "r" );
    if (!fp) {
	fprintf( stderr, "Could not open %s\n", name );
	return;
    }    

    if (!refmap) refmap = SRCreate();
    
    while (1) {
	if (!fgets( sbuf, 256, fp )) break;
	p       = sbuf;
	if (p[0] == ';' || p[0] == '\n') continue;
/*    
      tagtype:%cname%ckind%ctext%c%cicon%cscheme://host.domain[.port]/path/filename
      */
	GetMapTagName( &p, namep ); if (DebugMap) printf( "name=%s\n", namep );
	text    = GetMapName( &p ); 
	if (text && DebugMap) 
	    printf( "text=%s\n", text );
	icon    = GetMapName( &p ); 
	if (icon && DebugMap) 
	    printf( "icon=%s\n", icon );
	kind    = GetMapName( &p ); if (DebugMap) printf( "kind=%s\n", kind );
	url     = GetMapURL( &p );  if (DebugMap) printf( "url = %s\n", url );
	/* Strip type off of url */
	if (strncmp( "file:", url, 5 ) == 0) url += 5;

	l       = SRInsert( refmap, namep, 0, &d );
	c       = NEW(MapRef);                         CHKPTR(c);
	/* Add URL and type to link private data */
	l->priv  = (void *)c;
	c->ikind = MapKind( kind );
	c->url   = CpyString( url );
	c->text  = CpyString( text );
	c->icon  = CpyString( icon );
    }
    fclose( fp );    
}

int RefMapLookup( tagtype, token, url, urltype, text )
char *tagtype;
char *token;
char **url, **text;
int  *urltype;
{
    LINK   *l;
    MapRef *r;
    int    dummy;
    char   fullname[128];

    strcpy( fullname, tagtype );
    strcat( fullname, token );

    l = SRLookup( refmap, fullname, (char *)0, &dummy );
    if (!l) return 0;

    r     = (MapRef *)(l->priv);
    *url  = 	r->url;
    *text = r->text;
    return 1;
}
