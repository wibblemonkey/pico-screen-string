#include "PicoDisplay28.hpp"

PicoDisplay28X::PicoDisplay28X() {
    display.Initialize();
    led.set_rgb(0, 0, 0);
    SetBacklight(0xFF);
}

bool PicoDisplay28X::UpdateButtonState(Button button, ButtonState& state, uint64_t lastChange, uint64_t currentTime) {
    bool isDown = button.raw();
    switch(state) {
        case ButtonState::None:
            if(isDown) {
                state = ButtonState::Pressed;
                return true;
            }
            break;
        case ButtonState::Pressed:
            if(!isDown) {
                state = ButtonState::Clicked;
                return true;
            }
            else if(currentTime - lastChange >= 500000) { // 500ms for held state
                state = ButtonState::Held;
                return true;
            }
            break;
        case ButtonState::Clicked:
        case ButtonState::Held:
            if(!isDown) {
                state = ButtonState::None;
                return true;
            }
            break;
    }
    return false;
}

void PicoDisplay28X::PollButtons() {
    bool buttonAChanged = false;
    static uint64_t lastButtonAChange = 0;
    bool buttonBChanged = false;
    static uint64_t lastButtonBChange = 0;
    bool buttonXChanged = false;
    static uint64_t lastButtonXChange = 0;
    bool buttonYChanged = false;
    static uint64_t lastButtonYChange = 0;
    
    static uint64_t lastPollTime = time_us_64();
    uint64_t currentTime = time_us_64();

    if (buttonAChanged = UpdateButtonState(buttonA, buttons.ButtonAState, lastButtonAChange, currentTime)) {
        lastButtonAChange = currentTime;
    }
    if (buttonBChanged = UpdateButtonState(buttonB, buttons.ButtonBState, lastButtonBChange, currentTime)) {
        lastButtonBChange = currentTime;
    }
    if (buttonXChanged = UpdateButtonState(buttonX, buttons.ButtonXState, lastButtonXChange, currentTime)) {
        lastButtonXChange = currentTime;
    }
    if (buttonYChanged = UpdateButtonState(buttonY, buttons.ButtonYState, lastButtonYChange, currentTime)) {
        lastButtonYChange = currentTime;
    }

    if (buttonEventHandler &&
        ((buttonAChanged && buttons.ButtonAState != ButtonState::None) ||
         (buttonBChanged && buttons.ButtonBState != ButtonState::None) ||
         (buttonXChanged && buttons.ButtonXState != ButtonState::None) ||
         (buttonYChanged && buttons.ButtonYState != ButtonState::None))) {
            buttonEventHandler(buttons);
    }

    lastPollTime = currentTime;
}
