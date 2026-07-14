#include <stdio.h>

#include "pico/stdlib.h"
#include "mcp2515.h"

int main()
{
    stdio_init_all();

    sleep_ms(2000);

    MCP2515 mcp(
        spi0,
        17,     // CS
        19,     // MOSI
        16,     // MISO
        18      // SCK
    );

    printf("Reset...\n");

    if (mcp.reset() != MCP2515::ERROR_OK)
    {
        printf("Reset failed\n");
        while (true);
    }

    //
    // Select the crystal frequency on YOUR board.
    //
    // If crystal says 8.000:
    //
    mcp.setBitrate(CAN_500KBPS, MCP_8MHZ);

    //
    // If crystal says 16.000 use:
    //
    // mcp.setBitrate(CAN_500KBPS, MCP_16MHZ);
    //

    mcp.setNormalMode();

    printf("CAN Ready\n");

    struct can_frame frame;

    frame.can_id  = 0x123;
    frame.can_dlc = 8;

    uint8_t counter = 0;

    while (true)
    {
        for(int i=0;i<8;i++)
            frame.data[i]=counter+i;

        MCP2515::ERROR err = mcp.sendMessage(&frame);

        if(err == MCP2515::ERROR_OK)
        {
            printf("Sent : ");

            for(int i=0;i<8;i++)
                printf("%02X ",frame.data[i]);

            printf("\n");
        }
        else
        {
            printf("TX Error = %d\n",err);
        }

        counter++;

        sleep_ms(1000);
    }
}