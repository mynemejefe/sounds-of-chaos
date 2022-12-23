#pragma once

#include <SDL/SDL_mixer.h>
#include <glm/glm.hpp>

class FractalSound
{
public:
	FractalSound(int fs);
	void PlaySoundAtPos(int fractalType, glm::vec2 pos, int freq);

	inline const int GetFs() { return fs_; };
private:
	int fs_;

	void FillFractal(int fractalType, glm::vec2 pos, int freq, float buff[]);
	float Length2(glm::vec2 vec);
	glm::vec2 Mul(glm::vec2 u, glm::vec2 v);
	void Mix_FreeChunk(Mix_Chunk* chunk);
};

