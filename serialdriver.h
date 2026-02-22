#ifndef __serialdriver_h
#define __serialdriver_h


#define COM1_IRQ 				4
#define COM1_RBR 				0x3F8
#define COM1_THR 				0x3F8


#define COM1_IER 				0x3F9
#define ACTIVATE_INTERRUPTS 	0x07
#define DEACTIVATE_INTERRUPTS 	0x00

#define COM1_IIR 				0x3FA
#define COM1_FCR 				0x3FA
#define SOFTWARE_BUFFER			0x00

#define COM1_LCR 				0x3FB
#define SET_SPEED 				0x80
#define TRANSMIT_8BIT			0x03

#define COM1_MCR				0x3FC
#define ALLOW_INTERRUPTS		0x0F

#define COM1_LSR				0x3FD

#define COM1_MSR				0x3FE

#define COM1_DLL				0x3F8
#define COM1_DLM				0x3F9

#define DLL_115K				0x01
#define DLM_115K				0x00


#endif