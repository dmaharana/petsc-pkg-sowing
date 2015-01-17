/*
 * This file contains the default paths for the basic definitions for the
 * various TextOut classes (HTML, NROFF, TeX, etc)
 * The source of this file is textpath.h.in; it is edited by configure
 * to generate the final file.
 */

#ifndef TEXTFILTER_PATH
#if defined(__MSDOS__) || defined(WIN32)
#define TEXTFILTER_PATH ". c:\\sowing\\share f:\\sowing\\share"
#define DIR_SEP '\\'
#else
#define TEXTFILTER_PATH ".:/Users/barrysmith/Src/petsc/arch-mpich/share"
#define DIR_SEP '/'
#endif
#endif
