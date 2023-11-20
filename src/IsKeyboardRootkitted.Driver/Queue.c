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
/*
VOID
IsKeyboardRootkittedDriverEvtIoDeviceControl(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode
    )
/*++

Routine Description:

    This event is invoked when the framework receives IRP_MJ_DEVICE_CONTROL request.

Arguments:

    Queue -  Handle to the framework queue object that is associated with the
             I/O request.

    Request - Handle to a framework request object.

    OutputBufferLength - Size of the output buffer in bytes

    InputBufferLength - Size of the input buffer in bytes

    IoControlCode - I/O control code.

Return Value:

    VOID

--*/
//{
//    TraceEvents(TRACE_LEVEL_INFORMATION, 
//                TRACE_QUEUE, 
//                "%!FUNC! Queue 0x%p, Request 0x%p OutputBufferLength %d InputBufferLength %d IoControlCode %d", 
//                Queue, Request, (int) OutputBufferLength, (int) InputBufferLength, IoControlCode);
//
//    WdfRequestComplete(Request, STATUS_SUCCESS);
//
//    return;
//}
//
//VOID
//IsKeyboardRootkittedDriverEvtIoStop(
//    _In_ WDFQUEUE Queue,
//    _In_ WDFREQUEST Request,
//    _In_ ULONG ActionFlags
//)
//++
//
//Routine Description:
//
//    This event is invoked for a power-managed queue before the device leaves the working state (D0).
//
//Arguments:
//
//    Queue -  Handle to the framework queue object that is associated with the
//             I/O request.
//
//    Request - Handle to a framework request object.
//
//    ActionFlags - A bitwise OR of one or more WDF_REQUEST_STOP_ACTION_FLAGS-typed flags
//                  that identify the reason that the callback function is being called
//                  and whether the request is cancelable.
//
//Return Value:
//
//    VOID
//
//--*/
//{
//    TraceEvents(TRACE_LEVEL_INFORMATION, 
//                TRACE_QUEUE, 
//                "%!FUNC! Queue 0x%p, Request 0x%p ActionFlags %d", 
//                Queue, Request, ActionFlags);
//
//    //
//    // In most cases, the EvtIoStop callback function completes, cancels, or postpones
//    // further processing of the I/O request.
//    //
//    // Typically, the driver uses the following rules:
//    //
//    // - If the driver owns the I/O request, it calls WdfRequestUnmarkCancelable
//    //   (if the request is cancelable) and either calls WdfRequestStopAcknowledge
//    //   with a Requeue value of TRUE, or it calls WdfRequestComplete with a
//    //   completion status value of STATUS_SUCCESS or STATUS_CANCELLED.
//    //
//    //   Before it can call these methods safely, the driver must make sure that
//    //   its implementation of EvtIoStop has exclusive access to the request.
//    //
//    //   In order to do that, the driver must synchronize access to the request
//    //   to prevent other threads from manipulating the request concurrently.
//    //   The synchronization method you choose will depend on your driver's design.
//    //
//    //   For example, if the request is held in a shared context, the EvtIoStop callback
//    //   might acquire an internal driver lock, take the request from the shared context,
//    //   and then release the lock. At this point, the EvtIoStop callback owns the request
//    //   and can safely complete or requeue the request.
//    //
//    // - If the driver has forwarded the I/O request to an I/O target, it either calls
//    //   WdfRequestCancelSentRequest to attempt to cancel the request, or it postpones
//    //   further processing of the request and calls WdfRequestStopAcknowledge with
//    //   a Requeue value of FALSE.
//    //
//    // A driver might choose to take no action in EvtIoStop for requests that are
//    // guaranteed to complete in a small amount of time.
//    //
//    // In this case, the framework waits until the specified request is complete
//    // before moving the device (or system) to a lower power state or removing the device.
//    // Potentially, this inaction can prevent a system from entering its hibernation state
//    // or another low system power state. In extreme cases, it can cause the system
//    // to crash with bugcheck code 9F.
//    //
//
//    return;
//}

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
