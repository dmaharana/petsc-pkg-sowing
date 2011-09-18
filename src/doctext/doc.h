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

//
extern const char *IgnoreString;

// Used for newlines.
extern char NewlineString[3];

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
#define DEFINE       'J'

// Special lead character types (only in the first column in a comment)
#define ARGUMENT    '.'
#define ARGUMENT_BEGIN '+'
#define ARGUMENT_END '-'
#define VERBATIM    '$'

// Don't forget to update these
#define DOCTEXT_VERSION "1.1.7"
#define DOCTEXT_DATE    "September 12, 2003"
#endif
