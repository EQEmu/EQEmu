
//put all the non-boost crap which changes a lot in here
//to reduce compile times, cause boost is a bitch
#include "../common/types.h"
#include "../zone/map.h"
#include "apathing.h"
#include "quadtree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gd.h>
#include "gpoint.h"

#include <vector>
#include <map>
#include <string>
#include <algorithm>
using namespace std;



//stats variables... quite lazy
int load_split_paths = 0;
int z_fixed_count = 0;
int z_not_fixed_count = 0;
int z_no_map_count = 0;
float z_fixed_diffs = 0;
float z_not_fixed_diffs = 0;
int wp_reduce_count = 0;
int trivial_merge_count = 0;
int closest_merge_count = 0;
int closest_merge2_count = 0;
int link_spawn_count = 0;
int link_spawn_invalid = 0;
int link_spawn2_count = 0;
int link_spawn3_count = 0;
int link_spawn_nocount = 0;
int combine_broke_los = 0;
int combined_grid_points = 0;
int removed_edges_los = 0;
int removed_long_edges_los = 0;
int broke_paths = 0;
int cross_edge_count = 0;
int cross_add_count = 0;
int los_cache_misses = 0;
int los_cache_hits = 0;

/*
//ye-olde prototypes
void repair_a_high_waypoint(Map *map, PathNode *it);
void repair_high_waypoints(Map *map, list<PathGraph*> &db_paths, list<PathNode*> &db_spawns);
bool almost_colinear(PathNode *first, PathNode *second, PathNode *third);
void reduce_waypoints(list<PathGraph*> &db_paths);
//void build_big_graph(PathGraph *big, list<PathGraph*> &db_paths, list<PathNode*> &db_spawns);
void combine_trivial_grids(Map *map, list<PathGraph*> &db_paths);
void combine_closest_grids(Map *map, list<PathGraph*> &db_paths);
void link_spawns(Map *map, PathGraph *big, list<PathNode*> &db_spawns);
void combine_grid_points(Map *map, PathGraph *big, float close_enough);
void draw_paths(Map *map, list<PathEdge *> &edges, const char *fname);
void draw_paths2(Map *map, list<PathEdge *> &edges1, list<PathEdge *> &edges2, const char *fname);
void check_edge_los(Map *map, PathGraph *big);
void check_long_edge_los(Map *map, PathGraph *big);
bool CheckLOS(Map *map, PathNode *from, PathNode *to);
void rebuild_node_list(list<PathEdge *> &edges, list<PathNode *> &nodes, list<PathNode *> *excess_nodes = NULL);
//void edge_stats(list<PathEdge *> &edges, const char *s);

void DrawGradientLine(gdImagePtr im, GPoint *first, GPoint *second, vector<ColorRecord> &colors);
void allocateGradient(gdImagePtr im, float r1, float g1, float b1, float r2, float g2, float b2, 
	float min, float max, float divs, vector<ColorRecord> &colors);
*/

void repair_a_high_waypoint(Map *map, PathNode *it) {
	VERTEX pt, res;
	pt.x = it->x;
	pt.y = it->y;
	pt.z = it->z + 10;
	
	float newz = map->FindBestZ(map->GetRoot(), pt, &res);
	
	if(newz == BEST_Z_INVALID) {
		pt.x += X_JITTER;
		pt.y += Y_JITTER;
		newz = map->FindBestZ(map->GetRoot(), pt, &res);
	}
	
	if(newz != BEST_Z_INVALID) {
		newz += 6;	//just some arbitrary height
		float diff = it->z - newz;
		if(diff > MIN_FIX_Z) {
			z_fixed_count++;
			z_fixed_diffs += diff;
			it->z = newz;
		} else {
			z_not_fixed_count++;
			z_not_fixed_diffs += diff;
		}
	} else {
		z_no_map_count++;
	}
}

void repair_high_waypoints(Map *map, list<PathGraph*> &db_paths, list<PathNode*> &db_spawns) {
/* - do not wanna drop people below docks though, so set min height first
 - for each waypoint/spawn point
   - if the best Z below the point (+6 on Z) is more than DROP_HEIGHT
   - then lower the waypoint to bestZ + 10ish*/
	list<PathGraph*>::iterator cur, end;
	cur = db_paths.begin();
	end = db_paths.end();
	for(; cur != end; cur++) {
		PathGraph *g = *cur;
		list<PathNode*>::iterator cur2,end2;
		cur2 = g->nodes.begin();
		end2 = g->nodes.end();
		for(; cur2 != end2; cur2++) {
			repair_a_high_waypoint(map, *cur2);
		}
	}
	
	list<PathNode*>::iterator cur3,end3;
	cur3 = db_spawns.begin();
	end3 = db_spawns.end();
	for(; cur3 != end3; cur3++) {
		repair_a_high_waypoint(map, *cur3);
	}
}

bool almost_colinear(PathNode *first, PathNode *second, PathNode *third) {
	//basically a dot product and a compare.
	GVector v1(*first, *second);
	GVector v2(*second, *third);
	
	//the - operator is apparently not working or something
	v1.x = second->x - first->x;
	v1.y = second->y - first->y;
	v1.z = second->z - first->z;
	
	v2.x = third->x - second->x;
	v2.y = third->y - second->y;
	v2.z = third->z - second->z;
	
	//just another option to consider:
	//v1.z = 0;
	//v2.z = 0;
	
	
/*	printf("(%.3f, %.3f, %.3f) (%.3f, %.3f, %.3f) (%.3f, %.3f, %.3f)\n",
		first->x, first->y, first->z,
		second->x, second->y, second->z,
		third->x, third->y, third->z);
	printf("v1(%.3f, %.3f) v2(%.3f, %.3f)\n", v1.x, v1.y, v2.x, v2.y);
//	printf("BB(%.3f, %.3f) BB(%.3f, %.3f)\n", , ,, );
*/	
	
	v1.normalize();
	v2.normalize();
	
	float cos_angle = v1.dot3(v2);
	
	return(cos_angle > ALMOST_COLINEAR_COS);
}

void reduce_waypoints(list<PathGraph*> &db_paths) {
/*
 - For each waypoint W where 0 <= W < N-2
   - determine unit vector from W to W+1
   - determine unit vector from W+1 to W+2
   - dot product the two vectors
   - if the dot product is very close to 1.0, eliminate W+1, and connect W to W+2
   - Run code from node W again, dont go to W+2 next
*/
	list<PathGraph*>::iterator cur, end;
	cur = db_paths.begin();
	end = db_paths.end();
	for(; cur != end; cur++) {
		PathGraph *g = *cur;
		if(g->nodes.size() < 3)
			continue;
		
		list<PathNode*>::iterator cur2,end2,trail;
		PathNode *first,*second,*last;
RESTART_WP_REDUCE:
		cur2 = g->nodes.begin();
		end2 = g->nodes.end();
		
		first = *cur2;
		cur2++;
		second = *cur2;
		trail = cur2;
		cur2++;
		
		int pos = 2;
		for(; cur2 != end2; cur2++) {

			last = *cur2;
			if(almost_colinear(first, second, last)) {
				//we are removing the second one
				wp_reduce_count++;
				
				//	*trail, second, (*trail)->x, (*trail)->y, (*trail)->z);
				//trail = cur2;
				
				//need to do something with its old edges...
				//we can assume at this point that there is only
				//one edge starting from us and one edge ending at us
				list<PathEdge*>::iterator cure, ende, rme = g->edges.end();
				cure = g->edges.begin();
				ende = g->edges.end();
				
				bool found_one = false;
				for(; cure != ende; cure++) {
					if((*cure)->to == second) {
						(*cure)->to = last;
						if(found_one)
							break;
						found_one = true;
					} else if((*cure)->from == second) {
						rme = cure;
						if(found_one)
							break;
						found_one = true;
					}
				}
				if(rme != g->edges.end())
					g->edges.erase(rme);
				
				g->nodes.erase(trail);
				delete second;
				//this is doing something fucked up, so we'll play dumb
				goto RESTART_WP_REDUCE;
			} else {
				
				first = second;
				trail++;
			}
			pos++;
			second = last;
		}
	}
	
}

