#include "Sphere.h"
#include "Ray.h"
#include "Hit.h"

#include <cmath>

bool Sphere::intersect(const Ray& ray, Hit* hit) {
	auto oc = ray.origin - center;

	auto det = dot(ray.direction, oc);
	float a = -det;
	det *= det;
	det = det - dot(oc, oc) + radius * radius;
	if (det < 0) return false;

	float b = ::sqrtf(det);
	float near = a - b;
	float far = a + b;
	float dist = near;
	if (near < 0) dist = far;

	if (dist < 0) return false;
	if (hit) {
		if (dist > hit->distance || dist < hit->minDistance) return false;

		hit->distance = dist;
		hit->material = material;
		hit->obj = this;
		hit->normal = (ray.origin + ray.direction * dist - center) / radius;
	}
	return true;
}

Vec3 Sphere::getRandomPoint(Prng& prng) {
	return center + prng.randomPointOnUnitSphere()*radius;
}

float Sphere::getSurfaceArea() {
	return 4 * M_PI * radius*radius;
}