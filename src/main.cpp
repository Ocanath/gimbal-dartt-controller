#include "serial.h"
#include "dartt.h"


int main()
{
    Serial serial;
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