#include "fractalsound.h"
#include <SDL/SDL_mixer.h>
#include <thread>
#include <synchapi.h>
#include "fractalutility.h"
#include <stdexcept>

FractalSound::FractalSound(int fs) {
	if (0 < fs) {
		fs_ = fs;
	}
	else {
		fs_ = 44100; // Default value
	}

	if (-1 == Mix_OpenAudio(fs_, AUDIO_F32, 2, 512))
	{
		return;
	}
}

FractalSound::~FractalSound() {
	Mix_CloseAudio();
}

void FractalSound::PlaySoundAtPos(InputVars inputVars, FractalVars fractalVars, SoundVars soundVars)
{
#ifndef TESTING
	std::thread task([this, inputVars, fractalVars, soundVars]() {
#endif
		Mix_Chunk* chunk = CreateMixChunk(soundVars.soundVolume);

		bool partOfFractal;
		switch (soundVars.soundGenerationMode) {
		case 0:
			partOfFractal = FillBufferSimple(inputVars, fractalVars, soundVars, (float*)chunk->abuf);
			break;
		case 1: default:
			partOfFractal = FillBufferAdditive(inputVars, fractalVars, soundVars, (float*)chunk->abuf);
			break;
		}

		if (!soundVars.allowCloseNeighbours && !partOfFractal) {
			FractalSound::Mix_FreeChunk(chunk);
			return;
		}

		FractalSound::PlaySoundFromMixChunk(chunk);
#ifndef TESTING
		Sleep(2000);
#endif
		FractalSound::Mix_FreeChunk(chunk);
#ifndef TESTING
	});

	task.detach();
#endif
}

// Make sure to deallocate the return value when you don't need it anymore
float* FractalSound::CreateSoundBufferFromLastPos(InputVars inputVars, FractalVars fractalVars, SoundVars soundVars)
{
	//A Mix_Chunk's buffer length is 4*2*FS but its type is Uint8 (1 byte), float is 4 bytes
	float* soundBuffer = new float[2 * fs_];

	switch (soundVars.soundGenerationMode) {
	case 0:
		FillBufferSimple(inputVars, fractalVars, soundVars, soundBuffer);
		break;
	case 1: default:
		FillBufferAdditive(inputVars, fractalVars, soundVars, soundBuffer);
		break;
	}

	return soundBuffer;
}

bool FractalSound::UsePianoKey(InputVars inputVars, FractalVars fractalVars, SoundVars soundVars, int n)
{
	if (n < 0 || n >= 10)
		return false;

	auto key = &pianoKeys[n];
	// Shift held down		= record new sound
	// Shift not held down	= play recorded sound
	if (inputVars.isShiftHeldDown)
	{
		//Freeing up unused buffers
		if (key->isFilled) {
			delete(key->soundToPlay->abuf);
		}
		else {
			key->soundToPlay = CreateMixChunk(soundVars.soundVolume);
			key->isFilled = true;
		}
		key->soundToPlay->abuf = (Uint8*)CreateSoundBufferFromLastPos(inputVars, fractalVars, soundVars);
		return true;
	}
	if (key->isFilled)
	{
		PlaySoundFromMixChunk(key->soundToPlay);
		return true;
	}

	return false;
}

void FractalSound::PlaySoundFromMixChunk(Mix_Chunk* chunkToPlay)
{
#ifdef TESTING
	return;
#else
	Mix_PlayChannel(-1, chunkToPlay, 0);
#endif
}

bool FractalSound::FillBufferSimple(InputVars inputVars, FractalVars fractalVars, SoundVars soundVars, float buff[]) {
	int maxIterations = soundVars.maxSoundIterations;
	int i = 0, len = fs_;
	float sinConst = 2 * (float)M_PI * soundVars.freq / fs_;
	bool partOfTheSet = false;

	auto distances = FractalUtility::MakeVectorWithIterationDistances(inputVars, fractalVars, maxIterations);

	if (distances.size() == maxIterations) {
		partOfTheSet = true;
	}

	for (i = 0; i < distances.size(); i++)
	{
		float sin = sinf(sinConst * i) * 0.25;

		buff[2 * i] = sin * distances[i]; //left channel
		buff[2 * i + 1] = buff[2 * i]; //right channel
	}

	if (i != len && i != 0) {
		//fill up rest of buffer
		int repeats = len / i - 1;
		int original_iterations = i;
		int j = 0;
		for (; i < len; i++) {
			buff[2 * i] = buff[2 * j];
			buff[2 * i + 1] = buff[2 * j];
			j = j % original_iterations + 1;
		}
	}

	return partOfTheSet;
}

