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

    struct can_frame tx;

tx.can_id = 0x123;
tx.can_dlc = 8;

uint8_t n0 = 0;
uint8_t n1 = 0;

while (true)
{
    tx.data[0] = n0;
    tx.data[1] = n1;
    for (int i = 2; i < 8; i++)
        tx.data[i] = 0;

    if (mcp.sendMessage(&tx) == MCP2515::ERROR_OK)
    {
        printf("TX: ");

        for (int i = 0; i < 8; i++)
            printf("%02X ", tx.data[i]);

        printf("\n");
    }

    n0 += 1;
    n1 += 1;
    if (n0 > 255) {
        n0 = 0;
    }
    if (n1 > 255) {
        n1 = 0;
    }

    sleep_ms(1000);
}
}