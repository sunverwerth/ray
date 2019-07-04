#ifndef Scene_h
#define Scene_h

#include "Sphere.h"
#include "Plane.h"
#include "Cube.h"
#include "Mesh.h"

#include <vector>

struct Ray;
struct Hit;
struct EnvironmentMap;
class Prng;

class Scene {
public:
    Scene();
    bool intersect(const Ray& ray, Hit* hit = nullptr);
    Vec3 lightDiffuse(Object* obj, const Vec3& pos, const Vec3& normal, Prng& prng);
	Vec3 lightSpecular(Object* obj, const Vec3& pos, const Vec3& direction, float roughness, Prng& prng);
    Vec3 sky(const Vec3& dir);
	
	void addLight(Object* o) {
		o->isLight = true;
		lights.push_back(o);
	}
	
	bool hasLights() {
		return !lights.empty();
	}

public:
    std::vector<Object*> objects;
	std::vector<Object*> lights;
	EnvironmentMap* envMap = nullptr;
	Vec3 sunDir;
	Vec3 sunColor;
};

#endif