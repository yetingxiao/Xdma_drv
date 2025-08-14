/******************************************************************************
* Copyright (C) 2002 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xuartns550_l.h
* @addtogroup uartns550_v3_6
* @{
*
* This header file contains identifiers and low-level driver functions (or
* macros) that can be used to access the device. The user should refer to the
* hardware device specification for more details of the device operation.
* High-level driver functions are defined in xuartns550.h.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b jhl  04/24/02 First release
* 1.11a sv   03/20/07 Updated to use the new coding guidelines.
* 1.11a rpm  11/13/07 Fixed bug in _EnableIntr
* 2.00a ktn  10/20/09 Converted all register accesses to 32 bit access.
*		      Updated to use HAL Processor APIs. _m is removed from the
*		      name of all the macro definitions.
* 3.4   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
*                     Changed the prototypes of XUartNs550_SendByte,
*                     XUartNs550_RecvByte, XUartNs550_SetBaud APIs.
* 3.6   sd   02/03/20 Updated the register macros for DRL and DRM registers.
* </pre>
*
******************************************************************************/

#ifndef XUARTNS550_L_H /* prevent circular inclusions */
#define XUARTNS550_L_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/*
 * Offset from the device base address to the IP registers.
 */
#define XUN_REG_OFFSET 0x1000

/** @name Register Map
 *
 * Register offsets for the 16450/16550 compatible UART device.
 * @{
 */
#define XUN_RBR_OFFSET	(XUN_REG_OFFSET) /**< Receive buffer, read only */
#define XUN_THR_OFFSET	(XUN_REG_OFFSET) /**< Transmit holding register */
#define XUN_IER_OFFSET	(XUN_REG_OFFSET + 0x04) /**< Interrupt enable */
#define XUN_IIR_OFFSET	(XUN_REG_OFFSET + 0x08) /**< Interrupt id, read only */
#define XUN_FCR_OFFSET	(XUN_REG_OFFSET + 0x08) /**< Fifo control, write only */
#define XUN_LCR_OFFSET	(XUN_REG_OFFSET + 0x0C) /**< Line Control Register */
#define XUN_MCR_OFFSET	(XUN_REG_OFFSET + 0x10) /**< Modem Control Register */
#define XUN_LSR_OFFSET	(XUN_REG_OFFSET + 0x14) /**< Line Status Register */
#define XUN_MSR_OFFSET	(XUN_REG_OFFSET + 0x18) /**< Modem Status Register */
#define XUN_DLL_OFFSET	(XUN_REG_OFFSET + 0x00) /**< Divisor Register LSB */
#define XUN_DLM_OFFSET	(XUN_REG_OFFSET + 0x04) /**< Divisor Register MSB */
#define XUN_DRLS_OFFSET	(XUN_REG_OFFSET + 0x00) /**< Divisor Register LSB */
#define XUN_DRLM_OFFSET	(XUN_REG_OFFSET + 0x04) /**< Divisor Register MSB */
/* @} */

/*
 * The following constant specifies the size of the FIFOs, the size of the
 * FIFOs includes the transmitter and receiver such that it is the total number
 * of bytes that the UART can buffer
 */
#define XUN_FIFO_SIZE			16


/**
 * @name Interrupt Enable Register (IER) mask(s)
 * @{
 */
#define XUN_IER_MODEM_STATUS	0x00000008 /**< Modem status interrupt */
#define XUN_IER_RX_LINE		0x00000004 /**< Receive status interrupt */
#define XUN_IER_TX_EMPTY	0x00000002 /**< Transmitter empty interrupt 发送缓冲区空中断*/
#define XUN_IER_RX_DATA		0x00000001 /**< Receiver data available 接收缓冲区有数据中断*/
/* @} */

/**
 * @name Interrupt ID Register (INT_ID) mask(s)
 * @{
 */
#define XUN_INT_ID_MASK		 0x0000000F /**< Only the interrupt ID */
#define XUN_INT_ID_FIFOS_ENABLED 0x000000C0 /**< Only the FIFOs enable */
/* @} */

/**
 * @name FIFO Control Register mask(s)
 * @{
 */
#define XUN_FIFO_RX_TRIG_MSB	0x00000080 /**< Trigger level MSB */
#define XUN_FIFO_RX_TRIG_LSB	0x00000040 /**< Trigger level LSB */
#define XUN_FIFO_TX_RESET	0x00000004 /**< Reset the transmit FIFO */
#define XUN_FIFO_RX_RESET	0x00000002 /**< Reset the receive FIFO */
#define XUN_FIFO_ENABLE		0x00000001 /**< Enable the FIFOs */
#define XUN_FIFO_RX_TRIGGER	0x000000C0 /**< Both trigger level bits */
/* @} */

/**
 * @name Line Control Register(LCR) mask(s)
 * @{
 */
