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
#ifndef __SIMPLE_GAIN_MOD_H
#define __SIMPLE_GAIN_MOD_H
#include "mae.h"
#include "mxtos.h"

#ifdef __cplusplus
#define EXTCDECL extern "C"
#else
#define EXTCDECL extern 
#endif

enum 
{
	SIMPLE_GAIN_MOD_ID = 0x91,					// default ID of the module
	SIMPLE_GAIN_MOD_BLOCK_SIZE	= 80,			// Processing size
};
//*****************************************************************************
// structures
//*****************************************************************************

//*****************************************************************************
// functions
//*****************************************************************************
// Forward declaration of the main module processing function.
//! \breif module function conforming to MAE requirements -- the 
//! function signature is:
//! void module_process(void *p_state, int msg, int param1, 
//!                                  int param2)
EXTCDECL MODULE_PROCESS simple_gain_mod_process FSPM_ENTRY;

extern driver_descriptor_t simple_gain_mod_descriptor;

#endif  /* __SIMPLE_GAIN_MOD_H */
