#include <ntddk.h>
#include <initguid.h>
#include <stdlib.h>

typedef struct _DEVICE_EXTENSI0N
{
	PDEVICE_OBJECT fdo;
	PDEVICE_OBJECT LowerDevice;
	UNICODE_STRING SymbolicLinkName;
}DEVICE_EXTENSI0N, *PDEVICE_EXTENSI0N;
NTSTATUS ForwardAndWait(IN PDEVICE_EXTENSI0N pdx, IN PIRP Irp);
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

NTSTATUS PnpQueryCapbilityHandler(IN PDEVICE_EXTENSI0N pdx, IN PIRP Irp)
{
	NTSTATUS status;
	PIO_STACK_LOCATION stack;
	PDEVICE_CAPABILITIES pdc;
	ULONG i;
	status = ForwardAndWait(pdx, Irp);
	if (NT_SUCCESS(status))
	{
		stack = IoGetCurrentIrpStackLocation(Irp);
		pdc = stack->Parameters.DeviceCapabilities.Capabilities;
		for (i = PowerSystemWorking; i < PowerSystemMaximum;i++)
		{
			KdPrint(("未改之前电源状态：%d,%d\n", i, pdc->DeviceState[i]));
		}

		pdc->DeviceState[PowerSystemWorking] = PowerDeviceD0;
		pdc->DeviceState[PowerSystemSleeping1] = PowerDeviceD3;
		pdc->DeviceState[PowerSystemSleeping1] = PowerDeviceD3;
		pdc->DeviceState[PowerSystemSleeping1] = PowerDeviceD3;
		pdc->DeviceState[PowerSystemHibernate] = PowerDeviceD3;
		pdc->DeviceState[PowerSystemShutdown] = PowerDeviceD3;


		pdc->Removable = TRUE;

		for (i = PowerSystemWorking; i < PowerSystemMaximum; i++)
		{
			KdPrint(("改之后电源状态：%d,%d\n", i, pdc->DeviceState[i]));
		}

	}

	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;

}

NTSTATUS IoCompletion(
	_In_     PDEVICE_OBJECT DeviceObject,
	_In_     PIRP           Irp,
	_In_opt_ PVOID          Context
	)
{
	
	PKEVENT pEvent = (PKEVENT)Context;
	KeSetEvent(pEvent, IO_NO_INCREMENT, FALSE);

	return STATUS_MORE_PROCESSING_REQUIRED;
	

}


NTSTATUS ForwardAndWait(IN PDEVICE_EXTENSI0N pdx, IN PIRP Irp)
{
	NTSTATUS status;
	KEVENT Event;
	KeInitializeEvent(&Event, NotificationEvent, FALSE);
	IoCopyCurrentIrpStackLocationToNext(Irp);
	IoSetCompletionRoutine(Irp, IoCompletion, &Event, TRUE, TRUE, TRUE);

	status = IoCallDriver(pdx->LowerDevice, Irp);

	KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
	return status;
}

VOID ShowResouce(PCM_PARTIAL_RESOURCE_LIST resouce)
{
	KdPrint(("******************************************\n"));
	KdPrint(("资源个数%d\n", resouce->Count));
	for (ULONG i = 0; i < resouce->Count; i++)
	{
		switch (resouce->PartialDescriptors[i].Type)
		{
		case CmResourceTypeMemory:
		{
			KdPrint(("内存%llx:%x\n", resouce->PartialDescriptors[i].u.Memory.Start.QuadPart,
				resouce->PartialDescriptors[i].u.Memory.Length));
		}
			break;
		case CmResourceTypePort:
		{
			KdPrint(("端口%llx:%x\n", resouce->PartialDescriptors[i].u.Port.Start.QuadPart,
				resouce->PartialDescriptors[i].u.Port.Length));

		}
			break;
		case CmResourceTypeInterrupt:
		{
			KdPrint(("中断leavel%x vector:%x %x\n", resouce->PartialDescriptors[i].u.Interrupt.Level,
				resouce->PartialDescriptors[i].u.Interrupt.Vector,
				resouce->PartialDescriptors[i].u.Interrupt.Affinity));

		}
		break;
		case CmResourceTypeDma:
		{
			KdPrint(("dma:channel=%d,port=%x\n", resouce->PartialDescriptors[i].u.Dma.Channel, resouce->PartialDescriptors[i].u.Dma.Port));
		}
			break;
		default:
			KdPrint(("无效%d\n", resouce->PartialDescriptors[i].Type));
			break;
		}
	}



}

NTSTATUS StartDeviceHandler(IN PDEVICE_EXTENSI0N pdx, IN PIRP Irp)
{
	NTSTATUS status;
	PIO_STACK_LOCATION stack;
	CM_PARTIAL_RESOURCE_LIST  raw;
	CM_PARTIAL_RESOURCE_LIST tranlated;
	status = ForwardAndWait(pdx, Irp);
	if (!NT_SUCCESS(status))
	{
		Irp->IoStatus.Status = status;
		Irp->IoStatus.Information = 0;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
		return status;
	}

	stack = IoGetCurrentIrpStackLocation(Irp);
	raw = stack->Parameters.StartDevice.AllocatedResources->List[0].PartialResourceList;
	tranlated = stack->Parameters.StartDevice.AllocatedResourcesTranslated->List[0].PartialResourceList;
	KdPrint(("原始资源\n"));
	ShowResouce(&raw);
	KdPrint(("翻译的资源\n"));
	ShowResouce(&tranlated);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
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
		PnpQueryCapbilityHandler,//IRP_MN_QUERY_CAPABILITIES			
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