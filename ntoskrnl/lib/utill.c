#include "utill.h"
int k_toupper( int c )
{
	if ( c >= 97 && c <= 122 )
	{
		return c - 32;
	}
	return c;
}
int k_tolower( int c )
{
	if ( c >= 65 && c <= 90 )
	{
		return c + 32;
	}
	return c;
}

int strncmp( const char *s1, const char *s2, INT n )
{
	for ( INT i = 0; i < n; i++ )
	{
		char c1 = s1[ i ], c2 = s2[ i ];
		if ( c1 != c2 )
			return c1 - c2;
		if ( !c1 )
			return 0;
	}

	return 0;
}

int memcmp( const void *s1, const void *s2, INT n )
{
	const UCHAR *p1 = ( const UCHAR * )s1;
	const UCHAR *p2 = ( const UCHAR * )s2;

	for ( INT i = 0; i < n; i++ )
	{
		if ( p1[ i ] != p2[ i ] )
			return p1[ i ] < p2[ i ] ? -1 : 1;
	}

	return 0;
}

int pow( int base, int exp )
{
	int result = 1;
	for ( exp; exp > 0; exp-- )
	{
		result = result * base;
	}
	return result;
}

void str_cpy( UCHAR* src, UCHAR* dest, UCHAR count )
{
	for ( int i = 0; i < count; i++ )
	{
		dest[ i ] = src[ i ];
	}
}

PCHAR strcpy( PCHAR destination, const char* source )
{
	if ( destination == NULL )
	{
		return NULL;
	}
	PCHAR ptr = destination;
	while ( *source != '\0' )
	{
		*destination = *source;
		destination++;
		source++;
	}
	*destination = '\0';
	return ptr;
}

PCHAR strcat( PCHAR destination, const char* source )
{
	PCHAR ptr = destination + strlen( destination );
	while ( *source != '\0' )
	{
		*ptr++ = *source++;
	}
	*ptr = '\0';
	return destination;
}

int strcmp( const char* a, const char* b )
{
	while ( *a && *a == *b ) { ++a; ++b; }
	return ( int )( unsigned char )( *a ) - ( int )( unsigned char )( *b );
}

PCHAR strstr( const char *str, const char *substring )
{
	const char *a = str, *b = substring;
	for ( ;;)
	{
		if ( !*b ) return ( char * )str;
		if ( !*a ) return NULL;
		if ( *a++ != *b++ ) { a = ++str; b = substring; }
	}
}

void *memset( void *s, int c, INT n )
{
	UCHAR *s_uint8 = ( UCHAR * )s;
	for ( INT i = 0; i < n; i++ )
	{
		s_uint8[ i ] = ( UCHAR )c;
	}
	return s;
}

void *memcpy( void *dest, const void *src, INT n )
{
	const UCHAR *src_uint8 = ( const UCHAR * )src;
	UCHAR *dest_uint8 = ( UCHAR * )dest;
	for ( INT i = 0; i < n; i++ )
	{
		dest_uint8[ i ] = src_uint8[ i ];
	}
	return dest;
}


void reverse( char s[ ] )
{
	int c, i, j;
	for ( i = 0, j = strlen( s ) - 1; i < j; i++, j-- )
	{
		c = s[ i ];
		s[ i ] = s[ j ];
		s[ j ] = c;
	}
}

PCHAR int_to_string( int num )
{
	BOOLEAN isMinus = FALSE;
	static char out[ 10 ];
	int g = 0;
	if ( num != 0 )
	{
		char temp[ 10 ];
		int i = 0;
		if ( num < 0 )
		{
			isMinus = TRUE;
			num = -num;
		}
		if ( num > 0 ); else { temp[ i++ ] = '8'; num = -( num / 10 ); }
		while ( num > 0 )
		{
			temp[ i++ ] = num % 10 + '0';
			num /= 10;
		}
		if ( isMinus )
		{
			out[ g ] = '-';
			g++;
		}
		while ( --i >= 0 )
		{
			out[ g ] = temp[ i ];
			g++;
		}
		return out;
	}
	else
	{
		return "0";
	}
}

int string_to_int( PCHAR str )
{
	int res = 0;
	for ( int i = 0; str[ i ] != '\0'; ++i )
	{
		res = res * 10 + str[ i ] - '0';
	}
	return res;
}
void memmove( void* dest, void* src, INT n )
{
	char *csrc = ( char * )src;
	char *cdest = ( char * )dest;
	char temp[ n ];
	for ( int i = 0; i < n; i++ )
		temp[ i ] = csrc[ i ];

	for ( int i = 0; i < n; i++ )
		cdest[ i ] = temp[ i ];
}
long oct_to_dec( int oct )
{
	int dec = 0, temp = 0;

	while ( oct != 0 )
	{
		dec = dec + ( oct % 10 ) * pow( 8, temp );
		temp++;
		oct = oct / 10;
	}

	return dec;
}


char *strchr( const char *s, int c )
{
	while ( *s )
	{
		if ( *s == c ) return s;
		s++;
	}
	return NULL;
}

char *strtok_r( char *str, const char *delim, char **saveptr )
{
	char *begin;
	if ( str )
	{
		begin = str;
	}
	else if ( *saveptr )
	{
		begin = *saveptr;
	}
	else
	{
		return NULL;
	}

	while ( strchr( delim, begin[ 0 ] ) )
	{
		begin++;
	}


	char *next = NULL;
	for ( int i = 0; i < strlen( delim ); i++ )
	{
		char *temp = strchr( begin, delim[ i ] );
		if ( temp < next || next == NULL )
		{
			next = temp;
		}
	}
	if ( !next )
	{
		*saveptr = NULL;
		return begin;
	}
	*next = 0;
	*saveptr = next + 1;
	return begin;
}


char *strdup( const char *s )
{
	char *news = malloc( strlen( s ) + 1 );
	char *temp = news;
	while ( *temp++ = *s++ );
	return news;
}
