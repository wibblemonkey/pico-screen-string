#include <cstdint>
#include <string>
#include <cstring>
#include <vector>
#include <cmath>

#include "pico/rand.h"

#include "ScreenStringProgrammer.hpp"
#include "ZxSpectrumColors.hpp"

int ProgramLineCount = 0;
int32_t ProgramIndex = 0;
int32_t CommandLineLen = 0;
std::string LISTLine;
int32_t LISTPosX = 0;
int32_t LISTPosY = 0;
int32_t LISTIndex = 0;
std::string CommandLine128k;
int32_t ListMax = 0;
bool LineEntered = false;

void ScreenStringProgrammer::AddCommands(const std::vector<std::string>& Commands) {
    context.Commandline.insert(context.Commandline.end(), Commands.begin(), Commands.end());
}

static std::string Marker(uint8_t value) {
    return std::string(1, static_cast<char>(value));
}

void ScreenStringProgrammer::AddProgramLine(int32_t number, const std::string& typed, const std::string& listed) {
    const std::string lineNumber = std::to_string(number);
    AddCommands({Marker(254), lineNumber + " " + typed, Marker(255), "  " + lineNumber + ">" + listed});
}

void ScreenStringProgrammer::Setup() {
    std::string pauseText = std::to_string(static_cast<int32_t>(std::round(Current_Environment.PauseLen * 50)));
    const bool is48k = Current_Environment.HardwareModel == h16k || Current_Environment.HardwareModel == h48k ||
        Current_Environment.HardwareModel == h128kUsr0 || Current_Environment.HardwareModel == hPlus2Usr0 ||
        Current_Environment.HardwareModel == hPlus2aUsr0 || Current_Environment.HardwareModel == hPlus3Usr0;
    const bool isPlus3 = Current_Environment.HardwareModel == hPlus2a || Current_Environment.HardwareModel == hPlus3 ||
        Current_Environment.HardwareModel == hPlus2aUsr0 || Current_Environment.HardwareModel == hPlus3Usr0;
    const std::string loadCommand = isPlus3 ? "LOAD \"t:\" SCREEN$" : "LOAD \"\"SCREEN$";
    const std::string load128 = isPlus3 ? "load \"t:\" screen$" : "load \"\" screen$";
    if (Current_Environment.PauseLen == 0) pauseText = "1";
    context.LOADLine = 0;
    context.LOADStatement = 1;

    if (Current_Environment.Programmer == pgmDirectCommand) {
        context.RUNEntered = false;
        context.Commandline.clear();
        context.ProgramPtr = 0;
        context.ProgramX = 0;
        if (is48k) {
            context.ProgramStage = ps48kBegin;
            std::string command;
            if (Current_Environment.Looped == 0) {
                if (Current_Environment.CLS == 0) {
                    command = "LOAD \"\" SCREEN$: PAUSE " + pauseText;
                    context.OkMessage = "0 Ok, 0:2";
                    context.LOADStatement = 1;
                }
                else if (Current_Environment.CLS == 1) {
                    command = "CLS: LOAD \"\" SCREEN$: PAUSE " + pauseText;
                    context.OkMessage = "0 Ok, 0:3";
                    context.LOADStatement = 2;
                }
                else {
                    command = "CLS: FOR f=1 TO " + std::to_string(Current_Environment.CLS) + ": LOAD \"\" SCREEN$: PAUSE " + pauseText + ": NEXT f";
                    context.OkMessage = "0 Ok, 0:5";
                    context.LOADStatement = 3;
                }
            }
            else if (Current_Environment.Looped == -1) {
                if (Current_Environment.CLS == 0) {
                    command = "FOR f=0 TO 1: LOAD \"\" SCREEN$: LET f=0: PAUSE " + pauseText + ": NEXT f";
                    context.OkMessage = "0 Ok, 0:5";
                    context.LOADStatement = 2;
                } else if (Current_Environment.CLS == 1) {
                    command = "FOR f=0 TO 1: CLS: LOAD \"\" SCREEN$: PAUSE " + pauseText + ": LET f=0: NEXT f";
                    context.OkMessage = "0 Ok, 0:6";
                    context.LOADStatement = 3;
                } else {
                    command = "FOR f=0 TO 1: CLS: FOR g=1 TO " + std::to_string(Current_Environment.CLS) +
                        ": LOAD \"\" SCREEN$: PAUSE " + pauseText + ": NEXT g: LET f=0: NEXT f";
                    context.OkMessage = "0 Ok, 0:8";
                    context.LOADStatement = 4;
                }
            } else {
                if (Current_Environment.CLS == 0) {
                    command = "FOR f=0 TO " + std::to_string(Current_Environment.Looped) +
                        ": LOAD \"\" SCREEN$: PAUSE " + pauseText + ": NEXT f";
                    context.OkMessage = "0 Ok, 0:4";
                    context.LOADStatement = 2;
                } else if (Current_Environment.CLS == 1) {
                    command = "FOR f=0 TO " + std::to_string(Current_Environment.Looped) +
                        ": CLS: LOAD \"\" SCREEN$: PAUSE " + pauseText + ": NEXT f";
                    context.OkMessage = "0 Ok, 0:5";
                    context.LOADStatement = 3;
                } else {
                    command = "FOR f=0 TO " + std::to_string(Current_Environment.Looped) +
                        ": CLS: FOR g=1 TO " + std::to_string(Current_Environment.CLS) +
                        ": LOAD \"\" SCREEN$: PAUSE " + pauseText + ": NEXT g: NEXT f";
                    context.OkMessage = "0 Ok, 0:7";
                    context.LOADStatement = 4;
                }
            }
            AddCommands({Marker(254), command});
        } else {
            context.ProgramStage = Current_Environment.Mode48k == md48BASIC ? ps128kTo48k : ps128kBegin;
            if (Current_Environment.Mode48k == mdUSR0) {
                AddCommands({Marker(254), "usr0"});
            } else if (Current_Environment.Mode48k == mdSPECTRUM) {
                AddCommands({Marker(254), "spectrum"});
                context.OkMessage = isPlus3 ? "OUT 7I, 0:1" : "0 Ok, 0:1";
            } else {
                std::string command;
                if (Current_Environment.Looped == 0) {
                    if (Current_Environment.CLS == 0) command = load128 + ": pause " + pauseText;
                    else if (Current_Environment.CLS == 1) command = "cls: " + load128 + ": pause " + pauseText;
                    else command = "cls: for f=1 to " + std::to_string(Current_Environment.CLS) + ": " + load128 +
                        ": pause " + pauseText + ": next f";
                } else if (Current_Environment.Looped == -1) {
                    if (Current_Environment.CLS == 0) command = "for f=0 to 1: " + load128 + ": pause " + pauseText + ": let f=0: next f";
                    else if (Current_Environment.CLS == 1) command = "for f=0 to 1: cls: " + load128 + ": pause " + pauseText + ": let f=0: next f";
                    else command = "for f=0 to 1: cls: for g=1 to " + std::to_string(Current_Environment.CLS) + ": " +
                        load128 + ": pause " + pauseText + ": next g: let f=0: next f";
                } else {
                    if (Current_Environment.CLS == 0) command = "for f=0 to " + std::to_string(Current_Environment.Looped) + ": " + load128 + ": pause " + pauseText + ": next f";
                    else if (Current_Environment.CLS == 1) command = "for f=0 to " + std::to_string(Current_Environment.Looped) + ": cls: " + load128 + ": pause " + pauseText + ": next f";
                    else command = "for f=0 to " + std::to_string(Current_Environment.Looped) + ": cls: for g=1 to " +
                        std::to_string(Current_Environment.CLS) + ": " + load128 + ": pause " + pauseText + ": next g: next f";
                }
                AddCommands({Marker(254), command});
                context.OkMessage = Current_Environment.Looped == 0 ? "0 Ok, 0:2" : "0 Ok, 0:5";
            }
        }
    } 
    else if (Current_Environment.Programmer == pgmProgram) {
        context.RUNEntered = is48k;
        context.ProgramStage = is48k ? ps48kBegin : (Current_Environment.Mode48k == md48BASIC ? ps128kTo48k : ps128kBegin);
        context.Commandline.clear();
        int32_t lineNumber = 10;
        if (is48k) {
            const bool endless = Current_Environment.Looped == -1;
            const bool finiteLoop = Current_Environment.Looped > 0;
            const bool cls = Current_Environment.CLS > 0;
            if (finiteLoop) {
                const std::string loopText = "FOR f=0 TO " + std::to_string(Current_Environment.Looped);
                AddProgramLine(lineNumber, loopText, loopText);
                lineNumber += 10;
            } else if (endless && Current_Environment.CLS > 1) {
                AddProgramLine(lineNumber, "CLS", "CLS");
                lineNumber += 10;
                const std::string loopText = "FOR f=1 TO " + std::to_string(Current_Environment.CLS);
                AddProgramLine(lineNumber, loopText, loopText);
                lineNumber += 10;
            }
            if (cls && Current_Environment.CLS == 1) {
                AddProgramLine(lineNumber, "CLS", "CLS");
                lineNumber += 10;
            }
            if (finiteLoop && cls && Current_Environment.CLS > 1) {
                const std::string loopText = "FOR g=1 TO " + std::to_string(Current_Environment.CLS);
                AddProgramLine(lineNumber, loopText, loopText);
                lineNumber += 10;
            }
            AddProgramLine(lineNumber, loadCommand, loadCommand);
            context.LOADLine = lineNumber;
            lineNumber += 10;
            AddProgramLine(lineNumber, "PAUSE " + pauseText, "PAUSE " + pauseText);
            lineNumber += 10;
            if (finiteLoop && cls && Current_Environment.CLS > 1) {
                AddProgramLine(lineNumber, "NEXT g", "NEXT g");
                lineNumber += 10;
            }
            if (endless && Current_Environment.CLS > 1) {
                AddProgramLine(lineNumber, "NEXT f", "NEXT f");
                lineNumber += 10;
            } else if (finiteLoop) {
                AddProgramLine(lineNumber, "NEXT f", "NEXT f");
                lineNumber += 10;
            }
            if (endless) {
                AddProgramLine(lineNumber, "GO TO 10", "GO TO 10");
                context.GOTOLine = Current_Environment.CLS > 1 ? lineNumber - 40 : 10;
                context.OkMessage = "D BREAK, Cont repeats 10:1";
            } else {
                context.GOTOLine = 10;
                context.OkMessage = "0 Ok, " + std::to_string(lineNumber - 10) + ":1";
            }
            AddCommands({Marker(254), "RUN "});
        } 
        else {
            for (int32_t programMode = 0; programMode < 1; ++programMode) {
                if (Current_Environment.Mode48k == mdUSR0 || Current_Environment.Mode48k == mdSPECTRUM) {
                    AddCommands({Marker(254), Current_Environment.Mode48k == mdUSR0 ? "usr0" : "spectrum"});
                } else {
                    std::string statement = load128;
                    AddProgramLine(lineNumber, statement, isPlus3 ? "LOAD \"t:\"SCREEN$" : "LOAD \"\"SCREEN$");
                    context.LOADLine = lineNumber;
                    lineNumber += 10;
                    AddProgramLine(lineNumber, "pause " + pauseText, "PAUSE " + pauseText);
                    lineNumber += 10;
                    AddCommands({Marker(254), "run"});
                }
            }
            context.GOTOLine = 10;
            context.OkMessage = "0 Ok, " + std::to_string(lineNumber - 10) + ":1";
        }
        context.LOADStatement = 1;
    }
    CommandLineLen = static_cast<int32_t>(context.Commandline.size()) - 1;
}

