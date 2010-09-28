#define MAX_FILE_NAME 1024

#include "sowing.h"
#if defined(HAVE_PWD_H)
#include <pwd.h>
#endif
#include <ctype.h>
/*
   Here's an unpleasent fact.  On Intel systems, include-ipsc/sys/types.h 
   contains "typedef long time_t" and
   include/time.h contains "typedef long int time_t".  We can fix this by 
   defining __TIME_T after types.h is included.
 */
#include <sys/types.h>
#if defined(intelnx) && !defined(intelparagon) && !defined(__TIME_T)
#define __TIME_T
#endif
#include <sys/stat.h>
/* Here's an unpleasent fact.  On Intel systems, unistd contains REDEFINITIONS
   of SEEK_SET, SEEK_CUR, and SEEK_END that are not guarded (in fact, the 
   unistd.h file contains no guards against multiple inclusion!).  */
#if defined(intelnx) && !defined(intelparagon)
#undef SEEK_SET
#undef SEEK_CUR
#undef SEEK_END
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/*
extern char *mktemp();
extern char *getcwd();
*/

/* WARNING - some systems don't have stdlib.h */
#if !defined(NOSTDHDR)
#include <stdlib.h>

#if defined(tc2000)
extern char *getenv();
#endif

#else
extern char *getenv();
#endif

#if defined(__MSDOS__) || defined(WIN32)
typedef unsigned short u_short;
#endif
#if (defined(intelnx) && !defined(intelparagon)) || defined(__MSDOS__) || defined(WIN32)
typedef u_short uid_t;
typedef u_short gid_t;
#endif

/* Prototypes for internal routines */
int SYiTestFile( char *, char, uid_t, gid_t );

#if defined(HAVE_PWD_H)
/*@
   SYGetFullPath - Given a filename, return the fully qualified 
                 file name.

   Input parameters:
.   path     - pathname to qualify
.   fullpath - pointer to buffer to hold full pathname
.   flen     - size of fullpath

@*/
void SYGetFullPath( char *path, char *fullpath, int flen )
{
struct passwd *pwde;
int           ln;

if (path[0] == '/') {
    if (strncmp( "/tmp_mnt/", path, 9 ) == 0) 
	strncpy( fullpath, path + 8, flen );
    else 
	strncpy( fullpath, path, flen ); 
    return;
    }
SYGetwd( fullpath, flen );
strncat( fullpath,"/",flen - strlen(fullpath) );
if ( path[0] == '.' && path[1] == '/' ) 
strncat( fullpath, path+2, flen - strlen(fullpath) - 1 );
else 
strncat( fullpath, path, flen - strlen(fullpath) - 1 );

/* Remove the various "special" forms (~username/ and ~/) */
if (fullpath[0] == '~') {
    char tmppath[MAX_FILE_NAME];
    if (fullpath[1] == '/') {
	pwde = getpwuid( geteuid() );
	if (!pwde) return;
	strcpy( tmppath, pwde->pw_dir );
	ln = strlen( tmppath );
	if (tmppath[ln-1] != '/') strcat( tmppath+ln-1, "/" );
	strcat( tmppath, fullpath + 2 );
	strncpy( fullpath, tmppath, flen );
	}
    else {
	char *p, *name;

	/* Find username */
	name = fullpath + 1;
	p    = name;
	while (*p && isalnum(*p)) p++;
	*p = 0; p++;
	pwde = getpwnam( name );
	if (!pwde) return;
	
	strcpy( tmppath, pwde->pw_dir );
	ln = strlen( tmppath );
	if (tmppath[ln-1] != '/') strcat( tmppath+ln-1, "/" );
	strcat( tmppath, p );
	strncpy( fullpath, tmppath, flen );
	}
    }
/* Remove the automounter part of the path */
if (strncmp( fullpath, "/tmp_mnt/", 9 ) == 0) {
    char tmppath[MAX_FILE_NAME];
    strcpy( tmppath, fullpath + 8 );
    strcpy( fullpath, tmppath );
    }
/* We could try to handle things like the removal of .. etc */
}
#else
void SYGetFullPath( char *path, char *fullpath, int flen )
{
  strcpy( fullpath, path );
}	
#endif

