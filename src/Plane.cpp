#include "Plane.h"
#include "Ray.h"
#include "Hit.h"

bool Plane::intersect(const Ray& ray, Hit* hit) {
	float ddn = dot(ray.direction, normal);
    float dist = dot(origin - ray.origin, normal) / ddn;
    if (dist < 0) return false;

	if (hit) {
		if (dist > hit->distance || dist < hit->minDistance) return false;
		auto pos = ray.origin + ray.direction * dist;

		hit->distance = dist;
		hit->material = material;
		hit->obj = this;
		hit->normal = normal;
	}
    return true;
}

Vec3 Plane::getRandomPoint(Prng& prng) {
	return Vec3(0,0,0);
}

float Plane::getSurfaceArea() {
	return 9999999999999;
}