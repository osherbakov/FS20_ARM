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
#ifndef __DRIVERS_H
#define __DRIVERS_H

#include "mae.h"

// Include the drivers/modules that will be a part of the MAE system in IROM 


//*****************************************************************************
// Driver and Module internal structures. Used only within MAXIM Audio Engine
typedef struct mae_driver_entry
{
	uint32_t			drv_number;
	DRIVER_PROCESS			*p_process;
	void 				*p_state;
	mae_queues_t			drv_queues;
	struct mae_driver_entry 	*p_next;
} mae_driver_entry_t;

// The starting pointer to the linked list of the MAE-registered drivers
extern mae_driver_entry_t *gp_mae_drivers;
// The pointer to the main system messaging queue 
extern  mae_msg_queue_t *gp_mae_msg_queue;
// The pointer to the free memory currently available (malloc will move it)
extern void *gp_free_memory;
// Globally visible MAE function table
extern mae_functions_table_t g_functions_table;
// The pointer to the mae function table, so all the function calls may be patched..
extern mae_functions_table_t *gp_functions_table;  

// The internal functions for the drivers/modules support
extern mae_driver_entry_t *mae_find_driver(unsigned int driver_num);
extern DRIVER_PROCESS *mae_get_driver_function(unsigned int driver_num);
extern int mae_count_msg_queue();
extern void mae_get_message(uint16_t *msg, uint16_t *param1, uint32_t *param2);
extern void mae_broadcast_unknown_message(uint16_t msg, uint16_t param1, uint32_t param2);


//*****************************************************************************
// Audio Engine functions - real implementation in the Maxim Audio Engine.
extern GET_QUEUE_SIZE          	mae_get_queue_size;
extern GET_QUEUE_COUNT			mae_get_queue_count;
extern GET_QUEUE_SPACE			mae_get_queue_space;
extern CLEAR_QUEUE 				mae_clear_queue;
extern FILL_QUEUE 				mae_fill_queue;
extern POP_QUEUE 				mae_pop_queue;
extern PUSH_QUEUE				mae_push_queue;
extern POP_QUEUE_DATA			mae_pop_queue_data;
extern PUSH_QUEUE_DATA			mae_push_queue_data;

extern REGISTER_DRIVER 			mae_register_driver;
extern REGISTER_QUEUE			mae_register_queue;
extern REGISTER_BUS				mae_register_bus;

extern UNREGISTER_DRIVER 		mae_unregister_driver;

extern MALLOC					mae_malloc;
extern GET_FREE_MEM_POINTER 	mae_get_free_mem_pointer;
extern GET_FREE_MEM_SIZE 		mae_get_free_mem_size;

extern COUNT_INPUT_QUEUES 		mae_count_input_queues;
extern COUNT_OUTPUT_QUEUES 		mae_count_output_queues;
extern GET_INPUT_QUEUES 		mae_get_input_queues;
extern GET_OUTPUT_QUEUES 		mae_get_output_queues;
extern GET_INPUT_QUEUE			mae_get_input_queue;
extern GET_OUTPUT_QUEUE			mae_get_output_queue;

extern GET_DRIVER_STATE 		mae_get_driver_state;

extern POST_MESSAGE				mae_post_message;
extern PROCESS_MESSAGE			mae_process_message;
extern PROCESS_DATA				mae_process_data;

extern AUDIO_ENGINE_INIT		mae_audio_engine_init;		
extern AUDIO_ENGINE_START		mae_audio_engine_start;
extern AUDIO_ENGINE_CONFIG		mae_audio_engine_config;
extern AUDIO_ENGINE_STOP		mae_audio_engine_stop;
extern AUDIO_ENGINE_CLOSE		mae_audio_engine_close;

extern CREATE_QUEUE				mae_create_queue;
extern ADD_INPUT_QUEUE			mae_add_input_queue;
extern ADD_OUTPUT_QUEUE			mae_add_output_queue;

extern REGISTER_CONNECTION 		mae_register_connection;
extern REGISTER_BLOCK 			mae_register_block;

#endif  /*__DRIVERS_H */
