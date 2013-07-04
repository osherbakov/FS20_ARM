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
#include "drivers.h"

// Global saved interrupt level
static unsigned int g_saved_ps;

// The pointer to the main system messaging queue 
mae_msg_queue_t *gp_mae_msg_queue;

static int exit_processing = 0; 

// The stored original free memory and stack pointers
// Every time there is re-initialization we will reuse those
void *gp_original_free_memory = 0;
void *gp_original_stack = 0;

// The pointer to the free memory currently available (malloc will move it)
void *gp_free_memory = 0;


#ifdef WIN32
	CRITICAL_SECTION irq_level;
	HANDLE irq_active = INVALID_HANDLE_VALUE;
	CRITICAL_SECTION command_lock;
	HANDLE command_done = INVALID_HANDLE_VALUE;

#endif
	
#ifdef UNIX
	pthread_mutex_t irq_level;
	sem_t irq_active;
	pthread_mutex_t command_lock;
	sem_t command_done;
#endif



	

//! int mae_count_msg_queue(void)
//! \brief Routine to count the messages in the message queue.
//! Returns the number of messages in the message queue. Messages are 64 bits wide.
int mae_count_msg_queue(void)
{
	int count = 0;
	if (gp_mae_msg_queue)
	{
		unsigned int saved_ps = ints_off();
		unsigned int get_index = gp_mae_msg_queue->c_queue.i_get;
		unsigned int put_index = gp_mae_msg_queue->c_queue.i_put;
		unsigned int size = gp_mae_msg_queue->c_queue.n_size;
		count = put_index - get_index;
		if(count < 0)
		{
			count += size;
		}
		ints_restore(saved_ps);
	}
	return count;
}

//! \brief Routine to get message from the message queue.
void mae_get_message(int *msg, int *param1, int *param2)
{
	if (gp_mae_msg_queue)
	{
		unsigned int diff;
		unsigned int saved_ps = ints_off();
		int *p_buffer =  (int *) gp_mae_msg_queue->p_base;
		unsigned int get_index = gp_mae_msg_queue->c_queue.i_get;
		unsigned int size = gp_mae_msg_queue->c_queue.n_size;
		unsigned int idx = get_index << 1;	// The index into 64-bit message buffer

		int word = p_buffer[idx];
		*msg = (word >> 16) & 0xffff;
		*param1 = word & 0xffff;
		*param2 = p_buffer[idx + 1];

		get_index++;
		diff = get_index - size;
		gp_mae_msg_queue->c_queue.i_get = diff ? get_index : diff;

		ints_restore(saved_ps);
	}
}

//! void mae_process_message(int module_id, int msg, int param1, int param2)
//! \brief Routine to process messages in the message queue.
void mae_process_message(int module_id, int msg, int param1, int param2)
{
	//Get the pointer to driver entry struct which now gives access to all the 
	//required information.
	mae_driver_entry_t	*p_drv = mae_find_driver(module_id);
	if(p_drv)
	{
		p_drv->p_process(p_drv->p_state, msg, param1, param2);
	}
}
//! void mae_post_message (int msg, int param1, int param2)
//! \brief Routine to post message onto the message queue.
//! 
void mae_post_message (int msg, int param1, int param2)
{
	if (gp_mae_msg_queue)
	{
		unsigned int diff;
		int msg_found;
		unsigned int lookup_index;
		unsigned int idx;

		unsigned int saved_ps = ints_off();
		int *p_buffer = (int *) gp_mae_msg_queue->p_base;
		unsigned int put_index = gp_mae_msg_queue->c_queue.i_put;
		unsigned int get_index = gp_mae_msg_queue->c_queue.i_get;
		unsigned int size = gp_mae_msg_queue->c_queue.n_size;
		int word =  ((msg << 16) | param1);
		
		// Check if the message is already in the queue.
		// If it is there - do not add
		// Start with the Get index and work all the way to the Put index
		msg_found = 0;
		lookup_index = get_index;
		while(lookup_index != put_index )
		{
			idx = lookup_index << 1;
			if( (p_buffer[ idx ] == word) &&
				(p_buffer[ idx + 1 ] == param2) )
			{
				msg_found = 1;
				break;
			}
			lookup_index++;
			diff = lookup_index - size;
			lookup_index = diff ? lookup_index: diff;
		}

		if( !msg_found)
		{
			idx = put_index << 1;	// The index into 64-bit message buffer
			p_buffer[idx] = word;
			p_buffer[idx + 1] = param2;
			put_index++;
			diff = put_index - size;
			gp_mae_msg_queue->c_queue.i_put = diff ? put_index : diff;
		}
		ints_signal();
		ints_restore(saved_ps);
	}	
}
//!static void mae_broadcast_specific_message(int msg)
//! \brief This function sends a specific message to all the drivers and modules 
//! meaningit will require all drivers and modules to process that message. Eg of 
//! such a message could be a MSG_INIT or MSG_START.
static void mae_broadcast_specific_message(int msg)
{
	mae_driver_entry_t *p_mae_drivers = gp_mae_drivers;
	//First send the message to all drivers.
	while (p_mae_drivers)
	{
		p_mae_drivers->p_process(p_mae_drivers->p_state, msg, 
				p_mae_drivers->drv_number, 
				(int) &(p_mae_drivers->drv_queues));
		p_mae_drivers = p_mae_drivers->p_next;
	}
}

