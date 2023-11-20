#ifndef __IS_KEYBOARD_RKT_OUTPUT_DATA_H__
#define __IS_KEYBOARD_RKT_OUTPUT_DATA_H__

#include "IsKbdRkt.h"

//
// Main Transfer Data
//
typedef struct IS_KEYBOARD_RKT_OUTPUT_DATA {
	unsigned short		cbSize;						// Size of Structure.
	unsigned long		dwRootComplexBaseAddress;	// Root Complex Base Address Register
	unsigned long		dwApicBaseAddress;			// Apic Base Address
	unsigned long		dwIoApicBaseAddress;		// IO Apic Base Address
	unsigned long long  qwIOTRn[IOTRn_COUNT];				// IOTR0 Chipset Configuration Register		
	unsigned long long  qwIOAPIC_REDTBL[IO_APIC_IRQ_COUNT];	// IOAPIC IRQ0		
	long				ntStatusCode;			// Driver NTSTATUS Error Code	
	char				szErrorMessage[MAX_STRING_BUFFER_SIZE];		// Driver ERROR MESSAGE	
} IS_KEYBOARD_RKT_OUTPUT_DATA, *PIS_KEYBOARD_RKT_OUTPUT_DATA;

#endif