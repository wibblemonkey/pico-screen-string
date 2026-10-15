#ifndef PICOSCREENSTRING_HPP
#define PICOSCREENSTRING_HPP

#include "PicoDisplay28.hpp"
#include "ZxSpectrumDisplayContext.hpp"
#include "ScreenStringLoader.hpp"

class PicoScreenString
{
private:
    enum class State {
        None,
        Initial,
        Sequence,
        Shuffle,
        Loading,
        Configuration,
        About,
        Test,
    };

    PicoDisplay28X* display;
    ZxSpectrumDisplayContext* activeContext = nullptr;

    ZxSpectrumDisplayContext loaderContext = ZxSpectrumDisplayContext();
    ZxSpectrumDisplayContext configContext = ZxSpectrumDisplayContext();
    ZxSpectrumDisplayContext aboutContext = ZxSpectrumDisplayContext();

    ScreenStringLoader loader = ScreenStringLoader(&loaderContext);

    const std::vector<ScreenString>* sequenceScreenStrings;
    const std::vector<ScreenString>* shuffleScreenStrings;
    std::vector<int> shuffleIndices;
    int nextShuffleIndex = 0;
    int nextSequenceIndex = 0;

    State previousState = State::None;
    State currentState = State::None;
    State nextState = State::None;

    PicoScreenStringOptions options = {
        .Mode = DisplayMode::Sequence,
        .ShowScreenFromShuffleAtEndOfSequence = true,
        .ResetBeforeSequence = true,
        .ClearScreenBetweenScreens = true,
    };

    void MakeTransition(State from, State to);

    void OnStateChanging(State from, State to);
    void OnEnterState(State state);
    void OnLeaveState(State state);
    void OnStateChanged(State from, State to);

    void HandleButtonEvent(const PicoDisplay28X::ButtonEventArgs& args);

    void IncreaseBacklight();
    void DecreaseBacklight();

    void ShuffleScreenStrings();
    void LoadNextSequenceScreenString();
    void LoadNextShuffleScreenString();

public:
    PicoScreenString(
        PicoDisplay28X* display, 
        const std::vector<ScreenString>* sequenceScreenStrings, 
        const std::vector<ScreenString>* shuffleScreenStrings
    ) : display(display), 
        sequenceScreenStrings(sequenceScreenStrings), 
        shuffleScreenStrings(shuffleScreenStrings)
    { }
    ~PicoScreenString() { }

    void Initialize();
    void Run();

    inline void UpdateDisplay() { if (activeContext) display->UpdateDisplay(activeContext); };
};

#endif // PICOSCREENSTRING_HPP