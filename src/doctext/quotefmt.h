#ifndef QUOTEFMT
#define QUOTEFMT

#include "textout.h"

class TextOutQFmt : public TextOut {
	int tt_cnt, em_cnt;
	int last_tt, last_em;
	char tt_char, em_char;
	public:
	TextOutQFmt( TextOut * );
	const char *SetMode( const char * );
	int PutChar( const char );
	int PutToken( int, const char * );
	int PutOp( const char * );
	int Flush( );
    };
#endif
