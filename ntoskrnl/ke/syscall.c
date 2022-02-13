#include "ki.h"
#include "../hal/idt/idt.h"
#include "../fsrtl/fsrtl.h"

#define SYSCALL_ISR 0x80

typedef enum
{
	_DbgPrintFmt,
	_NtCreateFile,
	_NtReadFile,
	_NtClose,
};

ULONG64
KiSyscallHandler(
	struct InterruptRegisters* Registers
);


VOID
KiInstallSyscalls( 

)
{
	HalRegisterInterrupt( SYSCALL_ISR, KiSyscallHandler );
}

ULONG64
KiSyscallHandler( 
	struct InterruptRegisters* Registers
)
{
	//arg1: rsi, arg2: rdx, arg3: rcx, arg4: r8, arg5: r9
	INT iReturn = 0;

	switch ( Registers->rdi )
	{
		case _DbgPrintFmt:
			DbgPrintFmt( ( UCHAR* )Registers->rsi, ( UCHAR* )Registers->rdx );

			break;
		case _NtCreateFile:

			break;
		case _NtReadFile:
			iReturn =
				NtReadFile( ( PHANDLE )Registers->rsi, ( UCHAR* )Registers->rdx,
				( ULONG )Registers->rcx, ( ULONG )&Registers->r8, ( ULONG )Registers->r9 /*or NULL*/ );

			break;
		case _NtClose:

			break;
	}

	return iReturn;
}

//global intsyscall
//
//intsyscall :
//push rax
//push rbx
//push rcx
//push rdx
//
//push rcx
//mov rax, rdi
//mov rbx, rsi
//mov rcx, rdx
//pop rdx
//
//int 0x45
//
//pop rdx
//pop rcx
//pop rbx
//pop rax
//
//ret
//
//global _start
//
//extern main
//
//_start :
//call main
//jmp $