#ifndef APATHING_H
#define APATHING_H

#include <mysql.h>
#include <gd.h>

#define DB_HOST "10.1.1.1"
#define DB_LOGIN "eqserver"
#define DB_PASSWORD "pw4eqserver"
#define DB_NAME "peq"

//for now, all of these were arbitrarily chosen 

//load up doors as spawn points, since they are also valid locations
extern bool INCLUDE_DOORS;

//this is the furthest a mob can be from a fear point and still expect 
//to find the node.
extern float FEAR_MAXIMUM_DISTANCE;

extern float ENDPOINT_CONNECT_MAX_DISTANCE;	//begin and end point must be this close
extern float MIN_FIX_Z; //minimum drop before correcting waypoint

#define ALMOST_COLINEAR_COS 0.99f //cosine of an angle to consider colinear... .99== 8 degrees

//if we miss the map on one Z-checking try, move the point by these
#define X_JITTER 3
#define Y_JITTER 3

//if two nodes are this close together, consider them the same
extern float CLOSE_ENOUGH;

//this causes us to check all of a node's edges for LOS from the new
//node when we are considering combining into that node
//this is not working as well as one might hope, leads to many invalids
extern bool COMBINE_CHECK_ALL_LOS;

//this is bigger than close enough, since we check LOS when combining these
//so that we prevent little juntions
extern float CLOSE_ENOUGH_COMBINE;

//a second link on a spawn must be this far away from the first.
extern float SPAWN_MIN_SECOND_DIST;

//uncomment to split a pathin with two waypoints which cannot see eachother into two
extern bool SPLIT_INVALID_PATHS;

//enabled linking of path endpoints as if they were spawn points.
extern bool LINK_PATH_ENDPOINTS;

//the maximum distance of the closest point to a spawn inorder to link it
extern float MAX_LINK_SPAWN_DIST;

//before we generate pathing info, we try to link all points within this range to eachother
extern float FINAL_LINK_POINTS_DIST;

//enables linking a spawn point to two nodes instead of just one.
extern bool SPAWN_LINK_TWICE;
extern bool SPAWN_LINK_THRICE; //link up to 3 times.. requires twice enabled

//a second link point must be further than this from the first for closest merge
extern float MERGE_MIN_SECOND_DIST;

//if an edge is longer than this, it will get cut into pieces interval long
extern float SPLIT_LINE_LENGTH;
extern float SPLIT_LINE_INTERVAL;

//an edge must cross this many lines before being considered for cross reduction
extern int CROSS_REDUCE_COUNT;
//an edge must be longer than this before we think it can cross anything
extern float CROSS_MIN_LENGTH;
//The intersect points of two edges must be within this range of Z to count
extern float CROSS_MAX_Z_DIFF;
//the max distance between two nodes to consider them the same when crossing
extern float CLOSE_ENOUGH_CROSS;

//check long paths on load for LOS
//#define LONG_PATH_CHECK_LOS

//minimum number of nodes for a graph to possible be a disjoint graph
#define MIN_DISJOINT_NODES 3	//5

//divide the image scale by this number in each direction
extern int IMAGE_SCALE;

//enable drawing of a color-by-reachability graph when coloring a graph
#define DRAW_ALL_COLORS 1

//enable drawing of the original non-reduced combined graph.
#define DRAW_PRETREE_GRAPH 1

#include "gpoint.h"
#include <vector>
#include <list>
using namespace std;

class PathNode : public GPoint {
public:
	PathNode() {
		init();
	}
	PathNode(const GPoint &them) : GPoint(them) {
		init();
	}
	PathNode(float ix, float iy, float iz) : GPoint(ix, iy, iz) {
		init();
	}
	void init() {
		color = 0;
//		color2 = 0;
		final_id = -1;
		longest_path = 0;
		valid = true;
		forced = false;
		disjoint = false;
	}
	
	int node_id;
	int final_id;
// inherited:
//	float x;
//	float y;
//	float z;
//	VertDesc vert;
	
	int color;
	int longest_path;
	
	char valid:1,
		 forced:1,
		 disjoint:1,
		 extra:5;
	
	float Dist2(const GPoint *o) const {
		float tmp;
		float sum;
		tmp = x - o->x;
		sum = tmp*tmp;
		tmp = y - o->y;
		sum += tmp*tmp;
		tmp = z - o->z;
		sum += tmp*tmp;
		return(sum);
	}
};

class PathEdge {
public:
	PathEdge( PathNode *_from, PathNode *_to) {
		from = _from;
		to = _to;
		normal_reach = -1;
		reverse_reach = -1;
		valid = true;
	}
	PathNode *from;
	PathNode *to;
	
	int normal_reach;
	int reverse_reach;
	bool valid;
//	EdgeDesc edge_id;
};

