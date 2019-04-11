#include <windows.h>
#include <SetupAPI.h>
#include <stdio.h>
#include <initguid.h>
#pragma comment(lib,"SetupAPI.lib")
DEFINE_GUID(INTERFACEGUID,
	0x82549ffd, 0xfe24, 0x4704, 0x8d, 0x40, 0x6a, 0x12, 0xa0, 0xc2, 0x36, 0x77);
int main()
{
	HDEVINFO hDevInfo = SetupDiGetClassDevs(&INTERFACEGUID, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
	if (hDevInfo == INVALID_HANDLE_VALUE)
	{

		printf("SetupDiGetClassDevs%d\n", GetLastError());
		return -1;
	}
	SP_DEVICE_INTERFACE_DATA ifdata;
	PSP_DEVICE_INTERFACE_DETAIL_DATA ifdataDetails = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(1024 * 4);
	ifdata.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	for (int i = 0;;i++)
	{
		if (!SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &INTERFACEGUID, i, &ifdata))
		{
			break;
		}
		RtlZeroMemory(ifdataDetails, 4096);
		ifdataDetails->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		if (SetupDiGetDeviceInterfaceDetail(hDevInfo, &ifdata, ifdataDetails, 4096, NULL, NULL))
		{
			printf("Â·¾¶%ls\n", ifdataDetails->DevicePath);
		}

	}



	
	return 0;
}