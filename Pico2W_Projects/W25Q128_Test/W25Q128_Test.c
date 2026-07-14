#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/spi.h"

#define SPI_PORT spi0

#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

//--------------------------------------------

void cs_select()
{
    gpio_put(PIN_CS, 0);
}

void cs_deselect()
{
    gpio_put(PIN_CS, 1);
}

//--------------------------------------------

void write_enable()
{
    uint8_t cmd = 0x06;

    cs_select();
    spi_write_blocking(SPI_PORT, &cmd, 1);
    cs_deselect();
}

//--------------------------------------------

uint8_t read_status()
{
    uint8_t cmd = 0x05;
    uint8_t status = 0;

    cs_select();

    spi_write_blocking(SPI_PORT, &cmd, 1);
    spi_read_blocking(SPI_PORT, 0, &status, 1);

    cs_deselect();

    return status;
}

//--------------------------------------------

void wait_busy()
{
    while (read_status() & 0x01)
        sleep_ms(1);
}

//--------------------------------------------

void sector_erase(uint32_t addr)
{
    write_enable();

    uint8_t cmd[4];

    cmd[0] = 0x20;
    cmd[1] = addr >> 16;
    cmd[2] = addr >> 8;
    cmd[3] = addr;

    cs_select();
    spi_write_blocking(SPI_PORT, cmd, 4);
    cs_deselect();

    wait_busy();
}

//--------------------------------------------

void page_program(uint32_t addr, const uint8_t *data, size_t len)
{
    write_enable();

    uint8_t cmd[4];

    cmd[0] = 0x02;
    cmd[1] = addr >> 16;
    cmd[2] = addr >> 8;
    cmd[3] = addr;

    cs_select();

    spi_write_blocking(SPI_PORT, cmd, 4);
    spi_write_blocking(SPI_PORT, data, len);

    cs_deselect();

    wait_busy();
}

//--------------------------------------------

void read_data(uint32_t addr, uint8_t *buffer, size_t len)
{
    uint8_t cmd[4];

    cmd[0] = 0x03;
    cmd[1] = addr >> 16;
    cmd[2] = addr >> 8;
    cmd[3] = addr;

    cs_select();

    spi_write_blocking(SPI_PORT, cmd, 4);
    spi_read_blocking(SPI_PORT, 0, buffer, len);

    cs_deselect();
}

//--------------------------------------------

int main()
{
    stdio_init_all();

    sleep_ms(6000);

    printf("W25Q128 Example\n");

    spi_init(SPI_PORT, 10 * 1000 * 1000);

    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    const char text[] = "Hello from Raspberry Pi Pico!";
    uint8_t buffer[64];

    memset(buffer, 0, sizeof(buffer));

    uint32_t address = 0x000000;

    printf("Erasing sector...\n");
    sector_erase(address);

    printf("Writing data...\n");
    page_program(address,
                 (const uint8_t*)text,
                 strlen(text) + 1);

    printf("Reading back...\n");

    read_data(address, buffer, sizeof(buffer));

    printf("Read: %s\n", buffer);

    while (true)
    {
        sleep_ms(1000);
    }
}