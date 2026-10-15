#include <array>
#include <cstring>

#include "ZxSpectrumDisplayContext.hpp"
#include "ZxScreenFont.h"

#define ZX_ATTR_COLOR_MASK 0b111
#define ZX_ATTR_PAPER_SHIFT 3

#define ZX_ATTR_INK_MASK    ZX_ATTR_COLOR_MASK
#define ZX_ATTR_PAPER_MASK  (ZX_ATTR_COLOR_MASK << ZX_ATTR_PAPER_SHIFT)

#define OFF 0b0
#define ON  0b1

#define ZX_ATTR_BRIGHT_SHIFT 6
#define ZX_ATTR_BRIGHT_MASK  (0b1 << ZX_ATTR_BRIGHT_SHIFT)

#define ZX_ATTR_FLASH_SHIFT 7
#define ZX_ATTR_FLASH_MASK  (0b1 << ZX_ATTR_FLASH_SHIFT)

#define ZX_ATTR_BRIGHT_OFF  (OFF << ZX_ATTR_BRIGHT_SHIFT)
#define ZX_ATTR_BRIGHT_ON   (ON << ZX_ATTR_BRIGHT_SHIFT)

#define ZX_ATTR_FLASH_OFF   (OFF << ZX_ATTR_FLASH_SHIFT)
#define ZX_ATTR_FLASH_ON    (ON << ZX_ATTR_FLASH_SHIFT)

namespace {

constexpr std::array<uint16_t, SCREEN_HEIGHT_PIXELS> MakePixelRowOffsets() {
    std::array<uint16_t, SCREEN_HEIGHT_PIXELS> offsets{};
    for (uint16_t y = 0; y < SCREEN_HEIGHT_PIXELS; ++y) {
        offsets[y] = (((y & 0xC0) << 5)
            + ((y & 0x07) << 8)
            + ((y & 0x38) << 2));
    }
    return offsets;
}

constexpr std::array<uint16_t, SCREEN_HEIGHT_PIXELS> MakeAttributeRowOffsets() {
    std::array<uint16_t, SCREEN_HEIGHT_PIXELS> offsets{};
    for (uint16_t y = 0; y < SCREEN_HEIGHT_PIXELS; ++y) {
        offsets[y] = ATTRIBUTE_DATA_OFFSET + ((y / CHAR_HEIGHT) * SCREEN_WIDTH_CHARS);
    }
    return offsets;
}

constexpr auto pixelRowOffsets = MakePixelRowOffsets();
constexpr auto attributeRowOffsets = MakeAttributeRowOffsets();

}

// ---- Black (Same for normal and bright) ----
#define ZX_BLACK_RGB565             0x0000

// ---- Normal / Dim Colors (approx. 80% / 0xCD intensity) ----
#define ZX_BLUE_RGB565              0x1900
#define ZX_RED_RGB565               0x00C8
#define ZX_MAGENTA_RGB565           0x19C8
#define ZX_GREEN_RGB565             0x6006
#define ZX_CYAN_RGB565              0x7906
#define ZX_YELLOW_RGB565            0x60CE
#define ZX_WHITE_RGB565             0x79CE  // Often referred to as light grey

// ---- Bright Colors (100% / 0xFF intensity) ----
#define ZX_BRIGHT_BLUE_RGB565       0x1F00
#define ZX_BRIGHT_RED_RGB565        0x00F8
#define ZX_BRIGHT_MAGENTA_RGB565    0x1FF8
#define ZX_BRIGHT_GREEN_RGB565      0xE007
#define ZX_BRIGHT_CYAN_RGB565       0xFF07
#define ZX_BRIGHT_YELLOW_RGB565     0xE0FF
#define ZX_BRIGHT_WHITE_RGB565      0xFFFF

#define X2(x) (((uint32_t)x << 16) | (uint32_t)x)

#define PALETTE_INDEX_FOREGROUND(attribute) ((attribute & ZX_ATTR_INK_MASK) \
    | ((attribute & ZX_ATTR_BRIGHT_MASK) >> 3))

#define PALETTE_INDEX_BACKGROUND(attribute) (((attribute & ZX_ATTR_PAPER_MASK) >> ZX_ATTR_PAPER_SHIFT) \
    | ((attribute & ZX_ATTR_BRIGHT_MASK) >> 3))

