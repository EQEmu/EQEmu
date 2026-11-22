#pragma once

// Bounded diagnostics for the disposable NPC 2011 fixture. Read once at startup.
// Queries still require exclusive zone-thread engine ownership. Context and budget
// are local to that thread; names are string literals borrowed for each scope.
#include <glm/vec3.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace CollisionObservation {
inline thread_local const char *caller = nullptr;
inline thread_local unsigned entity = 0;
inline thread_local unsigned remaining = 4096;
inline bool Enabled(unsigned npc) {
	static const bool enabled = [] {
		const char *value = std::getenv("EQEMU_COLLISION_OBSERVE");
		return value && std::strcmp(value, "server-collision-npc-2011") == 0;
	}();
	return enabled && npc == 2011;
}
struct Scope {
	const char *previous = caller;
	unsigned previous_entity = entity;
	Scope(const char *name, unsigned npc, unsigned id) {
		caller = Enabled(npc) ? name : nullptr;
		entity = id;
	}
	Scope(const Scope&) = delete;
	Scope& operator=(const Scope&) = delete;
	~Scope() { caller = previous; entity = previous_entity; }
};
inline void Query(const char *kind, const glm::vec3 &from, const glm::vec3 &to, float result) {
	if (!caller || !remaining) { return; }
	--remaining;
	// Preserve the accepted Rust receipt label for endpoint comparisons.
	const char *backend = "ON";
	std::printf("SERVER_COLLISION_NATIVE caller=%s entity=%u backend=%s kind=%s from=%.9g,%.9g,%.9g to=%.9g,%.9g,%.9g result=%.9g remaining=%u\n",
		caller, entity, backend, kind, from.x, from.y, from.z, to.x, to.y, to.z, result, remaining);
}
}
