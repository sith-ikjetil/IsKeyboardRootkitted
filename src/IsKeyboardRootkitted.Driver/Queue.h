/*++

Module Name:

    queue.h

Abstract:

    This file contains the queue definitions.

Environment:

    Kernel-mode Driver Framework

--*/

EXTERN_C_START

//
// This is the context that can be placed per queue
// and would contain per queue information.
//
typedef struct _QUEUE_CONTEXT {

    // Here we allocate a buffer from a test write so it can be read back 
    WDFMEMORY	WriteMemory;
    NTSTATUS	CurrentStatus;
    WDFREQUEST	CurrentRequest;
    WDFDEVICE   CurrentDevice;

} QUEUE_CONTEXT, *PQUEUE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(QUEUE_CONTEXT, QueueGetContext)

NTSTATUS
IsKeyboardRootkittedDriverQueueInitialize(
    _In_ WDFDEVICE Device
    );

EVT_WDF_IO_QUEUE_CONTEXT_DESTROY_CALLBACK KKSKmdIsKeyboardRktEvtIoQueueContextDestroy;

//
// Events from the IoQueue object
//
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL IsKeyboardRootkittedDriverEvtIoDeviceControl;
EVT_WDF_IO_QUEUE_IO_STOP IsKeyboardRootkittedDriverEvtIoStop;

EVT_WDF_IO_QUEUE_IO_READ KKSKmdIsKeyboardRktEvtIoRead;
EVT_WDF_IO_QUEUE_IO_WRITE KKSKmdIsKeyboardRktEvtIoWrite;

EXTERN_C_END
