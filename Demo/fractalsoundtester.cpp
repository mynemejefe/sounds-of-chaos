#include "fractalsoundtester.h"
#include <iostream>
#include <sysinfoapi.h>
#include <Psapi.h>
#include <processthreadsapi.h>
#include "fractalutility.h"
#include <string>
#include <mutex>

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

#define CHECK_NEARBY(num1, num2, precision, isAssert)				\
	if ((num1 > num2 && num1 - num2 > precision)					\
		|| (num2 > num1 && num2 - num1 > precision)) {				\
		std::cout << "\nCheck failed while running " << __func__    \
			<< ", at line " << __LINE__ << std::endl				\
			<< "Expected the difference of " << int(num1)			\
			<< " (" << #num1 << ") and "							\
			<< int(num2) << " (" << #num2 << ") to be at most "		\
			<< precision << std::endl;								\
			failCount_++;											\
			if (isAssert) {											\
				return;												\
			}														\
	}

#define ASSERT_EQ(actual, expected) CHECK_EQ(actual, expected, true)
#define ASSERT_NE(actual, expected) CHECK_NE(actual, expected, true)
#define ASSERT_TRUE(arg) CHECK_TRUE(arg, true)
#define ASSERT_FALSE(arg) CHECK_FALSE(arg, true)
#define ASSERT_NEARBY(num1, num2, precision) CHECK_NEARBY(num1, num2, precision, true)
#define EXPECT_EQ(actual, expected) CHECK_EQ(actual, expected, false)
#define EXPECT_NE(actual, expected) CHECK_NE(actual, expected, false)
#define EXPECT_TRUE(arg) CHECK_TRUE(arg, false)
#define EXPECT_FALSE(arg) CHECK_FALSE(arg, false)
#define EXPECT_NEARBY(num1, num2, precision) CHECK_NEARBY(num1, num2, precision, false)

