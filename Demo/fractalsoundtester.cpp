#include "fractalsoundtester.h"
#include <iostream>

#define ASSERT_EQ(actual, expected)                                 \
    if(actual != expected){                                         \
        std::cout << "Assert failed while running " << __func__     \
                  << ", at line " << __LINE__ << std::endl          \
                  << "Expected: " << expected                       \
                  << "(" << #expected << ")" << std::endl           \
                  << "Actual: " << actual                           \
                  << "(" << #actual << ")" << std::endl;            \
        return false;                                               \
    }

#define ASSERT_NE(actual, expected)                                 \
    if(actual == expected){                                         \
        std::cout << "Assert failed while running " << __func__     \
                  << ", at line " << __LINE__ << std::endl          \
                  << "Expected: " << expected                       \
                  << "(" << #expected << ")" << std::endl           \
                  << "Actual: " << actual                           \
                  << "(" << #actual << ")" << std::endl;            \
        return false;                                               \
    }

#define ASSERT_TRUE(arg)                                            \
    if(!(arg)) {                                                    \
        std::cout << "Assert failed while running " << __func__     \
                  << ", at line " << __LINE__ << std::endl          \
                  << "Expected " << arg << "(" << #arg << ")"       \
                  << " to be true, but it was false" << std::endl;  \
        return false;                                               \
    }

#define ASSERT_FALSE(arg)                                           \
    if(arg) {                                                       \
        std::cout << "Assert failed while running " << __func__     \
                  << ", at line " << __LINE__ << std::endl          \
                  << "Expected " << arg << "(" << #arg << ")"       \
                  << " to be false, but it was true" << std::endl;  \
        return false;                                               \
    }

void FractalSoundTester::RunAllTests() {
    bool failed = false;

    std::cout << "\nRunning all tests..." << std::endl;
    failed = failed || TestConstructor();
    failed = failed || TestPianoKey();

    std::cout << "Testing completed. ";
    if (failed) {
        std::cout << "There were one or more failed tests." << std::endl;
    }
    else {
        std::cout << "All tests were successful." << std::endl;
    }
    
}

bool FractalSoundTester::TestConstructor()
{
	std::cout << "Testing constructor..." << std::endl;

	ASSERT_EQ(_fractalSound, NULL);

	int FS = 10;
	_fractalSound = new FractalSound(FS);

	ASSERT_NE(_fractalSound, NULL);
	ASSERT_EQ(_fractalSound->GetFs(), FS);

	_fractalSound->~FractalSound();

	return true;
}

bool FractalSoundTester::TestPianoKey() {
	std::cout << "Testing PianoKey..." << std::endl;

	_fractalSound = new FractalSound(44100);

	for (const auto& key : _fractalSound->pianoKeys) {
		ASSERT_EQ(key.soundToPlay, NULL);
		ASSERT_FALSE(key.isFilled);
	}

	InputVars input;
	FractalVars fractal;
	SoundVars sound;
	input.lastClickPos = glm::vec2(1, 0);
	input.isShiftHeldDown = true;

	//Invalid arguments for n
	ASSERT_FALSE(_fractalSound->UsePianoKey(input, fractal, sound, -1));
	ASSERT_FALSE(_fractalSound->UsePianoKey(input, fractal, sound, 10));
	ASSERT_FALSE(_fractalSound->UsePianoKey(input, fractal, sound, INT_MAX));

	for (const auto& key : _fractalSound->pianoKeys) {
		ASSERT_EQ(key.soundToPlay, NULL);
		ASSERT_FALSE(key.isFilled);
	}

	//Filling only the first key
	ASSERT_TRUE(_fractalSound->UsePianoKey(input, fractal, sound, 0));

	bool first = true;
	for (const auto& key : _fractalSound->pianoKeys) {
		if (first) {
			ASSERT_NE(key.soundToPlay, NULL); // needs assert
			ASSERT_TRUE(key.isFilled);
			ASSERT_NE(key.soundToPlay->abuf, NULL); // needs assert

			auto actualBuff = key.soundToPlay->abuf;
			auto expectedBuff = (Uint8*)_fractalSound->CreateSoundBufferFromLastPos(input, fractal, sound);
			for (int i = 0; i < key.soundToPlay->alen; i++) {
				ASSERT_EQ(actualBuff[i], expectedBuff[i]);
			}

			first = false;
		}
		else {
			ASSERT_EQ(key.soundToPlay, NULL);
			ASSERT_FALSE(key.isFilled);
		}
	}

	//Playing an already recorded key
	input.isShiftHeldDown = false;
	ASSERT_TRUE(_fractalSound->UsePianoKey(input, fractal, sound, 0));

	first = true;
	for (const auto& key : _fractalSound->pianoKeys) {
		if (first) {
			ASSERT_NE(key.soundToPlay, NULL); // needs assert
			ASSERT_TRUE(key.isFilled);
			ASSERT_NE(key.soundToPlay->abuf, NULL); // needs assert

			auto actualBuff = key.soundToPlay->abuf;
			auto expectedBuff = (Uint8*)_fractalSound->CreateSoundBufferFromLastPos(input, fractal, sound);
			for (int i = 0; i < key.soundToPlay->alen; i++) {
				ASSERT_EQ(actualBuff[i], expectedBuff[i]);
			}

			first = false;
		}
		else {
			ASSERT_EQ(key.soundToPlay, NULL);
			ASSERT_FALSE(key.isFilled);
		}
	}

	//Filling all keys
	input.isShiftHeldDown = true;
	input.lastClickPos = glm::vec2(-0.058, -0.81);
	for (int i = 1; i < 10; i++) {
		ASSERT_TRUE(_fractalSound->UsePianoKey(input, fractal, sound, i));
	}

	for (const auto& key : _fractalSound->pianoKeys) {
		ASSERT_NE(key.soundToPlay, NULL); // needs assert
		ASSERT_TRUE(key.isFilled);
		ASSERT_NE(key.soundToPlay->abuf, NULL); // needs assert

		auto actualBuff = key.soundToPlay->abuf;
		auto expectedBuff = (Uint8*)_fractalSound->CreateSoundBufferFromLastPos(input, fractal, sound);
		for (int i = 0; i < key.soundToPlay->alen; i++) {
			ASSERT_EQ(actualBuff[i], expectedBuff[i]);
		}
	}

	_fractalSound->~FractalSound();

	return true;
}