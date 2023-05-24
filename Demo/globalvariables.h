#pragma once
#include <glm/ext/vector_float2.hpp>

struct InputVars {
	glm::vec2 lastClickPos{ 0,0 };
	float zoomValue = 0.5f;
	bool isShiftHeldDown = false;
};

struct FractalVars {
	int maxIterations = 2500;
	int fractalType = 0;
	int power = 2;
};

struct SoundVars {	
	int soundGenerationMode = 1; // possible values: 0-2
	int maxSoundIterations = 150;
	int freq = 440;
	bool allowCloseNeighbours = true;
	float normalizationLevel = 0.5; // Range: [0,1]
	int soundVolume = 128; // Range: [1,128]
};

struct GraphicVars {
	float insideColor[3] = { 0.9f,0.5f,0.3f };
	float outsideColor[3] = { 0.9f,0.5f,0.3f };
	float backgroundBrightness = 1;
	float lastClickColor[3] = { 0.3f, 0.8f, 0.5f };
};