const uint32_t WORDS_PER_PIXEL = sizeof(uint32_t) / sizeof(RGB565);

const uint32_t zx_spectrum_palette_x2[16] = {
    X2(ZX_BLACK_RGB565),
    X2(ZX_BLUE_RGB565),
    X2(ZX_RED_RGB565),
    X2(ZX_MAGENTA_RGB565),
    X2(ZX_GREEN_RGB565),
    X2(ZX_CYAN_RGB565),
    X2(ZX_YELLOW_RGB565),
    X2(ZX_WHITE_RGB565),
    X2(ZX_BLACK_RGB565),
    X2(ZX_BRIGHT_BLUE_RGB565),
    X2(ZX_BRIGHT_RED_RGB565),
    X2(ZX_BRIGHT_MAGENTA_RGB565),
    X2(ZX_BRIGHT_GREEN_RGB565),
    X2(ZX_BRIGHT_CYAN_RGB565),
    X2(ZX_BRIGHT_YELLOW_RGB565),
    X2(ZX_BRIGHT_WHITE_RGB565)
};

const uint16_t zx_spectrum_palette[8] = {
    ZX_BLACK_RGB565,
    ZX_BLUE_RGB565,
    ZX_RED_RGB565,
    ZX_MAGENTA_RGB565,
    ZX_GREEN_RGB565,
    ZX_CYAN_RGB565,
    ZX_YELLOW_RGB565,
    ZX_WHITE_RGB565
};

const uint16_t zx_spectrum_bright_palette[8] = {
    ZX_BLACK_RGB565,
    ZX_BRIGHT_BLUE_RGB565,
    ZX_BRIGHT_RED_RGB565,
    ZX_BRIGHT_MAGENTA_RGB565,
    ZX_BRIGHT_GREEN_RGB565,
    ZX_BRIGHT_CYAN_RGB565,
    ZX_BRIGHT_YELLOW_RGB565,
    ZX_BRIGHT_WHITE_RGB565
};

static uint32_t zx_bitbit_masks[4] = {
  0x00000000,
  0xFFFF0000,
  0x0000FFFF,
  0xFFFFFFFF,
};

void ZxSpectrumDisplayContext::Cls() {
    Cls(ink | paper);
}

void ZxSpectrumDisplayContext::Cls(ZxSpectrumAttribute attribute) {
    std::memset(&screen[0], 0, PIXEL_DATA_SIZE);
    std::memset(&screen[0] + ATTRIBUTE_DATA_OFFSET, attribute, ATTRIBUTE_DATA_SIZE);
}

void ZxSpectrumDisplayContext::Cls(Ink ink, Paper paper) {
    Cls(ink | paper);
}

void ZxSpectrumDisplayContext::DrawText(std::string text, int x, int y) {
    DrawText(text, ink | paper | bright | flash, x, y);
}

void ZxSpectrumDisplayContext::DrawText(std::string text, ZxSpectrumAttribute attribute, int x, int y) {
    for (auto i = 0; i < text.size(); ++i) {
        DrawCharacter(text[i], attribute, x + i, y);
    }
}

void ZxSpectrumDisplayContext::DrawCharacter(char character, ZxSpectrumAttribute attribute, int x, int y) {
    const int offset = static_cast<uint8_t>(character - 0x20) * 8;

    int address = pixelRowOffsets[y * CHAR_HEIGHT] + x;
    screen[attributeRowOffsets[y * CHAR_HEIGHT] + x] = attribute;
    for (auto row = 0; row <= 7; ++row) {
        screen[address] = ZxScreenFont[offset + row];
        address += 0x100;
    }
}

void ZxSpectrumDisplayContext::ScrollUp(ZxSpectrumAttribute attribute, int lines) {
    for (auto y = 192 - lines * 8; y < SCREEN_HEIGHT_PIXELS - 8; y++) {
        for (auto x = 0; x < SCREEN_WIDTH_CHARS; x++) {
            screen[pixelRowOffsets[y] + x] = screen[pixelRowOffsets[y + 8] + x];
            if (y % 7 == 0) {
                screen[attributeRowOffsets[y] + x] = screen[attributeRowOffsets[y + 8] + x];
            }
        }
    }
    DrawText(std::string(32, ' '), attribute, 0, 23);
}

