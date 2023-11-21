/*++

Module Name:

    queue.c

Abstract:

    This file contains the queue entry points and callbacks.

Environment:

    Kernel-mode Driver Framework

--*/

#include "driver.h"
#include "queue.tmh"
#include <winerror.h>
#include "../Include/IS_KEYBOARD_RKT_OUTPUT_DATA.h"

EXTERN_C NTSTATUS FillOutputData(IS_KEYBOARD_RKT_OUTPUT_DATA* pOutputData);

#define RCBA_CHUNK_SIZE			0x2000
#define APIC_CHUNK_SIZE			0x1000
#define IRQ_APIC_KEYBOARD		0x1
#define IO_APIC_BASE_ADDRESS	0xFEC00000
#define L_APIC_BASE_ADDRESS		0xFEE00000

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, IsKeyboardRootkittedDriverQueueInitialize)
#pragma alloc_text (PAGE, KKSKmdIsKeyboardRktEvtIoRead)
#pragma alloc_text (PAGE, KKSKmdIsKeyboardRktEvtIoWrite)
#pragma alloc_text (PAGE, KKSKmdIsKeyboardRktEvtIoQueueContextDestroy)
#pragma alloc_text (PAGE, FillOutputData)
#endif

extern unsigned long GetRootComplexBaseAddress();
extern unsigned long GetApicBaseAddress();

NTSTATUS
IsKeyboardRootkittedDriverQueueInitialize(
    _In_ WDFDEVICE Device
    )
/*++

Routine Description:

     The I/O dispatch callbacks for the frameworks device object
     are configured in this function.

     A single default I/O Queue is configured for parallel request
     processing, and a driver context memory allocation is created
     to hold our structure QUEUE_CONTEXT.

Arguments:

    Device - Handle to a framework device object.

Return Value:

    VOID

--*/
{
    PAGED_CODE();

    //WriteDebugInfoLine("QueueInitialize - STARTED!");

    WDFQUEUE queue;
    NTSTATUS status;
    WDF_IO_QUEUE_CONFIG    queueConfig;
    PQUEUE_CONTEXT queueContext;
    WDF_OBJECT_ATTRIBUTES  queueAttributes;

    //
    // Configure a default queue so that requests that are not
    // configure-fowarded using WdfDeviceConfigureRequestDispatching to goto
    // other queues get dispatched here.
    //
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
        &queueConfig,
        WdfIoQueueDispatchSequential
    );

    //queueConfig.EvtIoDeviceControl = KKSKmdIsKeyboardRktEvtIoDeviceControl;
    //queueConfig.EvtIoStop = KKSKmdIsKeyboardRktEvtIoStop;
    queueConfig.EvtIoRead = KKSKmdIsKeyboardRktEvtIoRead;
    queueConfig.EvtIoWrite = KKSKmdIsKeyboardRktEvtIoWrite;

    //
    // Fill in a callback for destroy, and our QUEUE_CONTEXT size
    //
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&queueAttributes, QUEUE_CONTEXT);

    //
    // Set synchronization scope on queue and have the timer to use queue as
    // the parent object so that queue and timer callbacks are synchronized
    // with the same lock.
    //
    //queueAttributes.SynchronizationScope = WdfSynchronizationScopeQueue; // DANGER!!

    queueAttributes.EvtDestroyCallback = KKSKmdIsKeyboardRktEvtIoQueueContextDestroy;

    status = WdfIoQueueCreate(
        Device,
        &queueConfig,
        &queueAttributes,//WDF_NO_OBJECT_ATTRIBUTES,
        &queue
    );

    if (!NT_SUCCESS(status)) {
        //TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "WdfIoQueueCreate failed %!STATUS!", status);
        //WriteDebugInfoLine("QueueInitialize - ENDED! [!NT_SUCCESS(status)][status = WdfIoQueueCreate]");
        return status;
    }

    queueContext = QueueGetContext(queue);
    queueContext->WriteMemory = NULL;
    queueContext->CurrentRequest = NULL;
    queueContext->CurrentStatus = STATUS_INVALID_DEVICE_REQUEST;
    queueContext->CurrentDevice = Device;

    //WriteDebugInfoLine("QueueInitialize - ENDED!");
    return status;
}

