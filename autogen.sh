#! /bin/sh
# Allow autogen.sh to be used within the dist target
#svnco=co
#if test "X$1" = "Xexport" ; then svnco=export ; fi
# runs autoheader and autoconf with local macros
echo making configure in `pwd`
# autoconf doesn't correctly determine when the cache values are not
# correct, so we must always delete the cache file
#
# This old code to share the autoconf macros can no longer be used; note also
# that some groups selfishly deleted some of the elements that they did not
# use, ignoring the use of these by sowing and other projects (sigh).
#svn=svn
#if [ -n "$SVN" ] ; then svn=$SVN ; fi
rm -rf autom4te.cache
#if [ ! -d confdb ] ; then
#   $svn $svnco https://svn.mcs.anl.gov/repos/mpi/mpich2/trunk/confdb
#else
#   $svn update
#fi
if autoconf -I config/confdb ; then
    echo "autoconf appears to be the new version"
    autoheader -I config/confdb
    if grep PAC_ configure >/dev/null 2>&1 ; then
        echo "Unable to find a PAC macro:"
	grep PAC_ configure
    fi
else
    echo "Could not build configure"
fi
# Ensure that the config.sub, config.guess, etc. scripts are executable
#chmod a+x config/config.sub config/config.guess config/install-sh \
 #  config/missing config/mkinstalldirs
