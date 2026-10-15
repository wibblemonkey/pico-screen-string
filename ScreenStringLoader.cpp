#include <cstring>

#include "pico/rand.h"

#include "ScreenStringLoader.hpp"
#include "ScreenStringProgrammer.hpp"

enum TMenuStage {
    msMessage1, msMessage2, msMessage3, msMessage4, msMenuStrip, msOptions1, msOptions2,
    msOptions3, msLeftBorder, msBottom1, msBottom2, msRightBorder, msHighlight
};
enum TDataStage { dsSync1, dsSync2, dsData1, dsData2 };
enum TDataType { dtHeader, dtData };
enum TChunteyType { ctForward, ctBackward };

std::string ErrorR;
std::string BREAKMessage;
uint32_t TapeWobble = 0;
uint32_t TapeHiss = 0;
uint32_t TapeAngle = 0;
uint32_t TapeWobbleFactor = 0;
bool ChunteyDisrupted = false;
bool WillDisruptChuntey = false;
uint32_t ChunteyPosition = 0;
bool LastScreen_Disrupted = false;
TChunteyType ChunteyType = ctForward;
bool Random48k = false;
uint32_t PilotFrameCount = 0;
TLoaderStage Last_Stage = lsReset;
TDataStage DataStage = dsSync1;
TDataType DataType = dtHeader;
uint8_t PreHeaderStage = 0;
uint8_t PulseNumber = 0;
int32_t DataPosition_Byte = 0;
uint8_t DataPosition_Bit = 0;
int32_t PilotCount = 0;
std::string Display_Name;
int32_t Screen_Number = 0;
int32_t Loop_Target = 0;
int32_t Pilot_Loop_Count = 0;
int32_t Pilot_Click_Count = 0;
int32_t LeftOverTStates = 0;
int32_t ResetAddress = 0;
TMenuStage MenuStage = msMessage1;

static void InitializeHeaderInfo(LoaderInfo& loaderInfo, int32_t modifyTs) {
    loaderInfo.Pilot_Border_1 = ZxSpectrumColor::Cyan;
    loaderInfo.Pilot_Border_2 = ZxSpectrumColor::Red;
    loaderInfo.Pilot_Repeats = 8064;
    loaderInfo.Pilot_Tone_Length = 2168 + modifyTs;
    loaderInfo.Pilot_Loops = 1;
    loaderInfo.Sync1_Length = 667;
    loaderInfo.Sync2_Length = 735;
    loaderInfo.Data_Border_1 = ZxSpectrumColor::Blue;
    loaderInfo.Data_Border_2 = ZxSpectrumColor::Yellow;
    loaderInfo.Data_One_Length = 1710;
    loaderInfo.Data_Zero_Length = 835;
    loaderInfo.Data_Pause_Length = 928;
    loaderInfo.PreLoad_Delay = 50 * 2;
    loaderInfo.PreHeader_Delay = 50 * 4;
    loaderInfo.Data_Length = 17;
    loaderInfo.FinalBorder = ZxSpectrumColor::Red;
}

static void InitializeDataInfo(LoaderInfo& loaderInfo, int32_t modifyTs) {
    loaderInfo.Pilot_Border_1 = ZxSpectrumColor::Cyan;
    loaderInfo.Pilot_Border_2 = ZxSpectrumColor::Red;
    loaderInfo.Pilot_Repeats = 3220;
    loaderInfo.Pilot_Tone_Length = 2168 + modifyTs;
    loaderInfo.Pilot_Loops = 1;
    loaderInfo.Sync1_Length = 667;
    loaderInfo.Sync2_Length = 735;
    loaderInfo.Data_Border_1 = ZxSpectrumColor::Blue;
    loaderInfo.Data_Border_2 = ZxSpectrumColor::Yellow;
    loaderInfo.Data_One_Length = 1710;
    loaderInfo.Data_Zero_Length = 835;
    loaderInfo.Data_Pause_Length = 2000;
    loaderInfo.PreLoad_Delay = 20;
    loaderInfo.PreHeader_Delay = 50;
    loaderInfo.Data_Length = 6912;
    loaderInfo.FinalBorder = ZxSpectrumColor::Red;
}

void ScreenStringLoader::MakeHeader() {
    int32_t extensionPosition;
    size_t index;

    headerData.resize(18);
    // Filename = GetFilename(Filename);
    // const std::size_t extension = Lower(Filename).find(".scr");
    // if (extension != std::string::npos) {
    //     Filename = Filename.substr(0, extension);
    // }
    // while (screen.Filename.size() < 10) {
    //     screen.Filename += ' ';
    // }
    // if (!Current_Environment.LongFilenames) {
    //     screen.Filename.resize(10);
    // }
    Display_Name = currentScreenString->Filename;
    // if (Current_Environment.LongFilenames) {
    //     headerData.resize(18 + Display_Name.size() - 10);
    // }

    headerData[0] = 3;
    for (index = 1; index < Display_Name.size(); index++) {
        headerData[index] = static_cast<uint8_t>(currentScreenString->Filename[index - 1]);
    }
    index = Display_Name.size();
    headerData[index + 1] = static_cast<uint8_t>(6912 & 255);
    headerData[index + 2] = static_cast<uint8_t>(6912 >> 8);
    headerData[index + 3] = static_cast<uint8_t>(16384 & 255);
    headerData[index + 4] = static_cast<uint8_t>(16384 >> 8);
    headerData[index + 5] = 0;
    headerData[index + 6] = 0;

    uint8_t checksum = 3;
    for (auto i = 0; i < SCREEN_DATA_SIZE; i++) {
        checksum ^= screenData[i];
    }
    headerData[index + 7] = checksum;
}

