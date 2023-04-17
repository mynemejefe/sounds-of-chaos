#include "fractalsound.h"
#include <SDL/SDL_mixer.h>
#include <thread>
#include <synchapi.h>
#include "fractalutility.h"

FractalSound::FractalSound(int fs) : fs_(fs) {
	if (-1 == Mix_OpenAudio(fs, AUDIO_F32, 2, 512))
	{
		return;
	}
}

FractalSound::~FractalSound() {
	Mix_CloseAudio();
}

void FractalSound::PlaySoundAtPos(InputVars inputVars, FractalVars fractalVars, SoundVars soundVars)
{
	Mix_Chunk* chunk = CreateMixChunk();
	
	bool partOfFractal;
	if (soundVars.soundGenerationMode == 0) {
		partOfFractal = FillBufferSimple(inputVars, fractalVars, soundVars, (float*)chunk->abuf);
	}
	else {
		partOfFractal = FillBufferAdditive(inputVars, fractalVars, soundVars, (float*)chunk->abuf);
	}

	if (!soundVars.allowCloseNeighbours && !partOfFractal) {
		FractalSound::Mix_FreeChunk(chunk);
		return;
	}

	FractalSound::PlaySoundFromMixChunk(chunk, true);
}

// Make sure to deallocate the return value when you don't need it anymore
float* FractalSound::CreateSoundBufferFromLastPos(InputVars inputVars, FractalVars fractalVars, SoundVars soundVars)
{
	//A Mix_Chunk's buffer length is 4*2*FS but its type is Uint8 (1 byte), float is 4 bytes
	float* soundBuffer = new float[2 * FS];

	if (soundVars.soundGenerationMode == 0) {
		FillBufferSimple(inputVars, fractalVars, soundVars, soundBuffer);
	}
	else if (soundVars.soundGenerationMode == 1) {
		FillBufferAdditive(inputVars, fractalVars, soundVars, soundBuffer);
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
			key->soundToPlay = CreateMixChunk();
			key->isFilled = true;
		}
		key->soundToPlay->abuf = (Uint8*)CreateSoundBufferFromLastPos(inputVars, fractalVars, soundVars);
		return true;
	}
	if (key->isFilled)
	{
		PlaySoundFromMixChunk(key->soundToPlay, false);
		return true;
	}

	return false;
}

void FractalSound::PlaySoundFromMixChunk(Mix_Chunk* chunkToPlay, bool freeUpAfterUse)
{
	Mix_Chunk* chunk = chunkToPlay;

	std::thread play([this, chunk, freeUpAfterUse]() {
		Mix_PlayChannel(-1, chunk, 0);

		if (freeUpAfterUse) {
			Sleep(2000);
			FractalSound::Mix_FreeChunk(chunk);
		}
	});

	play.detach();
}

bool FractalSound::FillBufferSimple(InputVars inputVars, FractalVars fractalVars, SoundVars soundVars, float buff[]) {
	glm::vec2 z = inputVars.lastClickPos;
	glm::vec2 c = inputVars.lastClickPos;
	int i = 0, max_iterations = 2500;
	int len = fs_;
	bool partOfTheSet = false;

	while (glm::length(z) <= 2 && i < max_iterations && i < len) {
		float length = glm::length(z); //abs value of point at the current iteration
		float sin = sinf(2 * (float)M_PI * soundVars.freq / fs_ * i) / 4; //sine wave

		buff[2 * i] = sin * length; //left channel
		buff[2 * i + 1] = sin * length; //right channel

		switch (fractalVars.fractalType) {
		case 0:
			//mandelbrot
			z = FractalUtility::VecPow(z, fractalVars.power) + c;
			break;
		case 1:
			//burning ship
			glm::vec2 z_abs = glm::abs(z);
			z = FractalUtility::VecPow(z_abs, fractalVars.power) + c;
			break;
		}
		
		i++;
	}

	if (i == max_iterations) {
		partOfTheSet = true;
	}

	if (i != len && i != 0) {
		//fill up rest of buffer
		int repeats = len / i - 1;
		int original_iterations = i;
		int j = 0;
		for (; i < len; i++) {
			buff[2 * i] = buff[2 * j];
			buff[2 * i + 1] = buff[2 * j + 1];
			j = j % original_iterations + 1;
		}
	}

	return partOfTheSet;
}

bool FractalSound::FillBufferAdditive(InputVars inputVars, FractalVars fractalVars, SoundVars soundVars, float buff[])
{
	glm::vec2 z = inputVars.lastClickPos;
	glm::vec2 c = inputVars.lastClickPos;
	int i = 0, max_iterations = 100;
	int len = fs_;
	bool partOfTheSet = false;

	float length = glm::length(z); //abs value of point at the current iteration
	float sin;

	for (int j = 0; j < len; j++) {
		sin = sinf(2 * (float)M_PI * soundVars.freq * length / fs_ * j) / 4;
		buff[2 * j] = sin; //left channel
		buff[2 * j + 1] = sin; //right channel
	}

	while (glm::length(z) <= 2 && i < max_iterations && i < len) {
		switch (fractalVars.fractalType) {
		case 0:
			//mandelbrot
			z = FractalUtility::VecPow(z, fractalVars.power) + c;
			break;
		case 1:
			//burning ship
			glm::vec2 z_abs = glm::abs(z);
			z = FractalUtility::VecPow(z_abs, fractalVars.power) + c;
			break;
		}

		length = glm::length(z);

		for (int j = i; j < len-i; j++) {
			sin = sinf(2 * (float)M_PI * soundVars.freq * length / fs_ * j) / 4;
			buff[2 * j] = buff[2 * j] + sin;
			buff[2 * j + 1] = buff[2 * j + 1] + sin;
		}

		i++;
	}

	if (soundVars.normalizeSound) {
		for (int j = 0; j < len; j++) {
			buff[2 * j] = buff[2 * j] / i;
			buff[2 * j + 1] = buff[2 * j + 1] / i;
		}
	}

	return true;
}

Mix_Chunk* FractalSound::CreateMixChunk() {
	Mix_Chunk* chunk = new Mix_Chunk;
	chunk->alen = 4 * 2 * fs_;
	chunk->abuf = new Uint8[chunk->alen];
	chunk->allocated = 0;
	chunk->volume = MIX_MAX_VOLUME;

	return chunk;
}

void FractalSound::Mix_FreeChunk(Mix_Chunk* chunk)
{
	delete(chunk->abuf);
	delete(chunk);
}
