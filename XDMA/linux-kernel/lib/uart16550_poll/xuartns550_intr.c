/******************************************************************************
* Copyright (C) 2002 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xuartns550_intr.c
* @addtogroup uartns550_v3_6
* @{
*
* This file contains the functions that are related to interrupt processing
* for the 16450/16550 UART driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b jhl  03/11/02 Repartitioned driver for smaller files.
* 1.11a sv   03/20/07 Updated to use the new coding guidelines.
* 2.00a ktn  10/20/09 Converted all register accesses to 32 bit access.
*		      Updated to use HAL Processor APIs. _m is removed from the
*		      name of all the macro definitions. XUartNs550_mClearStats
*		      macro is removed, XUartNs550_ClearStats function should be
*		      used in its place.
* 2.02a adk 09/16/13 Updated the ReceiveDataHandler function to be the same as
*                     ReceiveTimeoutHandler.  The ReceiveTimeoutHandler will
*		      call the callback function with XUN_EVENT_RECV_TIMEOUT when
*		      there is data received which is less than the requested
*		      data (this will also happen for the case where the
*		      data is equal to the threshold).
*		      The callback function with XUN_EVENT_RECV_DATA will be
*		      called when all the requested data has been received
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xuartns550.h"
#include "xuartns550_i.h"
#include "xil_io.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

static void NoInterruptHandler(XUartNs550 *InstancePtr);
static void ReceiveStatusHandler(XUartNs550 *InstancePtr);
static void ReceiveTimeoutHandler(XUartNs550 *InstancePtr);
static void ReceiveDataHandler(XUartNs550 *InstancePtr);
static void SendDataHandler(XUartNs550 *InstancePtr);
static void ModemHandler(XUartNs550 *InstancePtr);

/************************** Variable Definitions ****************************/

typedef void (*Handler)(XUartNs550 *InstancePtr);

/* The following tables is a function pointer table that contains pointers
 * to each of the handlers for specific kinds of interrupts. The table is
 * indexed by the value read from the interrupt ID register.
 */
static Handler HandlerTable[13] = {
	ModemHandler,		/* 0 */
	NoInterruptHandler,	/* 1 */
	SendDataHandler,	/* 2 */
	NULL,			/* 3 */
	ReceiveDataHandler,	/* 4 */
	NULL,			/* 5 */
	ReceiveStatusHandler,	/* 6 */
	NULL,			/* 7 */
	NULL,			/* 8 */
	NULL,			/* 9 */
	NULL,			/* 10 */
	NULL,			/* 11 */
	ReceiveTimeoutHandler	/* 12 */
};

/****************************************************************************/
/**
*
* 此函数设置在驱动程序中发生事件（中断）时将调用的处理程序。
* 该处理程序的目的是允许执行应用程序特定的处理。
*
* @param InstancePtr 是指向 XUartNs550 实例的指针。
* @param FuncPtr 是指向回调函数的指针。
* @param CallBackRef 是调用回调函数时传回的上层回调函数引用。
* This function sets the handler that will be called when an event (interrupt)
* occurs in the driver. The purpose of the handler is to allow application
* specific processing to be performed.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance.
* @param	FuncPtr is the pointer to the callback function.
* @param	CallBackRef is the upper layer callback reference passed back
*		when the callback function is invoked.
*
* @return	None.
*
* @note
*
* There is no assert on the CallBackRef since the driver doesn't know what it
* is (nor should it)
*
*****************************************************************************/
void XUartNs550_SetHandler(XUartNs550 *InstancePtr,
				XUartNs550_Handler FuncPtr, void *CallBackRef)
{
	/*
	 * Assert validates the input arguments
	 * CallBackRef not checked, no way to know what is valid
	 */
	printf("In XUartNs550_SetHandler\r\n");
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(FuncPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->Handler = FuncPtr;
	InstancePtr->CallBackRef = CallBackRef;
}

/****************************************************************************/
/**
** 此函数是 16450/16550 UART 驱动程序的中断处理程序。

* 用户必须将其连接到中断系统，以便在任何 16450/16550 UART 发生中断时调用它。

* 此函数不会保存或恢复处理器上下文，因此用户必须确保执行此操作。
* @param InstancePtr 包含指向中断所针对的 UART 实例的指针。
* This function is the interrupt handler for the 16450/16550 UART driver.
* It must be connected to an interrupt system by the user such that it is
* called when an interrupt for any 16450/16550 UART occurs. This function
* does not save or restore the processor context such that the user must
* ensure this occurs.
*
* @param	InstancePtr contains a pointer to the instance of the UART that
*		the interrupt is for.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XUartNs550_InterruptHandler(XUartNs550 *InstancePtr)
{
	u8 IsrStatus;

	Xil_AssertVoid(InstancePtr != NULL);

	/** 读取中断 ID IIR Interrupt Identification Register 寄存器以确定哪个（只有一个）中断处于活动状态
	 * Read the interrupt ID register to determine which, only one,
	 * interrupt is active
	 */
	IsrStatus = (u8)XUartNs550_ReadReg(InstancePtr->BaseAddress,
					XUN_IIR_OFFSET) &
					XUN_INT_ID_MASK;

	/** 确保处理程序表中已为处于活动状态的中断定义了处理程序然后调用该处理程序
	 * Make sure the handler table has a handler defined for the interrupt
	 * that is active, and then call the handler
	 */
	Xil_AssertVoid(HandlerTable[IsrStatus] != NULL);
	printf("XUartNs550_InterruptHandler For Every Interrupt\r\n");
	HandlerTable[IsrStatus](InstancePtr);
}

