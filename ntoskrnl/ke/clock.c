#include "../hal/hal.h"

#define CMOS_IO		0x70
#define CMOS_DATA	0x71

#define RTC_A		0x0A
#define RTC_B		0x0B
#define RTC_C		0x0C
#define RTC_D		0x0D

#define RTC_SECONDS 0x00
#define RTC_MINUTES 0x02
#define RTC_HOURS	0x04
#define RTC_DAYS	0x07
#define RTC_MONTHS	0x08
#define RTC_YEARS	0x09

KSYSTEM_TIME KiGlobalTime = { 0 };


UCHAR
KiConvertBcd(
	UCHAR ucValue
)
{
	return ( ucValue >> 4 ) * 10 + ( ucValue & 15 );
}

UCHAR
KiSetRTCData(
	INT iValue,
	INT iValue2
)
{
	outb( CMOS_IO, iValue );
	outb( CMOS_DATA, iValue2 );
}


UCHAR
KiGetRTCData(
	INT iValue
)
{
	outb( CMOS_IO, iValue );
	return KiConvertBcd( inb( CMOS_DATA ) );
}


STATIC VOID
KiUpdateClock( 
	//struct InterruptRegisters *reg
)
{
	KiGlobalTime.Seconds = KiGetRTCData( RTC_SECONDS );
	KiGlobalTime.Minutes = KiGetRTCData( RTC_MINUTES );
	KiGlobalTime.Hours	 = KiGetRTCData( RTC_HOURS );
	KiGlobalTime.Days	 = KiGetRTCData( RTC_DAYS );
	KiGlobalTime.Months  = KiGetRTCData( RTC_MONTHS );
	KiGlobalTime.Years	 = KiGetRTCData( RTC_YEARS );
}

VOID
KeQuerySystemTime(
	PKSYSTEM_TIME ClockTime
)
{
	KiUpdateClock( );
	*ClockTime = KiGlobalTime;
}

VOID
KeInitializeKernelClock(

)
{
	KiPrintToScreen( "[ OK ] Initialized Kernel Clock" );
	KiUpdateClock( );
}