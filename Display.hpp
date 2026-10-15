#pragma once

#include <cstdint>
#include <functional>
#include <math.h>
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#include "common/pimoroni_common.hpp"

typedef uint8_t RGB332;
typedef uint16_t RGB565;
typedef uint16_t RGB555;
typedef uint32_t RGB888;

struct RGB {
    int16_t r, g, b;

    constexpr RGB() : r(0), g(0), b(0) {}
    constexpr RGB(RGB332 c) :
        r((c & 0b11100000) >> 0),
        g((c & 0b00011100) << 3),
        b((c & 0b00000011) << 6) {}
    constexpr RGB(RGB565 c) :
        r((__builtin_bswap16(c) & 0b1111100000000000) >> 8),
        g((__builtin_bswap16(c) & 0b0000011111100000) >> 3),
        b((__builtin_bswap16(c) & 0b0000000000011111) << 3) {}
    constexpr RGB(uint c) :
        r((c >> 16) & 0xff),
        g((c >> 8) & 0xff),
        b(c & 0xff) {}
    constexpr RGB(int16_t r, int16_t g, int16_t b) : r(r), g(g), b(b) {}

    constexpr uint8_t blend(uint8_t s, uint8_t d, uint8_t a) {
        return d + ((a * (s - d) + 127) >> 8);
    }

    constexpr RGB blend(RGB with, const uint8_t alpha) {
        return RGB(
        blend(with.r, r, alpha),
        blend(with.g, g, alpha),
        blend(with.b, b, alpha)
        );
    }

    static RGB from_hsv(float h, float s, float v) {
        float i = floor(h * 6.0f);
        float f = h * 6.0f - i;
        v *= 255.0f;
        uint8_t p = v * (1.0f - s);
        uint8_t q = v * (1.0f - f * s);
        uint8_t t = v * (1.0f - (1.0f - f) * s);

        switch (int(i) % 6) {
        case 0: return RGB(v, t, p);
        case 1: return RGB(q, v, p);
        case 2: return RGB(p, v, t);
        case 3: return RGB(p, q, v);
        case 4: return RGB(t, p, v);
        case 5: return RGB(v, p, q);
        default: return RGB(0, 0, 0);
        }
    }

    constexpr operator bool() {return r || g || b;};
    constexpr RGB  operator+ (const RGB& c) const {return RGB(r + c.r, g + c.g, b + c.b);}
    constexpr RGB& operator+=(const RGB& c) {r += c.r; g += c.g; b += c.b; return *this;}
    constexpr RGB& operator-=(const RGB& c) {r -= c.r; g -= c.g; b -= c.b; return *this;}
    constexpr RGB  operator- (const RGB& c) const {return RGB(r - c.r, g - c.g, b - c.b);}

    // a rough approximation of how bright a colour is used to compare the
    // relative brightness of two colours
    int luminance() const {
        // weights based on https://www.johndcook.com/blog/2009/08/24/algorithms-convert-color-grayscale/
        return r * 21 + g * 72 + b * 7;
    }

    // a relatively low cost approximation of how "different" two colours are
    // perceived which avoids expensive colour space conversions.
    // described in detail at https://www.compuphase.com/cmetric.htm
    int distance(const RGB& c) const {
        int rmean = (r + c.r) / 2;
        int rx = r - c.r;
        int gx = g - c.g;
        int bx = b - c.b;
        return abs((int)(
        (((512 + rmean) * rx * rx) >> 8) + 4 * gx * gx + (((767 - rmean) * bx * bx) >> 8)
        ));
    }

    int closest(const RGB *palette, size_t len) const {
        int d = INT_MAX, m = -1;
        for(size_t i = 0; i < len; i++) {
        int dc = distance(palette[i]);
        if(dc < d) {m = i; d = dc;}
        }
        return m;
    }

    constexpr RGB565 to_rgb565() {
        uint16_t p = ((r & 0b11111000) << 8) |
                    ((g & 0b11111100) << 3) |
                    ((b & 0b11111000) >> 3);

        return __builtin_bswap16(p);
    }

    constexpr RGB555 to_rgb555() {
        uint16_t p = ((r & 0b11111000) << 7) |
                    ((g & 0b11111000) << 2) |
                    ((b & 0b11111000) >> 3);

        return p;
    }

    constexpr RGB565 to_rgb332() {
        return (r & 0b11100000) | ((g & 0b11100000) >> 3) | ((b & 0b11000000) >> 6);
    }

    constexpr RGB888 to_rgb888() {
        return (r << 16) | (g << 8) | (b << 0);
    }
};

