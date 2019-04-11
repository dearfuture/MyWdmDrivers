#include <windows.h>
#include <stdio.h>

UCHAR In_8(PUCHAR Port)
{
	UCHAR Value;
	__asm
	{
		mov edx,Port
		mov eax,0
		in al,dx
		mov Value, al
		nop
		nop
	}

	return Value;

}

USHORT In_16(PUSHORT Port)
{
	USHORT Value;
	__asm
	{
		mov edx, Port
		mov eax, 0
		in ax, dx
		mov Value, ax
		nop
		nop
	}

	return Value;

}

ULONG In_32(PULONG Port)
{
	ULONG Value;
	__asm
	{
		mov edx, Port
		mov eax, 0
		in eax, dx
		mov Value, eax
		nop
		nop
	}

	return Value;
}

void Out_8(PUCHAR Port, UCHAR Value)
{
	__asm
	{
		mov edx,Port
		mov al,Value
		out dx,al
		nop
		nop

	}


}

void Out_16(PUSHORT Port, USHORT Value)
{
	__asm
	{
		mov edx, Port
		mov ax, Value
		out dx, ax
		nop
		nop

	}

}

void Out_32(PULONG Port, ULONG Value)
{
	__asm
	{
		mov edx, Port
		mov eax, Value
		out dx, eax
		nop
		nop

	}
}


int main()
{


	UCHAR Value = In_8((PUCHAR)0x43);

	return 0;
}