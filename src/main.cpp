#include "serial.h"
#include "dartt.h"
#include "dartt_sync.h"
#include "cobs.h"
#include "dartt_controller_params.h"
#include <SDL.h>

Serial serial;
unsigned char tx_mem[64] = {};
unsigned char rx_dartt_mem[64] = {};
unsigned char rx_cobs_mem[64] = {};
dartt_controller_params_t ctl_dp = {};
dartt_controller_params_t ctl_periph = {};


int tx_blocking(unsigned char addr, buffer_t * b, uint32_t timeout)
{
	cobs_buf_t cb = {
		.buf = b->buf,
		.size = b->size,
		.length = b->len,
		.encoded_state = COBS_DECODED
	};
	int rc = cobs_encode_single_buffer(&cb);
	if (rc != 0)
	{
		return rc;
	}
    rc = serial.write(cb.buf, (int)cb.length);
	if(rc == cb.length)
	{
		return DARTT_PROTOCOL_SUCCESS;
	}
	else
	{
		return -1;
	}
}

int rx_blocking(unsigned char addr, buffer_t * buf, uint32_t timeout)
{
	cobs_buf_t cb_enc =
	{
		.buf = rx_cobs_mem,
		.size = sizeof(rx_cobs_mem),
		.length = buf->len
	};

    int rc = serial.read(cb_enc.buf, cb_enc.size);	//implement our own cobs blocking read, similar to hdlc/ppp ~ check
	if (rc >= 0)
	{
		buf->len = rc;
		cb_enc.length = buf->len;
	}
	else
	{
		return -1;
	}

	cobs_buf_t cb_dec =
	{
		.buf = buf->buf,
		.size = buf->size,
		.length = 0
	};
	rc = cobs_decode_double_buffer(&cb_enc, &cb_dec);
	if (rc != COBS_SUCCESS)
	{
		return rc;
	}
	else
	{
		return DARTT_PROTOCOL_SUCCESS;
	}
    
}


int main()
{
    dartt_sync_t ds = 
    {
        .address = 3,
        .base = 
        {
            .buf = (unsigned char *)(&ctl_dp),
            .size = sizeof(ctl_dp),
            .len = sizeof(ctl_dp)
        },
        .msg_type = TYPE_SERIAL_MESSAGE,
        .tx_buf = 
        {
            .buf = tx_mem,
            .size = sizeof(tx_mem),
            .len = 0
        },
        .rx_buf = {
            .buf = rx_dartt_mem,
            .size = sizeof(rx_dartt_mem),
            .len = 0
        },
        .blocking_tx_callback = &tx_blocking,
        .blocking_rx_callback = &rx_blocking,
        .timeout_ms = 0
    };

    if(serial.autoconnect(2000000))
    {
        buffer_t ctl_alias = 
        {
            .buf = (unsigned char *)(&ctl_dp),
            .size = sizeof(ctl_dp),
            .len = sizeof(ctl_dp)
        };
        buffer_t periph_alias = 
        {
            .buf = (unsigned char *)(&ctl_periph),
            .size = sizeof(ctl_periph),
            .len = sizeof(ctl_periph)
        };

		ctl_alias.len = 4;	//read only the first 4 bytes
        int rc = dartt_ctl_read(&ctl_alias, &periph_alias, &ds);
		return rc;
    }
    else
    {
        printf("Failed to find a serial port\n");
        return 1;
    }
}


