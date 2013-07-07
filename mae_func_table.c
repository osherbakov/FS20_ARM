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
#include "mae.h"
#include "config.h"
#include "drivers.h"

mae_functions_table_t g_functions_table =
{
        mae_register_driver,
        mae_register_block,
        mae_register_connection,
        mae_register_bus,
        mae_register_queue,
        mae_unregister_driver,

        mae_get_driver_state,

        mae_create_queue,
        mae_add_input_queue,
        mae_add_output_queue,

        mae_count_input_queues,
        mae_count_output_queues,
        mae_get_input_queues,
        mae_get_output_queues,
        mae_get_input_queue,
        mae_get_output_queue,
        mae_get_queue_size,
        mae_get_queue_count,
        mae_get_queue_space,
        
        mae_clear_queue,
        mae_fill_queue,
        mae_pop_queue,
        mae_push_queue,
        mae_pop_queue_data,
        mae_push_queue_data,
        
        mae_post_message,
        mae_process_message,
	mae_process_data,
		
        mae_malloc,
        mae_get_free_mem_pointer,
        mae_get_free_mem_size,
        
        mae_audio_engine_init,
        mae_audio_engine_start,
        mae_audio_engine_config,
	mae_audio_engine_stop,
	mae_audio_engine_close
};

// The pointer to the mae function table, so all the function calls may be patched..
mae_functions_table_t *gp_functions_table = &g_functions_table;  


