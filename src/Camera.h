#ifndef Camera_h
#define Camera_h

#include "Vec3.h"

class Camera {
public:
    Vec3 position;
    Vec3 direction;
	Vec3 right;
	Vec3 up;
	float pitch = 0;
	float yaw = 0;
    float apertureSize;
    float focalLength;
    float horizontalFov;
};

#endif