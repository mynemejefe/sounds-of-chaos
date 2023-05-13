#include "fractalsoundtester.h"
#include <iostream>

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
                  << "Expected " << arg << " (" << #arg << ")"      \
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
    TestPianoKey();

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
	FractalSound* fractalSound = new FractalSound(FS);

	ASSERT_NE(fractalSound, NULL);
	EXPECT_EQ(fractalSound->GetFs(), FS);

	fractalSound->~FractalSound();
}

void FractalSoundTester::TestPianoKey() {
	std::cout << "\nTesting PianoKey...\n";

	FractalSound* fractalSound = new FractalSound(44100);

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