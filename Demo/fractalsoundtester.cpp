#include "fractalsoundtester.h"
#include <iostream>

void FractalSoundTester::FractalSoundConstructor()
{
    int FS = 10;
    FractalSound* fractalSound = new FractalSound(FS);

    if (fractalSound->GetFs() == FS) {
        std::cout << "Test run successfully" << std::endl;
    }

    fractalSound->~FractalSound();
}
