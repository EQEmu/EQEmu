#include "pathfinder_nav_mesh.h"

#include "common/compression.h"
#include "zone/client.h"
#include "zone/water_map.h"
#include "zone/zone.h"

#include "recastnavigation/DetourCommon.h"
#include "recastnavigation/DetourNavMeshQuery.h"
#include <cstdio>
#include <memory>
#include <vector>

extern Zone *zone;

struct PathfinderNavmesh::Implementation
{
	dtNavMesh *nav_mesh;
	dtNavMeshQuery *query;
};

PathfinderNavmesh::PathfinderNavmesh(const std::string &path)
{
	m_impl = std::make_unique<Implementation>();
	m_impl->nav_mesh = nullptr;
	m_impl->query = nullptr;
	Load(path);
}

PathfinderNavmesh::~PathfinderNavmesh()
{
	Clear();
}

IPathfinder::IPath PathfinderNavmesh::FindRoute(const glm::vec3 &start, const glm::vec3 &end, bool &partial, bool &stuck, int flags)
{
	partial = false;

	if (!m_impl->nav_mesh) {
		return IPath();
	}

	if (!m_impl->query) {
		m_impl->query = dtAllocNavMeshQuery();
	}

	m_impl->query->init(m_impl->nav_mesh, RuleI(Pathing, MaxNavmeshNodes));
	glm::vec3 current_location(start.x, start.z, start.y);
	glm::vec3 dest_location(end.x, end.z, end.y);

	dtQueryFilter filter;
	filter.setIncludeFlags(flags);
	filter.setAreaCost(0, 1.0f); //Normal
	filter.setAreaCost(1, 3.0f); //Water
	filter.setAreaCost(2, 5.0f); //Lava
	filter.setAreaCost(4, 1.0f); //PvP
	filter.setAreaCost(5, 2.0f); //Slime
	filter.setAreaCost(6, 2.0f); //Ice
	filter.setAreaCost(7, 4.0f); //V Water (Frigid Water)
	filter.setAreaCost(8, 1.0f); //General Area
	filter.setAreaCost(9, 0.1f); //Portal
	filter.setAreaCost(10, 0.1f); //Prefer

	dtPolyRef start_ref;
	dtPolyRef end_ref;
	glm::vec3 ext(5.0f, 100.0f, 5.0f);

	m_impl->query->findNearestPoly(&current_location[0], &ext[0], &filter, &start_ref, 0);
	m_impl->query->findNearestPoly(&dest_location[0], &ext[0], &filter, &end_ref, 0);

	if (!start_ref || !end_ref) {
		return IPath();
	}

	int npoly = 0;
	dtPolyRef path[1024] = { 0 };
	auto status = m_impl->query->findPath(start_ref, end_ref, &current_location[0], &dest_location[0], &filter, path, &npoly, 1024);

	if (npoly) {
		glm::vec3 epos = dest_location;
		if (path[npoly - 1] != end_ref) {
			m_impl->query->closestPointOnPoly(path[npoly - 1], &dest_location[0], &epos[0], 0);
			partial = true;

			auto dist = DistanceSquared(epos, current_location);
			if (dist < 10000.0f) {
				stuck = true;
			}
		}

		float straight_path[2048 * 3];
		unsigned char straight_path_flags[2048];

		int n_straight_polys;
		dtPolyRef straight_path_polys[2048];

		status = m_impl->query->findStraightPath(&current_location[0], &epos[0], path, npoly,
			straight_path, straight_path_flags,
			straight_path_polys, &n_straight_polys, 2048, DT_STRAIGHTPATH_AREA_CROSSINGS);

		if (dtStatusFailed(status)) {
			return IPath();
		}

		if (n_straight_polys) {
			IPath Route;
			for (int i = 0; i < n_straight_polys; ++i)
			{
				glm::vec3 node;
				node.x = straight_path[i * 3];
				node.z = straight_path[i * 3 + 1];
				node.y = straight_path[i * 3 + 2];

				Route.push_back(node);

				unsigned short flag = 0;
				if (dtStatusSucceed(m_impl->nav_mesh->getPolyFlags(straight_path_polys[i], &flag))) {
					if (flag & 512) {
						Route.push_back(true);
					}
				}
			}

			return Route;
		}
	}

	IPath Route;
	Route.push_back(end);
	return Route;
}

