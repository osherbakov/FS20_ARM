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
#include <string.h>

int mae_get_queue_size(mae_queue_t *p_queue)
{
    return p_queue->c_queue.n_size;
}

int mae_get_queue_count(mae_queue_t *p_queue)
{
    int diff;
	unsigned int saved_ps = ints_off();
	{
	    int diff1;
	    int get_index = p_queue->c_queue.i_get;
	    int put_index = p_queue->c_queue.i_put;
	    int size = p_queue->c_queue.n_size;
	    diff = put_index - get_index;
	    diff1 = diff + size;
	    diff = (diff < 0) ? diff1 : diff;
	}
	ints_restore(saved_ps);
    return diff;
}

int mae_get_queue_space(mae_queue_t *p_queue)
{
    int diff;
	unsigned int saved_ps = ints_off();
	{
	    int get_index = p_queue->c_queue.i_get;
	    int put_index = p_queue->c_queue.i_put;
	    int size = p_queue->c_queue.n_size;
	    diff = put_index - get_index;
	    diff = (diff < 0) ? (diff + size) : diff;
	    diff = size - diff;
	}
	ints_restore(saved_ps);
    return diff;
}


void mae_clear_queue(mae_queue_t *p_queue)
{
	unsigned int saved_ps = ints_off();
	{
		while(p_queue)
		{
			p_queue->c_queue.i_put = p_queue->c_queue.i_get = 0;
			p_queue = p_queue->p_next;
		}
	}
	ints_restore(saved_ps);
}

void mae_fill_queue(mae_queue_t *p_queue)
{
	unsigned int saved_ps = ints_off();
	{
		while(p_queue)
		{
		    p_queue->c_queue.i_get = 0;
		    p_queue->c_queue.i_put = p_queue->c_queue.n_size; 
			p_queue = p_queue->p_next;
		}
	}
	ints_restore(saved_ps);
}

uint32_t mae_pop_queue(mae_queue_t *p_queue)
{
    int result = 0;
    int diff;
	unsigned int saved_ps = ints_off();
	{
	    void *buffer = p_queue->p_base;
	    int get_index = p_queue->c_queue.i_get;
	    int put_index = p_queue->c_queue.i_put;
	    int size = p_queue->c_queue.n_size;
	    
	    // Queue is empty - return 0, and do not advance indices
	    if(get_index != put_index)
	    {
		    switch(p_queue->c_queue.n_shift)
		    {
		        case 0:
		            result = ((char *)buffer)[get_index];
		            break;
		        case 1:
		            result = ((short *)buffer)[get_index];
		            break;
		        case 2:
		        default:
		            result = ((int *)buffer)[get_index];
		            break;
		    }
			// The pop data was from the full queue - mark it as not full
			diff = put_index - size;
			p_queue->c_queue.i_put = diff >= 0 ? get_index : put_index; 
			
		    // Adjust the index
		    get_index++;
			diff = get_index - size ;
			p_queue->c_queue.i_get = (diff < 0) ? get_index : diff;
	    }
	}
	ints_restore(saved_ps);
    return result;
}

void mae_push_queue(mae_queue_t *p_queue, uint32_t data)
{
	unsigned int saved_ps = ints_off();
	{
	    int diff;
	    void *buffer = p_queue->p_base;
	    int put_index = p_queue->c_queue.i_put;
	    int get_index;
	    int put_index_full;
	    int size = p_queue->c_queue.n_size;

	    // If queue is full - don't push anything - just return
	    if(put_index < size ) 
	    {
		    switch(p_queue->c_queue.n_shift)
		    {
		        case 0:
		            ((char *)buffer)[put_index] = data;
		            break;
		        case 1:
		            ((short *)buffer)[put_index] = data;
		            break;
		        case 2:
		        default:
		            ((int *)buffer)[put_index] = data;
		            break;
		    }
		    // Increment the queue put index and check for wrap-around
		    put_index++;
		    diff = put_index - size;
		    put_index = (diff < 0) ? put_index : diff;
		    put_index_full = put_index + size;	// Special value for the index to 
		    									//  mark the queue as full
		    // Update all put pointers in the linked queues
		    while(p_queue)
		    {
		    	// If the new put index collides with the get index -  
		    	//     mark the queue as full
		    	get_index = p_queue->c_queue.i_get;
		        diff = put_index - get_index;
		        p_queue->c_queue.i_put = diff ? put_index : put_index_full ;
		        p_queue = p_queue->p_next;
		    }
	    }
	}
	ints_restore(saved_ps);
}


