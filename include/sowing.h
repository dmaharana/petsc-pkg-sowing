
#ifndef _SOWING
#define _SOWING

/* 
   Include prototypes for the basic functions that some compilation systems, 
   such as Gnu gcc under SunOS, do not provide(!) 
 */
#ifdef NEEDS_STDLIB_PROTOTYPES
#include "protofix.h"
#endif

#include <stdio.h>

#include <stdlib.h> 
/* Include the definitions from configure */
#if !defined(WIN32)
#include "sowingconfig.h"
#else
#define __MSDOS__
#define HAVE_STDLIB_H 1
#include <string.h>
#endif

/* Include space tracing code */
#include "trspace.h"

/* Include argument processing code */
#include "getopts.h"

/* Include common file operations */
#include "sowingfile.h"

/* Include common text procession operations */
#include "sowingtxt.h"

/* Include daytime information */
#include "daytime.h"

#endif