void break_long_lines(list<PathGraph*> &db_paths) {
/*
	Idea being to split up long lines into several line segments,
	generating several more nodes along paths in a controlled fashion.
*/
	GVector v1;
	
	GPoint curp;
	GPoint last;
	
	PathEdge *e,*ee;
	PathNode *n, *last_node;
	
	float cutlen2 = SPLIT_LINE_LENGTH*SPLIT_LINE_LENGTH;
	list<PathEdge*>::iterator cur4,end4;
	
	
	list<PathGraph*>::iterator cur, end;
	cur = db_paths.begin();
	end = db_paths.end();
	for(; cur != end; cur++) {
		PathGraph *g = *cur;
	
		cur4 = g->edges.begin();
		end4 = g->edges.end();
		for(; cur4 != end4; cur4++) {
			e = *cur4;
			
			//first check length of the edge...
			float len2 = e->from->Dist2(e->to);
			if(len2 < cutlen2)
				continue;
			

			float len = sqrt(len2);
			v1.x = (e->to->x - e->from->x)/len;
			v1.y = (e->to->y - e->from->y)/len;
			v1.z = (e->to->z - e->from->z)/len;
			
			float cuts = len / SPLIT_LINE_INTERVAL;
			
			v1 *= len / cuts;
			
			curp = *e->from;
			last_node = e->from;	//first source is the original from node
			for(; cuts > 1; cuts -= 1) {
				curp += v1;
				
				n = new PathNode(curp);
				ee = new PathEdge(last_node, n);
				last_node = n;
				
				g->edges.push_back(ee);
				g->nodes.push_back(n);
				
				broke_paths++;
			}
			//set the old edge to point from the last break to the original end node
			e->from = last_node;
		}
	}

}


void combine_trivial_grids(Map *map, list<PathGraph*> &db_paths) {
	list<PathGraph*>::iterator cur, end, check_cur, check_end;
	PathGraph *g;
	
	float CloseEnough2 = CLOSE_ENOUGH*CLOSE_ENOUGH;
	
	int cur_pos = 0;
	int r;
	bool merge;		//do we merge the current two grids
	PathNode *match1 = NULL, *match2 = NULL;	//the two points causing the merge
	PathGraph *look = NULL;	//the grid were looking at for merging into
	
//we are restarting so we dont have to deal with iterators
//when we delete a grid... they make my life difficult
RESTART_TRIVIAL_COMBINE:
	cur = db_paths.begin();
	end = db_paths.end();
	for(r = 0; r < cur_pos; r++)
		cur++;
	for(; cur != end; cur++, cur_pos++) {
		g = *cur;
		check_cur = cur;
		check_cur++;
		check_end = db_paths.end();
		
		merge = false;
		
		for(; check_cur != check_end && !merge; check_cur++) {
			look = *check_cur;
			if(g == look)
				continue;
			list<PathNode*>::iterator cur2,end2,cur3,end3;
			cur2 = g->nodes.begin();
			end2 = g->nodes.end();
			for(; cur2 != end2 && !merge; cur2++) {

				PathNode *git = *cur2;
				cur3 = look->nodes.begin();
				end3 = look->nodes.end();
				for(; cur3 != end3; cur3++) {
					if(git->Dist2(*cur3) < CloseEnough2) {
						match1 = git;
						match2 = *cur3;
						merge = true;
						break;
					}
				}
			}
		}
		
		if(look && merge) {
			/*printf("Merge on (%.3f, %.3f, %.3f) (%.3f, %.3f, %.3f)\n",
			match1->x, match1->y, match1->z,
			match2->x, match2->y, match2->z);
			*/
			
			//merge look into cur
			//this may or may not be a good idea:
//			look->nodes.reserve(look->nodes.size() + g->nodes.size());
//			look->edges.reserve(look->edges.size() + g->edges.size() + 1);
			
			//steal all the nodes
			list<PathNode*>::iterator cur2,end2;
			cur2 = g->nodes.begin();
			end2 = g->nodes.end();
			for(; cur2 != end2; cur2++) {
				if(*cur2 != match1)
					look->nodes.push_back(*cur2);
			}
			
			match1->valid = false;
			
			//steal all the edges
			list<PathEdge*>::iterator cur4,end4;
			cur4 = g->edges.begin();
			end4 = g->edges.end();
			for(; cur4 != end4; cur4++) {
				PathEdge *e = *cur4;
				//remap old node to new node
				if(e->to == match1)
					e->to = match2;
				if(e->from == match1)
					e->from = match2;
				look->edges.push_back(e);
			}
			
			//stop it from freeing up all the stuff we just stole.
			g->nodes.clear();
			g->edges.clear();
			delete g;
			
			trivial_merge_count++;
			
			db_paths.erase(cur);
			goto RESTART_TRIVIAL_COMBINE;
		}
	}
}

void combine_closest_grids(Map *map, list<PathGraph*> &db_paths) {
/*

run algorithm to connect all grids to eachother
 - form one large connected graph
 - maybe choose several connection points for each grid
   - only if they are very different (begin, middle, end?? 3 closest disjoint?)
 - check LOS for each connection
 - avoid large Z variance if possible
*/
	list<PathGraph*>::iterator cur, end, check_cur, check_end;
	PathGraph *g;
	
	int cur_pos = 0;
	
	float md2 = MERGE_MIN_SECOND_DIST*MERGE_MIN_SECOND_DIST;
	
	float dist, dist2, cdist;
	PathGraph *source1 = NULL, *source2 = NULL;
	PathNode *match1 = NULL, *match2 = NULL;	//the two points closest together
	PathNode *match3 = NULL, *match4 = NULL;	//the two points 2nd closest
//	VERTEX p1, p2, liz_res;
	
	PathGraph *look = NULL;	//the grid were looking at for merging into
	
	bool printing = false;
//	if(db_paths.size() > 100) {
		printf("Combining Grids (%d dots)", db_paths.size());
		fflush(stdout);
		printing = true;
//	}
	
//we are restarting so we dont have to deal with iterators
//when we delete a grid... they make my life difficult
RESTART_CLOSEST_COMBINE:
	cur = db_paths.begin();
	end = db_paths.end();
	for(; cur_pos > 0; cur_pos--)
		cur++;
	//for each grid
	for(; cur != end; cur++) {
		cur_pos++;
		g = *cur;
		//check_cur = cur;
		//check_cur++;
		check_cur = db_paths.begin();
		check_end = db_paths.end();
		
		if(printing) {
			printf(".");
			fflush(stdout);
		}
		
		dist = 9999999999e100f;
		dist2 = 9999999999e100f;
		match1 = NULL;
		match2 = NULL;
		match3 = NULL;
		match4 = NULL;
		source1 = NULL;
		source2 = NULL;
		
		//for each other grid
		for(; check_cur != check_end; check_cur++) {
			look = *check_cur;
			if(g == look)
				continue;
			list<PathNode*>::iterator cur2,end2,cur3,end3;
			cur2 = g->nodes.begin();
			end2 = g->nodes.end();
			//for each node in g
			for(; cur2 != end2; cur2++) {

				PathNode *git = *cur2;
				cur3 = look->nodes.begin();
				end3 = look->nodes.end();
				//for each node in look
				for(; cur3 != end3; cur3++) {

					cdist = git->Dist2(*cur3);
					if(cdist > dist2)
						continue;	//not a candidate
					
					if(!CheckLOS(map, git, *cur3))
						continue;	//cannot see
					
					if(cdist < dist) {
						//new closest
						//demote this closest to #2 if its far enough away
						if(match2 && dist < dist2 && match2->Dist2(*cur3) > md2) {
							dist2 = dist;
							match3 = match1;
							match4 = match2;
							source2 = source1;
						}
						
						//setup new closest
						dist = cdist;
						match1 = git;
						match2 = *cur3;
						source1 = look;
					} else if(cdist < dist2 && match2->Dist2(*cur3) > md2) {
						//new second closest
						dist2 = cdist;
						match3 = git;
						match4 = *cur3;
						source2 = look;
					}
				}
			}
		}
		
		if(look != NULL && source1 != NULL) {
			/*printf("Link on (%.3f, %.3f, %.3f) (%.3f, %.3f, %.3f) at dist %.3f\n",
			match1->x, match1->y, match1->z,
			match2->x, match2->y, match2->z, dist);
			printf("Combine %d nodes with %d nodes.\n", source1->nodes.size(), g->nodes.size());
			*/
			
			//merge look into cur
			//this may or may not be a good idea:
//			source1->nodes.reserve(look->nodes.size() + g->nodes.size());
//			source1->edges.reserve(look->edges.size() + g->edges.size() + 1);
			
			//steal all the nodes
			/* this just gets done later by re-node so dont do it now.
			
			list<PathNode*>::iterator cur2,end2;
			cur2 = g->nodes.begin();
			end2 = g->nodes.end();
			for(; cur2 != end2; cur2++) {

				if(*cur2 != match1)
					source1->nodes.push_back(*cur2);
			}*/
			
			//steal all the edges
			list<PathEdge*>::iterator cur4,end4;
			cur4 = g->edges.begin();
			end4 = g->edges.end();
			for(; cur4 != end4; cur4++) {
				PathEdge *e = *cur4;
				//remap old node to new node
				if(e->to == match1)
					e->to = match2;
				if(e->from == match1)
					e->from = match2;

				source1->edges.push_back(e);
			}

			
			
			check_cur = cur;
			check_cur++;
			check_end = db_paths.end();
			for(; check_cur != check_end; check_cur++) {
				look = *check_cur;
				list<PathEdge*>::iterator cur4,end4;
				cur4 = g->edges.begin();
				end4 = g->edges.end();
				for(; cur4 != end4; cur4++) {
					PathEdge *e = *cur4;
					//remap old node to new node
					if(e->to == match1)
						e->to = match2;
					if(e->from == match1)
						e->from = match2;
				}
			}
			
			closest_merge_count++;
		}
		
		//kinda a hack... assume the 'same node merge' code will bridge this
		//new point with the other point in the real 'source2' graph.
		if(source2 != NULL) {
			/*printf("Link2 on (%.3f, %.3f, %.3f) (%.3f, %.3f, %.3f) at dist %.3f\n",
			match3->x, match3->y, match3->z,
			match4->x, match4->y, match4->z, dist);*/
			
			PathNode *n = new PathNode(*match4);
			source1->nodes.push_back(n);
			source1->add_edge(match3, n);
			
			closest_merge2_count++;
		}
		
		
		if(source1 != NULL) {
			//stop it from freeing up all the stuff we just stole.
			g->nodes.clear();
			g->edges.clear();
			delete g;
			
			db_paths.erase(cur);
			goto RESTART_CLOSEST_COMBINE;
		}
	}
	
	if(printing) {
		printf("\n");
	}
	
	
	//finally just do a dummy merge of the remaining grids into the first one
	g = db_paths.front();
	cur = db_paths.begin();
	end = db_paths.end();
	cur++;	//skip the first one
	for(; cur != end; cur++) {
		g->add_edges((*cur)->edges);
	}
	
	
	//rebuild our node array based on the edges we have...
	//this is because there is some sort of error where a node is used in
	//an edge which is supposed to have been combined with another node.
	//I cannot figure out why, and this fixes it without any harmful effects
	//as far as I can tell, the trees still span and what not.
	printf("Closest Merge: had %d nodes and %d edges\n", g->nodes.size(), g->edges.size());


/*	g->nodes.resize(0);
	std::map<PathNode *, int> havenodelist;
	list<PathEdge*>::iterator cur4,end4;
	cur4 = g->edges.begin();
	end4 = g->edges.end();
	for(; cur4 != end4; cur4++) {
		PathEdge *e = *cur4;
		//remap old node to new node
		if(havenodelist.count(e->from) != 1) {
			g->nodes.push_back(e->from);
			havenodelist[e->from] = 1;
		}
		if(havenodelist.count(e->to) != 1) {
			g->nodes.push_back(e->to);
			havenodelist[e->to] = 1;
		}
	}*/
	rebuild_node_list(g->edges, g->nodes);
	
	printf("Closest Merge: re-node yeilded %d nodes and %d edges\n", g->nodes.size(), g->edges.size());
}


