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

#ifndef MAX
#define MAX(a,b) ((a) > (b)? (a) : (b))
#endif


//
//  Defines to simplify the configuration data array parsing 
//
#define B3(a)		((a) & 0xFF000000)
#define B2(a)		((a) & 0x00FF0000)
#define B1(a)		((a) & 0x0000FF00)
#define B0(a)		((a) & 0x000000FF)

#define IS_STREAMS(a) 	( ((a) & 0xFFFFFF00) == 0x00000100 )
#define IS_PARAMS(a) 	( ((a) & 0xFFFFFF00) == 0x00000200 )
#define IS_CONFIG(a) 	( ((a) & 0xFFFFFF00) == 0x00000300 )
#define IS_SYSCONFIG(a) ( ((a) & 0xFFFFFF00) == 0x00000400 )
#define IS_BUS8(a) 	(  (a) == 0x00000502 )
#define IS_BUS16(a) 	(  (a) == 0x00000503 )
#define IS_CONN16(a)	(  (a) == 0x00000602 )
#define IS_CONN32(a)	(  (a) == 0x00000603 )

#define IS_CONN8(a)		( B3(a))

#define IS_DRIVER8(a) 	( !B3(a) && B2(a) && B1(a) && !B0(a) )
#define IS_DRIVER8_P(a) ( !B3(a) && B2(a) && B1(a) && (B0(a)==0x01))

#define IS_DRIVER16(a) 	( (a) == 0x00000001 )
#define IS_DRIVER16_P(a)( (a) == 0x00000002 )


#define IS_ANY_DRIVER(a) 	( IS_DRIVER8(a) || IS_DRIVER16(a) || IS_DRIVER8_P(a) || IS_DRIVER16_P(a) )
#define IS_ANY_BUS(a) 		(IS_BUS8(a) || IS_BUS16(a))
#define IS_ANY_CONNECTION(a)    (IS_CONN8(a) || IS_CONN16(a) || IS_CONN32(a))

#define IS_END(a) 		( ((a) == 0x00000000) || (~(a) == 0x00000000) )

#define ID1(a)			( ((a) & 0x00FF0000) >> 16 )
#define ID2(a)			( ((a) & 0x0000FF00) >> 8 )
#define ID16_1(a)		( ((a) & 0xFFFF0000) >> 16 )
#define ID16_2(a)		( ((a) & 0x0000FFFF) >> 0 )

#define WORD_SIZE(a)	( (a) & 0x000000FF )

