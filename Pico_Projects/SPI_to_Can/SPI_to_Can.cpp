#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "mcp2515.h"
#include "hardware/uart.h"

#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

constexpr uint PIN_MISO = 16;
constexpr uint PIN_CS   = 17;
constexpr uint PIN_SCK  = 18;
constexpr uint PIN_MOSI = 19;
constexpr uint PIN_INT  = 20;

MCP2515 mcp2515(spi0, PIN_CS);

int main()
{
    stdio_init_all();
    sleep_ms(5000);
    spi_init(spi0, 1000 * 1000);
    uart_init(UART_ID, BAUD_RATE);

    uart_puts(UART_ID, "Hello from Pico 2W\r\n");

    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    sleep_ms(100);
    uart_puts(UART_ID, "Before mcp2515 reset\r\n");
    auto err = mcp2515.reset();

    if (err != MCP2515::ERROR_OK)
    {
        uart_puts(UART_ID, "MCP2515 reset failed\r\n");
        while (true)
            tight_loop_contents();
    }

    uart_puts(UART_ID, "Reset success!\r\n");
    mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
    mcp2515.setNormalMode();

    struct can_frame txFrame;
    txFrame.can_id  = 0x123;
    txFrame.can_dlc = 8;
    uint8_t counter = 0;

    uart_puts(UART_ID, "Starting while!\r\n");
    while (true)
    {
        txFrame.data[0] = counter++;
        txFrame.data[1] = 0x22;
        txFrame.data[2] = 0x33;
        txFrame.data[3] = 0x44;
        txFrame.data[4] = 0x55;
        txFrame.data[5] = 0x66;
        txFrame.data[6] = 0x77;
        txFrame.data[7] = 0x88;

        err = mcp2515.sendMessage(&txFrame);
        if (err == MCP2515::ERROR_OK)
        {
            uart_puts(UART_ID, "Frame sent\r\n");
        } else {
            uart_puts(UART_ID, "Error\r\n");
        }

        sleep_ms(1000);
    }
}