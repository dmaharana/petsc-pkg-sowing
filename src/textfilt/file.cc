#include "tfilter.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "tfile.h"
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
/*
 * Useful file operations
 */


/*@
    SYLastChangeToFile - Gets the date that a file was changed as a string or
    timeval structure

    Input Parameter:
.   fname - name of file

    Output Parameters:
.   date - string to hold date (may be null)
.   ltm   - tm structure for time (may be null)
@*/
void SYLastChangeToFile( const char *fname, char *date, struct tm *ltm )
{
    struct stat buf;
    struct tm   *tim;
    
    if (stat( fname, &buf ) == 0) {
	tim = localtime( &(buf.st_mtime) );
	if (ltm) *ltm = *tim;
	if (date) 
	    sprintf( date, "%d/%d/%d", 
		    tim->tm_mon+1, tim->tm_mday, tim->tm_year+1900 );
	}
    else {
	/* Could not stat file */	
	if (date)
	    date[0] = '\0';
	if (ltm) {
	    ltm->tm_mon = ltm->tm_mday = ltm->tm_year = 0;
	    }
	}
}

#if defined(HAVE_REALPATH) && !defined(STDC_HEADERS)
    extern "C" { char *realpath( const char *, char *); }
#endif

/*@
   SYGetRealpath - get the path without symbolic links etc and in absolute
   form.

   Input Parameter:
.  path - path to resolve

   Output Parameter:
.  rpath - resolved path

   Note: rpath is assumed to be of length MAXPATHLEN

   Note: Systems that use the automounter often generate absolute paths
   of the form "/tmp_mnt....".  However, the automounter will fail to
   mount this path if it isn't already mounted, so we remove this from
   the head of the line.  This may cause problems if, for some reason,
   /tmp_mnt is valid and not the result of the automounter.
@*/
char *SYGetRealpath( const char *path, char *rpath )
{
#if defined(HAVE_REALPATH)
    realpath( path, rpath );

#elif !defined(HAVE_READLINK)
    strcpy( rpath, path );

#else
#if defined(IRIX)
    extern char *strchr();
#endif
    int  n, m, N, nsteps;
    char tmp1[MAXPATHLEN], tmp3[MAXPATHLEN], tmp4[MAXPATHLEN], *tmp2;
    /* Algorithm: we move through the path, replacing links with the
       real paths.
       */
    strcpy( rpath, path );
    
    /* We must exercise real care here, because readlink may NOT set
       N to 0 on failure (IRIX)! */
    N = strlen(rpath);
    nsteps = 20;
    // Stop when we've done 20 steps, just in case
    while (N && nsteps--) {
	strncpy(tmp1,rpath,N); tmp1[N] = 0;
	n = readlink(tmp1,tmp3,MAXPATHLEN);
	if (n > 0) {
	    /* readlink does not automatically add 0 to string end */
	    tmp3[n] = 0; 
	    if (tmp3[0] != '/') {
		tmp2 = strchr(tmp1,'/');
		m = strlen(tmp1) - strlen(tmp2);
		strncpy(tmp4,tmp1,m); tmp4[m] = 0;
		strncat(tmp4,"/",MAXPATHLEN - strlen(tmp4));
		strncat(tmp4,tmp3,MAXPATHLEN - strlen(tmp4));
		SYGetRealpath(tmp4,rpath);
		strncat(rpath,path+N,MAXPATHLEN - strlen(rpath));
		return rpath;
		}
	    else {
		SYGetRealpath(tmp3,tmp1);
		strncpy(rpath,tmp1,MAXPATHLEN);
		strncat(rpath,path+N,MAXPATHLEN - strlen(rpath));
		return rpath;
		}
	    }  
	tmp2 = strchr(tmp1,'/');
	if (tmp2) N = strlen(tmp1) - strlen(tmp2);
	else N = strlen(tmp1);
	}
    strncpy(rpath,path,MAXPATHLEN);
#endif
    if (strncmp( "/tmp_mnt/", rpath, 9 ) == 0) {
	char tmp3[MAXPATHLEN];
	strcpy( tmp3, rpath + 8 );
	strcpy( rpath, tmp3 );
	}
    return rpath;
}
