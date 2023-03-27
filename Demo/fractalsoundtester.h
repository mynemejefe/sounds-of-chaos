#pragma once

#include "fractalsound.h"

class FractalSoundTester
{
public:
	FractalSoundTester(FractalSound& testedClass) : _testedClass(testedClass) {};
	void FractalSoundConstructor();

private:
	FractalSound _testedClass;
};