IPathfinder::IPath PathfinderNavmesh::FindPath(const glm::vec3 &start, const glm::vec3 &end, bool &partial, bool &stuck, const PathfinderOptions &opts)
{
	partial = false;

	if (!m_impl->nav_mesh) {
		return IPath();
	}

	if (!m_impl->query) {
		m_impl->query = dtAllocNavMeshQuery();
	}

	m_impl->query->init(m_impl->nav_mesh, RuleI(Pathing, MaxNavmeshNodes));
	glm::vec3 current_location(start.x, start.z, start.y);
	glm::vec3 dest_location(end.x, end.z, end.y);

	dtQueryFilter filter;
	filter.setIncludeFlags(opts.flags);
	filter.setAreaCost(0, opts.flag_cost[0]); //Normal
	filter.setAreaCost(1, opts.flag_cost[1]); //Water
	filter.setAreaCost(2, opts.flag_cost[2]); //Lava
	filter.setAreaCost(4, opts.flag_cost[3]); //PvP
	filter.setAreaCost(5, opts.flag_cost[4]); //Slime
	filter.setAreaCost(6, opts.flag_cost[5]); //Ice
	filter.setAreaCost(7, opts.flag_cost[6]); //V Water (Frigid Water)
	filter.setAreaCost(8, opts.flag_cost[7]); //General Area
	filter.setAreaCost(9, opts.flag_cost[8]); //Portal
	filter.setAreaCost(10, opts.flag_cost[9]); //Prefer

	static const int max_polys = 256;
	dtPolyRef start_ref;
	dtPolyRef end_ref;
	glm::vec3 ext(10.0f, 200.0f, 10.0f);

	m_impl->query->findNearestPoly(&current_location[0], &ext[0], &filter, &start_ref, 0);
	m_impl->query->findNearestPoly(&dest_location[0], &ext[0], &filter, &end_ref, 0);

	if (!start_ref || !end_ref) {
		return IPath();
	}

	int npoly = 0;
	dtPolyRef path[max_polys] = { 0 };
	auto status = m_impl->query->findPath(start_ref, end_ref, &current_location[0], &dest_location[0], &filter, path, &npoly, max_polys);

	if (npoly) {
		glm::vec3 epos = dest_location;
		if (path[npoly - 1] != end_ref) {
			m_impl->query->closestPointOnPoly(path[npoly - 1], &dest_location[0], &epos[0], 0);
			partial = true;

			auto dist = DistanceSquared(epos, current_location);
			if (dist < 10000.0f) {
				stuck = true;
			}
		}

		int n_straight_polys;
		glm::vec3 straight_path[max_polys];
		unsigned char straight_path_flags[max_polys];
		dtPolyRef straight_path_polys[max_polys];

		auto status = m_impl->query->findStraightPath(&current_location[0], &epos[0], path, npoly,
			(float*)&straight_path[0], straight_path_flags,
			straight_path_polys, &n_straight_polys, 2048, DT_STRAIGHTPATH_AREA_CROSSINGS | DT_STRAIGHTPATH_ALL_CROSSINGS);

		if (dtStatusFailed(status)) {
			return IPath();
		}

		if (n_straight_polys) {
			if (opts.smooth_path) {
				IPath Route;

				//Add the first point
				{
					auto &flag = straight_path_flags[0];
					if (flag & DT_STRAIGHTPATH_OFFMESH_CONNECTION) {
						auto &p = straight_path[0];

						Route.push_back(glm::vec3(p.x, p.z, p.y));
					}
					else {
						auto &p = straight_path[0];

						float h = 0.0f;
						if (dtStatusSucceed(GetPolyHeightOnPath(path, npoly, p, &h))) {
							p.y = h + opts.offset;
						}

						Route.push_back(glm::vec3(p.x, p.z, p.y));
					}
				}

				for (int i = 0; i < n_straight_polys - 1; ++i)
				{
					auto &flag = straight_path_flags[i];

					if (flag & DT_STRAIGHTPATH_OFFMESH_CONNECTION) {
						auto &poly = straight_path_polys[i];

						auto &p2 = straight_path[i + 1];
						glm::vec3 node(p2.x, p2.z, p2.y);
						Route.push_back(node);

						unsigned short pflag = 0;
						if (dtStatusSucceed(m_impl->nav_mesh->getPolyFlags(straight_path_polys[i], &pflag))) {
							if (pflag & 512) {
								Route.push_back(true);
							}
						}
					}
					else {
						auto &p1 = straight_path[i];
						auto &p2 = straight_path[i + 1];
						auto dist = glm::distance(p1, p2);
						auto dir = glm::normalize(p2 - p1);
						float total = 0.0f;
						glm::vec3 previous_pt = p1;

						while (total < dist) {
							glm::vec3 current_pt;
							float dist_to_move = opts.step_size;
							float ff = opts.step_size / 2.0f;

							if (total + dist_to_move + ff >= dist) {
								current_pt = p2;
								total = dist;
							}
							else {
								total += dist_to_move;
								current_pt = p1 + dir * total;
							}

							float h = 0.0f;
							if (dtStatusSucceed(GetPolyHeightOnPath(path, npoly, current_pt, &h))) {
								current_pt.y = h + opts.offset;
							}

							Route.push_back(glm::vec3(current_pt.x, current_pt.z, current_pt.y));
							previous_pt = current_pt;
						}
					}
				}

				return Route;
			}
			else {
				IPath Route;
				for (int i = 0; i < n_straight_polys; ++i)
				{
					auto &current = straight_path[i];
					glm::vec3 node(current.x, current.z, current.y);
					Route.push_back(node);

					unsigned short flag = 0;
					if (dtStatusSucceed(m_impl->nav_mesh->getPolyFlags(straight_path_polys[i], &flag))) {
						if (flag & 512) {
							Route.push_back(true);
						}
					}
				}

				return Route;
			}
		}
	}

	return IPath();
}

