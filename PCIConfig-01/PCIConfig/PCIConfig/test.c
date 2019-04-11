#include <ntddk.h>

#ifdef _AMD64_
ULONG
HalGetBusData(
_In_ BUS_DATA_TYPE BusDataType,
_In_ ULONG BusNumber,
_In_ ULONG SlotNumber,
_Out_writes_bytes_(Length) PVOID Buffer,
_In_ ULONG Length
);
#endif
VOID Unload(IN PDRIVER_OBJECT DriverObject)
{

	KdPrint(("Çý¶¯Ð¶ÔØ\n"));

}


VOID DisplayConfig(int bus,int dev,int func)
{
	PCI_COMMON_CONFIG Config;
	PPCI_COMMON_HEADER pConfigHeader;
	PCI_SLOT_NUMBER SlotNumber;
	ULONG Addr = 0;
	ULONG Data;
	ULONG i;
	SlotNumber.u.AsULONG = 0;
	SlotNumber.u.bits.DeviceNumber = dev;
	SlotNumber.u.bits.FunctionNumber = func;

	Addr = 0x80000000 | (bus << 16) | (SlotNumber.u.AsULONG << 8);
	for (i = 0; i < 0x100; i += 4)
	{
		Addr |= i;
		WRITE_PORT_ULONG((PULONG)0xcf8, Addr|i);
		Data = READ_PORT_ULONG((PULONG)0xcfc);
		((PULONG)&Config)[i>>2] = Data;
		
	}

	pConfigHeader = (PPCI_COMMON_HEADER)&Config;
	KdPrint(("VendorID:%x\n", pConfigHeader->VendorID));
	KdPrint(("VendorID:%x\n", pConfigHeader->DeviceID));
	KdPrint(("VendorID:%x\n", pConfigHeader->Command));
	KdPrint(("VendorID:%x\n", pConfigHeader->Status));
	KdPrint(("VendorID:%x\n", pConfigHeader->RevisionID));
	KdPrint(("VendorID:%x\n", pConfigHeader->ProgIf));
	KdPrint(("VendorID:%x\n", pConfigHeader->SubClass));
	KdPrint(("VendorID:%x\n", pConfigHeader->BaseClass));
	KdPrint(("VendorID:%x\n", pConfigHeader->CacheLineSize));
	KdPrint(("VendorID:%x\n", pConfigHeader->LatencyTimer));
	KdPrint(("VendorID:%x\n", pConfigHeader->HeaderType));
	KdPrint(("VendorID:%x\n", pConfigHeader->BIST));

	for (i = 0; i < 6;i++)
	{
		KdPrint(("Bar:%x\n", pConfigHeader->u.type0.BaseAddresses[i]));
	}
	KdPrint(("VendorID:%x\n", pConfigHeader->u.type0.InterruptLine));
	KdPrint(("VendorID:%x\n", pConfigHeader->u.type0.InterruptPin));
}


VOID DisplayConfig1(int bus, int dev, int func)
{

	PCI_SLOT_NUMBER SlotNumber;
	PCI_COMMON_CONFIG Config;
	PPCI_COMMON_HEADER pConfigHeader;
	ULONG Size;
	ULONG i;
	SlotNumber.u.AsULONG = 0;
	
	SlotNumber.u.bits.DeviceNumber = dev;
	SlotNumber.u.bits.FunctionNumber = func;


	Size = HalGetBusData(PCIConfiguration, bus, SlotNumber.u.AsULONG, &Config, PCI_COMMON_HDR_LENGTH);
	if (Size == PCI_COMMON_HDR_LENGTH)
	{
		pConfigHeader = (PPCI_COMMON_HEADER)&Config;
		KdPrint(("VendorID:%x\n", pConfigHeader->VendorID));
		KdPrint(("VendorID:%x\n", pConfigHeader->DeviceID));
		KdPrint(("VendorID:%x\n", pConfigHeader->Command));
		KdPrint(("VendorID:%x\n", pConfigHeader->Status));
		KdPrint(("VendorID:%x\n", pConfigHeader->RevisionID));
		KdPrint(("VendorID:%x\n", pConfigHeader->ProgIf));
		KdPrint(("VendorID:%x\n", pConfigHeader->SubClass));
		KdPrint(("VendorID:%x\n", pConfigHeader->BaseClass));
		KdPrint(("VendorID:%x\n", pConfigHeader->CacheLineSize));
		KdPrint(("VendorID:%x\n", pConfigHeader->LatencyTimer));
		KdPrint(("VendorID:%x\n", pConfigHeader->HeaderType));
		KdPrint(("VendorID:%x\n", pConfigHeader->BIST));

		for (i = 0; i < 6; i++)
		{
			KdPrint(("Bar:%x\n", pConfigHeader->u.type0.BaseAddresses[i]));
		}
		KdPrint(("VendorID:%x\n", pConfigHeader->u.type0.InterruptLine));
		KdPrint(("VendorID:%x\n", pConfigHeader->u.type0.InterruptPin));
	}
	else
	{

	}

}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
	int bus, dev, func;
	KdPrint(("Çý¶¯¼ÓÔØ\n"));
	DriverObject->DriverUnload = Unload;




	for (bus = 0; bus < PCI_MAX_BRIDGE_NUMBER;bus++)
	{
		for (dev = 0; dev < PCI_MAX_DEVICES;dev++)
		{
			for (func = 0; func < PCI_MAX_FUNCTION;func++)
			{
				DisplayConfig(bus, dev, func);
			}
		}
	}


	return STATUS_SUCCESS;
}