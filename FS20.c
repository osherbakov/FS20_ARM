#include "mxtos.h"
#include "config.h"
#include "simple_gain_mod.h"
#include "FlexSound20.h"

#define USE_CONFIG (1)

#ifndef ARDUINO
// char  __heap[4096];
uint32_t config[] = 
{
	MODULE(0x50, &simple_gain_mod_process),
	MODULE(0x51, &simple_gain_mod_process),

	CONNECTION(0x50, 0, 0x51, 0),
	CONNECTION(0x51, 0, 0x50, 0),

	END_DATA
};

void setup()
{

	mae_functions_table_t *p_ft = gp_functions_table;

	p_ft->p_audio_engine_init(0, 64);

#ifdef USE_CONFIG
	p_ft->p_audio_engine_config(&config[0]);
#else
	p_ft->p_register_driver(0x55, &module1_descriptor);
	p_ft->p_register_driver(0x66, &dsound_module_descriptor);
	p_ft->p_register_connection(0x66, 0, 0x55, 0, 4096);
	p_ft->p_register_connection(0x66, 1, 0x55, 1, 4096);
	p_ft->p_register_connection(0x55, 0, 0x66, 0, 4096);
	p_ft->p_register_connection(0x55, 1, 0x66, 1, 4096);

#endif

	// Never returns from this function - stays in the loop.
	p_ft->p_audio_engine_start();

}


void loop()
{
	FS_init();
	FS_config((uint32_t *)config);
	FS_start();

  /* add main program code here */

}

#endif
