#include <windows.h>
#include <bluetoothapis.h>
#include <stdio.h>
#include <stdlib.h>

#define DEVICENAME "Galaxy Buds+ (5187)"
#define PRINT_GUID(x) printf("{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}\n", \
  x.Data1, x.Data2, x.Data3, \
  x.Data4[0], x.Data4[1], x.Data4[2], x.Data4[3], \
  x.Data4[4], x.Data4[5], x.Data4[6], x.Data4[7])

void ErrorExit(DWORD err);

int main(void)
{
    int i;
    char name[256], last_seen[20];
    BOOL devicefound = FALSE;
    UCHAR address[256];
    SYSTEMTIME LocalTime;

    DWORD num_services = 0;
    GUID *service_list;

    HBLUETOOTH_DEVICE_FIND handle;

    BLUETOOTH_DEVICE_INFO btdevice = {sizeof(BLUETOOTH_DEVICE_INFO), 0};
    BLUETOOTH_DEVICE_SEARCH_PARAMS search = {
        .dwSize = sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS),
        .fReturnAuthenticated = TRUE,
        .fReturnRemembered = TRUE,
        .fReturnUnknown = FALSE,
        .fReturnConnected = TRUE,
        .fIssueInquiry = TRUE,
        .cTimeoutMultiplier = 5,
        .hRadio = NULL
    };

    handle = BluetoothFindFirstDevice(&search, &btdevice);   
    if(handle == NULL) 
        ErrorExit(GetLastError());
    do {
        wcstombs(name, btdevice.szName, wcslen(btdevice.szName));
        sprintf(address, 
                "%02X:%02X:%02X:%02X:%02X:%02X", 
                btdevice.Address.rgBytes[5],
                btdevice.Address.rgBytes[4],
                btdevice.Address.rgBytes[3],
                btdevice.Address.rgBytes[2],
                btdevice.Address.rgBytes[1],
                btdevice.Address.rgBytes[0]);
        SystemTimeToTzSpecificLocalTime(NULL, &btdevice.stLastSeen, &LocalTime);
        sprintf(last_seen, "%d-%02d-%02d %02d:%02d:%02d",
                LocalTime.wYear,
                LocalTime.wMonth,
                LocalTime.wDay,
                LocalTime.wHour,
                LocalTime.wMinute,
                LocalTime.wSecond);
        printf("Name: %s Address: %s\nLast Seen: %s\n", name, address, last_seen);
        if(strcmp(DEVICENAME, name) == 0 ) {
            devicefound = TRUE;
            printf("this is the device we want.\n");
            break;
        }

    } while(BluetoothFindNextDevice(handle, &btdevice));
    BluetoothFindDeviceClose(handle);
    if(!devicefound) {
        printf("couldn't find target device!\n");
        return 1;
    }
    /* count number of installed services first */
    BluetoothEnumerateInstalledServices(NULL, &btdevice, &num_services, NULL);
    if(num_services == 0) {
        printf("no services found!\n");
        return 1;
    }

    /* now retreive the list of service GUIDs */
    service_list = calloc(num_services, sizeof(GUID));
    BluetoothEnumerateInstalledServices(NULL, &btdevice, &num_services, service_list);
    for(i=0;i<num_services;i++) {
        printf("disabling service: ");
        PRINT_GUID(service_list[i]);
        BluetoothSetServiceState(NULL, &btdevice, &service_list[i], 0);
    }
    printf("re-enabling services...\n");
    for(i=0;i<num_services;i++)
        BluetoothSetServiceState(NULL, &btdevice, &service_list[i], 1);

    free(service_list);
    return 0;
}

void ErrorExit(DWORD err)
{
    char msgbuf[512];

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)msgbuf, sizeof(msgbuf), 0);
    printf("error: %s\n", msgbuf);
    exit(err);
}
