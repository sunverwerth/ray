#include "Scene.h"
#include "Ray.h"
#include "Hit.h"
#include "Sphere.h"
#include "Cube.h"
#include "Plane.h"
#include "Material.h"
#include "mathutils.h"
#include "EnvironmentMap.h"
#include "Prng.h"
#include "Mesh.h"
#include "Quad.h"

#include <cmath>
#include <fstream>
#include <Windows.h>

float frand() {
	return (float)rand() / RAND_MAX;
}

Scene::Scene() {
	srand(GetTickCount());

	const auto white = Vec3(0.9, 0.9, 0.9);
	const auto red = Vec3(0.9, 0.2, 0.2);
	const auto green = Vec3(0.2, 0.9, 0.2);
	const auto blue = Vec3(0.2, 0.2, 0.9);

	objects.push_back(new Quad(Vec3(-1, -1, -1), Vec3(0, 2, 0), Vec3(0, 0, 2), new DefaultMaterial(red)));
	objects.push_back(new Quad(Vec3(1, -1, -1), Vec3(0, 2, 0), Vec3(0, 0, 2), new DefaultMaterial(green)));
	objects.push_back(new Quad(Vec3(-1,  1, -1), Vec3(2, 0, 0), Vec3(0, 0, 2), new DefaultMaterial(white)));
	objects.push_back(new Quad(Vec3(-1, -1, -1), Vec3(2, 0, 0), Vec3(0, 0, 2), new DefaultMaterial(white)));
	objects.push_back(new Quad(Vec3(-1, -1,  1), Vec3(2, 0, 0), Vec3(0, 2, 0), new DefaultMaterial(white)));

	auto light = new Quad(Vec3(0, 0.99999, -0.5), Vec3(0.5, 0, 0), Vec3(0, 0, 0.5), new DefaultMaterial(white, white * 10));
	objects.push_back(light);
	addLight(light);

	auto barrier = new Quad(Vec3(-0.2, -0.5, -1), Vec3(0, 1.5, 0), Vec3(0, 0, 2), new DefaultMaterial(white));
	objects.push_back(barrier);

	auto mirror = new Quad(Vec3(0, -1, -0.5), Vec3(0.7, 0.7, 0), Vec3(-0.5, 0, 1), new DefaultMaterial(white, 0, 0, 1.0f, 1.0f));
	objects.push_back(mirror);

	auto sun = new Sphere(Vec3(0, 1000, 0), 50, new DefaultMaterial(Vec3(0, 0, 0), Vec3(1, 1, 0.7) * 200));
	objects.push_back(sun);
	addLight(sun);
	
	sunDir = normalized(-sun->center);
	sunColor = Vec3(1, 1, 0.8);
}

Vec3 Scene::sky(const Vec3& dir) {
	float nl = dot(dir, -sunDir);
	nl = max(nl, 0);
	nl *= nl;
	nl *= nl;
	nl *= nl;
	nl *= nl;
	return (Vec3(1, 1, 1) + Vec3(-0.25, -0.25, 0.5) * dir.y) + sunColor * nl;
}

Vec3 Scene::lightDiffuse(Object* obj, const Vec3& pos, const Vec3& normal, Prng& prng) {
	int i = prng.frand(0, lights.size());
	if (i >= lights.size()) i = lights.size() - 1;
	if (lights[i] == obj) return Vec3(0, 0, 0);

	auto randomPoint = lights[i]->getRandomPoint(prng);
	auto lightDir = randomPoint - pos;
	auto ddn = dot(lightDir, normal);
	if (ddn < 0) return Vec3(0, 0, 0);
	auto l = length(lightDir);
	lightDir /= l;
	ddn /= l;
	Ray ray(pos, lightDir);
	Hit hit;
	if (!intersect(ray, &hit) || hit.obj == lights[i]) {
		return (ddn * lights[i]->material->sample(randomPoint, Vec3(0, 0, 0)).emission / (1 + l * l)) * lights[i]->getSurfaceArea() / M_PI * lights.size();
	}
	return Vec3(0, 0, 0);
}

Vec3 Scene::lightSpecular(Object* obj, const Vec3& pos, const Vec3& direction, float roughness, Prng& prng) {
	int i = prng.frand(0, lights.size() - 1);
	if (lights[i] == obj) return Vec3(0, 0, 0);

	auto randomPoint = lights[i]->getRandomPoint(prng);
	auto lightDir = randomPoint - pos;
	auto dld = dot(lightDir, direction);
	if (dld < 0) return Vec3(0, 0, 0);
	auto l = length(lightDir);
	lightDir /= l;
	dld /= l;
	Ray ray(pos, lightDir);

	Hit hit;
	hit.distance = l;
	if (!intersect(ray, &hit) || hit.obj == lights[i]) {
		return pow(dld, 1.0f + (1.0f - roughness) * 1000) / (1.0f + roughness * 10) * lights[i]->material->sample(randomPoint, Vec3(0, 0, 0)).emission + lights.size();
	}
	return Vec3(0, 0, 0);
}

thread_local int numrays = 0;

bool Scene::intersect(const Ray& ray, Hit* hit) {
	numrays++;
	bool found = false;

	for (auto& object: objects) {
		found |= object->intersect(ray, hit);
	}

	return found;
}