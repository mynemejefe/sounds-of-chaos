#pragma once

#include <glm/glm.hpp>
#include "globalvariables.h"
#include <vector>

class FractalUtility
{
public:
	static glm::vec2 Mul(glm::vec2 u, glm::vec2 v);
	static glm::vec2 VecPow(glm::vec2 u, int pow);
	static std::vector<float> MakeVectorWithIterationDistances(InputVars inputVars, FractalVars fractalVars, int length = -1);
};

