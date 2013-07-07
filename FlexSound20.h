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
#ifndef __FLEXSOUND_2_0_H
#define __FLEXSOUND_2_0_H

#include <stdint.h>
#include <stddef.h>

#ifndef EXTCDECL
#ifdef __cplusplus
#define EXTCDECL extern "C"
#else	// __cplusplus
#define EXTCDECL extern
#endif	// __cplusplus
#endif	// EXTCDECL


typedef void	FS_queue_t;


EXTCDECL void		*FS_malloc( 
	size_t		n_bytes, 
	uint32_t	alignment);
// allocate n_bytes of memory with the starting address aligned on (2^ alignment) 
// (1, 2, 4, 8, �) bytes boundary. The memory returned is filled with 0, 
// so no calls to memset(...) are necessary.

EXTCDECL int		FS_init();
// the call to initialize the FlexSound processing environment.

EXTCDECL int		FS_config(
	uint32_t	*p_Configuration);
// this call configures the FlexSound Processing Modules and establishes the connections between them.

EXTCDECL int		FS_add_module(
	uint32_t	module_id,
	void		*p_process_function);
// This function registers the module with the Flexsound Framework

EXTCDECL int		FS_start();
// to start the processing of the data. This function should be called from the separate 
// processing thread, and that thread becomes the main execution thread on which 
// all Processing Modules are called to do data processing. 
// This call never returns, and, as said before, all execution of the Processing Modules 
// will be done in the context of that calling thread.


EXTCDECL int		FS_stop();
// to finish the processing, break out of the loop and terminate the separate 
//   processing thread

EXTCDECL int		FS_close();


//
//Functions to connect to input and output queues
//
//  Get the FlexSound Processing Module input queue (sink)
EXTCDECL FS_queue_t* 	FS_input_queue(
 	uint32_t 	module_id,
	uint32_t 	queue_id,
	uint32_t	queue_size);


// Get the FlexSound Processing Module output queue (source)
EXTCDECL FS_queue_t* 	FS_output_queue(
 	uint32_t 	module_id,
	uint32_t 	queue_id,
	uint32_t	queue_size);

//
// Functions to get the attached queue information (constant)
//
EXTCDECL size_t		FS_queue_size(
	FS_queue_t	*p_queue);
// Returns the maximum number of elements that can be placed into the queue.

EXTCDECL uint32_t 	FS_queue_stride(
	FS_queue_t	*p_queue);
// Returns the size of the elements (in bytes) that queue can handle. Valid values are 1, 2, and 4.

//
// Functions to get the current status of the queue
//
EXTCDECL size_t		FS_queue_space (
	FS_queue_t	*p_queue);
// Returns the number of elements in the queue that can to be placed by FS_queue_put_data(�) function

EXTCDECL size_t		FS_queue_count(
	FS_queue_t	*p_queue); 
// Returns the number of elements that can be taken from the queue by calling FS_queue_get_data(�)

//
// Functions to clear or fill the queue
//
EXTCDECL int		FS_queue_clear(
	FS_queue_t	*p_queue); 
// Clears the queue

EXTCDECL int		FS_queue_fill(
	FS_queue_t	*p_queue); 
// Fills the queue with all zeroes

//
// Functions to send and receive data/samples to/from the queues
//
EXTCDECL int		FS_queue_put_data(
	FS_queue_t	*p_queue,
	void		*p_data,
	uint32_t	n_elements );
// Place n_elements into the queue

EXTCDECL int		FS_queue_get_data(
	FS_queue_t	*p_queue,
	void		*p_data,
	uint32_t	n_elements );
// Get n elements for the queue

// 
// Function to fill queue and notify only specific module
// 
EXTCDECL int		FS_queue_put_direct_data(
	FS_queue_t	*p_queue,
	void		*p_data,
	uint32_t	n_elements,
	uint32_t	module_id );

//
// Functions to control the Processing Modules by providing Processing Module configuration and parameters
//
EXTCDECL int	        FS_set_config(
	uint32_t 	module_id,
	void		*p_data,
 	uint32_t	n_bytes	);


EXTCDECL int		FS_set_param(
	uint32_t 	module_id,
	uint32_t 	param_id,
	uint32_t	param_value);

EXTCDECL uint32_t	FS_get_param(
	uint32_t 	module_id,
	uint32_t 	param_id);


#endif  /*__FLEXSOUND_2_0_H */