/*@
   SYGetRelativePath - Given a filename, return the relative path (remove
                 all directory specifiers)

   Input parameters:
.   fullpath  - full pathname
.   path      - pointer to buffer to hold relative pathname
.   flen     - size of path

@*/
void SYGetRelativePath( char *fullpath, char *path, int flen )
{
char  *p;

/* Find last '/' */
p = strrchr( fullpath, '/' );
if (!p) p = fullpath;
else    p++;
strncpy( path, p, flen );
}

#if defined(HAVE_PWD_H)

/*@
    SYGetUserName - Return the name of the user.

    Input Parameter:
    nlen - length of name
    Output Parameter:
.   name - contains user name.  Must be long enough to hold the name

@*/
void SYGetUserName( char *name, int nlen )
{
struct passwd *pw;

pw = getpwuid( getuid() );
if (!pw) 
    strncpy( name, "Unknown",nlen );
else
    strncpy( name, pw->pw_name,nlen );
}
#else
void SYGetUserName( name, nlen )
int  nlen;
char *name;
{
strncpy( name, "Unknown", nlen );
}
#endif /* !HAVE_PWD_H */
#if !defined(__MSDOS__) && !defined(WIN32) && (!defined(intelnx) || defined(paragon))

#if defined(HAVE_UNAME)
#include <sys/utsname.h>
#endif
#if defined(HAVE_GETHOSTBYNAME)
#if defined(HAVE_NETDB_H)
/* Some Solaris systems can't compile netdb.h */
#include <netdb.h>
#else
#undef HAVE_GETHOSTBYNAME
#endif
#endif /* HAVE_GETHOSTBYNAME */
#if defined(HAVE_SYSINFO)
#if defined(HAVE_SYS_SYSTEMINFO_H)
#include <sys/systeminfo.h>
#else
#ifdef HAVE_SYSINFO
#undef HAVE_SYSINFO
#endif
#endif
#endif


/*@
    SYGetHostName - Return the name of the host.

    Input Parameter:
    nlen - length of name
    Output Parameter:
.   name - contains host name.  Must be long enough to hold the name
           This is the fully qualified name, including the domain.
@*/
void SYGetHostName( char *name, int nlen )
{
/* This is the perfered form, IF IT WORKS. */
#if defined(HAVE_UNAME) && defined(HAVE_GETHOSTBYNAME)
    struct utsname utname;
    struct hostent *he;
    uname( &utname );
    he = gethostbyname( utname.nodename );
    /* We must NOT use strncpy because it will null pad to the full length
       (nlen).  If the user has not allocated MPI_MAX_PROCESSOR_NAME chars,
       then this will unnecessarily overwrite storage.
     */
    /* strncpy(name,he->h_name,nlen); */
    {
	char *p_out = name;
	char *p_in  = he->h_name;
	int  i;
	for (i=0; i<nlen-1 && *p_in; i++) 
	    *p_out++ = *p_in++;
	*p_out = 0;
    }
#else
#if defined(HAVE_UNAME)
    struct utsname utname;
    uname(&utname);
    /* We must NOT use strncpy because it will null pad to the full length
       (nlen).  If the user has not allocated MPI_MAX_PROCESSOR_NAME chars,
       then this will unnecessarily overwrite storage.
     */
    /* strncpy(name,utname.nodename,nlen); */
    {
	char *p_out = name;
	char *p_in  = utname.nodename;
	int  i;
	for (i=0; i<nlen-1 && *p_in; i++) 
	    *p_out++ = *p_in++;
	*p_out = 0;
    }
#elif defined(HAVE_GETHOSTNAME)
    gethostname( name, nlen );
#elif defined(HAVE_SYSINFO)
    sysinfo(SI_HOSTNAME, name, nlen);
#else 
    strncpy( name, "Unknown!", nlen );
#endif
/* See if this name includes the domain */
    if (!strchr(name,'.')) {
    int  l;
    l = strlen(name);
    name[l++] = '.';
    name[l] = 0;  /* In case we have neither SYSINFO or GETDOMAINNAME */
#if defined(HAVE_SYSINFO) && defined(SI_SRPC_DOMAIN)
    sysinfo( SI_SRPC_DOMAIN,name+l,nlen-l);
#elif defined(HAVE_GETDOMAINNAME)
    getdomainname( name+l, nlen - l );
#endif
    }
#endif
#ifdef FOO
#if defined(solaris)
  struct utsname utname;
  uname(&utname);
  strncpy(name,utname.nodename,nlen);
#else
/*  sysinfo(SI_HOSTNAME, name, nlen); */
  gethostname(name, nlen);
#endif
/* See if this name includes the domain */
  if (!strchr(name,'.')) {
    int  l;
    l = strlen(name);
    name[l++] = '.';
#if defined(solaris)
    sysinfo( SI_SRPC_DOMAIN,name+l,nlen-l);
#else
    getdomainname( name+l, nlen - l );
#endif
  }
#endif
}
#else
void SYGetHostName( char *name, nlen )
{
#if defined(intelnx)
#if defined(inteldelta)
strncpy( name, "IntelDelta", nlen );
#else
strncpy( name, "IntelNX", nlen );
#endif
#else
strncpy( name, "Unknown", nlen );
#endif
}
#endif 