#define XUN_LCR_DLAB		0x00000080 /**< Divisor latch access */
#define XUN_LCR_SET_BREAK	0x00000040 /**< Cause a break condition */
#define XUN_LCR_STICK_PARITY	0x00000020 /**< Stick Parity */
#define XUN_LCR_EVEN_PARITY	0x00000010 /**< 1 = even, 0 = odd parity */
#define XUN_LCR_ENABLE_PARITY	0x00000008 /**< 1 = Enable, 0 = Disable parity*/
#define XUN_LCR_2_STOP_BITS	0x00000004 /**< 1= 2 stop bits,0 = 1 stop bit */
#define XUN_LCR_8_DATA_BITS	0x00000003 /**< 8 Data bits selection */
#define XUN_LCR_7_DATA_BITS	0x00000002 /**< 7 Data bits selection */
#define XUN_LCR_6_DATA_BITS	0x00000001 /**< 6 Data bits selection */
#define XUN_LCR_LENGTH_MASK	0x00000003 /**< Both length bits mask */
#define XUN_LCR_PARITY_MASK	0x00000018 /**< Both parity bits mask */
/* @} */

/**
 * @name Mode Control Register(MCR) mask(s)
 * @{
 */
#define XUN_MCR_LOOP		0x00000010 /**< Local loopback */
#define XUN_MCR_OUT_2		0x00000008 /**< General output 2 signal */
#define XUN_MCR_OUT_1		0x00000004 /**< General output 1 signal */
#define XUN_MCR_RTS		0x00000002 /**< RTS signal */
#define XUN_MCR_DTR		0x00000001 /**< DTR signal */
/* @} */

/**接收器和发射器的当前状态
 * @name Line Status Register(LSR) mask(s)
 * @{
 */
 /**< An errored byte is in FIFO RCVR FIFO 错误(1)：RCVR FIFO 至少包含一个接收器错误（奇偶校验、帧错误、中断条件）。*/
#define XUN_LSR_RX_FIFO_ERROR	0x00000080 
/*
*< Transmitter is empty 
0 - THR 或发送器移位寄存器		包含数据。
1 - THR 和发送器移位寄存器		为空。
在 FIFO 模式下，发送器 FIFO 和移位寄存器均为空。
*/
#define XUN_LSR_TX_EMPTY	0x00000040 
/*
*< Transmit holding reg empty
0 - THR 或发送器FIFO中		有数据需要发送。
1 - THR 为空。在FIFO模式下		发送器 FIFO 为空。 
*/
#define XUN_LSR_TX_BUFFER_EMPTY 0x00000020 
/*
*< Break detected interrupt 当Sin保持低电平并持续整个字符时间（起始位 + 数据位 + 奇偶校验位 + 停止位）时，设置此错误。
*在FIFO模式下，此错误与FIFO中的特定字符相关联。如果Sin进入标记状态并接收到下一个有效的起始位，则启用下一个字符传输。
*/
#define XUN_LSR_BREAK_INT	0x00000010 
 /*
*< Framing error on current byte 字符缺少停止位。
发生帧错误时，UART 会尝试重新同步，并假设帧错误是由下一个字符的起始位引起的，因此它会对起始位进行两次采样，然后接收后续数据。
在FIFO模式下，此错误与 FIFO 中的特定字符相关*/
#define XUN_LSR_FRAMING_ERROR	0x00000008
/*
*< Parity error on current byte表示接收到的数据字符不具有由偶校验选择位选择的正确的偶校验或奇校验。
在 FIFO 模式下，此错误与 FIFO 中的特定字符相关 
*/
#define XUN_LSR_PARITY_ERROR	0x00000004 
/*
*< Overrun error on receive FIFO 在接收到下一个字符之前未读取RBR，从而破坏了前一个字符,发生溢出错误。
在FIFO模式下，数据持续填充FIFO，超出触发级别.
只有在 FIFO已满且下一个字符已完全接收到移位寄存器后，才会发生溢出错误。
移位寄存器中的字符会被覆盖，但不会传输到 FIFO。
*/
#define XUN_LSR_OVERRUN_ERROR	0x00000002 
#define XUN_LSR_DATA_READY	0x00000001 /**< Receive data ready */
#define XUN_LSR_ERROR_BREAK	0x0000001E /**< Errors except FIFO error and break detected */
/* @} */

#define XUN_DIVISOR_BYTE_MASK	0x000000FF

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
* Read a UART register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to select the specific register.
*
* @return	The value read from the register.
*
* @note		C-Style signature:
*		u32 XUartNs550_ReadReg(u32 BaseAddress, u32 RegOffset);
*
******************************************************************************/
#define XUartNs550_ReadReg(BaseAddress, RegOffset) \
	Xil_In32((BaseAddress) + (RegOffset))

