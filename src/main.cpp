#include "serial.h"
#include "dartt.h"


int main()
{
    Serial serial;

    unsigned char write_mem[24] = {};
    misc_write_message_t wmsg = 
    {
        .address = 3,
        .index = 1,
        .payload = {
            .buf = write_mem,
            .size = sizeof(write_mem),
            .len = 0
        }        
    };
    unsigned char uart_tx_buf[128] = {};
    buffer_t uart_tx = {
        .buf = uart_tx_buf,
        .size = sizeof(uart_tx_buf),
        .len = 0
    };
    int rc = dartt_create_write_frame(&wmsg, TYPE_SERIAL_MESSAGE, &uart_tx);

    if(serial.autoconnect())
    {
        printf("Connection success\n");
        return 0;
    }
    else
    {
        printf("Fuck off\n");
        return 1;
    }
}