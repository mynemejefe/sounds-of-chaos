#pragma once
#include <glm/ext/vector_float2.hpp>

struct Variables {

	//General variables
	glm::vec2 lastClickPos{ 0,0 };
	float zoomValue = 0.5f;
	bool isShiftHeldDown = false;

	//Sound variables
	const int FS = 44100;
	int soundGenerationMode = 0;
	int freq = 440;
	bool allowCloseNeighbours = true;
	bool normalizeSound = true;

	//Graphic variables
	int maxIterations = 2500;
	int fractalType = 0;
	int power = 2;
	float insideColor[3] = { 0.9f,0.5f,0.3f };
	float outsideColor[3] = { 0.9f,0.5f,0.3f };
	float backgroundBrightness = 1;
	float lastClickColor[3] = { 0.3f, 0.8f, 0.5f };
};