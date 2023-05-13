#pragma once

#include "fractalsound.h"

class FractalSoundTester
{
public:
	void RunAllTests();
	void TestConstructor();
	void TestPianoKey();

private:
	int failCount_;
};

