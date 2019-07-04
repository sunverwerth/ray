#ifndef Material_h
#define Material_h

#include <cmath>
#include <vector>

struct MaterialProperties {
	MaterialProperties(const Vec3& color, const Vec3& emission, float roughness, float opacity, float metallic, float ior) :
		color(color),
		emission(emission),
		roughness(roughness),
		opacity(opacity),
		metallic(metallic),
		ior(ior)
	{
	}

	Vec3 color;
	Vec3 emission;
	float roughness;
	float opacity;
	float metallic;
	float ior;
};

struct Material {
	virtual MaterialProperties sample(const Vec3& pos, const Vec3& uvw) = 0;
};

struct DefaultMaterial: Material {
	DefaultMaterial(const Vec3& color, const Vec3& emission = Vec3(0, 0, 0), float roughness = 0, float opacity = 1, float metallic = 0, float ior = 1.5f) : props(color, emission, roughness, opacity, metallic, ior) {
	}

	MaterialProperties sample(const Vec3& pos, const Vec3& uvw) override { return props; }

	MaterialProperties props;
};

struct Texture {
	int width;
	int height;
	std::vector<Vec3> data;

};
struct TextureMaterial : Material {
	
	MaterialProperties sample(const Vec3& pos, const Vec3& uvw) override {
		int x = int(texture->height * uvw.x) % texture->width;
		int y = int(texture->height * uvw.y) % texture->height;
		if (x < 0) x += texture->width;
		if (y < 0) y += texture->height;

		auto col = texture->data[y * texture->width + x];
		
		return {
			col,
			col * col * col * emission * 10,
			0.02f,
			opacity,
			0.0f,
			1.3f
		};
	}

	Vec3 emission;
	Texture* texture;
	float opacity;
};

struct CheckerMaterial: Material {
	CheckerMaterial():
		a(Vec3(0.3, 0.3, 0.3), Vec3(0, 0, 0), 0.0002, 1, 1, 1.5),
		b(Vec3(0.3, 0.3, 0.3), Vec3(0, 0, 0), 0.00002, 1, 1, 1.5) {
	}

	MaterialProperties sample(const Vec3& pos, const Vec3& uvw) override {
		return (int)(floor(pos.x*0.5) + floor(pos.z*0.5)) % 2 ? a : b;
	}

	MaterialProperties a;
	MaterialProperties b;
};

#endif