#pragma once

#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

class OrientedBoundingBox
{
public:
	OrientedBoundingBox() = default;
	OrientedBoundingBox(const glm::vec3 &pos, const glm::vec3 &rot, const glm::vec3 &scale, const glm::vec3 &extents);
	~OrientedBoundingBox() = default;

	bool ContainsPoint(const glm::vec3 &p) const;
private:
	float min_x, max_x;
	float min_y, max_y;
	float min_z, max_z;
	glm::mat4 transformation;
	glm::mat4 inverted_transformation;
};
