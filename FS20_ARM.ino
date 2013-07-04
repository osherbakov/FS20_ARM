#include "mxtos.h"
#include "config.h"
#include "simple_gain_mod.h"
#include "FlexSound20.h"

char  __heap[4096];
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

	p_ft->p_audio_engine_init(&__heap[0], 64);

	p_ft->p_audio_engine_config((int *)&config[0]);
	// Never returns from this function - stays in the loop.
	p_ft->p_audio_engine_start();

}


void loop()
{

	FS_init();
	FS_config(config);
	FS_start();
  /* add main program code here */

}
