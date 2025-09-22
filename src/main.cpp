#include "serial.h"
#include "dartt.h"
#include "cobs.h"


int main()
{
    Serial serial;

    unsigned char write_mem[24] = {1,2,3};
    misc_write_message_t wmsg = 
    {
        .address = 3,
        .index = 1,
        .payload = {
            .buf = write_mem,
            .size = sizeof(write_mem),
            .len = 3
        }        
    };
    unsigned char uart_tx_buf[128] = {};
    buffer_t uart_tx = {
        .buf = uart_tx_buf,
        .size = sizeof(uart_tx_buf),
        .len = 0
    };
    int rc = dartt_create_write_frame(&wmsg, TYPE_SERIAL_MESSAGE, &uart_tx);
    cobs_buf_t cobs_tx_buf = 
    {
        .buf = uart_tx_buf,
        .size = sizeof(uart_tx_buf),
        .length = uart_tx.len,
        .encoded_state = COBS_DECODED
    };
    rc = cobs_encode_single_buffer(&cobs_tx_buf);
    if(rc != COBS_SUCCESS)
    {
        printf("Cobs usage error %d\n", rc);
        return rc;
    }


    if(serial.autoconnect(2000000))
    {
        printf("Connection success\n");
		serial.write(uart_tx.buf, (int)uart_tx.len);
		uart_tx.len = 0;
        return 0;
    }
    else
    {
        printf("Fuck off\n");
        return 1;
    }
}