#include "fractalsoundtester.h"
#include <iostream>
#include <sysinfoapi.h>
#include <Psapi.h>
#include <processthreadsapi.h>

#define CHECK_EQ(actual, expected, isAssert)                        \
    if(actual != expected){                                         \
        std::cout << "\nCheck failed while running " << __func__    \
                  << ", at line " << __LINE__ << std::endl          \
                  << "Expected: " << int(expected)                  \
                  << " (" << #expected << ")" << std::endl          \
                  << "Actual: " << int(actual)                      \
                  << " (" << #actual << ")" << std::endl;           \
		failCount_++;												\
        if(isAssert) {												\
			return;													\
		}															\
    }

#define CHECK_NE(actual, expected, isAssert)                        \
    if(actual == expected){                                         \
        std::cout << "\nCheck failed while running " << __func__    \
                  << ", at line " << __LINE__ << std::endl          \
                  << "Expected: " << int(expected)                  \
                  << " (" << #expected << ")" << std::endl          \
                  << "Actual: " << int(actual)                      \
                  << " (" << #actual << ")" << std::endl;           \
		failCount_++;												\
        if(isAssert) {												\
			return;													\
		}															\
    }

#define CHECK_TRUE(arg, isAssert)                                   \
    if(!(arg)) {                                                    \
        std::cout << "\nCheck failed while running " << __func__    \
                  << ", at line " << __LINE__ << std::endl          \
                  << "Expected " << (arg) << " (" << #arg << ")"      \
                  << " to be true, but it was false" << std::endl;  \
		failCount_++;												\
        if(isAssert) {												\
			return;													\
		}															\
    }

#define CHECK_FALSE(arg, isAssert)                                  \
    if(arg) {                                                       \
        std::cout << "\nCheck failed while running " << __func__    \
                  << ", at line " << __LINE__ << std::endl          \
                  << "Expected " << arg << " (" << #arg << ")"      \
                  << " to be false, but it was true" << std::endl;  \
		failCount_++;												\
        if(isAssert) {												\
			return;													\
		}															\
    }

#define ASSERT_EQ(actual, expected) CHECK_EQ(actual, expected, true)
#define ASSERT_NE(actual, expected) CHECK_NE(actual, expected, true)
#define ASSERT_TRUE(arg) CHECK_TRUE(arg, true)
#define ASSERT_FALSE(arg) CHECK_FALSE(arg, true)
#define EXPECT_EQ(actual, expected) CHECK_EQ(actual, expected, false)
#define EXPECT_NE(actual, expected) CHECK_NE(actual, expected, false)
#define EXPECT_TRUE(arg) CHECK_TRUE(arg, false)
#define EXPECT_FALSE(arg) CHECK_FALSE(arg, false)

void FractalSoundTester::RunAllTests() {
    std::cout << "\nRunning all tests..." << std::endl;

	failCount_ = 0;
    TestConstructor();
	TestMixChunkCreation();
    TestPianoKey();
	CheckMemoryLeaks();

    std::cout << "\nTesting completed. ";
    if (failCount_ == 0) {
		std::cout << "All tests were successful." << std::endl;
    }
    else {
		std::cout << "There were " << failCount_ << " failed tests." << std::endl;
    }
    
}

void FractalSoundTester::TestConstructor()
{
	std::cout << "\nTesting constructor...\n";

	int FS = 10;
	int defaultFS = 44100;
	FractalSound* fractalSound = new FractalSound(FS);

	ASSERT_NE(fractalSound, NULL);
	EXPECT_EQ(fractalSound->GetFs(), FS);

	fractalSound = new FractalSound(-10);
	ASSERT_NE(fractalSound, NULL);
	EXPECT_EQ(fractalSound->GetFs(), defaultFS);

	fractalSound->~FractalSound();
}

