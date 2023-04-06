#include "fractalutility.h"

glm::vec2 FractalUtility::Mul(glm::vec2 u, glm::vec2 v) {
	return glm::vec2(u.x * v.x - u.y * v.y, u.x * v.y + u.y * v.x);
}

glm::vec2 FractalUtility::VecPow(glm::vec2 u, int pow) {
	int i = 1;
	glm::vec2 v = u;
	while (i < pow) {
		v = Mul(v, u);
		i++;
	}
	return v;
}

glm::vec2* FractalUtility::FillArrayWithFractalIterations(Variables variables, int length)
{
	int i = 0;
	glm::vec2* positions = new glm::vec2[length];
	glm::vec2 z = variables.lastClickPos, c = z;

	while (glm::length(z) <= 2 && i < length) {

		switch (variables.fractalType) {
		case 0:
			//mandelbrot
			z = VecPow(z, variables.power) + c;
			break;
		case 1:
			//burning ship
			glm::vec2 z_abs = glm::abs(z);
			z = VecPow(z_abs, variables.power) + c;
			break;
		}

		positions[i] = glm::vec2(z);
		i++;
	}

	if (i < length) {
		positions[i] = glm::vec2(FLT_MAX);
	}

	return positions;
}