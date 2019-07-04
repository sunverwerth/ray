#ifndef Sphere_h
#define Sphere_h

#include "Vec3.h"
#include "Material.h"
#include "Object.h"

struct Sphere: Object {
    Vec3 center;
    float radius;

    Sphere(const Vec3& center, float radius, Material* material): center(center), radius(radius) { this->material = material; }

	bool intersect(const Ray& ray, Hit* hit) final override;
	Vec3 getRandomPoint(Prng& prng) final override;
	float getSurfaceArea() final override;
};

#endif