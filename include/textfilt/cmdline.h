#ifndef CMDLINE
#define CMDLINE

/* 
   This is a class for accessing the command line.  Eventually, we may
   add other methods, for example, a table driven approach that 
   reads all arguments and stores them in a table.  That's why the
   functions are virtual.
 */
class CmdLine {
    
    int SqueezeArg( );

    public:
    char **argv;
    int  argc;
    int  cnt;

    CmdLine( int, char ** );
    virtual int FindArg( const char * );
    virtual int HasArg( const char * );
    virtual int GetArg( const char *, int * );
    virtual int GetArg( const char *, char *, int );
    virtual int GetArgPtr( const char *, const char ** );
    virtual int GetList( const char *, int, int * );
    virtual int GetList( const char *, int, char ** );
    virtual int NextArg( const char ** );
    virtual ~CmdLine();
    };
#endif