void ZxSpectrumDisplayContext::ScrollScreen() {
    for (auto y = 0; y < SCREEN_HEIGHT_PIXELS - 8; y++) {
        for (auto x = 0; x < SCREEN_WIDTH_CHARS; x++) {
            screen[pixelRowOffsets[y] + x] = screen[pixelRowOffsets[y + 8] + x];
            if (y % 7 == 0) {
                screen[attributeRowOffsets[y] + x] = screen[attributeRowOffsets[y + 8] + x];
            }
        }
    }
    DrawText(std::string(32, ' '), 0, 23);
}


void ZxSpectrumDisplayContext::GetFrame(ColorFormat colorFormat, GetFrameCallbackAction callback) {
    const int bufferLength = SCREEN_TOTAL_WIDTH_PIXELS / WORDS_PER_PIXEL;

    uint32_t rowBuffer[2][bufferLength];
    int bufferIndex = 0;

    this->frame++;
    bool flash = GetFlashState();

    for(auto i = 0; i < SCREEN_TOTAL_HEIGHT_PIXELS; i++) {
        GetScanline(i,  flash, rowBuffer[bufferIndex]);
        callback(&rowBuffer[bufferIndex], bufferLength * sizeof(uint32_t));
        bufferIndex ^= 1;
    }

    callback(nullptr, 0);
}

void ZxSpectrumDisplayContext::GetScanline(uint32_t y, bool flashInvert, uint32_t* rowBuffer) {
    const uint32_t scanLineWords = SCREEN_TOTAL_WIDTH_PIXELS / WORDS_PER_PIXEL;
    const uint32_t leftRightBorderWords = BORDER_WIDTH_LEFT_RIGHT / WORDS_PER_PIXEL;

    if (y < BORDER_HEIGHT_TOP_BOTTOM || y >= (BORDER_HEIGHT_TOP_BOTTOM + SCREEN_HEIGHT_PIXELS)) {
        for (auto i = 0; i < scanLineWords; i++) {
            *rowBuffer++ = zx_spectrum_palette_x2[borderByte];
        }
    }
    else {
        for (auto i = 0; i < leftRightBorderWords; ++i) {
            *rowBuffer++ = zx_spectrum_palette_x2[borderByte];
        }
        
        const uint32_t pixelRow = y - 24;
        uint32_t pixelOffset = pixelRowOffsets[pixelRow];
        uint32_t attributeOffset = attributeRowOffsets[pixelRow];
        //const int m = (frame >> 5) & 1;   
        
        for (auto i = 0; i < SCREEN_WIDTH_CHARS; ++i) {
            uint8_t attribute = this->screen[attributeOffset++];
            uint8_t pixels = flashInvert && (attribute & ZX_ATTR_FLASH_MASK) ? this->screen[pixelOffset++] ^ 0xFF : this->screen[pixelOffset++];
            uint8_t paperIndex = PALETTE_INDEX_BACKGROUND(attribute);
            uint8_t inkIndex = PALETTE_INDEX_FOREGROUND(attribute);
            uint32_t bcw = zx_spectrum_palette_x2[paperIndex]; // The background colour word
            uint32_t fcw = zx_spectrum_palette_x2[inkIndex]; // The foreground colour word
            uint32_t fgm;
            uint32_t bgm;
            fgm = zx_bitbit_masks[(pixels >> 6) & 3];
            bgm = ~fgm;
            *rowBuffer++ = (fgm & fcw) | (bgm & bcw);
            fgm = zx_bitbit_masks[(pixels >> 4) & 3];
            bgm = ~fgm;
            *rowBuffer++ = (fgm & fcw) | (bgm & bcw);
            fgm = zx_bitbit_masks[(pixels >> 2) & 3];
            bgm = ~fgm;
            *rowBuffer++ = (fgm & fcw) | (bgm & bcw);
            fgm = zx_bitbit_masks[pixels & 3];
            bgm = ~fgm;
            *rowBuffer++ = (fgm & fcw) | (bgm & bcw);
        }
        
        for (int i = 0; i < leftRightBorderWords; ++i){
            *rowBuffer++ = zx_spectrum_palette_x2[borderByte];
        }
    }
}
