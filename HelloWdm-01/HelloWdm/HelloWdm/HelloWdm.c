#include <ntddk.h>

typedef struct _DEVICE_EXTENSI0N
{
	PDEVICE_OBJECT fdo;
	PDEVICE_OBJECT LowerDevice;

}DEVICE_EXTENSI0N, *PDEVICE_EXTENSI0N;

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

NTSTATUS MyPnpRoutine(IN PDEVICE_OBJECT fdo, IN PIRP Irp)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_EXTENSI0N pdx = fdo->DeviceExtension;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	KdPrint(("进入pnp处理函\n"));
	if (stack->MinorFunction == IRP_MN_REMOVE_DEVICE)
	{
		IoSkipCurrentIrpStackLocation(Irp);
		status = IoCallDriver(pdx->LowerDevice, Irp);

		UNICODE_STRING SymbolicLinkName = RTL_CONSTANT_STRING(L"\\??\\HelloWdm");
		IoDetachDevice(pdx->LowerDevice);//关键一部分
		IoDeleteDevice(pdx->fdo);
		IoDeleteSymbolicLink(&SymbolicLinkName);
		return status;
	}
	else
	{
		IoSkipCurrentIrpStackLocation(Irp);
		status = IoCallDriver(pdx->LowerDevice, Irp);
	}


	return status;
}

NTSTATUS MyAddDevice(PDRIVER_OBJECT DriverObject,PDEVICE_OBJECT pdo)
{
	NTSTATUS status =  STATUS_SUCCESS;
	PDEVICE_OBJECT fdo;
	PDEVICE_EXTENSI0N pdx;
	UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\HelloWdm");
	UNICODE_STRING SymbolicLinkName = RTL_CONSTANT_STRING(L"\\??\\HelloWdm");
	status = IoCreateDevice(DriverObject, sizeof(DEVICE_EXTENSI0N), &DeviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &fdo);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("创建设备对象失败%x\n", status));
		return status;
	}

	status = IoCreateSymbolicLink(&SymbolicLinkName, &DeviceName);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(fdo);
		return status;
	}
	pdx = (PDEVICE_EXTENSI0N)fdo->DeviceExtension;

	pdx->fdo = fdo;
	pdx->LowerDevice = IoAttachDeviceToDeviceStack(fdo, pdo);
	if (pdx->LowerDevice == NULL)
	{
		IoDeleteDevice(fdo);
		IoDeleteSymbolicLink(&SymbolicLinkName);
		return STATUS_UNSUCCESSFUL;
	}

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