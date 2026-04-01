/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/astar_search.hpp>
#include <string>
#include <memory>
#include <iostream>
#include <stdio.h>

#include "pathfinder_waypoint.h"
#include "zone.h"
#include "client.h"

extern Zone *zone;

#pragma pack(1)
struct NeighbourNode {
	int16 id;
	float distance;
	uint8 Teleport;
	int16 DoorID;
};

struct PathNode {
	uint16 id;
	glm::vec3 v;
	float bestz;
	NeighbourNode Neighbours[50];
};

struct PathFileHeader {
	uint32 version;
	uint32 PathNodeCount;
};
#pragma pack()

struct Edge
{
	float distance;
	bool teleport;
	int door_id;
};

struct Node
{
	int id;
	glm::vec3 v;
	float bestz;
	std::map<int, Edge> edges;
};

template <class Graph, class CostType, class NodeMap>
class distance_heuristic : public boost::astar_heuristic<Graph, CostType>
{
public:
	typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;

	distance_heuristic(NodeMap n, Vertex goal)
		: m_node(n), m_goal(goal) {}
	CostType operator()(Vertex u)
	{
		CostType dx = m_node[m_goal].v.x - m_node[u].v.x;
		CostType dy = m_node[m_goal].v.y - m_node[u].v.y;
		CostType dz = m_node[m_goal].v.z - m_node[u].v.z;
		return ::sqrt(dx * dx + dy * dy + dz * dz);
	}
private:
	NodeMap m_node;
	Vertex m_goal;
};

struct found_goal {};
template <class Vertex>
class astar_goal_visitor : public boost::default_astar_visitor
{
public:
	astar_goal_visitor(Vertex goal) : m_goal(goal) {}
	template <class Graph>
	void examine_vertex(Vertex u, Graph& g) {
		if (u == m_goal)
			throw found_goal();
	}
private:
	Vertex m_goal;
};

typedef boost::geometry::model::point<float, 3, boost::geometry::cs::cartesian> Point;
typedef std::pair<Point, unsigned int> RTreeValue;
typedef boost::adjacency_list<boost::listS, boost::vecS, boost::directedS, boost::no_property,
	boost::property<boost::edge_weight_t, float>> GraphType;
typedef boost::property_map<GraphType, boost::edge_weight_t>::type WeightMap;

struct PathfinderWaypoint::Implementation {
	bool PathFileValid;
	boost::geometry::index::rtree<RTreeValue, boost::geometry::index::quadratic<16>> Tree;
	GraphType Graph;
	std::vector<Node> Nodes;
	std::string FileName;
};

PathfinderWaypoint::PathfinderWaypoint(const std::string &path)
{
	m_impl.reset(new Implementation());
	m_impl->PathFileValid = false;
	m_impl->FileName = path;
	Load(path);
}

PathfinderWaypoint::~PathfinderWaypoint()
{
}

