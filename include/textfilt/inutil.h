#ifndef INUTIL
#define INUTIL
int FindPattern( InStream *ins, const char *pattern, char *match );
int SkipWhite( InStream *ins );
int SkipLine( InStream *ins );
int SkipOver( InStream *ins, const char *str );
#endif
