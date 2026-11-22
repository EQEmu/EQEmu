/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "map_geometry.h"
#include "common/compression.h"
#include <cmath>
#include <cstring>
#include <map>
#include <memory>
#include <tuple>
#include <string>
#include <stdexcept>
#include <type_traits>

bool MapGeometry::LoadV1(FILE *f, std::vector<glm::vec3> &verts, std::vector<uint32> &indices) {
	uint32 face_count;
	uint16 node_count;
	uint32 facelist_count;

	if(fread(&face_count, sizeof(face_count), 1, f) != 1) {
		return false;
	}

	if(fread(&node_count, sizeof(node_count), 1, f) != 1) {
		return false;
	}

	if(fread(&facelist_count, sizeof(facelist_count), 1, f) != 1) {
		return false;
	}

	if (face_count > 8000000 / 3) return false;
	for(uint32 i = 0; i < face_count; ++i) {
		glm::vec3 a;
		glm::vec3 b;
		glm::vec3 c;
		float normals[4];
		if(fread(&a, sizeof(glm::vec3), 1, f) != 1) {
			return false;
		}

		if(fread(&b, sizeof(glm::vec3), 1, f) != 1) {
			return false;
		}

		if(fread(&c, sizeof(glm::vec3), 1, f) != 1) {
			return false;
		}

		if(fread(normals, sizeof(normals), 1, f) != 1) {
			return false;
		}

		for (const auto& point : {a, b, c}) {
			if (!std::isfinite(point.x) || !std::isfinite(point.y) || !std::isfinite(point.z)) return false;
		}
		size_t sz = verts.size();
		verts.push_back(a);
		indices.push_back((uint32)sz);

		verts.push_back(b);
		indices.push_back((uint32)sz + 1);

		verts.push_back(c);
		indices.push_back((uint32)sz + 2);
	}

	return true;
}

struct ModelEntry
{
	struct Poly
	{
		uint32 v1, v2, v3;
		uint8 vis;
	};
	std::vector<glm::vec3> verts;
	std::vector<Poly> polys;
};

// Packed V2 fields may be unaligned. Check every read before copying it.
namespace {
constexpr size_t MaxVertices = 8000000;
constexpr size_t MaxIndices = 12000000;
constexpr uint32 MaxPackedBytes = 512 * 1024 * 1024;
class MapReader {
	const char *current;
	size_t remaining;
public:
	explicit MapReader(const std::vector<char>& bytes) : current(bytes.data()), remaining(bytes.size()) {}
	void Skip(size_t count) {
		if (count > remaining) throw std::runtime_error("truncated map geometry");
		current += count;
		remaining -= count;
	}
	template<class T> T Read() {
		if (sizeof(T) > remaining) throw std::runtime_error("truncated map scalar");
		T value;
		std::memcpy(&value, current, sizeof(T));
		Skip(sizeof(T));
		if constexpr (std::is_floating_point_v<T>) {
			if (!std::isfinite(value)) throw std::runtime_error("nonfinite map scalar");
		}
		return value;
	}
	std::string Name() {
		auto end = static_cast<const char*>(std::memchr(current, 0, remaining));
		if (!end) throw std::runtime_error("unterminated map model name");
		std::string name(current, end);
		Skip(name.size() + 1);
		return name;
	}
};
void CheckGrowth(size_t vertices, size_t indices, size_t add_vertices, size_t add_indices) {
	if (vertices > MaxVertices || add_vertices > MaxVertices - vertices ||
		indices > MaxIndices || add_indices > MaxIndices - indices)
		throw std::runtime_error("map geometry count limit");
}
}

