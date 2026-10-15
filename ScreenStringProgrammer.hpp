#ifndef SCREEN_STRING_PROGRAMMER_HPP
#define SCREEN_STRING_PROGRAMMER_HPP

#include <string>
#include <vector>

#include "ZxSpectrumDisplayContext.hpp"
#include "PicoScreenStringOptions.hpp"

enum TLoaderStage { lsReset, lsProgram, lsPreLoad, lsPostProgram, lsPrePilot, lsPilot, lsData, lsPause, lsFinish, lsQuit };
enum TResetStage { rsStart, rsFill2, rsClear, rs128kClear, rs128kMenu, rsCLS, rsMessage, rsPause };
enum TProgramStage {
    ps48kBegin, ps48kCommand, ps48kClearLower, ps48kList, ps128kBegin,
    psMenuChoose, ps128kClear, ps128kCommand, ps128kDone, ps128kTo48k, ps128kRUN
};

struct ScreenStringLoaderContext {
    std::vector<std::string> Commandline;
    int32_t LOADStatement;
    int32_t LOADLine;
    bool RUNEntered;
    int32_t ProgramPtr;
    int32_t ProgramX;
    TProgramStage ProgramStage;
    std::string OkMessage;
    int32_t GOTOLine;
    ZxSpectrumColor CurrentBorder;
    uint32_t TsPerFrame;
    int32_t TStateCount;
    int32_t ScreenATLine;
    int32_t FrameCount;
    int32_t FrameTarget;
    std::array<uint8_t, 672> Program128kImage;
    TLoaderStage Stage;
    TResetStage ResetStage;
    uint32_t Max_Memory;
};


class ScreenStringProgrammer {
private:
    ZxSpectrumDisplayContext* screen;
    EnvironmentInfo Current_Environment;
    ScreenStringLoaderContext context;

public:
    ScreenStringProgrammer(ZxSpectrumDisplayContext* screen, EnvironmentInfo environment, ScreenStringLoaderContext context)
        : screen(screen), Current_Environment(environment), context(context) { }
    ~ScreenStringProgrammer() { }

    void Setup();
    bool Run();
private:
    void AddCommands(const std::vector<std::string>& Commands);
    void AddProgramLine(int32_t number, const std::string& typed, const std::string& listed);
};

#endif // SCREEN_STRING_PROGRAMMER_HPP