/****************************************************************************/
/**
** 此函数处理从中断 ID 寄存器读取的值指示无需处理任何中断的情况。*
* This function handles the case when the value read from the interrupt ID
* register indicates no interrupt is to be serviced.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance .
*
* @return	None.
*
* @note		None.
*
* @internal
*
* The LsrRegister is volatile to ensure that optimization will not cause the
* statement to be optimized away.
*
*****************************************************************************/
static void NoInterruptHandler(XUartNs550 *InstancePtr)
{
	volatile u32 LsrRegister;

	/*读取 ID 寄存器会清除当前断言的中断
	 * Reading the ID register clears the currently asserted interrupts
	 */
	LsrRegister = XUartNs550_GetLineStatusReg(InstancePtr->BaseAddress);

	/*
	 * Update the stats to reflect any errors that might be read
	 */
	XUartNs550_UpdateStats(InstancePtr, (u8)LsrRegister);
}

/****************************************************************************/
/**
* 此函数处理接收状态的中断
* 包括溢出错误、帧错误、奇偶校验错误和中断。
*
* @param InstancePtr 是指向 XUartNs550 实例的指针。
* This function handles interrupts for receive status updates which include
* overrun errors, framing errors, parity errors, and the break interrupt.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance.
*
* @return	None.
*
* @note
*
* If this handler executes and data is not supposed to be received, then
* this probably means data is being received that contains errors and the
* the user may need to clear the receive FIFOs to dump the data.
*
*****************************************************************************/
static void ReceiveStatusHandler(XUartNs550 *InstancePtr)
{
	u32 LsrRegister;

	/** 如果指定缓冲区中仍有待接收的字节则继续接收
	 * If there are bytes still to be received in the specified buffer
	 * go ahead and receive them
	 */
	if (InstancePtr->ReceiveBuffer.RemainingBytes != 0) {
		XUartNs550_ReceiveBuffer(InstancePtr);
	} else {
		/** 读取 ID LSR Line Status Register寄存器会清除当前断言的中断，由于没有数据因此必须执行此接收操作，更新读取状态的状态
		 * Reading the ID register clears the currently asserted
		 * interrupts and this must be done since there was no data
		 * to receive, update the status for the status read
		 */
		LsrRegister =
			XUartNs550_GetLineStatusReg(InstancePtr->BaseAddress);
		XUartNs550_UpdateStats(InstancePtr, (u8)LsrRegister);
	}

	/** 调用应用程序处理程序来指示存在接收错误或中断。如果应用程序关心错误，则调用函数来获取最后的错误。
	 * Call the application handler to indicate that there is a receive
	 * error or a break interrupt, if the application cares about the
	 * error it call a function to get the last errors
	 */
	InstancePtr->Handler(InstancePtr->CallBackRef, XUN_EVENT_RECV_ERROR,
				InstancePtr->ReceiveBuffer.RequestedBytes -
				InstancePtr->ReceiveBuffer.RemainingBytes);

	/*
	 * Update the receive stats to reflect the receive interrupt
	 */
	InstancePtr->Stats.StatusInterrupts++;
}
/****************************************************************************/
/**
*
* 此函数处理接收超时中断。

* 当 FIFO 中存在一定数量的字节，且持续 4 个字符时间，而接收器未接收到任何数据
* 且当前存在的字节数小于 FIFO 阈值时，就会发生此中断。
* InstancePtr 是指向 XUartNs550 实例的指针。
* This function handles the receive timeout interrupt. This interrupt occurs
* whenever a number of bytes have been present in the FIFO for 4 character
* times, the receiver is not receiving any data, and the number of bytes
* present is less than the FIFO threshold.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
static void ReceiveTimeoutHandler(XUartNs550 *InstancePtr)
{
	u32 Event;

	/** 如果指定缓冲区中仍有待接收的字节则继续接收
	 * If there are bytes still to be received in the specified buffer
	 * go ahead and receive them
	 */
	if (InstancePtr->ReceiveBuffer.RemainingBytes != 0) {
		XUartNs550_ReceiveBuffer(InstancePtr);
	}

	/*
	* 如果没有更多字节可接收，是已到达缓冲区末尾,不是接收超时
	* 即超时通常发生在字节数不能被 FIFO 阈值整除的情况下，不依赖之前对剩余字节的测试因为接收函数会更新它
	 * If there are no more bytes to receive then indicate that this is
	 * not a receive timeout but the end of the buffer reached, a timeout
	 * normally occurs if # of bytes is not divisible by FIFO threshold,
	 * don't rely on previous test of remaining bytes since receive function
	 * updates it
	 */
	if (InstancePtr->ReceiveBuffer.RemainingBytes != 0) {
		Event = XUN_EVENT_RECV_TIMEOUT;
	} else {
		Event = XUN_EVENT_RECV_DATA;//如果没有更多字节可接收，则指示这不是接收超时，而是已到达缓冲区末尾
	}

	/** 调用应用程序处理程序来指示存在接收超时或数据事件
	 * Call the application handler to indicate that there is a receive
	 * timeout or data event
	 */
	InstancePtr->Handler(InstancePtr->CallBackRef, Event,
			 InstancePtr->ReceiveBuffer.RequestedBytes -
			 InstancePtr->ReceiveBuffer.RemainingBytes);

	/*
	 * Update the receive stats to reflect the receive interrupt
	 */
	InstancePtr->Stats.ReceiveInterrupts++;
}
/****************************************************************************/
/**
* 此函数用于处理接收到数据时的中断。

*如果未启用 FIFO，则接收单个字节；如果启用 FIFO，则接收多个字节。
*
* @param InstancePtr 是指向 XUartNs550 实例的指针。
* This function handles the interrupt when data is received, either a single
* byte when FIFOs are not enabled, or multiple bytes with the FIFO.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
static void ReceiveDataHandler(XUartNs550 *InstancePtr)
{
	u32 Event;
	printf("XUartNs550_InterruptHandler For ReceiveDataHandler\r\n");

	/*
	 * If there are bytes still to be received in the specified buffer
	 * go ahead and receive them
	 *它会检测到是接收中断，会从UART的接收FIFO中读取数据，并将其存入 XUartNs550Buffer 的 NextBytePtr 指向的地址。
	 */
	if (InstancePtr->ReceiveBuffer.RemainingBytes != 0) {
		XUartNs550_ReceiveBuffer(InstancePtr);
	}


	if (InstancePtr->ReceiveBuffer.RemainingBytes != 0) {

		/*
		* 如果还有更多字节需要接收，则指示这是一个接收超时事件,一直没接收到数据事件。
		* 因为超时中断被屏蔽了,如果接收到的字节数等于FIFO阈值，才会发生这种情况。
		 * If there are more bytes to receive then indicate that this is
		 * a Receive Timeout.
		 * This happens in the case the number of bytes received equal
		 * to the FIFO threshold as the Timeout Interrupt is masked
		 */
		Event = XUN_EVENT_RECV_TIMEOUT;

	} else {

		/*
		* 如果收到了消息的最后一个字节，则调用应用程序处理程序
		* 指示所有数据已经接收完毕
		 * If the last byte of a message was received then call the
		 * application handler 
		 */
		Event = XUN_EVENT_RECV_DATA;
	}

	/*
	 * Call the application handler to indicate that there is a receive
	 * timeout or data event
	 */
	InstancePtr->Handler(InstancePtr->CallBackRef, Event,
			 InstancePtr->ReceiveBuffer.RequestedBytes -
			 InstancePtr->ReceiveBuffer.RemainingBytes);


	/*
	 * Update the receive stats to reflect the receive interrupt
	 */
	InstancePtr->Stats.ReceiveInterrupts++;
}

