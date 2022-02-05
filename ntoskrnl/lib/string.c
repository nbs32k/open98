#include "utill.h"


ULONG64
strlen(
	PCSTR String
)
{
	ULONG64 Len = 0;
	while ( String[ Len ] != '\0' )
		++Len;
	return Len;
}