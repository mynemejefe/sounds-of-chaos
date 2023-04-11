#pragma once

#include "fractalsound.h"

class FractalSoundTester
{
public:
	void RunAllTests();
	bool TestConstructor();
	bool TestPianoKey();

private:
	FractalSound* _fractalSound;
};

