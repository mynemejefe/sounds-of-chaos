#pragma once

#include <SDL/SDL_mixer.h>
#include <glm/glm.hpp>
#include "variables.h"

struct PianoKey {
	Mix_Chunk* soundToPlay;
	bool isFilled = false;
};

class FractalSound
{
public:
	FractalSound(int fs);
	~FractalSound() {};
	void PlaySoundAtPos(Variables variables);
	float* CreateSoundBufferFromLastPos(Variables variables);
	void ModifyPianoKey(Variables variables, int n);
	void PlaySoundFromMixChunk(Mix_Chunk* chunkToPlay, bool freeUpAfterUse);

	inline const int GetFs() { return fs_; };
private:
	int fs_;
	PianoKey pianoKeys[10]{};

	Mix_Chunk* CreateMixChunk();
	void Mix_FreeChunk(Mix_Chunk* chunk);
	bool FillBufferSimple(Variables variables, float buff[]);
	bool FillBufferAdditive(Variables variables, float buff[]);

#ifdef TESTING
	friend class FractalSoundTester;
#endif
};

