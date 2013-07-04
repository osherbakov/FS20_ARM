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
#include "FlexSound20.h"
#include "mae.h"
#include "mxtos.h"


// static 	char __heap[64000];

extern MODULE_PROCESS FS_mod_process;
extern driver_descriptor_t FS_mod_descriptor;

extern thread_start_t FS_working_thread;

static mae_functions_table_t *p_ft;


int32_t			FS_init()
{
	p_ft = gp_functions_table;
	p_ft->p_audio_engine_init(0, 128);
	return 0;
}

void			*FS_malloc( 
	size_t		n_bytes, 
	uint32_t	alignment)
{
	return p_ft->p_malloc(n_bytes, alignment);
}

int32_t			FS_config( uint32_t	*p_Configuration )
{
	return p_ft->p_audio_engine_config((int *)p_Configuration);
}

int32_t			FS_start()
{
	start_command();
	// register the framework module as the last module
	p_ft->p_register_driver(0xFFFFFFFF, &FS_mod_descriptor);
	create_working_thread(FS_working_thread, NULL);
	wait_command_done();
	finish_command();
	return 0;
}

int32_t			FS_stop()
{
	start_command();
	p_ft->p_post_message(MSG_STOP, 0, 0);
	wait_command_done();
	finish_command();
	return 0;
}

int32_t			FS_close()
{
	p_ft->p_audio_engine_close();
	return 0;
}

// This function registers the module with the Flexsound Framework
int32_t			FS_add_module(uint32_t	module_id,
	void		*p_process_function)
{
	int	result = -1;
	driver_descriptor_t	*p_dd;

	if(p_process_function)
	{
		((MODULE_PROCESS *) p_process_function)(NULL, MSG_GET_INFO, module_id, (int)&p_dd);
		result = p_ft->p_register_driver(module_id, p_dd);
	}
	return result;
}

FS_queue_t* 	FS_input_queue(	uint32_t 	module_id,
	uint32_t 	queue_id,	uint32_t	queue_size)
{
	FS_queue_t* 	result;

	result = p_ft->p_get_input_queue(module_id, queue_id);

	if(result == NULL)
	{
		mae_queue_entry_t	*p_module_qe, *p_user_qe;
		mae_stream_descriptor_t user_sd;
		int	counter;
		// Get the information about the module queue we are connecting to
		p_module_qe = p_ft->p_get_input_queues(module_id);
		counter = 0;
		while(p_module_qe)
		{
			if( (queue_id == counter) || (queue_id == p_module_qe->pin_name)) 
			{
				break;
			}
			p_module_qe = p_module_qe->p_next;
			counter++;
		}
		if(p_module_qe == 0)
			return (FS_queue_t*) 0;

		// Create the stream that matches the input of the module
		user_sd.direction = OUTPUT_STREAM;
		user_sd.bus_name = 0x000000;
		user_sd.pin_name = 0x00;
		user_sd.n_size = p_module_qe->n_size;
		user_sd.n_shift = p_module_qe->n_shift;

		// Create the queue based on that stream
		p_user_qe = p_ft->p_create_queue(&user_sd);

		p_ft->p_register_queue(p_user_qe, p_module_qe, queue_size);
		result =  p_user_qe->p_queue;
	}
	return result;
}


FS_queue_t* 	FS_output_queue(	uint32_t 	module_id,
	uint32_t 	queue_id,	uint32_t	queue_size)
{
	FS_queue_t* 	result;

	result = p_ft->p_get_output_queue(module_id, queue_id);
	if(result == NULL)
	{
		mae_queue_entry_t	*p_module_qe, *p_user_qe;
		mae_stream_descriptor_t user_sd;
		int	counter;

		// Get the information about the module queue we are connecting to
		p_module_qe = p_ft->p_get_output_queues(module_id);
		counter = 0;
		while(p_module_qe)
		{
			if( (queue_id == counter) || (queue_id == p_module_qe->pin_name)) 
			{
				break;
			}
			p_module_qe = p_module_qe->p_next;
			counter++;
		}
		if(p_module_qe == 0)
			return (FS_queue_t*) 0;

		// Create the stream that matches the output of the module
		user_sd.direction = INPUT_STREAM;
		user_sd.bus_name = 0x000000;
		user_sd.pin_name = 0x00;
		user_sd.n_size = p_module_qe->n_size;
		user_sd.n_shift = p_module_qe->n_shift;

		// Create the queue based on that stream
		p_user_qe = p_ft->p_create_queue(&user_sd);

		p_ft->p_register_queue(p_module_qe, p_user_qe, queue_size);
		result =  p_user_qe->p_queue;
	}
	return result;
}


