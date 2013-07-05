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
#ifndef __MAE_H
#define __MAE_H

#include <stdint.h>
#include "streams.h"
#include "messages.h"


//*****************************************************************************
// Processing functions - the declarations for the different functions that are 
// get called by the Maxim Audio Engine.
// Some of the have fixed number of parameters, some of them not.
// The common thing for all is that all of them take a state pointer as the first parameter.
typedef int BLOCK_PROCESS(void *, ... );
typedef void MODULE_PROCESS(void *, uint16_t Msg, uint16_t Param1, uint32_t Param2);
typedef void DRIVER_PROCESS(void *, uint16_t Msg, uint16_t Param1, uint32_t Param2);
typedef void ISR_PROCESS(void *);


// User-visible structures that define the Data-processing Modules,
//  incoming and outgoing Streams, Hardware-handling Drivers..

//*****************************************************************************
// Stream descriptor - the structure that defines the incoming or outgoing data 
enum {
	OUTPUT_STREAM	= 0,	// For that Module it sends data out of it
	INPUT_STREAM	= 1,	// For that Module it receives data from the outside world
};

enum {
	SIZE_8BITS		= 0,
	SIZE_16BITS		= 1,
	SIZE_24BITS		= 2,
	SIZE_32BITS		= 2
};


typedef struct mae_stream_descriptor 
{
	uint32_t 		direction;			//! 0 - output, 1 - input
	uint16_t 		n_shift;			//! data shift values ( 0, 1, 2)
	uint16_t 		n_size;				//! minimum frame size that may be processed
	uint32_t 		bus_name;			//! audio bus ID 
	uint32_t 		pin_name;			//! audio pin on the bus
} mae_stream_descriptor_t;

//-----------------------------------------------------------------------------

// Block descriptor - the structure that will fully describe ins- and outs- of particular Block  
typedef struct block_descriptor 
{
	uint16_t				unique_id;		// Block unique number
	uint16_t 				n_streams;		// Number of input/output buffer streams
	uint32_t 				n_data_bytes;	// Number of data bytes needed for state data
	BLOCK_PROCESS			*p_process;		// Main block data-processing function
	mae_stream_descriptor_t	*p_streams;		// Specifies the number and type of input/output streams
} block_descriptor_t;

// Module descriptor - the structure that will fully describe ins- and outs- of particular Module
typedef struct module_descriptor
{
	uint16_t				unique_id;		// Module unique number
	uint16_t				n_streams;		// Number of input/output streams
	uint32_t 				n_data_bytes;	// Number of data bytes needed for state data
	MODULE_PROCESS			*p_process;		// Main message processing function for the module
	mae_stream_descriptor_t	*p_streams;		// Specifies the number and type of input/output streams
} module_descriptor_t;

// Driver descriptor - the structure that will fully describe ins- and outs- of particular Driver  
typedef struct driver_descriptor 
{
	uint16_t				unique_id;		// Driver unique number
	uint16_t				n_streams;		// Number of input/output streams
	uint32_t 				n_data_bytes;	// Number of data bytes needed for state data
	DRIVER_PROCESS			*p_process;		// Main message processing function for the driver 
	mae_stream_descriptor_t	*p_streams;		// Specifies the number and type of input/output streams
} driver_descriptor_t;


//*****************************************************************************
// Audio Engine functions - the declarations for the different functions that are 
// defined in the Maxim Audio Engine.
// The purpose of this definition is to provide a strong casting of all function
// pointers in drivers and modules.

typedef int REGISTER_BLOCK (block_descriptor_t *p_d);
typedef int REGISTER_DRIVER (unsigned int driver_num, driver_descriptor_t *p_d);
typedef int REGISTER_CONNECTION (unsigned int n_src_num, unsigned int n_src_queue, unsigned int n_dest_num, unsigned int n_dest_queue, int min_buffer);
typedef int REGISTER_BUS (unsigned int n_src_num, unsigned int src_bus, unsigned int n_dest_num, unsigned int  dest_bus, int min_buffer);
typedef int REGISTER_QUEUE(mae_queue_entry_t *src_queue, mae_queue_entry_t *dest_queue, int min_buffer);

typedef int UNREGISTER_DRIVER (unsigned int driver_num);

typedef void *GET_DRIVER_STATE (unsigned int driver_num);

typedef mae_queue_entry_t *CREATE_QUEUE(mae_stream_descriptor_t *p_descriptor);

typedef int ADD_INPUT_QUEUE(unsigned int driver_num, mae_queue_entry_t *queue);
typedef int ADD_OUTPUT_QUEUE(unsigned int driver_num, mae_queue_entry_t *queue);


typedef int COUNT_INPUT_QUEUES(unsigned int driver_num);
typedef int COUNT_OUTPUT_QUEUES(unsigned int driver_num);

