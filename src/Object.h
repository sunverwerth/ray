#ifndef Object_h
#define Object_h

#include "Ray.h"
#include "Hit.h"
#include "Prng.h"

struct Object {
	virtual bool intersect(const Ray& ray, Hit* hit) = 0;
	virtual Vec3 getRandomPoint(Prng& prng) = 0;
	virtual float getSurfaceArea() = 0;

	bool isLight = false;
	Material* material = nullptr;
};

#endif
