#include "zone/map_geometry.h"
#include <cassert>
#include <cstring>
#include <limits>
#include <zlib.h>

#ifdef NDEBUG
#error "geometry_test requires active assertions"
#endif

using Bytes = std::vector<char>;
template<class T> void append(Bytes& bytes, T value) {
	const auto* data = reinterpret_cast<const char*>(&value);
	bytes.insert(bytes.end(), data, data + sizeof(value));
}
Bytes header(uint32 vertices = 0, uint32 indices = 0, uint32 models = 0) {
	Bytes bytes;
	for (uint32 count : {vertices, indices, 0u, 0u, models, 0u, 0u, 0u, 0u}) append(bytes, count);
	append(bytes, 1.0f);
	return bytes;
}
bool decode(const Bytes& bytes, std::vector<glm::vec3>& vertices, std::vector<uint32>& indices) {
	uLongf size = compressBound(bytes.size());
	Bytes compressed(size);
	assert(compress(reinterpret_cast<Bytef*>(compressed.data()), &size,
		reinterpret_cast<const Bytef*>(bytes.data()), bytes.size()) == Z_OK);
	auto* file = tmpfile(); assert(file);
	uint32 packed = size, unpacked = bytes.size();
	assert(fwrite(&packed, sizeof(packed), 1, file) == 1);
	assert(fwrite(&unpacked, sizeof(unpacked), 1, file) == 1);
	assert(fwrite(compressed.data(), size, 1, file) == 1);
	rewind(file);
	bool result = MapGeometry::LoadV2(file, vertices, indices);
	fclose(file);
	return result;
}
int main() {
	std::vector<glm::vec3> vertices;
	std::vector<uint32> indices;
	auto run = [&](const Bytes& bytes) { vertices.clear(); indices.clear(); return decode(bytes, vertices, indices); };
	assert(run(header()) && vertices.empty() && indices.empty());
	assert(!run(Bytes{0}));
	assert(!run(header(8000001)));
	auto triangle = header(3, 3);
	for (float value : {0.f,0.f,0.f,4.f,0.f,0.f,0.f,4.f,0.f}) append(triangle, value);
	for (uint32 index : {0u,1u,2u}) append(triangle, index);
	assert(run(triangle) && vertices.size() == 3 && indices.size() == 3);
	for (size_t end = 0; end < triangle.size(); ++end) {
		// Empty packed input is rejected before decompression.
		if (end) assert(!run(Bytes(triangle.begin(), triangle.begin() + end)));
	}
	// A valid flat tile must never repair an invalid base-mesh index.
	auto expanded = triangle;
	uint32 tile_count = 1, quads = 1;
	std::memcpy(expanded.data() + 28, &tile_count, 4);
	std::memcpy(expanded.data() + 32, &quads, 4);
	append(expanded, uint8(1));
	for (float coordinate : {10.f, 10.f, 0.f}) append(expanded, coordinate);
	assert(run(expanded) && vertices.size() == 7 && indices.size() == 9);
	uint32 formerly_repaired = 3;
	std::memcpy(expanded.data() + triangle.size() - 4, &formerly_repaired, 4);
	assert(!run(expanded));
	auto terrain = header();
	std::memcpy(terrain.data() + 28, &tile_count, 4);
	std::memcpy(terrain.data() + 32, &quads, 4);
	append(terrain, uint8(0));
	append(terrain, 10.f); append(terrain, 10.f);
	append(terrain, uint8(0));
	for (float height : {1.f, 2.f, 3.f, 4.f}) append(terrain, height);
	assert(run(terrain) && vertices.size() == 4 && indices.size() == 6);
	terrain[40] = 2; // Reject invalid bool representation.
	assert(!run(terrain));
	auto placements = header(0, 0, 1);
	uint32 one = 1;
	std::memcpy(placements.data() + 20, &one, 4);
	std::memcpy(placements.data() + 24, &one, 4);
	auto name = [&] { placements.push_back('m'); placements.push_back(0); };
	auto transform = [&] {
		for (float value : {0.f,0.f,0.f,0.f,0.f,0.f,1.f,1.f,1.f}) append(placements, value);
	};
	name(); append(placements, 3u); append(placements, 1u);
	for (float value : {0.f,0.f,0.f,4.f,0.f,0.f,0.f,4.f,0.f}) append(placements, value);
	for (uint32 index : {0u,1u,2u}) append(placements, index);
	append(placements, uint8(1));
	name(); transform(); // Ordinary model placement.
	transform(); // Group transform, tile translation, and member count.
	for (int axis = 0; axis < 3; ++axis) append(placements, 0.f);
	append(placements, 1u);
	name(); transform();
	assert(run(placements) && vertices.size() == 6 && indices.size() == 6);
	auto invalid = triangle;
	uint32 bad_index = 3;
	std::memcpy(invalid.data() + invalid.size() - 4, &bad_index, 4);
	assert(!run(invalid));
	invalid = triangle;
	float nonfinite = std::numeric_limits<float>::quiet_NaN();
	std::memcpy(invalid.data() + 40, &nonfinite, 4);
	assert(!run(invalid));
	auto model = header(0, 0, 1);
	model.insert(model.end(), {'a','b','c'}); // Missing name terminator.
	assert(!run(model));
	model.push_back(0); append(model, 1u); append(model, 1u);
	for (int axis = 0; axis < 3; ++axis) append(model, 0.f);
	for (uint32 index : {0u,0u,1u}) append(model, index);
	append(model, uint8(1)); // Packed polygon has an invalid local index.
	assert(!run(model));
}
