#pragma once

#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "common/pimoroni_common.hpp"
#include "common/pimoroni_bus.hpp"
#include "libraries/pico_graphics/pico_graphics.hpp"


#ifndef NO_QSTR
#include "st7789_parallel.pio.h"
#endif

#include <algorithm>


namespace pimoroni {

  class ST7789 : public DisplayDriver {
    spi_inst_t *spi = PIMORONI_SPI_DEFAULT_INSTANCE;
  
  public:
    bool round;

    //--------------------------------------------------
    // Variables
    //--------------------------------------------------
  private:

    // interface pins with our standard defaults where appropriate
    uint cs;
    uint dc;
    uint wr_sck;
    uint rd_sck = PIN_UNUSED;
    uint d0;
    uint bl;
    uint vsync  = PIN_UNUSED; // only available on some products
    uint parallel_sm;
    PIO parallel_pio;
    uint parallel_offset;
    uint st_dma;

    // The ST7789 requires 16 ns between SPI rising edges.
    // 16 ns = 62,500,000 Hz
    // 2350 doesn't support 62,500,000 so use 75,000,000 seems to work.
    #if !PICO_RP2350
      static const uint32_t SPI_BAUD = 62'500'000;
    #else
      static const uint32_t SPI_BAUD = 75'000'000;
    #endif

  public:

    // Serial init
    ST7789(uint16_t width, uint16_t height, Rotation rotation, bool round, SPIPins pins) :
      DisplayDriver(width, height, rotation),
      spi(pins.spi), round(round),
      cs(pins.cs), dc(pins.dc), wr_sck(pins.sck), d0(pins.mosi), bl(pins.bl) {

      // configure spi interface and pins
      spi_init(spi, SPI_BAUD);

      gpio_set_function(wr_sck, GPIO_FUNC_SPI);
      gpio_set_function(d0, GPIO_FUNC_SPI);

      st_dma = dma_claim_unused_channel(true);
      dma_channel_config config = dma_channel_get_default_config(st_dma);
      channel_config_set_transfer_data_size(&config, DMA_SIZE_8);
      channel_config_set_bswap(&config, false);
      channel_config_set_dreq(&config, spi_get_dreq(spi, true));
      dma_channel_configure(st_dma, &config, &spi_get_hw(spi)->dr, NULL, 0, false);
  
      common_init();
    }

    void cleanup() override;
    void update(PicoGraphics *graphics) override;
    void set_backlight(uint8_t brightness) override;

  private:
    void common_init();
    void configure_display(Rotation rotate);
    void write_blocking_dma(const uint8_t *src, size_t len);
    void write_blocking_parallel(const uint8_t *src, size_t len);
    void command(uint8_t command, size_t len = 0, const char *data = NULL);
  };

}
