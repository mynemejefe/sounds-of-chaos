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

void FractalSound::PlaySoundAtPos(int fractalType, glm::vec2 pos, int freq, bool allowCloseNeighbours)
{
	Mix_Chunk* chunk = new Mix_Chunk;
	chunk->alen = 4 * 2 * fs_;
	chunk->abuf = new Uint8[chunk->alen];
	chunk->allocated = 0;
	chunk->volume = MIX_MAX_VOLUME;
	
	bool partOfFractal = FillBuffer(fractalType, pos, freq, (float*)chunk->abuf);

	if (!allowCloseNeighbours && !partOfFractal) {
		Mix_FreeChunk(chunk);
		return;
	}

	if (allowCloseNeighbours || partOfFractal) {
		std::thread play([this, chunk]() {
			Mix_PlayChannel(-1, chunk, 0);

			Sleep(2000);

			Mix_FreeChunk(chunk);
		});

		play.detach();
	}
}

bool FractalSound::FillBuffer(int fractalType, glm::vec2 pos, int freq, float buff[]) {
	glm::vec2 z = pos;
	glm::vec2 c = pos;
	int i = 0, max_iterations = 2000;
	int len = fs_;
	bool partOfTheSet = false;

	while (glm::length(z) <= 2 && i < max_iterations && i < len) {
		float length = glm::length(z); //abs value of point at the current iteration

		float fval = sinf(2 * (float)M_PI * freq / fs_ * i) / 4;
		buff[2 * i] = fval * length; //left channel
		buff[2 * i + 1] = fval * length; //right channel

		switch (fractalType) {
		case 0:
			z = Mul(z, z) + c;
			break;
		case 1:
			z = Mul(Mul(z, z), z) + c;
			break;
		case 2:
			glm::vec2 z_abs = glm::abs(z);
			z = Mul(z_abs, z_abs) + c;
			break;
		}
		
		i++;
	}

	if (i == max_iterations) {
		partOfTheSet = true;
	}

	if (i != len && i != 0) {
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

void FractalSound::Mix_FreeChunk(Mix_Chunk* chunk)
{
	delete(chunk->abuf);
	delete(chunk);
}
