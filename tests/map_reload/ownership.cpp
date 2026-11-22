#include "zone/replace_collision_map.h"
#include <cassert>
#include <stdexcept>

struct MapProbe {
	int height;
	int &destroyed;
	~MapProbe() { ++destroyed; }
	int Query() const { return height; }
};

int main()
{
	int old_destroyed = 0, new_destroyed = 0, empty_destroyed = 0;
	MapProbe *map = nullptr;
	assert(!ReplaceCollisionMap(map, std::unique_ptr<MapProbe>()));
	assert(!map); // First-load failure stays absent.
	assert(ReplaceCollisionMap(map, std::unique_ptr<MapProbe>(new MapProbe{12, old_destroyed})));
	for (int failure = 0; failure < 2; ++failure) { // Missing and rejected candidates.
		assert(!ReplaceCollisionMap(map, std::unique_ptr<MapProbe>()));
		assert(map->Query() == 12 && old_destroyed == 0);
	}
	try {
		auto load = []() -> std::unique_ptr<MapProbe> { throw std::runtime_error("load"); };
		ReplaceCollisionMap(map, load());
		assert(false);
	} catch (const std::runtime_error &) {
		assert(map->Query() == 12 && old_destroyed == 0);
	}
	assert(ReplaceCollisionMap(map, std::unique_ptr<MapProbe>(new MapProbe{34, new_destroyed})));
	assert(map->Query() == 34 && old_destroyed == 1 && new_destroyed == 0);
	// A valid empty engine is still a candidate, irrespective of its query result.
	assert(ReplaceCollisionMap(map, std::unique_ptr<MapProbe>(new MapProbe{-99999, empty_destroyed})));
	assert(map->Query() == -99999 && new_destroyed == 1 && empty_destroyed == 0);
	delete map; // Zone cleanup owns the final candidate.
	assert(old_destroyed == 1 && new_destroyed == 1 && empty_destroyed == 1);
}
