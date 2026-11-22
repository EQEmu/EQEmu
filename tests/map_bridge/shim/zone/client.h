#pragma once
#include <string>
#include <cmath>
#include <cstdint>
#include <fmt/format.h>
#include <glm/vec3.hpp>
extern int map_test_adjust;
#define RuleI(category, name) map_test_adjust
#undef LogInfo
#define LogInfo(...) ((void)0)
#undef LogError
#define LogError(...) ((void)0)
#undef safe_delete
#define safe_delete(p) do { delete (p); (p)=nullptr; } while(0)
struct PathManager {
 static PathManager* Instance() { static PathManager p; return &p; }
 std::string GetMapsPath() const { return "/no-maps"; }
};