#if !defined(__MSDOS__) && !defined(WIN32)
int SYiTestFile ANSI_ARGS(( char *, char, uid_t, gid_t ));

/*+
  SYiTestFile - Test for a file existing with a specified mode.

  Input Parameters:
. fname - name of file
. mode  - mode.  One of 'r', 'w', 'e'
. uid,gid - user and group id to use

  Returns:
  1 if file exists with given mode, 0 otherwise.
+*/
int SYiTestFile( char *fname, char mode, uid_t uid, gid_t gid )
{
int         err;
struct stat statbuf;
int         stmode, rbit, wbit, ebit;

if (!fname) return 0;

/* Check to see if the environment variable is a valid regular FILE */
err = stat( fname, &statbuf );
if (err != 0) return 0;

/* At least the file exists ... */
stmode = statbuf.st_mode;
#if defined(rs6000) || defined(HPUX)
#define S_IFREG _S_IFREG
#endif
/* if (!S_ISREG(stmode)) return 0; */
#if defined(intelparagon)
if (!S_ISREG(stmode)) return 0;
#else
if (!(S_IFREG & stmode)) return 0;
#endif
/* Test for accessible. */
if (statbuf.st_uid == uid) {
    rbit = S_IRUSR;
    wbit = S_IWUSR;
    ebit = S_IXUSR;
    }
else if (statbuf.st_gid == gid) {
    rbit = S_IRGRP;
    wbit = S_IWGRP;
    ebit = S_IXGRP;
    }
else {
    rbit = S_IROTH;
    wbit = S_IWOTH;
    ebit = S_IXOTH;
    }
if (mode == 'r') {
    if ((stmode & rbit)) return 1;
    }
else if (mode == 'w') {
    if ((stmode & wbit)) return 1;
    }
else if (mode == 'e') {
    if ((stmode & ebit)) return 1;
    }
return 0;
}
int SYiFileExists( char *fname, char mode )
{
    static int set_ids = 0;
    static uid_t uid;
    static gid_t gid;
    int         err;
    struct stat statbuf;
    int         stmode, rbit, wbit, ebit;

    if (!fname) return 0;

    if (!set_ids) {
	uid = getuid();
	gid = getgid();
	set_ids = 1;
    }

/* Check to see if the environment variable is a valid regular FILE */
    err = stat( fname, &statbuf );
    if (err != 0) return 0;

/* At least the file exists ... */
    stmode = statbuf.st_mode;
#if defined(rs6000) || defined(HPUX)
#define S_IFREG _S_IFREG
#endif
/* if (!S_ISREG(stmode)) return 0; */
#if defined(intelparagon)
if (!S_ISREG(stmode)) return 0;
#else
if (!(S_IFREG & stmode)) return 0;
#endif
/* Test for accessible. */
if (statbuf.st_uid == uid) {
    rbit = S_IRUSR;
    wbit = S_IWUSR;
    ebit = S_IXUSR;
    }
else if (statbuf.st_gid == gid) {
    rbit = S_IRGRP;
    wbit = S_IWGRP;
    ebit = S_IXGRP;
    }
else {
    rbit = S_IROTH;
    wbit = S_IWOTH;
    ebit = S_IXOTH;
    }
if (mode == 'r') {
    if ((stmode & rbit)) return 1;
    }
else if (mode == 'w') {
    if ((stmode & wbit)) return 1;
    }
else if (mode == 'e') {
    if ((stmode & ebit)) return 1;
    }
return 0;
}
#else
int SYiTestFile( fname, mode, uid, gid )
char  *fname, mode;
uid_t uid;
gid_t gid;
{
int         err;
struct stat statbuf;
int         stmode, rbit, wbit, ebit;

if (!fname) return 0;

/* Check to see if the environment variable is a valid regular FILE */
err = stat( fname, &statbuf );
if (err != 0) return 0;

/* At least the file exists ... */
stmode = statbuf.st_mode;
if (!(S_IFREG & stmode)) return 0;
/* Test for accessible. */
rbit = S_IREAD;
wbit = S_IWRITE;
ebit = S_IEXEC;
if (mode == 'r') {
    if ((stmode & rbit)) return 1;
    }
else if (mode == 'w') {
    if ((stmode & wbit)) return 1;
    }
else if (mode == 'e') {
    if ((stmode & ebit)) return 1;
    }
return 0;
}
#endif /* MSDOS */

