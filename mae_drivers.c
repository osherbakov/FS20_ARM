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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// The starting pointer to the linked list of the MAE-registered drivers
mae_driver_entry_t *gp_mae_drivers = 0;

//
//-----------------------------------------------------------------------------
//  Driver descriptor tables for the drivers that go into the IROM
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  					All IROM drivers table
//-----------------------------------------------------------------------------
driver_descriptor_t *(g_mae_drivers[]) = 
{
//	&I2C_driver_descriptor,
//	&timer_descriptor,
//	&limiter_descriptor,
//	&ax48_reg_driver_descriptor,
//	&audio_out_driver_descriptor,
//	&g_audio_in_driver_descriptor,
//	&middleman_descriptor,
//	&sync_descriptor,
	0									// terminate the array with a zero entry
};

//
//-----------------------------------------------------------------------------
//  End of driver descriptor tables for the drivers that go into the IROM
//-----------------------------------------------------------------------------



// n_alignment  = 0 - 1-byte alignment
//				= 1 - 2-byte (16-bit)
//				= 2 - 4-byte (32-bit)
//				= 3 - 8-byte (64-bit)
//				= 4 - 16-byte (128-bit)
void *mae_malloc (int n_size, int n_alignment)
{
	unsigned int n_mem_start = (unsigned int) gp_free_memory;
if(n_mem_start)
{
	unsigned int m_mem_next;
	unsigned int *p_zero; 
	unsigned int mask = (1 << n_alignment) - 1;
	n_size = (n_size + mask) & ~ mask;
	n_mem_start = (n_mem_start + mask) & ~mask;
	m_mem_next = n_mem_start + n_size;
	gp_free_memory = (void *) m_mem_next;
	
	// Now zero out the memory
	p_zero = (unsigned int *)n_mem_start;
	while(((unsigned int )p_zero) < m_mem_next)
	{
		*p_zero++ = 0;
	}
}else
{
  n_mem_start = (unsigned int) malloc(n_size);
  memset((void *)n_mem_start, 0, n_size);
}

	return (void *) n_mem_start;
}


void *mae_get_free_mem_pointer()
{
	return gp_free_memory;
}

int mae_get_free_mem_size()
{
	return ((int)get_stack_pointer() - (int) gp_free_memory - 96);
}

int mae_register_queue(mae_queue_entry_t *src_queue, mae_queue_entry_t *dest_queue, int min_buffer_size)
{
	// Adjust the queue size - we do not currently support 24-bit wide queues
	unsigned int  	n_size_in_bytes;
	int				n_shift_amount;
	mae_queue_t *p_q;
	
	// First some sanity checks
	if( (0 == src_queue) ||				// Check for NULL pointers 
		(0 == dest_queue) || 
		(dest_queue->p_queue != 0) ||	// Can have only one queue in destination  
		(dest_queue->n_shift != src_queue->n_shift)
		)
	{
		// Did not pass checks - return error
		return -1;
	}

	n_shift_amount = src_queue->n_shift;
	// Set up the queue size to the MAX of both sizes and specified minimum
	// The corrected algorithm:
	//  size = 2 * max( a, b )
	if(min_buffer_size == 0)
	{
		unsigned int size_s = src_queue->n_size;
		unsigned int size_d = dest_queue->n_size;
		
		//1.
		//min_buffer_size = MAX(MIN(size_s, size_d) << 1, MAX(size_s, size_d));

		// module with queue sizes 40->16 conection allocates 40 samples area  
		// if module size == queue size, cause no audio in flow graph.
		// Fix for flowgraph test : test_flowgraph_40_16_33_67 

		//2.
		//min_buffer_size = ((min_buffer_size == size_s) || (min_buffer_size == size_d))? min_buffer_size << 1 :min_buffer_size;
		

		// For ROM optimization : replace 1. and 2.  with double the maximum queue size for all test cases at the expense of some RAM area.
		// all flow graph tests passed 
		// Fix for flowgraph test: test_gain_var_queue_config1 

		//3.
		min_buffer_size = MAX(size_s, size_d) << 1;
	}
	
	n_size_in_bytes =  min_buffer_size << n_shift_amount;
	
	// First, create a queue control structure 
	p_q = gp_functions_table->p_malloc(sizeof(mae_queue_t), 2);

	// Fill the elements
	p_q->c_queue.n_size = min_buffer_size; 
	p_q->c_queue.n_shift = n_shift_amount;
	
	// Assign the same freshly created queue to both input and output entries
	//
	// From MAE point of view the Driver/App feeds
	// the data to another Driver/App, so if the Driver/App makes it's data visible
	// to the MAE - it is a Source, if the Driver/App receives the data from MAE-managed queue -
	//  it is a Destination.  
	//
	dest_queue->p_queue = p_q;	// Assign to the queue 
	if(0 == src_queue->p_queue)	 
	{
		// This is a first output stream - link it directly
		src_queue->p_queue = p_q;
		// Allocate the buffer for the actual queue storage
		// Fix for the [ADFS-2] bug - the buffers for 24/32-bit queues must be 
		//  aligned on 64-bit boundaries. 
		p_q->p_base = gp_functions_table->p_malloc(n_size_in_bytes, n_shift_amount + 1);
	}else
	{
		// This is a shared queue - add reference to the previously registered
		mae_queue_t **pp_q = &(src_queue->p_queue);
		while ( 0 != (*pp_q)->p_next )
		{
			pp_q = &(*pp_q)->p_next;
		}
		(*pp_q)->p_next = p_q;
		// Assign the already allocated buffer to the queue.
		p_q->p_base = src_queue->p_queue->p_base;
	}
	return 0;
}


