#include "zone/map_geometry.h"

#include <bit>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string_view>

namespace {

enum class Verdict : uint32 {
	Invalid = 0,
	Empty = 1,
	Geometry = 2,
};

void Write(FILE *output, const void *bytes, size_t size)
{
	if (fwrite(bytes, 1, size, output) != size) {
		throw std::runtime_error("cannot write oracle output");
	}
}

void WriteWord(FILE *output, uint32 word)
{
	Write(output, &word, sizeof(word));
}

bool Decode(FILE *input, std::vector<glm::vec3> &vertices, std::vector<uint32> &indices)
{
	uint32 version;
	if (fread(&version, sizeof(version), 1, input) != 1) {
		return false;
	}

	// Mirror Map::Load's version dispatch. The decoder reads past the version word.
	switch (version) {
	case 0x01000000:
		return MapGeometry::LoadV1(input, vertices, indices);
	case 0x02000000:
		return MapGeometry::LoadV2(input, vertices, indices);
	default:
		return false;
	}
}

} // namespace

int main(int argc, char **argv)
try {
	static_assert(std::endian::native == std::endian::little);
	static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559);
	static_assert(sizeof(glm::vec3) == 3 * sizeof(float));

	if (argc != 4) {
		throw std::runtime_error("usage: map_decoder_oracle INPUT.map OUTPUT.oracle INPUT_SHA256");
	}

	const std::string_view identity = argv[3];
	if (identity.size() != 64 || identity.find_first_not_of("0123456789abcdef") != std::string_view::npos) {
		throw std::runtime_error("expected lowercase input SHA-256");
	}

	std::unique_ptr<FILE, decltype(&fclose)> input(fopen(argv[1], "rb"), &fclose);
	if (!input) {
		throw std::runtime_error("cannot open map input");
	}

	std::vector<glm::vec3> vertices;
	std::vector<uint32> indices;
	const bool accepted = Decode(input.get(), vertices, indices);
	if (ferror(input.get())) {
		throw std::runtime_error("cannot read map input");
	}

	Verdict verdict = Verdict::Invalid;
	if (accepted) {
		verdict = indices.empty() ? Verdict::Empty : Verdict::Geometry;
	} else {
		// A rejected decode may already have appended geometry.
		vertices.clear();
		indices.clear();
	}

	std::unique_ptr<FILE, decltype(&fclose)> output(fopen(argv[2], "wbx"), &fclose);
	if (!output) {
		throw std::runtime_error("cannot exclusively create oracle output");
	}
	Write(output.get(), "EQMAP001", 8);
	WriteWord(output.get(), static_cast<uint32>(verdict));
	WriteWord(output.get(), static_cast<uint32>(vertices.size()));
	WriteWord(output.get(), static_cast<uint32>(indices.size()));
	Write(output.get(), identity.data(), identity.size());

	for (const auto &vertex : vertices) {
		WriteWord(output.get(), std::bit_cast<uint32>(vertex.x));
		WriteWord(output.get(), std::bit_cast<uint32>(vertex.y));
		WriteWord(output.get(), std::bit_cast<uint32>(vertex.z));
	}
	for (uint32 index : indices) {
		WriteWord(output.get(), index);
	}
	if (fclose(output.release()) != 0) {
		throw std::runtime_error("cannot close oracle output");
	}
	return 0;
} catch (const std::exception &error) {
	std::cerr << error.what() << '\n';
	return 1;
}