/****************************************************************************/
/**
* Write to a UART register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to select the specific register.
* @param	RegisterValue is the value to be written to the register.
*
* @return	None.
*
* @note		C-Style signature:
*		u32 XUartNs550_WriteReg(u32 BaseAddress, u32 RegOffset,
*						u32 RegisterValue);
*
******************************************************************************/
#define XUartNs550_WriteReg(BaseAddress, RegOffset, RegisterValue) \
	Xil_Out32((BaseAddress) + (RegOffset), (RegisterValue))

/****************************************************************************/
/**
* Get the UART Line Status Register.
*
* @param	BaseAddress contains the base address of the device.
*
* @return	The value read from the register.
*
* @note		C-Style signature:
*		u32 XUartNs550_GetLineStatusReg(u32 BaseAddress);
*
******************************************************************************/
#define XUartNs550_GetLineStatusReg(BaseAddress)   \
	XUartNs550_ReadReg((BaseAddress), XUN_LSR_OFFSET)

/****************************************************************************/
/**
* Get the UART Line Status Register.
*
* @param	BaseAddress contains the base address of the device.
*
* @return	The value read from the register.
*
* @note		C-Style signature:
*		u32 XUartNs550_GetLineControlReg(u32 BaseAddress);
*
******************************************************************************/
#define XUartNs550_GetLineControlReg(BaseAddress)  \
	XUartNs550_ReadReg((BaseAddress), XUN_LCR_OFFSET)

/****************************************************************************/
/**
* Set the UART Line Status Register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegisterValue is the value to be written to the register.
*
* @return	None.
*
* @note		C-Style signature:
*		void XUartNs550_SetLineControlReg(u32 BaseAddress,
*				u32 RegisterValue);
*
******************************************************************************/
#define XUartNs550_SetLineControlReg(BaseAddress, RegisterValue) \
	XUartNs550_WriteReg((BaseAddress), XUN_LCR_OFFSET, (RegisterValue))

/****************************************************************************/
/**
* Enable the transmit and receive interrupts of the UART.
*
* @param	BaseAddress contains the base address of the device.
*
* @return	None.
*
* @note		C-Style signature:
*		void XUartNs550_EnableIntr(u32 BaseAddress);,
*
******************************************************************************/
#define XUartNs550_EnableIntr(BaseAddress)				\
	XUartNs550_WriteReg((BaseAddress), XUN_IER_OFFSET,		\
			 XUartNs550_ReadReg((BaseAddress), XUN_IER_OFFSET) | \
			 (XUN_IER_RX_LINE | XUN_IER_TX_EMPTY | XUN_IER_RX_DATA))

/****************************************************************************/
/**
* Disable the transmit and receive interrupts of the UART.
*
* @param	BaseAddress contains the base address of the device.
*
* @return	None.
*
* @note		C-Style signature:
*		void XUartNs550_DisableIntr(u32 BaseAddress);,
*
******************************************************************************/
#define XUartNs550_DisableIntr(BaseAddress)				\
	XUartNs550_WriteReg((BaseAddress), XUN_IER_OFFSET,		\
			XUartNs550_ReadReg((BaseAddress), XUN_IER_OFFSET) & \
			~(XUN_IER_RX_LINE | XUN_IER_TX_EMPTY | XUN_IER_RX_DATA))

/****************************************************************************/
/**
* 判断接收器和/或 FIFO 中是否有接收数据。
*
* @param BaseAddress 包含设备的基地址。
*
* @return 如果有接收数据，则返回 TRUE；否则返回 FALSE。
* Determine if there is receive data in the receiver and/or FIFO.
*
* @param	BaseAddress contains the base address of the device.
*
* @return	TRUE if there is receive data, FALSE otherwise.
*
* @note		C-Style signature:
*		int XUartNs550_IsReceiveData(u32 BaseAddress);,
*
******************************************************************************/
#define XUartNs550_IsReceiveData(BaseAddress)				\
	(XUartNs550_GetLineStatusReg(BaseAddress) & XUN_LSR_DATA_READY)

/****************************************************************************/
/**
* 确定是否可以使用发射器发送一个字节的数据。
*
* @param BaseAddress 包含设备的基地址。
*
* @return 如果可以发送一个字节，则返回 TRUE；否则，返回 FALSE。
* Determine if a byte of data can be sent with the transmitter.
*
* @param	BaseAddress contains the base address of the device.
*
* @return	TRUE if a byte can be sent, FALSE otherwise.
*
* @note		C-Style signature:
*		int XUartNs550_IsTransmitEmpty(u32 BaseAddress);,
*
******************************************************************************/
#define XUartNs550_IsTransmitEmpty(BaseAddress)			\
	(XUartNs550_GetLineStatusReg(BaseAddress) & XUN_LSR_TX_BUFFER_EMPTY)

/************************** Function Prototypes ******************************/

void XUartNs550_SendByte(UINTPTR BaseAddress, u8 Data);

u8 XUartNs550_RecvByte(UINTPTR BaseAddress);

void XUartNs550_SetBaud(UINTPTR BaseAddress, u32 InputClockHz, u32 BaudRate);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