int mae_register_driver (int driver_num, driver_descriptor_t *p_d)
{
	unsigned int 			stream_idx;
	mae_driver_entry_t		*p_new_driver;
	DRIVER_PROCESS			*p_process;
	mae_stream_descriptor_t	*p_streams;
	unsigned int			num_streams;
	mae_driver_entry_t		**pp_drv;
	mae_queue_entry_t		**pp_iq;
	mae_queue_entry_t		**pp_oq;
	int						msg, param1, param2;
	
	// Verify that the parameter passed is OK
	if(!p_d) return -1;
	if( driver_num == DRV_NULL) return -1;  

	// Check if the driver was already registerd - if yes, return SUCCESS
	if(0 != mae_find_driver(driver_num) ) return 0;
	
	p_process = p_d->p_process;
	p_streams = p_d->p_streams;
	num_streams = p_d->n_streams;
	//------------------------------------------------------------------------
	//  SPECIAL CASE - if the pointer to the processing function is NULL,
	//  then we are asked to use the standard driver residing in IROM
	if(0 == p_process)
	{
		// Check if the driver requested is the one from IROM
		//  To do this, walk thru the array of all registered drivers
		driver_descriptor_t **pp_irom_driver = &(g_mae_drivers[0]);
		while(*pp_irom_driver)
		{
			if((*pp_irom_driver)->unique_id == driver_num)
			{
				p_d = *pp_irom_driver; 
				p_process = (*pp_irom_driver)->p_process;
				// Check if the driver has streams description table
				if(0 == p_streams)
				{
					num_streams = (*pp_irom_driver)->n_streams;
					p_streams = (*pp_irom_driver)->p_streams;
				}
				break;	// Entry found - no need to loop anymore
			}
			pp_irom_driver++;
		}
	}

	// No valid driver process function was specified - exit with error code
	if( 0 == p_process ) return -1;
	
	// Find the last driver in the list
	pp_drv = &gp_mae_drivers;

	// Create the new driver in the drivers table
	p_new_driver = (mae_driver_entry_t *) gp_functions_table->p_malloc(sizeof(mae_driver_entry_t), 2);
	// Save the pointer to that driver entry into the head or the p_next field
	if(*pp_drv == 0)
	{
	    *pp_drv = p_new_driver;
	}else
	{
		// The last driver in the chain will have the NULL pointer in the p_next field
		while ( (*pp_drv)->p_next != 0 )
		{
			pp_drv = &((*pp_drv)->p_next);
		}
	    (*pp_drv)->p_next = p_new_driver;
	}	
	// Copy all possible information from the descriptor into the driver entry
	p_new_driver->drv_number = driver_num;
	p_new_driver->p_process = p_process;
	// Allocate and store the driver-requested amount of local storage
	p_new_driver->p_state = gp_functions_table->p_malloc(p_d->n_data_bytes, 2);
	
	// Initialize the pointers to store the queues information
	pp_iq = &(p_new_driver->drv_queues.p_input_queue);
	pp_oq = &(p_new_driver->drv_queues.p_output_queue);

	for(stream_idx = 0; stream_idx < num_streams; stream_idx++)
	{
		// Get the current stream descriptor
		mae_stream_descriptor_t *p_s = &(p_streams[stream_idx]);
		// Allocate the memory for the queue table entry
		mae_queue_entry_t *p_qe = (mae_queue_entry_t *) gp_functions_table->p_malloc(sizeof(mae_queue_entry_t), 2);
		
		// Copy all relative fields from the desriptor into the queue entry
		p_qe->pin_name = p_s->pin_name;				
		p_qe->bus_name = p_s->bus_name;				
		p_qe->n_size = p_s->n_size;		
		p_qe->n_shift = p_s->n_shift;
		
		// Attach freshly created queue entry to the appropriate linked list
		if(p_s->direction == OUTPUT_STREAM)
		{
			*pp_oq = p_qe;
			pp_oq = &(p_qe->p_next);
		}else
		{
			*pp_iq = p_qe; 
			pp_iq = &(p_qe->p_next);
		}
	}
	
	//Send the INIT message to the driver/module .
	p_new_driver->p_process(p_new_driver->p_state, MSG_INIT, 
					p_new_driver->drv_number, (int) gp_functions_table);

	// Now go thru the message queue and if there are any messages in it - get them and 
	// send to all registered drivers and modules.
	while (mae_count_msg_queue())
	{
		mae_get_message(&msg, &param1, &param2);
		mae_broadcast_unknown_message(msg, param1, param2);
	}	
					
	return 0;
}

