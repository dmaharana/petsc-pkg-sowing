#ifndef ERRHAND
#define ERRHAND

#include <stdio.h>

class ErrHand {

	public:
	virtual int ErrMsg( int, const char * );
        virtual ~ErrHand();
};

class ErrHandMsg : public ErrHand {
	FILE *errout;

	public:
	ErrHandMsg( );
	ErrHandMsg( const char *path );
	~ErrHandMsg( );
	int ErrMsg( int, const char * );
};

class ErrHandReturn : public ErrHand {
	public:
	ErrHandReturn( );
	~ErrHandReturn( );
	int ErrMsg( int, const char *);
};
#endif