//
// Functions to get the attached queue information (constant)
//

// Returns the maximum number of elements that can be placed into the queue.
int32_t 	FS_queue_size(FS_queue_t	*p_queue)
{
	return p_ft->p_get_queue_size(p_queue);
}

// Returns the size of the elements (in bytes) that queue can handle. Valid values are 1, 2, and 4.
int32_t 	FS_queue_stride(FS_queue_t	*p_queue)
{
	uint32_t result;
	switch(((mae_queue_t *)p_queue)->c_queue.n_shift)
	{
		case SIZE_8BITS:
			result = 1;
			break;
		case SIZE_16BITS:
			result = 2;
			break;
		default:
			result = 4;
			break;
	}
	return result;
}


//
// Functions to get the current status of the queue
//
// Returns the number of elements in the queue that can to be placed by FS_queue_put_data(…) function
int32_t 	FS_queue_space ( FS_queue_t	*p_queue)
{
	return p_ft->p_get_queue_space(p_queue);
}

// Returns the number of elements that can be taken from the queue by calling FS_queue_get_data(…)
int32_t 	FS_queue_count(	FS_queue_t	*p_queue) 
{
	return p_ft->p_get_queue_count(p_queue);
}

//
// Functions to clear or fill the queue
//
// Clears the queue
int32_t		FS_queue_clear(	FS_queue_t	*p_queue) 
{
	p_ft->p_clear_queue(p_queue);
	return 0;
}

// Fill the queue with all zeroes
int32_t		FS_queue_fill( FS_queue_t	*p_queue)
{
	p_ft->p_fill_queue(p_queue);
	return 0;
}


//
// Functions to send and receive data/samples to the queues
//

// Place n_elements into the queue
int32_t		FS_queue_put_data(	FS_queue_t	*p_queue,
	void		*p_data,	uint32_t	n_elements )
{
	p_ft->p_push_queue_data(p_queue, p_data, n_elements);
	p_ft->p_post_message(MSG_DATA_RDY, 0, (int) p_queue);
	return 0;
}

/*
// Place n_elements into queue for particular module
extern int32_t          FS_queue_put_direct_data(
      FS_queue_t  *p_queue,
      void        *p_data,
      uint32_t    n_elements,
      uint32_t    module_id )
{
    p_ft->p_push_queue_data(p_queue, p_data, n_elements);
	start_command();
	p_ft->p_post_message(USER_DATA_RDY, module_id, (uint32_t)p_queue);
	wait_command_done();
	finish_command();
	return 0;
}
*/

// Get n_elements from the queue
int32_t		FS_queue_get_data(	FS_queue_t	*p_queue,
	void		*p_data,	uint32_t	n_elements )
{
	p_ft->p_pop_queue_data(p_queue, p_data, n_elements);
	p_ft->p_post_message(MSG_DATA_RDY, 0, (int) p_queue);
	return 0;
}

// Place n_ements into queue for particular module
extern int32_t		FS_queue_put_direct_data(
	FS_queue_t	*p_queue,
	void		*p_data,
	uint32_t	n_elements,
	uint32_t	module_id )
{
	p_ft->p_push_queue_data(p_queue, p_data, n_elements);
	start_command();
	p_ft->p_post_message(USER_DIRECT_DATA, module_id, 0);
	wait_command_done();
	finish_command();
	return 0;
}


//
// Functions to control the Processing Modules by providing Processing Module configuration and parameters
//
int32_t		FS_set_config(	uint32_t 	module_id,
	void		*p_data, 	uint32_t	n_bytes	)
{
	start_command();
	p_ft->p_post_message(USER_CONFIGURE, module_id, (int)&p_data);
	wait_command_done();
	finish_command();
	return 0;
}


int32_t		FS_set_param(	uint32_t 	module_id,
	uint8_t 	param_id, uint32_t	param_value)
{
	uint32_t	parameter = (((uint32_t)param_id) << 24 ) | 
							(param_value & 0x00FFFFFF) ;
	start_command();
	p_ft->p_post_message(USER_SET_PARAM, module_id, parameter);
	wait_command_done();
	finish_command();
	return 0;
}