int mae_unregister_driver(int driver_num)
{
	mae_driver_entry_t **pp_drv = &gp_mae_drivers;
	
	if( (*pp_drv) == 0 ) return -1;

	if( driver_num == DRV_NULL)
	{
		// Special case - clear all drivers/modules
		*pp_drv = 0;
	}
	else if( (*pp_drv)->drv_number == driver_num )
	{
		// The very first driver/module the required one -
		//  use it's next field as the head of the linked list
		*pp_drv = (*pp_drv)->p_next;
	}else
	{
		// Check if the driver pointed by the current is the
		//  required one. If yes - replace the current p_next pointer
		//  with the p_next from the driver to be removed.
		// As the result whoever was pointing to it - does not
		// point to it anymore, but to the next one.
		while( (*pp_drv)->p_next)
		{
			if((*pp_drv)->p_next->drv_number == driver_num)
			{
				(*pp_drv)->p_next = (*pp_drv)->p_next->p_next;
				break;
			}else
			{
				pp_drv = &((*pp_drv)->p_next);
			}
		}
	}
	
	return 0;
}


int mae_register_connection(int n_src_num, int n_src_queue, int n_dest_num, int n_dest_queue, int min_buffer)
{
	int	counter;
	mae_driver_entry_t *drv_src = mae_find_driver(n_src_num);
	mae_driver_entry_t *drv_dst = mae_find_driver(n_dest_num);
	
	mae_queue_entry_t *p_qs = gp_functions_table->p_get_output_queues(n_src_num);
	mae_queue_entry_t *p_qd = gp_functions_table->p_get_input_queues(n_dest_num);

	if((drv_src == 0) || (drv_dst == 0)) return -1;

	// Find the source queue
	counter = 0;
	while(p_qs)
	{
		if( (n_src_queue == counter) || (n_src_queue == p_qs->pin_name)) 
		{
			break;
		}
		p_qs = p_qs->p_next;
		counter++;
	}
	
	// Find the destination queue
	counter = 0;
	while(p_qd)
	{
		if( (n_dest_queue == counter) || (n_dest_queue == p_qd->pin_name)) 
		{
			break;
		}
		p_qd = p_qd->p_next;
		counter++;
	}

	if((p_qs == 0) || (p_qd == 0)) return -1;
	
	return gp_functions_table->p_register_queue(p_qs, p_qd, min_buffer);
}


