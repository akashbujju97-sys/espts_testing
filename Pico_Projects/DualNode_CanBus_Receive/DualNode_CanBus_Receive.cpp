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

    struct can_frame rx;

while (true)
{
    if (mcp.checkReceive())
    {
        if (mcp.readMessage(&rx) == MCP2515::ERROR_OK)
        {
            uint16_t sum = rx.data[0] + rx.data[1];
            printf("ID: %03lX  DLC=%d  DATA: ",
                   rx.can_id,
                   rx.can_dlc);

            printf("SUM: %d", sum);
            /*for (int i = 0; i < rx.can_dlc; i++)
                printf("%02X ", rx.data[i]);*/

            printf("\n");
        }
    }

    sleep_ms(5);
}
}