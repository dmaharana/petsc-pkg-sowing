//
// This file contains routine to remember the fields in a type, macro, struct,
// or enum.  
//
// Need:
//  Create new list
//  Add entry to list
//  Return next entry in list as string.
//
// Should these read directly?
// Do we want to use this general approach or do we want one for each 
// type (struct, enum, function arg).

#include "doc.h"
#include "docutil.h"
#include <stdlib.h>
#include <string.h>

// DocField is a structure so that we can hide it :)
typedef struct _DocField { 
  struct _DocField *next;
  char             *type_name;   // Type name (e.g., int
  char             *name;        // Name of variable
  int              qualifiers;   // array, pointer, etc.  (Not rich enough)
} DocField;

void *DocNewField( void *head, char *name, char *type )
{
  DocField *newdoc = new(DocField);

  if (!newdoc) {
    fprintf( stderr, "Unable to allocate new field\n" );
    exit(1);
  }
  
  if (head) {
    DocField *p = (DocField *)head;
    while (p->next) p = p->next;
    p->next = newdoc;
  }
  else {
    head = newdoc;
  }
  newdoc->next       = 0;
  newdoc->name       = strdup( name );
  newdoc->type_name  = strdup( type );
  newdoc->qualifiers = 0;

  return head;
}

void DocFreeFields( void *head )
{
  DocField *p = (DocField *)head, *pn;

  while (p) {
    pn = p->next;
    delete( p );
    p = pn;
  }
}

