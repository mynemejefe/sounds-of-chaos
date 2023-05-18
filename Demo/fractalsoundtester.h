#pragma once

#include "fractalsound.h"

class FractalSoundTester
{
public:
	void RunAllTests();
	void TestConstructor();
	void TestMixChunkCreation();
	void TestCreatingSoundBuffer();
	void TestFillBufferSimple();
	void TestFillBufferAdditive();
	void TestFillBufferAdditiveWithKernel();
	void TestPianoKey();
	void CheckMemoryLeaks();

private:
	int failCount_;
};