/*void build_big_graph(PathGraph *big, list<PathGraph*> &db_paths, list<PathNode*> &db_spawns) {
//basically just move all the nodes and edges into one big graph
	list<PathGraph*>::iterator cur, end;
	cur = db_paths.begin();
	end = db_paths.end();
	for(; cur != end; cur++) {
		PathGraph *g = *cur;
		
		//this may or may not be a good idea:
		big->nodes.reserve(big->nodes.size() + g->nodes.size());
		big->edges.reserve(big->edges.size() + g->edges.size());
		
		//steal all the nodes
		list<PathNode*>::iterator cur2,end2;
		cur2 = g->nodes.begin();
		end2 = g->nodes.end();
		for(; cur2 != end2; cur2++) {
			big->nodes.push_back(*cur2);
		}
		
		//steal all the edges
		list<PathEdge*>::iterator cur4,end4;
		cur4 = g->edges.begin();
		end4 = g->edges.end();
		for(; cur4 != end4; cur4++) {
			big->edges.push_back(*cur4);
		}
		
		g->nodes.clear();
		g->edges.clear();
		delete g;
	}
	db_paths.clear();
	
	list<PathNode*>::iterator cur3,end3;
	big->nodes.reserve(big->nodes.size() + db_spawns.size());
	cur3 = db_spawns.begin();
	end3 = db_spawns.end();
	for(; cur3 != end3; cur3++) {
		big->nodes.push_back(*cur3);
	}
	
	db_spawns.clear();
	
	
}
*/

void link_spawns(Map *map, PathGraph *big, list<PathNode*> &db_spawns, 
  float maxdist, map< pair<PathNode *, PathNode *>, bool > *edgelist) {
/*

run algorithm to connect all spawn points to the big grid
 - first find the closest node we have LOS to
 - optionally try to find a second node too, just to give us more options
   - I am not sure if this will buy us anything... since MST will very likely kill this link
   - find the second closest node which has a vector thats not close to the closest
   - for each node in big grid N
     - Check LOS between mob and N
     - find distance from mob to N, if not possibly 2nd closest: continue
	 - determine
   - check LOS for each connection
   - connect to 1 or 2 of the closest points (prolly 2 if possible)
   - avoid large Z variance if possible
*/
	
	//still not sure if this actually does us any good.
//	big->nodes.reserve(big->nodes.size() + db_spawns.size());
	
	
	printf("Linking (%d dots)", db_spawns.size());
	fflush(stdout);
	float md2 = SPAWN_MIN_SECOND_DIST*SPAWN_MIN_SECOND_DIST;
	float maxdist2 = maxdist*maxdist;
	
	float dist,tmp;
	PathNode *closest = NULL;
	float dist2;
	PathNode *closest2 = NULL;
	float dist3;
	PathNode *closest3 = NULL;
	
	VERTEX p1, liz_res /*, p2*/;
	
	list<PathNode*>::iterator cur,end;
	list<PathNode*>::iterator cur3,end3;
	
	cur3 = db_spawns.begin();
	end3 = db_spawns.end();
	for(; cur3 != end3; cur3++) {
		printf(".");
		fflush(stdout);
		PathNode *n = *cur3;
		big->nodes.push_back(n);
		p1.x = n->x; p1.y = n->y; p1.z = n->z;
		//elevate a little, to about eye height
		p1.z += 6.0f;
		
		//make sure this spawn point is even within the map
		NodeRef mynode;
		mynode = map->SeekNode(map->GetRoot(), p1.x, p1.y);
		if(mynode == NODE_NONE) {
			link_spawn_invalid++;
			continue;
		}
		if(map->FindBestZ(mynode, p1, &liz_res) == BEST_Z_INVALID) {
			link_spawn_invalid++;
			continue;
		}
		
		dist = 999999e100f;
		closest = NULL;
		dist2 = 999999e100f;
		closest2 = NULL;
		dist3 = 999999e100f;
		closest3 = NULL;
		
		cur = big->nodes.begin();
		end = big->nodes.end();
		for(; cur != end; cur++) {
			if(n == *cur)
				continue;	//dont link to ourself
			
			//if an edge list was supplied, make sure this edge isnt on it
			if(edgelist) {
				pair<PathNode *, PathNode *> id;
				if(int32(n) < int32(*cur)) {
					id.first = *cur;
					id.second = n;
				} else {
					id.first = n;
					id.second = *cur;
				}
				if(edgelist->find(id) != edgelist->end())
					continue;	//found in the list
			}

			//get the distance between the 
			tmp = n->Dist2(*cur);
			if(tmp > dist2)
				continue;
			
			if(!CheckLOS(map, n, *cur))
				continue;	//cannot see
			
			//we can see to it, see if its closer
			if(tmp < dist) {
				//its a new closest
				
				//see if we bump #2
				if(SPAWN_LINK_TWICE && closest && dist < dist2 && closest->Dist2(*cur) > md2) {
					//the old #1 replaces #2
					//see if we bump #3
					if(SPAWN_LINK_THRICE && closest2 && dist2 < dist3 
						&& closest2->Dist2(*cur) > md2
						&& closest2->Dist2(closest) > md2) {
						dist3 = dist2;
						closest3 = closest2;
					}
					dist2 = dist;
					closest2 = closest;
				}
					
				dist = tmp;
				closest = *cur;
			}
			//we can assume closest is set, or else we would never get here
			 else if(SPAWN_LINK_TWICE && tmp < dist2 && closest->Dist2(*cur) > md2) {
				//see if we bump #3
				if(SPAWN_LINK_THRICE && closest2 && dist2 < dist3 
					&& closest2->Dist2(*cur) > md2
					&& closest2->Dist2(closest) > md2) {
					dist3 = dist2;
					closest3 = closest2;
				}
				dist2 = tmp;
				closest2 = *cur;
			}
			//we can assume closest and closest2 are set, or else we would never get here
			 else if(SPAWN_LINK_THRICE && tmp < dist3 && closest->Dist2(*cur) > md2 && closest2->Dist2(*cur) > md2) {
				dist3 = tmp;
				closest3 = *cur;
			 }
		}
		
		if(closest == NULL || dist > maxdist2) {
			link_spawn_nocount++;
			//should delete this point....
			continue;
		}
		
/*printf("SL (%.3f, %.3f, %.3f) (%.3f, %.3f, %.3f) d2=%.3f\n",
		n->x, n->y, n->z,
		closest->x, closest->y, closest->z, n->Dist2(closest));
		link_spawn_count++;
*/		
		
		big->add_edge(n, closest);
		
		if(SPAWN_LINK_TWICE && closest2 && dist2 < maxdist2) {

/*printf("SL2 (%.3f, %.3f, %.3f) (%.3f, %.3f, %.3f) d2=%.3f\n",
		n->x, n->y, n->z,
		closest2->x, closest2->y, closest2->z, n->Dist2(closest2));
*/			big->add_edge(n, closest2);
			link_spawn2_count++;
		}
		if(SPAWN_LINK_THRICE && closest3 && dist3 < maxdist2) {

/*printf("SL3 (%.3f, %.3f, %.3f) (%.3f, %.3f, %.3f) d2=%.3f\n",
		n->x, n->y, n->z,
		closest2->x, closest2->y, closest2->z, n->Dist2(closest2));
*/			big->add_edge(n, closest3);
			link_spawn3_count++;
		}
	}
	
	printf("\n");
}

