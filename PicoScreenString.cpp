#include <cstring>
#include <functional>

#include "pico/rand.h"

#include "PicoScreenString.hpp"

#include "Display.hpp"
#include "ScreenString.hpp"
#include "screens.h"

static void InitializeShuffleIndices(std::vector<int>& shuffleIndices, size_t size) {
    shuffleIndices.clear();
    for (size_t i = 0; i < size; ++i) {
        shuffleIndices.push_back(static_cast<int>(i));
    }
}

void PicoScreenString::Initialize() {
    display->SetButtonEventHandler(std::bind_front(&PicoScreenString::HandleButtonEvent, this));

    AboutData.Decompress(aboutContext.GetBuffer());

    configContext.SetBorder(ZxSpectrumColor::Blue);
    configContext.SetPaper(Paper::Blue);
    configContext.SetInk(Ink::Yellow);
    configContext.Cls();
    configContext.DrawText("Configuration", 10, 1);

    InitializeShuffleIndices(shuffleIndices, shuffleScreenStrings->size());

    ShuffleScreenStrings();

    MakeTransition(currentState, State::Initial);
}

void PicoScreenString::Run()
{
    uint64_t thisFrame, lastFrame;

    lastFrame = time_us_64();
    while (true) {
        switch (currentState)
        {
        case State::Initial:
            switch (options.Mode) {
                case DisplayMode::Sequence:
                    nextState = State::Sequence;
                    break;
                case DisplayMode::Shuffle:
                    nextState = State::Shuffle;
                    break;
                default:
                    nextState = State::Sequence;
                    break;
            }
            break;
        case State::Sequence:
            nextState = State::Loading;
            break;
        case State::Shuffle:
            nextState = State::Loading;
            break;
        case State::Loading:
            loader.Frame();
            if (!loader.InProgress()) {
                nextState = previousState;
            }
            break;
        case State::Configuration:
            /* code */
            break;
        case State::About:
            /* code */
            break;
        case State::Test:
            if (!loader.InProgress()) {
                loader.TestReset();
            }
            loader.TestResetFrame();
            break;
        default:
            break;
        }

        display->PollButtons();

        if (nextState != State::None) {
            MakeTransition(currentState, nextState);
        }

        while ((thisFrame = time_us_64()) - lastFrame < 16667) {
            sleep_us(1);
        }

        lastFrame = thisFrame;
    }
}

void PicoScreenString::MakeTransition(State from, State to)
{
    OnStateChanging(from, to);
    OnLeaveState(from);
    OnEnterState(to);
    OnStateChanged(from, to);
    previousState = from;
    currentState = to;
    nextState = State::None;
}

void PicoScreenString::OnStateChanging(State from, State to)
{
}

void PicoScreenString::OnEnterState(State state)
{
    switch (state)
    {
        case State::Sequence:
            printf("Entering Sequence state\n");
            activeContext = &loaderContext;
            LoadNextSequenceScreenString();
            break;
        case State::Shuffle:
            printf("Entering Shuffle state\n");
            activeContext = &loaderContext;
            LoadNextShuffleScreenString();
            break;
        case State::Loading:
            printf("Entering Loading state\n");
            activeContext = &loaderContext;
            break;
        case State::Configuration:
            printf("Entering Configuration state\n");
            activeContext = &configContext;
            break;
        case State::About:
            printf("Entering About state\n");
            activeContext = &aboutContext;
            break;
        case State::Test:
            printf("Entering Test state\n");
            activeContext = &loaderContext;
            break;
        default:
            printf("This should never happen!!\n");
            activeContext = nullptr;
            break;
    }
}

void PicoScreenString::OnLeaveState(State state) {
}

void PicoScreenString::OnStateChanged(State from, State to) {
}

