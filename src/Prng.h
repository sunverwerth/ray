#ifndef Prng_h
#define Prng_h

#include "Vec3.h"

#include <random>

class Prng {
public:
	Prng(unsigned int seed);
	void seed(unsigned int sd);

	float frand(float min, float max);
	Vec3 randomPointOnUnitSpherePatch(float tmin, float tmax, float pmin, float pmax);
	Vec3 randomPointOnUnitSphere();
	Vec3 randomPointOnUnitHemisphere(const Vec3& n);
	Vec3 randomPointOnUnitHemisphereCosine(const Vec3& n);
	Vec3 randomPointOnUnitHemisphere(const Vec3& n, float amount);
	Vec3 randomPointOnUnitDisc();
	Vec3 randomPointInUnitCube();

private:
	std::vector<float> values;
	std::mt19937 rng;
	unsigned int m_seed;
};

#endif