//! mae_pop_queue_data
//!
//! This function take in a pointer to mae_queue_t, the pointer to data buffer 
//! and total number of elements that need to be popped from the queue. It then   
//! copies the elements from the queue to the data buffer keeping in mind 
//! wrapping of queue pointer. It also updates the i_get element.
void mae_pop_queue_data (
		mae_queue_t *p_queue,  	//!< (in) pointer to queue
		void *p_data, 			//!< (in) pointer to data buffer
		int n_elements) 		//!< (in) number of elements
{

	unsigned int saved_ps = ints_off();
	{
		int	shift_factor;
		int n_elements_0, n_elements_1;
		void *p_buffer;

		int size = p_queue->c_queue.n_size;
		int get_index = p_queue->c_queue.i_get;
		int put_index = p_queue->c_queue.i_put;
		int diff = size - get_index;
		shift_factor = p_queue->c_queue.n_shift;

	    if(get_index != put_index)
		{
			n_elements_0 = MIN(diff, n_elements);
			n_elements_1 = n_elements - n_elements_0;
	
			
			// Adjust the buffer pointer and counter
			p_buffer = (void *)((int) p_queue->p_base + (get_index << shift_factor));  
			n_elements_0 <<= shift_factor;
			n_elements_1 <<= shift_factor;
			
			
			memcpy(p_data, p_buffer, n_elements_0 );
			if (n_elements_1)
			{
				memcpy((void *)((int)p_data + n_elements_0), p_queue->p_base, n_elements_1);
			}
		
			// The pop data was from the full queue - mark it as not full
			diff = put_index - size;
			p_queue->c_queue.i_put = diff >= 0 ? get_index : put_index; 
			
			//update get index
			get_index += n_elements;
			diff = get_index - size ;
			p_queue->c_queue.i_get = (diff < 0) ? get_index : diff;
		}else
		{
			memset(p_data, 0, n_elements << shift_factor);
		}
	}
	ints_restore(saved_ps);
}

//! mae_push_queue_data
//!
//! This function takes in a pointer to mae_queue_t, the pointer to data buffer 
//! and total number of elements that need to be pushed into the queue. It then   
//! copies the elements from the data buffer to the queue keeping in mind 
//! wrapping of queue pointer. It also updates the i_get element.
void mae_push_queue_data (
		mae_queue_t *p_queue,  	//!< (in) pointer to queue
		void *p_data, 			//!< (in) pointer to data buffer
		int n_elements) 		//!< (in) number of elements
{
	unsigned int saved_ps = ints_off();
	{
		int	shift_factor;
		int n_elements_0, n_elements_1;
		int put_index_full;
		int get_index;
		
		void *p_buffer;
		int size = p_queue->c_queue.n_size;
		int put_index = p_queue->c_queue.i_put;
		int diff = size - put_index;

		if(diff > 0)
		{
			n_elements_0 = MIN(diff, n_elements);
			n_elements_1 = n_elements - n_elements_0;

			shift_factor = p_queue->c_queue.n_shift;
			
			p_buffer = (void *)((int)p_queue->p_base + (put_index << shift_factor));  
			n_elements_0 <<= shift_factor;
			n_elements_1 <<= shift_factor;

			memcpy(p_buffer, p_data, n_elements_0);
			if (n_elements_1)
			{
				memcpy(p_queue->p_base,	(void *)((int)p_data + n_elements_0), n_elements_1);
			}

			//update put index and wrap it around
			put_index += n_elements;
			diff = put_index - size;
			put_index = (diff < 0) ? put_index : diff;
			put_index_full = put_index + size;	// Special value for the index to 
	    										//  mark the queue as full
		    
			// Update all put pointers in the linked queues
			while(p_queue)
			{
	    		// If the new put index collides with the get index -  
	    		//     mark the queue as full
	    		get_index = p_queue->c_queue.i_get;
				diff = put_index - get_index;
				p_queue->c_queue.i_put = diff ? put_index : put_index_full ;
				p_queue = p_queue->p_next;
			}
		}
	}
	ints_restore(saved_ps);
}
