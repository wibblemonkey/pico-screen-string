#ifndef PICO_DISPLAY_28_HPP
#define PICO_DISPLAY_28_HPP

#include <cstdint>
#include <functional>

#include "rgbled.hpp"
#include "button.hpp"

#include "Display.hpp"

using RGBLED = pimoroni::RGBLED;
using Button = pimoroni::Button;
using Rotation = pimoroni::Rotation;

class PicoDisplay28X {
public:
    static const int WIDTH = 320;
    static const int HEIGHT = 240;
    static const uint8_t A = 12;
    static const uint8_t B = 13;
    static const uint8_t X = 14;
    static const uint8_t Y = 15;
    static const uint8_t LED_R = 26;
    static const uint8_t LED_G = 27;
    static const uint8_t LED_B = 28;

    enum class ButtonState : uint8_t {
        None,
        Pressed,
        Clicked,
        Held
    };

    typedef struct {
        ButtonState ButtonAState;
        ButtonState ButtonBState;
        ButtonState ButtonXState;
        ButtonState ButtonYState;
    } ButtonEventArgs;

    typedef std::function<void(const ButtonEventArgs&)> ButtonEventHandler;

private:
    ButtonEventArgs buttons = {
        .ButtonAState = ButtonState::None,
        .ButtonBState = ButtonState::None,
        .ButtonXState = ButtonState::None,
        .ButtonYState = ButtonState::None
    };

    RGBLED led = RGBLED(PicoDisplay28X::LED_R, PicoDisplay28X::LED_G, PicoDisplay28X::LED_B);

    Button buttonA = Button(PicoDisplay28X::A);
    Button buttonB = Button(PicoDisplay28X::B);
    Button buttonX = Button(PicoDisplay28X::X);
    Button buttonY = Button(PicoDisplay28X::Y);

    ST7789 display = ST7789(PicoDisplay28X::WIDTH, PicoDisplay28X::HEIGHT, Rotation::ROTATE_0, ColorFormat::RGB565);

    ButtonEventHandler buttonEventHandler;

    uint8_t brightness = 0;

public:
    PicoDisplay28X();

    inline uint8_t GetBacklight() const { return brightness; };
    inline void SetBacklight(uint8_t brightness) { this->brightness = brightness; display.SetBacklight(brightness); };
    inline void SetLEDColor(uint8_t r, uint8_t g, uint8_t b) { led.set_rgb(r, g, b); };
    inline void UpdateDisplay(DisplayContext* context) { display.Update(context); };

    inline void SetButtonEventHandler(ButtonEventHandler handler) { buttonEventHandler = handler; };
    void PollButtons();
    bool UpdateButtonState(Button button, ButtonState& state, uint64_t lastChange, uint64_t current_time);
};

#endif // PICO_DISPLAY_28_HPP
