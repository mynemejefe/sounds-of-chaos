#pragma once

#include <SDL/SDL_mixer.h>
#include <glm/glm.hpp>
#include "variables.h"

class FractalSound
{
public:
	FractalSound(int fs);
	void PlaySoundAtPos(Variables variables);
	void PlaySoundAtPos(int fractalType, int power, glm::vec2 pos, int freq, bool allowCloseNeighbours);

	inline const int GetFs() { return fs_; };
private:
	int fs_;

	bool FillBuffer(int fractalType, int power, glm::vec2 pos, int freq, float buff[]);
	glm::vec2 Mul(glm::vec2 u, glm::vec2 v);
	glm::vec2 VecPow(glm::vec2 u, int pow);
	void Mix_FreeChunk(Mix_Chunk* chunk);
};

