#include "EnvironmentMap.h"

#include <string>
#include <vector>

Vec3 rgbeToColor(RGBE data) {
	float f = ldexp(1.0f, data.e - (int)(128 + 8));
	return Vec3(data.r * f, data.g * f, data.b * f);
}

EnvironmentMap::EnvironmentMap(std::istream& file) {
	std::string line;

	do {
		std::getline(file, line);
	} while (!line.empty());

	// -Y 512 +X 1024
	std::getline(file, line);

	size_t a = line.find(' ', 0);
	size_t b = line.find(' ', a + 1);
	size_t c = line.find(' ', b + 1);

	width = std::stoi(line.substr(c + 1));
	height = std::stoi(line.substr(a + 1, b - a - 1));
	data = new RGBE[width * height];

	RGBE p;
	uint8_t buf[2];

	for (int y = 0; y < height; y++) {
		file.read((char*)&p, sizeof(p));
		for (int i = 0; i < 4; i++) {
			uint8_t* ptr = (uint8_t*)(&data[y * width]) + i;
			for (int x = 0; x < width;) {
				file.read((char*)buf, 2);
				if (buf[0] > 128) {
					int count = buf[0] - 128;
					if (count == 0 || count > width - x) {
						exit(1);
					}
					x += count;
					while (count-- > 0) {
						*ptr = buf[1];
						ptr += sizeof(RGBE);
					}
				}
				else {
					int count = buf[0];
					if (count == 0 || count > width - x) {
						exit(1);
					}
					x += count;
					*ptr = buf[1];
					ptr += sizeof(RGBE);
					while (--count > 0) {
						file.read((char*)ptr, 1);
						ptr += sizeof(RGBE);
					}
				}
			}
		}
	}
}

Vec3 EnvironmentMap::texcoordToVector(float x, float y) {
	float theta = (0.75f - (x + 0.5f) / width) * 2 * M_PI;
	float phi = (0.5f - (y + 0.5f) / height) * 0.5f * M_PI;
	return Vec3(
		::cos(phi) * ::cos(theta),
		::sin(phi),
		::cos(phi) * ::sin(theta)
	);
}

Vec3 EnvironmentMap::sample(const Vec3& dir) {
	unsigned int x = width / 2 + ::atan2(dir.x, dir.z) / M_PI * width * 0.5f;
	unsigned int y = (1 - ::asin(dir.y) / M_PI * 2) * height * 0.5f;
	x %= width;
	y %= height;
	return rgbeToColor(data[y * width + x]);
}