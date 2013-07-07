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
#include "mxtos.h"
#include "config.h"
#include "simple_gain_mod.h"

// The structure that keeps all parameters for the module/driver
//! \breif holder for all internal data preserved between calls
typedef struct simple_gain_mod_state {
	//------------------Module ID section----------------
	// The module may have multiple instances with multiple IDs.
	// The modules share the same code, but have different states.
	// Keep the current Module ID in the state. 
	int32_t				module_id;			// The ID of the driver/module

	//-------------------Functions section-------------------
	// The MAE functions that will be used by the module/driver go here
	mae_functions_table_t *p_ft;
	
	//-------------------Queues section-------------------
	// If the module has any queues attached - keep it there 
	mae_queue_t			*input_queue;			// Storage for the input queue pointer
	mae_queue_t			*output_queue;			// Storage for the output queue pointer
	int16_t				buffer[SIMPLE_GAIN_MOD_BLOCK_SIZE];

	//-------------------Parameters section-------------------
	// All module parameters will go there
	int32_t				enabled;			// Current enable processing flag
	int32_t				volume;				// Current volume/gain
} simple_gain_mod_state_t;


//*****************************************************************************
// structures
//*****************************************************************************
mae_stream_descriptor_t simple_gain_mod_streams[] =  
{
	{INPUT_STREAM, SIZE_16BITS, SIMPLE_GAIN_MOD_BLOCK_SIZE, (uint32_t)'AUD','D'}, 
	{OUTPUT_STREAM, SIZE_16BITS, SIMPLE_GAIN_MOD_BLOCK_SIZE, (uint32_t)'AUD','D'}, 
};

driver_descriptor_t simple_gain_mod_descriptor = 
{
	SIMPLE_GAIN_MOD_ID,						//! This is a Module
	NUM_ELEMS(simple_gain_mod_streams),		//! It has so many queues
	sizeof(simple_gain_mod_state_t),		//! It needs so many bytes for the state
	simple_gain_mod_process,				//! Message processing function
	simple_gain_mod_streams			    	//! The streams/queues descriptor
};

//! simple_gain_mod_process
//!
//! This is a main message processing function for the module
//! It will be called every time an Audio Engine has any message in the queue
//! \return void
void 
simple_gain_mod_process(
			void *p_this,  //!< (i/o) explicit '*this' pointer
			uint16_t msg,       //!< (in) message to process
			uint16_t param1,    //!< (in) message dependent parameter
			uint32_t param2)    //!< (in) message dependent parameter 
{
	// Cast a generic "void *" pointer into the module state
	simple_gain_mod_state_t *p_state = (simple_gain_mod_state_t *) p_this;   
		
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
			
			// Initialize the gain to Unity  
			p_state->volume = (0x02L << 16) | 0x00008000L;	// Unity gain 

			// Initially enable processing.
			p_state->enabled = 1;
		}
		break;

	case MSG_START:	// This messasge is sent to all registerd modules to get the info 
					// about queues connected and to start the processing
		{
			// param2 for that message is the pointer to the structure
			// that has input and output queues for the driver/module
			mae_queues_t 		*p_queues = (mae_queues_t *) param2;
			p_state->input_queue = p_queues->p_input_queue->p_queue;
			p_state->output_queue = p_queues->p_output_queue->p_queue;
		}
		break;

	case MSG_DATA_RDY:
		// This message is sent to all modules/drivers when data is placed in queue 
		// Modules should check if they have to do anything.
		{
			int n_samples;
	
			// First, check if there is enough data in the input queues
			// and enough space in the output queue
			// n_samples = take minimum of input and output buffer sizes because of variable length
			// check min buffer is full then start processing
			n_samples = p_state->p_ft->p_get_queue_count(p_state->input_queue);
			n_samples = MIN(n_samples, p_state->p_ft->p_get_queue_space(p_state->output_queue));
			n_samples = MIN(SIMPLE_GAIN_MOD_BLOCK_SIZE, n_samples);
			if(n_samples)
			{
				//----------------------------------
				//  Before processing:
				// Ask MAE to fill the linear buffers
				p_state->p_ft->p_pop_queue_data(p_state->input_queue, &p_state->buffer[0], n_samples);
				
				//------------------------------------------------------------------------
				//  Now we have all we needed - the input and 
				//	output buffers of size BLOCK_SIZE - data processing will start here.
				//------------------------------------------------------------------------
				
				// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				// Here is the place where customer code that processes the data in the linear buffer will start
				// The input data are in the p_state->buffer
				// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				
				// In this particular case the bits of the volume 23-16 are exponent,
				//   bits 15-0 are mantissa
				// Do in-place gain application - if enabled
				if(p_state->enabled)
				{
					int j;
					uint32_t 	v_shift = (p_state->volume >> 16) & 0x00FF;
					uint32_t 	v_gain = (p_state->volume & 0x00FFFF);
					
					for( j = 0; j < n_samples; j++ )
					{
						 p_state->buffer[j] = ((p_state->buffer[j] * v_gain) >> (16 - v_shift));
					}
				}
				// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				// Here is the place where customer processing code will end
				// The processed data are in the p_state->buffer 
				// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

				//----------------------------------
				//  After processing:
				// Ask MAE to move data from the p_state->buffer into the output queue
				p_state->p_ft->p_push_queue_data(p_state->output_queue, &p_state->buffer[0], n_samples);

				// Inform the next module that the data is ready
				p_state->p_ft->p_post_message(MSG_DATA_RDY, 0, 0);
			}
		}
		break;
	
	case MSG_SET_PARAM:
		// This message is sent to this particular driver 
		// to change the value of certain parameter.
		if( p_state->module_id == param1 )
		{
			mae_parameter_t *p_param = (mae_parameter_t *) &param2;
			if(p_param->number == 0)		// Parameter #0 is usually Enabled flag
			{	// Save new volume
				p_state->enabled = p_param->value;
			}else if(p_param->number == 1)	// Parameter #1 is the Gain
			{	// Save new volume
				p_state->volume = p_param->value;
			}
		}
		break;
		
	case MSG_GET_PARAM:
		// This message is sent to this particular driver 
		// to get the specified value from it.
		{
			mae_parameter_t *p_param = (mae_parameter_t *) param2;
			if(p_param->number == 0)		// Parameter #0 is usually Enabled flag
			{
				p_param->value = p_state->enabled;
			}else if(p_param->number == 1)	// Parameter #1 is the Gain
			{
				p_param->value = p_state->volume;
			}
		}
		break;

    case MSG_GET_INFO:
		// This message is sent to this particular module 
		// to get the module descriptor.
		// The module fills the field with the pointer to that structure.
	    {
			driver_descriptor_t **pp_descriptor = (driver_descriptor_t **) param2;
			*pp_descriptor = &simple_gain_mod_descriptor;
	    }
		break;

    default:
		break;		
	}
	return;
}