void ScreenStringLoader::NewScreen(const ScreenString* screenString, bool cls, bool reset) {
    int32_t chunteyChance;
    int32_t index;
    bool doReset;

    currentScreenString = screenString;
    currentScreenString->Decompress(screenData);

    int32_t modifyTs = get_rand_32() % 20 - 10;
    InitializeHeaderInfo(currentHeaderInfo, modifyTs);
    InitializeDataInfo(currentDataInfo, modifyTs);

    // if (Current_Environment.Looped > 0) {
    //     Loop_Target = Current_Environment.CLS > 0
    //         ? (Current_Environment.Looped + 1) * Current_Environment.CLS
    //         : Current_Environment.Looped + 1;
    // } else {
    //     Loop_Target = 0;
    // }

    // if (Current_Environment.Programmer != pgmNone && Screen_Number == -1) {
    //     context.Stage = lsReset;
    //     Screen_Number = Loop_Target - 1;
    // } else if (Current_Environment.Programmer != pgmNone) {
    //     context.Stage = lsProgram;
    // } else {
    //     context.Stage = lsPostProgram;
    // }

    if (reset) {
        context.Stage = lsReset;
    } else {
        context.Stage = lsPostProgram;
    }

    context.FrameCount &= 15;
    DataType = dtHeader;
    context.FrameTarget = 50;
    PulseNumber = 0;
    ResetAddress = context.Max_Memory;
    context.CurrentBorder = ZxSpectrumColor::White;
    MakeHeader();
    screen->SetBorder(context.CurrentBorder);
    // screen->Cls();
    // context.ScreenATLine = 0;
    inProgress = true;
    // ++Screen_Number;
    // if (Random48k) {
    //     Current_Environment.Mode48k = static_cast<T48kMode>(get_rand_32() % 4 + 1);
    // }

    // if (Current_Environment.Programmer != pgmNone && Current_Environment.ResetMethod >= 0) {
    //     if (Current_Environment.ResetMethod == 0) {
    //         doReset = get_rand_32() % 50 > 25 || reset;
    //     } else {
    //         doReset = Screen_Number % Current_Environment.ResetMethod == 0;
    //     }
    //     if (doReset) {
    //         context.Commandline.clear();
    //         if (Current_Environment.RandomHWAfterReset) {
    //             index = get_rand_32() % 5;
    //             switch (index) {
    //             case 0: Current_Environment.HardwareModel = h16k; break;
    //             case 1: Current_Environment.HardwareModel = h48k; break;
    //             case 2: Current_Environment.HardwareModel = h128k; break;
    //             case 3: Current_Environment.HardwareModel = hPlus2; break;
    //             case 4: Current_Environment.HardwareModel = hPlus2a; break;
    //             case 5: Current_Environment.HardwareModel = hPlus3; break;
    //             }
    //             if (Current_Environment.HardwareModel == h16k || Current_Environment.HardwareModel == h48k) {
    //                 context.TsPerFrame = 69888;
    //                 context.Max_Memory = Current_Environment.HardwareModel == h16k ? 32768 : 65535;
    //             } else {
    //                 context.TsPerFrame = 70908;
    //                 context.Max_Memory = 65535;
    //             }
    //         }

    //         switch (Current_Environment.HardwareModel) {
    //         case h128kUsr0: Current_Environment.HardwareModel = h128k; break;
    //         case hPlus2Usr0: Current_Environment.HardwareModel = hPlus2; break;
    //         case hPlus2aUsr0: Current_Environment.HardwareModel = hPlus2a; break;
    //         case hPlus3Usr0: Current_Environment.HardwareModel = hPlus3; break;
    //         default: break;
    //         }
    //         context.ProgramStage = (Current_Environment.HardwareModel == h16k || Current_Environment.HardwareModel == h48k) ? ps48kBegin : ps128kBegin;
    //         context.Stage = lsReset;
    //         context.ResetStage = rsStart;
    //         Screen_Number = Loop_Target;
    //         programmer.Setup();
    //     }
    // }

    TapeAngle = 0;
    TapeWobbleFactor = get_rand_32() % 20 + 5;
    ChunteyDisrupted = false;
    WillDisruptChuntey = false;
    if (Current_Environment.Chuntey) {
        chunteyChance = 10;
        if (Current_Environment.TapeWobble) chunteyChance += 10;
        if (Current_Environment.TapeHiss) chunteyChance += 10;
        if (get_rand_32() % 1000 < chunteyChance) {
            WillDisruptChuntey = true;
            ChunteyPosition = get_rand_32() % currentDataInfo.Data_Length;
        }
    }
}

