#ifndef SRLIST
#define SRLIST

struct _SrEntry {
    char            *name;
    void            *extra_data; 
    struct _SrEntry *next;
    };
typedef struct _SrEntry SrEntry;

class SrList {
    int     hash_size;
    SrEntry **table;
    int HashFn( const char * );

    public:
    SrList( );
    SrList( int );
    ~SrList();
    int Lookup( const char *, SrEntry **);
    int Insert( const char *, SrEntry **);
    int Delete( const char * );
    int Walk( int (*)( const char *, SrEntry * ) );
    };
#endif
