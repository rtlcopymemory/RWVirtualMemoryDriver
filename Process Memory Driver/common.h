#pragma once

#define IOCTL_PMD CTL_CODE(FILE_DEVICE_UNKNOWN, 0X2049, METHOD_NEITHER, FILE_ANY_ACCESS)

struct PMD_DATA {
	ULONG PID;
	ULONG64 Address;
	PVOID Buffer;
	ULONG Size;
};
