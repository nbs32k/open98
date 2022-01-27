#include "utill.h"


#define PI 3.141592653589793238462
// fast isqrt method, skidded from quake3 algorithm

// error of 0.2% - 2%, but who cares
FLOAT fast_isqrt( FLOAT x )
{
	LONG i;
	FLOAT x2, y;
	const FLOAT threehalfs = 1.5F;

	x2 = x * 0.5F;
	y = x;
	i = *( LONG* )&y;  // evil FLOATing poINT bit level hacking
	i = 0x5f3759df - ( i >> 1 ); // what the fuck? 
	y = *( FLOAT* )&i;
	y = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
	// y = * (threehalfs - (x2*y*y)); can be removed
	return y;
}

FLOAT sqrt( FLOAT x )
{
	return fast_isqrt( x ) * x;
}

FLOAT hypot( FLOAT x, FLOAT y )
{
	// hypot = sqrt(x² + y²)
	return sqrt( x*x + y * y );
}

// sin, cos, tan

FLOAT fabs( FLOAT x )
{
	return x < 0 ? -x : x;
}

static FLOAT sin_table[ 1024 ];

VOID 
KiInitSinTable(

)
{
	for ( INT n = 0; 1024 > n; n++ )
	{
		FLOAT x = n / 1024. * PI;
		INT i = 2;
		FLOAT cur = x;
		FLOAT acc = 1;
		FLOAT fact = 1;
		FLOAT pow = x;
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

FLOAT sin( FLOAT x )
{
	INT index = ( INT )( x * ( 1. / ( PI / 1024 ) ) );
	return
		( ( index & 0x400 ) != 0 ? -1 : 1 ) *
		sin_table[ index & 1023 ];
}

FLOAT cos( FLOAT x )
{
	return sin( PI / 2. + x );
}

FLOAT tan( FLOAT x )
{
	return sin( x ) / cos( x );
}