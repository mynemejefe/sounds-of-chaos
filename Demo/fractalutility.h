#pragma once

#include <glm/glm.hpp>
#include "variables.h"

class FractalUtility
{
public:
	static glm::vec2 Mul(glm::vec2 u, glm::vec2 v);
	static glm::vec2 VecPow(glm::vec2 u, int pow);
	static glm::vec2* FillArrayWithFractalIterations(Variables variables, int length);
};
