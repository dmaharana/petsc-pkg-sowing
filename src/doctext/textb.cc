/*
    This file contains very simple code to hold a common block of text to
    be inserted in responce to a .n name line
 */

#include "textout.h"
#include <string.h>
 
#define MAX_TEXT_BLOCK 16380
typedef struct _NameBlock {
    char *name;
    int  nbytes;
    char *replacement_text;
    struct _NameBlock *next;
    } NameBlock;

static NameBlock *head  = 0;

/* This routine returns 1 if a * / is found, 0 otherwise.  The input
   character position is the same if * / is NOT found; otherwise it is
   after the * / .
 */
int CheckForEndOfBlock( InStream *ins )
{
char c1, c2;	

ins->GetChar( &c1 );
if (c1 == '*') {
    ins->GetChar( &c2 );
    if (c2 == '/') {
        /* Found end of text */
	return 1;
  	}
    else {
        ins->UngetChar( c2 );
	ins->UngetChar( c1 );
        }
    }
else {
    ins->UngetChar( c1 );
    }

return 0;    
}

int SaveName( InStream *ins, char *name )
{
  NameBlock *newblock;
  char c;
  int  cnt;
  
  newblock = new NameBlock;
  
  newblock->name = new char[strlen(name) + 1];
  strcpy(newblock->name, name );
  
  newblock->next = head;
  head = newblock;

  newblock->replacement_text = new char[MAX_TEXT_BLOCK];
  
  cnt = 0;
  while (cnt < MAX_TEXT_BLOCK && (!ins->GetChar( &c ))) {
    /* Look for N * / */
    if (c == 'N') {
      if(CheckForEndOfBlock( ins )) break;
    }
    newblock->replacement_text[cnt++] = c;
  }
  newblock->replacement_text[cnt] = 0;
  newblock->nbytes = cnt;

  /* Could trim storage here ... */
  return 0;
}
    
int IncludeNameBlock( InStream *ins, char *name )
{
  NameBlock *b;

  b = head;
  while (b) {
    if (strcmp( name, b->name ) == 0) {
      if (ins->UngetToken( b->replacement_text )) {
	fprintf( stderr, "Could not push pack full text for %s\n", name );
      }
      return 1;
    }
    b = b->next;
    }
  return 0;
}
