#pragma once
#include "common/types.h"
#include <glm/vec3.hpp>
#include <cstdio>
#include <vector>

// Production map decoding and expansion. Call with empty owned arrays.
// FILE begins immediately after the version word. No MMF or raycaster state.
class MapGeometry {
public:
 static bool LoadV1(FILE *, std::vector<glm::vec3> &, std::vector<uint32> &);
 static bool LoadV2(FILE *, std::vector<glm::vec3> &, std::vector<uint32> &);
private:
 static void RotateVertex(glm::vec3 &, float, float, float);
 static void ScaleVertex(glm::vec3 &, float, float, float);
 static void TranslateVertex(glm::vec3 &, float, float, float);
};
