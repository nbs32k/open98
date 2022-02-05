#include "utill.h"
/*

#define PI 3
// fast isqrt method, skidded from quake3 algorithm

// error of 0.2% - 2%, but who cares
INT fast_isqrt( INT x )
{
	LONG i;
	INT x2, y;
	const INT threehalfs = 1.5F;

	x2 = x * 0.5F;
	y = x;
	i = *( LONG* )&y;  // evil INTing poINT bit level hacking
	i = 0x5f3759df - ( i >> 1 ); // what the fuck? 
	y = *( INT* )&i;
	y = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
	// y = * (threehalfs - (x2*y*y)); can be removed
	return y;
}

INT sqrt( INT x )
{
	return fast_isqrt( x ) * x;
}

INT hypot( INT x, INT y )
{
	// hypot = sqrt(x² + y²)
	return sqrt( x*x + y * y );
}

// sin, cos, tan

INT fabs( INT x )
{
	return x < 0 ? -x : x;
}

static INT sin_table[ 1024 ];

VOID 
KiInitSinTable(

)
{
	for ( INT n = 0; 1024 > n; n++ )
	{
		INT x = n / 1024. * PI;
		INT i = 2;
		INT cur = x;
		INT acc = 1;
		INT fact = 1;
		INT pow = x;
		while ( fabs( acc ) > .000001 && i < 200 )
		{
			fact *= i * ( i + 1 );
			pow *= -x * x;
			acc = pow / fact;
			cur += acc;
			i += 2;
		}
		sin_table[ n ] = cur;
	}
}

INT sin( INT x )
{
	INT index = ( INT )( x * ( 1. / ( PI / 1024 ) ) );
	return
		( ( index & 0x400 ) != 0 ? -1 : 1 ) *
		sin_table[ index & 1023 ];
}

INT cos( INT x )
{
	return sin( PI / 2. + x );
}

INT tan( INT x )
{
	return sin( x ) / cos( x );
}*/