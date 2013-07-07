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
#ifndef __CONFIG_H
#define __CONFIG_H

#include <stdint.h>
#include "streams.h"
#include "messages.h"

#define STD_DRIVER8(ID)\
	(uint32_t)((0L << 24) | (((uint32_t)ID) << 16) | (((uint32_t)ID) << 8) | (0 << 0) )

#define STD_DRIVER16(ID)\
	(uint32_t)((0L << 24) | (0L << 16) | (0 << 8) | (1 << 0) ), \
	(uint32_t)( (((uint32_t)ID) << 16) | (((uint32_t)ID) << 0) )

#define STD_MODULE8(ID)\
	(uint32_t)((0L << 24) | (((uint32_t)ID) << 16) | (((uint32_t)ID) << 8) | (0 << 0) )

#define STD_MODULE16(ID)\
	(uint32_t)((0L << 24) | (0L << 16) | (0 << 8) | (1 << 0) ), \
	(uint32_t)( (((uint32_t)ID) << 16) | (((uint32_t)ID) << 0) )

#define NEW_DRIVER8(ID_OLD, ID_NEW)\
	(uint32_t)((0L << 24) | (((uint32_t)ID_OLD) << 16) | (((uint32_t)ID_NEW) << 8) | (0 << 0) )

#define NEW_MODULE8(ID_OLD, ID_NEW)\
	(uint32_t)((0L << 24) | (((uint32_t)ID_OLD) << 16) | (((uint32_t)ID_NEW) << 8) | (0 << 0) )

#define NEW_DRIVER16(OLD_ID, NEW_ID)\
	(uint32_t)((0L << 24) | (0L << 16) | (0 << 8) | (1 << 0) ), \
	(uint32_t)( (((uint32_t)OLD_ID) << 16) | (((uint32_t)NEW_ID) << 0) )

#define NEW_MODULE16(OLD_ID, NEW_ID)\
	(uint32_t)((0L << 24) | (0L << 16) | (0 << 8) | (1 << 0) ), \
	(uint32_t)( (((uint32_t)OLD_ID) << 16) | (((uint32_t)NEW_ID) << 0) )

#define MODULE8(ID, P_FUNC)\
	(uint32_t)((0L << 24) | (((uint32_t)ID) << 16) | (((uint32_t)ID) << 8) | (1 << 0) ), \
	(uint32_t) (P_FUNC)

#define DRIVER8(ID, P_FUNC)\
	(uint32_t)((0L << 24) | (((uint32_t)ID) << 16) | (((uint32_t)ID) << 8) | (1 << 0) ), \
	(uint32_t) (P_FUNC)

#define MODULE16(ID, P_FUNC)\
	(uint32_t)((0L << 24) | (0L << 16) | (0 << 8) | (2 << 0) ), \
	(uint32_t)((((uint32_t)ID) << 16) | (((uint32_t)ID) << 0) ), \
	(uint32_t) (P_FUNC)

#define DRIVER16(ID, P_FUNC)\
	(uint32_t)((0L << 24) | (0L << 16) | (0 << 8) | (2 << 0) ), \
	(uint32_t)((((uint32_t)ID) << 16) | (((uint32_t)ID) << 0) ), \
	(uint32_t) (P_FUNC)



#define STREAMS(N)\
	(uint32_t)((0L << 24) | (0L << 16) | (1 << 8) | ((N) << 2) )

#define PARAMS(N)\
	(uint32_t)((0L << 24) | (0L << 16) | (2 << 8) | ((N) << 0) )

#define CONFIG(N)\
	(uint32_t)((0L << 24) | (0L << 16) | (3 << 8) | ((N) << 0) )

#define PARAM(NUMBER, VALUE)\
	(uint32_t)((((uint32_t)NUMBER) << 24) | ((VALUE) << 0) )

#define DATA(VALUE)\
	(uint32_t)((VALUE) << 0)

#define STREAM(DIR, WID, FRAME, BUS_NAME, PIN_NAME)\
	(uint32_t)( ((DIR) << 0) ), \
	(uint32_t)( (((uint32_t)FRAME) << 16) | ((WID) << 0) ),\
	(uint32_t)(((BUS_NAME) << 0) ), \
	(uint32_t)(((PIN_NAME) << 0) )
	
#define CONNECTION8(SRC_ID, SRC_PIN, DST_ID, DST_PIN)\
	(uint32_t)((((uint32_t)SRC_ID) << 24) | (((uint32_t)SRC_PIN) << 16) | ((DST_ID) << 8) | ((DST_PIN) << 0) )

#define CONNECTION16(SRC_ID, SRC_PIN, DST_ID, DST_PIN)\
	(uint32_t)((0L << 24) | (0L << 16) | (6 << 8) | (2 << 0) ), \
	(uint32_t)( (((uint32_t)SRC_ID) << 16) |   ((DST_ID) << 0) ), \
	(uint32_t)( (((uint32_t)SRC_PIN) << 16) |  ((DST_PIN) << 0) )

#define CONNECTION32(SRC_ID, SRC_PIN, DST_ID, DST_PIN)\
	(uint32_t)((0L << 24) | (0L << 16) | (6 << 8) | (3 << 0) ), \
	(uint32_t)( (((uint32_t)SRC_ID) << 16) |   ((DST_ID) << 0) ), \
	(uint32_t)( ((SRC_PIN) << 0 ) ), \
	(uint32_t)( ((DST_PIN) << 0 ) )

#define BUS8(SRC_ID, SRC_BUS, DST_ID, DST_BUS)\
	(uint32_t)((0L << 24) | (0L << 16) | (5 << 8) | (2 << 0) ),\
	(uint32_t)((((uint32_t)SRC_ID) << 24) | ((SRC_BUS) << 0) ),\
	(uint32_t)((((uint32_t)DST_ID) << 24) | ((DST_BUS) << 0) )

#define BUS16(SRC_ID, SRC_BUS, DST_ID, DST_BUS)\
	(uint32_t)((0L << 24) | (0L << 16) | (5 << 8) | (3 << 0) ), \
	(uint32_t)( (((uint32_t)SRC_ID) << 16) |   ((DST_ID) << 0) ), \
	(uint32_t)( ((SRC_BUS) << 0) ),\
	(uint32_t)( ((DST_BUS) << 0) )

#define SYSCONFIG_1(HEAP_START) \
	(uint32_t)((0L << 24) | (0L << 16) | (4 << 8) | (1 << 0) ), \
	(uint32_t) (HEAP_START)

#define SYSCONFIG_2(HEAP_START, MSG_Q_SIZE) \
	(uint32_t)((0L << 24) | (0L << 16) | (4 << 8) | (2 << 0) ), \
	(uint32_t) (HEAP_START), \
	(uint32_t) (MSG_Q_SIZE)

#define END_DATA \
	(uint32_t)(0)
          
#define END_DATA_AND_START \
	(uint32_t)(0xFFFFFFFF)

// Now define which version will be used by default

#define STD_MODULE(a) STD_MODULE16(a)
#define STD_DRIVER(a) STD_DRIVER16(a)
#define NEW_MODULE(a,b) NEW_MODULE16(a,b)
#define NEW_DRIVER(a,b) NEW_DRIVER16(a,b)
#define MODULE(a,b) MODULE16(a,b)
#define DRIVER(a,b) DRIVER16(a,b)
#define CONNECTION(a,b,c,d) CONNECTION16(a,b,c,d) 
#define BUS(a,b,c,d) BUS16(a,b,c,d) 


#endif  /*__CONFIG_H */
