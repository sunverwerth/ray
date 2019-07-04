#include "Tracer.h"
#include "Vec3.h"
#include "Ray.h"
#include "Sphere.h"
#include "Hit.h"
#include "mathutils.h"
#include "Prng.h"

#include <cmath>
#include <cstring>
#include <cstdlib>

const int numThreads = 8;
std::vector<Prng> prngs;
Tracer::Tracer() {
	camera.position = Vec3(0, 0, -3);
    camera.direction = Vec3(0,0,1);
    camera.horizontalFov = 3.141f / 2;
    camera.apertureSize = 0;
    camera.focalLength = 5;
	camera.pitch = 0;
    resize(400, 400);
}

Tracer::~Tracer() {
    delete[] buffer;
}

void Tracer::resize(int newWidth, int newHeight) {
    delete[] buffer;
    numSamples = 0;
    width = newWidth;
    height = newHeight;
    buffer = new Vec3[width * height];
	clear();
}

void Tracer::clear() {
	numSamples = 0;
	memset(buffer, 0, sizeof(Vec3) * width * height);
}

Vec3 Tracer::trace(const Ray& _ray, Prng& prng) {
    Vec3 emission(0, 0, 0);
    Vec3 transmission(1, 1, 1);
    Ray ray(_ray);
	bool includeLights = true;
	void* obj = nullptr;
	float ior = 1;

	int level = 0;
    while (level++ < 5) {
        Hit hit;
		if (!scene.intersect(ray, &hit)) {
			emission += scene.sky(ray.direction) * transmission;
			break;
		}

		auto position = ray.origin + ray.direction * hit.distance;
		auto normal = hit.normal;
		if (dot(normal, ray.direction) > 0) normal *= -1;
		auto material = hit.material->sample(position, hit.uvw);
		if (includeLights || !hit.obj->isLight) emission += material.emission * transmission;

		float iorout = (obj == hit.obj) ? 1 : material.ior;

		float totalReflectivity = 0;// fresnel(ray.direction, normal, ior, iorout);
		if (totalReflectivity > prng.frand(0, 1)) {
			// total reflect
			auto refl = reflect(ray.direction, normal);
			if (false && material.roughness > 0.001f && scene.hasLights()) {
				emission += transmission * scene.lightSpecular(hit.obj, position, refl, material.roughness, prng);
				includeLights = false;
			}
			else includeLights = true;
			ray.direction = prng.randomPointOnUnitHemisphere(refl, material.roughness);
			ray.origin = position;
		}
		else {
			if (material.metallic > prng.frand(0, 1)) {
				// metal reflect
				auto refl = reflect(ray.direction, normal);
				transmission *= material.color;
				if (false && material.roughness > 0.001f && scene.hasLights()) {
					emission += transmission * scene.lightSpecular(hit.obj, position, refl, material.roughness, prng);
					includeLights = false;
				}
				else includeLights = true;
				ray.direction = prng.randomPointOnUnitHemisphere(refl, material.roughness);
				ray.origin = position;
			}
			else {
				// dielectric
				if (material.opacity > prng.frand(0, 1)) {
					// diffuse scatter
					transmission *= material.color;
					if (scene.hasLights()) {
						emission += transmission * scene.lightDiffuse(hit.obj, position, normal, prng);
						includeLights = false;
					}
					else includeLights = true;
					ray.origin = position;
					ray.direction = prng.randomPointOnUnitHemisphereCosine(normal);
				}
				else {
					// refract
					ray.direction = refract(ray.direction, normal, ior, iorout);
					ray.direction = prng.randomPointOnUnitHemisphere(ray.direction, material.roughness);
					ray.origin = position;
					ior = iorout;
					transmission *= material.color;
					includeLights = true;
					obj = hit.obj;
				}
			}
        }

		//if (level < 2) continue;
		
		// Russian Roulette
		// Randomly terminate a path with a probability inversely equal to the throughput
		float p = std::max(transmission.x, std::max(transmission.y, transmission.z));
		//p = p * p * p;
		if (prng.frand(0, 1) > p) {
			break;
		}

		// Add the energy we 'lose' by randomly terminating paths
		transmission *= 1 / p;
	}

    return emission;
}

Ray Tracer::pixelToRay(int x, int y, float tanFov, Prng& prng) {
	auto fx = float(-width / 2 + x + prng.frand(-0.5, 0.5)) / width;
	auto fy = float(height / 2 - y + prng.frand(-0.5, 0.5)) / height;

	auto from = camera.position;
	auto to = from + (camera.right * tanFov * fx + camera.up * tanFov * fy * height / width + camera.direction) * camera.focalLength;
	from += prng.randomPointOnUnitDisc() * camera.apertureSize;
	auto dir = normalized(to - from);
	return Ray(from, dir);
}

extern Tracer g_tracer;
extern thread_local int numrays;
int thread_num_rays[numThreads]{0};

void threadfunc(int i, int n, Prng& prng) {
	float tanFov = tanf(g_tracer.camera.horizontalFov / 2);
	numrays = 0;
	for (int y = i; y < g_tracer.height; y += n) {
		for (int x = 0; x < g_tracer.width; x++) {
			g_tracer.buffer[y * g_tracer.width + x] += g_tracer.trace(g_tracer.pixelToRay(x, y, tanFov, prng), prng);
		}
	}
	thread_num_rays[i] += numrays;
	numrays = 0;
}

void Tracer::sample() {
	camera.direction = Vec3(sinf(camera.yaw)*cosf(camera.pitch), sinf(camera.pitch), cosf(camera.yaw)*cosf(camera.pitch));
	camera.right = Vec3(cosf(camera.yaw), 0, -sinf(camera.yaw));
	camera.up = cross(camera.direction, camera.right);

	if (prngs.empty()) {
		for (int i = 0; i < numThreads; i++) {
			prngs.push_back(Prng(rand()));
		}
	}

	threads.clear();
	for (int i = 0; i < numThreads; i++) {
		threads.push_back(std::thread(threadfunc, i, numThreads, std::ref(prngs[i])));
	}
	for (int i = 0; i < numThreads; i++) {
		threads[i].join();
	}

	numSamples++;
}