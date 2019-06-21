#ifndef _GETOPTS
#define _GETOPTS

void SYArgSqueeze(int *, char **);
int SYArgFindName(int, char **, const char *);
int SYArgGetInt(int *, char **, int, const char *, int *);
int SYArgGetDouble(int *, char **, int, const char *, double *);
int SYArgGetString(int *, char **, int, const char *, char *, int);
int SYArgHasName(int *, char **, int, const char *);
int SYArgGetIntVec(int *, char **, int, const char *, int, int *);
int SYArgGetIntList(int *, char **, int, const char *, int, int *);

#endif