bool ScreenStringLoader::Frame() {
    uint32_t lastTs;
    int32_t changeAtTStates;
    int32_t index;
    int64_t loadingByte;
    ZxSpectrumAttribute attribute;
    bool update = false;
    std::string tempStr;

    bool result = false;

    while (context.TStateCount < context.TsPerFrame) {
        if (Current_Environment.TapeWobble && context.Stage > lsPostProgram) {
            TapeAngle += static_cast<int32_t>(std::round(static_cast<double>(currentLoaderInfo.Pilot_Tone_Length) / currentLoaderInfo.Data_Zero_Length));
            if (TapeAngle == 10000) {
                TapeAngle = 0;
                TapeWobbleFactor = get_rand_32() % 20 + 5;
            }
            TapeWobble = static_cast<int32_t>(std::round(TapeWobbleFactor * std::sin((TapeAngle / 27.0) * 3.14159265358979323846 / 180.0)));
        }
        if (Current_Environment.TapeHiss) {
            TapeHiss = get_rand_32() % 100 - 50;
        }

        switch (context.Stage) {
        case lsReset:
            // printf("Entering lsReset stage\n");
            update = true;
            if (!Reset()) {
                context.Stage = lsProgram;
                if (Current_Environment.HardwareModel == h128k || Current_Environment.HardwareModel == hPlus2 ||
                    Current_Environment.HardwareModel == hPlus2a || Current_Environment.HardwareModel == hPlus3) {
                    context.Program128kImage.fill(32);
                }
            }
            break;

        case lsProgram:
            // printf("Entering lsProgram stage\n");
            // if (!LastScreen_Disrupted && (Screen_Number < Loop_Target || (Screen_Number > 0 && Loop_Target == 0))) {
                context.Stage = lsPostProgram;
            // } else {
            //     update = true;
            //     if (!programmer.Run() && context.Stage != lsReset) {
            //         context.FrameCount &= 15;
            //         Screen_Number = 0;
            //         LastScreen_Disrupted = false;
            //         context.Stage = lsPostProgram;
            //     }
            // }
            break;

        case lsPostProgram:
            // printf("Entering lsPostProgram stage\n");
            currentLoaderInfo = currentHeaderInfo;
            context.Stage = lsPreLoad;
            // if (context.RUNEntered) {
            //     context.RUNEntered = false;
                // std::memset(Display.data(), 0, 6144);
                // std::memset(Display.data() + 6144, 56, 768);
            screen->Cls();
            context.ScreenATLine = 0;
            // }
            break;

        case lsPreLoad:
            // printf("Entering lsPreLoad stage\n");
            if (context.FrameCount > currentLoaderInfo.PreLoad_Delay) {
                context.Stage = lsPrePilot;
                PreHeaderStage = 0;
                if (Current_Environment.CLS != 0 && Screen_Number % Current_Environment.CLS == 0) {
                    screen->Cls(56);
                    // std::memset(Display.data(), 0, 6144);
                    // std::memset(Display.data() + 6144, 56, 768);
                    context.ScreenATLine = 0;
                }
            }
            context.TStateCount = context.TsPerFrame;
            // DrawDisplay(context.CurrentBorder, 0, context.TsPerFrame);
            screen->SetBorder(context.CurrentBorder);
            result = true;
            break;

        case lsPrePilot:
            // printf("Entering lsPrePilot stage\n");
            if (context.FrameCount > context.FrameTarget) {
                if (context.FrameCount > currentLoaderInfo.PreHeader_Delay + currentLoaderInfo.PreLoad_Delay) {
                    context.Stage = lsPilot;
                    PilotCount = 0;
                    Pilot_Loop_Count = currentLoaderInfo.Pilot_Loops;
                    Pilot_Click_Count = currentLoaderInfo.Pilot_Click_Repeats;
                    PilotFrameCount = 50;
                    screen->SetBorder(PulseNumber == 0 ? currentLoaderInfo.Pilot_Border_1 : currentLoaderInfo.Pilot_Border_2);
                    changeAtTStates = get_rand_32() % context.TsPerFrame;
                    if (PulseNumber == 0) {
                        // DrawDisplay(currentLoaderInfo.Pilot_Border_2, 0, changeAtTStates);
                        // DrawDisplay(currentLoaderInfo.Pilot_Border_1, changeAtTStates, TsPerFrame);
                        screen->SetBorder(currentLoaderInfo.Pilot_Border_2);
                        sleep_us(changeAtTStates / 3.5);
                        screen->SetBorder(currentLoaderInfo.Pilot_Border_1);
                        //sleep_us((TsPerFrame - changeAtTStates) / 3.5);
                    } else {
                        // DrawDisplay(currentLoaderInfo.Pilot_Border_1, 0, changeAtTStates);
                        // DrawDisplay(currentLoaderInfo.Pilot_Border_2, changeAtTStates, TsPerFrame);
                        screen->SetBorder(currentLoaderInfo.Pilot_Border_1);
                        sleep_us(changeAtTStates / 3.5);
                        screen->SetBorder(currentLoaderInfo.Pilot_Border_2);
                        //sleep_us((TsPerFrame - changeAtTStates) / 3.5);
                    }
                    context.FrameTarget = context.FrameCount + 1;
                    PreHeaderStage = 1;
                    context.TStateCount = context.TsPerFrame;
                    result = true;
                } else {
                    // DrawDisplay(PulseNumber == 0 ? currentLoaderInfo.Pilot_Border_1 : currentLoaderInfo.Pilot_Border_2, 0, TsPerFrame);
                    screen->SetBorder(PulseNumber == 0 ? currentLoaderInfo.Pilot_Border_1 : currentLoaderInfo.Pilot_Border_2);
                    printf("Setting border for pulse number %d, border value %d\n", PulseNumber, screen->GetBorder());
                    PulseNumber = 1 - PulseNumber;
                    context.FrameTarget = context.FrameCount + 50 + get_rand_32() % 20 - 10;
                    context.TStateCount = context.TsPerFrame;
                    result = true;
                }
            } else {
                context.TStateCount = context.TsPerFrame;
                // DrawDisplay(CurrentBorder, 0, TsPerFrame);
                //spectrumScreen.SetBorder(CurrentBorder);
                result = true;
            }
            break;

        case lsPilot:
            // printf("Entering lsPilot stage\n");
            result = true;
            if (Pilot_Loop_Count > 0) {
                if (PilotCount < currentLoaderInfo.Pilot_Repeats) {
                    lastTs = context.TStateCount;
                    context.TStateCount += currentLoaderInfo.Pilot_Tone_Length + TapeWobble + TapeHiss;
                    if (PilotFrameCount == 0) {
                        // DrawDisplay(PulseNumber == 0 ? currentLoaderInfo.Pilot_Border_1 : currentLoaderInfo.Pilot_Border_2, lastTs, context.TStateCount);
                        screen->SetBorder(PulseNumber == 0 ? currentLoaderInfo.Pilot_Border_1 : currentLoaderInfo.Pilot_Border_2);
                        sleep_us((context.TStateCount - lastTs) / 3.5);
                    } else {
                        // DrawDisplay(context.CurrentBorder, lastTs, context.TStateCount);
                        screen->SetBorder(context.CurrentBorder);
                        sleep_us((context.TStateCount - lastTs) / 3.5);
                    }
                    PulseNumber = 1 - PulseNumber;
                    ++PilotCount;
                    if (PilotCount == currentLoaderInfo.Pilot_Repeats) {
                        Pilot_Click_Count = currentLoaderInfo.Pilot_Click_Repeats;
                    }
                } else if (Pilot_Click_Count > 0) {
                    lastTs = context.TStateCount;
                    context.TStateCount += currentLoaderInfo.Pilot_Click_Length + TapeWobble + TapeHiss;
                    if (PilotFrameCount == 0) {
                        screen->SetBorder(PulseNumber == 0 ? currentLoaderInfo.Pilot_Border_1 : currentLoaderInfo.Pilot_Border_2);
                        sleep_us((context.TStateCount - lastTs) / 3.5);
                    } else {
                        screen->SetBorder(context.CurrentBorder);
                        sleep_us((context.TStateCount - lastTs) / 3.5);
                    }
                    PulseNumber = 1 - PulseNumber;
                    --Pilot_Click_Count;
                } else {
                    --Pilot_Loop_Count;
                    PilotCount = 0;
                }
            } else {
                context.Stage = lsData;
                ChunteyDisrupted = false;
                DataStage = dsSync1;
            }
            break;

        case lsData:
            // printf("Entering lsData stage\n");
            switch (DataStage) {
            case dsSync1:
                // printf("Entering dsSync1 stage\n");
                lastTs = context.TStateCount;
                context.TStateCount += currentLoaderInfo.Sync1_Length + TapeHiss;
                screen->SetBorder(currentLoaderInfo.Data_Border_1);
                sleep_us((context.TStateCount - lastTs) / 3.5);
                DataStage = dsSync2;
                break;

            case dsSync2:
                // printf("Entering dsSync2 stage\n");
                lastTs = context.TStateCount;
                context.TStateCount += currentLoaderInfo.Sync2_Length + TapeHiss;
                screen->SetBorder(currentLoaderInfo.Data_Border_2);
                sleep_us((context.TStateCount - lastTs) / 3.5);
                DataStage = dsData1;
                DataPosition_Byte = 0;
                DataPosition_Bit = 128;
                data = DataType == dtHeader ? headerData.data() : screenData;
                break;

            case dsData1:
                // printf("Entering dsData1 stage\n");
                lastTs = context.TStateCount;
                loadingByte = *data;
                context.TStateCount += ((loadingByte & DataPosition_Bit) == 0 ? currentLoaderInfo.Data_Zero_Length : currentLoaderInfo.Data_One_Length) + TapeHiss;
                screen->SetBorder(currentLoaderInfo.Data_Border_1);
                sleep_us((context.TStateCount - lastTs) / 3.5);
                DataStage = dsData2;
                break;

            case dsData2:
                // printf("Entering dsData2 stage\n");
                lastTs = context.TStateCount;
                loadingByte = *data;
                // if (WillDisruptChuntey && DataPosition_Byte >= ChunteyPosition) {
                //     //DisruptChuntey();
                // }
                // if (ChunteyDisrupted) {
                //     if (ChunteyType == ctForward) {
                //         loadingByte = DataPosition_Byte < Current_Loader.Data_Length - 1
                //             ? (loadingByte << 1) | (CurrentScreen.Data[DataPosition_Byte + 1] >> 7)
                //             : loadingByte << 1;
                //     } else {
                //         loadingByte = DataPosition_Byte > 0
                //             ? (loadingByte >> 1) | (CurrentScreen.Data[DataPosition_Byte - 1] & 1)
                //             : loadingByte >> 1;
                //     }
                // }
                context.TStateCount += ((loadingByte & DataPosition_Bit) == 0 ? currentLoaderInfo.Data_Zero_Length : currentLoaderInfo.Data_One_Length) + TapeHiss;
                // DrawDisplay(currentLoaderInfo.Data_Border_2, lastTs, TStateCount);
                screen->SetBorder(currentLoaderInfo.Data_Border_2);
                // EARStatus = !EARStatus;
                sleep_us((context.TStateCount - lastTs) / 3.5);
                // SoundOut(TStateCount - lastTs);
                DataPosition_Bit >>= 1;
                if (DataPosition_Bit > 0) {
                    DataStage = dsData1;
                } else {
                    ++DataPosition_Byte;
                    DataPosition_Bit = 128;
                    if (DataPosition_Byte <= currentLoaderInfo.Data_Length) {
                        ++data;
                        DataStage = dsData1;
                        if (DataType == dtData) {
                            screen->GetBuffer()[DataPosition_Byte - 1] = static_cast<uint8_t>(loadingByte & 0xFF);
                        } 
                    } else if (DataType == dtHeader) {
                        ++context.ScreenATLine;
                        if (context.ScreenATLine == 22) {
                            --context.ScreenATLine;
                            screen->ScrollScreen();
                        }
                        screen->DrawText("Bytes: " + Display_Name, 56, 0, context.ScreenATLine);
                        // DrawDisplay(7, 0, TsPerFrame);
                        screen->SetBorder(ZxSpectrumColor::White);
                        context.Stage = lsPrePilot;
                        DataType = dtData;
                        context.FrameCount &= 0xF;
                        context.FrameTarget = context.FrameCount + currentLoaderInfo.Data_Pause_Length / 20 + 50;
                        PilotFrameCount = 50;
                        currentLoaderInfo = currentDataInfo;
                        update = true;
                        context.TStateCount = context.TsPerFrame;
                    } else {
                        screen->SetBorder(currentLoaderInfo.FinalBorder);
                        if (ChunteyDisrupted && Current_Environment.Programmer != pgmNone) {
                            uint8_t attribute = static_cast<Paper>(currentLoaderInfo.FinalBorder);
                            screen->DrawText(std::string(32, ' '), attribute, 0, 22);
                            screen->DrawText(std::string(32, ' '), attribute, 0, 23);
                            attribute = currentLoaderInfo.FinalBorder < 4 ? Ink::White : Ink::Black;
                            attribute |= static_cast<Paper>(currentLoaderInfo.FinalBorder);
                            screen->DrawText("R Tape loading error, " + std::to_string(context.LOADLine) + ":" + std::to_string(context.LOADStatement), attribute, 0, 23);
                            LastScreen_Disrupted = true;
                            if (Current_Environment.Programmer == pgmDirectCommand) {
                                programmer.Setup();
                                context.ProgramStage = (Current_Environment.HardwareModel == h128k ||
                                    Current_Environment.HardwareModel == hPlus2 ||
                                    Current_Environment.HardwareModel == hPlus2a ||
                                    Current_Environment.HardwareModel == hPlus3) ? ps128kClear : ps48kBegin;
                                if (context.ProgramStage == ps128kClear) {
                                    context.ProgramX = 4;
                                    context.ProgramPtr = 0;
                                }
                            } 
                            else {
                                context.Commandline.clear();
                                if (Current_Environment.HardwareModel == h16k || Current_Environment.HardwareModel == h48k ||
                                    Current_Environment.HardwareModel == h128kUsr0 || Current_Environment.HardwareModel == hPlus2Usr0 ||
                                    Current_Environment.HardwareModel == hPlus2aUsr0 || Current_Environment.HardwareModel == hPlus3Usr0) {
                                    context.ProgramStage = ps48kBegin;
                                    // AddCommands({std::string(1, static_cast<char>(254)), "GO TO "});
                                    AddCommands({"\x00FE", "GO TO "});
                                    tempStr = std::to_string(context.GOTOLine);
                                    for (char character : tempStr) AddCommands({std::string(1, character)});
                                } 
                                else {
                                    context.ProgramX = 4;
                                    context.ProgramPtr = 0;
                                    context.ProgramStage = ps128kClear;
                                    AddCommands({"\x00FE", "goto " + std::to_string(context.GOTOLine)});
                                }
                            }
                        } 
                        else if (Current_Environment.Programmer != pgmNone && Screen_Number == Loop_Target - 1) {
                            screen->DrawText(std::string(32, ' '), static_cast<Paper>(currentLoaderInfo.FinalBorder), 0, 22);
                            screen->DrawText(std::string(32, ' '), static_cast<Paper>(currentLoaderInfo.FinalBorder), 0, 23);
                            attribute = currentLoaderInfo.FinalBorder < 4 ? Ink::White : Ink::Black;
                            attribute |= static_cast<Paper>(currentLoaderInfo.FinalBorder);
                            screen->DrawText(context.OkMessage, attribute, 0, 23);
                            context.Commandline.clear();
                            if (Current_Environment.Programmer == pgmDirectCommand) {
                                programmer.Setup();
                                context.ProgramStage = (Current_Environment.HardwareModel == h128k ||
                                    Current_Environment.HardwareModel == hPlus2 ||
                                    Current_Environment.HardwareModel == hPlus2a ||
                                    Current_Environment.HardwareModel == hPlus3) ? ps128kClear : ps48kBegin;
                                if (context.ProgramStage == ps128kClear) {
                                    context.ProgramX = 4;
                                    context.ProgramPtr = 0;
                                }
                            } 
                            else if (Current_Environment.HardwareModel == h16k || Current_Environment.HardwareModel == h48k ||
                                       Current_Environment.HardwareModel == h128kUsr0 || Current_Environment.HardwareModel == hPlus2Usr0 ||
                                       Current_Environment.HardwareModel == hPlus2aUsr0 || Current_Environment.HardwareModel == hPlus3Usr0) {
                                context.ProgramStage = ps48kBegin;
                                AddCommands({"\x00FE", "RUN "});
                                context.RUNEntered = true;
                            } 
                            else {
                                context.ProgramX = 4;
                                context.ProgramPtr = 0;
                                context.ProgramStage = ps128kClear;
                                AddCommands({"\x00FE", "run"});
                                context.RUNEntered = true;
                            }
                        }
                        screen->SetBorder(currentLoaderInfo.FinalBorder);
                        context.Stage = lsFinish;
                        context.FrameCount &= 15;
                        context.FrameTarget = context.FrameCount + static_cast<int32_t>(std::round(Current_Environment.PauseLen * 50));
                        context.TStateCount = context.TsPerFrame;
                        update = true;
                    }
                }
                break;
            }
            result = true;
            break;

        case lsFinish:
            // printf("Entering lsFinish stage\n");
            context.TStateCount = context.TsPerFrame;
            if (context.FrameCount >= context.FrameTarget) {
                // screen->SetBorder(currentLoaderInfo.FinalBorder);
                inProgress = false;
            } else {
                screen->SetBorder(currentScreenString->Border);
                result = true;
            }
            break;

        // case lsQuit:
        //     // printf("Entering lsQuit stage\n");
        //     if (Current_Environment.Programmer == pgmNone || Last_Stage == lsReset || Last_Stage == lsProgram) {
        //         inProgress = false;
        //         context.TStateCount = context.TsPerFrame;
        //         update = true;
        //     }
        //     else if (Last_Stage == lsPreLoad || Last_Stage == lsPostProgram || Last_Stage == lsPrePilot || Last_Stage == lsPilot || Last_Stage == lsData || Last_Stage == lsPause) {
        //         screen->SetBorder(currentLoaderInfo.FinalBorder);
        //         screen->DrawText(std::string(32, ' '), static_cast<Paper>(currentLoaderInfo.FinalBorder), 0, 22);
        //         screen->DrawText(std::string(32, ' '), static_cast<Paper>(currentLoaderInfo.FinalBorder), 0, 23);
        //         attribute = currentLoaderInfo.FinalBorder < 4 ? Ink::White : Ink::Black;
        //         attribute |= static_cast<Paper>(currentLoaderInfo.FinalBorder);
        //         screen->DrawText("D Break - CONT repeats, " + std::to_string(context.LOADLine) + ":" + std::to_string(context.LOADStatement), attribute, 0, 23);
        //         context.FrameTarget = context.FrameCount + 125;
        //         context.TStateCount = context.TsPerFrame;
        //         update = true;
        //         context.Stage = lsFinish;
        //     }
        //     break;

        default:
            context.TStateCount = context.TsPerFrame;
            result = false;
            break;
        }
    }

    if (update) {
        // DrawDisplay(CurrentBorder, 0, TsPerFrame);
        screen->SetBorder(context.CurrentBorder);
        // SoundSize = 0;
        // SoundPos = static_cast<pByte>(SoundHandle);
        // SoundOut(TsPerFrame);
        result = true;
    }
    if (context.TStateCount >= context.TsPerFrame) context.TStateCount -= context.TsPerFrame;
    // if (SoundSize == 0) SoundSize = 44100 / 50;
    // else --SoundSize;
    ++context.FrameCount;
    if (PilotFrameCount > 0) --PilotFrameCount;
    return result;
}

