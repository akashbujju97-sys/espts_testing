#include <stdio.h>
#include <cstring>
#include "pico/stdlib.h"
#include "mcp2515.h"
#include "gy521.h"

void set_gy521() {
    i2c_init(i2c0, 400000);

    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);

    gpio_pull_up(4);
    gpio_pull_up(5);

    sleep_ms(100);
}

auto init_mcp2515() {
    MCP2515 *mcp{};

    mcp = new MCP2515(
        spi0,
        17,
        19,
        16,
        18
    );

    printf("Reset...\n");
    if (mcp->reset() != MCP2515::ERROR_OK)
    {
        printf("Reset failed\n");
        while (true);
    }

    mcp->setBitrate(CAN_500KBPS, MCP_8MHZ);
    mcp->setNormalMode();

    printf("CAN Ready\n");
    return mcp;
}

void pack(uint8_t* data, float id, float gx, float gy, float gz, bool status) {
    int16_t gx_i = static_cast<int16_t>(gx * 100);
    int16_t gy_i = static_cast<int16_t>(gy * 100);
    int16_t gz_i = static_cast<int16_t>(gz * 100);

    memcpy(&data[1], &gx_i, 2);
    memcpy(&data[3], &gy_i, 2);
    memcpy(&data[5], &gz_i, 2);
    data[7] = status ? 1 : 0;
    data[0] = id;
}

int main()
{
    stdio_init_all();

    sleep_ms(2000);
    auto mcp = init_mcp2515();
    set_gy521();
    mpu_init();

    struct can_frame tx;
    tx.can_id = 0x123;
    tx.can_dlc = 8;

    while (true)
    {
        auto [gx, gy, gz, status] = read_gyro();
        //printf("%d, gx: %.2f, gy: %.2f, gz: %.2f, %s\n", index++, gx, gy, gz, (status ? "True": "False"));

        pack(tx.data, 1, gx, gy, gz, status);
        auto err = mcp->sendMessage(&tx);
        if (err == MCP2515::ERROR_OK)
        {
            printf("TX: ");
            for (int i = 0; i < 8; i++)
                printf("%02X ", tx.data[i]);
            printf("\n");
        }
        else {
            printf("Error sending %d\n", err);
        }

        sleep_ms(1000);
    }
}