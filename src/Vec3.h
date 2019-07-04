#ifndef Vec3_h
#define Vec3_h

struct Vec3 {
    float x, y, z;

    Vec3() = default;
	Vec3(float v): x(v), y(v), z(v) {}
    Vec3(float x, float y, float z): x(x), y(y), z(z) {}

    Vec3& operator=(const Vec3& v);
    Vec3& operator+=(const Vec3& v);
    Vec3& operator-=(const Vec3& v);
    Vec3& operator*=(const Vec3& v);
    Vec3& operator/=(const Vec3& v);
    Vec3& operator*=(float f);
    Vec3& operator/=(float f);
    void normalize();
};

float length(const Vec3& v);
Vec3 normalized(const Vec3& v);
Vec3 cross(const Vec3& a, const Vec3& b);
float dot(const Vec3& a, const Vec3& b);
Vec3 boxnormal(const Vec3& p);

Vec3 operator-(const Vec3& v);
bool operator==(const Vec3& a, const Vec3& b);
Vec3 operator+(const Vec3& a, const Vec3& b);
Vec3 operator-(const Vec3& a, const Vec3& b);
Vec3 operator*(const Vec3& a, const Vec3& b);
Vec3 operator/(const Vec3& a, const Vec3& b);
Vec3 operator*(const Vec3& v, float f);
Vec3 operator*(float f, const Vec3& v);
Vec3 operator/(const Vec3& v, float f);

Vec3 lerp(const Vec3& a, const Vec3& b, float f);
Vec3 reflect(const Vec3& in, const Vec3& n);
Vec3 refract(const Vec3& in, const Vec3& n, float ior1, float ior2);
float fresnel(const Vec3& I, const Vec3& N, float ior1, float ior2);

#endif