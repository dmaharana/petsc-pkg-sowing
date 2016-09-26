#ifndef _SOWINGTXT
#define _SOWINGTXT

extern int SYTxtGetLine(FILE *, char *, int);
extern int SYTxtSkipBlankLines(FILE *, char *, int);
extern int SYTxtLineIsBlank(char *, int);
extern int SYTxtFindNextToken(FILE *, char *, int, int *);
extern int SYTxtGetChar(FILE *);
extern int SYTxtFindNextANToken(FILE *, char *, int, int *);
extern int SYTxtFindNextAToken(FILE *, char *, int, int *);
extern int SYTxtSkipWhite(FILE *);
extern int SYTxtSkipOver(FILE *, char *);
extern void SYTxtDiscardToEndOfLine(FILE *);
extern int SYTxtTrimLine(char *);
extern void SYTxtUpperCase(char *);
extern void SYTxtLowerCase(char *);

#endif
