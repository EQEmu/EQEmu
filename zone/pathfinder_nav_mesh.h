#pragma once

#include "pathfinder_interface.h"

#include "glm/vec3.hpp"
#include "recastnavigation/DetourNavMesh.h"
#include <memory>
#include <string>

class PathfinderNavmesh : public IPathfinder
{
public:
	PathfinderNavmesh(const std::string &path);
	virtual ~PathfinderNavmesh();

	virtual IPath FindRoute(const glm::vec3 &start, const glm::vec3 &end, bool &partial, bool &stuck, int flags = PathingNotDisabled);
	virtual IPath FindPath(const glm::vec3 &start, const glm::vec3 &end, bool &partial, bool &stuck, const PathfinderOptions& opts);
	virtual glm::vec3 GetRandomLocation(const glm::vec3 &start, int flags = PathingNotDisabled);
	virtual void DebugCommand(Client *c, const Seperator *sep);

private:
	void Clear();
	void Load(const std::string &path);
	void ShowPath(Client *c, const glm::vec3 &start, const glm::vec3 &end);
	dtStatus GetPolyHeightNoConnections(dtPolyRef ref, const float *pos, float *height) const;
	dtStatus GetPolyHeightOnPath(const dtPolyRef *path, const int path_len, const glm::vec3 &pos, float *h) const;

	struct Implementation;
	std::unique_ptr<Implementation> m_impl;
};
