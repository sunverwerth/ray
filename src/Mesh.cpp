#include "Mesh.h"
#include "tiny_obj_loader.h"
#include <unordered_map>
#include "Material.h"
#include <fstream>

Vec3 palette[256];

Mesh::Mesh(const std::string& filename) {
	auto pal = std::ifstream("textures/colormap.pcx", std::ios::binary);
	pal.seekg(-768, pal.end);
	for (int i = 0; i < 256; i++) {
		uint8_t r, g, b;
		pal.read((char*)&r, 1);
		pal.read((char*)&g, 1);
		pal.read((char*)&b, 1);
		palette[i] = Vec3(r, g, b) / 255;
	}

	material = new DefaultMaterial(Vec3(0.9, 0.9, 0.9));
	//material = new CheckerMaterial();
	//loadObj(filename);
	loadBsp("demo1.bsp");
}

bool operator==(const Vertex& a, const Vertex& b) {
	return a.color == b.color && a.pos == b.pos && a.uv == b.uv;
}

namespace std {
	template<> struct hash<::Vec3> {
		size_t operator()(::Vec3 const& v) const {
			return v.x + v.y * 10000 + v.z * 100000000;
		}
	};

	template<> struct hash<::Vertex> {
		size_t operator()(::Vertex const& vertex) const {
			return ((hash<::Vec3>()(vertex.pos) ^
				(hash<::Vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<::Vec3>()(vertex.uv) << 1);
		}
	};
}

AABB enclose(const Vec3& a, const Vec3& b, const Vec3& c) {
	AABB aabb = AABB();
	aabb.enclose(a);
	aabb.enclose(b);
	aabb.enclose(c);
	return aabb;
}

#pragma pack(push, 1)
struct bsp_lump
{
	uint32_t    offset;     // offset (in bytes) of the data from the beginning of the file
	uint32_t    length;     // length (in bytes) of the data
};

struct bsp_header
{
	uint32_t    magic;      // magic number ("IBSP")
	uint32_t    version;    // version of the BSP format (38)
	bsp_lump  lump[19];   // directory of the lumps
};

struct bsp_edge {
	uint16_t a;
	uint16_t b;
};

struct bsp_face
{
	uint16_t   plane;             // index of the plane the face is parallel to
	uint16_t   plane_side;        // set if the normal is parallel to the plane normal

	uint32_t   first_edge;        // index of the first edge (in the face edge array)
	uint16_t   num_edges;         // number of consecutive edges (in the face edge array)

	uint16_t   texture_info;      // index of the texture info structure	

	uint8_t    lightmap_syles[4]; // styles (bit flags) for the lightmaps
	uint32_t   lightmap_offset;   // offset of the lightmap (in bytes) in the lightmap lump
};

typedef int32_t bsp_face_edge;

struct bsp_plane
{
	Vec3   normal;      // A, B, C components of the plane equation
	float     distance;    // D component of the plane equation
	uint32_t    type;        // ?
};

struct bsp_texinfo
{
	Vec3  u_axis;
	float    u_offset;

	Vec3 v_axis;
	float    v_offset;

	uint32_t   flags;
	uint32_t   value;

	char     texture_name[32];

	uint32_t   next_texinfo;
};

enum BspLump {
	kEntities = 0,
	kPlanes,
	kVertices,
	kVisibility,
	kNodes,
	kTextureInformation,
	kFaces,
	kLightmaps,
	kLeaves,
	kLeafFaceTable,
	kLeafBrushTable,
	kEdges,
	kFaceEdgeTable,
	kModels,
	kBrushes,
	kBrushSides,
	kPop,
	kAreas,
	kAreaPortals
};

#define	MIPLEVELS	4
struct miptex_s
{
	char		name[32];
	uint32_t	width, height;
	uint32_t	offsets[MIPLEVELS];		// four mip maps stored
	char		animname[32];			// next frame in animation chain
	int32_t			flags;
	int32_t			contents;
	int32_t			value;
};

#pragma pack(pop)

Material* Mesh::loadWal(const std::string& name, int lightLevel, float opacity) {
	std::string key = name + "_light:" + std::to_string(lightLevel) + "_opacity:" + std::to_string(opacity);
	auto it = textures.find(key);
	if (it != textures.end()) return it->second;

	Material* mat = material;

	auto file = std::ifstream("textures/" + name + ".wal", std::ios::binary);
	if (file.is_open() && file.good()) {
		miptex_s wal;
		file.read((char*)&wal, sizeof(miptex_s));
		auto tex = new Texture();
		tex->width = wal.width;
		tex->height = wal.height;
		tex->data.resize(tex->width* tex->height);
		file.seekg(wal.offsets[0]);
		for (int y = 0; y < wal.height; y++) {
			for (int x = 0; x < wal.width; x++) {
				uint8_t c;
				file.read((char*)&c, 1);
				tex->data[y * wal.width + x] = palette[c];
			}
		}
		auto tmat = new TextureMaterial();
		tmat->texture = tex;
		tmat->emission = (float)lightLevel / 2000.0f;
		tmat->opacity = opacity;
		mat = tmat;
	}

	textures[key] = mat;
	return mat;
}

void Mesh::loadBsp(const std::string& filename) {
	auto file = std::ifstream(filename, std::ios::binary | std::ios::ate);
	auto size = file.tellg();
	file.seekg(0);
	char* buf = new char[size];
	file.read(buf, size);

	auto header = (bsp_header*)buf;

	int numverts = header->lump[kVertices].length / sizeof(Vec3);
	auto verts = (Vec3*)(buf + header->lump[kVertices].offset);

	auto vertices = std::vector<Vertex>();
	vertices.reserve(numverts);
	for (int i = 0; i < numverts; i++) {
		vertices.push_back(Vertex{
			Vec3(verts[i].x, verts[i].z, verts[i].y) * 0.01
		});
	}

	int numedges = header->lump[kEdges].length / sizeof(bsp_edge);
	auto edges = (bsp_edge*)(buf + header->lump[kEdges].offset);

	int numfaces = header->lump[kFaces].length / sizeof(bsp_face);
	auto faces = (bsp_face*)(buf + header->lump[kFaces].offset);

	int numplanes = header->lump[kPlanes].length / sizeof(bsp_plane);
	auto planes = (bsp_plane*)(buf + header->lump[kPlanes].offset);

	int numfaceedges = header->lump[kFaceEdgeTable].length / sizeof(bsp_face_edge);
	auto faceedges = (bsp_face_edge*)(buf + header->lump[kFaceEdgeTable].offset);

	char* entities = (buf + header->lump[kEntities].offset);
	std::ofstream ents("entities.txt");
	ents << entities;
	ents.close();

	int numtexinfos = header->lump[kTextureInformation].length / sizeof(bsp_texinfo);
	auto texinfos = (bsp_texinfo*)(buf + header->lump[kTextureInformation].offset);
	for (int i = 0; i < numtexinfos; i++) {
		std::swap(texinfos[i].u_axis.y, texinfos[i].u_axis.z);
		std::swap(texinfos[i].v_axis.y, texinfos[i].v_axis.z);
	}
	std::vector<Triangle> triangles;
	for (int i = 0; i < numfaces; i++) {
		int startIndex;
		bool start = true;
		auto texinfo = texinfos[faces[i].texture_info];
		if (texinfo.flags == 0 || texinfo.flags == 1) {
			for (int j = faces[i].first_edge; j < faces[i].first_edge + faces[i].num_edges - 1; j++) {
				int a, b;
				if (faceedges[j] < 0) {
					a = edges[-faceedges[j]].b;
					b = edges[-faceedges[j]].a;
				}
				else {
					a = edges[faceedges[j]].a;
					b = edges[faceedges[j]].b;
				}
				if (start) {
					startIndex = a;
					start = false;
					continue;
				}
				auto v0 = vertices[startIndex];
				auto v1 = vertices[a];
				auto v2 = vertices[b];
				float opacity = 1;
				auto wal = (TextureMaterial*)loadWal(texinfo.texture_name, (texinfo.flags & 1) ? texinfo.value : 0, opacity);
				v0.uv.x = (dot(v0.pos * 100, texinfo.u_axis) + texinfo.u_offset) / wal->texture->height;
				v0.uv.y = (dot(v0.pos * 100, texinfo.v_axis) + texinfo.v_offset) / wal->texture->height;
				v1.uv.x = (dot(v1.pos * 100, texinfo.u_axis) + texinfo.u_offset) / wal->texture->height;
				v1.uv.y = (dot(v1.pos * 100, texinfo.v_axis) + texinfo.v_offset) / wal->texture->height;
				v2.uv.x = (dot(v2.pos * 100, texinfo.u_axis) + texinfo.u_offset) / wal->texture->height;
				v2.uv.y = (dot(v2.pos * 100, texinfo.v_axis) + texinfo.v_offset) / wal->texture->height;
				/*
				u = x * u_axis.x + y * u_axis.y + z * u_axis.z + u_offset
				v = x * v_axis.x + y * v_axis.y + z * v_axis.z + v_offset
				*/
				triangles.push_back({ v0, v1, v2, wal });
			}
		}
	}

	for (auto& vertex : vertices) {
		bounds.enclose(vertex.pos);
	}

	/*triangles.clear();
	auto wal = loadWal("e1u1/color1_3");
	triangles.push_back({
		{
			{0,0,2},
			{0,1,0},
			{1,1,1},
			{0,1,0}
		},
		{
			{0,1,2},
			{0,1,0},
			{1,1,1},
			{0,0,0}
		},
		{
			{1,1,2},
			{0,1,0},
			{1,1,1},
			{1,0,0}
		},
		wal
	});
	triangles.push_back({
		{
			{0,0,2},
			{0,1,0},
			{1,1,1},
			{0,1,0}
		},
		{
			{1,1,2},
			{0,1,0},
			{1,1,1},
			{1,0,0}
		},
		{
			{1,0,2},
			{0,1,0},
			{1,1,1},
			{1,1,0}
		},
		wal
	});*/

	Vec3 numcells = (bounds.max - bounds.min) * 0.5;
	numcells.x = 1 + (int)numcells.x;
	numcells.y = 1 + (int)numcells.y;
	numcells.z = 1 + (int)numcells.z;
	Vec3 cellsize = (bounds.max - bounds.min) / numcells;
	for (int x = 0; x < numcells.x; x++) {
		for (int y = 0; y < numcells.y; y++) {
			for (int z = 0; z < numcells.z; z++) {
				cells.push_back(Cell());
				auto& cell = cells.back();
				cell.aabb = { bounds.min + Vec3(x, y, z) * cellsize, bounds.min + Vec3(x + 1, y + 1, z + 1) * cellsize };
				for (auto& tri: triangles) {
					auto triaabb = enclose(tri.a.pos, tri.b.pos, tri.c.pos);
					if (cell.aabb.intersects(triaabb)) {
						cell.triangles.push_back(tri);
					}
				}
				if (cell.triangles.empty()) cells.pop_back();
			}
		}
	}

	delete buf;
}

void Mesh::loadObj(const std::string& filename) {
	/*vertices.clear();
	indices.clear();

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str())) {
		throw std::runtime_error(warn + err);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex = {};

			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.uv = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
				0
			};

			vertex.color = { 1.0f, 1.0f, 1.0f };

			vertices.push_back(vertex);

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}
	}

	for (auto& vertex : vertices) {
		if (vertex.pos.x > bounds.max.x) bounds.max.x = vertex.pos.x;
		else if (vertex.pos.x < bounds.min.x) bounds.min.x = vertex.pos.x;
		if (vertex.pos.y > bounds.max.y) bounds.max.y = vertex.pos.y;
		else if (vertex.pos.y < bounds.min.y) bounds.min.y = vertex.pos.y;
		if (vertex.pos.z > bounds.max.z) bounds.max.z = vertex.pos.z;
		else if (vertex.pos.z < bounds.min.z) bounds.min.z = vertex.pos.z;
	}

	Vec3 numcells = (bounds.max - bounds.min);
	numcells.x = 1 + (int)numcells.x;
	numcells.y = 1 + (int)numcells.y;
	numcells.z = 1 + (int)numcells.z;
	Vec3 cellsize = (bounds.max - bounds.min) / numcells;
	for (int x = 0; x < numcells.x; x++) {
		for (int y = 0; y < numcells.y; y++) {
			for (int z = 0; z < numcells.z; z++) {
				cells.push_back(Cell());
				auto& cell = cells.back();
				cell.aabb = { bounds.min + Vec3(x, y, z) * cellsize, bounds.min + Vec3(x + 1, y + 1, z + 1) * cellsize };
				for (int i = 0; i < indices.size(); i += 3) {
					auto& a = vertices[indices[i]];
					auto& b = vertices[indices[i + 1]];
					auto& c = vertices[indices[i + 2]];

					if (cell.aabb.intersects(enclose(a.pos, b.pos, c.pos))) {
						cell.triangles.push_back({ a, b, c });
					}
				}
				if (cell.triangles.empty()) cells.pop_back();
			}
		}
	}*/
}

bool rayTriangle(const Ray& ray, const Vertex& v0, const Vertex& v1, const Vertex& v2, Hit* hit) {
#if true
	Vec3 edge1, edge2, h, s, q;
	float a, f, u, v;
	edge1 = v1.pos - v0.pos;
	edge2 = v2.pos - v0.pos;
	h = cross(ray.direction, edge2);
	a = dot(edge1, h);
	if (a > -kEpsilon && a < kEpsilon)
		return false;    // This ray is parallel to this triangle.
	f = 1.0 / a;
	s = ray.origin - v0.pos;
	u = f * dot(s, h);
	if (u < 0.0 || u > 1.0)
		return false;
	q = cross(s, edge1);
	v = f * dot(ray.direction, q);
	if (v < 0.0 || u + v > 1.0)
		return false;
	// At this stage we can compute t to find out where the intersection point is on the line.
	float t = f * dot(edge2, q);
	if (t < kEpsilon)
		return false;

	if (hit) {
		if (t > hit->distance || t < hit->minDistance) return false;
		hit->distance = t;
		hit->normal = normalized(cross(edge1, edge2));
		hit->uvw = v0.uv + (v1.uv - v0.uv) * u + (v2.uv - v0.uv) * v;
	}

	return true;
#else	
	// compute plane's normal
	Vec3 v0v1 = v1.pos - v0.pos;
	Vec3 v0v2 = v2.pos - v0.pos;
	// no need to normalize
	Vec3 N = cross(v0v1, v0v2); // N
	float area2 = dot(N, N);
	if (area2 < kEpsilon) return false;

	float ddn = dot(ray.direction, N);
	float t = dot(v0.pos - ray.origin, N) / ddn;
	if (t < 0 || (hit && t > hit->distance)) return false;

	Vec3 P = ray.origin + ray.direction * t;

	// Step 2: inside-outside test
	Vec3 C; // vector perpendicular to triangle's plane

	// edge 0
	Vec3 vp0 = P - v0.pos;
	C = cross(v0v1, vp0);
	if (dot(N, C) < 0) return false; // P is on the right side

	// edge 2
	Vec3 vp2 = P - v2.pos;
	C = cross(-v0v2, vp2);
	if (dot(N, C) < 0) return false; // P is on the right side;

    // edge 1
	Vec3 edge1 = v2.pos - v1.pos;
	Vec3 vp1 = P - v1.pos;
	C = cross(edge1, vp1);
	if (dot(N, C) < 0) return false; // P is on the right side

	if (hit) {
		hit->distance = t;
		hit->normal = N;
	}
	return true; // this ray hits the triangle
#endif
}

float testAABB(const Ray& ray, const AABB& aabb) {
	auto origin = ray.origin - (aabb.min + aabb.max) / 2;
	auto hsize = (aabb.max - aabb.min) / 2;

	float tmin = (-hsize.x - origin.x) / ray.direction.x;
	float tmax = (hsize.x - origin.x) / ray.direction.x;

	if (tmin > tmax) std::swap(tmin, tmax);

	float tymin = (-hsize.y - origin.y) / ray.direction.y;
	float tymax = (hsize.y - origin.y) / ray.direction.y;

	if (tymin > tymax) std::swap(tymin, tymax);

	if ((tmin > tymax) || (tymin > tmax))
		return -1;

	if (tymin > tmin)
		tmin = tymin;

	if (tymax < tmax)
		tmax = tymax;

	float tzmin = (-hsize.z - origin.z) / ray.direction.z;
	float tzmax = (hsize.z - origin.z) / ray.direction.z;

	if (tzmin > tzmax) std::swap(tzmin, tzmax);

	if ((tmin > tzmax) || (tzmin > tmax))
		return -1;

	if (tzmin > tmin)
		tmin = tzmin;

	if (tzmax < tmax)
		tmax = tzmax;

	if (aabb.contains(ray.origin)) {
		tmin = tmax;
	}

	return tmin;
}

struct SortedCell {
	Cell* cell;
	float t;
};

bool Mesh::intersect(const Ray& ray, Hit* hit) {
	static thread_local std::vector<SortedCell> sortedcells;

	float t = testAABB(ray, bounds);
	if (t < 0 || hit && (t > hit->distance || t < hit->minDistance)) return false;

	bool isHit = false;
	Hit myHit;
	if (hit) {
		myHit.distance = hit->distance;
		myHit.minDistance = hit->minDistance;
	}

	sortedcells.clear();
	for (auto& cell : cells) {
		float t = kEpsilon;
		t = testAABB(ray, cell.aabb);
		if (t < 0 || t > myHit.distance || t < myHit.minDistance) continue;
		sortedcells.push_back({ &cell, t });
	}
	
	std::sort(sortedcells.begin(), sortedcells.end(), [](const SortedCell& a, const SortedCell& b) {
		return a.t < b.t;
	});

	for (auto& scell : sortedcells) {
		auto& cell = *scell.cell;
		for (auto& tri: cell.triangles) {
			if (rayTriangle(ray, tri.a, tri.b, tri.c, &myHit)) {
				myHit.material = tri.material;
				isHit = true;
			}
		}

		if (isHit) break;
	}

	if (hit && isHit) {
		if (myHit.distance > hit->distance || myHit.distance < hit->minDistance) return false;
		*hit = myHit;
		hit->obj = this;
		hit->normal = normalized(myHit.normal);
	}

	return isHit;
}

float Mesh::getSurfaceArea() {
	return 1;
}

Vec3 Mesh::getRandomPoint(Prng& prng) {
	return Vec3(0, 0, 0);
}