typedef mae_queue_entry_t *GET_INPUT_QUEUES(unsigned int driver_num);
typedef mae_queue_entry_t *GET_OUTPUT_QUEUES(unsigned int driver_num);

typedef mae_queue_t *GET_INPUT_QUEUE(unsigned int driver_num, unsigned int queue_num);
typedef mae_queue_t *GET_OUTPUT_QUEUE(unsigned int driver_num, unsigned int queue_num);

typedef int GET_QUEUE_SIZE(mae_queue_t *p_queue);
typedef int GET_QUEUE_COUNT(mae_queue_t *p_queue);
typedef int GET_QUEUE_SPACE(mae_queue_t *p_queue);

typedef void CLEAR_QUEUE(mae_queue_t *p_queue);
typedef void FILL_QUEUE(mae_queue_t *p_queue);

typedef uint32_t POP_QUEUE(mae_queue_t *p_queue);
typedef void PUSH_QUEUE(mae_queue_t *p_queue, uint32_t data);

typedef void POP_QUEUE_DATA(mae_queue_t *p_queue, void *p_data, int n_elements);
typedef void PUSH_QUEUE_DATA(mae_queue_t *p_queue, void *p_data, int n_elements);

typedef void POST_MESSAGE(unsigned int msg, unsigned int param1, uint32_t param2);
typedef void PROCESS_MESSAGE(unsigned int module_id, unsigned int msg, unsigned int param1, uint32_t param2);
typedef void PROCESS_DATA(unsigned int block_id);

typedef void *MALLOC(unsigned int n_size, unsigned int n_alignment);
typedef void *GET_FREE_MEM_POINTER();
typedef int GET_FREE_MEM_SIZE();

typedef void AUDIO_ENGINE_INIT(void *heap_start, unsigned int depth_msg_queue);
typedef void AUDIO_ENGINE_START();
typedef int AUDIO_ENGINE_CONFIG(uint32_t *p_config_data);
typedef void AUDIO_ENGINE_STOP();
typedef void AUDIO_ENGINE_CLOSE();

//*****************************************************************************
// Audio Engine functions table - keeps the pointers to all functions supplied 
// by the Audio Engine. This table is retrieved by making SYSCALL(2) call.
typedef struct mae_functions_table
{
	REGISTER_DRIVER 		*p_register_driver;
	REGISTER_BLOCK 			*p_register_block;
	REGISTER_CONNECTION 	*p_register_connection;
	REGISTER_BUS 			*p_register_bus;
	REGISTER_QUEUE			*p_register_queue;

	UNREGISTER_DRIVER 		*p_unregister_driver;

	GET_DRIVER_STATE 		*p_get_driver_state;

	CREATE_QUEUE 			*p_create_queue;

	ADD_INPUT_QUEUE			*p_add_input_queue;
	ADD_OUTPUT_QUEUE		*p_add_output_queue;
	
	COUNT_INPUT_QUEUES		*p_count_input_queues;
	COUNT_OUTPUT_QUEUES		*p_count_output_queues;

	GET_INPUT_QUEUES		*p_get_input_queues;
	GET_OUTPUT_QUEUES		*p_get_output_queues;

	GET_INPUT_QUEUE			*p_get_input_queue;
	GET_OUTPUT_QUEUE		*p_get_output_queue;

    GET_QUEUE_SIZE          *p_get_queue_size;
	GET_QUEUE_COUNT			*p_get_queue_count;
	GET_QUEUE_SPACE			*p_get_queue_space;

	CLEAR_QUEUE				*p_clear_queue;
	FILL_QUEUE				*p_fill_queue;

	POP_QUEUE				*p_pop_queue;
	PUSH_QUEUE				*p_push_queue;

	POP_QUEUE_DATA			*p_pop_queue_data;
	PUSH_QUEUE_DATA			*p_push_queue_data;

	POST_MESSAGE			*p_post_message;
	PROCESS_MESSAGE			*p_process_message;
	PROCESS_DATA			*p_process_data;

	MALLOC					*p_malloc;
	GET_FREE_MEM_POINTER	*p_get_free_mem_pointer;
	GET_FREE_MEM_SIZE		*p_get_free_mem_size;

	AUDIO_ENGINE_INIT		*p_audio_engine_init;
	AUDIO_ENGINE_START		*p_audio_engine_start;
	AUDIO_ENGINE_CONFIG		*p_audio_engine_config;
	AUDIO_ENGINE_STOP		*p_audio_engine_stop;
	AUDIO_ENGINE_CLOSE		*p_audio_engine_close;
}mae_functions_table_t;

extern mae_functions_table_t *gp_functions_table;
extern mae_functions_table_t g_functions_table;

#endif  /*__MAE_H */