int mae_audio_engine_config(uint32_t *p_config_data)
{
	// Start parsing the configuration data
	// The rules are as follows:
	// 1. Every configuration is an array of 32-bit unsigned integers (words)
	// 2. If the MSB is not 0x00, then it is a connection record.
	// 		  ID1 PIN1 ID2 PIN2	- connect PIN1 of Module/Driver ID1
	//			to the PIN2 of Module/Driver2
	//	Because the Driver/Module ID cannot be 0x00, we use that special ID of 0x00
	//  	to code other special cases. 
	// 3. If the MSB byte of the uint is 0x00, then it has a special meaning:
	// 4. The LSB will tell how many 32-bit uints (words) follow, and the rest should 
	//		be interpreted as:
	//
	//        00  ID1 ID1 00  	- use standard driver/module with ID ID1
	//
	//		  00  ID1 ID2 00	- use already defined or standard driver/module with ID of ID1 
	//			but assign it ID2 (this can be useful for creating multiple drivers with the same 
	//       	excutable code, but different IDs and parameters).
	//
	//		  00  ID1 ID1 01	- register the driver with ID1 and p_Process function 
	//		  p_Process
	//			
	//		  00  00  00  01	- register driver with 16-bit IDs
	//		  ID  D1  ID  D2		
	//
	//		  00  00  00  02	- register driver with 16-bit ID and p_Process function 
	//		  ID  D1  ID  D1		
	//		  p_Process
	//
	//		  00  00  01  NN	- The N 32-bit words that describe the STREAMS for the driver/module
	//			will be following.
	//
	//		  00  00  02  NN	- The N 32-bit words that describe the PARAMETERS for the driver/module
	//			will be following.
	//
	//		  00  00  03  NN	- The N 32-bit words that describe the CONFIGURATION data for the driver/module
	//			will be following.
	//
	//		  00  00  04  01	- The 2 32-bit words that describe the SYSTEM configuration
	//		  HEAP_START_ADDR	- Specifies the starting address of the heap
	//
	//		  00  00  04  02	- The 2 32-bit words that describe the SYSTEM configuration
	//		  HEAP_START_ADDR	- Specifies the starting address of the heap
	//		  MESSAGE_QUEUE_SIZE- Specified the size of the message queue
	//
	//		  00  00  05  02	- 2 32-bit words  that describe the BUS connection for the driver/module
	//		  ID1  BUS1_NAME	- Bus name for the Driver/Module with the ID1	
	//		  ID2  BUS2_NAME	- Bus name for the Driver/Module with the ID2
	//
	//		  00  00  05  03	- 3 32-bit words  that describe the BUS connection for the driver/module
	//		  IDD1  IDD2		- Module IDs - IDD1 - source, IDD2 - destination
	//		  BUS1_NAME			- Bus name for the Driver/Module with the IDD1	
	//		  BUS2_NAME			- Bus name for the Driver/Module with the IDD2
	//
	//		  00  00  06  02	- 2 32-bit words  that describe the pin connection for the driver/module
	//		  IDD1  IDD2		- Module IDs - IDD1 - source, IDD2 - destination
	//		  PIN1  PIN2		- Pin numbers/names to be connected
	//
	//		  00  00  06  03	- 3 32-bit words  that describe the pin connection for the driver/module
	//		  IDD1  IDD2		- Module IDs - IDD1 - source, IDD2 - destination
	//		  PIN1  			- Pin number/name for Module IDD1 to be connected
	//		  PIN2				- Pin number/name for Module IDD2 to be connected
	//		
	//		  00  00  00  00	- the END of the configuration data
	//
	// So the logic should be 2-pass - first pass to initialize the modules/drivers
	//		and the second pass will establish connections between the modules.
	// Here is the format of the connections data
	// 
	//		  ID1 PIN1 ID2 PIN2	- Connect the PIN1 of Module/Driver ID1 with PIN2 of Module ID2

	
	//   Pass 1  - Modules/drivers instantiation and initialization.
	//		 
	//
	unsigned int 	b_done; 
	uint32_t 	*p_data;
	uint32_t 	*p_next_data; 
	uint32_t	data_word;			// Currently fetched data
	void 		*p_data_heap_start;		// The heap start address
	unsigned int	n_msg_queue_size;		// Value for MESSAGE QUEUE SIZE
	
	driver_descriptor_t	s_driver_desc;	
	
	driver_descriptor_t	*p_driver_desc;
	DRIVER_PROCESS		*p_process;
	uint16_t		n_driver_id1, n_driver_id2;
	unsigned int		n_words;
	unsigned int		i;

	void	 *p_state;
	uint32_t   *p_streams;
	uint32_t *p_config;
	uint32_t *p_params;

	uint16_t src_id, 
		dst_id, 
		src_pin, 
		dst_pin;

	// Check for the null pointer
	if(0 == p_config_data) return 0;

	p_data = p_config_data;
	data_word = *p_data;
	
	//
	// If the start of the heap is expicitly specified in the config word - use that value
	//	If not specified in the config AND the previous value was set - use that previous value 
	//    If not specified  in the config AND was not set before - use reasonable default (_end) 
	if(IS_SYSCONFIG(data_word))
	{	
		// Set up default values for the heap start and msg queue size
		p_data_heap_start = (void *) 0;
		n_msg_queue_size = 128;

		// System configuration - re-init everything
		p_data++;
		p_data_heap_start = (void *) *p_data++;	// The heap start param will be there
		if(WORD_SIZE(data_word) == 0x02)		// MSG QUEUE SIZE is optional
		{
			n_msg_queue_size = *p_data++;
		}
		p_config_data =  p_data;			// Skip the SYSCONFIG data
		gp_functions_table->p_audio_engine_init(p_data_heap_start, n_msg_queue_size);
	}
	
	b_done = 0;
	while(!b_done)
	{
		data_word = *p_data++;
		
		if(IS_END(data_word))	
		{	// We got to the closing word
			b_done = 1;		// Go to the phase 2
		}else if(IS_CONN8(data_word))
		{	// Connection word - skip it
		}else if( IS_ANY_BUS(data_word) || IS_CONN16(data_word))
		{	// That is the bus connection word - skip it
			p_data += WORD_SIZE(data_word); // The size of the records to skip is in byte_0
		}else if( IS_ANY_DRIVER(data_word))
		{
			p_process = 0;						// Initially we don't have any process function
                        n_driver_id1 = n_driver_id2 = 0;
			if(IS_DRIVER8(data_word))
			{	// The new driver is specified
				n_driver_id1 = ID1(data_word); 
				n_driver_id2 = ID2(data_word);		// that will be new ID for the driver
			}else if(IS_DRIVER16(data_word))
			{	// The new driver is specified
				data_word = *p_data++;
				n_driver_id1 = ID16_1(data_word); 
				n_driver_id2 = ID16_2(data_word);	// that will be new ID for the driver
			// special case of the new driver
			}else if(IS_DRIVER8_P(data_word))
			{	// The new driver is specified
				// The address of the processing function is in the next word
				n_driver_id1 = ID1(data_word); 
				n_driver_id2 = ID2(data_word);		// that will be new ID for the driver
				p_process = (DRIVER_PROCESS *) *p_data++;
			}else if (IS_DRIVER16_P(data_word))
			{
				data_word = *p_data++;
				n_driver_id1 = ID16_1(data_word); 
				n_driver_id2 = ID16_2(data_word);	// that will be new ID for the driver
				p_process = (DRIVER_PROCESS *) *p_data++;
			}
			p_next_data = p_data;	// Save the pointer where we will start 
									//    looking for the next driver
			// No processing function is specified - get it from the driver
			if(p_process == 0)
			{
				p_process = mae_get_driver_function(n_driver_id1);
			}

			// Get driver descriptor via MSG_GET_INFO message 
			p_driver_desc = 0;			// Initialize the pointer
			if(p_process) 
			{
				p_process(0, MSG_GET_INFO, n_driver_id1, (uint32_t) &p_driver_desc);
			}
			if(p_driver_desc != 0)		// If valid pointer returned - copy the structure
			{
				s_driver_desc = *p_driver_desc;
			}
			
			s_driver_desc.unique_id = n_driver_id2; 
			s_driver_desc.p_process = p_process;
			
			// Find all streams for that driver
			// To do this, loop thru all data words that follow, until you find a STREAMS(N)
			// data word, or hit the next driver or bus or connection definition, 
			//   or reach the end of data.
			p_streams = p_data;
			while(1) 
			{
				data_word = *p_streams++;
				n_words = WORD_SIZE(data_word);
				if(IS_END(data_word) || IS_ANY_DRIVER(data_word) || IS_ANY_BUS(data_word) || IS_ANY_CONNECTION(data_word))
				{
					p_streams--;	// Roll back the pointer 
					break;
				}else if (IS_CONFIG(data_word) || IS_PARAMS(data_word) )
				{
					p_streams += n_words;	// Skip all the data that follow 
				}
				else if(IS_STREAMS(data_word))
				{
					// We found the STREAMS(N) record - update the fields
					s_driver_desc.n_streams = (n_words >> 2); // One stream descriptor takes 4 words
					s_driver_desc.p_streams = (mae_stream_descriptor_t *) p_streams;
					p_streams += n_words;
					break;
				}
			}
			p_next_data = MAX(p_next_data, p_streams);
			
			//
			// Register driver with the system - MAE will allocate memory, issue MSG_INIT message
			//  
			if(p_process)
			{
				gp_functions_table->p_register_driver(n_driver_id2, &s_driver_desc);
			}
			
			// Get the driver/module state - it was allocated by the MAE
			p_state = gp_functions_table->p_get_driver_state(n_driver_id2);
			
			//
			//  Configure freshly registered driver, then change the parameters
			//

			// Find all Configuration bytes for that driver
			p_config = p_data;
			while(1) 
			{
				data_word = *p_config++;
				n_words = WORD_SIZE(data_word);
				if(IS_END(data_word) || IS_ANY_DRIVER(data_word) || IS_ANY_BUS(data_word) || IS_ANY_CONNECTION(data_word) )
				{
					p_config--;
					break;
				}else if (IS_STREAMS(data_word) || IS_PARAMS(data_word) )
				{
					p_config += n_words;	// Skip all the data that follow 
				}else if(IS_CONFIG(data_word))
				{
					// We found the CONFIG(N) record - send MSG_CONFIG message
					if(p_process) 
					{
						p_process(p_state, MSG_CONFIGURE, n_driver_id2, (uint32_t) p_config);
					}
					p_config += n_words;
					break;
				}
			}
			p_next_data = MAX(p_next_data, p_config);
			
			//Find all parameters for that driver 
			p_params = p_data;
			while(1) 
			{
				data_word = *p_params++;
				n_words = WORD_SIZE(data_word);
				if(IS_END(data_word) || IS_ANY_DRIVER(data_word) || IS_ANY_BUS(data_word) || IS_ANY_CONNECTION(data_word) )
				{
					p_params--;
					break;
				}else if (IS_STREAMS(data_word) || IS_CONFIG(data_word) )
				{
					p_params += n_words;	// Skip all the data that follow 
				}else if(IS_PARAMS(data_word))
				{
					// We found the PARAMS(N) record - send MSG_SET_PARAM message
					for(i = 0; i < n_words; i++)
					{
						if(p_process)
						{
							p_process(p_state, MSG_SET_PARAM, n_driver_id2, (uint32_t)*p_params++);
						}
					}
					break;
				}
			}
			p_data = MAX(p_next_data, p_params);
		}
	}
	
	//
	// Phase 2 - start scan all data records again  
	//  looking for connection records.
	//
	p_data =  p_config_data;
	b_done = 0;
	
	while(!b_done)
	{
		data_word = *p_data++;
		
		if(IS_END(data_word))	
		{	// We got to the closing word
			b_done = 1;
		}else if(IS_CONN8(data_word))
		{	
			// Connection word
			// 	(int)((SRC_ID << 24) | (SRC_PIN << 16) | (DST_ID << 8) | (DST_PIN << 0) )
			src_id = ( data_word >> 24 ) & 0x00FF;
			src_pin = ( data_word >> 16 ) & 0x00FF;
			dst_id = ( data_word >> 8 ) & 0x00FF;
			dst_pin = ( data_word >> 0 ) & 0x00FF;
			gp_functions_table->p_register_connection(src_id, src_pin, dst_id, dst_pin, 0);
		}else if (IS_BUS8(data_word))
		{
			// Bus Connection word
			// BUS(SRC_ID, SRC_BUS, DST_ID, DST_BUS)
			//	(int)((0 << 24) | (0 << 16) | (5 << 8) | (2 << 0) ),
			//	(int)((SRC_ID << 24) | (SRC_BUS << 0) ),
			//	(int)((DST_ID << 24) | (DST_BUS << 0) )
			data_word = *p_data++;
			src_id = ( data_word >> 24 ) & 0x00FF;
			src_pin =  data_word  & 0x00FFFFFF;
			data_word = *p_data++;
			dst_id = ( data_word >> 24 ) & 0x00FF;
			dst_pin =  data_word  & 0x00FFFFFF;
			gp_functions_table->p_register_bus(src_id, src_pin, dst_id, dst_pin, 0);
		}else if(IS_CONN16(data_word))
		{	
			// Connection word
			//	(int)((0 << 24) | (0 << 16) | (6 << 8) | (2 << 0) ),
			// 	(int)((SRC_ID << 16) | (DST_ID << 0) )
			// 	(int)((SRC_PIN << 16 ) | (DST_PIN << 0) )
			data_word = *p_data++;
			src_id = ID16_1(data_word) ;
			dst_id = ID16_2(data_word);
			data_word = *p_data++;
			src_pin = ID16_1(data_word);
			dst_pin = ID16_2(data_word) ;
			gp_functions_table->p_register_connection(src_id, src_pin, dst_id, dst_pin, 0);
		}else if(IS_CONN32(data_word))
		{	
			// Connection word
			//	(int)((0 << 24) | (0 << 16) | (6 << 8) | (3 << 0) ),
			// 	(int)((SRC_ID << 16) | (DST_ID << 0) )
			// 	(int)((SRC_PIN << 0) )
			// 	(int)((DST_PIN << 0) )
			data_word = *p_data++;
			src_id = ID16_1(data_word) ;
			dst_id = ID16_2(data_word) ;
			src_pin = *p_data++ ;
			dst_pin = *p_data++ ;
			gp_functions_table->p_register_connection(src_id, src_pin, dst_id, dst_pin, 0);
		}else if (IS_BUS16(data_word))
		{
			// Bus Connection word
			// BUS(SRC_ID, SRC_BUS, DST_ID, DST_BUS)
			//	(int)((0 << 24) | (0 << 16) | (5 << 8) | (3 << 0) ),
			//	(int)((SRC_ID << 16) | (DST_ID << 0) ),
			//	(int)((SRC_BUS << 0) )
			//	(int)((DST_BUS << 0) )
			data_word = *p_data++;
			src_id = ID16_1(data_word) ;
			dst_id = ID16_2(data_word);
			src_pin = *p_data++;
			dst_pin =  *p_data++;
			gp_functions_table->p_register_bus(src_id, src_pin, dst_id, dst_pin, 0);
		}else if(IS_ANY_DRIVER(data_word) || IS_PARAMS(data_word) || IS_CONFIG(data_word) || IS_STREAMS(data_word))
		{
			p_data += WORD_SIZE(data_word);	// Skip all the data that follow
		}
	}

	// If the last word is -1, then start audio engine immediately 
	if(data_word == 0xFFFFFFFF)
	{
		gp_functions_table->p_audio_engine_start();
	}

	return 0;
}
