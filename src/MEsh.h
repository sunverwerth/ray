#ifndef Mesh_h
#define Mesh_h

#include "Object.h"
#include <string>
#include "Vec3.h"
#include <map>

struct Vertex {
	Vec3 pos;
	Vec3 norm;
	Vec3 color;
	Vec3 uv;
};

struct AABB {
	Vec3 min = Vec3(99999999);
	Vec3 max = Vec3(-99999999);

	bool intersects(const AABB& other) const {
		return
			min.x <= other.max.x
			&& min.y <= other.max.y
			&& min.z <= other.max.z
			&& max.x >= other.min.x
			&& max.y >= other.min.y
			&& max.z >= other.min.z;
	}

	bool contains(const Vec3& p) const {
		return
			p.x >= min.x && p.x <= max.x
			&& p.y >= min.y && p.y <= max.y
			&& p.z >= min.z && p.z <= max.z;
	}

	void enclose(const Vec3& p) {
		if (p.x > max.x) max.x = p.x;
		if (p.x < min.x) min.x = p.x;
		if (p.y > max.y) max.y = p.y;
		if (p.y < min.y) min.y = p.y;
		if (p.z > max.z) max.z = p.z;
		if (p.z < min.z) min.z = p.z;
	}
};

struct Triangle {
	Vertex a;
	Vertex b;
	Vertex c;
	Material* material;
};

struct Cell {
	AABB aabb;
	std::vector<Triangle> triangles;
};

struct BspLight {
	Vec3 pos;
	float val;
};

struct Mesh : Object {
	Mesh(const std::string& filename);
	bool intersect(const Ray& ray, Hit* hit) final override;
	Vec3 getRandomPoint(Prng& prng) final override;
	Material* loadWal(const std::string& name, int lightLevel, float opacity);
	float getSurfaceArea() final override;

	void loadObj(const std::string& filename);
	void loadBsp(const std::string& filename);
	AABB bounds;
	std::vector<Cell> cells;
	std::map<std::string, Material*> textures;
	std::vector<BspLight> lights;
};

#endif