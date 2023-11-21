; check for smi keyboard rootkit

.data

; bit 31=1        (bit is always set for a PCI access)
; bits30:24=0 (reserved)
; bit 23:16=bus number (0-255)
; bits15:11=device # (0-31)
; bits10:8=function # (0-7)
; bits7:0=register number (0-255)
;
; you send the above bit-encoded value out to the index port (cf8h) and then do a 32bit read from the data port (cfch)

RCBA_BUS		equ 0h		; 
RCBA_DEVICE		equ 31		; 
RCBA_FUNCTION	equ 0h		; 

LPC_RCBA_REGISTER_OFFSET					equ 0f0h		; Root Complex Base Address Offset
PCI_CONFIGURATION_SPACE_ADDRESS_PORT		equ 0cf8h		; Configuration Space Address 
PCI_CONFIGURATION_SPACE_DATA_PORT			equ 0cfch		; Configuration Space Data 

.code
GetRootComplexBaseAddress PROC	
;
; Save registers to stack
;
	PUSH R8
	PUSH R9
	PUSH R10
	PUSH R11	
	PUSH RDX
	PUSH RSI
	PUSH RDI
	PUSH RBX	
;
; Get RCBA Address...	
;
	XOR R8, R8
	MOV R8, RCBA_BUS
	SHL R8, 16

	XOR R9, R9
	MOV R9, RCBA_DEVICE
	SHL R9, 11

	XOR R10, R10
	MOV R10, RCBA_FUNCTION
	SHL R10, 8

	XOR R11, R11
	MOV R11, LPC_RCBA_REGISTER_OFFSET	

	MOV RAX, R8
	XOR RAX, R9
	XOR RAX, R10
	XOR RAX, R11
	XOR RAX, 80000000h	

	CLI
	
	MOV DX, PCI_CONFIGURATION_SPACE_ADDRESS_PORT
	OUT DX, EAX

	MOV DX, PCI_CONFIGURATION_SPACE_DATA_PORT
	IN  EAX, DX
	
	STI

;
; POP registers from stack
;
	POP RBX
	POP RDI
	POP RSI
	POP RDX	
	POP R11
	POP R10
	POP R9
	POP R8
;
; Return
;
	RET
GetRootComplexBaseAddress ENDP

GetApicBaseAddress PROC
;
; Save registers to stack
;	
	PUSH RDX
	PUSH RCX	
; 
	MOV      ECX, 1Bh
	XOR      EAX, EAX
	XOR      EDX, EDX
	RDMSR
	; EDX:EAX = APICBASE
	SHL	RDX, 32
	MOV EDX, EAX
	MOV RAX, RDX
;
; POP registers from stack
;
	POP RCX
	POP RDX	
;
; Return
;
	RET
GetApicBaseAddress ENDP
END