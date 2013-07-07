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
#ifndef __STREAMS_H
#define __STREAMS_H

#include <stdint.h>

//-----------------------------------------------------------------------------
// Circular buffer definition structure - at some point it will be directly mapped
//  into the single TIE access instruction
// The same buffer structure can be passed to specially designed TIE instructions:
//		Put - will update i_put and check for wraparound
//		Get - will update i_get and check for wraparound
// 	n_shift will also specify the size of data fetch shift - 
//		0 - 1 byte 	(8 bits)
//		1 - 2 bytes (16 bits)
//		2 - 4 bytes (32 bits)
typedef struct mae_buffer_control 
{
	uint16_t 		i_put;			//! Put index
	uint16_t 		i_get;			//! Get index
	uint16_t 		n_size;			//! Buffer size (wrap-around index value)
	uint16_t 		n_shift;		//! The shift - amount of shift in address calculation 
} mae_buffer_control_t;

//-----------------------------------------------------------------------------
// Queue basic structure  
//  Defines queue has a link field to support writes into the multiple queues 
typedef struct mae_queue
{
	void 					*p_base;		//! The pointer to the start of the buffer
	mae_buffer_control_t	c_queue;		//! Queue parameters and indices
	struct mae_queue 		*p_next;		//! Queue link in case of shared queues
} mae_queue_t;


//-----------------------------------------------------------------------------
// Queue definition structure  
//  Keeps additional information that will be populated when 2 streams are 
//  connected together. Before such connection is made, we need to keep 
//  all streams related data available to veryfy that they are properly 
//  connected.
typedef struct mae_queue_entry
{
	uint16_t 				n_size;			//! Buffer size (wrap-around index value)
	uint16_t 				n_shift;		//! The shift 0 - byte, 1 - 16-bit, 2 - 32-bit
	uint32_t 				bus_name;		//! audio bus ID 
	uint32_t 				pin_name;		//! audio pin on the bus
	struct mae_queue		*p_queue;		//! Link to the actual queue that
											//!   connects two streams
	struct mae_queue_entry	*p_next;		//! The next entry in linked list
} mae_queue_entry_t;


//-----------------------------------------------------------------------------
// Queues structure  
// Every driver and module may have any number of Input and Output queues.
// This structure provides the starting pointers to such linked lists.
// The pointer to that structure is provided to every Driver and Module
//  when MSG_START message is bradcast.
// The purpose is to inform every Module and Driver about the queues that
//  are got attached after all initialization and configuration is done.
typedef struct mae_queues
{
	mae_queue_entry_t			*p_output_queue;
	mae_queue_entry_t			*p_input_queue;
} mae_queues_t;


//-----------------------------------------------------------------------------
// Buffer basic structure - the same as for the queue  
//  Defines queue has a link field to support writes into the multiple queues
// Technically, the linear buffer is no different from the queue
typedef struct mae_buffer 
{
	void 					*p_base;
	mae_buffer_control_t	c_buffer;
	struct mae_buffer 		*p_next;
} mae_buffer_t;


//! Message Queue basic structure  
typedef struct mae_msg_queue
{
	void 						*p_base;		//! The pointer to the start of the buffer
	mae_buffer_control_t		c_queue;		//! Queue parameters and indices
	struct mae_queue 			*p_next;		//! Queue link in case of shared queues
} mae_msg_queue_t;

#endif  /*__STREAMS_H */
