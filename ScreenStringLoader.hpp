#ifndef SCREEN_STRING_LOADER_HPP
#define SCREEN_STRING_LOADER_HPP

#include <cstdint>
#include <vector>

#include "ZxSpectrumColors.hpp"
#include "ScreenString.hpp"
#include "ZxSpectrumDisplayContext.hpp"
#include "PicoScreenStringOptions.hpp"
#include "ScreenStringProgrammer.hpp"

struct LoaderInfo {
    ZxSpectrumColor Pilot_Border_1;
    ZxSpectrumColor Pilot_Border_2;
    int32_t Pilot_Click_Length;
    int32_t Pilot_Click_Repeats;
    int32_t Pilot_Tone_Length;
    int32_t Pilot_Repeats;
    int32_t Pilot_Loops;
    int32_t Sync1_Length;
    int32_t Sync2_Length;
    ZxSpectrumColor Data_Border_1;
    ZxSpectrumColor Data_Border_2;
    int32_t Data_One_Length;
    int32_t Data_Zero_Length;
    int32_t Data_Length;
    int32_t PreLoad_Delay;
    int32_t PreHeader_Delay;
    int32_t Data_Pause_Length;
    ZxSpectrumColor FinalBorder;
};

class ScreenStringLoader {
private:
    ZxSpectrumDisplayContext* screen = nullptr;
    // ZxSpectrumEmulatorOptions options;
    ScreenStringProgrammer programmer;

    const ScreenString* currentScreenString = nullptr;

    LoaderInfo currentLoaderInfo;
    LoaderInfo currentHeaderInfo;
    LoaderInfo currentDataInfo;

    EnvironmentInfo Current_Environment = {
        .HardwareModel = h48k,
        .Programmer = pgmNone,
        .Mode48k = mdRandom,
        .ResetMethod = 0,
        .CLS = 0,
        .Looped = 0,
        .TapeWobble = false,
        .TapeHiss = false,
        .RandomHWAfterReset = true,
        .PauseLen = 8,
        .LongFilenames = false,
        .Chuntey = false
    };

    ScreenStringLoaderContext context = {
        .LOADStatement = 0,
        .LOADLine = 0,
        .RUNEntered = false,
        .ProgramPtr = 0,
        .ProgramX = 0,
        .ProgramStage = ps48kBegin,
        .OkMessage = "",
        .GOTOLine = 0,
        .CurrentBorder = ZxSpectrumColor::White,
        .TsPerFrame = 69888,
        .TStateCount = 0,
        .ScreenATLine = 0,
        .FrameCount = 0,
        .FrameTarget = 0,
        .Program128kImage = {},
        .Stage = lsReset,
        .ResetStage = rsStart,
        .Max_Memory = 65535
    };

    std::vector<uint8_t> headerData;
    uint8_t screenData[SCREEN_DATA_SIZE];
    const uint8_t* data = nullptr;

    bool inProgress = false;

public:
    ScreenStringLoader(ZxSpectrumDisplayContext* screen)
        : screen(screen), programmer(screen, Current_Environment, context) {}
    ~ScreenStringLoader() { };

    // ZxSpectrumEmulatorOptions Options() const { return options; }

    bool Reset();
    void NewScreen(const ScreenString* screenString, bool cls = false,bool reset = false);
    bool Frame();
    void AddCommands(const std::vector<std::string>& Commands);
    void AddProgramLine(int32_t number, const std::string& typed, const std::string& listed);
    void Finish();

    bool InProgress() const { return inProgress; }

    void TestReset();
    bool TestResetFrame();

private:
    void MakeHeader();
    void DisruptChuntey();
};

#endif // SCREEN_STRING_LOADER_HPP