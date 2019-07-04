#ifndef Hit_h
#define Hit_h

struct Material;
struct Object;

const float kEpsilon = 0.000001f;

struct Hit {
	float minDistance = 0.0001f;
    float distance = 99999;
    Material* material = nullptr;
	Object* obj = nullptr;
	Vec3 normal;
	Vec3 uvw;
};

#endif