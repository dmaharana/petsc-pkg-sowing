#ifndef DOCTEXT
#define DOCTEXT

#include "sowingconfig.h"
#define MAX_ROUTINE_NAME 64
#define MAX_LINE         512
// The next is for MAXPATHLEN; MSDOS does not have param.h
#ifndef MAX_PATH_LEN
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#elif defined(MAX_PATH)
#define MAXPATHLEN MAX_PATH
#else
#define MAXPATHLEN 1024
#endif
#endif
#define MAX_PATH_LEN     1024

// Match characters should be changeable.  These are the defaults
// @ routine
// M Macro
// D Documentation (Text)
// I Include information
// J Define (to include CPP define in documentation)
// N Named block
// S Structure definition
// E Enum definition
// P routine in prototype form (may be prototype definition instead
//   of actual function definition
#define MATCH_STRING "/*[@MDINSEPJ]"

// Leading string (null for C, could be // for C++, C for Fortran, # for
// perl or shells)
extern char LeadingString[10];

// Indicate if we're in an argument list or not
extern int InArgList;
extern int CntArgList;

// Use to enable the old-style argument processing (allowing multiple ". "
// commands
extern int OldArgList;

extern int LastCmdArg;

//
extern const char *IgnoreString;

// Used for newlines.
extern char NewlineString[3];

// Used for debugging
extern int verbose;

// User for warning and error message control
extern int warningEnable;
extern int warningNoArgDesc;

// Used for the rare case (especially in verbatim output) where we still need
// to know the output format.
typedef enum { FMT_UNKNOWN, FMT_NROFF, FMT_LATEX, FMT_HTML, FMT_MYST } outFormat_t;
extern outFormat_t outFormat;

// Many of these are KINDS returned by the tests
#define ROUTINE     '@'
#define DESCRIPTION 'D'
#define FORTRAN     'F'
#define INTERNAL    '+'
#define HELP        'H'
#define MACRO       'M'
#define INCLUDE     'I'
#define TEXT        'N'
#define ENUMDEF     'E'
#define STRUCTDEF   'S'
#define PROTOTYPEDEF 'P'
#define DEFINE      'J'

// Special lead character types (only in the first column in a comment)
#define ARGUMENT    '.'
#define ARGUMENT_BEGIN '+'
#define ARGUMENT_END '-'
#define VERBATIM    '$'

// Don't forget to update these.  Use sowing package version if available
#ifdef PACKAGE_VERSION
#define DOCTEXT_VERSION PACKAGE_VERSION
#else
#define DOCTEXT_VERSION "1.1.8"
#endif
#define DOCTEXT_DATE    "June 19, 2019"
#endif /* DOCTEXT */