int mae_register_bus(int n_src_num, int src_bus, int n_dest_num, int dest_bus, int min_buffer)
{
	mae_driver_entry_t *drv_src = mae_find_driver(n_src_num);
	mae_driver_entry_t *drv_dst = mae_find_driver(n_dest_num);
	mae_queue_entry_t *p_qs = gp_functions_table->p_get_output_queues(n_src_num);
	mae_queue_entry_t *p_qd = gp_functions_table->p_get_input_queues(n_dest_num);

	if((drv_src == 0) || (drv_dst == 0)) return -1;

	// Here is the logic for the bus registration:
	//	
	//  Go thru every entry in the source queue and check for the bus name
	//   if there is a pin that belongs to the specified source bus,
	//   then go thru all destination pins and find matching pin 
	while(p_qs)
	{
		// Is this Source pin from the src_bus ? 
		if( (src_bus == p_qs->bus_name)) 
		{
			// Go thru all destination pins looking for matching pin
			mae_queue_entry_t *p_t = p_qd;
			while(p_t)
			{
				// Does the destination pin belong to the dest_bus 
				//		and has the same name as the Source pin? Then we have a match 
				if ((dest_bus == p_t->bus_name) && (p_qs->pin_name == p_t->pin_name))
				{
					if( gp_functions_table->p_register_queue(p_qs, p_t, min_buffer) != 0) return -1;
				}
				p_t = p_t->p_next;
			}
		}
		p_qs = p_qs->p_next;
	}
	return 0;
}

mae_queue_entry_t * mae_create_queue(mae_stream_descriptor_t *p_descriptor)
{
	mae_queue_entry_t *p_qe = 0;
	if(p_descriptor != 0)
	{
		// Allocate the memory for the queue table entry
		p_qe = (mae_queue_entry_t *) gp_functions_table->p_malloc(sizeof(mae_queue_entry_t), 2);
	
		// Copy all relative fields from the desriptor into the queue entry
		p_qe->pin_name = p_descriptor->pin_name;				
		p_qe->bus_name = p_descriptor->bus_name;				
		p_qe->n_size = p_descriptor->n_size;		
		p_qe->n_shift = p_descriptor->n_shift;
	}
	return p_qe;
}

int mae_add_input_queue(int driver_num, mae_queue_entry_t *queue)
{
	mae_queue_entry_t **pp_iq;
	mae_driver_entry_t	*p_driver = mae_find_driver(driver_num);
	
	if( (p_driver == 0) || (queue == 0)) return -1;
	
	pp_iq = &(p_driver->drv_queues.p_input_queue);
	if(*pp_iq == 0)
	{
	    *pp_iq = queue;
	}
	else
	{
		// The last driver in the chain will have the NULL pointer in the p_next field
		while ( (*pp_iq)->p_next != 0 )
		{
			pp_iq = &((*pp_iq)->p_next);
		}
	    (*pp_iq)->p_next = queue;
	}	

	return 0;
}

int mae_add_output_queue(int driver_num, mae_queue_entry_t *queue)
{
	mae_queue_entry_t **pp_oq;
	mae_driver_entry_t	*p_driver = mae_find_driver(driver_num);
	
	if( (p_driver == 0) || (queue == 0)) return -1;
	
	pp_oq = &(p_driver->drv_queues.p_output_queue);
	if(*pp_oq == 0)
	{
	    *pp_oq = queue;
	}
	else
	{
		// The last driver in the chain will have the NULL pointer in the p_next field
		while ( (*pp_oq)->p_next != 0 )
		{
			pp_oq = &((*pp_oq)->p_next);
		}
	    (*pp_oq)->p_next = queue;
	}	

	return 0;
}



mae_driver_entry_t *mae_find_driver(int driver_num)
{
	mae_driver_entry_t	*p_drv = gp_mae_drivers;
	while (p_drv)
	{
		if(p_drv->drv_number == driver_num)
		{
			break;
		}else
		{
			p_drv = p_drv->p_next;
		}
	}
	return p_drv;
}


