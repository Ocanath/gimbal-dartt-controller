#include "dartt_init.h"
#include "sauron-eye-closedform-ik.h"
#include "trig_fixed.h"
#include "checksum.h"
#include <SDL.h>
#include <chrono>


int main(int argc, char* args[])
{
	if(!serial.autoconnect(2000000))
	{
        printf("Failed to find a serial port\n");
        return -1;		
	}

	//read the full dartt structure from the dartt target (in this case, the controller middleman over UART - which also uses DARTT to communicate with the motor controllers. The record contains a snapshot of the motor state as well.)
	int rc = dartt_read_multi(&ctl_alias_full, &periph_alias_full, &ds);
	if(rc != DARTT_PROTOCOL_SUCCESS)
	{
		printf("Failed to get read data\n");
		return rc;
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
    uint16_t crc = get_crc16(periph_alias_full.buf, periph_alias_full.size);
    for(int i = 0; i < periph_alias_full.size; i++)
    {
        periph_alias_full.buf[i] = 0;
    }
    rc = dartt_read_multi(&ctl_alias_full, &periph_alias_full, &ds);
    uint16_t crc2 = get_crc16(periph_alias_full.buf, periph_alias_full.size);
    if(crc != crc2)
    {
        printf("Double read checksum mismatch\n");
        return -1;
    }

    

	SDL_Window* window = NULL;
	SDL_Color bgColor = { 10, 10, 10, 255 };
	const int SCREEN_WIDTH = 1200;
	const int SCREEN_HEIGHT = 800;
	//Initialize SDL
	rc = SDL_Init(SDL_INIT_VIDEO);
	if(rc < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return rc;
	}
	window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return -1;
	}
	SDL_Event e; 
	bool quit = false; 

	uint64_t start_tick = SDL_GetTicks64();
	//SDL mouse config settings
	SDL_SetRelativeMouseMode(SDL_TRUE);
	SDL_WarpMouseInWindow(window, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
	//mouse memory init
	int prev_mouse_x = SCREEN_WIDTH / 2;
	int prev_mouse_y = SCREEN_HEIGHT / 2;
	int32_t accum_mouse_x = 0;
	int32_t accum_mouse_y = 0;
	double forward = 0;
	const double slow_speedcap = 200;
	const double fast_speedcap = 3000;
	double velocity_threshold = slow_speedcap;
	double th1 = 0;
	double th2 = 0;
	const int num_keys = 322;
	bool keys[num_keys];  // 322 is the number of SDLK_DOWN events
	for (int i = 0; i < num_keys; i++) { // init them all to false
		keys[i] = false;
	}
	int32_t delta = 0;
	uint64_t ms_ts = 0;
	uint64_t w_ts = 0;
	uint64_t s_ts = 0;
	uint64_t w_stop_ts = 0;
	uint64_t s_stop_ts = 0;
	bool w_pressed_prev = false;
	bool s_pressed_prev = false;
	int zerocount = 0;
	uint64_t mouse_activity_ts = 0;
	uint64_t print_ts = 0;
	uint64_t udp_tx_ts = 0;
	int32_t deltax_lastvalid = 0;
	uint8_t throttle = 0;
	uint8_t prev_throttle = 0;
	uint64_t throttle_stop_ts = 0;
	double targx = 0;
	double targy = 0;
	while (quit == false) 
	{
		uint64_t tick = SDL_GetTicks64() - start_tick;
		if (tick - ms_ts > 0)
		{
			delta = (int32_t)(tick - ms_ts);
			ms_ts = tick;
		}
		float t = ((float)tick) * 0.001f;

		uint8_t mouse_motion = 0;
		SDL_PollEvent(&e);
		{
			if (e.type == SDL_QUIT)
				quit = true;
			else if (e.type == SDL_MOUSEMOTION)
			{
				mouse_motion = 1;
				mouse_activity_ts = tick;
			}
		}

		int mouse_x, mouse_y;
		uint32_t bts = SDL_GetMouseState(&mouse_x, &mouse_y);
		int32_t gain_x = 1;
		int32_t gain_y = -1;
		int32_t deltax = (mouse_x - prev_mouse_x)* gain_x;
		int32_t deltay = (mouse_y - prev_mouse_y) * gain_y;

		accum_mouse_x = ( (accum_mouse_x + deltax) );
		accum_mouse_y = ( ( accum_mouse_y + deltay) );
		
		if (mouse_motion != 0 || (tick - mouse_activity_ts) > 10)
		{
			deltax_lastvalid = deltax;
		}



		double kbvx = -(double)accum_mouse_x / 1000.;
		double kbvy = (double)accum_mouse_y / 1000.;

		double vx = kbvx;
		double vy = kbvy;
		if (bts == 1)	//left mouse button
		{
			vx = targx;
			vy = targy;
		}
		if (bts == 2)	//center mouse button
		{
			targx = vx;
			targy = vy;
		}
		
		//manually rotate the target frame by 45 degrees Z so it's aligned to the base, which we've also rotated by 45 degrees.
		//the ik solution is expressed assuming that joint 1 is parallel to the x axis
		const double rotangle = M_PI / 4;
		double vxr = vx * cos(rotangle) - vy * sin(rotangle);
		double vyr = vx * sin(rotangle) + vy * cos(rotangle);
		get_ik_angles_double(vxr, vyr, 10, &th1, &th2);
		int32_t th1_i32 = (int32_t)(th1 * (float)PI_14B);
		int32_t th2_i32 = (int32_t)(th2 * (float)PI_14B);

		if ( (tick - print_ts) > 50)
		{
			printf("%f, %f, %d, %d, %d\n", vx, vy, th1_i32, th2_i32, bts);
			print_ts = tick;
		}

		SDL_WarpMouseInWindow(window, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

		//TODO: capture mouse click information and pass it to the controller to fire

		if (tick - udp_tx_ts > 10)
		{
			udp_tx_ts = tick;
			//TODO: replace this with dartt request transmission

			// uint8_t pld[32] = { 0 };
			// uint16_t* pu16 = (uint16_t*)(&pld[0]);
			// int i = 0;
			// pld[i++] = POSITION;
			// pld[i++] = 0;

			// int32_t* p_cmd32 = (int32_t*)(&pld[i]);
			// p_cmd32[0] = th1_i32;
			// i += sizeof(int32_t);
			// p_cmd32[1] = th2_i32;
			// i += sizeof(int32_t);

			// int chkidx = (i + (i % 2)) / 2;	//pad a +1 byte if it's odd, divide by 2, set that as the start of our 16bit checksum
			// pu16[chkidx] = fletchers_checksum16(pu16, chkidx);
			// chkidx++;
			// int pld_size = chkidx * sizeof(uint16_t);
			// int stuffed_size = PPP_stuff(pld, pld_size, gl_ppp_stuffing_buffer, sizeof(gl_ppp_stuffing_buffer));
			// //sendto(client.s, (const char*)gl_ppp_stuffing_buffer, stuffed_size, 0, (struct sockaddr*)&client.si_other, client.slen);
			// serial_write(gl_ppp_stuffing_buffer, stuffed_size);
		}
	}








	
}


