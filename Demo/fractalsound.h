#pragma once

#include <SDL/SDL_mixer.h>
#include <glm/glm.hpp>
#include "globalvariables.h"
#include <vector>

#define TESTING

struct PianoKey {
	Mix_Chunk* soundToPlay;
	bool isFilled = false;
};

class FractalSound
{
public:
	FractalSound(int fs);
	~FractalSound();
	void PlaySoundAtPos(InputVars inputVars, FractalVars fractalVars, SoundVars soundVars);
	bool UsePianoKey(InputVars inputVars, FractalVars fractalVars, SoundVars soundVars, int n);

	inline const int GetFs() { return fs_; };
private:
	int fs_;
	PianoKey pianoKeys[10]{};
	friend class FractalSoundTester;

	Mix_Chunk* CreateMixChunk(int volume);
	void Mix_FreeChunk(Mix_Chunk* chunk);
	bool FillBufferAM(InputVars inputVars, FractalVars fractalVars, SoundVars soundVars, float buff[]);
	bool FillBufferAdditive(InputVars inputVars, FractalVars fractalVars, SoundVars soundVars, float buff[]);
	float* CreateSoundBufferFromLastPos(InputVars inputVars, FractalVars fractalVars, SoundVars soundVars);
	void PlaySoundFromMixChunk(Mix_Chunk* chunkToPlay);
	std::vector<float> ApplyKernelToDistances(std::vector<float> distances, std::vector<int> kernel);
};