void FractalSoundTester::RunAllTests() {
    std::cout << "\nRunning all tests..." << std::endl;

	failCount_ = 0;

    TestConstructor();
	TestMixChunkCreation();
	TestMakeVectorWithIterationDistances();
	TestKernel();
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

void FractalSoundTester::RunPerformanceTests() {
	const int RepetitionsWithinTest = 100;
	InputVars input;
	FractalVars fractal;
	SoundVars sound;
	input.lastClickPos = glm::vec2(-1, 0);
	fractal.fractalType = 0;
	fractal.power = 2;
	sound.allowCloseNeighbours = true;
	sound.freq = 440;
	sound.normalizationLevel = 0;
	sound.soundGenerationMode = 1;
	FractalSound* fractalSound = new FractalSound(44100);
	float* soundBuffer = new float[2 * 441000];

	sound.maxSoundIterations = 50;
	for (int i = 0; i < 20; i++)
	{
		auto t1 = std::chrono::system_clock::now();

		for (size_t i = 0; i < RepetitionsWithinTest; i++)
		{
			fractalSound->FillBufferAdditive(input, fractal, sound, soundBuffer);
		}

		auto t2 = std::chrono::system_clock::now();
		auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
		auto ms = microseconds.count();
		std::cout << "Average time of " << RepetitionsWithinTest << " repetitions: "
			<< ms / RepetitionsWithinTest << "\tmicroseconds at " << sound.maxSoundIterations << " fractal iterations\n";

		sound.maxSoundIterations += 50;
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

void FractalSoundTester::TestMakeVectorWithIterationDistances() {
	std::cout << "\nTesting the correctness of the fractal algorithm calculations...\n";

	InputVars input;
	FractalVars fractal;
	SoundVars sound;
	fractal.maxIterations = 10;
	FractalSound* fractalSound = new FractalSound(44100);
	std::vector<float> expectedDistances;

	ASSERT_NE(fractalSound, NULL);

	// Mandelbrot tests
	fractal.fractalType = 0;
	fractal.power = 2;
	input.lastClickPos = glm::vec2(-1, 0);
	auto distances = FractalUtility::MakeVectorWithIterationDistances(input, fractal);
	EXPECT_EQ(distances.size(), fractal.maxIterations)
	for (int i = 0; i < distances.size(); i++)
	{
		if (i % 2 == 0) {
			EXPECT_EQ(distances[i], 0);
		}
		else {
			EXPECT_EQ(distances[i], 1);
		}
	}

	input.lastClickPos = glm::vec2(0.25, -0.25);
	distances = FractalUtility::MakeVectorWithIterationDistances(input, fractal, 3);
	expectedDistances = { 0.45069, 0.47005, 0.40997 };
	EXPECT_EQ(distances.size(), expectedDistances.size())
	for (int i = 0; i < distances.size(); i++)
	{
		EXPECT_NEARBY(distances[i], expectedDistances[i], 0.00001);
	}

	input.lastClickPos = glm::vec2(1, 0);
	expectedDistances = { 2, 5 };
	// when a value is greater than 2, the algorithm returns early
	distances = FractalUtility::MakeVectorWithIterationDistances(input, fractal, expectedDistances.size() + 1); 
	EXPECT_EQ(distances.size(), expectedDistances.size())
	for (int i = 0; i < distances.size(); i++)
	{
		EXPECT_NEARBY(distances[i], expectedDistances[i], 0.00001);
	}

	fractal.fractalType = 0;
	fractal.power = 3;
	input.lastClickPos = glm::vec2(0, 1);
	distances = FractalUtility::MakeVectorWithIterationDistances(input, fractal);
	EXPECT_EQ(distances.size(), fractal.maxIterations)
	for (int i = 0; i < distances.size(); i++)
	{
		if (i % 2 == 0) {
			EXPECT_EQ(distances[i], 0);
		}
		else {
			EXPECT_EQ(distances[i], 1);
		}
	}

	input.lastClickPos = glm::vec2(0, -1);
	distances = FractalUtility::MakeVectorWithIterationDistances(input, fractal);
	EXPECT_EQ(distances.size(), fractal.maxIterations)
	for (int i = 0; i < distances.size(); i++)
	{
		if (i % 2 == 0) {
			EXPECT_EQ(distances[i], 0);
		}
		else {
			EXPECT_EQ(distances[i], 1);
		}
	}

	// Burning ship tests
	fractal.fractalType = 1;
	fractal.power = 2;
	input.lastClickPos = glm::vec2(-1, 0);
	distances = FractalUtility::MakeVectorWithIterationDistances(input, fractal);
	EXPECT_EQ(distances.size(), fractal.maxIterations)
	for (int i = 0; i < distances.size(); i++)
	{
		if (i % 2 == 0) {
			EXPECT_EQ(distances[i], 0);
		}
		else {
			EXPECT_EQ(distances[i], 1);
		}
	}

	input.lastClickPos = glm::vec2(0.25, -1);
	distances = FractalUtility::MakeVectorWithIterationDistances(input, fractal, 3);
	expectedDistances = { 0.85009, 0.56662, 0.79851 };
	EXPECT_EQ(distances.size(), expectedDistances.size())
	for (int i = 0; i < distances.size(); i++)
	{
		EXPECT_NEARBY(distances[i], expectedDistances[i], 0.00001);
	}

	input.lastClickPos = glm::vec2(0, 1);
	expectedDistances = { sqrtf(2), 3 };
	distances = FractalUtility::MakeVectorWithIterationDistances(input, fractal, expectedDistances.size() + 1);
	EXPECT_EQ(distances.size(), expectedDistances.size())
		for (int i = 0; i < distances.size(); i++)
		{
			EXPECT_NEARBY(distances[i], expectedDistances[i], 0.00001);
		}

	fractalSound->~FractalSound();
}

void FractalSoundTester::TestKernel()
{
	std::cout << "\nTesting kernel function...\n";

	FractalSound* fractalSound = new FractalSound(44100);

	ASSERT_NE(fractalSound, NULL);

	std::vector<float> distances = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	std::vector<int> kernel = { 0, 1, 0 };
	std::vector<float> distancesWithKernel;
	std::vector<float> expectedDistances = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

	distancesWithKernel = fractalSound->ApplyKernelToDistances(distances, kernel);

	ASSERT_EQ(distances.size(), distancesWithKernel.size());
	for (int i = 0; i < distances.size(); i++)
	{
		EXPECT_EQ(distancesWithKernel[i], expectedDistances[i]);
	}

	distances = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	kernel = { 1, 2, 1 };
	expectedDistances = { 4.f/3, 2, 3, 4, 5, 6, 7, 8, 9, 29.f/3 };

	distancesWithKernel = fractalSound->ApplyKernelToDistances(distances, kernel);

	ASSERT_EQ(distances.size(), distancesWithKernel.size());
	for (int i = 0; i < distances.size(); i++)
	{
		EXPECT_EQ(distancesWithKernel[i], expectedDistances[i]);
	}

	distances = { 5, 0, 3, 4, 5, 7, 3, 8, 9, 0 };
	kernel = { 1, 3, 2, 0, 1 };
	expectedDistances = { 13.f/3, 19.f/6, 16.f/7, 24.f/7, 28.f/7, 41.f/7, 41.f/7, 32.f/7, 45.f/6, 35.f/6 };

	distancesWithKernel = fractalSound->ApplyKernelToDistances(distances, kernel);

	ASSERT_EQ(distances.size(), distancesWithKernel.size());
	for (int i = 0; i < distances.size(); i++)
	{
		EXPECT_EQ(distancesWithKernel[i], expectedDistances[i]);
	}

	// Checking errors
	distances = { 1 };
	kernel = { 1,2,3 };

	try {
		distancesWithKernel = fractalSound->ApplyKernelToDistances(distances, kernel);
	}
	catch (const std::runtime_error& e) {
		std::string message = std::string(e.what());
		std::string expectedMessage = "The distances vector is too small for the kernel!";
		EXPECT_TRUE(message.compare(expectedMessage) == 0);
	}

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
	input.lastClickPos = glm::vec2(-1, 0);
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
	std::cout << "\nChecking potential memory leak locations...\n\n"
		<< "   Consumed total virtual memory by current process:\n";

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

	std::cout
		<< "   Before test 1:\t" << virtualMemUsedByProcess1 << " Bytes\n"
		<< "   After test 1:\t" << virtualMemUsedByProcess2 << " Bytes\n";
	EXPECT_NEARBY(virtualMemUsedByProcess1, virtualMemUsedByProcess2, 0.05 * virtualMemUsedByProcess2);

	// Checking for memory leaks when re-recording sounds

	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	virtualMemUsedByProcess1 = pmc.PrivateUsage;

	input.isShiftHeldDown = true;
	for (int i = 0; i < 1000; i++) {
		fractalSound->UsePianoKey(input, fractal, sound, 0);
	}

	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	virtualMemUsedByProcess2 = pmc.PrivateUsage;

	std::cout
		<< "   Before test 2:\t" << virtualMemUsedByProcess1 << " Bytes\n"
		<< "   After test 2:\t" << virtualMemUsedByProcess2 << " Bytes\n";
	EXPECT_NEARBY(virtualMemUsedByProcess1, virtualMemUsedByProcess2, 0.05 * virtualMemUsedByProcess2);
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