//! static void mae_broadcast_unknown_message(int msg, int param1, int param2)
//! \brief This function sends an unknown message to all the drivers and modules.
//! It is used when after reading msg, param1 and param2 using the get_message 
//! function need to be sent over to all modules and drivers. 
void mae_broadcast_unknown_message(int msg, int param1, int param2)
{
	mae_driver_entry_t *p_mae_drivers = gp_mae_drivers;
	//Send the message to drivers first and then modules
	while (p_mae_drivers)
	{
		p_mae_drivers->p_process(p_mae_drivers->p_state, msg, param1, param2);
		p_mae_drivers = p_mae_drivers->p_next;
	}
}

//! void mae_audio_engine_start()
//! \brief Audio Engine start routine.
//! This routine first checks for any messages present in the message queue and if
//! there are any, it sends them all to all drivers and modules. Then it sends START
//! message to all drivers and modules. Then it enters the while loop where it keeps 
//! checking the messages in the message queue and sends them to all drivers and modules.
void mae_audio_engine_start()
{
	int msg, param1, param2;
	
	// Disable all interrupts
    g_saved_ps = ints_off();

	// Initialize the stack pointer - we will never return from mae_audio_engine_start();
	if(gp_original_stack)	set_stack_pointer(gp_original_stack);

	// Now count the message queue and if there are any messages in it get them and 
	// send to all drivers and modules.
	while (mae_count_msg_queue())
	{
		mae_get_message(&msg, &param1, &param2);
		mae_broadcast_unknown_message(msg, param1, param2);
	}	
	
	//Send the START message to drivers first and then modules
	mae_broadcast_specific_message(MSG_START);
	
	//Turn on the interrupts at this point
	ints_on();
	
	//Enter the main while loop now
	exit_processing = 0;
	while (!exit_processing)
	{
		//If there is any messages in the queue, send them
		if (mae_count_msg_queue())
		{
			mae_get_message(&msg, &param1, &param2);
			mae_broadcast_unknown_message(msg, param1, param2);
		}else
		{	//Send the IDLE message...
			mae_broadcast_specific_message(MSG_IDLE);
		}
	}
}

void mae_audio_engine_stop()
{
	exit_processing = 1;	
}

void mae_audio_engine_init(void *end_bss, int depth_msg_queue)
{
	ints_init();	// Initialize the syncronization logic (OS-specific)
	// Disable all interrupts - data buffers will be invalid!!!!!
    g_saved_ps = ints_off();
	
	// Send everyone the MSG_STOP message to terminate nicely
	mae_broadcast_specific_message(MSG_STOP);

	//  If end_bss is not null - use it as the start of the heap and save it 
	//  If end_bss is null - use the previous saved value 
	if(end_bss)
	{
		gp_original_free_memory = end_bss; 	
	}else
	{
		end_bss = gp_original_free_memory;
	}
	
	// If there is a first call - save the stack pointer
	if(0 == gp_original_stack)
	{
		gp_original_stack = get_stack_pointer();
	}

	gp_free_memory = end_bss;
	gp_mae_drivers = 0;

	//Allocate message queue struct
	gp_mae_msg_queue = gp_functions_table->p_malloc(sizeof(mae_msg_queue_t), 2);
	//Allocate the actual message queue buffer. Width of messages is 64 bits (8 bytes).
	gp_mae_msg_queue->p_base =  gp_functions_table->p_malloc((depth_msg_queue << 3), 3);
	//i_put and i_get are zeroed out during malloc. Assign n_buffer_size
	gp_mae_msg_queue->c_queue.n_size = depth_msg_queue;
	gp_mae_msg_queue->c_queue.n_shift = 3;

	ints_restore(g_saved_ps);
}

void mae_audio_engine_close()
{
	ints_close();	
}
