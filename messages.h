/*=============================================================================
# 
# The content of this file or document is CONFIDENTIAL and PROPRIETARY
# to Maxim Integrated Products.  It is subject to the terms of a
# License Agreement between Licensee and Maxim Integrated Products.
# restricting among other things, the use, reproduction, distribution
# and transfer.  Each of the embodiments, including this information and
# any derivative work shall retain this copyright notice.
# 
#============================================================================*/
#ifndef __MESSAGES_H
#define __MESSAGES_H
//*****************************************************************************
// All system-registered messages
typedef enum {
	MSG_NULL		= 0x0000,
	MSG_INIT		= 0x0001,
	MSG_START		= 0x0002,
	MSG_STOP		= 0x0003,
	MSG_IDLE		= 0x0004,
	MSG_TERMINATE	= 0x0005,
	MSG_TIMER_TICK	= 0x0006,
	MSG_CONFIGURE	= 0x0007,
	MSG_SET_PARAM	= 0x0008,
	MSG_GET_PARAM	= 0x0009,
	MSG_DATA_RDY	= 0x000A, 
	MSG_CONFIG_RDY	= 0x000B,
	MSG_GET_RESP	= 0x000C,
	MSG_RESP_RDY	= 0x000D,
	MSG_GET_VERSION = 0x000E,
	MSG_GET_INFO 	= 0x000F,
	MSG_DIRECT_DATA_RDY = 0x0010
} mae_msg_t;

// Users may define their own messages.
typedef enum {
	USER_MSG_NULL	= 0x8000,
	USER_CONFIGURE	= 0x8001,
	USER_SET_PARAM	= 0x8002,
	USER_GET_PARAM	= 0x8003,
	USER_DATA_RDY	= 0x8004,
	USER_MSG_LAST	= 0x8006,
	USER_DIRECT_DATA = 0x8007
} mae_user_msg_t;


// Every system driver must have ID to be registered with the Audio Engine
typedef enum {
	DRV_NULL		= 0x00,	// We reserve 0x10 IDs for Chip ID

	DRV_INP0		= 0x20,	// Reserve 0x20 (32) IDs for Input channels

	DRV_OUT0		= 0x40,	// Reserve 0x20 (32) IDs for Output channels

	DRV_GPIO		= 0x80,	// Driver 0x80 will be responsible for GPIO 
	DRV_TIMER		= 0x81,	// Driver 0x81 will be responsible for 10 ms Timer Ticks 
	DRV_SYNC		= 0x82	// Driver 0x82 is the syncronization driver
} mae_drivers_t;	

typedef struct mae_parameter {
	uint32_t value:24;
	uint32_t number:8;
} mae_parameter_t;

#endif  /*__MESSAGES_H */