class PathGraph {
public:
	~PathGraph() {
		{
			list<PathNode *>::iterator cur,end;
			cur = nodes.begin();
			end = nodes.end();
			for(; cur != end; cur++) {
				delete *cur;
			}
		}
		{
			list<PathEdge *>::iterator cur,end;
			cur = edges.begin();
			end = edges.end();
			for(; cur != end; cur++) {
				delete *cur;
			}
		}
	}
	
	void add_edge(PathNode *b, PathNode *e) {
		edges.push_back(new PathEdge(b, e));
	}
	
	void add_edges(list<PathEdge *> &o) {
		list<PathEdge *>::iterator cur,end;
		cur = o.begin();
		end = o.end();
		for(; cur != end; cur++) {
			edges.push_back(*cur);
		}
	}
	
	list<PathNode *> nodes;
	list<PathEdge *> edges;
	
	//used for graph color accounts
	int curcolor;
	int ccount;
};


class ColorRecord {
public:
	int color;
	float height;
};




extern int load_split_paths;
extern int z_fixed_count;
extern int z_not_fixed_count;
extern int z_no_map_count;
extern float z_fixed_diffs;
extern float z_not_fixed_diffs;
extern int wp_reduce_count;
extern int trivial_merge_count;
extern int closest_merge_count;
extern int closest_merge2_count;
extern int link_spawn_count;
extern int link_spawn_invalid;
extern int link_spawn2_count;
extern int link_spawn3_count;
extern int link_spawn_nocount;
extern int combine_broke_los;
extern int combined_grid_points;
extern int removed_edges_los;
extern int removed_long_edges_los;
extern int broke_paths;
extern int cross_edge_count;
extern int cross_add_count;
extern int los_cache_misses;
extern int los_cache_hits;


#include <vector>
#include <map>
#include <string>
#include <algorithm>
using namespace std;

class QTNode;

//ye-olde prototypes
bool load_paths_from_db(MYSQL *m, Map *map, const char *zone, list<PathGraph*> &db_paths, list<PathNode*> &end_points);
bool load_spawns_from_db(MYSQL *m, const char *zone, list<PathNode*> &db_spawns);
bool load_doors_from_db(MYSQL *m, const char *zone, list<PathNode*> &db_spawns);
bool load_hints_from_db(MYSQL *m, const char *zone, list<PathNode*> &db_spawns);
bool load_settings_from_db(MYSQL *m, const char *zone);
void repair_a_high_waypoint(Map *map, PathNode *it);
void repair_high_waypoints(Map *map, list<PathGraph*> &db_paths, list<PathNode*> &db_spawns);
bool almost_colinear(PathNode *first, PathNode *second, PathNode *third);
void reduce_waypoints(list<PathGraph*> &db_paths);
void break_long_lines(list<PathGraph*> &db_paths);
//void build_big_graph(PathGraph *big, list<PathGraph*> &db_paths, list<PathNode*> &db_spawns);
void combine_trivial_grids(Map *map, list<PathGraph*> &db_paths);
void combine_closest_grids(Map *map, list<PathGraph*> &db_paths);
void link_spawns(Map *map, PathGraph *big, list<PathNode*> &db_spawns, float maxdist, map< pair<PathNode *, PathNode *>, bool > *edgelist);
void combine_grid_points(Map *map, PathGraph *big, float close_enough);
void draw_paths(Map *map, list<PathEdge *> &edges, list<PathEdge *> &edges2, const char *fname);
void draw_paths2(Map *map, list<PathEdge *> &edges1, list<PathEdge *> &edges2, list<PathEdge *> &edges3, list<PathNode *> &spawns, const char *fname);
void check_edge_los(Map *map, PathGraph *big);
void check_long_edge_los(Map *map, PathGraph *big);
bool CheckLOS(Map *map, PathNode *from, PathNode *to);
void cut_crossed_grids(PathGraph *big, map<PathEdge*, vector<GPoint> > &cross_list);
void rebuild_node_list(list<PathEdge *> &edges, list<PathNode *> &nodes, list<PathNode *> *excess_nodes = NULL);
QTNode *build_quadtree(Map *map, PathGraph *big);
bool write_path_file(QTNode *_root, PathGraph *big, const char *file, vector< vector<PathEdge*> > &path_finding);
bool load_eq_map(const char *zone, PathGraph *eqmap);
void write_eq_map(list<PathEdge *> &edges, const char *fname);
//void edge_stats(list<PathEdge*> &edges, const char *s);
void choose_biggest_graph(PathGraph *big, vector<int> &counts, vector<int> &first_node);
void find_path_info(Map *map, PathGraph *big, vector< vector<PathEdge *> > &path_finding);
void find_node_edges(PathGraph *big, std::map<PathNode*, vector<PathEdge*> > &node_edges);
void validate_edges(Map *map, PathGraph *big);

void DrawGradientLine(gdImagePtr im, GPoint *first, GPoint *second, vector<ColorRecord> &colors);
void allocateGradient(gdImagePtr im, float r1, float g1, float b1, float r2, float g2, float b2, 
	float min, float max, float divs, vector<ColorRecord> &colors);


#endif

