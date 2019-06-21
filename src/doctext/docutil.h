#ifndef DOCUTIL
#define DOCUTIL
#include "textout.h"

int DocReadName( InStream *ins, char *routinename, int maxlen );
int DocReadDescription( InStream *ins, char *matchstring,
			TextOut *textout, int flag, int *at_end );
int DocSkipToFuncSynopsis( InStream *ins, char *matchstring );
int DocReadFuncSynopsis( InStream *ins, /*OutStream*/TextOut *outs );
int DocSkipToMacroSynopsis( InStream *ins, char *matchstring );
int DocReadMacroSynopsis( InStream *ins, char *matchstring,
			  TextOut /*OutStream*/ *outs,
			  int *at_end );
int DocReadTypeDefinition( InStream *ins, TextOut /*OutStream*/ *outs );
int DocReadEnumDefinition( InStream *ins, TextOut *outs );
int DocReadDefineDefinition(InStream *ins, TextOut *outs);
int DocGetSubOptions( InStream *ins );
int DocMatchTokens( const char *, const char * );
int IncludeNameBlock( InStream *, char * );
int SaveName( InStream *, char * );
const char *GetCurrentRoutinename( void );
const char *GetCurrentFileName( void );
const char *GetCurrentInputFileName( void );
int SkipLeadingString( InStream *, char * );
char *SkipOverLeadingString( char * );
void UngetLeadingString( InStream * );

int IndexFileInit(const char *idxname, const char *idxdir_in);
int IndexFileAdd(const char *name, const char *outname,
		 const char *outfilename, const char *label);
void IndexFileEnd(void);
int JumpFileInit(const char *jumpfile);
int JumpFileAdd(const char *infilename, const char *routine, int linenum);
void JumpFileEnd(void);

void indexArgsSet(int val);
int indexArgsPut(const char *argname);

// textb.cc
int CheckForEndOfBlock(InStream *);
int SaveName(InStream *, char *);
int IncludeNameBlock(InStream *, char *);

// docfields.cc
void *DocNewField(void *, char *, char *);
void DocFreeFields(void *);

// keyword.cc
int KeywordOpen(const char *);
int KeywordOut(char *);

#endif
