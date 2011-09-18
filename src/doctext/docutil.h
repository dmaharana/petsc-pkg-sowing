#ifndef DOCUTIL
#define DOCUTIL
#include "textout.h"

int DocReadName( InStream *ins, char *routinename, int maxlen );
int DocReadDescription( InStream *ins, char *matchstring, 
			TextOut *textout, int flag, int *at_end );
int DocSkipToFuncSynopsis( InStream *ins, char *matchstring );
int DocReadFuncSynopsis( InStream *ins, OutStream *outs );
int DocSkipToMacroSynopsis( InStream *ins, char *matchstring );
int DocReadMacroSynopsis( InStream *ins, char *matchstring, OutStream *outs, 
			  int *at_end );
int DocReadTypeDefinition( InStream *ins, OutStream *outs );
int DocReadDefineDefinition( InStream *ins, OutStream *outs );
int DocGetSubOptions( InStream *ins );
int DocMatchTokens( const char *, const char * );
int IncludeNameBlock( InStream *, char * );
int SaveName( InStream *, char * );
const char *GetCurrentRoutinename( void );
const char *GetCurrentFileName( void );
int SkipLeadingString( InStream *, char * );
char *SkipOverLeadingString( char * );
void UngetLeadingString( InStream * );
#endif
