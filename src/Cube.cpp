#include "Cube.h"
#include "Ray.h"
#include "Hit.h"

#include <cmath>
#include <array>

bool Cube::intersect(const Ray& ray, Hit* hit) {

	auto origin = ray.origin - center;
	auto hsize = size / 2;

	float tmin = (-hsize.x - origin.x) / ray.direction.x;
	float tmax = (hsize.x - origin.x) / ray.direction.x;

	if (tmin > tmax) std::swap(tmin, tmax);

	float tymin = (-hsize.y - origin.y) / ray.direction.y;
	float tymax = (hsize.y - origin.y) / ray.direction.y;

	if (tymin > tymax) std::swap(tymin, tymax);

	if ((tmin > tymax) || (tymin > tmax))
		return false;

	if (tymin > tmin)
		tmin = tymin;

	if (tymax < tmax)
		tmax = tymax;

	float tzmin = (-hsize.z - origin.z) / ray.direction.z;
	float tzmax = (hsize.z - origin.z) / ray.direction.z;

	if (tzmin > tzmax) std::swap(tzmin, tzmax);

	if ((tmin > tzmax) || (tzmin > tmax))
		return false;

	if (tzmin > tmin)
		tmin = tzmin;

	if (tzmax < tmax)
		tmax = tzmax;

	if (
		origin.x > -hsize.x && origin.x < hsize.x
		&& origin.y > -hsize.y && origin.y < hsize.y
		&& origin.z > -hsize.z && origin.z < hsize.z
		) {
		tmin = tmax;
	}

	if (tmin < 0) return false;

	if (hit) {
		if (tmin > hit->distance || tmin < hit->minDistance) return false;
		hit->distance = tmin;
		hit->material = material;
		hit->obj = this;
		hit->normal = boxnormal((ray.origin + ray.direction * tmin - center) / size);
	}

	return true;
}

float Cube::getSurfaceArea() {
	return 2 * size.x*size.y + 2 * size.x*size.z + 2 * size.y*size.z;
}

Vec3 Cube::getRandomPoint(Prng& prng) {
	return center + prng.randomPointInUnitCube() * size;
}