#include "Display.hpp"

#include "common/pimoroni_common.hpp"

static const unsigned int PIN_UNUSED = INT_MAX;         // Intentionally INT_MAX to avoid overflowing MicroPython's int type

static const unsigned int SPI_PIN_CS = 17;              // Chip Select
static const unsigned int SPI_PIN_SCK = 18;             // Serial Clock
static const unsigned int SPI_PIN_COPI = 19;            // Controller Out, Peripheral In
static const unsigned int SPI_PIN_CIPO = PIN_UNUSED;    // Controller In, Peripheral Out
static const unsigned int SPI_PIN_DC = 16;              // Data/Command
static const unsigned int SPI_PIN_BL = 20;              // Backlight

uint8_t madctl;
uint16_t caset[2] = {0, 0};
uint16_t raset[2] = {0, 0};

enum MADCTL : uint8_t {
    ROW_ORDER   = 0b10000000,
    COL_ORDER   = 0b01000000,
    SWAP_XY     = 0b00100000,  // AKA "MV"
    SCAN_ORDER  = 0b00010000,
    RGB_BGR     = 0b00001000,
    HORIZ_ORDER = 0b00000100
};

void ST7789::Initialize() {
    // configure spi interface and pins
    spi_init(spi, SPI_BAUD);

    gpio_set_function(SPI_PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI_PIN_COPI, GPIO_FUNC_SPI);

    st_dma = dma_claim_unused_channel(true);
    dma_channel_config config = dma_channel_get_default_config(st_dma);
    channel_config_set_transfer_data_size(&config, DMA_SIZE_8);
    channel_config_set_bswap(&config, false);
    channel_config_set_dreq(&config, spi_get_dreq(spi, true));
    dma_channel_configure(st_dma, &config, &spi_get_hw(spi)->dr, NULL, 0, false);

    gpio_set_function(SPI_PIN_DC, GPIO_FUNC_SIO);
    gpio_set_dir(SPI_PIN_DC, GPIO_OUT);

    gpio_set_function(SPI_PIN_CS, GPIO_FUNC_SIO);
    gpio_set_dir(SPI_PIN_CS, GPIO_OUT);

    // if a backlight pin is provided then set it up for
    // pwm control
    pwm_config cfg = pwm_get_default_config();
    pwm_set_wrap(pwm_gpio_to_slice_num(SPI_PIN_BL), 65535);
    pwm_init(pwm_gpio_to_slice_num(SPI_PIN_BL), &cfg, true);
    gpio_set_function(SPI_PIN_BL, GPIO_FUNC_PWM);
    SetBacklight(0); // Turn backlight off initially to avoid nasty surprises

    SendCommand(Command::SWRESET);

    sleep_ms(150);

    // Common init
    SendCommand(Command::TEON);                                                 // enable frame sync signal if used
    SendCommand(Command::COLMOD,    1, (const char*)&colorFormat);              // 16 bits per pixel

    SendCommand(Command::PORCTRL, 5, "\x0c\x0c\x00\x33\x33");
    SendCommand(Command::LCMCTRL, 1, "\x2c");
    SendCommand(Command::VDVVRHEN, 1, "\x01");
    SendCommand(Command::VRHS, 1, "\x12");
    SendCommand(Command::VDVS, 1, "\x20");
    SendCommand(Command::PWCTRL1, 2, "\xa4\xa1");
    SendCommand(Command::FRCTRL2, 1, "\x0f");

    // As noted in https://github.com/pimoroni/pimoroni-pico/issues/1040
    // this is required to avoid a weird light grey banding issue with low brightness green.
    // The banding is not visible without tweaking gamma settings (GMCTRP1 & GMCTRN1) but
    // it makes sense to fix it anyway.
    SendCommand(Command::RAMCTRL, 2, "\x00\xc0");

    SendCommand(Command::GCTRL, 1, "\x35");
    SendCommand(Command::VCOMS, 1, "\x1f");
    SendCommand(Command::GMCTRP1, 14, "\xD0\x08\x11\x08\x0C\x15\x39\x33\x50\x36\x13\x14\x29\x2D");
    SendCommand(Command::GMCTRN1, 14, "\xD0\x08\x10\x08\x06\x06\x39\x44\x51\x0B\x16\x14\x2F\x31");

    SendCommand(Command::INVON);   // set inversion mode
    SendCommand(Command::SLPOUT);  // leave sleep mode
    SendCommand(Command::DISPON);  // turn display on

    sleep_ms(100);

    ConfigureDisplay(rotation);

    sleep_ms(50); // Wait for the update to apply
    SetBacklight(255); // Turn backlight on now that surprises have passed
}

void ST7789::Cleanup() {
    if(dma_channel_is_claimed(st_dma)) {
        dma_channel_abort(st_dma);
        dma_channel_unclaim(st_dma);
    }
}

void ST7789::write_blocking_dma(const uint8_t *src, size_t len) {
    while (dma_channel_is_busy(st_dma))
        ;
    dma_channel_set_trans_count(st_dma, len, false);
    dma_channel_set_read_addr(st_dma, src, true);
}

void ST7789::SendCommand(Command command, size_t len, const char *data) {
    gpio_put(SPI_PIN_DC, 0); // SendCommand mode

    gpio_put(SPI_PIN_CS, 0);

    spi_write_blocking(spi, (const uint8_t*)&command, 1);

    if(data) {
        gpio_put(SPI_PIN_DC, 1); // data mode
        spi_write_blocking(spi, (const uint8_t*)data, len);
    }

    gpio_put(SPI_PIN_CS, 1);
}

void ST7789::ConfigureDisplay(pimoroni::Rotation rotation) {
    caset[0] = 0;
    caset[1] = 319;
    raset[0] = 0;
    raset[1] = 239;
    madctl = (rotation == pimoroni::ROTATE_180 || rotation == pimoroni::ROTATE_90) ? MADCTL::ROW_ORDER : MADCTL::COL_ORDER;
    madctl |= MADCTL::SWAP_XY | MADCTL::SCAN_ORDER;

    // Byte swap the 16bit rows/cols values
    caset[0] = __builtin_bswap16(caset[0]);
    caset[1] = __builtin_bswap16(caset[1]);
    raset[0] = __builtin_bswap16(raset[0]);
    raset[1] = __builtin_bswap16(raset[1]);

    SendCommand(Command::CASET,  4, (char *)caset);
    SendCommand(Command::RASET,  4, (char *)raset);
    SendCommand(Command::MADCTL, 1, (char *)&madctl);
}

void ST7789::Update(DisplayContext *display) {
    Command cmd = Command::RAMWR;

    gpio_put(SPI_PIN_DC, 0); // SendCommand mode
    gpio_put(SPI_PIN_CS, 0);

    spi_write_blocking(spi, (const uint8_t*)&cmd, 1);

    gpio_put(SPI_PIN_DC, 1); // data mode

    display->GetFrame(colorFormat, [this](void *data, size_t length) {
        if (length > 0) {
            write_blocking_dma((const uint8_t*)data, length);
        }
        else {
            dma_channel_wait_for_finish_blocking(st_dma);
        }
    });

    gpio_put(SPI_PIN_CS, 1);
}

void ST7789::SetBacklight(uint8_t brightness) {
    // gamma correct the provided 0-255 brightness value onto a
    // 0-65535 range for the pwm counter
    float gamma = 2.8;
    uint16_t value = (uint16_t)(pow((float)(brightness) / 255.0f, gamma) * 65535.0f + 0.5f);
    pwm_set_gpio_level(SPI_PIN_BL, value);
}