#ifndef __MSDOS__
/*@
   SYGetFileFromPath - Find a file from a name and an path string
                              A default may be provided.

   Input Parameters:
.  path - A string containing "directory:directory:..." (without the
	  quotes, of course).
	  As a special case, if the name is a single FILE, that file is
	  used.

.  defname - default name
.  name - file name to use with the directories from env
.  mode - file mode desired (usually 'r' for readable; 'w' for writable and
          'e' for executable are also supported)

   Output Parameter:
.  fname - qualified file name

    Returns:
    1 on success, 0 on failure.
@*/
int SYGetFileFromPath( char *path, char *defname, char *name, char *fname, 
		       char mode )
{
    char   *p, *cdir;
    int    ln;
    uid_t  uid;
    gid_t  gid;
    char   trial[MAX_FILE_NAME];
    char   *senv, *env;

/* Setup default */
    SYGetFullPath(defname,fname,MAX_FILE_NAME);

/* Get the (effective) user and group of the caller */
    uid = geteuid();
    gid = getegid();

    if (path) {

/* Check to see if the path is a valid regular FILE */
	if (SYiTestFile( path, mode, uid, gid )) {
	    strcpy( fname, path );
	    return 1;
	}
    
/* Make a local copy of path and mangle it */
	senv = env = (char *)MALLOC( strlen(path) + 1 ); 
	if (!senv) { fprintf( stderr, "Error allocating memory\n" ); return 0; }
	strcpy( env, path );
	while (env) {
	    /* Find next directory in env */
	    cdir = env;
	    p    = strchr( env, ':' );
	    if (p) {
		*p  = 0;
		env = p + 1;
	    }
	    else
		env = 0;

	    /* Form trial file name */
	    strcpy( trial, cdir );
	    ln = strlen( trial );
	    if (trial[ln-1] != '/') 
		trial[ln++] = '/';
	
	    strcpy( trial + ln, name );

	    if (SYiTestFile( trial, mode, uid, gid )) {
		/* need SYGetFullPath rather then copy in case path has . in it */
		SYGetFullPath( trial,  fname, MAX_FILE_NAME );
		FREE( senv );
		return 1;
	    }
	}

	FREE( senv );
    } /* end of if path */

    if (SYiTestFile( fname, mode, uid, gid )) return 1;
    else return 0;
}
#else
/* MSDOS version does not use uid, gid */
int SYGetFileFromPath( path, defname, name, fname, mode )
char   *path, *defname, *name, *fname, mode;
{
char   *p, *cdir;
int    ln;
char   trial[MAX_FILE_NAME];
char   *senv, *env;
uid_t  uid;
gid_t  gid;

/* Setup default */
SYGetFullPath(defname,fname,MAX_FILE_NAME);

if (path) {

/* Check to see if the path is a valid regular FILE */
if (SYiTestFile( path, mode, uid, gid )) {
    strcpy( fname, path );
    return 1;
    }
    
/* Make a local copy of path and mangle it */
senv = env = (char *)MALLOC( strlen(path) + 1 ); 
if (!senv) { fprintf( stderr, "Error allocating memory\n" ); return 0; }
strcpy( env, path );
while (env) {
    /* Find next directory in env */
    cdir = env;
    p    = strchr( env, ':' );
    if (p) {
	*p  = 0;
	env = p + 1;
	}
    else
	env = 0;

    /* Form trial file name */
    strcpy( trial, cdir );
    ln = strlen( trial );
    if (trial[ln-1] != '/') 
	trial[ln++] = '/';
	
    strcpy( trial + ln, name );

    if (SYiTestFile( trial, mode, uid, gid )) {
        /* need SYGetFullPath rather then copy in case path has . in it */
	SYGetFullPath( trial,  fname, MAX_FILE_NAME );
	FREE( senv );
	return 1;
	}
    }

FREE( senv );
} /* end of if path */

if (SYiTestFile( fname, mode, uid, gid )) return 1;
else return 0;
}
#endif /* __MSDOS__ */