glm::vec3 PathfinderNavmesh::GetRandomLocation(const glm::vec3 &start, int flags)
{
	if (start.x == 0.0f && start.y == 0.0)
		return glm::vec3(0.f);

	if (!m_impl->nav_mesh) {
		return glm::vec3(0.f);
	}

	if (!m_impl->query) {
		m_impl->query = dtAllocNavMeshQuery();
		m_impl->query->init(m_impl->nav_mesh, RuleI(Pathing, MaxNavmeshNodes));
	}

	dtQueryFilter filter;
	filter.setIncludeFlags(flags);
	filter.setAreaCost(0, 1.0f); //Normal
	filter.setAreaCost(1, 3.0f); //Water
	filter.setAreaCost(2, 5.0f); //Lava
	filter.setAreaCost(4, 1.0f); //PvP
	filter.setAreaCost(5, 2.0f); //Slime
	filter.setAreaCost(6, 2.0f); //Ice
	filter.setAreaCost(7, 4.0f); //V Water (Frigid Water)
	filter.setAreaCost(8, 1.0f); //General Area
	filter.setAreaCost(9, 0.1f); //Portal
	filter.setAreaCost(10, 0.1f); //Prefer

	dtPolyRef randomRef;
	float point[3];

	dtPolyRef start_ref;
	glm::vec3 current_location(start.x, start.z, start.y);
	glm::vec3 ext(5.0f, 100.0f, 5.0f);

	m_impl->query->findNearestPoly(&current_location[0], &ext[0], &filter, &start_ref, 0);

	if (!start_ref)
	{
		return glm::vec3(0.f);
	}

	if (dtStatusSucceed(m_impl->query->findRandomPointAroundCircle(start_ref, &current_location[0], 100.f, &filter, []() { return (float)zone->random.Real(0.0, 1.0); }, &randomRef, point)))
	{
		return glm::vec3(point[0], point[2], point[1]);
	}

	return glm::vec3(0.f);
}

void PathfinderNavmesh::DebugCommand(Client *c, const Seperator *sep)
{
	if (sep->arg[1][0] == '\0' || !strcasecmp(sep->arg[1], "help"))
	{
		c->Message(Chat::White, "#path show: Plots a path from the user to their target.");
		return;
	}

	if (!strcasecmp(sep->arg[1], "show"))
	{
		if (c->GetTarget() != nullptr) {
			auto target = c->GetTarget();
			glm::vec3 start(c->GetX(), c->GetY(), c->GetZ());
			glm::vec3 end(target->GetX(), target->GetY(), target->GetZ());

			ShowPath(c, start, end);
		}

		return;
	}
}

void PathfinderNavmesh::Clear()
{
	if (m_impl->nav_mesh) {
		dtFreeNavMesh(m_impl->nav_mesh);
	}

	if (m_impl->query) {
		dtFreeNavMeshQuery(m_impl->query);
	}
}

