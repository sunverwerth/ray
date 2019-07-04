#ifndef Cube_h
#define Cube_h

#include "Vec3.h"
#include "Material.h"
#include "Object.h"

struct Cube: Object {
	Vec3 center;
	Vec3 size;

	Cube(const Vec3& center, const Vec3& size, Material* material) : center(center), size(size) { this->material = material; }

	float getSurfaceArea() final override;
	bool intersect(const Ray& ray, Hit* hit) final override;
	Vec3 getRandomPoint(Prng& prng) final override;
};

#endif