/*@
   SYGetFileFromEnvironment - Find a file from a name and an environment
                              variable.  A default may be provided.

   Input Parameters:
.  env  - name of environment variable.  The contents of this variable
          should be of the form "directory:directory:..." (without the
	  quotes, of course).
	  As a special case, if the name is a single FILE, that file is
	  used.

.  defname - default name
.  name - file name to use with the directories from env
.  mode - file mode desired (usually 'r' for readable; 'w' for writable and
          'e' for executable are also supported)

   Output Parameter:
.  fname - qualified file name

    Returns:
    1 on success, 0 on failure.
@*/
int SYGetFileFromEnvironment( char *env, char *defname, char *name, 
			      char *fname, char mode ) 
{
    char   *edir;

    /* Get the environment values */
    edir = getenv( env );

    return SYGetFileFromPath( edir, defname, name, fname, mode );
}

/*@
   SYOpenWritableFile - Open a writeable file, given a directory path and
                        a filename.

   Input Parameters:
.  dirpath - The list of directories. The contents of this variable
          should be of the form "directory:directory:..." (without the
	  quotes, of course).
.  defname - default name (unused for now)
.  name - file name to use with the directories from env
.  istmp - if 1, use mktemp to generate a file name.  "name" must be in the 
           correct form for mktemp (six trailing X's).

   Output Parameter:
.  fname - qualified file name

   Returns:
   Pointer to open FILE.
@*/
FILE *SYOpenWritableFile( dirpath, defname, name, fname, istmp )
char *dirpath, *defname, *name, *fname;
int  istmp;
{
char   *p, *cdir;
int    ln;
FILE   *fp;

while (dirpath && *dirpath) {
    /* Find next directory in env */
    cdir = dirpath;
    p    = strchr( dirpath, ':' );
    if (p) {
	dirpath = p + 1;
	}
    else {
	p       = dirpath + strlen(dirpath);
	dirpath = 0;
	}

    /* Form trial file name */
    strncpy( fname, cdir, (int)(p-cdir) );
    fname[(int)(p-cdir)] = 0;
    ln = strlen( fname );
    if (fname[ln-1] != '/') 
	fname[ln++] = '/';
	
    strcpy( fname + ln, name );
    if (istmp) 
	fname = mktemp( fname );

    /* Form full path name */
    /* strcpy( path, fname ); */
    /* SYGetFullPath( path, fname, MAX_FILE_LEN ); */

    /* Is the file accessible? */
    /* fprintf( stderr, "Trying filename %s\n", fname ); */
    fp = fopen( fname, "w" );
    if (fp) return fp;
    }

/* Try the default name */
if (istmp)
    fname = mktemp( fname );
fp = fopen( fname, "w" );

return fp;
}

/*@
  SYGetwd - Get the current working directory

  Input paramters:
. path - use to hold the result value
. len  - maximum length of path
@*/
void SYGetwd( path, len )
char *path;
int  len;
{
#if defined(tc2000) || (defined(sun4) && !defined(solaris))
getwd( path );
#elif defined(__MSDOS__) || defined(WIN32)
/* path[0] = 'A' + (_getdrive() - 1);
path[1] = ':';
_getcwd( path + 2, len - 2 );
 */
#if defined(__TURBOC__)
getcwd( path, len );
#else 
_getcwd( path, len );
#endif
#else
getcwd( path, len );
#endif
}

