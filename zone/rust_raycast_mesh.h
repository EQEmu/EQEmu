#pragma once
#include "raycast_mesh.h"
#include <string>
#include <vector>
#include <array>

// Construction copies all geometry. Failure returns null with a diagnostic.
RaycastMesh *createRustRaycastMesh(const std::vector<std::array<float, 3>>& vertices,
    const std::vector<unsigned int>& indices, std::string& error);
