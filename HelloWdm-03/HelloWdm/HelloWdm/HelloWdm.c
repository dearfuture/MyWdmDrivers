#include <ntddk.h>
#include <initguid.h>
#include <stdlib.h>

typedef struct _DEVICE_EXTENSI0N
{
	PDEVICE_OBJECT fdo;
	PDEVICE_OBJECT LowerDevice;
	UNICODE_STRING SymbolicLinkName;
}DEVICE_EXTENSI0N, *PDEVICE_EXTENSI0N;

// {82549ffd-fe24-4704-8d40-6a12a0c23677} 
DEFINE_GUID(INTERFACEGUID, 
0x82549ffd, 0xfe24, 0x4704, 0x8d, 0x40, 0x6a, 0x12, 0xa0, 0xc2, 0x36, 0x77);

VOID Unload(IN PDRIVER_OBJECT DriverObject)
{
	
	KdPrint(("驱动卸载\n"));
}

NTSTATUS MyDispatchRoutine(IN PDEVICE_OBJECT fdo, IN PIRP Irp)
{
	KdPrint(("进入MyDispatchRoutine\n"));
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS DefaultHandler(IN PDEVICE_EXTENSI0N pdx, IN PIRP Irp)
{
	NTSTATUS status;
	IoSkipCurrentIrpStackLocation(Irp);
	status = IoCallDriver(pdx->LowerDevice, Irp);
	return status;

}


NTSTATUS RemoveHandler(IN PDEVICE_EXTENSI0N pdx, IN PIRP Irp)
{
	NTSTATUS status;
	IoSkipCurrentIrpStackLocation(Irp);
	status = IoCallDriver(pdx->LowerDevice, Irp);

	IoDetachDevice(pdx->LowerDevice);//关键一部分
	IoDeleteDevice(pdx->fdo);
	IoSetDeviceInterfaceState(&pdx->SymbolicLinkName, FALSE);
	return status;
}


NTSTATUS MyPnpRoutine(IN PDEVICE_OBJECT fdo, IN PIRP Irp)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_EXTENSI0N pdx = NULL;
	PIO_STACK_LOCATION stack = NULL;
	static char *funname[] =
	{ 
	"IRP_MN_START_DEVICE				",
	"IRP_MN_QUERY_REMOVE_DEVICE			",
	"IRP_MN_REMOVE_DEVICE				",
	"IRP_MN_CANCEL_REMOVE_DEVICE		",
	"IRP_MN_STOP_DEVICE					",
	"IRP_MN_QUERY_STOP_DEVICE			",
	"IRP_MN_CANCEL_STOP_DEVICE			",
	"IRP_MN_QUERY_DEVICE_RELATIONS		",
	"IRP_MN_QUERY_INTERFACE				",
	"IRP_MN_QUERY_CAPABILITIES			",
	"IRP_MN_QUERY_RESOURCES				",
	"IRP_MN_QUERY_RESOURCE_REQUIREMENTS	",
	"IRP_MN_QUERY_DEVICE_TEXT			",
	"IRP_MN_FILTER_RESOURCE_REQUIREMENTS",
	"",
	"IRP_MN_READ_CONFIG					",
	"IRP_MN_WRITE_CONFIG				",
	"IRP_MN_EJECT						",
	"IRP_MN_SET_LOCK					",
	"IRP_MN_QUERY_ID					",
	"IRP_MN_QUERY_PNP_DEVICE_STATE		",
	"IRP_MN_QUERY_BUS_INFORMATION		",
	"IRP_MN_DEVICE_USAGE_NOTIFICATION	",
	"IRP_MN_SURPRISE_REMOVAL			"
	};


	NTSTATUS(*funtable[])(IN PDEVICE_EXTENSI0N pdx, IN PIRP Irp)=
	{
		DefaultHandler,//IRP_MN_START_DEVICE				
		DefaultHandler,//IRP_MN_QUERY_REMOVE_DEVICE			
		RemoveHandler,//IRP_MN_REMOVE_DEVICE				
		DefaultHandler,//IRP_MN_CANCEL_REMOVE_DEVICE		
		DefaultHandler,//IRP_MN_STOP_DEVICE					
		DefaultHandler,//IRP_MN_QUERY_STOP_DEVICE			
		DefaultHandler,//IRP_MN_CANCEL_STOP_DEVICE			
		DefaultHandler,//IRP_MN_QUERY_DEVICE_RELATIONS		
		DefaultHandler,//IRP_MN_QUERY_INTERFACE				
		DefaultHandler,//IRP_MN_QUERY_CAPABILITIES			
		DefaultHandler,//IRP_MN_QUERY_RESOURCES				
		DefaultHandler,//IRP_MN_QUERY_RESOURCE_REQUIREMENTS	
		DefaultHandler,//IRP_MN_QUERY_DEVICE_TEXT			
		DefaultHandler,//IRP_MN_FILTER_RESOURCE_REQUIREMENTS
		DefaultHandler,//",
		DefaultHandler,//IRP_MN_READ_CONFIG					
		DefaultHandler,//IRP_MN_WRITE_CONFIG				
		DefaultHandler,//IRP_MN_EJECT						
		DefaultHandler,//IRP_MN_SET_LOCK					
		DefaultHandler,//IRP_MN_QUERY_ID					
		DefaultHandler,//IRP_MN_QUERY_PNP_DEVICE_STATE		
		DefaultHandler,//IRP_MN_QUERY_BUS_INFORMATION		
		DefaultHandler,//IRP_MN_DEVICE_USAGE_NOTIFICATION	
		DefaultHandler,//IRP_MN_SURPRISE_REMOVAL			

	};

	pdx = fdo->DeviceExtension;
	stack = IoGetCurrentIrpStackLocation(Irp);
	if (stack->MinorFunction >= _countof(funtable))
	{
		status = DefaultHandler(pdx, Irp);
	}
	else
	{
		KdPrint(("%s\n", funname[stack->MinorFunction]));
		status = funtable[stack->MinorFunction](pdx, Irp);
	}

	return status;
}