VOID
KKSKmdIsKeyboardRktEvtIoRead(
	IN WDFQUEUE   Queue,
	IN WDFREQUEST Request,
	IN size_t      Length
)
/*++

Routine Description:

This event is called when the framework receives IRP_MJ_READ request.
It will copy the content from the queue-context buffer to the request buffer.
If the driver hasn't received any write request earlier, the read returns zero.

Arguments:

Queue -  Handle to the framework queue object that is associated with the
I/O request.

Request - Handle to a framework request object.

Length  - number of bytes to be read.
The default property of the queue is to not dispatch
zero lenght read & write requests to the driver and
complete is with status success. So we will never get
a zero length request.

Return Value:

VOID

--*/
{
	//WriteDebugInfoLine("EvtIoRead - STARTED!");

	/*if (g_hIoMemMap == NULL) {
		InitializeIoMemMap();
	}*/

	PAGED_CODE();

	NTSTATUS ntStatus;
	PQUEUE_CONTEXT queueContext = QueueGetContext(Queue);
	WDFMEMORY memory;

	if (Length != sizeof(IS_KEYBOARD_RKT_OUTPUT_DATA)) {
		WdfRequestCompleteWithInformation(Request, E_NOT_SUFFICIENT_BUFFER, 0L);
		//WriteDebugInfoLine("EvtIoRead - ENDED! [Length != sizeof(OutputData)]");
		return;
	}

	//
	// Get the request memory
	//
	ntStatus = WdfRequestRetrieveOutputMemory(Request, &memory);
	if (!NT_SUCCESS(ntStatus)) {
		//KdPrint(("EchoEvtIoRead Could not get request memory buffer 0x%x\n", Status));
		//WdfVerifierDbgBreakPoint();
		WdfRequestCompleteWithInformation(Request, ntStatus, 0L);
		//WriteDebugInfoLine("EvtIoRead - ENDED! [!NT_SUCCESS(Status)][Status = WdfRequestRetrieveOutputMemory]");
		return;
	}

	IS_KEYBOARD_RKT_OUTPUT_DATA outputData = { 0 };
	ntStatus = FillOutputData(&outputData);

	// Copy the memory out
	ntStatus = WdfMemoryCopyFromBuffer(memory, // destination
		0,      // offset into the destination memory
		&outputData,
		Length);

	if (!NT_SUCCESS(ntStatus)) {
		//KdPrint(("EchoEvtIoRead: WdfMemoryCopyFromBuffer failed 0x%x\n", Status));
		queueContext->CurrentRequest = Request;
		queueContext->CurrentStatus = ntStatus;
		WdfRequestComplete(Request, ntStatus);
		//WriteDebugInfoLine("EvtIoRead - ENDED! [!NT_SUCCESS(Status)][Status = WdfMemoryCopyFromBuffer]");
		return;
	}

	// Set transfer information
	WdfRequestSetInformation(Request, (ULONG_PTR)Length);

	// Mark the request is cancelable
	//WdfRequestMarkCancelable(Request, KKSKmdIsKeyboardRktEvtRequestCancel);

	// Defer the completion to another thread from the timer dpc
	queueContext->CurrentRequest = Request;
	queueContext->CurrentStatus = ntStatus;

	//WriteDebugInfoLine("EvtIoRead - ENDED!");

	WdfRequestComplete(Request, ntStatus);

	return;
}

