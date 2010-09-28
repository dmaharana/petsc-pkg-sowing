/*
 * OutStream - Manages an output stream which could go to a file or memory
 */

#ifndef OUTSTREAM
#define OUTSTREAM

/*
 * OutStream is the base class.  The derived classes for different kinds
 * of OutStreams 
 */
class OutStream {
//    /* Private data for stream */
//    void     *extra_data;

    protected:
    int      bufmode;
    int      debug_flag;

    public:
    int      status; /* 0 for ok, errno for error, -1 for other error */
    OutStream *next;       /* Next stream in chain */
    virtual ~OutStream();
    virtual int PutChar( const char );
    virtual int PutToken( int, const char * );
    virtual int PutQuoted( int, const char * );
    virtual int PutInt( int );
    virtual void Flush( void );
    virtual void SetBufMode( int );
    virtual int Close( void );
    virtual int Debug( int );
    virtual const char *MyName( void );
    };

/*
 * Derived classes.  
 * WARNING: You'd think that deriving a class from another with public would
 * allow you to import the public functions from the base class without having
 * to redeclare them.  You'd be wrong... .  Note that you DO get the variables!
 */
#include <stdio.h>
class OutStreamFile : public OutStream {
    FILE *fp;

    public:
    OutStreamFile( void );
    ~OutStreamFile( void );
    OutStreamFile( const char *, const char * );
    int PutChar( const char );
    void Flush( void );
    int Close( void );
    const char *MyName( void );
    };

class OutStreamBuf : public OutStream {
    char *buffer, *position;
    int  curlen, maxlen;

    public:
    OutStreamBuf( int );
    ~OutStreamBuf( void );
    int PutChar( const char );
    char *GetBuffer( void );
    void Reset( void );
    const char *MyName( void );
    };

#endif
