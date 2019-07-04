#ifndef Tracer_h
#define Tracer_h

#include "Scene.h"
#include "Camera.h"
#include "Ray.h"
#include "Vec3.h"

#include <thread>

extern const int numThreads;

class Prng;

class Tracer {
public:
    Tracer();
    ~Tracer();

    void sample();
    void resize(int newWidth, int newHeight);
    Vec3 trace(const Ray& ray, Prng& prng);
	Ray pixelToRay(int x, int y, float tanFov, Prng& prng);
	void clear();

public:
	std::vector<std::thread> threads;
    Camera camera;
    int width;
    int height;
    int numSamples = 0;
	Vec3* buffer = nullptr;
    Scene scene;
};

#endif