bool MapGeometry::LoadV2(FILE *f, std::vector<glm::vec3> &verts, std::vector<uint32> &indices) try {
	uint32 data_size;
	if (fread(&data_size, sizeof(data_size), 1, f) != 1) {
		return false;
	}

	uint32 buffer_size;
	if (fread(&buffer_size, sizeof(buffer_size), 1, f) != 1) {
		return false;
	}

	if (!data_size || data_size > MaxPackedBytes || !buffer_size || buffer_size > MaxPackedBytes) return false;
	std::vector<char> data;
	data.resize(data_size);
	if (fread(&data[0], data_size, 1, f) != 1) {
		return false;
	}

	std::vector<char> buffer;
	buffer.resize(buffer_size);
	uint32 v = EQ::InflateData(&data[0], data_size, &buffer[0], buffer_size);

	if (v != buffer_size) return false;
	MapReader buf(buffer);
	uint32 vert_count;
	uint32 ind_count;
	uint32 nc_vert_count;
	uint32 nc_ind_count;
	uint32 model_count;
	uint32 plac_count;
	uint32 plac_group_count;
	uint32 tile_count;
	uint32 quads_per_tile;
	float units_per_vertex;

	vert_count = buf.Read<uint32>();

	ind_count = buf.Read<uint32>();

	nc_vert_count = buf.Read<uint32>();

	nc_ind_count = buf.Read<uint32>();

	model_count = buf.Read<uint32>();

	plac_count = buf.Read<uint32>();

	plac_group_count = buf.Read<uint32>();

	tile_count = buf.Read<uint32>();

	quads_per_tile = buf.Read<uint32>();

	units_per_vertex = buf.Read<float>();

	CheckGrowth(0, 0, vert_count, ind_count);
	if (ind_count % 3) return false;
	verts.reserve(vert_count);
	indices.reserve(ind_count);

	for (uint32 i = 0; i < vert_count; ++i) {
		float x;
		float y;
		float z;

		x = buf.Read<float>();

		y = buf.Read<float>();

		z = buf.Read<float>();

		verts.emplace_back(x, y, z);
	}

	for (uint32 i = 0; i < ind_count; ++i) {
		const auto index = buf.Read<uint32>();
		if (index >= vert_count) return false;
		indices.emplace_back(index);
	}

	for (uint32 i = 0; i < nc_vert_count; ++i) {
		buf.Skip(sizeof(float) * 3);
	}

	for (uint32 i = 0; i < nc_ind_count; ++i) {
		buf.Skip(sizeof(uint32));
	}

	std::map<std::string, std::unique_ptr<ModelEntry>> models;
	for (uint32 i = 0; i < model_count; ++i) {
		std::unique_ptr<ModelEntry> me(new ModelEntry);
		std::string name = buf.Name();

		uint32 vert_count = buf.Read<uint32>();

		uint32 poly_count = buf.Read<uint32>();

		CheckGrowth(0, 0, vert_count, size_t(poly_count) * 3);
		me->verts.reserve(vert_count);
		for (uint32 j = 0; j < vert_count; ++j) {
			float x = buf.Read<float>();
			float y = buf.Read<float>();
			float z = buf.Read<float>();

			me->verts.emplace_back(x, y, z);
		}

		me->polys.reserve(poly_count);
		for (uint32 j = 0; j < poly_count; ++j) {
			uint32 v1 = buf.Read<uint32>();
			uint32 v2 = buf.Read<uint32>();
			uint32 v3 = buf.Read<uint32>();
			uint8 vis = buf.Read<uint8>();

			if (v1 >= vert_count || v2 >= vert_count || v3 >= vert_count) return false;
			ModelEntry::Poly p;
			p.v1 = v1;
			p.v2 = v2;
			p.v3 = v3;
			p.vis = vis;
			me->polys.push_back(p);
		}

		models[name] = std::move(me);
	}

	for (uint32 i = 0; i < plac_count; ++i) {
		std::string name = buf.Name();

		float x = buf.Read<float>();
		float y = buf.Read<float>();
		float z = buf.Read<float>();

		float x_rot = buf.Read<float>();
		float y_rot = buf.Read<float>();
		float z_rot = buf.Read<float>();

		float x_scale = buf.Read<float>();
		float y_scale = buf.Read<float>();
		float z_scale = buf.Read<float>();

		if (models.count(name) == 0) return false;

		auto &model = models[name];
		auto &mod_polys = model->polys;
		auto &mod_verts = model->verts;
		for (uint32 j = 0; j < mod_polys.size(); ++j) {
			auto &current_poly = mod_polys[j];
			if (current_poly.vis == 0)
				continue;
			auto v1 = mod_verts[current_poly.v1];
			auto v2 = mod_verts[current_poly.v2];
			auto v3 = mod_verts[current_poly.v3];

			RotateVertex(v1, x_rot, y_rot, z_rot);
			RotateVertex(v2, x_rot, y_rot, z_rot);
			RotateVertex(v3, x_rot, y_rot, z_rot);

			ScaleVertex(v1, x_scale, y_scale, z_scale);
			ScaleVertex(v2, x_scale, y_scale, z_scale);
			ScaleVertex(v3, x_scale, y_scale, z_scale);

			TranslateVertex(v1, x, y, z);
			TranslateVertex(v2, x, y, z);
			TranslateVertex(v3, x, y, z);

			CheckGrowth(verts.size(), indices.size(), 3, 3);
			verts.emplace_back(v1.y, v1.x, v1.z); // x/y swapped
			verts.emplace_back(v2.y, v2.x, v2.z);
			verts.emplace_back(v3.y, v3.x, v3.z);

			indices.emplace_back((uint32)verts.size() - 3);
			indices.emplace_back((uint32)verts.size() - 2);
			indices.emplace_back((uint32)verts.size() - 1);
		}
	}

	for (uint32 i = 0; i < plac_group_count; ++i) {
		float x = buf.Read<float>();
		float y = buf.Read<float>();
		float z = buf.Read<float>();

		float x_rot = buf.Read<float>();
		float y_rot = buf.Read<float>();
		float z_rot = buf.Read<float>();

		float x_scale = buf.Read<float>();
		float y_scale = buf.Read<float>();
		float z_scale = buf.Read<float>();

		float x_tile = buf.Read<float>();
		float y_tile = buf.Read<float>();
		float z_tile = buf.Read<float>();

		uint32 p_count = buf.Read<uint32>();

		for (uint32 j = 0; j < p_count; ++j) {
			std::string name = buf.Name();

			float p_x = buf.Read<float>();
			float p_y = buf.Read<float>();
			float p_z = buf.Read<float>();

			float p_x_rot = buf.Read<float>() * 3.14159f / 180;
			float p_y_rot = buf.Read<float>() * 3.14159f / 180;
			float p_z_rot = buf.Read<float>() * 3.14159f / 180;

			float p_x_scale = buf.Read<float>();
			float p_y_scale = buf.Read<float>();
			float p_z_scale = buf.Read<float>();

			if (models.count(name) == 0) return false;

			auto &model = models[name];

			for (size_t k = 0; k < model->polys.size(); ++k) {
				auto &poly = model->polys[k];
				if (poly.vis == 0)
					continue;
				glm::vec3 v1, v2, v3;

				v1 = model->verts[poly.v1];
				v2 = model->verts[poly.v2];
				v3 = model->verts[poly.v3];

				ScaleVertex(v1, p_x_scale, p_y_scale, p_z_scale);
				ScaleVertex(v2, p_x_scale, p_y_scale, p_z_scale);
				ScaleVertex(v3, p_x_scale, p_y_scale, p_z_scale);

				TranslateVertex(v1, p_x, p_y, p_z);
				TranslateVertex(v2, p_x, p_y, p_z);
				TranslateVertex(v3, p_x, p_y, p_z);

				RotateVertex(v1, x_rot * 3.14159f / 180.0f, 0, 0);
				RotateVertex(v2, x_rot * 3.14159f / 180.0f, 0, 0);
				RotateVertex(v3, x_rot * 3.14159f / 180.0f, 0, 0);

				RotateVertex(v1, 0, y_rot * 3.14159f / 180.0f, 0);
				RotateVertex(v2, 0, y_rot * 3.14159f / 180.0f, 0);
				RotateVertex(v3, 0, y_rot * 3.14159f / 180.0f, 0);

				glm::vec3 correction(p_x, p_y, p_z);

				RotateVertex(correction, x_rot * 3.14159f / 180.0f, 0, 0);

				TranslateVertex(v1, -correction.x, -correction.y, -correction.z);
				TranslateVertex(v2, -correction.x, -correction.y, -correction.z);
				TranslateVertex(v3, -correction.x, -correction.y, -correction.z);

				RotateVertex(v1, p_x_rot, 0, 0);
				RotateVertex(v2, p_x_rot, 0, 0);
				RotateVertex(v3, p_x_rot, 0, 0);

				RotateVertex(v1, 0, -p_y_rot, 0);
				RotateVertex(v2, 0, -p_y_rot, 0);
				RotateVertex(v3, 0, -p_y_rot, 0);

				RotateVertex(v1, 0, 0, p_z_rot);
				RotateVertex(v2, 0, 0, p_z_rot);
				RotateVertex(v3, 0, 0, p_z_rot);

				TranslateVertex(v1, correction.x, correction.y, correction.z);
				TranslateVertex(v2, correction.x, correction.y, correction.z);
				TranslateVertex(v3, correction.x, correction.y, correction.z);

				RotateVertex(v1, 0, 0, z_rot * 3.14159f / 180.0f);
				RotateVertex(v2, 0, 0, z_rot * 3.14159f / 180.0f);
				RotateVertex(v3, 0, 0, z_rot * 3.14159f / 180.0f);

				ScaleVertex(v1, x_scale, y_scale, z_scale);
				ScaleVertex(v2, x_scale, y_scale, z_scale);
				ScaleVertex(v3, x_scale, y_scale, z_scale);

				TranslateVertex(v1, x_tile, y_tile, z_tile);
				TranslateVertex(v2, x_tile, y_tile, z_tile);
				TranslateVertex(v3, x_tile, y_tile, z_tile);

				TranslateVertex(v1, x, y, z);
				TranslateVertex(v2, x, y, z);
				TranslateVertex(v3, x, y, z);

				CheckGrowth(verts.size(), indices.size(), 3, 3);
			verts.emplace_back(v1.y, v1.x, v1.z); // x/y swapped
				verts.emplace_back(v2.y, v2.x, v2.z);
				verts.emplace_back(v3.y, v3.x, v3.z);

				indices.emplace_back((uint32)verts.size() - 3);
				indices.emplace_back((uint32)verts.size() - 2);
				indices.emplace_back((uint32)verts.size() - 1);
			}
		}
	}

	if (quads_per_tile > 2048) return false;
	uint32 ter_quad_count = (quads_per_tile * quads_per_tile);
	uint32 ter_vert_count = ((quads_per_tile + 1) * (quads_per_tile + 1));
	std::vector<uint8> flags;
	std::vector<float> floats;
	flags.resize(ter_quad_count);
	floats.resize(ter_vert_count);
	for (uint32 i = 0; i < tile_count; ++i) {
		bool flat;
		auto flat_byte = buf.Read<uint8>();
		if (flat_byte > 1) return false;
		flat = flat_byte != 0;

		float x;
		x = buf.Read<float>();

		float y;
		y = buf.Read<float>();

		if (flat) {
			float z;
			z = buf.Read<float>();

			float QuadVertex1X = x;
			float QuadVertex1Y = y;
			float QuadVertex1Z = z;

			float QuadVertex2X = QuadVertex1X + (quads_per_tile * units_per_vertex);
			float QuadVertex2Y = QuadVertex1Y;
			float QuadVertex2Z = QuadVertex1Z;

			float QuadVertex3X = QuadVertex2X;
			float QuadVertex3Y = QuadVertex1Y + (quads_per_tile * units_per_vertex);
			float QuadVertex3Z = QuadVertex1Z;

			float QuadVertex4X = QuadVertex1X;
			float QuadVertex4Y = QuadVertex3Y;
			float QuadVertex4Z = QuadVertex1Z;

			CheckGrowth(verts.size(), indices.size(), 4, 6);
			uint32 current_vert = (uint32)verts.size() + 3;
			verts.emplace_back(QuadVertex1X, QuadVertex1Y, QuadVertex1Z);
			verts.emplace_back(QuadVertex2X, QuadVertex2Y, QuadVertex2Z);
			verts.emplace_back(QuadVertex3X, QuadVertex3Y, QuadVertex3Z);
			verts.emplace_back(QuadVertex4X, QuadVertex4Y, QuadVertex4Z);

			indices.emplace_back(current_vert);
			indices.emplace_back(current_vert - 2);
			indices.emplace_back(current_vert - 1);

			indices.emplace_back(current_vert);
			indices.emplace_back(current_vert - 3);
			indices.emplace_back(current_vert - 2);
		}
		else {
			//read flags
			for (uint32 j = 0; j < ter_quad_count; ++j) {
				uint8 f;
				f = buf.Read<uint8>();

				flags[j] = f;
			}

			//read floats
			for (uint32 j = 0; j < ter_vert_count; ++j) {
				float f;
				f = buf.Read<float>();

				floats[j] = f;
			}

			int row_number = -1;
			std::map<std::tuple<float, float, float>, uint32> cur_verts;
			for (uint32 quad = 0; quad < ter_quad_count; ++quad) {
				if ((quad % quads_per_tile) == 0) {
					++row_number;
				}

				if (flags[quad] & 0x01)
					continue;

				float QuadVertex1X = x + (row_number * units_per_vertex);
				float QuadVertex1Y = y + (quad % quads_per_tile) * units_per_vertex;
				float QuadVertex1Z = floats[quad + row_number];

				float QuadVertex2X = QuadVertex1X + units_per_vertex;
				float QuadVertex2Y = QuadVertex1Y;
				float QuadVertex2Z = floats[quad + row_number + quads_per_tile + 1];

				float QuadVertex3X = QuadVertex1X + units_per_vertex;
				float QuadVertex3Y = QuadVertex1Y + units_per_vertex;
				float QuadVertex3Z = floats[quad + row_number + quads_per_tile + 2];

				float QuadVertex4X = QuadVertex1X;
				float QuadVertex4Y = QuadVertex1Y + units_per_vertex;
				float QuadVertex4Z = floats[quad + row_number + 1];

				CheckGrowth(verts.size(), indices.size(), 4, 6);
				uint32 i1, i2, i3, i4;
				std::tuple<float, float, float> t = std::make_tuple(QuadVertex1X, QuadVertex1Y, QuadVertex1Z);
				auto iter = cur_verts.find(t);
				if (iter != cur_verts.end()) {
					i1 = iter->second;
				}
				else {
					i1 = (uint32)verts.size();
					verts.emplace_back(QuadVertex1X, QuadVertex1Y, QuadVertex1Z);
					cur_verts[std::make_tuple(QuadVertex1X, QuadVertex1Y, QuadVertex1Z)] = i1;
				}

				t = std::make_tuple(QuadVertex2X, QuadVertex2Y, QuadVertex2Z);
				iter = cur_verts.find(t);
				if (iter != cur_verts.end()) {
					i2 = iter->second;
				}
				else {
					i2 = (uint32)verts.size();
					verts.emplace_back(QuadVertex2X, QuadVertex2Y, QuadVertex2Z);
					cur_verts[std::make_tuple(QuadVertex2X, QuadVertex2Y, QuadVertex2Z)] = i2;
				}

				t = std::make_tuple(QuadVertex3X, QuadVertex3Y, QuadVertex3Z);
				iter = cur_verts.find(t);
				if (iter != cur_verts.end()) {
					i3 = iter->second;
				}
				else {
					i3 = (uint32)verts.size();
					verts.emplace_back(QuadVertex3X, QuadVertex3Y, QuadVertex3Z);
					cur_verts[std::make_tuple(QuadVertex3X, QuadVertex3Y, QuadVertex3Z)] = i3;
				}

				t = std::make_tuple(QuadVertex4X, QuadVertex4Y, QuadVertex4Z);
				iter = cur_verts.find(t);
				if (iter != cur_verts.end()) {
					i4 = iter->second;
				}
				else {
					i4 = (uint32)verts.size();
					verts.emplace_back(QuadVertex4X, QuadVertex4Y, QuadVertex4Z);
					cur_verts[std::make_tuple(QuadVertex4X, QuadVertex4Y, QuadVertex4Z)] = i4;
				}

				indices.emplace_back(i4);
				indices.emplace_back(i2);
				indices.emplace_back(i3);

				indices.emplace_back(i4);
				indices.emplace_back(i1);
				indices.emplace_back(i2);
			}
		}
	}

	for (auto index : indices) if (index >= verts.size()) return false;
	for (const auto& vertex : verts) {
		if (!std::isfinite(vertex.x) || !std::isfinite(vertex.y) || !std::isfinite(vertex.z)) return false;
	}

	return true;
} catch (const std::exception&) {
	return false;
}


void MapGeometry::RotateVertex(glm::vec3 &v, float rx, float ry, float rz) {
	glm::vec3 nv = v;

	nv.y = (std::cos(rx) * v.y) - (std::sin(rx) * v.z);
	nv.z = (std::sin(rx) * v.y) + (std::cos(rx) * v.z);

	v = nv;

	nv.x = (std::cos(ry) * v.x) + (std::sin(ry) * v.z);
	nv.z = -(std::sin(ry) * v.x) + (std::cos(ry) * v.z);

	v = nv;

	nv.x = (std::cos(rz) * v.x) - (std::sin(rz) * v.y);
	nv.y = (std::sin(rz) * v.x) + (std::cos(rz) * v.y);

	v = nv;
}

void MapGeometry::ScaleVertex(glm::vec3 &v, float sx, float sy, float sz) {
	v.x = v.x * sx;
	v.y = v.y * sy;
	v.z = v.z * sz;
}

void MapGeometry::TranslateVertex(glm::vec3 &v, float tx, float ty, float tz) {
	v.x = v.x + tx;
	v.y = v.y + ty;
	v.z = v.z + tz;
}