VOID
KKSKmdIsKeyboardRktEvtIoQueueContextDestroy(
	WDFOBJECT Object
)
/*++

Routine Description:

This is called when the Queue that our driver context memory
is associated with is destroyed.

Arguments:

Context - Context that's being freed.

Return Value:

VOID

--*/
{
	//WriteDebugInfoLine("EvtIoQueueContextDestroy - STARTED!");
	PAGED_CODE();
	PQUEUE_CONTEXT queueContext = QueueGetContext(Object);

	//
	// Release any resources pointed to in the queue context.
	//
	// The body of the queue context will be released after
	// this callback handler returns
	//

	//
	// If Queue context has an I/O buffer, release it
	//
	if (queueContext->WriteMemory != NULL) {
		WdfObjectDelete(queueContext->WriteMemory);
		queueContext->WriteMemory = NULL;
	}

	//WriteDebugInfoLine("EvtIoQueueContextDestroy - ENDED");

	return;
}

VOID
KKSKmdIsKeyboardRktEvtIoWrite(
	IN WDFQUEUE   Queue,
	IN WDFREQUEST Request,
	IN size_t     Length
)
/*++

Routine Description:

This event is invoked when the framework receives IRP_MJ_WRITE request.
This routine allocates memory buffer, copies the data from the request to it,
and stores the buffer pointer in the queue-context with the length variable
representing the buffers length. The actual completion of the request
is defered to the periodic timer dpc.

Arguments:

Queue -  Handle to the framework queue object that is associated with the
I/O request.

Request - Handle to a framework request object.

Length  - number of bytes to be read.
The default property of the queue is to not dispatch
zero lenght read & write requests to the driver and
complete is with status success. So we will never get
a zero length request.

Return Value:

VOID

--*/
{
	//WriteDebugInfoLine("EvtIoWrite - STARTED!");
	PAGED_CODE();

	PQUEUE_CONTEXT queueContext = QueueGetContext(Queue);
	queueContext->CurrentStatus = STATUS_SUCCESS;

	WdfRequestSetInformation(Request, (ULONG_PTR)Length);

	//WriteDebugInfoLine("EvtIoWrite - ENDED!");

	return;
}