map< pair<PathNode *, PathNode *>, bool > _los_cache;

bool CheckLOS(Map *map, PathNode *from, PathNode *to) {
	static VERTEX p1, p2, liz_res;	//no reason to allocate them several times
	
	//hack to make order on the ID not matter
	if(int32(from) < int32(to)) {
		PathNode *tmp = from;
		from = to;
		to = tmp;
	}
	
	pair<PathNode *, PathNode *> id(from, to);
	if(_los_cache.count(id) == 1) {
		los_cache_hits++;
		return(_los_cache[id]);
	}
	
	//if its a candidate, check LOS
	p1.x = from->x; p1.y = from->y; p1.z = from->z;
	p2.x = to->x; p2.y = to->y; p2.z = to->z;
	//elevate a little, to about eye height
	p1.z += 6.0f; p2.z += 6.0f;
	
	bool res = !map->LineIntersectsZone(p1, p2, 0.5, &liz_res);
	
	_los_cache[id] = res;
	los_cache_misses++;
	
	return(res);
}

void combine_grid_points(Map *map, PathGraph *big, float close_enough) {
/*
run an algorithm to remove redundancy:
 - Two points close to eachother (connected or not): merge links into 1, delete other
  - for each node, check dist to all other nodes...
 - Two of the same link after merge: delete one
  - for each node, for each link in that node, see if its the same as any other link
  - do we need this? MST will eliminate these, but will run slower
*/
	list<PathNode*>::iterator cur,end,cur2;
	list<PathEdge*>::iterator cur4,end4;
	PathNode *n,*f;
	PathEdge *e;
	
	combined_grid_points = 0;
	combine_broke_los = 0;
	float ce2 = close_enough*close_enough;
	bool merge;
	int cur_pos = 0, stat = 0;
	int r;
	
	printf("Combining.");
	fflush(stdout);

#ifdef COMBINE_CHECK_ALL_LOS
	bool badlos = false;
	//build our node->edge map for use later checking combine LOS
	std::map<PathNode*, vector<PathEdge*> > node_edges;
	vector<PathEdge*>::iterator curE,endE;
	
	cur4 = big->edges.begin();
	end4 = big->edges.end();
	for(; cur4 != end4; cur4++) {
		e = *cur4;
		
		if(node_edges.count(e->from) == 1) {
			node_edges[e->from].push_back(e);
		} else {
			vector<PathEdge*> t(1);
			t[0] = e;
			node_edges[e->from] = t;
		}
		
		if(node_edges.count(e->to) == 1) {
			node_edges[e->to].push_back(e);
		} else {
			vector<PathEdge*> t(1);
			t[0] = e;
			node_edges[e->to] = t;
		}
	}
	
#endif
	
//restart since deleting a node pisses the iterators off and
//im too lazy to figure out how to make them happy right now
RESTART_GRID_CLEAN:
	cur = big->nodes.begin();
	end = big->nodes.end();
	for(r = 0; r < cur_pos; r++)
		cur++;
	for(; cur != end; cur++, cur_pos++, stat++) {
		if(stat%1000 == 0) {
			printf(".");
			fflush(stdout);
		}

		n = *cur;
		cur2 = cur;
		cur2++;
		merge = false;
		
		if(!n->valid)
			continue;
		
		for(; cur2 != end; cur2++) {
			f = *cur2;
			if(n == f)
				continue;
			if(n->Dist2(f) > ce2) {
				continue;
			}
			
#ifdef COMBINE_CHECK_ALL_LOS
			//we should merge these. Now we wanna check to make sure combining
			//them will not cause the old node's edges to become invalid.
			
			badlos = false;
			
			if(node_edges.count(f) == 1) {
				curE = node_edges[f].begin();
				endE = node_edges[f].end();
				for(; curE != endE; curE++) {
					e = *curE;
					if(e->to == f) {
						if(!CheckLOS(map, n, e->from)) {
							badlos = true;
							combine_broke_los++;
							break;
						}
					} else if(e->from == f) {
						if(!CheckLOS(map, n, e->to)) {
							badlos = true;
							combine_broke_los++;
							break;
						}
					}
				}
			}
			
/*			cur4 = big->edges.begin();
			end4 = big->edges.end();
			for(; cur4 != end4; cur4++) {
				e = *cur4;
				if(e->to == f) {
					if(!CheckLOS(map, n, e->from)) {
						badlos = true;
						combine_broke_los++;
						break;
					}
				} else if(e->from == f) {
					if(!CheckLOS(map, n, e->to)) {
						badlos = true;
						combine_broke_los++;
						break;
					}
				}
			}
*/
			if(badlos)
				continue;
#else
			//check LOS between the nodes is an OK compromise
			if(!CheckLOS(map, n, f))
				continue;
#endif
			//cant merge two forced nodes
			if(n->forced && f->forced)
				continue;

			//passed the checks, lets merge with it.
			merge = true;
			break;
		}
//		printf("checked %d, merge? %d\n", cur_pos, merge);
		if(merge) {
			f = *cur2;
			
			//normally we merge into n, from f.
			//if f is forced, then we must reverse that...
			if(f->forced) {
				*cur = f;		//swap them in the array
				PathNode *tmp = f;
				f = n;
				n = tmp;
			}
			
			//steal all its edges...
			//changing references to old node to point to the other node
			cur4 = big->edges.begin();
			end4 = big->edges.end();
			for(; cur4 != end4; cur4++) {
				e = *cur4;
				if(e->to == f)
					e->to = n;
				if(e->from == f)
					e->from = n;
			}
			
#ifdef COMBINE_CHECK_ALL_LOS
			//merge over the edge list too..
			if(node_edges.count(f) == 1 && node_edges.count(n) == 1) {
				vector<PathEdge*> &dst = node_edges[n];
				curE = node_edges[f].begin();
				endE = node_edges[f].end();
				for(; curE != endE; curE++) {
					e = *curE;
					dst.push_back(e);
				}
			}
#endif
			
			combined_grid_points++;
			
			/*printf("Removed point using %d (0x%x and 0x%x)\n", cur_pos, n, f);
			
			printf("Pts (%.3f, %.3f, %.3f) (%.3f, %.3f, %.3f) at dist %.3f\n",
			n->x, n->y, n->z,
			f->x, f->y, f->z, n->Dist2(f));*/
			
			//who needs to free memory, when we can leak it....
			//delete f;
			
			big->nodes.erase(cur2);
			goto RESTART_GRID_CLEAN;
		}
	}
	printf("\n");
}


