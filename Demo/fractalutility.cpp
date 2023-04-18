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

std::vector<float> FractalUtility::MakeVectorWithIterationDistances(InputVars inputVars, FractalVars fractalVars, int length)
{
	if (length == -1) {
		length = fractalVars.maxIterations;
	}

	int i = 0;
	std::vector<float> positions;
	glm::vec2 z = inputVars.lastClickPos, c = z;

	while (glm::length(z) <= 2 && i < length) {

		switch (fractalVars.fractalType) {
		case 0:
			//mandelbrot
			z = VecPow(z, fractalVars.power) + c;
			break;
		case 1:
			//burning ship
			glm::vec2 z_abs = glm::abs(z);
			z = VecPow(z_abs, fractalVars.power) + c;
			break;
		}

		positions.push_back(glm::length(z)); //abs value of point at the current iteration
		i++;
	}

	return positions;
}