IPathfinder::IPath PathfinderWaypoint::FindRoute(const glm::vec3 &start, const glm::vec3 &end, bool &partial, bool &stuck, int flags)
{
	stuck = false;
	partial = false;
	std::vector<RTreeValue> result_start_n;
	m_impl->Tree.query(boost::geometry::index::nearest(Point(start.x, start.y, start.z), 1), std::back_inserter(result_start_n));
	if (result_start_n.size() == 0) {
		return IPath();
	}

	std::vector<RTreeValue> result_end_n;
	m_impl->Tree.query(boost::geometry::index::nearest(Point(end.x, end.y, end.z), 1), std::back_inserter(result_end_n));
	if (result_end_n.size() == 0) {
		return IPath();
	}

	auto &nearest_start = *result_start_n.begin();
	auto &nearest_end = *result_end_n.begin();

	if (nearest_start.second == nearest_end.second) {
		IPath Route;
		Route.push_back(start);
		Route.push_back(end);
		return Route;
	}

	std::vector<GraphType::vertex_descriptor> p(boost::num_vertices(m_impl->Graph));
	try {
		boost::astar_search(m_impl->Graph, nearest_start.second,
			distance_heuristic<GraphType, float, Node*>(&m_impl->Nodes[0], nearest_end.second),
			boost::predecessor_map(&p[0])
				.visitor(astar_goal_visitor<size_t>(nearest_end.second)));
	}
	catch (found_goal)
	{
		IPath Route;

		Route.push_front(end);
		for (size_t v = nearest_end.second;; v = p[v]) {
			if (p[v] == v) {
				Route.push_front(m_impl->Nodes[v].v);
				break;
			}
			else {
				auto &node = m_impl->Nodes[v];

				auto iter = node.edges.find((int)p[v + 1]);
				if (iter != node.edges.end()) {
					auto &edge = iter->second;
					if (edge.teleport) {
						Route.push_front(m_impl->Nodes[v].v);
						Route.push_front(true);
					}
					else {
						Route.push_front(m_impl->Nodes[v].v);
					}
				}
				else {
					Route.push_front(m_impl->Nodes[v].v);
				}
			}
		}

		Route.push_front(start);
		return Route;
	}

	return IPath();
}

glm::vec3 PathfinderWaypoint::GetRandomLocation(const glm::vec3 &start, int flags)
{
	if (m_impl->Nodes.size() > 0) {
		auto idx = zone->random.Int(0, (int)m_impl->Nodes.size() - 1);
		auto &node = m_impl->Nodes[idx];

		return node.v;
	}

	return glm::vec3();
}

