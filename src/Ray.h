#ifndef Ray_h
#define Ray_h

#include "Vec3.h"

struct Ray {
    Vec3 origin;
    Vec3 direction;

    Ray(const Vec3& origin, const Vec3& direction): origin(origin), direction(direction) {}
};

#endif