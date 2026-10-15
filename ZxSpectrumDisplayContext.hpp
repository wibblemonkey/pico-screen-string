#ifndef ZX_SPECTRUM_SCREEN_HPP
#define ZX_SPECTRUM_SCREEN_HPP

#include <stdint.h>
#include <string>

#include "Display.hpp"
#include "ZxSpectrumColors.hpp"

#define CHAR_HEIGHT 8
#define CHAR_WIDTH 8

#define SCREEN_WIDTH_CHARS 32
#define SCREEN_HEIGHT_CHARS 24

#define SCREEN_WIDTH_PIXELS (SCREEN_WIDTH_CHARS * CHAR_WIDTH)
#define SCREEN_HEIGHT_PIXELS (SCREEN_HEIGHT_CHARS * CHAR_HEIGHT)

#define PIXEL_DATA_SIZE (SCREEN_WIDTH_CHARS * SCREEN_HEIGHT_PIXELS)
#define ATTRIBUTE_DATA_OFFSET PIXEL_DATA_SIZE
#define ATTRIBUTE_DATA_SIZE (SCREEN_WIDTH_CHARS * SCREEN_HEIGHT_CHARS)
#define SCREEN_DATA_SIZE (PIXEL_DATA_SIZE + ATTRIBUTE_DATA_SIZE)

#define BORDER_HEIGHT_TOP_BOTTOM 24
#define BORDER_WIDTH_LEFT_RIGHT 32

#define BORDER_HEIGHT_TOTAL (BORDER_HEIGHT_TOP_BOTTOM * 2)
#define BORDER_WIDTH_TOTAL (BORDER_WIDTH_LEFT_RIGHT * 2)

#define SCREEN_TOTAL_WIDTH_PIXELS (SCREEN_WIDTH_PIXELS + BORDER_WIDTH_TOTAL)
#define SCREEN_TOTAL_HEIGHT_PIXELS (SCREEN_HEIGHT_PIXELS + BORDER_HEIGHT_TOTAL)

// enum class ZxSpectrumColor : uint8_t {
//     BLACK,
//     BLUE,
//     RED,
//     MAGENTA,
//     GREEN,
//     CYAN,
//     YELLOW,
//     WHITE
// };

class ZxSpectrumDisplayContext : public DisplayContext {
    private:
        uint8_t screen[SCREEN_DATA_SIZE];
        ZxSpectrumColor border;
        uint8_t borderByte;
        Ink ink;
        Paper paper;
        Bright bright;
        Flash flash;
        uint32_t frame;
        bool flashState;

    public:
        ZxSpectrumDisplayContext()
            : DisplayContext(SCREEN_TOTAL_WIDTH_PIXELS, SCREEN_TOTAL_HEIGHT_PIXELS, ColorFormat::RGB565),
                border(ZxSpectrumColor::White),
                paper(ZxSpectrumColor::White),
                ink(ZxSpectrumColor::Black),
                bright(false),
                flash(false),
                frame(0),
                flashState(false) {}

        void Cls();
        void Cls(ZxSpectrumAttribute attribute);
        void Cls(Ink ink, Paper paper);

        inline uint8_t* GetBuffer() { return &screen[0]; }

        ZxSpectrumColor GetBorder() { return border; }
        void SetBorder(ZxSpectrumColor color) { border = color; borderByte = static_cast<uint8_t>(color); }

        Ink GetInk() { return ink; }
        void SetInk(Ink color) { ink = color; }

        Paper GetPaper() { return paper; }
        void SetPaper(Paper color) { paper = color; }

        bool GetBright() { return bright; }
        void SetBright(bool value) { bright = value; }

        bool GetFlash() { return flash; }
        void SetFlash(bool value) { flash = value; }

        bool GetFlashState() {
            if (frame % 16 == 0) {
                flashState = !flashState;
            }
            return flashState;
        }

        void DrawText(std::string text, int x, int y);
        void DrawText(std::string text, ZxSpectrumAttribute attribute, int x, int y);
        void DrawCharacter(char character, ZxSpectrumAttribute attribute, int x, int y);

        void ScrollUp(ZxSpectrumAttribute attribute, int lines);
        void ScrollScreen();

        virtual void GetFrame(ColorFormat colorFormat, GetFrameCallbackAction callback) override;
        void GetScanline(uint32_t y, bool flashInvert, uint32_t* rowBuffer);
};

#endif // ZX_SPECTRUM_SCREEN_HPP
