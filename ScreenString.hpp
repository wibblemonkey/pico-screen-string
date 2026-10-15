#ifndef SCREENSTRING_HPP
#define SCREENSTRING_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "ZxSpectrumColors.hpp"

#include "screens.h"

class ScreenString {
public:
    const std::string Filename;
    const ZxSpectrumColor Border;

    ScreenString(const std::string& filename, ZxSpectrumColor border, const uint8_t* data)
        : Filename(filename), Border(border), Data(data) { }
    ~ScreenString() { }

    inline void Decompress(uint8_t *output) const { dzx0_turbo(Data, output); };

private:
    const uint8_t* Data;
};

const ScreenString AboutData("About", ZxSpectrumColor::Black, About);

const std::vector<ScreenString> Sequence = {
    ScreenString("Sequence01", ZxSpectrumColor::White, Sequence01),
    ScreenString("Sequence02", ZxSpectrumColor::White, Sequence02),
    ScreenString("Sequence03", ZxSpectrumColor::White, Sequence03),
};

const std::vector<ScreenString> Shuffled = {
    ScreenString("Shuffled01", ZxSpectrumColor::White, Shuffled01),
    ScreenString("Shuffled02", ZxSpectrumColor::White, Shuffled02),
    ScreenString("Shuffled03", ZxSpectrumColor::White, Shuffled03),
};

#endif
