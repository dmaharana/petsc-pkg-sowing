#ifndef QUOTEFMT
#define QUOTEFMT

#include "textout.h"

class TextOutQFmt : public TextOut {
	int tt_cnt, em_cnt;
	int last_tt, last_em;
	char tt_char, em_char;
	int action_flag;
	public:
	TextOutQFmt( TextOut * );
	const char *SetMode( const char * );
	int PutChar( const char );
	int PutToken( int, const char * );
	int PutOp( const char * );
	int PutOp( const char *, char *, char *, int );
	int PutOp( const char *, char *, char *, char *, char * );
	int PutOp( const char *, char * );
	int PutOp( const char *, char *, int );
	int Flush( );
	int SetAction( int );  // 1 for use quote formating, 0 for don't
    };
#endif
