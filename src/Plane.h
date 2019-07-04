#ifndef Plane_h
#define Plane_h

#include "Vec3.h"
#include "Material.h"
#include "Object.h"

struct Plane: Object {
    Vec3 origin;
    Vec3 normal;

    Plane(const Vec3& origin, const Vec3& normal, Material* material): origin(origin), normal(normal) { this->material = material; }
    bool intersect(const Ray& ray, Hit* hit) final override;
	Vec3 getRandomPoint(Prng& prng) final override;
	float getSurfaceArea() final override;
};

#endif