void ScreenStringLoader::Finish() {
    if(inProgress)
    {
        std::memcpy(screen->GetBuffer(), screenData, SCREEN_DATA_SIZE);
        screen->SetBorder(currentLoaderInfo.FinalBorder);
        context.Stage = lsFinish;
        context.FrameCount &= 15;
        context.FrameTarget = context.FrameCount + static_cast<int32_t>(std::round(Current_Environment.PauseLen * 50));
        context.TStateCount = context.TsPerFrame;
    }
}

void ScreenStringLoader::TestReset() {
    Current_Environment.HardwareModel = (THardware)(((int)Current_Environment.HardwareModel + 1) % ((int)hPlus3Usr0 + 1));
    context.Stage = lsReset;
    context.FrameCount &= 15;
    DataType = dtHeader;
    context.FrameTarget = 50;
    PulseNumber = 0;
    ResetAddress = context.Max_Memory;
    context.CurrentBorder = ZxSpectrumColor::White;
    screen->SetBorder(context.CurrentBorder);
    screen->Cls();
    context.ScreenATLine = 0;
    inProgress = true;
}

bool ScreenStringLoader::TestResetFrame() {
    uint32_t lastTs;
    int32_t changeAtTStates;
    int32_t index;
    int64_t loadingByte;
    ZxSpectrumAttribute attribute;
    bool update = false;
    std::string tempStr;

    bool result = false;

    while (context.TStateCount < context.TsPerFrame) {

        switch (context.Stage) {
        case lsReset:
            // printf("Entering lsReset stage\n");
            update = true;
            if (!Reset()) {
                context.Stage = lsFinish;
                context.FrameCount &= 15;
                context.FrameTarget = context.FrameCount + static_cast<int32_t>(std::round(Current_Environment.PauseLen * 50));
                context.TStateCount = context.TsPerFrame;
                update = true;
            }
            break;

        case lsFinish:
            // printf("Entering lsFinish stage\n");
            context.TStateCount = context.TsPerFrame;
            if (context.FrameCount >= context.FrameTarget) {
                // screen->SetBorder(currentLoaderInfo.FinalBorder);
                inProgress = false;
            } else {
                screen->SetBorder(context.CurrentBorder);
                result = true;
            }
            break;

        default:
            context.TStateCount = context.TsPerFrame;
            result = false;
            break;
        }
    }

    if (update) {
        screen->SetBorder(context.CurrentBorder);
        result = true;
    }
    if (context.TStateCount >= context.TsPerFrame) {
        context.TStateCount -= context.TsPerFrame;
    }
    ++context.FrameCount;
    return result;
}


