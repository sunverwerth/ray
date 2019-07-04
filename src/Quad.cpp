#include "Quad.h"

bool Quad::intersect(const Ray& ray, Hit* hit) {
	auto v0 = origin;
	auto v1 = origin + u;
	auto v2 = origin + v;
	Vec3 edge1, edge2, h, s, q;
	float a, f, u, v;

	edge1 = v1 - v0;
	edge2 = v2 - v0;
	h = cross(ray.direction, edge2);
	a = dot(edge1, h);
	if (a > -kEpsilon && a < kEpsilon)
		return false;    // This ray is parallel to this triangle.
	f = 1.0 / a;
	s = ray.origin - v0;
	u = f * dot(s, h);
	if (u < 0.0 || u > 1.0)
		return false;
	q = cross(s, edge1);
	v = f * dot(ray.direction, q);
	if (v < 0.0 || v > 1.0)
		return false;
	// At this stage we can compute t to find out where the intersection point is on the line.
	float t = f * dot(edge2, q);
	if (t < kEpsilon)
		return false;

	if (hit) {
		if (t > hit->distance || t < hit->minDistance) return false;
		hit->distance = t;
		hit->normal = normalized(cross(edge1, edge2));
		hit->material = material;
		hit->obj = this;
	}

	return true;
}

Vec3 Quad::getRandomPoint(Prng& prng) {
	return origin + u * prng.frand(0, 1) + v * prng.frand(0, 1);
}

float Quad::getSurfaceArea() {
	return length(cross(u, v));
}