void draw_paths(Map *map, list<PathEdge *> &edges, list<PathEdge *> &edges2, const char *fname) {
	FILE *pngout;
	pngout = fopen(fname, "wb");
	if(pngout == NULL) {
		printf("Unable to open %s\n", fname);
		return;
	}
	
	list<PathEdge*>::iterator cur,end;
	PathEdge *e;
	
	gdImagePtr im;
	int minx = int(map->GetMinX());
	int maxx = int(map->GetMaxX());
	int miny = int(map->GetMinY());
	int maxy = int(map->GetMaxY());
	
//	float minz = map->GetMinZ();
//	float maxz = map->GetMaxZ();
	//find better z ranges
	float minz = 9999e99;
	float maxz = -9999e99;
	cur = edges.begin();
	end = edges.end();
	for(; cur != end; cur++) {
		e = *cur;
		if((*cur)->from->z < minz)
			minz = (*cur)->from->z;
		if((*cur)->from->z > maxz)
			maxz = (*cur)->from->z;
		if((*cur)->to->z < minz)
			minz = (*cur)->to->z;
		if((*cur)->to->z > maxz)
			maxz = (*cur)->to->z;
	}
	
	
	im = gdImageCreate((maxx - minx)/IMAGE_SCALE, (maxy - miny)/IMAGE_SCALE);
	
	//allocate this first, to make it the BG color.
	/*int black =*/ gdImageColorAllocate(im, 0, 0, 0); 
	
	int blue = gdImageColorAllocate(im, 200, 200, 255);
	
	
	//draw our EQ map
	cur = edges2.begin();
	end = edges2.end();
	for(; cur != end; cur++) {
		e = *cur;
		int x1 = int(e->from->x) - minx;
		int y1 = int(e->from->y) - miny;
		int x2 = int(e->to->x) - minx;
		int y2 = int(e->to->y) - miny;
		x1 /= IMAGE_SCALE;
		y1 /= IMAGE_SCALE;
		x2 /= IMAGE_SCALE;
		y2 /= IMAGE_SCALE;
		gdImageLine(im, x1, y1, x2, y2, blue);
	}
	
	
	GPoint p1, p2;
	vector<ColorRecord> colors;
	allocateGradient(im, 255, 255, 0, 255, 0, 0, minz, maxz, 100, colors);
	
	//draw the edges supplied with gradient lines
	cur = edges.begin();
	end = edges.end();
	for(; cur != end; cur++) {
		e = *cur;
		
		p1 = *e->from;
		p2 = *e->to;
		p1.x -= minx;
		p1.y -= miny;
		p2.x -= minx;
		p2.y -= miny;
		p1.x /= IMAGE_SCALE;
		p1.y /= IMAGE_SCALE;
		p2.x /= IMAGE_SCALE;
		p2.y /= IMAGE_SCALE;
		DrawGradientLine(im, &p1, &p2, colors);
	}
	
	gdImagePng(im, pngout);
	gdImageDestroy(im);
	
	fclose(pngout);
	
	printf("Wrote image: %s\n", fname);
}


void draw_paths2(Map *map, list<PathEdge *> &edges, list<PathEdge *> &edges2, list<PathEdge *> &edges3, list<PathNode *> &spawns, const char *fname) {
	FILE *pngout;
	pngout = fopen(fname, "wb");
	if(pngout == NULL) {
		printf("Unable to open %s\n", fname);
		return;
	}
	
	list<PathEdge*>::iterator cur,end;
	list<PathNode*>::iterator curn,endn;
	PathEdge *e;
	PathNode *n;
	
	gdImagePtr im;
	int minx = int(map->GetMinX());
	int maxx = int(map->GetMaxX());
	int miny = int(map->GetMinY());
	int maxy = int(map->GetMaxY());


//	float minz = map->GetMinZ();
//	float maxz = map->GetMaxZ();
	//find better z ranges
	float minz = 9999e99;
	float maxz = -9999e99;
	cur = edges2.begin();
	end = edges2.end();
	for(; cur != end; cur++) {
		e = *cur;
		if((*cur)->from->z < minz)
			minz = (*cur)->from->z;
		if((*cur)->from->z > maxz)
			maxz = (*cur)->from->z;
		if((*cur)->to->z < minz)
			minz = (*cur)->to->z;
		if((*cur)->to->z > maxz)
			maxz = (*cur)->to->z;
	}
	
	im = gdImageCreate((maxx - minx)/IMAGE_SCALE, (maxy - miny)/IMAGE_SCALE);
	
	//allocate this first, to make it the BG color.
	/*int black =*/ gdImageColorAllocate(im, 0, 0, 0);
	
	int grey = gdImageColorAllocate(im, 100, 100, 100);
	int purple = gdImageColorAllocate(im, 255, 100, 255);
	
//	int red = gdImageColorAllocate(im, 190, 0, 0); 
	int axis = gdImageColorAllocate(im, 75, 0, 0);
	int blue = gdImageColorAllocate(im, 200, 200, 255);
	int green = gdImageColorAllocate(im, 0, 255, 0);
	
	
	//draw the axes
	gdImageLine(im, -minx/IMAGE_SCALE, 0, -minx/IMAGE_SCALE, (maxy - miny)/IMAGE_SCALE, axis);
	gdImageLine(im, 0, -miny/IMAGE_SCALE, (maxx - minx)/IMAGE_SCALE, -miny/IMAGE_SCALE, axis);
	
	
	//draw the original paths in grey
	//draw these first cause they are messy and not important really
	cur = edges.begin();
	end = edges.end();
	for(; cur != end; cur++) {
		e = *cur;
		int x1 = int(e->from->x) - minx;
		int y1 = int(e->from->y) - miny;
		int x2 = int(e->to->x) - minx;
		int y2 = int(e->to->y) - miny;
		x1 /= IMAGE_SCALE;
		y1 /= IMAGE_SCALE;
		x2 /= IMAGE_SCALE;
		y2 /= IMAGE_SCALE;
		gdImageLine(im, x1, y1, x2, y2, grey);
	}
	
	
	//draw the EQ map second, so we can see it
	cur = edges3.begin();
	end = edges3.end();
	for(; cur != end; cur++) {
		e = *cur;
		int x1 = int(e->from->x) - minx;
		int y1 = int(e->from->y) - miny;
		int x2 = int(e->to->x) - minx;
		int y2 = int(e->to->y) - miny;
		
		//invert the EQ map in screen coords
		/*int tmp;
		tmp = x1; x1 = y1; y1 = tmp;
		tmp = x2; x2 = y2; y2 = tmp;*/
		
		x1 /= IMAGE_SCALE;
		y1 /= IMAGE_SCALE;
		x2 /= IMAGE_SCALE;
		y2 /= IMAGE_SCALE;
		gdImageLine(im, x1, y1, x2, y2, blue);
	}
	
	GPoint p1, p2;
	vector<ColorRecord> colors;
	allocateGradient(im, 255, 255, 0, 255, 0, 0, minz, maxz, 100, colors);
	
	cur = edges2.begin();
	end = edges2.end();
	for(; cur != end; cur++) {
		e = *cur;
		
		if(e->valid) {
			p1 = *e->from;
			p2 = *e->to;
			p1.x -= minx;
			p1.y -= miny;
			p2.x -= minx;
			p2.y -= miny;
			p1.x /= IMAGE_SCALE;
			p1.y /= IMAGE_SCALE;
			p2.x /= IMAGE_SCALE;
			p2.y /= IMAGE_SCALE;
			DrawGradientLine(im, &p1, &p2, colors);
		} else {
			int x1 = int(e->from->x) - minx;
			int y1 = int(e->from->y) - miny;
			int x2 = int(e->to->x) - minx;
			int y2 = int(e->to->y) - miny;
			x1 /= IMAGE_SCALE;
			y1 /= IMAGE_SCALE;
			x2 /= IMAGE_SCALE;
			y2 /= IMAGE_SCALE;
			gdImageLine(im, x1, y1, x2, y2, green);
//			printf("L (%d,%d,%.1f) -> (%d,%d,%.1f) dist=%.3f\n", x1, y1, e->from->z, x2, y2, e->to->z, e->from->Dist2(e->to));
		}
	}
	
	curn = spawns.begin();
	endn = spawns.end();
	for(; curn != endn; curn++) {
		n = *curn;
		int x1 = int(n->x) - minx;
		int y1 = int(n->y) - miny;
		x1 /= IMAGE_SCALE;
		y1 /= IMAGE_SCALE;
		gdImageSetPixel(im, x1, y1, purple);
	}
	
	gdImagePng(im, pngout);
	gdImageDestroy(im);
	
	fclose(pngout);
	
	printf("Wrote image: %s\n", fname);
}