bool ScreenStringProgrammer::Run() {
    int32_t index;
    std::string text;
    ZxSpectrumAttribute attribute = context.CurrentBorder < 4 ? Ink::White : Ink::Black;
    attribute |= static_cast<Paper>(context.CurrentBorder);
    bool result = true;
    context.TStateCount = context.TsPerFrame;

    switch (context.ProgramStage) {
    case ps48kBegin:
        if (context.FrameCount >= context.FrameTarget) {
            screen->DrawText(std::string(32, ' '), attribute, 0, 23);
            screen->DrawText(std::string(32, ' '), attribute, 0, 22);
            screen->DrawText("K", attribute | Flash::On, 0, 23);
            context.FrameTarget = context.FrameCount + 75;
            context.ProgramStage = ps48kCommand;
            context.ProgramX = 0;
            context.ProgramPtr = 0;
            ProgramLineCount = 2;
        }
        break;

    case ps48kCommand:
        if (context.FrameCount > context.FrameTarget) {
            if (context.ProgramPtr == static_cast<int32_t>(context.Commandline.size())) {
                result = false;
                ProgramLineCount = std::max(ProgramLineCount, 2);
                for (index = 1; index <= ProgramLineCount; ++index)
                    screen->DrawText(std::string(32, ' '), attribute, 0, 23 - (index - 1));
                context.ProgramPtr = 0;
                context.ScreenATLine = 1;
                return result;
            }
            text = context.Commandline[context.ProgramPtr++];
            const unsigned char first = text.empty() ? 0 : static_cast<unsigned char>(text[0]);
            if (first == 0) {
                screen->DrawText("E", attribute | Flash::On, context.ProgramX, 23);
            } else if (first == 254) {
                ProgramLineCount = std::max(ProgramLineCount, 2);
                for (index = 1; index <= ProgramLineCount; ++index)
                    screen->DrawText(std::string(32, ' '), attribute, 0, 23 - (index - 1));
                context.ProgramX = 0;
                ProgramLineCount = 2;
            } else if (first == 255) {
                context.ProgramX = 0;
                ProgramLineCount = 2;
                context.ProgramStage = ps48kList;
            } else {
                if (context.ProgramX + static_cast<int32_t>(text.size()) > 31) {
                    screen->DrawText(text.substr(0, 32 - context.ProgramX), attribute, context.ProgramX, 23);
                    ProgramLineCount++;
                    screen->ScrollUp(attribute, ProgramLineCount);
                    text = text.substr(32 - context.ProgramX);
                    screen->DrawText(text, attribute, 0, 23);
                    context.ProgramX = static_cast<int32_t>(text.size());
                    screen->DrawText("L", attribute | Flash::On, context.ProgramX, 23);
                } else {
                    screen->DrawText(text, attribute, context.ProgramX, 23);
                    context.ProgramX += static_cast<int32_t>(text.size());
                    if (context.ProgramX == 32) {
                        ProgramLineCount++;
                        screen->ScrollUp(attribute, ProgramLineCount);
                        context.ProgramX = 0;
                    }
                    screen->DrawText("L", attribute | Flash::On, context.ProgramX, 23);
                }
            }
            if (first == 254 || first == 255) {
                context.FrameTarget = context.FrameCount;
            } else {
                const bool repeated = context.ProgramPtr > 0 && text == context.Commandline[context.ProgramPtr - 1];
                context.FrameTarget = context.FrameCount + (repeated ? (get_rand_32() % 15) + 15 : (get_rand_32() % 25) + 15);
                if (first == 0) context.FrameTarget += 18;
            }
        }
        break;

    case ps48kList:
        if (context.ProgramX == 0) {
            std::memset(screen->GetBuffer(), 0, 3072);
            ++context.ProgramX;
        } else if (context.ProgramX == 1) {
            std::memset(screen->GetBuffer() + 3072, 0, 3072);
            ++context.ProgramX;
        } else if (context.ProgramX == 2) {
            std::memset(screen->GetBuffer() + 6144, 56, 768);
            LISTPosX = LISTPosY = 0;
            ++context.ProgramX;
        } else if (context.ProgramX == 3) {
            LISTLine = context.Commandline[context.ProgramPtr++];
            ++context.ProgramX;
            LISTIndex = 1;
        } else {
            for (index = 0; index <= 28; ++index) {
                if (LISTIndex <= static_cast<int32_t>(LISTLine.size())) {
                    if (LISTLine[LISTIndex - 1] == 13 || LISTPosX == 32) {
                        LISTPosX = 0;
                        ++LISTPosY;
                        if (LISTLine[LISTIndex - 1] == 13) ++LISTIndex;
                    }
                    if (LISTIndex <= static_cast<int32_t>(LISTLine.size())) {
                        screen->DrawText(std::string(1, LISTLine[LISTIndex - 1]), 56, LISTPosX, LISTPosY);
                        ++LISTPosX;
                        ++LISTIndex;
                    }
                } else {
                    context.ProgramStage = ps48kCommand;
                    context.FrameTarget = context.FrameCount + 30 + (get_rand_32() % 30);
                    context.ProgramX = 0;
                    ProgramLineCount = 2;
                    screen->DrawText("K", 184, 0, 23);
                    return result;
                }
            }
            ++context.ProgramX;
        }
        break;

    case ps128kBegin:
        if (context.FrameCount >= context.FrameTarget) {
            if (context.ProgramX == 0) {
                screen->DrawText("128 BASIC   ", 120, 7, 8);
                screen->DrawText("128 BASIC   ", 104, 7, 9);
                ++context.ProgramX;
                context.FrameTarget = context.FrameCount + 30 + (get_rand_32() % 25);
            } else if (context.ProgramX == 1) {
                std::memset(screen->GetBuffer(), 0, 3072);
                ++context.ProgramX;
            } else if (context.ProgramX == 2) {
                std::memset(screen->GetBuffer() + 3072, 0, 3072);
                ++context.ProgramX;
            } else {
                std::memset(screen->GetBuffer() + 6144, 56, 768);
                LISTPosX = LISTPosY = 0;
                ++context.ProgramX;
                context.ProgramStage = ps128kClear;
                LineEntered = false;
                ProgramLineCount = 0;
            }
        }
        break;

    case ps128kClear:
        if (context.ProgramX == 4) {
            screen->DrawText(std::string(32, ' '), 0, 0, 21);
            screen->DrawText(std::string(32, ' '), 56, 0, 22);
            screen->DrawText(std::string(32, ' '), 56, 0, 23);
            ++context.ProgramX;
        } else if (context.ProgramX == 5) {
            screen->DrawText("128 BASIC                 ", 71, 0, 21);
            screen->DrawText("\x0080", 66, 26, 21);
            screen->DrawText("\x0080", 86, 27, 21);
            screen->DrawText("\x0080", 116, 28, 21);
            screen->DrawText("\x0080", 101, 29, 21);
            screen->DrawText("\x0080", 104, 30, 21);
            screen->DrawText(" ", 0, 31, 21);
            ++context.ProgramX;
            LISTPosX = LISTPosY = ListMax = 0;
        } else if (context.ProgramX == 6) {
            if (LineEntered) {
                LineEntered = false;
            }
            if (LISTPosY < 21) {
                for (index = 0; index <= 28; ++index) {
                    screen->DrawText(std::string(1, static_cast<char>(context.Program128kImage[LISTPosX + LISTPosY * 32])),
                             56, LISTPosX, LISTPosY);
                    if (++LISTPosX == 32) {
                        LISTPosX = 0;
                        if (++LISTPosY == 21) break;
                    }
                }
            } else {
                context.ProgramStage = ps128kCommand;
                context.FrameTarget = context.FrameCount + 30 + (get_rand_32() % 10);
                context.ProgramX = 9999;
                LISTPosX = 0;
                LISTPosY = ProgramLineCount;
                screen->DrawText(" ", 249, 0, ProgramLineCount);
                return result;
            }
        }
        break;

    case ps128kCommand:
        if (context.FrameCount > context.FrameTarget) {
            if (context.ProgramX >= static_cast<int32_t>(CommandLine128k.size()) && context.ProgramPtr == static_cast<int32_t>(context.Commandline.size())) {
                context.ProgramStage = ps128kDone;
                context.FrameTarget = context.FrameCount + (get_rand_32() % 25) + 16;
                // BufferSample(1);
                return result;
            }
            if (context.ProgramX >= static_cast<int32_t>(CommandLine128k.size())) {
                CommandLine128k = context.Commandline[context.ProgramPtr++] + ' ';
            }
            if (!CommandLine128k.empty() && static_cast<unsigned char>(CommandLine128k[0]) == 254) {
                CommandLine128k.erase(0, 1);
                LISTPosX = 0;
                LISTPosY = ProgramLineCount;
                context.ProgramX = 1;
            } else if (!CommandLine128k.empty() && static_cast<unsigned char>(CommandLine128k[0]) == 255) {
                CommandLine128k = "  " + context.Commandline[context.ProgramPtr++];
                for (index = 0; index < static_cast<int32_t>(CommandLine128k.size()); ++index)
                    context.Program128kImage[index + ProgramLineCount * 32] = static_cast<uint8_t>(CommandLine128k[index]);
                ++ProgramLineCount;
                context.FrameTarget = context.FrameCount + 25;
                context.ProgramStage = ps128kClear;
                context.ProgramX = 6;
                LISTPosX = LISTPosY = 0;
                LineEntered = true;
            } else {
                if (context.ProgramX < static_cast<int32_t>(CommandLine128k.size()))
                    screen->DrawText(std::string(1, CommandLine128k[context.ProgramX]), 56, LISTPosX, LISTPosY);
                ++context.ProgramX;
                if (++LISTPosX == 32) {
                    LISTPosX = 5;
                    ++LISTPosY;
                }
                screen->DrawText(" ", 249, LISTPosX, LISTPosY);
            }
            if (CommandLine128k == Marker(254) || CommandLine128k == Marker(255)) {
                context.FrameTarget = context.FrameCount;
            } else if (context.ProgramX > 1 && context.ProgramX <= static_cast<int32_t>(CommandLine128k.size()) &&
                       CommandLine128k[context.ProgramX - 1] == CommandLine128k[context.ProgramX - 2]) {
                context.FrameTarget = context.FrameCount + (get_rand_32() % 5) + 10;
            } else {
                context.FrameTarget = context.FrameCount + (get_rand_32() % 10) + 10;
            }
        }
        break;

    case ps128kDone:
        if (context.FrameCount >= context.FrameTarget) {
            result = false;
            if (Current_Environment.Mode48k == mdUSR0) {
                switch (Current_Environment.HardwareModel) {
                case h128k: Current_Environment.HardwareModel = h128kUsr0; break;
                case hPlus2: Current_Environment.HardwareModel = hPlus2Usr0; break;
                case hPlus2a: Current_Environment.HardwareModel = hPlus2aUsr0; break;
                case hPlus3: Current_Environment.HardwareModel = hPlus3Usr0; break;
                default: break;
                }
                Setup();
                context.Stage = lsReset;
                context.ResetStage = rsStart;
                context.FrameTarget = context.FrameCount + 30 + (get_rand_32() % 25);
            } else if (Current_Environment.Mode48k == mdSPECTRUM) {
                screen->Cls(56);
                // std::memset(screen->GetBuffer(), 0, 6144);
                // std::memset(screen->GetBuffer() + 6144, 56, 768);
                for (index = 1; index <= 3; ++index) screen->DrawText(std::string(32, ' '), attribute, 0, 23 - (index - 1));
                screen->DrawText(context.OkMessage, 56, 0, 23);
                context.FrameTarget = context.FrameCount + 30 + (get_rand_32() % 25);
                Setup();
                result = true;
            } else {
                context.ProgramPtr = 0;
                context.FrameTarget = context.FrameCount + 30 + (get_rand_32() % 25);
                context.ProgramStage = ps128kRUN;
                result = true;
            }
        }
        break;

    case ps128kRUN:
        context.ProgramPtr = 0;
        for (index = 1; index <= 3; ++index) screen->DrawText(std::string(32, ' '), attribute, 0, 23 - (index - 1));
        screen->DrawText(" ", 56, LISTPosX, LISTPosY);
        context.ScreenATLine = LISTPosY + 1;
        context.FrameTarget = context.FrameCount + 4;
        result = false;
        break;

    case ps128kTo48k:
        if (context.FrameCount >= context.FrameTarget) {
            if (context.ProgramX == 0) {
                screen->DrawText("\x0081Tape Loader \x0085", 120, 7, 8);
                screen->DrawText("\x0081" "128 BASIC   \x0085", 104, 7, 9);
                ++context.ProgramX;
            } else if (context.ProgramX == 1) {
                screen->DrawText("\x0081" "128 BASIC   \x0085", 120, 7, 9);
                screen->DrawText("\x0081" "Calculator  \x0085", 104, 7, 10);
                ++context.ProgramX;
            } else if (context.ProgramX == 2) {
                screen->DrawText("\x0081" "Calculator  \x0085", 120, 7, 10);
                screen->DrawText("\x0081" "48 BASIC    \x0085", 104, 7, 11);
                ++context.ProgramX;
            } else if (context.ProgramX == 3) {
                screen->DrawText("              ", 56, 7, 7);
                screen->DrawText("              ", 120, 7, 8);
                ++context.ProgramX;
            } else if (context.ProgramX == 4) {
                for (index = 7; index <= 13; ++index) screen->DrawText("              ", 56, 7, index);
                ++context.ProgramX;
            } else {
                switch (Current_Environment.HardwareModel) {
                case h128k: Current_Environment.HardwareModel = h128kUsr0; break;
                case hPlus2: Current_Environment.HardwareModel = hPlus2Usr0; break;
                case hPlus2a: Current_Environment.HardwareModel = hPlus2aUsr0; break;
                case hPlus3: Current_Environment.HardwareModel = hPlus3Usr0; break;
                default: break;
                }
                context.FrameTarget = context.FrameCount + 1;
                Setup();
                context.Stage = lsReset;
                context.ResetStage = rsStart;
                result = false;
            }
            if (context.ProgramX < 5) {
                context.FrameTarget = context.FrameCount + 10 + (get_rand_32() % 25);
            }
            else if (context.ProgramX == 5) {
                context.FrameTarget = context.FrameCount + 1;
            }
        }
        break;

    default:
        break;
    }
    return result;
}