void FractalSoundTester::TestPianoKey() {
	std::cout << "\nTesting PianoKeys...\n";

	FractalSound* fractalSound = new FractalSound(44100);

	ASSERT_NE(fractalSound, NULL);

	for (const auto& key : fractalSound->pianoKeys) {
		ASSERT_EQ(key.soundToPlay, NULL);
		EXPECT_FALSE(key.isFilled);
	}

	InputVars input;
	FractalVars fractal;
	SoundVars sound;
	input.lastClickPos = glm::vec2(1, 0);
	input.isShiftHeldDown = true;

	//Invalid arguments for n
	EXPECT_FALSE(fractalSound->UsePianoKey(input, fractal, sound, -1));
	EXPECT_FALSE(fractalSound->UsePianoKey(input, fractal, sound, 10));
	EXPECT_FALSE(fractalSound->UsePianoKey(input, fractal, sound, INT_MAX));

	for (const auto& key : fractalSound->pianoKeys) {
		ASSERT_EQ(key.soundToPlay, NULL);
		EXPECT_FALSE(key.isFilled);
	}

	//Filling only the first key
	EXPECT_TRUE(fractalSound->UsePianoKey(input, fractal, sound, 0));

	bool first = true;
	for (const auto& key : fractalSound->pianoKeys) {
		if (first) {
			ASSERT_NE(key.soundToPlay, NULL);
			EXPECT_TRUE(key.isFilled);
			ASSERT_NE(key.soundToPlay->abuf, NULL);

			auto actualBuff = key.soundToPlay->abuf;
			auto expectedBuff = (Uint8*)fractalSound->CreateSoundBufferFromLastPos(input, fractal, sound);
			for (int i = 0; i < key.soundToPlay->alen; i++) {
				EXPECT_EQ(actualBuff[i], expectedBuff[i]);
			}

			first = false;
		}
		else {
			ASSERT_EQ(key.soundToPlay, NULL);
			EXPECT_FALSE(key.isFilled);
		}
	}

	//Playing an already recorded key
	input.isShiftHeldDown = false;
	EXPECT_TRUE(fractalSound->UsePianoKey(input, fractal, sound, 0));

	first = true;
	for (const auto& key : fractalSound->pianoKeys) {
		if (first) {
			ASSERT_NE(key.soundToPlay, NULL);
			EXPECT_TRUE(key.isFilled);
			ASSERT_NE(key.soundToPlay->abuf, NULL);

			auto actualBuff = key.soundToPlay->abuf;
			auto expectedBuff = (Uint8*)fractalSound->CreateSoundBufferFromLastPos(input, fractal, sound);
			for (int i = 0; i < key.soundToPlay->alen; i++) {
				EXPECT_EQ(actualBuff[i], expectedBuff[i]);
			}

			first = false;
		}
		else {
			ASSERT_EQ(key.soundToPlay, NULL);
			EXPECT_FALSE(key.isFilled);
		}
	}

	//Filling all keys
	input.isShiftHeldDown = true;
	input.lastClickPos = glm::vec2(-0.058, -0.81);
	for (int i = 0; i < 10; i++) {
		EXPECT_TRUE(fractalSound->UsePianoKey(input, fractal, sound, i));
	}

	for (const auto& key : fractalSound->pianoKeys) {
		ASSERT_NE(key.soundToPlay, NULL);
		EXPECT_TRUE(key.isFilled);
		ASSERT_NE(key.soundToPlay->abuf, NULL);

		auto actualBuff = key.soundToPlay->abuf;
		auto expectedBuff = (Uint8*)fractalSound->CreateSoundBufferFromLastPos(input, fractal, sound);
		for (int i = 0; i < key.soundToPlay->alen; i++) {
			EXPECT_EQ(actualBuff[i], expectedBuff[i]);
		}
	}

	fractalSound->~FractalSound();
}

void FractalSoundTester::CheckMemoryLeaks() {
	std::cout << "\nChecking potential memory leak locations...\n";

	FractalSound* fractalSound = new FractalSound(44100);

	ASSERT_NE(fractalSound, NULL);

	InputVars input;
	FractalVars fractal;
	SoundVars sound;
	input.lastClickPos = glm::vec2(0.126, -0.611);
	sound.maxSoundIterations = 5; // fewer iterations for faster completion

	// Checking for memory leaks while calling PlaySoundAtPos

	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	SIZE_T virtualMemUsedByProcess1 = pmc.PrivateUsage;

	for (int i = 0; i < 1000; i++)
	{
		fractalSound->PlaySoundAtPos(input, fractal, sound);
	}

	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	SIZE_T virtualMemUsedByProcess2 = pmc.PrivateUsage;

	EXPECT_TRUE(virtualMemUsedByProcess2 < 1.01 * virtualMemUsedByProcess1);
	std::cout
		<< "Before test 1: " << virtualMemUsedByProcess1 << "Bytes"
		<< ", after test 1: " << virtualMemUsedByProcess2 << "Bytes" << "\n";

	// Checking for memory leaks when re-recording sounds

	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	virtualMemUsedByProcess1 = pmc.PrivateUsage;

	input.isShiftHeldDown = true;
	for (int i = 0; i < 1000; i++) {
		fractalSound->UsePianoKey(input, fractal, sound, 0);
	}

	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	virtualMemUsedByProcess2 = pmc.PrivateUsage;

	EXPECT_TRUE(virtualMemUsedByProcess2 < 1.01 * virtualMemUsedByProcess1);
	std::cout
		<< "Before test 2: " << virtualMemUsedByProcess1 << "Bytes"
		<< ", after test 2: " << virtualMemUsedByProcess2 << "Bytes" << "\n";

}

void FractalSoundTester::TestMixChunkCreation()
{
	std::cout << "\nTesting creation of MixChunk objects...\n";

	int firstFS = 10;
	int secondFS = 369;

	FractalSound* fractalSound1 = new FractalSound(firstFS);
	FractalSound* fractalSound2 = new FractalSound(secondFS);

	ASSERT_NE(fractalSound1, NULL);
	ASSERT_NE(fractalSound2, NULL);

	// Testing with valid parameters

	auto chunk1 = fractalSound1->CreateMixChunk(128);
	auto chunk2 = fractalSound2->CreateMixChunk(128);

	ASSERT_NE(chunk1, NULL);
	ASSERT_NE(chunk2, NULL);
	EXPECT_EQ(chunk1->volume, 128);
	EXPECT_EQ(chunk2->volume, 128);
	EXPECT_EQ(chunk1->alen, 4*2*firstFS);
	EXPECT_EQ(chunk2->alen, 4*2* secondFS);
	// Can't test chunk1->abuf because its type is Uint8* and if I used chunk1->alen(the length of the buffer), the test would have no purpose.

	// Testing with invalid parameters

	auto chunk3 = fractalSound1->CreateMixChunk(MIX_MAX_VOLUME + 1); // default value for volume is MIX_MAX_VOLUME
	auto chunk4 = fractalSound1->CreateMixChunk(-1);

	EXPECT_EQ(chunk3->volume, MIX_MAX_VOLUME);
	EXPECT_EQ(chunk4->volume, MIX_MAX_VOLUME);
	
	fractalSound1->~FractalSound();
	fractalSound2->~FractalSound();
}