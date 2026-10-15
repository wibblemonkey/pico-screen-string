#ifndef PICO_SCREEN_STRING_OPTIONS_HPP
#define PICO_SCREEN_STRING_OPTIONS_HPP

#include <cstdint>

enum THardware { h16k, h48k, h128k, hPlus2, hPlus2a, hPlus3, h128kUsr0, hPlus2Usr0, hPlus2aUsr0, hPlus3Usr0 };
enum TProgrammer { pgmNone, pgmProgram, pgmDirectCommand };
enum T48kMode { mdRandom, mdNone, md48BASIC, mdSPECTRUM, mdUSR0 };

struct EnvironmentInfo {
    THardware HardwareModel;
    TProgrammer Programmer;
    T48kMode Mode48k;
    int32_t ResetMethod;
    int32_t CLS;
    int32_t Looped;
    bool TapeWobble;
    bool TapeHiss;
    bool RandomHWAfterReset;
    int32_t PauseLen;
    bool LongFilenames;
    bool Chuntey;
};

enum class DisplayMode {
    Sequence,
    Shuffle,
    Test,
};

struct PicoScreenStringOptions {
    DisplayMode Mode;
    bool ShowScreenFromShuffleAtEndOfSequence;
    bool ResetBeforeSequence;
    bool ClearScreenBetweenScreens;
};

#endif // PICO_SCREEN_STRING_OPTIONS_HPP