void check_edge_los(Map *map, PathGraph *big) {
	list<PathEdge*>::iterator cur,end,tmp;
	PathEdge *e;
//	VERTEX p1, p2, liz_res;
	cur = big->edges.begin();
	end = big->edges.end();
	for(; cur != end;) {
		e = *cur;
		
/*		//if its a candidate, check LOS
		p1.x = e->from->x; p1.y = e->from->y; p1.z = e->from->z;
		p2.x = (e->to)->x; p2.y = (e->to)->y; p2.z = (e->to)->z;
		//elevate a little, to about eye height
		p1.z += 6.0f; p2.z += 6.0f;
		
		if(map->LineIntersectsZone(p1, p2, 0.1, &liz_res)) {*/
		if(!CheckLOS(map, e->from, e->to)) {
			tmp = cur;
			cur++;
			big->edges.erase(tmp);
			removed_edges_los++;
		} else {
			cur++;
		}
	}
	
}


void check_long_edge_los(Map *map, PathGraph *big) {
#ifdef LONG_PATH_CHECK_LOS
	list<PathEdge*>::iterator cur,end,tmp;
	PathEdge *e;
	
	float ml2 = LONG_PATH_CHECK_LOS*LONG_PATH_CHECK_LOS;
//	VERTEX p1, p2, liz_res;
	cur = big->edges.begin();
	end = big->edges.end();
	for(; cur != end;) {
		e = *cur;
		
		if(e->from->Dist2(e->to) < ml2) {
			cur++;
			continue;
		}
		
/*		//if its a candidate, check LOS
		p1.x = e->from->x; p1.y = e->from->y; p1.z = e->from->z;
		p2.x = (e->to)->x; p2.y = (e->to)->y; p2.z = (e->to)->z;
		//elevate a little, to about eye height
		p1.z += 6.0f; p2.z += 6.0f;
		
		if(map->LineIntersectsZone(p1, p2, 0.1, &liz_res)) {*/
		if(!CheckLOS(map, e->from, e->to)) {
			tmp = cur;
			cur++;
			big->edges.erase(tmp);
			removed_long_edges_los++;
		} else {
			cur++;
		}
	}
#endif	
}

//take rgb as floats for simplicity
void allocateGradient(gdImagePtr im, float r1, float g1, float b1, float r2, float g2, float b2, 
	float min, float max, float divs, vector<ColorRecord> &colors) {
	
	float step = (max - min) / divs;
	float rstep = (r2 - r1)/divs;
	float gstep = (g2 - g1)/divs;
	float bstep = (b2 - b1)/divs;
	
	ColorRecord c;
	
	c.height = min;
	float r;
	for(r = 0; r < divs; r++) {
		c.color = gdImageColorAllocate(im, int(r1), int(g1), int(b1));
		c.height += step;
		r1 += rstep;
		g1 += gstep;
		b1 += bstep;
		colors.push_back(c);
	}
}


void DrawGradientLine(gdImagePtr im, GPoint *first, GPoint *second, vector<ColorRecord> &colors) {
	GVector v1(*first, *second);
	
//	float len = sqrt(first->Dist2(second));
	
	//the - operator is apparently not working or something
	v1.x = second->x - first->x;
	v1.y = second->y - first->y;
	v1.z = second->z - first->z;
	
//	float len = v1.length();
	
//	v1.normalize();	//calcs length again, but im lazy.
	
	
	GPoint cur(*first);
	GPoint last;
	
	float step = 1/100.0f;
	float pos;
	
	v1 *= step;

/*printf("Line From (%.3f, %.3f, %.3f) (%.3f, %.3f, %.3f)\n",
		first->x, first->y, first->z,
		second->x, second->y, second->z);*/
	
	vector<ColorRecord>::iterator curc,end;
	
	for(pos = 0; pos < 1; pos += step) {
		last = cur;
		cur += v1;
		//shitty method, dont care, lazy
		curc = colors.begin();
		end = colors.end();
		for(; curc != end; curc++) {
			if((*curc).height > last.z)
				break;
		}
		int ccolor;
		if(curc == end) {
//			printf("Unable to find color at height %.3f (range %.3f -> %.3f)\n", 
//				last.z, colors[0].height, colors[colors.size()-1].height);
			ccolor = colors[colors.size()-1].color;
		} else {
//			printf("Found color at height %.3f (range %.3f -> %.3f)\n", 
//				last.z, colors[0].height, colors[colors.size()-1].height);
			ccolor = (*curc).color;
		}
		gdImageLine(im, int(last.x), int(last.y), int(cur.x), int(cur.y), ccolor);

/*printf("   Segment (%.3f, %.3f) (%.3f, %.3f) color %d\n",
		last.x, last.y,
		cur.x, cur.y, ccolor);*/
	}
}

bool edges_cross(PathEdge *e1, PathEdge *e2, GPoint &out) {
//I love macros
#define IntersectDenom(p1, p2, p3, p4) \
((p4->y - p3->y)*(p2->x - p1->x) - (p4->x - p3->x)*(p2->y - p1->y))
#define IntersectNumerX(p1, p2, p3, p4) \
((p4->x - p3->x)*(p1->y - p3->y) - (p4->y - p3->y)*(p1->x - p3->x))
#define IntersectNumerY(p1, p2, p3, p4) \
((p2->x - p1->x)*(p1->y - p3->y) - (p2->y - p1->y)*(p1->x - p3->x))

#define IntersectX(p1, p2, p3, p4, denom) \
(p1->x + IntersectNumerX(p1, p2, p3, p4)*(p2->x - p1->x)/denom)
#define IntersectY(p1, p2, p3, p4, denom) \
(p1->y + IntersectNumerX(p1, p2, p3, p4)*(p2->y - p1->y)/denom)

#define CheckEqualXY(p1, p2) \
 (p1->x == p2->x && p1->y == p2->y)

#define CoordOnLine(p1, p2, coord, dim) \
 (p1->dim > p2->dim? (coord > p2->dim && coord < p1->dim) : (coord > p1->dim && coord < p2->dim))

#define IntersectZfromX(p1, p2, inter) \
 (p2->x > p1->x? \
 (p1->z + ((inter - p1->x)/(p2->x - p1->x) * (p2->z - p1->z))) \
 :(p2->z + ((inter - p2->x)/(p1->x - p2->x) * (p1->z - p2->z))))
	
 	if(e1 == e2)
 		return(false);
	
	float denom = IntersectDenom(e1->from, e1->to, e2->from, e2->to);
	if(denom != 0) {
		
		//see if this is at the end points... that dosent count.
		//caught below by strict inequality
/*		if(		CheckEqualXY(e1->from, e2->from)
			||	CheckEqualXY(e1->from, e2->to)
			||	CheckEqualXY(e1->to, e2->from)
			||	CheckEqualXY(e1->to, e2->to) ) {
				return(false);
		}*/
		
		
		//the lines intersect, check segments now
		float xinter = IntersectX(e1->from, e1->to, e2->from, e2->to, denom);
		float yinter = IntersectY(e1->from, e1->to, e2->from, e2->to, denom);
		
		//need to add a Z check in here
		float zinter1 = IntersectZfromX(e1->from, e1->to, xinter);
		float zinter2 = IntersectZfromX(e2->from, e2->to, xinter);
		
		float dist = zinter1 - zinter2;
		zinter1 += dist * 0.5;		//middle ground on intersect point.
		
		if(dist < 0) {	//z2 is above z1
			dist = 0 - dist;
		}
		
		if(dist > CROSS_MAX_Z_DIFF)
			return(false);

		//now see if this point is on both segments
		if(		CoordOnLine(e1->from, e1->to, xinter, x)
			&&	CoordOnLine(e1->from, e1->to, yinter, y)
			&&	CoordOnLine(e2->from, e2->to, xinter, x)
			&&	CoordOnLine(e2->from, e2->to, yinter, y) ){
//			printf("Line (%.3f,%.3f,%.3f) -> (%.3f,%.3f,%.3f) d=%.3f\n", e1->from->x, e1->from->y, e1->from->z, e1->to->x, e1->to->y, e1->to->z, e1->from->Dist2(e1->to));
//			printf("Hits (%.3f,%.3f,%.3f) -> (%.3f,%.3f,%.3f) d=%.3f\n", e2->from->x, e2->from->y, e2->from->z, e2->to->x, e2->to->y, e2->to->z, e2->from->Dist2(e2->to));
//			printf("At (%.3f, %.3f), which IS on both segments.\n", xinter, yinter);
			out.x = xinter;
			out.y = yinter;
			out.z = zinter1;
			return(true);
		}
//		printf("At (%.3f, %.3f), which is not on both segments.\n", xinter, yinter);
	}
	return(false);
}

