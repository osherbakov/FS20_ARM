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

#ifndef __MXTOS_H
#define __MXTOS_H

//! \defgroup MXTOS Maxim Xtensa Tasking OS
//!
//! \brief MXTOS: provides OS definitions and inline system functions 
//!
//! The mxtos.h gives users the ability to access and use OS functions.
//! It is a public header file that is intended for use by applications
//! that want the access to the system functionality provided by MAXIM OS.
//! The majority of the functions are defined as inline functions
//! using special opcodes - interrupts handling, processor status and control
//! reads and writes.
//!
//! \ingroup MXTOS
//! @{

// #define to simplify the concatenation of the register names and the
//  opcodes
#define _XTSTR(x) # x
#define XTSTR(x) _XTSTR(x)

// mark the entry point with a special attribute
#define FSPM_ENTRY 
#define _TCHAR void 
#define WINAPI __stdcall
#define __inline__  inline

// Common defines that will be translated into the single instruction.
#ifndef MAX
#define MAX(a,b) ((a) > (b)? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b)? (a) : (b))
#endif

#ifndef ABS
#define ABS(a)  ((a) < 0 ? -(a) : (a))
#endif

// Useful macro to calculate the number of the elements in the array.
#define NUM_ELEMS(a) (sizeof(a)/sizeof(a[0]))


// The inline function to initialize interrupts or other sync
// methods for queues access
static __inline__ void ints_init()
{
}

// The inline function to initialize interrupts or other sync
// methods for queues access
static __inline__ void ints_close()
{
}


// The inline function to enable all interrupts
// This is done by lowering current execution level to the lowest
// interrupt level (which is 0)
static __inline__ void ints_on()
{
}

// The inline function to restore the previous interrupts level
// It is used after ints_off, when we have to restore the original PS.   
static __inline__ void ints_restore(unsigned int saved_ps)
{
}

// The inline function to disable all interrupts
// This is done by rasing current execution level to the highest
// interrupt level
// Returns the current level and CPU status
static __inline__ unsigned int ints_off()
{
	return 0;
}

// The inline function to check and wait until interrupt level drops
static __inline__ void ints_active()
{
}
// The inline function to release interrupt
static __inline__ void ints_done()
{
}


// The inline function to wait for the interrupts
static __inline__ void ints_wait()
{
}

static __inline__ void ints_signal()
{	
}

// The inline function to start the command to be executed
static __inline__ void start_command()
{
}

// The inline function to wait for command to be executed
static __inline__ void wait_command_done()
{
}

// The inline function to mark that new command can be executed
static __inline__ void finish_command()
{
}

// The inline function to signal that command was executed
static __inline__ void signal_command_done()
{
}


// The inline function to get the current stack pointer
static __inline__ void *get_stack_pointer()
{
    void *ret;
	ret = 0;
	return ret;
}

// The inline function to set stack pointer
static __inline__ void set_stack_pointer(void *new_sp)
{
  
}

// The thread start function takes one parameter - pointer to anything
#define THREAD_TYPE
typedef unsigned int  thread_start_t(void *);

static __inline__ void create_working_thread(thread_start_t *p_thread_function, void *p_thread_data)
{
}

static __inline__ void set_thread_priority()
{
}


//! @} end MXTOS ingroup

#endif  /* __MXTOS_H */