NTSTATUS MyAddDevice(PDRIVER_OBJECT DriverObject,PDEVICE_OBJECT pdo)
{
	NTSTATUS status =  STATUS_SUCCESS;
	PDEVICE_OBJECT fdo;
	PDEVICE_EXTENSI0N pdx;
	//UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\HelloWdm");
	UNICODE_STRING SymbolicLinkName = RTL_CONSTANT_STRING(L"\\??\\HelloWdm");
	status = IoCreateDevice(DriverObject, sizeof(DEVICE_EXTENSI0N), NULL, FILE_DEVICE_UNKNOWN, 0, FALSE, &fdo);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("创建设备对象失败%x\n", status));
		return status;
	}

	pdx = (PDEVICE_EXTENSI0N)fdo->DeviceExtension;

	pdx->fdo = fdo;
	pdx->LowerDevice = IoAttachDeviceToDeviceStack(fdo, pdo);
	if (pdx->LowerDevice == NULL)
	{
		IoDeleteDevice(fdo);
	
		return STATUS_UNSUCCESSFUL;
	}

	IoRegisterDeviceInterface(pdo, &INTERFACEGUID, NULL, &pdx->SymbolicLinkName);
	KdPrint(("%wZ\n", &pdx->SymbolicLinkName));
	IoSetDeviceInterfaceState(&pdx->SymbolicLinkName, TRUE);

	fdo->Flags |= DO_BUFFERED_IO|DO_POWER_PAGABLE;
	fdo->Flags&= ~DO_DEVICE_INITIALIZING;

	return status;
}


NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
	NTSTATUS status = STATUS_SUCCESS;
	ULONG i;
	DriverObject->DriverUnload = Unload;
	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION+1;i++)
	{
		DriverObject->MajorFunction[i] = MyDispatchRoutine;
	}
	DriverObject->MajorFunction[IRP_MJ_PNP] = MyPnpRoutine;


	DriverObject->DriverExtension->AddDevice = MyAddDevice;


	return status;
}