/// <summary>
/// Fills output structure with data.
/// </summary>
/// <param name="pOutputData"></param>
/// <returns></returns>
NTSTATUS FillOutputData(IS_KEYBOARD_RKT_OUTPUT_DATA* pOutputData)
{
	if (pOutputData == NULL)
	{
		return STATUS_INVALID_ADDRESS;
	}

	//PAGED_CODE();

	NTSTATUS ntStatus = STATUS_SUCCESS;
	PVOID hIoMemMap = NULL;

	// IOTRn
	pOutputData->cbSize = sizeof(IS_KEYBOARD_RKT_OUTPUT_DATA);
	pOutputData->dwRootComplexBaseAddress = GetRootComplexBaseAddress();
	if (pOutputData->dwRootComplexBaseAddress != 0 && pOutputData->dwRootComplexBaseAddress != 0xFFFFFFFF) {
		PHYSICAL_ADDRESS physAddress = { 0 };
		physAddress.LowPart = pOutputData->dwRootComplexBaseAddress & RCBA_MASK;
		hIoMemMap = MmMapIoSpace(physAddress, RCBA_CHUNK_SIZE, MmCached);//PAGE_READONLY);
		if (hIoMemMap == NULL) {
			ntStatus = STATUS_UNSUCCESSFUL;
			pOutputData->ntStatusCode = ntStatus;
			strcpy(pOutputData->szErrorMessage, "MmMapIoSpaceEx[physAddress, RCBA_CHUNK_SIZE, PAGE_READONLY] FAILED!");
			return ntStatus;
		}
	}
	else {
		ntStatus = STATUS_UNSUCCESSFUL;
		pOutputData->ntStatusCode = ntStatus;
		if (pOutputData->dwApicBaseAddress == 0) {
			strcpy(pOutputData->szErrorMessage, "Root Complex Base Address == 0x00000000");
		}
		else {
			strcpy(pOutputData->szErrorMessage, "Root Complex Base Address == 0xFFFFFFFF");
		}
		return ntStatus;
	}

	volatile PULONG pIOTR0 = (PULONG)((ULONG64)hIoMemMap + 0x1E80);
	volatile PULONG pIOTR1 = (PULONG)((ULONG64)hIoMemMap + 0x1E88);
	volatile PULONG pIOTR2 = (PULONG)((ULONG64)hIoMemMap + 0x1E90);
	volatile PULONG pIOTR3 = (PULONG)((ULONG64)hIoMemMap + 0x1E98);

	pOutputData->qwIOTRn[0] = READ_REGISTER_ULONG(pIOTR0);
	pOutputData->qwIOTRn[0] |= ((ULONGLONG)READ_REGISTER_ULONG(pIOTR0 + 1) << 32);
	pOutputData->qwIOTRn[1] = READ_REGISTER_ULONG(pIOTR1);
	pOutputData->qwIOTRn[1] |= ((ULONGLONG)READ_REGISTER_ULONG(pIOTR1 + 1) << 32);
	pOutputData->qwIOTRn[2] = READ_REGISTER_ULONG(pIOTR2);
	pOutputData->qwIOTRn[2] |= ((ULONGLONG)READ_REGISTER_ULONG(pIOTR2 + 1) << 32);
	pOutputData->qwIOTRn[3] = READ_REGISTER_ULONG(pIOTR3);
	pOutputData->qwIOTRn[3] |= ((ULONGLONG)READ_REGISTER_ULONG(pIOTR3 + 1) << 32);

	if (hIoMemMap != NULL) {
		MmUnmapIoSpace(hIoMemMap, RCBA_CHUNK_SIZE);
		hIoMemMap = NULL;
	}

	// I/O APIC
	pOutputData->dwApicBaseAddress = GetApicBaseAddress();
	pOutputData->dwIoApicBaseAddress = IO_APIC_BASE_ADDRESS;
	if (pOutputData->dwIoApicBaseAddress != 0 && pOutputData->dwIoApicBaseAddress != 0xFFFFFFFF) {
		PHYSICAL_ADDRESS physAddress = { 0 };
		physAddress.LowPart = pOutputData->dwIoApicBaseAddress;// &APIC_MASK;
		hIoMemMap = MmMapIoSpace(physAddress, APIC_CHUNK_SIZE, MmNonCached);
		if (hIoMemMap == NULL) {
			ntStatus = STATUS_UNSUCCESSFUL;
			pOutputData->ntStatusCode = ntStatus;
			strcpy(pOutputData->szErrorMessage, "MmMapIoSpaceEx[physAddress, APIC_CHUNK_SIZE, PAGE_READONLY] FAILED!");
			return ntStatus;
		}
	}
	else {
		ntStatus = STATUS_UNSUCCESSFUL;
		pOutputData->ntStatusCode = ntStatus;
		if (pOutputData->dwApicBaseAddress == 0) {
			strcpy(pOutputData->szErrorMessage, "IO APIC Base Address == 0x00000000");
		}
		else {
			strcpy(pOutputData->szErrorMessage, "IO APIC Base Address == 0xFFFFFFFF");
		}
		return ntStatus;
	}

	volatile PULONG pApicIoRegSel = (PULONG)hIoMemMap;
	volatile PULONG pApicIoWin = (PULONG)((ULONGLONG)hIoMemMap + 0x10);

	for (int i = 0, irq = 0x10; i < IO_APIC_IRQ_COUNT, irq <= 0x3E; i++, irq += 2)
	{
		WRITE_REGISTER_ULONG(pApicIoRegSel, irq);
		pOutputData->qwIOAPIC_REDTBL[i] = READ_REGISTER_ULONG(pApicIoWin);

		WRITE_REGISTER_ULONG(pApicIoRegSel, irq + 1);
		pOutputData->qwIOAPIC_REDTBL[i] |= ((ULONGLONG)READ_REGISTER_ULONG(pApicIoWin) << 32);
	}

	if (hIoMemMap != NULL) {
		MmUnmapIoSpace(hIoMemMap, APIC_CHUNK_SIZE);
		hIoMemMap = NULL;
	}


	return ntStatus;
}
