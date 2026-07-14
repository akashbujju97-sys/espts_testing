#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19



int main()
{
    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    int index = 0;
    while (true) {
        uint8_t cmd0[] = {0x01, 0x11, 0x22};
        uint8_t cmd1 = 0x02;
        uint8_t res0 = 0;

        gpio_put(PIN_CS, 0);
        spi_write_blocking(SPI_PORT, cmd0, sizeof(cmd0));
        gpio_put(PIN_CS, 1);
        sleep_ms(200);

        gpio_put(PIN_CS, 0);
        spi_write_blocking(SPI_PORT, &cmd1, 1);
        spi_read_blocking(SPI_PORT, 0, &res0, 1);
        gpio_put(PIN_CS, 1);
        sleep_ms(200);
    }
}
