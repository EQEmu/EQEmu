#include "pathfinder_interface.h"

#include "common/seperator.h"
#include "zone/client.h"
#include "zone/pathfinder_nav_mesh.h"
#include "zone/pathfinder_null.h"

#include "fmt/format.h"
#include <sys/stat.h>

IPathfinder *IPathfinder::Load(const std::string &zone) {
	struct stat statbuffer;
	std::string navmesh_path = fmt::format("{}/maps/nav/{}.nav", PathManager::Instance()->GetServerPath(), zone);
	if (stat(navmesh_path.c_str(), &statbuffer) == 0) {
		return new PathfinderNavmesh(navmesh_path);
	}

	return new PathfinderNull();
}
