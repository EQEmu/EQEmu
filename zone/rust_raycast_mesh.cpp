#include "rust_raycast_mesh.h"
#ifdef EQEMU_RUST_MAP_RAYCASTER
#include "map-raycaster/src/lib.rs.h"
#include <algorithm>
#include <exception>
#include <memory>
#include <stdexcept>

namespace {
class RustRaycastMesh final : public RaycastMesh {
    rust::Box<eqemu_map::Engine> engine;
    eqemu_map::Bounds bounds;
    float minimum[3], maximum[3];
public:
    explicit RustRaycastMesh(rust::Box<eqemu_map::Engine> e)
        : engine(std::move(e)), bounds(engine->bounds()),
          minimum{bounds.min.x, bounds.min.y, bounds.min.z},
          maximum{bounds.max.x, bounds.max.y, bounds.max.z} {}
    bool raycast(const float* from, const float* to, float* location, float* normal, float* distance) override {
        // Rust errors become C++ exceptions. Never disguise an invalid query as a miss.
        auto h = engine->segment({from[0], from[1], from[2]}, {to[0], to[1], to[2]});
        if (!h.present) return false;
        if (location) { location[0]=h.position.x; location[1]=h.position.y; location[2]=h.position.z; }
        if (normal) { normal[0]=h.normal.x; normal[1]=h.normal.y; normal[2]=h.normal.z; }
        if (distance) *distance=h.distance;
        return true;
    }
    bool bruteForceRaycast(const float* a, const float* b, float* p, float* n, float* d) override {
        auto h=engine->brute_segment({a[0],a[1],a[2]}, {b[0],b[1],b[2]});
        if (!h.present) return false;
        if (p) { p[0]=h.position.x; p[1]=h.position.y; p[2]=h.position.z; }
        if (n) { n[0]=h.normal.x; n[1]=h.normal.y; n[2]=h.normal.z; }
        if (d) *d=h.distance;
        return true;
    }
    const float* getBoundMin() const override { return bounds.present ? minimum : nullptr; }
    const float* getBoundMax() const override { return bounds.present ? maximum : nullptr; }
    void release() override { delete this; }
};
}
RaycastMesh* createRustRaycastMesh(const std::vector<std::array<float,3>>& vertices,
    const std::vector<unsigned int>& indices, std::string& error) {
    try {
        if (vertices.size()>8000000 || indices.size()>12000000 || indices.size()%3) {
            throw std::runtime_error("invalid geometry counts");
        }
        std::vector<eqemu_map::Point> points;
        points.reserve(vertices.size());
        for (const auto& v: vertices) points.push_back({v[0],v[1],v[2]});
        auto engine=eqemu_map::create({points.data(),points.size()}, {indices.data(),indices.size()});
        return new RustRaycastMesh(std::move(engine));
    } catch (const std::exception& e) { error=e.what(); return nullptr; }
}
#endif