void count_crossing_lines(list<PathEdge *> &edges, PathGraph *out, PathGraph *excess, map<PathEdge*, vector<GPoint> > &cross_list) {
	list<PathEdge*>::iterator cur,end,cur2;
	PathEdge *e, *look;
	
	out->edges.resize(0);
	excess->edges.resize(0);
	
	float cml2 = CROSS_MIN_LENGTH*CROSS_MIN_LENGTH;
	
	cur = edges.begin();
	end = edges.end();
	vector<GPoint> hits;
	GPoint hit;
	for(; cur != end; cur++) {
		e = *cur;
		
		//assume that small edges dont matter...
		if(e->from->Dist2(e->to) < cml2)
			continue;
		
		hits.clear();
		int count = 0;
		
		cur2 = edges.begin();
		for(; cur2 != end; cur2++) {
			look = *cur2;
			
			if(edges_cross(e, look, hit)) {
				count++;
				hits.push_back(hit);
			}
		}
		
		
		if(count >= CROSS_REDUCE_COUNT) {
			out->edges.push_back(e);
			cross_edge_count++;
			cross_list[e] = hits;
		} else {
			excess->edges.push_back(e);
		}
	}
	
	rebuild_node_list(out->edges, out->nodes, &excess->nodes);
}

void rebuild_node_list(list<PathEdge *> &edges, list<PathNode *> &nodes, list<PathNode *> *excess_nodes) {
	list<PathNode *> in_nodes;
	if(excess_nodes != NULL) {
		in_nodes = nodes;
	}
	
	nodes.resize(0);
	std::map<PathNode *, int> havenodelist;
	list<PathEdge*>::iterator cur4,end4;
	cur4 = edges.begin();
	end4 = edges.end();
	for(; cur4 != end4; cur4++) {
		PathEdge *e = *cur4;
		if(havenodelist.count(e->from) != 1) {
			nodes.push_back(e->from);
			havenodelist[e->from] = 1;
		}
		if(havenodelist.count(e->to) != 1) {
			nodes.push_back(e->to);
			havenodelist[e->to] = 1;
		}
	}
	
	//if they wanted a list of nodes not used, give it to them.
	if(excess_nodes != NULL) {
		list<PathNode *>::iterator cur, end;
		cur = in_nodes.begin();
		end = in_nodes.end();
		for(; cur != end; cur++) {
			if(havenodelist.count(*cur) != 1)
				excess_nodes->push_back(*cur);
		}
	}
}

void cut_crossed_grids(PathGraph *big, map<PathEdge*, vector<GPoint> > &cross_list) {
	map<PathEdge*, vector<GPoint> >::iterator cur,end;
	vector<GPoint>::iterator curp, endp;
	
	cur = cross_list.begin();
	end = cross_list.end();
	for(; cur != end; cur++) {
		PathEdge *e = cur->first;
		vector<GPoint> &it = cur->second;
		
		sort(it.begin(), it.end());
		
		//if the first point is to the left of our from, reverse the ordering
		if(*e->from > it[0]) {
			reverse(it.begin(), it.end());
		}
		
		PathNode *last,*curn;
		last = e->from;
		
		curp = it.begin();
		endp = it.end();
		for(; curp != endp; curp++) {
			curn = new PathNode(*curp);
			big->nodes.push_back(curn);
			big->add_edge(last, curn);
			last = curn;
			cross_add_count++;
		}
		e->from = last;
	}
	
}

//written fast cause I dont care
bool load_eq_map(const char *zone, PathGraph *eqmap) {
	char buf[256];
	FILE *in;
	
	//try to locate the file
	sprintf(buf, "eqmaps/%s.txt", zone);
	in = fopen(buf, "r");
	if(in == NULL) {
		sprintf(buf, "eqmaps/%s_1.txt", zone);
		in = fopen(buf, "r");
		if(in == NULL) {
//			printf("Unable to load '%s'\n", buf);
			return(false);
		}
	}
	
	//assume that a map file is smaller than a meg
	char *file = new char[1024 * 1024];
	
	//read the whole thing in at once
	int len;
	len = fread(file, 1, 1024*1024, in);
	if(len == -1) {
		printf("Unable to read EQ map file.\n");
		return(false);
	}
	
	fclose(in);
	
	
	//start parsing it
	int pos = 0;
	char *start, *next;
	
	next = file;
	
	while(pos < len) {
		start = next;
		
		//find the end of the line
		for(; pos < len; pos++, next++) {
			if(*next == '\n' || *next == '\r')
				break;
		}
		if(pos >= len)
			break;
		*next = '\0';	//null therminate the 'start' string
		next++;
		pos++;
		//watch for goodl old windows line endings
		if(*next == '\n' || *next == '\r') { //could write past EOF, dont care, buffer is big
			next++;	//skip over it too.
			pos++;
		}
		
		//now start is our line, and next is in position, parse start.
		GPoint p1, p2, color;
		//apparently the map files allready use our inverted XY
		//try the first common format
		if(sscanf(start, "L %f, %f, %f, %f, %f, %f, %f, %f, %f",
			&p1.y, &p1.x, &p1.z, &p2.y, &p2.x, &p2.z, &color.y, &color.x, &color.z)
			== 9) {
			//do nothing, using if structure
		//try another format, not sure how sscanf handles white space
		} else if(sscanf(start, "L %f, %f, %f,  %f, %f, %f,  %f, %f, %f",
			&p1.y, &p1.x, &p1.z, &p2.y, &p2.x, &p2.z, &color.y, &color.x, &color.z)
			== 9) {
			//do nothing, using if structure
		} else {
			//cannot parse this line...
			continue;
		}
		
		//invert our XY, and +-
		float tmp;
		tmp = p1.x; p1.x = -p1.y; p1.y = -tmp;
		tmp = p2.x; p2.x = -p2.y; p2.y = -tmp;
		
		//color is prolly really integers
		
		//now we have our points...
		
		//sloppily add both nodes and the edge, prolly uses a lot more nodes than needed
		PathNode *n1, *n2;
		n1 = new PathNode(p1);
		n2 = new PathNode(p2);
		eqmap->nodes.push_back(n1);
		eqmap->nodes.push_back(n2);
		eqmap->add_edge(n1, n2);
	}
	
	delete[] file;
	
	return(true);
}


void write_eq_map(list<PathEdge *> &edges, const char *fname) {
	FILE *mapout;
	mapout = fopen(fname, "w");
	if(mapout == NULL) {
		printf("Unable to open EQ Client map %s\n", fname);
		return;
	}
	
	list<PathEdge*>::iterator cur,end;
	PathEdge *e;
	
	//draw our EQ map
	cur = edges.begin();
	end = edges.end();
	for(; cur != end; cur++) {
		e = *cur;
		
		GPoint p1(*e->from), p2(*e->to);
//		float tmp;
//		tmp = p1.x; p1.x = -p1.y; p1.y = -tmp;
//		tmp = p2.x; p2.x = -p2.y; p2.y = -tmp;
		p1.x = -p1.x; p1.y = -p1.y;
		p2.x = -p2.x; p2.y = -p2.y;
		
		fprintf(mapout, "L %.3f, %.3f, %.3f,  %.3f, %.3f, %.3f,  255, 165, 0\n",
			p1.x, p1.y, p1.z, p2.x, p2.y, p2.z);
		
	}
	
	fclose(mapout);
	
	printf("Wrote EQ Client map: %s\n", fname);
}

QTNode *build_quadtree(Map *map, PathGraph *big) {

	//TODO: make sure we have the right XY in here...
	QTNode *_root = new QTNode(map, FEAR_MAXIMUM_DISTANCE*FEAR_MAXIMUM_DISTANCE,
		map->GetMinX(), map->GetMaxX(), map->GetMinY(), map->GetMaxY());
	if(_root == NULL) {
		printf("Unable to allocate new QTNode.\n");
		return(false);
	}
	
	_root->nodes = big->nodes;	
			list<PathNode *>::iterator curs,end;
		curs = _root->nodes.begin();
		end = _root->nodes.end();
//int findex = 0;
		for(; curs != end; curs++) {
}
		
	_root->divideYourself(0);
	
	return(_root);
}

