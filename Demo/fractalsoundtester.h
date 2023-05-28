#pragma once

#include "fractalsound.h"

class FractalSoundTester
{
public:
	void RunAllTests();

private:
	void TestConstructor();
	void TestMixChunkCreation();
	void TestMakeVectorWithIterationDistances();
	void TestKernel();
	void TestPianoKey();
	void CheckMemoryLeaks();

	int failCount_;
};