bool FractalSound::FillBufferAdditive(InputVars inputVars, FractalVars fractalVars, SoundVars soundVars, float buff[])
{
	int maxIterations = soundVars.maxSoundIterations;
	int i = 0, len = fs_;
	float sin, sinConst = 2 * (float)M_PI * soundVars.freq / fs_;
	bool partOfTheSet = false;

	auto distances = FractalUtility::MakeVectorWithIterationDistances(inputVars, fractalVars, maxIterations);

	if (distances.size() == maxIterations) {
		partOfTheSet = true;
	}

	if (soundVars.soundGenerationMode == 2) {
		try {
			std::vector<int> kernel = { 1, 2, 4, 8, 4, 2, 1 };
			auto distancesWithKernel = ApplyKernelToDistances(distances, kernel);
			distances = std::move(distancesWithKernel);
		}
		catch (const std::runtime_error& e) {}
	}
	
	for (int j = 0; j < len; j++) {
		buff[2 * j] = 0; //left channel
		buff[2 * j + 1] = 0; //right channel
	}

	int waveCount = min(maxIterations, distances.size());
	for (i = 0; i < waveCount; i++)
	{
		for (int j = 0; j < len; j++) {
			sin = sinf(sinConst * distances[i] * j) * 0.25;

			buff[2 * j] += sin;
		}
	}

	float divisor = pow(i, soundVars.normalizationLevel);

	if (divisor != 1) {
		for (int j = 0; j < len; j++) {
			buff[2 * j] = buff[2 * j] / divisor;
		}
	}

	// Copying values from "left ear" channel to "right ear" channel
	for (int i = 0; i < len; i++)
	{
		buff[2 * i + 1] = buff[2 * i];
	}

	return partOfTheSet;
}

// only kernels with odd numbers of elements are allowed
std::vector<float> FractalSound::ApplyKernelToDistances(std::vector<float> distances, std::vector<int> kernel)
{
	assert(kernel.size() % 2 == 1);

	if (kernel.size() > distances.size()) {
		throw std::runtime_error("The distances vector is too small for the kernel!");
	}

	std::vector<float> distancesWithKernel(distances.size(), 0);

	for (int i = 0; i < distances.size(); i++)
	{
		int kernelSizeHalved = kernel.size() / 2;
		int j_start = i < kernelSizeHalved ? kernelSizeHalved - i : 0;
		int j_end = i >= distances.size() - kernelSizeHalved ? distances.size() - i + kernelSizeHalved : kernel.size();
		int sumOfKernelValues = 0;
		for (int j = j_start; j < j_end; j++) {
			distancesWithKernel[i] += kernel[j] * distances[i + j - kernelSizeHalved];
			sumOfKernelValues += kernel[j];
		}
		distancesWithKernel[i] /= sumOfKernelValues;

	}

	return distancesWithKernel;
}

Mix_Chunk* FractalSound::CreateMixChunk(int volume) {
	Mix_Chunk* chunk = new Mix_Chunk;
	chunk->alen = 4 * 2 * fs_;
	chunk->abuf = new Uint8[chunk->alen];
	chunk->allocated = 0;
	if (1 <= volume && volume <= MIX_MAX_VOLUME) {
		chunk->volume = volume;
	}
	else {
		chunk->volume = MIX_MAX_VOLUME;
	}

	return chunk;
}

void FractalSound::Mix_FreeChunk(Mix_Chunk* chunk)
{
	delete(chunk->abuf);
	delete(chunk);
}