/****************************************************************************/
/**
* 此函数用于在数据已经发送完毕，传输FIFO为空（发送器保持寄存器）时处理中断。
*
* @param InstancePtr 是指向 XUartNs550 实例的指针。
* This function handles the interrupt when data has been sent, the transmit
* FIFO is empty (transmitter holding register).
*
* @param	InstancePtr is a pointer to the XUartNs550 instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
static void SendDataHandler(XUartNs550 *InstancePtr)
{
	u32 IerRegister;
	printf("XUartNs550_InterruptHandler For SendDataHandler\r\n");
	/** 如果指定缓冲区没有要发送的字节，则中断服务程序会禁用发送空中断，这样它就会停止中断，因为FIFO 为空时就会中断
	 * If there are not bytes to be sent from the specified buffer then
	 * disable the transmit interrupt so it will stop interrupting as it
	 * interrupts any time the FIFO is empty
	 */
	if (InstancePtr->SendBuffer.RemainingBytes == 0) {
		IerRegister = XUartNs550_ReadReg(InstancePtr->BaseAddress,
							XUN_IER_OFFSET);
		XUartNs550_WriteReg(InstancePtr->BaseAddress, XUN_IER_OFFSET,
				 IerRegister & ~XUN_IER_TX_EMPTY);//缓冲区没有要发送的字节，禁用发送中断

		/*当所有数据都发送完毕后，并调用一个“发送完成”的回调函数，通知应用程序传输已完成。
		 * Call the application handler to indicate the data
		 * has been sent
		 */
		 /**< Data has been sent*/
		InstancePtr->Handler(InstancePtr->CallBackRef,
				XUN_EVENT_SENT_DATA,
				InstancePtr->SendBuffer.RequestedBytes -
				InstancePtr->SendBuffer.RemainingBytes);
	}
	/*
	* 否则，指定的缓冲区中仍有更多数据需要发送,因此请继续发送,送过程会一直重复，直到RemainingBytes 变为0。
	 * Otherwise there is still more data to send in the specified buffer
	 * so go ahead and send it
	 */
	else {
		/*
		*发送会从 XUartNs550Buffer 中读取 NextBytePtr 和 RemainingBytes 的值。它会将 RemainingBytes 指示的下一批数据写入UART的FIFO中。
		*每写入一个字节，NextBytePtr 就会增加，RemainingBytes 就会减少
		*/
		XUartNs550_SendBuffer(InstancePtr);
	}

	/*
	 * Update the transmit stats to reflect the transmit interrupt
	 */
	InstancePtr->Stats.TransmitInterrupts++;
}

/****************************************************************************/
/**
*
* 此函数处理调制解调器中断。
* 它不执行任何处理，除了调用应用程序处理程序来指示调制解调器事件。
*
* @param InstancePtr 是指向 XUartNs550 实例的指针。
* This function handles modem interrupts. It does not do any processing
* except to call the application handler to indicate a modem event.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
static void ModemHandler(XUartNs550 *InstancePtr)
{
	u32 MsrRegister;

	/** 读取调制解调器状态寄存器，以便确认中断，并将其与事件一起传递给回调处理程序
	 * Read the modem status register so that the interrupt is acknowledged
	 * and so that it can be passed to the callback handler with the event
	 */
	MsrRegister = XUartNs550_ReadReg(InstancePtr->BaseAddress,
						XUN_MSR_OFFSET);

	/*
	 * Call the application handler to indicate the modem status changed,
	 * passing the modem status and the event data in the call
	 */
	InstancePtr->Handler(InstancePtr->CallBackRef, XUN_EVENT_MODEM,
						 (u8) MsrRegister);

	/*
	 * Update the modem stats to reflect the modem interrupt
	 */
	InstancePtr->Stats.ModemInterrupts++;
}
/** @} */