struct Rect;

struct Point {
    int32_t x = 0, y = 0;

    Point() = default;
    Point(int32_t x, int32_t y) : x(x), y(y) {}

    inline Point& operator-= (const Point &a) { x -= a.x; y -= a.y; return *this; }
    inline Point& operator+= (const Point &a) { x += a.x; y += a.y; return *this; }
    inline Point& operator/= (const int32_t a) { x /= a;   y /= a;  return *this; }

    Point clamp(const Rect &r) const;
};

inline bool operator== (const Point &lhs, const Point &rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }
inline bool operator!= (const Point &lhs, const Point &rhs) { return !(lhs == rhs); }
inline Point operator-  (Point lhs, const Point &rhs) { lhs -= rhs; return lhs; }
inline Point operator-  (const Point &rhs) { return Point(-rhs.x, -rhs.y); }
inline Point operator+  (Point lhs, const Point &rhs) { lhs += rhs; return lhs; }
inline Point operator/  (Point lhs, const int32_t a) { lhs /= a; return lhs; }

struct Rect {
    int32_t x = 0, y = 0, w = 0, h = 0;

    Rect() = default;
    Rect(int32_t x, int32_t y, int32_t w, int32_t h) : x(x), y(y), w(w), h(h) {}
    Rect(const Point &tl, const Point &br) : x(tl.x), y(tl.y), w(br.x - tl.x), h(br.y - tl.y) {}

    bool empty() const;
    bool contains(const Point &p) const;
    bool contains(const Rect &p) const;
    bool intersects(const Rect &r) const;
    Rect intersection(const Rect &r) const;

    void inflate(int32_t v);
    void deflate(int32_t v);
};

static const RGB565 rgb332_to_rgb565_lut[256] = {
    0x0000, 0x0800, 0x1000, 0x1800, 0x0001, 0x0801, 0x1001, 0x1801, 0x0002, 0x0802, 0x1002, 0x1802, 0x0003, 0x0803, 0x1003, 0x1803,
    0x0004, 0x0804, 0x1004, 0x1804, 0x0005, 0x0805, 0x1005, 0x1805, 0x0006, 0x0806, 0x1006, 0x1806, 0x0007, 0x0807, 0x1007, 0x1807,
    0x0020, 0x0820, 0x1020, 0x1820, 0x0021, 0x0821, 0x1021, 0x1821, 0x0022, 0x0822, 0x1022, 0x1822, 0x0023, 0x0823, 0x1023, 0x1823,
    0x0024, 0x0824, 0x1024, 0x1824, 0x0025, 0x0825, 0x1025, 0x1825, 0x0026, 0x0826, 0x1026, 0x1826, 0x0027, 0x0827, 0x1027, 0x1827,
    0x0040, 0x0840, 0x1040, 0x1840, 0x0041, 0x0841, 0x1041, 0x1841, 0x0042, 0x0842, 0x1042, 0x1842, 0x0043, 0x0843, 0x1043, 0x1843,
    0x0044, 0x0844, 0x1044, 0x1844, 0x0045, 0x0845, 0x1045, 0x1845, 0x0046, 0x0846, 0x1046, 0x1846, 0x0047, 0x0847, 0x1047, 0x1847,
    0x0060, 0x0860, 0x1060, 0x1860, 0x0061, 0x0861, 0x1061, 0x1861, 0x0062, 0x0862, 0x1062, 0x1862, 0x0063, 0x0863, 0x1063, 0x1863,
    0x0064, 0x0864, 0x1064, 0x1864, 0x0065, 0x0865, 0x1065, 0x1865, 0x0066, 0x0866, 0x1066, 0x1866, 0x0067, 0x0867, 0x1067, 0x1867,
    0x0080, 0x0880, 0x1080, 0x1880, 0x0081, 0x0881, 0x1081, 0x1881, 0x0082, 0x0882, 0x1082, 0x1882, 0x0083, 0x0883, 0x1083, 0x1883,
    0x0084, 0x0884, 0x1084, 0x1884, 0x0085, 0x0885, 0x1085, 0x1885, 0x0086, 0x0886, 0x1086, 0x1886, 0x0087, 0x0887, 0x1087, 0x1887,
    0x00a0, 0x08a0, 0x10a0, 0x18a0, 0x00a1, 0x08a1, 0x10a1, 0x18a1, 0x00a2, 0x08a2, 0x10a2, 0x18a2, 0x00a3, 0x08a3, 0x10a3, 0x18a3,
    0x00a4, 0x08a4, 0x10a4, 0x18a4, 0x00a5, 0x08a5, 0x10a5, 0x18a5, 0x00a6, 0x08a6, 0x10a6, 0x18a6, 0x00a7, 0x08a7, 0x10a7, 0x18a7,
    0x00c0, 0x08c0, 0x10c0, 0x18c0, 0x00c1, 0x08c1, 0x10c1, 0x18c1, 0x00c2, 0x08c2, 0x10c2, 0x18c2, 0x00c3, 0x08c3, 0x10c3, 0x18c3,
    0x00c4, 0x08c4, 0x10c4, 0x18c4, 0x00c5, 0x08c5, 0x10c5, 0x18c5, 0x00c6, 0x08c6, 0x10c6, 0x18c6, 0x00c7, 0x08c7, 0x10c7, 0x18c7,
    0x00e0, 0x08e0, 0x10e0, 0x18e0, 0x00e1, 0x08e1, 0x10e1, 0x18e1, 0x00e2, 0x08e2, 0x10e2, 0x18e2, 0x00e3, 0x08e3, 0x10e3, 0x18e3,
    0x00e4, 0x08e4, 0x10e4, 0x18e4, 0x00e5, 0x08e5, 0x10e5, 0x18e5, 0x00e6, 0x08e6, 0x10e6, 0x18e6, 0x00e7, 0x08e7, 0x10e7, 0x18e7,
};

