/******************************************************************************
* Copyright (C) 2002 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xuartns550_sinit.c
* @addtogroup uartns550_v3_6
* @{
*
* The implementation of the XUartNs550 component's static initialization
* functionality.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.01a jvb  10/13/05 First release
* 1.11a sv   03/20/07 Updated to use the new coding guidelines.
* 2.00a ktn  10/20/09 Updated to use HAL Processor APIs.
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xstatus.h"
#include "xparameters.h"
#include "xuartns550_i.h"

/************************** Constant Definitions ****************************/

#ifndef XPAR_DEFAULT_BAUD_RATE
#define XPAR_DEFAULT_BAUD_RATE 1562500
#endif

/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Variable Definitions ****************************/


/************************** Function Prototypes *****************************/

/****************************************************************************/
/**
** 根据唯一设备 ID 查找设备配置。系统会生成一个表，其中包含系统中每个设备的配置信息。
*
* @param DeviceId 包含要查找配置的设备 ID。
*
* @return 指向找到的配置的指针；如果未找到指定的设备 ID，则返回 NULL。
* Looks up the device configuration based on the unique device ID. A table
* contains the configuration info for each device in the system.
*
* @param	DeviceId contains the ID of the device to look up the
*		configuration for.
*
* @return	A pointer to the configuration found or NULL if the specified
*		device ID was not found.
*
* @note		None.
*
******************************************************************************/
XUartNs550_Config *XUartNs550_LookupConfig(u16 DeviceId)
{
	XUartNs550_Config *CfgPtr = NULL;
	XUartNs550_Config temp;
	u32 Index;
	//=========================add by ycf 2025.8.6====================
	GetUartBase();

	{
		temp.DeviceId		=	XPAR_UARTNS550_0_DEVICE_ID;
		temp.BaseAddress	=	UART_REGS+0x30000;
		temp.InputClockHz	=	XPAR_UARTNS550_0_CLOCK_HZ;
	}
	XUartNs550_ConfigTable[0] =temp;

	{
		temp.DeviceId		=	XPAR_UARTNS550_1_DEVICE_ID;
		temp.BaseAddress	=	UART_REGS+0x40000;
		temp.InputClockHz	=	XPAR_UARTNS550_1_CLOCK_HZ;
	}
	XUartNs550_ConfigTable[1] =temp;
	//=========================add by ycf 2025.8.6====================

	for (Index=0; Index < XPAR_XUARTNS550_NUM_INSTANCES; Index++) {
		if (XUartNs550_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XUartNs550_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}

/****************************************************************************/
/**
*
* 初始化特定的 XUartNs550 实例，使其可供使用。 设备的数据格式默认设置为 8 个数据位、1 个停止位和无奇偶校验。
* 如果定义了符号，则将波特率设置为由XPAR_DEFAULT_BAUD_RATE 指定的默认值，否则设置为19.2K 波特。
* 如果设备具有 FIFO（16550 个），则启用它们并将接收 FIFO 阈值设置为 8 字节。

*驱动程序的默认工作模式为轮询模式。
*
* @param InstancePtr 是指向 XUartNs550 实例的指针。
* @param DeviceId 是此 XUartNs550 实例控制的设备的唯一 ID。传入设备ID会将通用 XUartNs550 实例关联到调用者或应用程序开发者选择的特定设备。
*
* @return
*
* - 如果初始化成功，则返回 XST_SUCCESS
* - 如果在配置表中找不到设备 ID，则返回 XST_DEVICE_NOT_FOUND
* - 如果波特率无法实现，则返回 XST_UART_BAUD_ERROR，因为输入时钟频率无法被可接受的误差量整除
* Initializes a specific XUartNs550 instance such that it is ready to be used.
* The data format of the device is setup for 8 data bits, 1 stop bit, and no
* parity by default. The baud rate is set to a default value specified by
* XPAR_DEFAULT_BAUD_RATE if the symbol is defined, otherwise it is set to
* 19.2K baud. If the device has FIFOs (16550), they are enabled and the a
* receive FIFO threshold is set for 8 bytes. The default operating mode of the
* driver is polled mode.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance .
* @param	DeviceId is the unique id of the device controlled by this
*		XUartNs550 instance. Passing in a device id associates the
*		generic XUartNs550 instance to a specific device, as chosen
*		by the caller or application developer.
*
* @return
*
* 		- XST_SUCCESS if initialization was successful
* 		- XST_DEVICE_NOT_FOUND if the device ID could not be found in
*		the configuration table
* 		- XST_UART_BAUD_ERROR if the baud rate is not possible because
*		the input clock frequency is not divisible with an acceptable
*		amount of error
*
* @note		None.
*
*****************************************************************************/
int XUartNs550_Initialize(XUartNs550 *InstancePtr, u16 DeviceId)
{
	XUartNs550_Config *ConfigPtr;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * Lookup the device configuration in the temporary CROM table. Use this
	 * configuration info down below when initializing this component
	 */
	ConfigPtr = XUartNs550_LookupConfig(DeviceId);
	if (ConfigPtr == (XUartNs550_Config *)NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	ConfigPtr->DefaultBaudRate = XPAR_DEFAULT_BAUD_RATE;
	return XUartNs550_CfgInitialize(InstancePtr, ConfigPtr,
					ConfigPtr->BaseAddress);
}
/** @} */
