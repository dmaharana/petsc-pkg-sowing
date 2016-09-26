#ifndef _GETOPTS
#define _GETOPTS

void SYArgSqueeze(int *, char **);
int SYArgFindName(int, char **, char *);
int SYArgGetInt(int *, char **, int, char *, int *);
int SYArgGetDouble(int *, char **, int, char *, double *);
int SYArgGetString(int *, char **, int, char *, char *, int);
int SYArgHasName(int *, char **, int, char *);
int SYArgGetIntVec(int *, char **, int, char *, int, int *);
int SYArgGetIntList(int *, char **, int, char *, int, int *);

#endif