bool ScreenStringLoader::Reset() {
    bool result = true;
    context.CurrentBorder = ZxSpectrumColor::White;
    switch (context.ResetStage) {
    case rsStart:
        if (context.FrameCount >= context.FrameTarget) {
            if (Current_Environment.HardwareModel == h16k || Current_Environment.HardwareModel == h48k ||
                Current_Environment.HardwareModel == h128kUsr0 || Current_Environment.HardwareModel == hPlus2Usr0 ||
                Current_Environment.HardwareModel == hPlus2aUsr0 || Current_Environment.HardwareModel == hPlus3Usr0) {
                ResetAddress = context.Max_Memory;
                context.ResetStage = rsFill2;
            } else {
                ResetAddress = 0;
                context.ResetStage = rs128kClear;
            }
        }
        context.TStateCount = context.TsPerFrame;
        break;

    case rsFill2:
        context.TStateCount += 32;
        if (ResetAddress < 23296) screen->GetBuffer()[ResetAddress - 16384] = 2;
        if (ResetAddress == 16384) context.ResetStage = rsClear;
        else --ResetAddress;
        break;

    case rsClear:
        context.TStateCount += 84;
        if (ResetAddress < 23296) screen->GetBuffer()[ResetAddress - 16384] = 0;
        if (ResetAddress == context.Max_Memory) context.ResetStage = rsCLS;
        ++ResetAddress;
        break;

    case rs128kClear:
        if (ResetAddress < 6912) {
            context.TStateCount += 21;
            screen->GetBuffer()[ResetAddress] = 0;
            ++ResetAddress;
        } else {
            context.ResetStage = rs128kMenu;
            context.FrameTarget = context.FrameCount + 30;
            context.TStateCount = context.TsPerFrame;
            MenuStage = msMessage1;
        }
        break;

    case rs128kMenu:
        if (context.FrameCount >= context.FrameTarget) {
            switch (MenuStage) {
            case msMessage1:
                std::memset(screen->GetBuffer() + 6144, 56, 768);
                if (Current_Environment.HardwareModel == h128k) {
                    screen->DrawText("\x007F 19", 56, 0, 23);
                }
                else if (Current_Environment.HardwareModel == hPlus2) {
                    screen->DrawText("\x007F" "198", 56, 0, 23);
                }
                else {
                    screen->DrawText("\x007F" "1982, 1986, 1987 A", 56, 0, 23);
                }
                MenuStage = msMessage2;
                break;

            case msMessage2:
                if (Current_Environment.HardwareModel == h128k) {
                    screen->DrawText("\x007F 1986 Sinclair Research Ltd", 56, 0, 23);
                    context.FrameTarget = context.FrameCount + 2;
                    MenuStage = msMenuStrip;
                }
                else if (Current_Environment.HardwareModel == hPlus2) {
                    screen->DrawText("\x007F" "1986, " "\x007F" "1982 Amstrad Consumer", 56, 0, 23);
                    MenuStage = msMessage3;
                } 
                else {
                    screen->DrawText("\x007F" "1982, 1986, 1987 Amstrad Plc.", 56, 0, 23);
                    MenuStage = msMessage3;
                    context.FrameTarget = context.FrameCount + 23;
                }
                break;

            case msMessage3:
                if (Current_Environment.HardwareModel == hPlus2) {
                    screen->DrawText("\x007F" "1986, " "\x007F" "1982 Amstrad Consumer", 56, 0, 22);
                    screen->DrawText("              Electronic", 56, 0, 23);
                } 
                else if (Current_Environment.HardwareModel == hPlus2a) {
                    screen->DrawText("\x007F" "1982, 1986, 1987 Amstrad Plc.", 56, 0, 22);
                    screen->DrawText("D                             ", 56, 0, 23);
                } 
                else {
                    screen->DrawText("\x007F" "1982, 1986, 1987 Amstrad Plc.", 56, 0, 22);
                    screen->DrawText("Drive                         ", 56, 0, 23);
                }
                MenuStage = msMessage4;
                break;

            case msMessage4:
                if (Current_Environment.HardwareModel == hPlus2) {
                    screen->DrawText("              Electronics Plc", 56, 0, 23);
                }
                else if (Current_Environment.HardwareModel == hPlus2a) {
                    screen->DrawText("Drive M: available.", 56, 0, 23);
                    context.FrameTarget = context.FrameCount + 2;
                } else if (Current_Environment.HardwareModel == hPlus3) {
                    screen->DrawText("Drives A:, B: and M: available.", 56, 0, 23);
                    context.FrameTarget = context.FrameCount + 2;
                }
                MenuStage = msMenuStrip;
                break;

            case msMenuStrip:
                if (Current_Environment.HardwareModel == h128k) {
                    screen->DrawText("128     ", 71, 7, 7);
                    screen->DrawText("\x0080", 66, 15, 7);
                    screen->DrawText("\x0080", 86, 16, 7);
                    screen->DrawText("\x0080", 116, 17, 7);
                } else if (Current_Environment.HardwareModel == hPlus2) {
                    screen->DrawText("128 ", 71, 7, 7);
                } else if (Current_Environment.HardwareModel == hPlus2a) {
                    screen->DrawText("128 +2A ", 71, 7, 7);
                    screen->DrawText("\x0080", 66, 15, 7);
                    screen->DrawText("\x0080", 86, 16, 7);
                    screen->DrawText("\x0080", 116, 17, 7);
                } else {
                    screen->DrawText("128 +3 ", 71, 7, 7);
                }
                MenuStage = msOptions1;
                break;

            case msOptions1:
                if (Current_Environment.HardwareModel == h128k || Current_Environment.HardwareModel == hPlus2a) {
                    screen->DrawText("\x0080", 101, 18, 7);
                    screen->DrawText("\x0080", 104, 19, 7);
                    screen->DrawText(" ", 0, 20, 7);
                    screen->DrawText(Current_Environment.HardwareModel == h128k ? " Tape Loader  " : " Loader       ", 120, 7, 8);
                    if (Current_Environment.HardwareModel == h128k) screen->DrawText(" 128 BA", 120, 7, 9);
                    else screen->DrawText(" +3 B", 120, 7, 9);
                } else if (Current_Environment.HardwareModel == hPlus2) {
                    screen->DrawText("128     ", 71, 7, 7);
                    screen->DrawText("\x0080", 66, 15, 7);
                    screen->DrawText("\x0080", 86, 16, 7);
                    screen->DrawText("\x0080", 116, 17, 7);
                    screen->DrawText("\x0080", 101, 18, 7);
                    screen->DrawText("\x0080", 104, 19, 7);
                    screen->DrawText(" ", 0, 20, 7);
                    screen->DrawText(" Tape Load", 120, 7, 8);
                } else {
                    screen->DrawText("128 +3  ", 71, 7, 7);
                    screen->DrawText("\x0080", 66, 15, 7);
                    screen->DrawText("\x0080", 86, 16, 7);
                    screen->DrawText("\x0080", 116, 17, 7);
                    screen->DrawText("\x0080", 101, 18, 7);
                    screen->DrawText("\x0080", 104, 19, 7);
                    screen->DrawText(" ", 0, 20, 7);
                    screen->DrawText(" Loader     ", 120, 7, 8);
                }
                MenuStage = msOptions2;
                break;

            case msOptions2:
                if (Current_Environment.HardwareModel == h128k) {
                    screen->DrawText("SIC    ", 120, 14, 9);
                    screen->DrawText(" Calculator   ", 120, 7, 10);
                    screen->DrawText(" 48 BASI", 120, 7, 11);
                } else if (Current_Environment.HardwareModel == hPlus2) {
                    screen->DrawText(" Tape Loader  ", 120, 7, 8);
                    screen->DrawText(" 128 BASIC    ", 120, 7, 9);
                    screen->DrawText(" Calculator", 120, 7, 10);
                } else if (Current_Environment.HardwareModel == hPlus2a) {
                    screen->DrawText(" +3 BASIC     ", 120, 7, 9);
                    screen->DrawText(" Calculator   ", 120, 7, 10);
                    screen->DrawText(" 48", 120, 7, 11);
                } else {
                    screen->DrawText(" Loader       ", 120, 7, 8);
                    screen->DrawText(" +3 BASIC     ", 120, 7, 9);
                    screen->DrawText(" Calculator", 120, 7, 10);
                }
                MenuStage = msOptions3;
                break;

            case msOptions3:
                if (Current_Environment.HardwareModel == h128k) {
                    screen->DrawText("C     ", 120, 15, 11);
                    screen->DrawText(" Tape Tester  ", 120, 7, 12);
                    screen->DrawText("         ", 120, 7, 13);
                } else if (Current_Environment.HardwareModel == hPlus2) {
                    screen->DrawText(" Calculator   ", 120, 7, 10);
                    screen->DrawText(" 48 BASIC     ", 120, 7, 11);
                    screen->DrawText("            ", 120, 7, 12);
                } else if (Current_Environment.HardwareModel == hPlus2a) {
                    screen->DrawText(" 48 BASIC     ", 120, 7, 11);
                    screen->DrawText("            ", 120, 7, 12);
                } else {
                    screen->DrawText(" Calculator   ", 120, 7, 10);
                    screen->DrawText(" 48 BASIC     ", 120, 7, 11);
                    screen->DrawText("         ", 120, 7, 12);
                }
                MenuStage = msLeftBorder;
                break;

            case msLeftBorder:
                for (auto row = 8; row <= (Current_Environment.HardwareModel == h128k ? 12 : 11); ++row)
                {
                    screen->DrawText("\x0081", 120, 7, row);
                }
                if (Current_Environment.HardwareModel == h128k) {
                    screen->DrawText("\x0081             ", 120, 7, 13);
                }
                else {
                    screen->DrawText("\x0082             ", 120, 7, 12);
                }
                MenuStage = msBottom1;
                break;

            case msBottom1:
                if (Current_Environment.HardwareModel == h128k) {
                    screen->DrawText("\x0082\x0083\x0083\x0083\x0083\x0083\x0083", 120, 7, 13);
                } else {
                    screen->DrawText("\x0082\x0083\x0083\x0083\x0083\x0083\x0083\x0083\x0083", 120, 7, 12);
                }
                // spectrumScreen->DrawText("\x0082" + std::string(Current_Environment.HardwareModel == h128k ? 6 : 8, "\x0083"), 120, 7, Current_Environment.HardwareModel == h128k ? 13 : 12);
                MenuStage = msBottom2;
                break;

            case msBottom2:
                if (Current_Environment.HardwareModel == h128k)
                    // screen->DrawText("\x0082" + std::string(13, "\x0083"), 120, 7, 13);
                    screen->DrawText("\x0082\x0083\x0083\x0083\x0083\x0083\x0083\x0083\x0083\x0083\x0083\x0083\x0083\x0083", 120, 7, 13);
                else
                    // screen->DrawText("\x0082" + std::string(12, "\x0083\x0084")), 120, 7, 12);
                    screen->DrawText("\x0082\x0083\x0083\x0083\x0083\x0083\x0083\x0083\x0083\x0083\x0083\x0083\x0083\x0084", 120, 7, 12);
                MenuStage = msRightBorder;
                break;

            case msRightBorder:
                if (Current_Environment.HardwareModel == h128k) {
                    for (auto row = 8; row <= 12; ++row) {
                        screen->DrawText("\x0085", 120, 20, row);
                    }
                    screen->DrawText("\x0084", 120, 20, 13);
                    MenuStage = msHighlight;
                } else {
                    for (auto row = 9; row <= 11; ++row) {
                        screen->DrawText("\x0085", 120, 20, row);
                    }
                    screen->DrawText("\x0084", 120, 20, 12);
                    if (Current_Environment.HardwareModel == hPlus2) {
                        screen->DrawText("\x0081Tape Loader \x0085", 104, 7, 8);
                    } else {
                        screen->DrawText("\x0081Loader      \x0085", 104, 7, 8);
                    }
                    // screen->DrawText("\x0081" + (Current_Environment.HardwareModel == hPlus2 ? "Tape Loader " : "Loader      ") + "\x0085", 104, 7, 8);
                    context.FrameTarget = context.FrameCount + 50;
                    context.ResetStage = rsPause;
                }
                break;

            case msHighlight:
                screen->DrawText("\x0081Tape Loader \x0085", 104, 7, 8);
                context.FrameTarget = context.FrameCount + 50;
                context.ResetStage = rsPause;
                break;
            }
        }
        context.TStateCount = context.TsPerFrame;
        break;

    case rsCLS:
        screen->SetBorder(ZxSpectrumColor::White);
        screen->Cls();
        // std::memset(Display.data(), 0, 6144);
        // std::memset(Display.data() + 6144, 56, 768);
        context.TStateCount = context.TsPerFrame;
        context.ResetStage = rsMessage;
        break;

    case rsMessage:
        if (Current_Environment.HardwareModel == h16k || Current_Environment.HardwareModel == h48k ||
            Current_Environment.HardwareModel == h128kUsr0) {
            screen->DrawText("\x7F 1982 Sinclair Research Ltd", 56, 0, 23);
        } 
        else if (Current_Environment.HardwareModel == hPlus2Usr0 || Current_Environment.HardwareModel == hPlus2aUsr0 ||
                   Current_Environment.HardwareModel == hPlus3Usr0) {
            screen->DrawText("\x7F 1982 Amstrad", 56, 0, 23);
        }
        context.TStateCount = context.TsPerFrame;
        context.FrameTarget = context.FrameCount + 50;
        context.ResetStage = rsPause;
        break;

    case rsPause:
        if (context.FrameCount > context.FrameTarget) {
            result = false;
            context.ResetStage = rsStart;
        }
        context.TStateCount = context.TsPerFrame;
        break;
    }
    return result;
}

// void TerminateSaver(bool Immediate) {
//     if (Immediate) {
//         Stage = lsFinish;
//         FrameTarget = FrameCount + 1;
//         InProgress = false;
//     } else {
//         Last_Stage = Stage;
//         Stage = lsQuit;
//     }
// }

void ScreenStringLoader::DisruptChuntey() {
    if (context.Stage == lsData) {
        ChunteyDisrupted = true;
        WillDisruptChuntey = false;
        ChunteyType = (get_rand_32() % 100) >= 50 ? ctForward : ctBackward;
    }
}

void ScreenStringLoader::AddCommands(const std::vector<std::string>& Commands) {
    context.Commandline.insert(context.Commandline.end(), Commands.begin(), Commands.end());
}

static std::string Marker(uint8_t value) {
    return std::string(1, static_cast<char>(value));
}

void ScreenStringLoader::AddProgramLine(int32_t number, const std::string& typed, const std::string& listed) {
    const std::string lineNumber = std::to_string(number);
    AddCommands({Marker(254), lineNumber + " " + typed, Marker(255), "  " + lineNumber + ">" + listed});
}
