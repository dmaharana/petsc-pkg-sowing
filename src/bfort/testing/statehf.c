/* ../system/state.h */
/* Fortran interface file for sun4 */
#include "system/state.h"
void  systatesetroutine_( state, r, ctx)
    SYstate *state;
    void    (*r)(), *ctx;
{
SYStateSetRoutine(
	(SYstate* )*((int*)state),r,ctx);
}
