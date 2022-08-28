#include <ntifs.h>
#include <wdm.h>

// always include after
#include "common.h"
#include "memory.h"

UNICODE_STRING DEVICE_NAME = RTL_CONSTANT_STRING(L"\\Device\\PMD_DEVICE");
UNICODE_STRING DEVICE_SYMBOLIC_NAME = RTL_CONSTANT_STRING(L"\\??\\PMD_DEVICE");

void DriverUnload(PDRIVER_OBJECT dob);
NTSTATUS MajorFunctions(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS HandleCustomIOCTL(PDEVICE_OBJECT DeviceObject, PIRP Irp);

extern "C" NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);

	NTSTATUS status = 0;

	DriverObject->DriverUnload = DriverUnload;

	DriverObject->MajorFunction[IRP_MJ_CREATE] = MajorFunctions;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = MajorFunctions;

	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HandleCustomIOCTL;

	status = IoCreateDevice(DriverObject, 0, &DEVICE_NAME, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DriverObject->DeviceObject);
	if (!NT_SUCCESS(status)) {
		KdPrint(("Could Not Create Device"));
		return status;
	}

	status = IoCreateSymbolicLink(&DEVICE_SYMBOLIC_NAME, &DEVICE_NAME);
	if (!NT_SUCCESS(status)) {
		IoDeleteDevice(DriverObject->DeviceObject);
		KdPrint(("Could Not Create Symlink"));
		return status;
	}

	KdPrint(("Driver loaded"));

	return STATUS_SUCCESS;
}

void DriverUnload(PDRIVER_OBJECT dob) {
	DbgPrint("Driver unloaded");

	IoDeleteSymbolicLink(&DEVICE_SYMBOLIC_NAME);
	IoDeleteDevice(dob->DeviceObject);
}

NTSTATUS MajorFunctions(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);

	PIO_STACK_LOCATION stackLocation = IoGetCurrentIrpStackLocation(Irp);

	switch (stackLocation->MajorFunction) {
	case IRP_MJ_CREATE:
		KdPrint(("Handle to Symlink created"));
		break;
	case IRP_MJ_CLOSE:
		KdPrint(("Handle to Symlink closed"));
		break;
	}

	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS HandleCustomIOCTL(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);

	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);

	switch (stack->Parameters.DeviceIoControl.IoControlCode) {
	case IOCTL_PMD:
		auto inputLen = stack->Parameters.DeviceIoControl.InputBufferLength;
		if (inputLen < sizeof(PMD_DATA)) {
			status = STATUS_BUFFER_TOO_SMALL;
			break;
		}

		auto inputBuffer = (PMD_DATA*) stack->Parameters.DeviceIoControl.Type3InputBuffer;
		KdPrint(("PID: %d", inputBuffer->PID));

		PEPROCESS proc;
		status = PsLookupProcessByProcessId(ULongToHandle(inputBuffer->PID), &proc);
		if (!NT_SUCCESS(status))
			break;

		// Do something with it
		KernelWriteVirtualMemory(proc, inputBuffer->Buffer, (PVOID) inputBuffer->Address, inputBuffer->Size);

		status = STATUS_SUCCESS;
		break;
	}

	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = status;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}
