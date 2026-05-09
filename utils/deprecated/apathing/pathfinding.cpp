
//this is the path finding portion of the program.

#include "../common/types.h"
#include "../zone/map.h"
#include "../common/rdtsc.h"
#include "quadtree.h"
#include "apathing.h"
#include "boostcrap.h"
#include <stdio.h>
#include <mysql.h>
#include <stdlib.h>
#include <string.h>

#include <vector>
#include <map>
#include <string>
#include <algorithm>
using namespace std;

void find_path_info(Map *map, PathGraph *big, vector< vector<PathEdge*> > &path_finding) {
	//make sure our path finding vector is big enough, and all NULL
	{
		int size = big->nodes.size();
		vector<PathEdge*> tmp(size, (PathEdge*)NULL);
		path_finding.resize(0);
		path_finding.resize(size, tmp);
	}
	
	//take our minimal spanning tree and try to link every single
	//point in it like spawns are linked. The idea is to create
	//a large set of alternative paths and cycles.
	//get our list of nodes.
	list<PathNode *> all_points = big->nodes;
	big->nodes.clear();
	//make our current edge list
	std::map< pair<PathNode *, PathNode *>, bool > edgelist;
	list<PathEdge*>::iterator cure,ende,tmp;
	PathEdge *e;
	cure = big->edges.begin();
	ende = big->edges.end();
	PathNode *from;
	PathNode *to;
	for(; cure != ende; cure++) {
		e = *cure;
		from = e->from;
		to = e->to;
		//hack to make order on the ID not matter
		if(int32(from) < int32(to)) {
			PathNode *tmp = from;
			from = to;
			to = tmp;
		}
		pair<PathNode *, PathNode *> id(from, to);
		edgelist[id] = true;
	}
	//link up the points, making sure not to add edges allready in the MST
	link_spawns(map, big, all_points, FINAL_LINK_POINTS_DIST, &edgelist);
	
	//now we have our graph all connected up.
	
	/*
	disabled in favor of all-pairs shortest path.
	
	//get the list of edges leaving each node.
	std::map<PathNode*, vector<PathEdge*> > node_edges;
	find_node_edges(big, node_edges);
	
	//color every node 0
	list<PathNode *>::iterator curn, endn;
	curn = big->nodes.begin();
	endn = big->nodes.end();
	for(; curn != endn; curn++) {
		(*curn)->color = 0;
	}
	
	//for each node, run a breadth first search from each node to find the
	//shortest path to each node from each node. 
	*/
	
	
	//run all pairs shortest path so we have some information to
	//use for our metric in A*
	
	//build our boost graph with length weights
	MyGraph vg(big->nodes.size());
	property_map<MyGraph, edge_weight_t>::type weightlist;
	std::map<PathEdge *, EdgeDesc> edgemap;
	build_boost_graph(vg, weightlist, edgemap, big, true);   //set weights to distances
	
	
	vector< vector<int> > D;
	vector<int> counts;
	vector<int> disjoint_counts;
	vector<int> first_node;
	
	//color the graph and get us the info we need to do our job (D)
	color_disjoint_graphs(big, vg, map, NULL, D, counts, disjoint_counts, first_node);
	
	//figure out what edges link to each node.
	std::map<PathNode*, vector<PathEdge*> > node_edges;
	find_node_edges(big, node_edges);

	int cur_node_id;
	list<PathNode*>::iterator curn,endn;
	PathNode *n;
	vector<int>::iterator curp,endp;
	vector<PathEdge*>::iterator curev,endev;
	vector<PathEdge *>::iterator cur_res, end_res;
	
	curn = big->nodes.begin();
	endn = big->nodes.end();
	for(; curn != endn; curn++) {
		n = *curn;
		
		//get all our vector refs that we need since
		vector<int> &cv = D[n->node_id];	//distance array
		vector<PathEdge *> &pf = path_finding[n->node_id];	//result
		vector<PathEdge *> &el = node_edges[n];	//our edges
		
		//for each other node, find an edge which gets us closer to that node
		curp = cv.begin();
		endp = cv.end();
		cur_res = pf.begin();
		end_res = pf.end();
		for(cur_node_id = 0; curp != endp && cur_res != end_res; curp++, cur_node_id++, cur_res++) {
			int distance_to_cur_node = *curp;
			
			//if this ourself, we have no path basically.
			if(n->node_id == cur_node_id) {
				*cur_res = NULL;
				continue;
			}
			//if we cant reach them, why look
			if(distance_to_cur_node == INT_MAX) {
				*cur_res = NULL;
				continue;
			}
			
			//see which edge gets us closer
			int closest = distance_to_cur_node; //start with the distance between this node and the target node, we need something closer
			PathEdge *best_edge = NULL;
			curev = el.begin();
			endev = el.end();
			for(; curev != endev; curev++) {
				e = *curev;
				int this_dist;		//distance from the other end of this edge to the current target node
				if(e->from == n) {
					if(e->to->node_id == cur_node_id)
						this_dist = 0;  //this is the goal node
					else
						this_dist = D[e->to->node_id][cur_node_id];
				} else {	//assume e->to == n
					if(e->from->node_id == cur_node_id)
						this_dist = 0;  //this is the goal node
					else
						this_dist = D[e->from->node_id][cur_node_id];
				}
				if(this_dist == 0) {
					//this will be the best.
					best_edge = e;
					break;
				}
				if(this_dist == INT_MAX)
					continue;   //not reachable
				if(this_dist < closest) {
					closest = this_dist;
					best_edge = e;
				}
			}
			if(best_edge == NULL) {
//unable to find a path...
				printf("Should have been able to find a path from %d to %d, but couldent.\n", n->node_id, cur_node_id);
			}
			*cur_res = best_edge;
		}
	}



	
	/*
	we have our node edges, our all pairs shortest path
	our graphs, and our MST edge list.
	now we can run A*
	
	- our metric should use the all pairs shortest path to
	  determine the value for h.
	- We want to favor edges from the MST over newly created edges,
	  so we will give such edges a lower weight, maybe by 20%?
	
	
	
	*/
	
}
