int32_t		FS_get_param(	uint32_t 	module_id,
	uint8_t 	param_id)
{
	uint32_t	result;

	start_command();
	result = (((uint32_t)param_id) << 24 ) | 0x00FFFFFF ;

	p_ft->p_post_message(USER_GET_PARAM, module_id, (int)&result);
	wait_command_done();
	finish_command();
	return result & 0x00FFFFFF;
}

// The structure that keeps all parameters for the module/driver
//! \breif holder for all internal data preserved between calls
typedef struct FS_mod_state {
	//------------------Module ID section----------------
	// The module may have multiple instances with multiple IDs.
	// The modules share the same code, but have different states.
	// Keep the current Module ID in the state. 
	int32_t				module_id;			// The ID of the driver/module

	//-------------------Functions section-------------------
	// The MAE functions that will be used by the module/driver go here
	mae_functions_table_t *p_ft;
	
} FS_mod_state_t;


//*****************************************************************************
// structures
//*****************************************************************************
driver_descriptor_t FS_mod_descriptor = 
{
	0xFFFFFFFF,								//! This is a Module
	0,										//! It has so many queues
	sizeof(FS_mod_state_t),					//! It needs so many bytes for the state
	FS_mod_process,							//! Message processing function
	NULL				    				//! The streams/queues descriptor
};

//! FS_mod_process
//!
//! This is a main message processing function for the module
//! It will be called every time an Audio Engine has any message in the queue
//! \return void
void 
FS_mod_process(
			void *p_this,  //!< (i/o) explicit '*this' pointer
			int msg,       //!< (in) message to process
			int param1,    //!< (in) message dependent parameter
			int param2)    //!< (in) message dependent parameter 
{
	// Cast a generic "void *" pointer into the module state
	FS_mod_state_t *p_state = (FS_mod_state_t *) p_this;   
		
	switch(msg)
	{
	case MSG_INIT:	// On initialization message prepare everything -
					// assign itself an ID, get the functions from the table
					// or save the functions global table pointer
		{	
			// save your ID - you will be always known to the system under this name
			p_state->module_id = param1;
			// Param2 has the pointer to all system-provided functions table
			p_state->p_ft = (mae_functions_table_t *) param2;
		}
		break;

	case MSG_START:
		{
			signal_command_done();
		}
		break;

	case USER_SET_PARAM:
		// This message is sent to this particular driver 
		// to set the specified parameter.
		{
			p_state->p_ft->p_process_message(param1, MSG_SET_PARAM, param1, param2);
			signal_command_done();
		}
		break;

	case USER_GET_PARAM:
		// This message is sent to this particular driver 
		// to get the specified value from it.
		{
			p_state->p_ft->p_process_message(param1, MSG_GET_PARAM, param1, param2);
			signal_command_done();
		}
		break;

	case USER_CONFIGURE:
		// This message is sent to this particular driver 
		// to get the specified value from it.
		{
			p_state->p_ft->p_process_message(param1, MSG_CONFIGURE, param1, param2);
			signal_command_done();
		}
		break;

	case USER_DIRECT_DATA:
		// This message is sent only to this particular module to 
		// indicate that data is available.
		{
			p_state->p_ft->p_process_message(param1, MSG_DIRECT_DATA_RDY, 0, 0);
			signal_command_done();
		}
		break;
	case USER_DATA_RDY:
		// This message is sent to this particular driver 
		// to get the specified value from it.
		{
			p_state->p_ft->p_process_message(param1, MSG_DIRECT_DATA_RDY, param1, param2);
			signal_command_done();
		}
		break;

	case MSG_GET_INFO:
		// This message is sent to this particular driver 
		// to get the driver descriptor for the driver.
		// The driver fills the field with the pointer to that structure.
	    {
			driver_descriptor_t **pp_descriptor = (driver_descriptor_t **) param2;
			*pp_descriptor = &FS_mod_descriptor;
	    }
		break;

	case MSG_STOP:
		{
			p_state->p_ft->p_audio_engine_stop();
		}
		break;

	case MSG_IDLE:
		ints_wait();
		break;

    default:
		break;		
	}
	return;
}

unsigned int THREAD_TYPE FS_working_thread(void *p_thread_data)
{
	set_thread_priority();
	p_ft->p_audio_engine_start();
	signal_command_done();
	return 0;
}
