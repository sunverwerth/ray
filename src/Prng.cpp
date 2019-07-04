#include "Prng.h"

#include <cstdlib>

Prng::Prng(unsigned int seed): m_seed(seed), rng(seed) {
}

void Prng::seed(unsigned int sd) {
	m_seed = sd;
	rng.seed(sd);
}

float Prng::frand(float min, float max) {
	float r = (float)rng() / rng.max();
	return min * (1.0f - r) + max * r;
}

Vec3 Prng::randomPointOnUnitSpherePatch(float tmin, float tmax, float pmin, float pmax) {
	float theta = frand(tmin, tmax);
	float phi = acosf(frand(pmin, pmax));
	auto r = Vec3(
		sinf(phi) * cosf(theta),
		cosf(phi),
		sinf(phi) * sinf(theta)
	);

	return r;
}

Vec3 Prng::randomPointOnUnitSphere() {
	float theta = frand(0, 2 * M_PI);
	float phi = acosf(frand(-1, 1));
	float sinphi = sinf(phi);
	auto r = Vec3(
		sinphi * cosf(theta),
		cosf(phi),
		sinphi * sinf(theta)
	);

	return r;
}

Vec3 Prng::randomPointOnUnitHemisphereCosine(const Vec3& up) {
	auto right = normalized(cross(up, up.y > 0.99 ? Vec3(0, 1, 1) : Vec3(1, 1, 0)));
	auto forward = cross(right, up);

	float u1 = frand(0, 1);
	float u2 = frand(0, 1);
	
	float r = sqrtf(u1);
	float theta = 2 * M_PI * u2;

	auto v = right * (cosf(theta) * r) + forward * (sinf(theta) * r) + up * sqrtf(fmaxf(0, 1 - u1));
	return v;
}

Vec3 Prng::randomPointOnUnitHemisphere(const Vec3& up) {
	auto right = normalized(cross(up, up.y > 0.99 ? Vec3(0, 0, 1) : Vec3(1, 0, 0)));
	auto forward = cross(right, up);

	float u1 = frand(0, 1);
	float u2 = frand(0, 1);

	float r = ::sqrt(1.0f - u1 * u1);
	float phi = 2 * M_PI * u2;

	auto v = right * (::cos(phi) * r) + forward * (::sin(phi) * r) + up * u1;
	return v;
}

Vec3 Prng::randomPointOnUnitHemisphere(const Vec3& up, float amount) {
	auto right = normalized(cross(up, up.y > 0.99 ? Vec3(0, 0, 1) : Vec3(1, 0, 0)));
	auto forward = cross(right, up);

	float u1 = frand(0, amount);
	float u2 = frand(0, 1);

	float r = sqrtf(u1);
	float theta = 2 * M_PI * u2;

	auto v = right * (cosf(theta) * r) + forward * (sinf(theta) * r) + up * sqrtf(fmaxf(0, 1 - u1));
	return v;
}

Vec3 Prng::randomPointOnUnitDisc() {
	float angle = frand(0, 3.1415926535f * 2);
	float radius = ::sqrtf(frand(0, 1));
	return Vec3(::cosf(angle) * radius, ::sinf(angle) * radius, 0);
}

Vec3 Prng::randomPointInUnitCube() {
	return Vec3(
		frand(-1.0f, 1.0f),
		frand(-1.0f, 1.0f),
		frand(-1.0f, 1.0f)
	);
}