void PathfinderNavmesh::Load(const std::string &path)
{
	Clear();

	FILE *f = fopen(path.c_str(), "rb");
	if (f) {
		char magic[9] = { 0 };
		if (fread(magic, 9, 1, f) != 1) {
			fclose(f);
			return;
		}

		if (strncmp(magic, "EQNAVMESH", 9) != 0)
		{
			fclose(f);
			return;
		}

		uint32_t version = 0;
		if (fread(&version, sizeof(uint32_t), 1, f) != 1) {
			fclose(f);
			return;
		}

		if (version != 2) {
			fclose(f);
			return;
		}

		uint32_t data_size;
		if (fread(&data_size, sizeof(data_size), 1, f) != 1) {
			fclose(f);
			return;
		}

		uint32_t buffer_size;
		if (fread(&buffer_size, sizeof(buffer_size), 1, f) != 1) {
			fclose(f);
			return;
		}

		std::vector<char> data;
		data.resize(data_size);
		if (fread(&data[0], data_size, 1, f) != 1) {
			fclose(f);
			return;
		}

		std::vector<char> buffer;
		buffer.resize(buffer_size);
		uint32_t v = EQ::InflateData(&data[0], data_size, &buffer[0], buffer_size);
		fclose(f);

		char *buf = &buffer[0];
		m_impl->nav_mesh = dtAllocNavMesh();

		uint32_t number_of_tiles = *(uint32_t*)buf;
		buf += sizeof(uint32_t);

		dtNavMeshParams params = *(dtNavMeshParams*)buf;
		buf += sizeof(dtNavMeshParams);

		dtStatus status = m_impl->nav_mesh->init(&params);
		if (dtStatusFailed(status))
		{
			dtFreeNavMesh(m_impl->nav_mesh);
			m_impl->nav_mesh = nullptr;
			return;
		}

		for (unsigned int i = 0; i < number_of_tiles; ++i)
		{
			uint32_t tile_ref = *(uint32_t*)buf;
			buf += sizeof(uint32_t);

			int32_t data_size = *(uint32_t*)buf;
			buf += sizeof(uint32_t);

			if (!tile_ref || !data_size) {
				dtFreeNavMesh(m_impl->nav_mesh);
				m_impl->nav_mesh = nullptr;
				return;
			}

			unsigned char* data = (unsigned char*)dtAlloc(data_size, DT_ALLOC_PERM);
			memcpy(data, buf, data_size);
			buf += data_size;

			m_impl->nav_mesh->addTile(data, data_size, DT_TILE_FREE_DATA, tile_ref, 0);
		}

		LogInfo("Loaded Navmesh V[{}] file [{}]", version, path.c_str());
	}
}

void PathfinderNavmesh::ShowPath(Client * c, const glm::vec3 &start, const glm::vec3 &end)
{
	auto &list = entity_list.GetNPCList();

	for (auto &iter : list) {
		auto npc = iter.second;
		auto name = npc->GetName();

		if (strstr(name, "PathNode") != nullptr) {
			npc->Depop();
		}
	}

	PathfinderOptions opts;
	opts.smooth_path = true;
	opts.step_size = RuleR(Pathing, NavmeshStepSize);
	bool partial = false;
	bool stuck = false;
	auto path = FindPath(start, end, partial, stuck, opts);

	for (auto &node : path) {
		if (!node.teleport) {
			NPC::SpawnNPC("PathNode 2253 1 0 1 2 1", glm::vec4(node.pos, 1.0));
		}
	}
}

dtStatus PathfinderNavmesh::GetPolyHeightNoConnections(dtPolyRef ref, const float *pos, float *height) const
{
	auto *m_nav = m_impl->nav_mesh;

	if (!m_nav) {
		return DT_FAILURE;
	}

	const dtMeshTile* tile = 0;
	const dtPoly* poly = 0;
	if (dtStatusFailed(m_nav->getTileAndPolyByRef(ref, &tile, &poly))) {
		return DT_FAILURE | DT_INVALID_PARAM;
	}

	if (poly->getType() != DT_POLYTYPE_OFFMESH_CONNECTION) {
		const unsigned int ip = (unsigned int)(poly - tile->polys);
		const dtPolyDetail* pd = &tile->detailMeshes[ip];
		for (int j = 0; j < pd->triCount; ++j)
		{
			const unsigned char* t = &tile->detailTris[(pd->triBase + j) * 4];
			const float* v[3];
			for (int k = 0; k < 3; ++k)
			{
				if (t[k] < poly->vertCount)
					v[k] = &tile->verts[poly->verts[t[k]] * 3];
				else
					v[k] = &tile->detailVerts[(pd->vertBase + (t[k] - poly->vertCount)) * 3];
			}
			float h;
			if (dtClosestHeightPointTriangle(pos, v[0], v[1], v[2], h))
			{
				if (height)
					*height = h;
				return DT_SUCCESS;
			}
		}
	}

	return DT_FAILURE | DT_INVALID_PARAM;
}

dtStatus PathfinderNavmesh::GetPolyHeightOnPath(const dtPolyRef *path, const int path_len, const glm::vec3 &pos, float *h) const
{
	if (!path || !path_len) {
		return DT_FAILURE;
	}

	for (int i = 0; i < path_len; ++i) {
		dtPolyRef ref = path[i];

		if (dtStatusSucceed(GetPolyHeightNoConnections(ref, &pos[0], h))) {
			return DT_SUCCESS;
		}
	}

	return DT_FAILURE;
}
