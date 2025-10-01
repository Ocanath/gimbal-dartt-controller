#include "dartt_init.h"
#include <SDL.h>
#include <chrono>


int main(int argc, char* args[])
{
	if(!serial.autoconnect(2000000))
	{
        printf("Failed to find a serial port\n");
        return 1;		
	}
    
	int rc = dartt_read_multi(&ctl_alias_full, &periph_alias_full, &ds);
	if(rc != DARTT_PROTOCOL_SUCCESS)
	{
		printf("Failed to get read data\n");
	}
	else
	{
		printf("Read data success.\n");
		for (int i = 0; i < sizeof(dartt_controller_params_t); i += sizeof(int32_t))
		{
			int32_t* pi32 = (int32_t*)(&periph_alias_full.buf[i]);
			printf("W%d = %d\n", i / sizeof(int32_t), *pi32);
		}
	}
	return rc;
}