bool write_path_file(QTNode *_root, PathGraph *big, const char *file, vector< vector<PathEdge*> > &path_finding) {
	if(_root == NULL)
		return(false);
	
	//im too lazy to give reasons for errors
	FILE *out = fopen(file, "w");
	if(out == NULL) {
		printf("Unable to open output file '%s'.\n", file);
		return(false);
	}
	
	PathFile_Header head;
	head.version = PATHFILE_VERSION;
	head.node_count = big->nodes.size();
	head.link_count = big->edges.size() * 2;	//each edge has a to and from node
	head.qtnode_count = _root->countQTNodes();
	head.nodelist_count = _root->countPathNodes();
	
	if(fwrite(&head, sizeof(head), 1, out) != 1) {
		printf("Error writting path file header.\n");
		fclose(out);
		return(false);
		
	}
	
	printf("Path File: %lu fear nodes, %lu fear links, %u  QT nodes, %lu node lists\n", head.node_count, head.link_count, head.qtnode_count, head.nodelist_count);

	//build our blocks...
	PathNode_Struct *nodeBlock = new PathNode_Struct[head.node_count];
	PathLink_Struct *linkBlock = new PathLink_Struct[head.link_count];
	//too big to store right now, since we dont _NEED_ it...
//	PathNodeRef *reachability = new PathNodeRef[head.node_count*head.node_count/2];
	
	PathNode *n;
	PathEdge *e;
	list<PathEdge*>::iterator cur4,end4;
	list<PathNode *>::iterator cur, end;
	
	PathNode_Struct *curn = nodeBlock;
	PathLink_Struct *curl = linkBlock;
	
	cur = big->nodes.begin();
	end = big->nodes.end();
	
	//number all the nodes for a final time.
	int index = 0;
	for(; cur != end; cur++, index++) {
		(*cur)->node_id = index;
	}
	
	//maps to get edge list offsets for our pathing stuff
	std::map<PathEdge *, PathLinkOffsetRef> to_edges;
	std::map<PathEdge *, PathLinkOffsetRef> from_edges;
	
	uint32 eoffset = 0;
	//fill the node block and edge block,  N*E complexity
	cur = big->nodes.begin();
	end = big->nodes.end();
int nn = 0;
	for(; cur != end; cur++, curn++, nn++) {
		n = *cur;
		if(n == NULL) {
			printf("Got NULL node building quadtree, WTF.");
			continue;
		}
		curn->x = n->x;
		curn->y = n->y;
		curn->z = n->z;
		curn->link_offset = eoffset;
		curn->distance = n->longest_path;
		
		int ecount = 0;
		cur4 = big->edges.begin();
		end4 = big->edges.end();
		for(; cur4 != end4; cur4++, curl++) {
			e = *cur4;
			if(e->from == n) {
//using this instead of pointer addition because it is giving me some
//strange ass results by doing curl++, like adding 1242 instead of 6...
				curl = &linkBlock[eoffset];
				curl->dest_node = e->to->node_id;
				curl->reach = e->normal_reach;
				from_edges[e] = ecount;
				ecount++;
				eoffset++;
//				curl++;
			} else if(e->to == n) {
				curl = &linkBlock[eoffset];
				curl->dest_node = e->from->node_id;
				curl->reach = e->reverse_reach;
				to_edges[e] = ecount;
				ecount++;
				eoffset++;
//				curl++;
			}
		}
		if(ecount >= PATH_LINK_OFFSET_NONE) {
			printf("ERROR: a node has more than %d links, number will be truncated!", PATH_LINK_OFFSET_NONE-1);
		}
		curn->link_count = ecount;
	}
	
	//write vertexBlock
	if(fwrite(nodeBlock, sizeof(PathNode_Struct), head.node_count, out) != head.node_count) {
		printf("Error writting path file nodes.\n");
		fclose(out);
		return(false);
		
	}
	
	//write faceBlock
	if(fwrite(linkBlock, sizeof(PathLink_Struct), head.link_count, out) != head.link_count) {
		printf("Error writting path file edges.\n");
		fclose(out);
		return(false);
	}
	
	delete[] nodeBlock;
	delete[] linkBlock;
	
	//make our node blocks to write out...
	PathTree_Struct *qtnodes = new PathTree_Struct[head.qtnode_count];
	PathPointRef *nodelist = new PathPointRef[head.nodelist_count];
	if(qtnodes == NULL || nodelist == NULL) {
		printf("Error allocating temporary memory for output.\n");
		fclose(out);
		return(false);
	}
	
	//extract the quad tree structure into linear arrays
	unsigned long hindex = 0;
	unsigned long findex = 0;
	_root->fillBlocks(qtnodes, nodelist, hindex, findex);
	
	if(fwrite(qtnodes, sizeof(PathTree_Struct), head.qtnode_count, out) != head.qtnode_count) {
		printf("Error writting path file quadtree nodes.\n");
		fclose(out);
		return(false);
	}
	if(fwrite(nodelist, sizeof(PathPointRef), head.nodelist_count, out) != head.nodelist_count) {
		printf("Error writting path file nodelist.\n");
		fclose(out);
		return(false);
	}
	
	delete[] qtnodes;
	delete[] nodelist;
	
	//assumes that path_finding is not empty
	PathLinkOffsetRef *refs = new PathLinkOffsetRef[head.node_count];
	//finally make our path finding blocks.
	std::map<PathEdge *, PathLinkOffsetRef>::iterator rese, ende;
	vector< vector<PathEdge*> >::iterator curoe, endoe;
	vector<PathEdge*>::iterator curie, endie;
	curoe = path_finding.begin();
	endoe = path_finding.end();
	int o,i;
	for(i = 0; curoe != endoe; curoe++, i++) {
		curie = (*curoe).begin();
		endie = (*curoe).end();
		for(o = 0; curie != endie; curie++, o++) {
			e = *curie;
			if(e == NULL) {	//not reachable
				refs[o] = PATH_LINK_OFFSET_NONE;
				continue;
			}
			if(e->from->node_id == i) {
				rese = to_edges.find(e);
				ende = to_edges.end();
			} else {	//assume to == i
				rese = from_edges.find(e);
				ende = from_edges.end();
			}
			if(rese == ende) {
				//not found in map... should this happen?
				refs[o] = PATH_LINK_OFFSET_NONE;
				continue;
			}
			refs[o] = rese->second;
		}
		
		if(fwrite(refs, sizeof(PathLinkOffsetRef), head.node_count, out) != head.node_count) {
			printf("Error writting path file's pathing information for node %d/%d.\n", i, head.node_count);
			fclose(out);
			return(false);
		}
	}
	delete[] refs;
	
	fclose(out);
	
	return(true);
}


/*
void edge_stats(list<PathEdge *> &edges, const char *s) {
	list<PathEdge*>::iterator cur,end;
	cur = edges.begin();
	end = edges.end();
	
	printf("Long edges for %s:\n", s);
	
	float watchlen = 1000*1000;
	
	for(; cur != end; cur++) {
		PathEdge *e = *cur;
		float d2 = e->from->Dist2(e->to);
		if(d2 > watchlen) {
			printf("LL (%.3f,%.3f,%.3f) -> (%.3f,%.3f,%.3f) d2=%.3f\n", e->from->x, e->from->y, e->from->z, e->to->x, e->to->y, e->to->z, d2);			
		}
	}
}*/

void find_node_edges(PathGraph *big,
	std::map<PathNode*, vector<PathEdge*> > &node_edges) {
	list<PathEdge*>::iterator curE,endE;
	PathEdge *e;
	
	curE = big->edges.begin();
	endE = big->edges.end();
	for(; curE != endE; curE++) {
		e = *curE;
		
		if(node_edges.count(e->from) == 1) {
			node_edges[e->from].push_back(e);
		} else {
			vector<PathEdge*> t(1);
			t[0] = e;
			node_edges[e->from] = t;
		}
		
		if(node_edges.count(e->to) == 1) {
			node_edges[e->to].push_back(e);
		} else {
			vector<PathEdge*> t(1);
			t[0] = e;
			node_edges[e->to] = t;
		}
	}
}

void validate_edges(Map *map, PathGraph *big) {
	list<PathEdge*>::iterator curE,endE;
	PathEdge *e;
	
	curE = big->edges.begin();
	endE = big->edges.end();
	int r;
	for(r = 0; curE != endE; curE++, r++) {
		e = *curE;
		
		if(!CheckLOS(map, e->from, e->to)) {
			printf("Edge %d does not have LOS. Between nodes %d and %d.\n", r, e->from->node_id, e->to->node_id);
			e->valid = false;
		}
	}
}










