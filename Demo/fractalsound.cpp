#include "fractalsound.h"
#include <SDL/SDL_mixer.h>
#include <thread>
#include <synchapi.h>

FractalSound::FractalSound(int fs) : fs_(fs) {
	if (-1 == Mix_OpenAudio(fs, AUDIO_F32, 2, 512))
	{
		return;
	}
}

void FractalSound::PlaySoundAtPos(Variables variables)
{
	Mix_Chunk* chunk = CreateMixChunk();
	
	bool partOfFractal;
	if (variables.soundGenerationMode == 0) {
		partOfFractal = FillBufferAdditive(variables, (float*)chunk->abuf);
	}
	else {
		partOfFractal = FillBufferSimple(variables, (float*)chunk->abuf);
	}

	if (!variables.allowCloseNeighbours && !partOfFractal) {
		FractalSound::Mix_FreeChunk(chunk);
		return;
	}

	FractalSound::PlaySoundFromMixChunk(chunk, true);
}

// Make sure to deallocate the return value when you don't need it anymore
float* FractalSound::CreateSoundBufferFromLastPos(Variables variables)
{
	//A Mix_Chunk's buffer length is 4*2*FS but its type is Uint8 (1 byte), float is 4 bytes
	float* soundBuffer = new float[2 * variables.FS];

	if (variables.soundGenerationMode == 0) {
		FillBufferAdditive(variables, soundBuffer);
	}
	else {
		FillBufferSimple(variables, soundBuffer);
	}

	return soundBuffer;
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

bool FractalSound::FillBufferSimple(Variables variables, float buff[]) {
	glm::vec2 z = variables.lastClickPos;
	glm::vec2 c = variables.lastClickPos;
	int i = 0, max_iterations = 2500;
	int len = fs_;
	bool partOfTheSet = false;

	while (glm::length(z) <= 2 && i < max_iterations && i < len) {
		float length = glm::length(z); //abs value of point at the current iteration
		float sin = sinf(2 * (float)M_PI * variables.freq / fs_ * i) / 4; //sine wave

		buff[2 * i] = sin * length; //left channel
		buff[2 * i + 1] = sin * length; //right channel

		switch (variables.fractalType) {
		case 0:
			//mandelbrot
			z = VecPow(z, variables.power) + c;
			break;
		case 1:
			//burning ship
			glm::vec2 z_abs = glm::abs(z);
			z = VecPow(z_abs, variables.power) + c;
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

bool FractalSound::FillBufferAdditive(Variables variables, float buff[])
{
	glm::vec2 z = variables.lastClickPos;
	glm::vec2 c = variables.lastClickPos;
	int i = 0, max_iterations = 100;
	int len = fs_;
	bool partOfTheSet = false;

	float length = glm::length(z); //abs value of point at the current iteration
	float sin;

	for (int j = 0; j < len; j++) {
		sin = sinf(2 * (float)M_PI * variables.freq * length / fs_ * j) / 4;
		buff[2 * j] = sin; //left channel
		buff[2 * j + 1] = sin; //right channel
	}

	while (glm::length(z) <= 2 && i < max_iterations && i < len) {
		switch (variables.fractalType) {
		case 0:
			//mandelbrot
			z = VecPow(z, variables.power) + c;
			break;
		case 1:
			//burning ship
			glm::vec2 z_abs = glm::abs(z);
			z = VecPow(z_abs, variables.power) + c;
			break;
		}

		length = glm::length(z);

		for (int j = i; j < len-i; j++) {
			sin = sinf(2 * (float)M_PI * variables.freq * length / fs_ * j) / 4;
			buff[2 * j] = buff[2 * j] + sin;
			buff[2 * j + 1] = buff[2 * j + 1] + sin;
		}

		i++;
	}

	if (variables.normalizeSound) {
		for (int j = 0; j < len; j++) {
			buff[2 * j] = buff[2 * j] / i;
			buff[2 * j + 1] = buff[2 * j + 1] / i;
		}
	}

	return true;
}

glm::vec2 FractalSound::Mul(glm::vec2 u, glm::vec2 v) {
	return glm::vec2(u.x * v.x - u.y * v.y, u.x * v.y + u.y * v.x);
}

glm::vec2 FractalSound::VecPow(glm::vec2 u, int pow) {
	int i = 1;
	glm::vec2 v = u;
	while (i < pow) {
		v = Mul(v, u);
		i++;
	}
	return v;
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
