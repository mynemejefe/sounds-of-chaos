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
	
	bool partOfFractal = FillBuffer(variables, (float*)chunk->abuf);

	if (!variables.allowCloseNeighbours && !partOfFractal) {
		FractalSound::Mix_FreeChunk(chunk);
		return;
	}

	if (variables.allowCloseNeighbours || partOfFractal) {
		std::thread play([this, chunk]() {
			Mix_PlayChannel(-1, chunk, 0);

			Sleep(2000);

			FractalSound::Mix_FreeChunk(chunk);
		});

		play.detach();
	}
}

// Make sure to deallocate the return value when you don't need it anymore
float* FractalSound::CreateSoundBufferFromLastPos(Variables variables)
{
	Mix_Chunk* chunk = CreateMixChunk();

	FillBuffer(variables, (float*)chunk->abuf);

	Uint8* buffer = chunk->abuf;
	chunk->abuf = new Uint8[chunk->alen];
	FractalSound::Mix_FreeChunk(chunk);

	return (float*)buffer;
}

void FractalSound::PlaySoundFromBuffer(float buff[]) 
{
	Mix_Chunk* chunk = CreateMixChunk();

	std::thread play([this, chunk]() {
		Mix_PlayChannel(-1, chunk, 0);

		Sleep(2000);

		chunk->abuf = new Uint8[chunk->alen];
		FractalSound::Mix_FreeChunk(chunk);
	});

	play.detach();
}

bool FractalSound::FillBuffer(Variables variables, float buff[]) {
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
