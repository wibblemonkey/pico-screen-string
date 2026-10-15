# pico-screen-string

A ZX Spectrum loading screen simulator for the Raspberry Pi Pico.

## Background

This project came about as a random thought after breakfast on a rainy Sunday
morning. I had hoped to have a Spectrum or Spectrum Next project finished before
attending Crash Live in October 2026, but life had got in the way. I had found a
Raspberry Pi Pico left over from an earlier unfinished project in a drawer the
previous day, and after checking out some LCD displays online thought that a
conference badge that simulated the Speccy loading screen experience would be a
fun thing to do.

The code is based on [Paul Dunn](https://github.com/ZXDunny)'s excellent Delphi
[screensaver](https://github.com/ZXDunny/Screen-) - a mainstay of my desktop for
many years - ported to C++ and hacked until I got it doing what I wanted it to do.
I am not a C++ programmer - I learned it back when it was mostly just "C with
classes", and my day job is .NET & C# - so the resulting code isn't pretty, but
it's the best I could do with the assistance of Google, and when I got really
frustrated, CoPilot.

The screens are ZX Spectrum native format screen data - I've included some
placeholders here, you'll need to download your own `.scr` files from one of the
many places where they live online, _e.g._

- [ZXArt](https://zxart.ee/)
- [Spectrum Computing](https://spectrumcomputing.co.uk/)
- [World of Spectrum](https://worldofspectrum.org/)

Simply dump them in the `screens` folder and make sure the filenames are no
longer than 10 characters, and don't contain any non alphanumeric characters.
You'll also need to add them to either the `Sequence` or `Shuffled` vectors
in `ScreenString.hpp`, _e.g._

```C++
const std::vector<ScreenString> Shuffled = {
    ScreenString("Knight Lore", ZxSpectrumColor::Black, KnightLore),
    //ScreenString("Display Name", {Border Color}, Filename),
};
```

Given that there's limited flash storage on the Pico, the screen data is
compressed with [Einar Saukas](https://github.com/einar-saukas)'s [ZX0](https://github.com/einar-saukas/ZX0) 
compressor. The ARM assembly decompression routine was ported from the original
Z80 assembly by CoPilot - it seems to work.

## Components

You'll need the following:

- A [Raspberry Pi Pico](https://www.raspberrypi.com/products/raspberry-pi-pico/) or compatible RP2040 board.
- A [Pimoroni Pico Display Pack 2.8"](https://shop.pimoroni.com/products/pico-display-pack-2-8?variant=42047194005587) LCD Display

## Building

### Windows

I mostly use Windows, so that meand VS Code and the
[Raspberry Pi Pico VS Code Extension](https://marketplace.visualstudio.com/items?itemName=raspberry-pi.raspberry-pi-pico).

Once you've cloned the repository, you'll need to get the [ZXO]() submodule:
```git
git submodule update --init
```

You'll then need to import the project using the Pi Pico Extension, and should
then be able to compile it.

### Linux

I haven't tried to build it on Linux yet - I'll update this readme with
instructions once I figure it out.

## Next Steps

I wasn't able to get this to the point I would have liked before setting off for
Crash Live - there are some features in the original screen saver that I had to
disable to get it to do what I wanted for that event, but I intend to try to
finish it off at some point when I get back.

## License

ZXDunny's original project is GPL v3 licensed, so this is as well. Except for
the bits hacked out of the Pimoroni SDK, which maintain their MIT license.
