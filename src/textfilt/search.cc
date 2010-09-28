#include "tfilter.h"

#include "srlist.h"
#include <string.h>

/* 
 * This is a VERY simple search system, based on hashed lists.
 * Experts or people with time on their hands can write something better.
 * This is good enough, however, for most of what we need here.
 */

/* Private functions */
int SrList::HashFn( const char *name )
{
    int idx = 0;
    while (*name) idx += (int)*name++;
    if (idx < 0) idx = -idx;
    idx = idx % hash_size;
    return idx;
}

/* Public functions */
SrList::SrList( int h_size )
{
    int i;

    hash_size = h_size;
    table     = new SrEntry*[hash_size];
    for (i=0; i<hash_size; i++) 
	table[i] = 0;
}

SrList::SrList( )
{
    int i;

    hash_size = 511;
    table     = new SrEntry*[hash_size];
    for (i=0; i<hash_size; i++) 
	table[i] = 0;
}

SrList::~SrList()
{
    int i;
    SrEntry *p, *n;
    for (i=0; i<hash_size; i++) {
	if ((p = table[i])) {
	    while (p) {
		n = p->next;
		delete(p);
		p = n;
		}
	    }
	}
    delete(table);
}

// val may be null, in which case this returns 0 if the entry is found and
// 1 otherwise.
int SrList::Lookup( const char *name, SrEntry **val )
{
    int     idx;
    SrEntry *p;

    idx = HashFn( name );
    p   = table[idx];
    while (p) {
	if (strcmp( p->name, name ) == 0) {
	    if (val) *val = p;
	    return 0;
	    }
	p = p->next;
	}
    return 1;
}

int SrList::Insert( const char *name, SrEntry **val )
{
    int     idx;
    SrEntry *p;

    idx = HashFn( name );
    p   = table[idx];
    while (p) {
	if (strcmp( p->name, name ) == 0) {
	    *val = p;
	    return 0;
	    }
	p = p->next;
	}
    /* Did not find an entry.  Create and return */
    p		  = new SrEntry;
    p->next	  = table[idx];
    p->name	  = new char[strlen(name)+1];
    p->extra_data = 0;
    strcpy( p->name, name );

    table[idx] = p;
    *val = p;
    return 0;

}

int SrList::Delete( const char *name )
{
    int i;
    SrEntry *p, *n;
    
    i = HashFn( name );
    p = table[i];
    n = 0;
    while (p) {
	if (strcmp( p->name, name ) == 0) {
	    if (n)
		n->next = p->next;
	    else
		table[i] = p->next;
	    delete(p);
	    }
	n = p;
	p = p->next;
	}
return 0;
}

int SrList::Walk( int (*fcn)( const char *, SrEntry * ) )
{
    int i;
    SrEntry *p;
    
    for (i=0; i<hash_size; i++) {
	p = table[i];
	while (p) {
	    fcn( p->name, p );
	    p = p->next;
	    }
	}
	return 0;
}
