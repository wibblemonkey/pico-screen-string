#include <stdio.h>
#include <cstring>

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "ScreenString.hpp"
#include "PicoDisplay28.hpp"
#include "PicoScreenString.hpp"

PicoDisplay28X display;
PicoScreenString simulation(&display, &Sequence, &Shuffled);

void __not_in_flash_func(core1main)() {
    char fpsbuf[512]; // Make sure buffer don't overflow
    int frame_counter; // Counter for number of frames displayed
    uint64_t start, elapsed; // Microsecond timers
    float fps; // Frames per second to display

    frame_counter = 0;

    // Get the start time
    start = time_us_64();

    while(true) {

        frame_counter++;

        // update screen
        simulation.UpdateDisplay();

        if(frame_counter % 200 == 0) {
            elapsed = time_us_64();
            // Get the current time to calculate the elapsed time next
            fps = (frame_counter / float(elapsed - start)) * 1000000;
            sprintf(fpsbuf, "Frame = %d, FPS = %5.2f", frame_counter, fps); 
            printf("%s\n", fpsbuf);
        }
        sleep_us(16666); // Sleep for ~17ms to cap at ~25 FPS
        // sleep_us(1);
    }
}

void __not_in_flash_func(core0main)() {
    simulation.Run();
}

int main()
{
    stdio_init_all();

    simulation.Initialize();

    multicore_launch_core1(core1main);
    core0main();
}