void *mae_get_driver_state(int driver_num)
{
	void *result = 0;
	// First - find the driver based on the driver's ID
	mae_driver_entry_t	*p_drv = mae_find_driver(driver_num);
	if(p_drv)
	{
		result = p_drv->p_state;
	}
	return result;
}

DRIVER_PROCESS *mae_get_driver_function(int driver_num)
{
	DRIVER_PROCESS *result = 0;
	// First - find the driver based on the driver's ID
	mae_driver_entry_t	*p_drv = mae_find_driver(driver_num);
	if(p_drv)
	{
		result = p_drv->p_process;
	}else
	{
		// No driver was found among registered - look into IROM drivers/modules
		driver_descriptor_t **pp_irom_driver = &(g_mae_drivers[0]);
		while(*pp_irom_driver)
		{
			if((*pp_irom_driver)->unique_id == driver_num)
			{
				result = (*pp_irom_driver)->p_process;
				break;	// Entry found - no need to loop anymore
			}
			pp_irom_driver++;
		}
	}
	return result;
}


int mae_count_input_queues(int driver_num)
{
	int q_count = -1;
	
	// First - find the driver based on the driver's ID
	mae_driver_entry_t	*p_drv = mae_find_driver(driver_num);
	if(p_drv)
	{
		mae_queue_entry_t *p_qe = p_drv->drv_queues.p_input_queue;
		q_count = 0;
		while(p_qe)
		{
			p_qe = p_qe->p_next;
			q_count++;
		}
	}
	return q_count;
}

int mae_count_output_queues(int driver_num)
{
	int q_count = -1;
	
	// First - find the driver based on the driver's ID
	mae_driver_entry_t	*p_drv = mae_find_driver(driver_num);
	if(p_drv)
	{
		mae_queue_entry_t *p_qe = p_drv->drv_queues.p_output_queue;
		q_count = 0;
		while(p_qe)
		{
			p_qe = p_qe->p_next;
			q_count++;
		}
	}
	return q_count;
}

mae_queue_entry_t *mae_get_input_queues(int driver_num)
{
	mae_queue_entry_t *result = (mae_queue_entry_t *) 0;
	
	// First - find the driver based on the driver's ID
	mae_driver_entry_t	*p_drv = mae_find_driver(driver_num);
	// Return the associated queue
	if(p_drv)
	{
		result = p_drv->drv_queues.p_input_queue;
	}
	return result;
}

mae_queue_entry_t   *mae_get_output_queues(int driver_num)
{
	mae_queue_entry_t *result = (mae_queue_entry_t *) 0;
	
	// First - find the driver based on the driver's ID
	mae_driver_entry_t	*p_drv = mae_find_driver(driver_num);
	// Return the associated queue
	if(p_drv)
	{
		result = p_drv->drv_queues.p_output_queue;
	}
	return result;
}

mae_queue_t *mae_get_input_queue(int driver_num, int queue_num)
{
	mae_queue_t *result = (mae_queue_t *) 0;
	
	// First - get the appropriate queue list for that driver ID
	mae_queue_entry_t *p_qe = gp_functions_table->p_get_input_queues(driver_num);
	int	counter = 0;
	while(p_qe)
	{
		if( (queue_num == counter) || (queue_num == p_qe->pin_name)) 
		{
			result = p_qe->p_queue;
			break;
		}
		p_qe = p_qe->p_next;
		counter++;
	}
	return result;
}

mae_queue_t *mae_get_output_queue(int driver_num, int queue_num)
{
	mae_queue_t *result = (mae_queue_t *) 0;
	
	// First - get the appropriate queue list for that driver ID
	mae_queue_entry_t *p_qe = gp_functions_table->p_get_output_queues(driver_num);
	int	counter = 0;
	while(p_qe)
	{
		if( (queue_num == counter) || (queue_num == p_qe->pin_name)) 
		{
			result = p_qe->p_queue;
			break;
		}
		p_qe = p_qe->p_next;
		counter++;
	}
	return result;
}

int mae_register_block (block_descriptor_t *p_d)
{
	return 0;
}

void mae_process_data(int block_id)
{
}


