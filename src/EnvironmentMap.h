#ifndef EnvironmentMap_h
#define EnvironmentMap_h

#include "Vec3.h"

#include <vector>
#include <istream>
#include <cstdint>

struct RGBE {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t e;
};

struct EnvLight {
	Vec3 dir;
	Vec3 color;
};

struct EnvironmentMap {
    RGBE* data;
    int width;
    int height;
	std::vector<EnvLight> lights;

	EnvironmentMap(std::istream& file);

    Vec3 sample(const Vec3& dir);
	Vec3 texcoordToVector(float x, float y);
};

Vec3 rgbeToColor(RGBE data);

#endif