void PicoScreenString::HandleButtonEvent(const PicoDisplay28X::ButtonEventArgs& args) {
    if (currentState == State::Initial) {
        if (args.ButtonAState == PicoDisplay28X::ButtonState::Pressed) {
            nextState = State::Test;
        }
        return;
    }
    if (currentState == State::Loading) {
        // Handle button events specific to the Loading state here
        if (args.ButtonAState == PicoDisplay28X::ButtonState::Held && 
            args.ButtonXState == PicoDisplay28X::ButtonState::Held) {
            // Handle the specific combination of Button A and Button X being held
            nextState = State::Configuration;
            return;
        }
        if (args.ButtonBState == PicoDisplay28X::ButtonState::Held && 
            args.ButtonYState == PicoDisplay28X::ButtonState::Held) {
            // Handle the specific combination of Button B and Button Y being held
            nextState = State::About;
            return;
        }
        if (args.ButtonXState == PicoDisplay28X::ButtonState::Clicked) {
            // Complete the current loading operation and go to the previous screen in the current mode
            loader.Finish();
            switch (previousState) {
                case State::Sequence:
                    if (nextSequenceIndex == 1) {
                        // We're currently loading the first screen in the sequence, so we want to go back to the last shuffle
                        // screen and load that
                        nextSequenceIndex = sequenceScreenStrings->size();
                        nextShuffleIndex = nextShuffleIndex > 0 ? nextShuffleIndex - 1 : shuffleScreenStrings->size() - 1;
                        return;
                    }
                    if (nextSequenceIndex == 0) {
                        // We're showing the screen from the shuffle, so we're going back to the last sequence item
                        nextSequenceIndex = sequenceScreenStrings->size() - 1;
                        nextShuffleIndex = nextShuffleIndex > 0 ? nextShuffleIndex - 1 : shuffleScreenStrings->size() - 1;
                        return;
                    }
                    nextSequenceIndex--;
                    break;
                case State::Shuffle:
                    if (nextShuffleIndex == 0) {
                        // We're showing the first item in the shuffle, so we're going back to the last shuffle item
                        nextShuffleIndex = shuffleScreenStrings->size() - 1;
                    }
                    else {
                        nextShuffleIndex--;
                    }
                    break;
                default:
                    break;
            }
        }
        if (args.ButtonYState == PicoDisplay28X::ButtonState::Clicked) {
            // Complete the current loading operation
            loader.Finish();
        }
    }

    if (currentState == State::Configuration) {
        // Handle button events specific to the Configuration state here
        if (args.ButtonAState == PicoDisplay28X::ButtonState::Held && 
            args.ButtonXState == PicoDisplay28X::ButtonState::Held) {
            // Handle the specific combination of Button A and Button X being held
            nextState = previousState;
            return;
        }
    }

    if (currentState == State::About) {
        // We don't care what the button actions are in the About state; 
        // any button press will return to the previous state
        nextState = previousState;
        return;
    }

    // If none of the above state-specific conditions are met, handle general button events here

    if (args.ButtonAState == PicoDisplay28X::ButtonState::Clicked) {
        IncreaseBacklight();
    }
    if (args.ButtonBState == PicoDisplay28X::ButtonState::Clicked) {
        DecreaseBacklight();
    }
}

void PicoScreenString::IncreaseBacklight() {
    uint8_t backlight = display->GetBacklight();
    if (backlight < 255) {
        backlight += 0x10;
        display->SetBacklight(backlight);
    }
}

void PicoScreenString::DecreaseBacklight() {
    uint8_t backlight = display->GetBacklight();
    if (backlight > 0) {
        backlight -= 0x10;
        display->SetBacklight(backlight);
    }
}

void PicoScreenString::ShuffleScreenStrings() {
    // Simple Fisher-Yates shuffle
    for (size_t i = shuffleIndices.size() - 1; i > 0; --i) {
        size_t j = get_rand_32() % (i + 1);
        std::swap(shuffleIndices[i], shuffleIndices[j]);
    }

    nextShuffleIndex = 0;
}

void PicoScreenString::LoadNextSequenceScreenString() {
    if (nextSequenceIndex >= static_cast<int>(sequenceScreenStrings->size())) {
        nextSequenceIndex = 0;
        if (options.ShowScreenFromShuffleAtEndOfSequence) {
            LoadNextShuffleScreenString();
            return;
        }
    }

    loader.NewScreen(
        &(*sequenceScreenStrings)[nextSequenceIndex], 
        options.ClearScreenBetweenScreens, 
        options.ResetBeforeSequence && nextSequenceIndex == 0
    );
    ++nextSequenceIndex;
}

void PicoScreenString::LoadNextShuffleScreenString() {
    if (nextShuffleIndex >= static_cast<int>(shuffleIndices.size())) {
        ShuffleScreenStrings();
        nextShuffleIndex = 0;
    }

    loader.NewScreen(
        &(*shuffleScreenStrings)[shuffleIndices[nextShuffleIndex]],
        options.ClearScreenBetweenScreens,
        options.ResetBeforeSequence && options.Mode == DisplayMode::Shuffle && nextShuffleIndex == 0
    );
    ++nextShuffleIndex;
}
