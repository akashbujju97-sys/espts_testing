#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#define SPI_PORT spi0

#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

void cs_low()
{
    gpio_put(PIN_CS, 0);
}

void cs_high()
{
    gpio_put(PIN_CS, 1);
}

int main()
{
    stdio_init_all();

    spi_init(SPI_PORT, 1000000);      // 1 MHz

    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    cs_high();

    sleep_ms(100);

    uint8_t tx[3];
    uint8_t rx[3];

    while (true)
    {
        //
        // RESET
        //
        cs_low();

        tx[0] = 0xC0;

        spi_write_blocking(SPI_PORT, tx, 1);

        cs_high();

        sleep_ms(10);

        //
        // READ CANSTAT (0x0E)
        //

        tx[0] = 0x03;
        tx[1] = 0x0E;
        tx[2] = 0x00;

        cs_low();

        spi_write_read_blocking(SPI_PORT, tx, rx, 3);

        cs_high();

        printf("RX = %02X\n", rx[2]);

        sleep_ms(100);
    }
}