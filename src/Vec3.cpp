#include "Vec3.h"
#include "mathutils.h"

#include <cmath>
#include <algorithm>

Vec3& Vec3::operator=(const Vec3& v) {
    x = v.x;
    y = v.y;
    z = v.z;
    return *this;
}

Vec3& Vec3::operator+=(const Vec3& v) {
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}

Vec3& Vec3::operator-=(const Vec3& v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
}

Vec3& Vec3::operator*=(const Vec3& v) {
    x *= v.x;
    y *= v.y;
    z *= v.z;
    return *this;
}

Vec3& Vec3::operator/=(const Vec3& v) {
    x /= v.x;
    y /= v.y;
    z /= v.z;
    return *this;
}

Vec3& Vec3::operator*=(float f) {
    x *= f;
    y *= f;
    z *= f;
    return *this;
}

Vec3& Vec3::operator/=(float f) {
    f = 1.0f / f;
    x *= f;
    y *= f;
    z *= f;
    return *this;
}

void Vec3::normalize() {
    float f = 1.0f / length(*this);
    x *= f;
    y *= f;
    z *= f;
}


float length(const Vec3& v) {
    return ::sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vec3 normalized(const Vec3& v) {
	float il = 1.0f / length(v);
	return Vec3(v.x * il, v.y * il, v.z * il);
}

Vec3 boxnormal(const Vec3& p) {
	float x = abs(p.x);
	float y = abs(p.y);
	float z = abs(p.z);

	if (x > y) {
		if (x > z) return p.x > 0 ? Vec3(1, 0, 0) : Vec3(-1, 0, 0);
		else return p.z > 0 ? Vec3(0, 0, 1) : Vec3(0, 0, -1);
	}
	else {
		if (y > z) return p.y > 0 ? Vec3(0, 1, 0) : Vec3(0, -1, 0);
		else return p.z > 0 ? Vec3(0, 0, 1) : Vec3(0, 0, -1);
	}
}

Vec3 cross(const Vec3& a, const Vec3& b) {
    return Vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

float dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 operator-(const Vec3& v) {
    return Vec3(-v.x, -v.y, -v.z);
}

bool operator==(const Vec3& a, const Vec3& b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

Vec3 operator+(const Vec3& a, const Vec3& b) {
    return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vec3 operator-(const Vec3& a, const Vec3& b) {
    return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vec3 operator*(const Vec3& a, const Vec3& b) {
    return Vec3(a.x * b.x, a.y * b.y, a.z * b.z);
}

Vec3 operator/(const Vec3& a, const Vec3& b) {
    return Vec3(a.x / b.x, a.y / b.y, a.z / b.z);
}

Vec3 operator*(const Vec3& v, float f) {
    return Vec3(v.x * f, v.y * f, v.z * f);
}

Vec3 operator*(float f, const Vec3& v) {
    return Vec3(v.x * f, v.y * f, v.z * f);
}

Vec3 operator/(const Vec3& v, float f) {
    return Vec3(v.x / f, v.y / f, v.z / f);
}

Vec3 lerp(const Vec3& a, const Vec3& b, float f) {
    return a * (1.0f - f) + b * f;
}

Vec3 reflect(const Vec3& in, const Vec3& n) {
    return in - n * dot(in, n) * 2;
}

Vec3 refract(const Vec3& I, const Vec3& N, float ior1, float ior2) {
    auto n = N;
    float cosi = clamp(-1, 1, dot(I, n));
    float etai = ior1, etat = ior2;
    if (cosi < 0) {
        cosi = -cosi;
    }
    else {
        std::swap(etai, etat); n = -N;
    }
    float eta = etai / etat;
    float k = 1 - eta * eta * (1 - cosi * cosi);
    return k < 0 ? reflect(I, n) : eta * I + (eta * cosi - sqrtf(k)) * n; 
}

float fresnel(const Vec3 &I, const Vec3 &N, float ior1, float ior2) {
    float cosi = clamp(-1, 1, dot(I, N));
    float etai = ior1, etat = ior2;
    if (cosi > 0) {
        std::swap(etai, etat);
    }
    
    // Compute sini using Snell's law
    float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
    
    // Total internal reflection
    if (sint >= 1) {
        return 1;
    }
    else {
        float cost = sqrtf(std::max(0.f, 1 - sint * sint));
        cosi = fabsf(cosi);
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
        return (Rs * Rs + Rp * Rp) / 2;
    }
    // As a consequence of the conservation of energy, transmittance is given by:
    // kt = 1 - kr;
}