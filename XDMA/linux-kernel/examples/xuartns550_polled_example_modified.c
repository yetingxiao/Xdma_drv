/******************************************************************************
* Copyright (C) 2002 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file     xuartns550_polled_example.c
*
* This file contains a design example using the Uart 16450/550 driver
* (XUartNs550) and hardware device using polled mode.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a jhl  02/13/02 First release
* 1.00b ecm  01/25/05 Modified for TestApp integration, updated boilerplate.
* 1.00b sv   06/08/05 Minor changes to comply to Doxygen and coding guidelines
* 2.00a ktn  10/20/09 Updated to use HAL processor APIs and minor modifications
*		      as per coding guidelines.
*		      Updated this example to wait for valid data in receive
*		      fifo instead of Tx fifo empty to update receive buffer
* 3.4   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include <time.h>
#include "xparameters.h"
#include "xuartns550.h"
#include "xil_printf.h"


/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define UART_DEVICE_ID		XPAR_UARTNS550_0_DEVICE_ID

/*
 * The following constant controls the length of the buffers to be sent
 * and received with the UART, this constant must be 16 bytes or less since
 * this is a single threaded non-interrupt driven example such that the
 * entire buffer will fit into the transmit and receive FIFOs of the UART
 */
#define TEST_BUFFER_SIZE 512

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int UartNs550PolledExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

XUartNs550 UartNs550;	/* Instance of the UART Device */

/*
 * The following buffers are used in this example to send and receive data
 * with the UART.
 */
u8 SendBuffer[TEST_BUFFER_SIZE];	/* Buffer for Transmitting Data */
char RecvBuffer[TEST_BUFFER_SIZE];	/* Buffer for Receiving Data */



/*****************************************************************************/
/**
*
* Main function to call the example. This function is not included if the
* example is generated from the TestAppGen test tool.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
#include "xuartns550.h"

// 假设 UartNs550Instance 已经被初始化
XUartNs550 UartNs550Instance;
XUartNs550Stats *StatsPtr;



int main(void)
{
	int Status;


	/*
	 * Run the UartNs550 polled example , specify the the Device ID that is
	 * generated in xparameters.h
	 */
	Status = UartNs550PolledExample(UART_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		printf("Uartns550 polled Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Uartns550 polled Example\r\n");
	return XST_SUCCESS;

}

