#include <ntddk.h>

// UCHAR In_8(PUCHAR Port)
// {
// 	UCHAR Value;
// 	__asm
// 	{
// 		mov edx, Port
// 		mov eax, 0
// 		in al, dx
// 		mov Value, al
// 		nop
// 		nop
// 	}
// 
// 	return Value;
// 
// }
// 
// USHORT In_16(PUSHORT Port)
// {
// 	USHORT Value;
// 	__asm
// 	{
// 		mov edx, Port
// 			mov eax, 0
// 			in ax, dx
// 			mov Value, ax
// 			nop
// 			nop
// 	}
// 
// 	return Value;
// 
// }
// 
// ULONG In_32(PULONG Port)
// {
// 	ULONG Value;
// 	__asm
// 	{
// 		mov edx, Port
// 			mov eax, 0
// 			in eax, dx
// 			mov Value, eax
// 			nop
// 			nop
// 	}
// 
// 	return Value;
// }
// 
// void Out_8(PUCHAR Port, UCHAR Value)
// {
// 	__asm
// 	{
// 		mov edx, Port
// 			mov al, Value
// 			out dx, al
// 			nop
// 			nop
// 
// 	}
// 
// 
// }
// 
// void Out_16(PUSHORT Port, USHORT Value)
// {
// 	__asm
// 	{
// 		mov edx, Port
// 			mov ax, Value
// 			out dx, ax
// 			nop
// 			nop
// 
// 	}
// 
// }
// 
// void Out_32(PULONG Port, ULONG Value)
// {
// 	__asm
// 	{
// 		mov edx, Port
// 			mov eax, Value
// 			out dx, eax
// 			nop
// 			nop
// 
// 	}
// }
VOID SoundOff();

VOID Unload(IN PDRIVER_OBJECT DriverObject)
{
	SoundOff();

	KdPrint(("驱动卸载\n"));
}

VOID Sound(int f)
{
	USHORT B = 1193180 / f;

	UCHAR temp = READ_PORT_UCHAR((PUCHAR)0x61);
	temp |= 3;
	WRITE_PORT_UCHAR((PUCHAR)0x61, temp);
	WRITE_PORT_UCHAR((PUCHAR)0x43, 0xb6);

	WRITE_PORT_USHORT((PUSHORT)0x42, B & 0xff);
	WRITE_PORT_USHORT((PUSHORT)0x42, (B >> 8) & 0xff);

}

VOID SoundOff()
{
	UCHAR temp = READ_PORT_UCHAR((PUCHAR)0x61);
	temp &= 0xfc;
	WRITE_PORT_UCHAR((PUCHAR)0x61, temp);
}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
	KdPrint(("驱动加载\n"));

	//UCHAR Value = In_8((PUCHAR)0x43);
	UCHAR Value=READ_PORT_UCHAR((PUCHAR)0x43);
	

	Sound(2000);
	KdPrint(("端口值%x\n", Value));
	DriverObject->DriverUnload = Unload;

	return STATUS_SUCCESS;
}