#include "serial.h"
#include "dartt.h"
#include "cobs.h"


int main()
{
    Serial serial;

    unsigned char write_mem[24] = {3,4,5,6,7};
    misc_write_message_t wmsg = 
    {
        .address = 3,
        .index = 1,
        .payload = {
            .buf = write_mem,
            .size = sizeof(write_mem),
            .len = 5
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
        char kill_str[128];
        for(int i = 0; i < sizeof(kill_str); i++)
        {
            kill_str[i] = 'f';
        }   
        serial.write((uint8_t*)kill_str, sizeof(kill_str));
        const char str[] = "garbage garbage\n";
        serial.write((uint8_t*)str, sizeof(str)-1);
        
        cobs_prepend_zero_single_buffer(&cobs_tx_buf);
		serial.write(cobs_tx_buf.buf, (int)cobs_tx_buf.length);

        // cobs_tx_buf.encoded_state = COBS_DECODED;
        // cobs_tx_buf.buf[0] = 'h';
        // cobs_tx_buf.buf[1] = 'e';
        // cobs_tx_buf.buf[2] = 'l';
        // cobs_tx_buf.buf[3] = 'l';
        // cobs_tx_buf.buf[4] = 'o';
        // cobs_tx_buf.length = 5;
        // rc = cobs_encode_single_buffer(&cobs_tx_buf);
        // serial.write(cobs_tx_buf.buf, (int)cobs_tx_buf.length);


		uart_tx.len = 0;
        return 0;
    }
    else
    {
        printf("Fuck off\n");
        return 1;
    }
}