enum class ColorFormat : uint8_t {
    RGB444 = 0x03,
    RGB565 = 0x05,
};

class DisplayContext {
public:
    uint16_t width;
    uint16_t height;
    ColorFormat colorFormat;

    typedef std::function<void(void *data, size_t length)> GetFrameCallbackAction;

    DisplayContext(uint16_t width, uint16_t height, ColorFormat colorFormat)
        : width(width), height(height), colorFormat(colorFormat) {};

    virtual void GetFrame(ColorFormat colorFormat, GetFrameCallbackAction callback);
};

class DisplayDriver {
public:
    DisplayDriver(uint16_t width, uint16_t height, pimoroni::Rotation rotation, ColorFormat colorFormat)
        : width(width), height(height), rotation(rotation), colorFormat(colorFormat) {};

    virtual void Update(DisplayContext *display) {};
    virtual void PartialUpdate(DisplayContext *display, Rect region) {};
    virtual bool SetUpdateSpeed(int update_speed) { return false; };
    virtual void SetBacklight(uint8_t brightness) {};
    virtual bool IsBusy() { return false; };
    virtual void PowerOff() {};
    virtual void Cleanup() {};

protected:
    uint16_t width;
    uint16_t height;
    pimoroni::Rotation rotation;
    ColorFormat colorFormat;
};

enum class Command : uint8_t {
    SWRESET   = 0x01,
    TEOFF     = 0x34,
    TEON      = 0x35,
    MADCTL    = 0x36,
    COLMOD    = 0x3A,
    RAMCTRL   = 0xB0,
    GCTRL     = 0xB7,
    VCOMS     = 0xBB,
    LCMCTRL   = 0xC0,
    VDVVRHEN  = 0xC2,
    VRHS      = 0xC3,
    VDVS      = 0xC4,
    FRCTRL2   = 0xC6,
    PWCTRL1   = 0xD0,
    PORCTRL   = 0xB2,
    GMCTRP1   = 0xE0,
    GMCTRN1   = 0xE1,
    INVOFF    = 0x20,
    SLPOUT    = 0x11,
    DISPON    = 0x29,
    GAMSET    = 0x26,
    DISPOFF   = 0x28,
    RAMWR     = 0x2C,
    INVON     = 0x21,
    CASET     = 0x2A,
    RASET     = 0x2B,
    PWMFRSEL  = 0xCC
};

class ST7789 : public DisplayDriver {
    spi_inst_t *spi = spi0;

private:
    // interface pins with our standard defaults where appropriate
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
    ST7789(uint16_t width, uint16_t height, pimoroni::Rotation rotation, ColorFormat colorFormat) :
        DisplayDriver(width, height, rotation, colorFormat) {};
    
    void Initialize();

    void Cleanup() override;
    void Update(DisplayContext *graphics) override;
    void SetBacklight(uint8_t brightness) override;

private:
    void write_blocking_dma(const uint8_t *src, size_t len);
    void ConfigureDisplay(pimoroni::Rotation rotation);
    void SendCommand(Command command, size_t len = 0, const char *data = NULL);
};
