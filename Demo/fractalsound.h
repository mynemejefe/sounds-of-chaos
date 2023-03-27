#pragma once

#include <SDL/SDL_mixer.h>
#include <glm/glm.hpp>
#include "variables.h"

class FractalSound
{
public:
	FractalSound(int fs);
	~FractalSound() {};
	void PlaySoundAtPos(Variables variables);
	float* CreateSoundBufferFromLastPos(Variables variables);
	void PlaySoundFromMixChunk(Mix_Chunk* chunkToPlay, bool freeUpAfterUse);
	Mix_Chunk* CreateMixChunk();

	inline const int GetFs() { return fs_; };
private:
	int fs_;

	bool FillBuffer(Variables variables, float buff[]);
	glm::vec2 Mul(glm::vec2 u, glm::vec2 v);
	glm::vec2 VecPow(glm::vec2 u, int pow);

	void Mix_FreeChunk(Mix_Chunk* chunk);

#ifdef TESTING
	friend class FractalSoundTester;
#endif
};

