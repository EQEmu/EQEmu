
#include "oriented_bounding_box.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

glm::mat4 CreateRotateMatrix(float rx, float ry, float rz) {
	glm::mat4 rot_x(1.0f);
	rot_x[1][1] = cos(rx);
	rot_x[2][1] = -sin(rx);
	rot_x[1][2] = sin(rx);
	rot_x[2][2] = cos(rx);

	glm::mat4 rot_y(1.0f);
	rot_y[0][0] = cos(ry);
	rot_y[2][0] = sin(ry);
	rot_y[0][2] = -sin(ry);
	rot_y[2][2] = cos(ry);

	glm::mat4 rot_z(1.0f);
	rot_z[0][0] = cos(rz);
	rot_z[1][0] = -sin(rz);
	rot_z[0][1] = sin(rz);
	rot_z[1][1] = cos(rz);

	return rot_z * rot_y * rot_x;
}

glm::mat4 CreateTranslateMatrix(float tx, float ty, float tz) {
	glm::mat4 trans(1.0f);
	trans[3][0] = tx;
	trans[3][1] = ty;
	trans[3][2] = tz;

	return trans;
}

glm::mat4 CreateScaleMatrix(float sx, float sy, float sz) {
	glm::mat4 scale(1.0f);
	scale[0][0] = sx;
	scale[1][1] = sy;
	scale[2][2] = sz;
	return scale;
}

OrientedBoundingBox::OrientedBoundingBox(const glm::vec3 &pos, const glm::vec3 &rot, const glm::vec3 &scale, const glm::vec3 &extents) {
	min_x = -extents.x;
	max_x = extents.x;

	if (min_x > max_x)
	{
		float t = min_x;
		min_x = max_x;
		max_x = t;
	}

	min_y = -extents.y;
	max_y = extents.y;
	if (min_y > max_y)
	{
		float t = min_y;
		min_y = max_y;
		max_y = t;
	}

	min_z = -extents.z;
	max_z = extents.z;
	if (min_z > max_z)
	{
		float t = min_z;
		min_z = max_z;
		max_z = t;
	}

	//rotate
	transformation = CreateRotateMatrix(rot.x * 3.14159f / 180.0f, rot.y * 3.14159f / 180.0f, rot.z * 3.14159f / 180.0f);
	
	//scale
	transformation = CreateScaleMatrix(scale.x, scale.y, scale.z) * transformation;

	//translate
	transformation = CreateTranslateMatrix(pos.x, pos.y, pos.z) * transformation;
	inverted_transformation = glm::inverse(transformation);
}

bool OrientedBoundingBox::ContainsPoint(const glm::vec3 &p) const {
	glm::vec4 pt(p.x, p.y, p.z, 1);
	glm::vec4 box_space_p = inverted_transformation * pt;

	if (box_space_p.x >= min_x && box_space_p.x <= max_x &&
		box_space_p.y >= min_y && box_space_p.y <= max_y &&
		box_space_p.z >= min_z && box_space_p.z <= max_z) {
		return true;
	}
	
	return false;
}
