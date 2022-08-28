#include <iostream>
#include <Windows.h>

// always include after
#include "..\Process Memory Driver\common.h"
#define DEVICE_NAME L"\\\\.\\PMD_DEVICE"

using namespace std;

int main() {
    HANDLE device = NULL;
    DWORD bytesReturned = 0;

    DWORD PID = 0;
    ULONG64 addr = 0;
    cout << "Process ID: ";
    cin >> dec >> PID;

    cout << "Target Address (hex): ";
    cin >> hex >> addr;

    while (cin.get() != '\n');

    device = CreateFile(DEVICE_NAME, GENERIC_ALL, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, 0);
    if (device == NULL) {
        cerr << "Couldnt open symlink: " << GetLastError() << endl;
        cin.get();
        return EXIT_FAILURE;
    }

    uint8_t buff[256]{ 'H', 0x00, 'A', 0x00, 'C', 0x00, 'K', 0x00 };
    PMD_DATA sending{ PID, addr, buff, 256 };

    BOOL sent = DeviceIoControl(device, IOCTL_PMD, &sending, sizeof(sending), nullptr, 0, &bytesReturned, NULL);
    if (!sent) {
        cerr << "Error sending IOCTL: " << GetLastError() << endl;
        cin.get();
        CloseHandle(device);
        return EXIT_FAILURE;
    }

    cout << "Read 256 Bytes at 0x" << hex << uppercase << addr << ":" << endl;
    for (int i(0); i < 256; ++i)
        cout << buff[i];
    cout << endl;

    cin.get();
    CloseHandle(device);
    return EXIT_SUCCESS;
}
