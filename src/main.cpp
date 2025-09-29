#include "serial.h"
#include "dartt.h"
#include "dartt_sync.h"
#include "cobs.h"
#include "dartt_controller_params.h"

Serial serial;
unsigned char tx_mem[64] = {};
unsigned char rx_mem[64] = {};
dartt_controller_params_t ctl_dp = {};
dartt_controller_params_t ctl_periph = {};


int tx_blocking(unsigned char addr, buffer_t * b, uint32_t timeout)
{
    serial.write(b->buf, b->len);
    return DARTT_PROTOCOL_SUCCESS;
}
int rx_blocking(unsigned char addr, buffer_t * buf, uint32_t timeout)
{
    serial.read(buf->buf, buf->size);
    return DARTT_PROTOCOL_SUCCESS;
}


int main()
{
    dartt_sync_t ds = 
    {
        .address = 0,
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
            .buf = rx_mem,
            .size = sizeof(rx_mem),
            .len = sizeof(rx_mem)
        },
        .blocking_tx_callback = &rx_blocking,
        .blocking_rx_callback = &tx_blocking,
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

        int rc = dartt_ctl_read(&ctl_alias, &periph_alias, &ds);
    }
    else
    {
        printf("Failed to find a serial port\n");
        return 1;
    }
}