void PathfinderWaypoint::DebugCommand(Client *c, const Seperator *sep)
{
	if(sep->arg[1][0] == '\0' || !strcasecmp(sep->arg[1], "help"))
	{
		c->Message(Chat::White, "Syntax: #path shownodes: Spawns a npc to represent every npc node.");
		c->Message(Chat::White, "#path show: Plots a path from the user to their target.");
		c->Message(Chat::White, "#path info node_id: Gives information about node info (requires shownode target).");
		return;
	}

	if(!strcasecmp(sep->arg[1], "shownodes"))
	{
		ShowNodes();
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

	if (!strcasecmp(sep->arg[1], "reload"))
	{
		Load(m_impl->FileName);
		return;
	}

	if (!strcasecmp(sep->arg[1], "info"))
	{
		NodeInfo(c);
		return;
	}
}

void PathfinderWaypoint::Load(const std::string &filename) {
	PathFileHeader Head;
	Head.PathNodeCount = 0;
	Head.version = 2;

	FILE *f = fopen(filename.c_str(), "rb");
	if (f) {
		char Magic[10];

		fread(&Magic, 9, 1, f);

		if (strncmp(Magic, "EQEMUPATH", 9))
		{
			LogError("Bad Magic String in .path file");
			fclose(f);
			return;
		}

		fread(&Head, sizeof(Head), 1, f);

		LogInfo("Path File Header: Version [{}], PathNodes [{}]",
			(long)Head.version, (long)Head.PathNodeCount);

		if (Head.version == 2)
		{
			LoadV2(f, Head);
			return;
		}
		else if (Head.version == 3) {
			LoadV3(f, Head);
			return;
		}
		else {
			LogError("Unsupported path file version");
			fclose(f);
			return;
		}
	}
}

void PathfinderWaypoint::LoadV2(FILE *f, const PathFileHeader &header)
{
	std::unique_ptr<PathNode[]> PathNodes(new PathNode[header.PathNodeCount]);

	fread(PathNodes.get(), sizeof(PathNode), header.PathNodeCount, f);

	int MaxNodeID = header.PathNodeCount - 1;

	m_impl->PathFileValid = true;

	m_impl->Nodes.reserve(header.PathNodeCount);
	for (uint32 i = 0; i < header.PathNodeCount; ++i)
	{
		auto &n = PathNodes[i];
		Node node;
		node.id = i;
		node.v = n.v;
		node.bestz = n.bestz;
		m_impl->Nodes.push_back(node);
	}

	auto weightmap = boost::get(boost::edge_weight, m_impl->Graph);
	for (uint32 i = 0; i < header.PathNodeCount; ++i) {
		for (uint32 j = 0; j < 50; ++j)
		{
			auto &node = m_impl->Nodes[i];
			if (PathNodes[i].Neighbours[j].id > MaxNodeID)
			{
				LogError("Path Node [{}], Neighbour [{}] ([{}]) out of range", i, j, PathNodes[i].Neighbours[j].id);
				m_impl->PathFileValid = false;
			}

			if (PathNodes[i].Neighbours[j].id > 0) {
				Edge edge;
				edge.distance = PathNodes[i].Neighbours[j].distance;
				edge.door_id = PathNodes[i].Neighbours[j].DoorID;
				edge.teleport = PathNodes[i].Neighbours[j].Teleport;

				node.edges[PathNodes[i].Neighbours[j].id] = edge;
			}
		}
	}

	BuildGraph();
	fclose(f);
}

void PathfinderWaypoint::LoadV3(FILE *f, const PathFileHeader &header)
{
	m_impl->Nodes.reserve(header.PathNodeCount);

	uint32 edge_count = 0;
	fread(&edge_count, sizeof(uint32), 1, f);

	for (uint32 i = 0; i < header.PathNodeCount; ++i)
	{
		uint32 id = 0;
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		float best_z = 0.0f;

		fread(&id, sizeof(uint32), 1, f);
		fread(&x, sizeof(float), 1, f);
		fread(&y, sizeof(float), 1, f);
		fread(&z, sizeof(float), 1, f);
		fread(&best_z, sizeof(float), 1, f);

		Node n;
		n.id = id;
		n.bestz = best_z;
		n.v.x = x;
		n.v.y = y;
		n.v.z = z;

		m_impl->Nodes.push_back(n);
	}

	for (uint32 j = 0; j < edge_count; ++j) {
		uint32 from = 0;
		uint32 to = 0;
		int8 teleport = 0;
		float distance = 0.0f;
		int32 door_id = 0;

		fread(&from, sizeof(uint32), 1, f);
		fread(&to, sizeof(uint32), 1, f);
		fread(&teleport, sizeof(int8), 1, f);
		fread(&distance, sizeof(float), 1, f);
		fread(&door_id, sizeof(int32), 1, f);

		Edge e;
		e.teleport = teleport > 0 ? true : false;
		e.distance = distance;
		e.door_id = door_id;

		auto &n = m_impl->Nodes[from];
		n.edges[to] = e;
	}

	m_impl->PathFileValid = true;

	BuildGraph();
	fclose(f);
}

void PathfinderWaypoint::ShowNodes()
{
	for (size_t i = 0; i < m_impl->Nodes.size(); ++i)
	{
		ShowNode(m_impl->Nodes[i]);
	}
}

void PathfinderWaypoint::ShowPath(Client *c, const glm::vec3 &start, const glm::vec3 &end)
{
	bool partial = false;
	bool stuck = false;
	auto path = FindRoute(start, end, partial, stuck);
	std::vector<FindPerson_Point> points;

	FindPerson_Point p;
	for (auto &node : path)
	{
		if (!node.teleport) {
			p.x = node.pos.x;
			p.y = node.pos.y;
			p.z = node.pos.z;

			points.push_back(p);
		}
	}

	c->SendPathPacket(points);
}

void PathfinderWaypoint::NodeInfo(Client *c)
{
	if (!c->GetTarget()) {
		return;
	}

	auto node = FindPathNodeByCoordinates(c->GetTarget()->GetX(), c->GetTarget()->GetY(), c->GetTarget()->GetZ());
	if (node == nullptr) {
		return;
	}

	c->Message(Chat::White, "Pathing node: %i at (%.2f, %.2f, %.2f) with bestz %.2f",
		node->id, node->v.x, node->v.y, node->v.z, node->bestz);

	for (auto &edge : node->edges) {
		c->Message(Chat::White, "id: %i, distance: %.2f, door id: %i, is teleport: %i",
			edge.first,
			edge.second.distance,
			edge.second.door_id,
			edge.second.teleport);
	}
}

Node *PathfinderWaypoint::FindPathNodeByCoordinates(float x, float y, float z)
{
	for (auto &node : m_impl->Nodes) {
		auto dist = Distance(glm::vec3(x, y, z), node.v);

		if (dist < 0.1) {
			return &node;
		}
	}

	return nullptr;
}

void PathfinderWaypoint::BuildGraph()
{
	m_impl->Graph = GraphType();
	m_impl->Tree = boost::geometry::index::rtree<RTreeValue, boost::geometry::index::quadratic<16>>();

	for (auto &node : m_impl->Nodes) {
		RTreeValue rtv;
		rtv.first = Point(node.v.x, node.v.y, node.v.z);
		rtv.second = node.id;
		m_impl->Tree.insert(rtv);
		boost::add_vertex(m_impl->Graph);
	}

	//Populate edges now that we've created all the nodes
	auto weightmap = boost::get(boost::edge_weight, m_impl->Graph);
	for (auto &node : m_impl->Nodes) {
		for (auto &edge : node.edges) {
			GraphType::edge_descriptor e;
			bool inserted;
			boost::tie(e, inserted) = boost::add_edge(node.id, edge.first, m_impl->Graph);
			weightmap[e] = edge.second.distance;
		}
	}
}

std::string DigitToWord(int i)
{
	std::string digit = std::to_string(i);
	std::string ret;
	for (size_t idx = 0; idx < digit.length(); ++idx) {
		if (!ret.empty()) {
			ret += "_";
		}

		switch (digit[idx]) {
		case '0':
			ret += "Zero";
			break;
		case '1':
			ret += "One";
			break;
		case '2':
			ret += "Two";
			break;
		case '3':
			ret += "Three";
			break;
		case '4':
			ret += "Four";
			break;
		case '5':
			ret += "Five";
			break;
		case '6':
			ret += "Six";
			break;
		case '7':
			ret += "Seven";
			break;
		case '8':
			ret += "Eight";
			break;
		case '9':
			ret += "Nine";
			break;
		default:
			break;
		}
	}

	return ret;
}

void PathfinderWaypoint::ShowNode(const Node &n) {
	auto npc_type = new NPCType;
	memset(npc_type, 0, sizeof(NPCType));

	sprintf(npc_type->name, "%s", DigitToWord(n.id).c_str());
	sprintf(npc_type->lastname, "%i", n.id);
	npc_type->current_hp = 4000000;
	npc_type->max_hp = 4000000;
	npc_type->race = 2254;
	npc_type->gender = Gender::Neuter;
	npc_type->class_ = 9;
	npc_type->deity = 1;
	npc_type->level = 75;
	npc_type->npc_id = 0;
	npc_type->loottable_id = 0;
	npc_type->texture = 1;
	npc_type->light = 0;
	npc_type->runspeed = 0;
	npc_type->d_melee_texture1 = 1;
	npc_type->d_melee_texture2 = 1;
	npc_type->merchanttype = 1;
	npc_type->bodytype = 1;
	npc_type->show_name = true;

	npc_type->STR = 150;
	npc_type->STA = 150;
	npc_type->DEX = 150;
	npc_type->AGI = 150;
	npc_type->INT = 150;
	npc_type->WIS = 150;
	npc_type->CHA = 150;

	npc_type->findable = 1;
	auto position = glm::vec4(n.v.x, n.v.y, n.v.z, 0.0f);
	auto npc = new NPC(npc_type, nullptr, position, GravityBehavior::Flying);
	npc->GiveNPCTypeData(npc_type);

	entity_list.AddNPC(npc, true, true);
}
