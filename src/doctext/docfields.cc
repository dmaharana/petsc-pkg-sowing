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

// DocField is a structure so that we can hide it :)
typedef struct _DocField { 
  struct _DocField *next;
  char             *type_name;   // Type name (e.g., int
  char             *name;        // Name of variable
  int              qualifiers;   // array, pointer, etc.  (Not rich enough)
} DocField;

void *DocNewField( void *head, char *name, char *type )
{
  DocField *new = (DocField *)malloc( sizeof(DocField) );
  if (!DocField) {
    fprintf( stderr, "Unable to allocate new field\n" );
    exit(1);
  }
  
  if (head) {
    DocField *p = head;
    while (p->next) p = p->next;
    p->next = new;
  }
  else {
    head = new;
  }
  new->next       = 0;
  new->name       = strdup( name );
  new->type_name  = strdup( type );
  new->qualifiers = 0;

  return head;
}

void DocFreeFields( void *head )
{
  DocField *p = head;

  while (head) {
    p = head->next;
    free( head );
    head = p;
  }
}

