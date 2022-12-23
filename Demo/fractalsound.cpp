#include "fractalsound.h"
#include <SDL/SDL_mixer.h>

FractalSound::FractalSound(int fs) : fs_(fs) {
	if (-1 == Mix_OpenAudio(fs, AUDIO_F32, 2, 512))
	{
		return;
	}
}

void FractalSound::PlaySoundAtPos(int fractalType, glm::vec2 pos, int freq)
{
	Mix_Chunk* chunk = new Mix_Chunk;
	chunk->alen = 4 * 2 * fs_;
	chunk->abuf = new Uint8[chunk->alen];
	//chunk->abuf = (Uint8*)malloc(chunk->alen * sizeof(Uint8));
	chunk->allocated = 0;
	chunk->volume = 127;

	FillFractal(fractalType, pos, freq, (float*)chunk->abuf);
	
	Mix_PlayChannel(-1, chunk, 0);
	
}

void FractalSound::FillFractal(int fractalType, glm::vec2 pos, int freq, float buff[]) {
	glm::vec2 z = pos;
	glm::vec2 c = pos;
	int i = 0, max_iterations = 2000;
	int len = fs_;

	while (Length2(z) <= 2 * 2 && i < max_iterations && i < len) {
		float length = Length2(z);

		float fval = sinf(2 * (float)M_PI * freq / fs_ * i) / 4;
		buff[2 * i] = fval * length;
		buff[2 * i + 1] = fval * length;

		switch (fractalType) {
		case 0:
			z = Mul(z, z) + c;
			break;
		case 1:
			z = Mul(Mul(z, z), z) + c;
			break;
		case 2:
			z = powf(sqrtf(Length2(z)), 2) + c;
			break;
		}

		i++;
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
}

float FractalSound::Length2(glm::vec2 vec) {
	return  vec.x * vec.x + vec.y * vec.y;
}

glm::vec2 FractalSound::Mul(glm::vec2 u, glm::vec2 v) {
	return glm::vec2(u.x * v.x - u.y * v.y, u.x * v.y + u.y * v.x);
}

void FractalSound::Mix_FreeChunk(Mix_Chunk* chunk)
{
	delete(chunk->abuf);
	delete(chunk);
}