/*****************************************************************************/
/**
*
* This function does a minimal test on the UART 16450/550 device and driver as a
* design example.  The purpose of this function is to illustrate how to use
* the XUartNs550 component.
*
* This function sends data and expects to receive the data through the UART
* using the local loopback mode of the UART hardware.
*
* This function polls the UART and does not require the use of interrupts.
*
* @param	DeviceId is the XPAR_<uartns550_instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		This function polls the UART such that it may be not return
*		if the hardware is not working correctly.
*
****************************************************************************/
int UartNs550PolledExample(u16 DeviceId)
{
	int Status;
	unsigned int SentCount;
	unsigned int ReceivedCount = 0;
	u16 Index;
	u16 Index2;
	u16 Options;
	int errornum=0;
	
	static int loop =0;
	static int loop2 =0;
	char* data="0123456789abcdefghijklmnopqrstuvwxyzzyxwvutsrqponmlkjihgfedcba98765432101234567890123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz0123456789012345678901234567890123456789abcdefghijklmnopqrstuvwxyz01234567899876543210abcdefghijklmnopq";
	char* Buffer=malloc(100*TEST_BUFFER_SIZE);
	StatsPtr=malloc(sizeof(XUartNs550Stats));
	/*
	 * Initialize the UART Lite driver so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h
	 */
	Status = XUartNs550_Initialize(&UartNs550, DeviceId);
	if (Status != XST_SUCCESS) {
		printf("XUartNs550_Initialize Error.\n");
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built  correctly
	 */
/* 	Status = XUartNs550_SelfTest(&UartNs550);
	if (Status != XST_SUCCESS) {
		printf("XUartNs550_SelfTest Error.\n");
		return XST_FAILURE;
	}
 */
	/*
	 * Enable the local loopback so data that is sent will be received,
	 * and keep the FIFOs enabled
	 */
	Options = XUN_OPTION_FIFOS_ENABLE;
	XUartNs550_SetOptions(&UartNs550, Options);

	/*
	 * Initialize the send buffer bytes with a pattern to send and the
	 * the receive buffer bytes to zero
	 */
	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		//SendBuffer[Index] = '0' + Index;
		RecvBuffer[Index] = 0;
	}

	/*
	 * Send the buffer through the UART waiting till the data can be
	 * sent (block), if the specified number of bytes was not sent
	 * successfully, then an error occurred
	 */
/* 	SentCount = XUartNs550_Send(&UartNs550, SendBuffer, TEST_BUFFER_SIZE);
	if (SentCount != TEST_BUFFER_SIZE) {
		printf("XUartNs550_Send Error.\n");
		return XST_FAILURE;
	} */

	/*
	 * Receive the number of bytes which is transferred.
	 * Data may be received in fifo with some delay hence we continuously
	 * check the receive fifo for valid data and update the receive buffer
	 * accordingly.
	 */
	while (1) {
			ReceivedCount += XUartNs550_Recv(&UartNs550,
					   RecvBuffer + ReceivedCount,
					   TEST_BUFFER_SIZE - ReceivedCount);
	
				
		   if (ReceivedCount == TEST_BUFFER_SIZE)
		   {
				//printf("XUartNs550_Receive %d cycle OK.\n",loop+1);
				
/* 				for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
					printf("Received byte %d: 0x%02X ('%c')\n", loop*TEST_BUFFER_SIZE+Index+1, RecvBuffer[Index], (isprint(RecvBuffer[Index]) ? RecvBuffer[Index] : '.'));
				} */ 
				for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
				//SendBuffer[Index] = '0' + Index;
					Buffer[loop*TEST_BUFFER_SIZE+Index]=RecvBuffer[Index];
				}
				
/* 				for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
				//SendBuffer[Index] = '0' + Index;
					RecvBuffer[Index] = 0;
				} */
				ReceivedCount=0;
				loop++;
		   }
		   
		   if (loop==100)
		   {
				//printf("XUartNs550_Receive %d cycle OK.\n",loop+1);
				
				for (Index = 0; Index < loop*2; Index++) {
					//printf("Received byte %d: 0x%02X ('%c')\n", (loop2)*100*TEST_BUFFER_SIZE+Index+1, Buffer[Index], (isprint(RecvBuffer[Index]) ? RecvBuffer[Index] : '.'));
					for (Index2 = 0; Index2 <256; Index2++) {
					//printf("Received byte %d: 0x%02X ('%c')\n", (loop2)*100*TEST_BUFFER_SIZE+Index+1, Buffer[Index], (isprint(RecvBuffer[Index]) ? RecvBuffer[Index] : '.'));
						if(data[Index2]!=Buffer[Index*256+Index2])
						{errornum++;}
					} 
				} 
				
				// 获取统计信息
				 XUartNs550_GetStats(&UartNs550,StatsPtr);

				// 现在，您可以检查这些错误计数器
				if (StatsPtr != NULL) {
					if (StatsPtr->ReceiveParityErrors > 0) {
						// 处理奇偶校验错误
						printf("Parity errors: %lu\n", (unsigned long)StatsPtr->ReceiveParityErrors);
					}
					if (StatsPtr->ReceiveFramingErrors > 0) {
						// 处理帧错误
						printf("Framing errors: %lu\n", (unsigned long)StatsPtr->ReceiveFramingErrors);
					}
					if (StatsPtr->ReceiveOverrunErrors > 0) {
						// 处理溢出错误
						printf("Overrun errors: %lu\n", (unsigned long)StatsPtr->ReceiveOverrunErrors);
					}
					if (StatsPtr != NULL) {
						// 打印已接收的字符数
						printf("Received characters: %lu\n", (unsigned long)StatsPtr->CharactersReceived);
					}

				}
				printf("Received error byte %d\n",errornum);
				loop=0;
				loop2++;
		   }
		   

			usleep(1); // 1微秒延迟
        }

	/*
	 * Check the receive buffer data against the send buffer and verify the
	 * data was correctly received
	 */
	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		if (SendBuffer[Index] != RecvBuffer[Index]) {
			return XST_FAILURE;
		}
	}

	/*
	 * Clean up the options
	 */
	Options = XUartNs550_GetOptions(&UartNs550);
	Options = Options & ~(XUN_OPTION_LOOPBACK | XUN_OPTION_FIFOS_ENABLE);
	XUartNs550_SetOptions(&UartNs550, Options);

	return XST_SUCCESS;
}