#if !defined(__MSDOS__) && !defined(WIN32)
#include <sys/param.h>
#endif
#ifndef MAXPATHLEN
/* sys/param.h in intelnx does not include MAXPATHLEN! */
#define MAXPATHLEN 1024
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
char *SYGetRealpath( path, rpath )
char *path, *rpath;
{
#if defined(sun4)
extern char *realpath();
realpath( path, rpath );

#elif defined(intelnx) || defined(__MSDOS__) || defined(WIN32)
strcpy( rpath, path );

#else
#if defined(IRIX)
extern char *strchr();
#endif
#ifdef FOO
  int  n, m, N;
  char tmp1[MAXPATHLEN], tmp3[MAXPATHLEN], tmp4[MAXPATHLEN], *tmp2;
#endif
  /* Algorithm: we move through the path, replacing links with the
     real paths.
   */
  strcpy( rpath, path );
#ifdef FOO  
  /* THIS IS BROKEN.  IT CAUSES INFINITE LOOPS ON IRIX, BECAUSE
     THE CODE ON FAILURE FROM READLINK WILL NEVER SET N TO ZERO */
  N = strlen(rpath);
  while (N) {
    strncpy(tmp1,rpath,N); tmp1[N] = 0;
    n = readlink(tmp1,tmp3,MAXPATHLEN);
    if (n > 0) {
      tmp3[n] = 0; /* readlink does not automatically add 0 to string end */
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
#endif
  if (strncmp( "/tmp_mnt/", rpath, 9 ) == 0) {
      char tmp3[MAXPATHLEN];
      strcpy( tmp3, rpath + 8 );
      strcpy( rpath, tmp3 );
      }
  return rpath;
}

#if !defined(__MSDOS__) && !defined(WIN32)
/*@
     SYRemoveHomeDir - Given a complete true path, remove user's home directory

   Input Parameter:
.  path - path to be possibly modified

   Note:
   Given a complete path (for instance by SYGetRealpath), if the path begins 
   with the users home directory, the user's home directory is removed.

   This is useful on systems where a users home directory may have
   different names on different machines, or different names each time
   one logs in (such systems exist!).
@*/
void SYRemoveHomeDir( path )
char *path;
{
  struct passwd *pw = 0;
  char          tmp1[MAXPATHLEN], tmp2[MAXPATHLEN], *d1, *d2;
  int           len;

  pw = getpwuid( getuid() );
  if (!pw)  return;
  strcpy(tmp1, pw->pw_dir);
  SYGetRealpath( tmp1, tmp2 );

  /* some have /tmp_mnt; others don't */
  if (!strncmp(path,"/tmp_mnt",8)) d1 = path + 8; else d1 = path;
  if (!strncmp(tmp2,"/tmp_mnt",8)) d2 = tmp2 + 8; else d2 = tmp2;
  if (strncmp(d1,d2,strlen(d2))) return;
  len = strlen(d2) + 1;
  strcpy(tmp1,d1+len); /* we know these will fit */
  strcpy(path,tmp1);
}
#else
void SYRemoveHomeDir( path )
char *path;
{
}
#endif /* MSDOS */

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif
/*@
     SYIsMachineHost - returns true if name passed in matches host
                       name else false.
 
  Input Paramters:
.  machinename - Name of the machine to test
 
@*/
int SYIsMachineHost( machinename )
char *machinename;
{
  static char localhost[MAXHOSTNAMELEN];
  static int  hostlength = 0;
  int         namelen;
  char        *tmp;
 
  if (!hostlength) {
    SYGetHostName(localhost,MAXHOSTNAMELEN);
    hostlength = strlen(localhost);
#if !defined(RELIABLEHOSTNAME)
    if ((tmp = strchr(localhost,'.'))) {
      hostlength = strlen(localhost) - strlen(tmp);
    }
#endif
  }
#if !defined(RELIABLEHOSTNAME)
  /* Only test leading characters of the localname against the leading 
     characters of the host */
  if ((tmp = strchr( machinename, '.' ))) 
      namelen = strlen( machinename ) - strlen( tmp );
  else
      namelen = strlen( machinename );
  return (namelen == hostlength && !strncmp(machinename,localhost,hostlength));
#else 
  return !strncmp(machinename,localhost,hostlength);
#endif
}

#ifndef S_ISDIR
#define	S_ISDIR(m)	(((m)&S_IFMT) == S_IFDIR)
#endif
/*@
  SYIsDirectory - Tests to see if a filename is a directory.

  Input Parameters:
. fname - name of file

  Returns:
  1 if file is a directory, 0 otherwise
@*/
int SYIsDirectory( fname )
char  *fname;
{
int         err;
struct stat statbuf;
int         stmode;

if (!fname) return 0;

err = stat( fname, &statbuf );
if (err != 0) return 0;

/* At least the file exists ... */
stmode = statbuf.st_mode;
return S_ISDIR(stmode);
}
#ifndef __MSDOS__

#include <fcntl.h>

/*@
  SYMakeAllDirs - Make all of the directories in the file name

  Input Parameter:
. name - name of file.  The last element is a file NAME, not a directory

  Notes:
  This routine attempts to create the intervening directories if they
  do not exist.
@*/
void SYMakeAllDirs( name, fmode )
char *name;
int  fmode;
{
char        *p, *pn;
char        dirname[1024];
int         err;
struct stat statbuf;

strcpy( dirname, name );
p = dirname;
/* Make sure that fmode has x bits set */
if (! (fmode & (S_IXUSR | S_IXGRP | S_IXOTH))) {
    fmode |= (S_IXUSR | S_IXGRP | S_IXOTH);
    }
while (*p) {
    /* find the next component */
    pn = p + 1;
    while (*pn && *pn != '/') pn++;
    if (*pn != '/') break;
    *pn = 0;
    err = stat( dirname, &statbuf );
    if (err != 0) {
	err = mkdir( dirname, fmode );
	if (err < 0) {
	    fprintf( stderr, "Failed to make directory %s", dirname );
	    return;
	    }
	}
    *pn = '/';
    p   = pn + 1;
    }
}

#include <time.h>   /*I <time.h> I*/
/* Get the date that a file was last changed */
/*@
    SYLastChangeToFile - Gets the date that a file was changed as a string or
    timeval structure

    Input Parameter:
.   fname - name of file

    Output Parameters:
.   date - string to hold date (may be null)
.   ltm   - tm structure for time (may be null)
@*/
void SYLastChangeToFile( char *fname, char *date, struct tm *ltm )
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

/* Return 1 if newfile is newer than oldfile */
/* Return 0 if newfile does not exist, and 1 if oldfile does not exist */
int SYFileNewer( char *newfile, char *oldfile )
{
    struct stat new_st, old_st;

    if (stat( newfile, &new_st )) return 0;
    if (stat( oldfile, &old_st )) return 1;
    
    return (new_st.st_mtime > old_st.st_mtime);
}

/* 
   These routines may be used to get exclusive access to a file; they
   replace fopen and fclose.

   We have to fcntl to insure that NFS-mounted files are locked.
 */

/* Experimentation on the SP-1 at ANL shows that fcntl does not work under
   AIX */
#if !defined(rs6000) && !defined(intelnx)
#define FCNTL_WORKS
#include <fcntl.h>
#endif

#ifndef FCNTL_WORKS
static char *lockfile = 0;
static int  lockfilefd = -1;
#endif

FILE *SYfopenLock( char *name, char *type )
{
FILE *fp;
int  fd;
int  cnt;
#ifdef FCNTL_WORKS
struct flock FL;
#endif

fp = fopen( name, type );
if (!fp) return 0;

#ifdef FCNTL_WORKS
fd = fileno(fp);

FL.l_type   = F_WRLCK;
FL.l_whence = SEEK_SET;
FL.l_start  = 0;
FL.l_len    = 0;
/*
fprintf( stdout, "About to lock file %s\n", name ); fflush( stdout );
 */
/* 
   This code attempts to avoid deadlock by not using blocking lock 
   requests.  However, it requires correct behavior by the operating 
   system.  AIX 3.2.4, as a counter-example, seems to block on these 
   non-blocking lock requests (on the remote nodes).  Sigh... 
 */
if (fcntl( fd, F_SETLK, &FL ) == -1) {
    /*
    fprintf( stdout, "Could not lock file %s; trying again...\n", name );
    fflush( stdout );
     */
    cnt = 10;
    while (cnt--) {
	sleep(5);
	if (fcntl( fd, F_SETLK, &FL ) != -1) break;
	}
    if (!cnt) {
	fprintf( stdout, "Could not lock file %s\n", name );
	fclose( fp );
	return 0;
	}
    }
/*
fprintf( stdout, "Locked file %s\n", name );
fflush( stdout );
 */
#else
/* We use a file-based locking mechanism here.  Only one file may be locked
   with this system */
if (lockfile) {
    fclose( fp );
    return 0;
    }
lockfile = (char *)MALLOC( strlen(name) + 6 );  CHKPTRN(lockfile);
strcpy( lockfile, name );
strcat( lockfile, ".lock" );
cnt = 10;
while (cnt--) {
    lockfilefd = open( lockfile, O_CREAT | O_WRONLY | O_TRUNC | O_EXCL, 0644 );
    if (lockfilefd >= 0) break;
    sleep( 5 );
    }
if (!cnt) {
    fprintf( stdout, "Could not lock file %s\n", name );
    fclose( fp );
    return 0;
    }
#endif
return fp;
}
void SYfcloseLock( FILE *fp )
{
int fd;
#ifdef FCNTL_WORKS
struct flock FL;
#endif

if (!fp) return;
#ifdef FCNTL_WORKS
fd = fileno(fp);

FL.l_type   = F_UNLCK;
FL.l_whence = SEEK_SET;
FL.l_start  = 0;
FL.l_len    = 0;
fflush( fp );
/* fprintf( stdout, "About to unlock file\n" ); fflush( stdout ); */
/* I should not have to wait since I have the lock */
fcntl( fd, F_SETLK, &FL );
fclose( fp );
#else
/* We use a file-based locking mechanism here */
fclose( fp );
close( lockfilefd );
lockfilefd = -1;
unlink( lockfile );
FREE( lockfile );
lockfile = 0;
#endif
}

#else
#include <time.h>
/* MSDOS code for makedirs and last change should go here */

/* This version of MakeAllDirs knows about '\' instead of '/'.
   It also doesn't bother with fmode for now */
void SYMakeAllDirs( name, fmode )
char *name;
int  fmode;
{
char        *p, *pn;
char        dirname[1024];
int         err;
struct stat statbuf;

strcpy( dirname, name );
p = dirname;
while (*p) {
    /* find the next component */
    pn = p + 1;
    while (*pn && *pn != '/' && *pn != '\\') pn++;
    if (*pn != '/' && *pn != '\\') break;
    *pn = 0;
    err = stat( dirname, &statbuf );
    if (err != 0) {
	err = mkdir( dirname, fmode );
	if (err < 0) {
	    fprintf( stderr, "Failed to make directory %s\n", dirname );
	    return;
	    }
	}
    *pn = '\\';
    p   = pn + 1;
    }
}
void SYLastChangeToFile( char *fname, char *date, struct tm *ltm )
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
#endif

/* Return 1 if the file exists.  If fname does not exist, try fname
   with each of the extensions (colon separated) in extensions.
   The extensions do not include the "." */
int SYFindFileWithExtension( char *fname, const char *extensions )
{
    char *extLoc;
    const char *p;

    if (SYiFileExists( fname, 'r' ) == 1) {
	return 1;
    }
    extLoc = fname + strlen(fname);
    *extLoc++ = '.';
    p = extensions;
    while (*p) {
	int i=0;
	while (*p && *p != ':') {
	    extLoc[i++] = *p++;
	}
	if (*p == ':') p++;
	extLoc[i] = 0;
	/* printf( "Checking for file %s\n", fname ); */
	if (SYiFileExists( fname, 'r' ) == 1) {
	    return 1;
	}
    }
    return 0;
}
