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
#define MATCH_STRING "/*[@MDIN]"

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

// Special lead character types (only in the first column in a comment)
#define ARGUMENT    '.'
#define ARGUMENT_BEGIN '+'
#define ARGUMENT_END '-'
#define VERBATIM    '$'

// Don't forget to update these
#define DOCTEXT_VERSION "1.1.3"
#define DOCTEXT_DATE    "October 12, 1999"
#endif
