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
	(int)((0 << 24) | ((ID) << 16) | ((ID) << 8) | (0 << 0) )

#define STD_DRIVER16(ID)\
	(int)((0 << 24) | (0 << 16) | (0 << 8) | (1 << 0) ), \
	(int)( ((ID) << 16) | ((ID) << 0) )

#define STD_MODULE8(ID)\
	(int)((0 << 24) | ((ID) << 16) | ((ID) << 8) | (0 << 0) )

#define STD_MODULE16(ID)\
	(int)((0 << 24) | (0 << 16) | (0 << 8) | (1 << 0) ), \
	(int)( ((ID) << 16) | ((ID) << 0) )

#define NEW_DRIVER8(ID_OLD, ID_NEW)\
	(int)((0 << 24) | ((ID_OLD) << 16) | ((ID_NEW) << 8) | (0 << 0) )

#define NEW_MODULE8(ID_OLD, ID_NEW)\
	(int)((0 << 24) | ((ID_OLD) << 16) | ((ID_NEW) << 8) | (0 << 0) )

#define NEW_DRIVER16(OLD_ID, NEW_ID)\
	(int)((0 << 24) | (0 << 16) | (0 << 8) | (1 << 0) ), \
	(int)( ((OLD_ID) << 16) | ((NEW_ID) << 0) )

#define NEW_MODULE16(OLD_ID, NEW_ID)\
	(int)((0 << 24) | (0 << 16) | (0 << 8) | (1 << 0) ), \
	(int)( ((OLD_ID) << 16) | ((NEW_ID) << 0) )

#define MODULE8(ID, P_FUNC)\
	(int)((0 << 24) | ((ID) << 16) | ((ID) << 8) | (1 << 0) ), \
	(int) (P_FUNC)

#define DRIVER8(ID, P_FUNC)\
	(int)((0 << 24) | ((ID) << 16) | ((ID) << 8) | (1 << 0) ), \
	(int) (P_FUNC)

#define MODULE16(ID, P_FUNC)\
	(int)((0 << 24) | (0 << 16) | (0 << 8) | (2 << 0) ), \
	(int)(((ID) << 16) | ((ID) << 0) ), \
	(int) (P_FUNC)

#define DRIVER16(ID, P_FUNC)\
	(int)((0 << 24) | (0 << 16) | (0 << 8) | (2 << 0) ), \
	(int)(((ID) << 16) | ((ID) << 0) ), \
	(int) (P_FUNC)



#define STREAMS(N)\
	(int)((0 << 24) | (0 << 16) | (1 << 8) | ((N) << 2) )

#define PARAMS(N)\
	(int)((0 << 24) | (0 << 16) | (2 << 8) | ((N) << 0) )

#define CONFIG(N)\
	(int)((0 << 24) | (0 << 16) | (3 << 8) | ((N) << 0) )

#define PARAM(NUMBER, VALUE)\
	(int)(((NUMBER) << 24) | ((VALUE) << 0) )

#define DATA(VALUE)\
	(int)((VALUE) << 0)

#define STREAM(DIR, WID, FRAME, BUS_NAME, PIN_NAME)\
	(int)( ((DIR) << 0) ), \
	(int)( ((FRAME) << 16) | ((WID) << 0) ),\
	(int)(((BUS_NAME) << 0) ), \
	(int)(((PIN_NAME) << 0) )
	
#define CONNECTION8(SRC_ID, SRC_PIN, DST_ID, DST_PIN)\
	(int)(((SRC_ID) << 24) | ((SRC_PIN) << 16) | ((DST_ID) << 8) | ((DST_PIN) << 0) )

#define CONNECTION16(SRC_ID, SRC_PIN, DST_ID, DST_PIN)\
	(int)((0 << 24) | (0 << 16) | (6 << 8) | (2 << 0) ), \
	(int)( ((SRC_ID) << 16) |   ((DST_ID) << 0) ), \
	(int)( ((SRC_PIN) << 16) |  ((DST_PIN) << 0) )

#define CONNECTION32(SRC_ID, SRC_PIN, DST_ID, DST_PIN)\
	(int)((0 << 24) | (0 << 16) | (6 << 8) | (3 << 0) ), \
	(int)( ((SRC_ID) << 16) |   ((DST_ID) << 0) ), \
	(int)( ((SRC_PIN) << 0 ) ), \
	(int)( ((DST_PIN) << 0 ) )

#define BUS8(SRC_ID, SRC_BUS, DST_ID, DST_BUS)\
	(int)((0 << 24) | (0 << 16) | (5 << 8) | (2 << 0) ),\
	(int)(((SRC_ID) << 24) | ((SRC_BUS) << 0) ),\
	(int)(((DST_ID) << 24) | ((DST_BUS) << 0) )

#define BUS16(SRC_ID, SRC_BUS, DST_ID, DST_BUS)\
	(int)((0 << 24) | (0 << 16) | (5 << 8) | (3 << 0) ), \
	(int)( ((SRC_ID) << 16) |   ((DST_ID) << 0) ), \
	(int)( ((SRC_BUS) << 0) ),\
	(int)( ((DST_BUS) << 0) )

#define SYSCONFIG_1(HEAP_START) \
	(int)((0 << 24) | (0 << 16) | (4 << 8) | (1 << 0) ), \
	(int) (HEAP_START)

#define SYSCONFIG_2(HEAP_START, MSG_Q_SIZE) \
	(int)((0 << 24) | (0 << 16) | (4 << 8) | (2 << 0) ), \
	(int) (HEAP_START), \
	(int) (MSG_Q_SIZE)

#define END_DATA \
	(int)(0)

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
