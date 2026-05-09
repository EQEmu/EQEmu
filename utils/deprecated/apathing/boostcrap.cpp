/*

Fear Pathing generation utility.
(c) 2005 Father Nitwit

*/




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
#include <gd.h>








/*
	We assume this overwrites the edge array in big
	and does not free the old edges
*/
void build_boost_graph(MyGraph &vg, property_map<MyGraph, edge_weight_t>::type &weightmap, map<PathEdge *, EdgeDesc> &em, PathGraph *big, bool set_weights) {
	
	list<PathEdge*>::iterator cur,end;
	list<PathNode*>::iterator curn,endn;
	PathNode *n;
	PathEdge *e;
	
	//first we need to number our nodes...
	curn = big->nodes.begin();
	endn = big->nodes.end();
	int r = 0;
	for(; curn != endn; curn++) {
		n = *curn;
		n->node_id = r;
		r++;
	}
	
	typedef std::pair < int, int > E;	//our edge type
	
	weightmap = get(edge_weight, vg); 
	
	//now add all our edges to the graph
	cur = big->edges.begin();
	end = big->edges.end();
	for(r = 0; cur != end; cur++, r++) {
		e = *cur;
		if(e->from == e->to)
			continue;
		
//		printf("A %d/%d (%.3f,%.3f,%.3f) -> (%.3f,%.3f,%.3f) d=%.3f\n", r, big->edges.size(), e->from->x, e->from->y, e->from->z, e->to->x, e->to->y, e->to->z, e->from->Dist2(e->to));
		EdgeDesc ed;
		bool inserted;
		tie(ed, inserted) = add_edge(e->from->node_id, e->to->node_id, vg);
		if(set_weights)
			weightmap[ed] = int(e->from->Dist2(e->to));
		else
			weightmap[ed] = 1;
		em[e] = ed;
//		e->edge_id = ed;
	}
	//now we should have a nice happy undirected graph...
}
	
void run_min_spanning_tree(MyGraph &vg, property_map<MyGraph, edge_weight_t>::type &weightmap, map<PathEdge *, EdgeDesc> &em, PathGraph *big, int start_node) {
	int noedge = 0;
	list<PathEdge *> out_paths;
	
	vector < EdgeDesc > spanning_tree;
	
	kruskal_minimum_spanning_tree(vg, back_inserter(spanning_tree));
	
	vector < EdgeDesc >::iterator cur,end;
	list<PathEdge*>::iterator cure,ende;
	cur = spanning_tree.begin();
	end = spanning_tree.end();
	for(; cur != end; cur++) {
		//find the edge
		cure = big->edges.begin();
		ende = big->edges.end();
		for(; cure != ende; cure++) {
			if(em[*cure] == *cur) {
				out_paths.push_back(new PathEdge((*cure)->from, (*cure)->to));
				break;
			}
		}
	}
	noedge = big->edges.size() - out_paths.size() - 1;
	printf("Ran Min Spanning Tree: %d paths were eliminated.\n", noedge);
	
	
	/*
	//make our results list.
	vector < VertDesc > p(num_vertices(vg));
	
	//finally run the stupid algorithm.
	prim_minimum_spanning_tree(vg, &p[0], root_vertex(start_node));
	
	
	for (std::size_t i = 0; i != p.size(); ++i) {
		if (p[i] != i) {
			out_paths.push_back(new PathEdge(big->nodes[p[i]], big->nodes[i]));
		} else {
			//no edge here...
			noedge++;
		}
	}
	printf("Ran Min Spanning Tree: %d nodes were disconnected.\n", noedge);
	
	
	*/
	
	//now swap out our edge list to the MST.
	big->edges = out_paths;
}

void find_disjoint_grids(Map *map, MyGraph &vg, PathGraph *big, const char *fname, vector<int> &start_nodes, vector<PathGraph *> &disjoints) {
	
	vector< vector<int> > D;
	vector<int> counts;
	vector<int> disjoint_counts;
	vector<int> first_node;
	
	//color the graph and get us the info we need to do out job
	color_disjoint_graphs(big, vg, map, fname, D, counts, disjoint_counts, first_node);
	
	
	//find the biggest grid, that one gets to be included no matter what
	int best_graph = 0;
	int best_count = counts[0];
	unsigned int r;
	for(r = 1; r < counts.size(); r++) {
		if(best_count < counts[r]) {
			best_count = counts[r];
			best_graph = r;
		}
	}
	disjoint_counts[best_graph] = 1;
	
	
	//break up the graphs based on color if we want them.
	vector<int>::iterator ccur,djcur,fcur,cend;
	list<PathEdge*>::iterator cur,end;
	PathEdge *e;
	PathGraph *pg;
	ccur = counts.begin();
	djcur = disjoint_counts.begin();
	fcur = first_node.begin();
	cend = counts.end();
	int color = 0;
	for(; ccur != cend; ccur++, djcur++, fcur++, color++) {
		int count = *ccur;
		int dj = *djcur;
//		int fn = *fcur;
		
		if(dj < 1)
			continue;   //skip disjoint sets not marked for use.
		
		if(count < MIN_DISJOINT_NODES)
			continue;   //make sure we have a reasonable node count
		
		pg = new PathGraph();
		
		cur = big->edges.begin();
		end = big->edges.end();
		for(; cur != end; cur++) {
			e = *cur;
			if(e->from->color != e->to->color) {
				printf("Color Mismatch %d-%d: #%d(%.3f,%.3f,%.3f) -> #%d(%.3f,%.3f,%.3f)\n", e->from->color, e->to->color, e->from->node_id, e->from->x, e->from->y, e->from->z, e->to->node_id, e->to->x, e->to->y, e->to->z);
				//... what to do...
			}
			//just use the color of the from node
			if(e->from->color == color) {
				pg->edges.push_back(e);
			}
		}
		
		//get our list of nodes based on our edge list.
		rebuild_node_list(pg->edges, pg->nodes, NULL);
		
		disjoints.push_back(pg);
		start_nodes.push_back(1);   //each graph only contains its own nodes, so any node will work.
	}
	
	
	
	/*
	int best_graph = 0;
	int best_count = counts[0];
	unsigned int r;
	for(r = 1; r < counts.size(); r++) {
		if(best_count < counts[r]) {
			best_count = counts[r];
			best_graph = r;
		}
//		printf("Graph %d has %d edges\n", r, counts[r]);
	}
	
	start_node = first_node[best_graph];
	printf("Best sub-graph: chose #%d of %d, has %d nodes, and contains node %d\n", best_graph, counts.size()-1, best_count, start_node);
	//todo: eliminate other graphs...
	*/
	
/*	list<PathEdge*>::iterator cure,ende;
	PathEdge *e;
	cure = big->edges.begin();
	ende = big->edges.end();
	for(; cure != ende; cure++) {
		e = *cure;
		if(e->from->color != e->to->color) {
			printf("Color Mismatch %d-%d: #%d(%.3f,%.3f,%.3f) -> #%d(%.3f,%.3f,%.3f)\n", e->from->color, e->to->color, e->from->node_id, e->from->x, e->from->y, e->from->z, e->to->node_id, e->to->x, e->to->y, e->to->z);
		} else if(e->from->color != best_graph) {
			printf("Color %d: (%.3f,%.3f,%.3f) -> (%.3f,%.3f,%.3f)\n", e->from->color, e->from->x, e->from->y, e->from->z, e->to->x, e->to->y, e->to->z);
		}
	}
*/
}



static const int INT_LIMIT = (std::numeric_limits < int >::max)();

void color_disjoint_graphs(
	PathGraph *big,
	MyGraph &vg, 
	Map *map,
	const char *fname,

	vector< vector<int> > &D,		//output
	vector<int> &counts,			//output
	vector<int> &disjoint_counts,   //output
	vector<int> &first_node			//output
) {
	
	
	int count = big->nodes.size();
//	int r;
	
//	vector< vector<int> > D(count, vector<int>(count, INT_LIMIT));
//	vector<int> counts(1, 0);
//	vector<int> first_node(1, 0);
	
	//make sure everything is inited right...
	D.resize(0);
	D.resize(count, vector<int>(count, INT_LIMIT));
	counts.resize(1);
	counts[0] = 0;
	disjoint_counts.resize(1);
	disjoint_counts[0] = 0;
	first_node.resize(1);
	first_node[0] = 0;
	
	//make up a weight map with all 1s, done while building now
/*	property_map < MyGraph, edge_weight_t >::type w = get(edge_weight, vg);
	graph_traits < MyGraph >::edge_iterator e, e_end;
	for (boost::tie(e, e_end) = edges(vg); e != e_end; ++e)
		w[*e] = 1;*/
	
	
	johnson_all_pairs_shortest_paths(vg, D);
	
	list<PathNode*>::iterator cur,end,cur2;
	PathNode *n,*f;
	
	int cur_color = 1;
	int cc,djc;
	
	//clear node colors
	cur = big->nodes.begin();
	end = big->nodes.end();
	for(; cur != end; cur++) {
		n = *cur;
		n->color = 0;
	}
	
	
	//color the graph based on reachability, basically labeling subgraphs
	//this finds the number of nodes reachable from each node
	//which is used to pick the best disconnected graph in the tree.
	//its a series of wrong bullshit that forces us to do this, but it works
	cur = big->nodes.begin();
	end = big->nodes.end();
	for(; cur != end; cur++) {
		n = *cur;
		if(n->color != 0)
			continue;	//allready visited


		cc = 1;
		djc = 0;
		
		if(n->disjoint)
			djc++;
		
		n->color = cur_color;
		cur_color++;
		
		cur2 = cur;
		for(; cur2 != end; cur2++) {
			f = *cur2;
			if(f->color == 0 && D[n->node_id][f->node_id] != INT_LIMIT) {
				cc++;
				f->color = n->color;
			}
			if(f->disjoint)
				djc++;
		}
		counts.push_back(cc);
		disjoint_counts.push_back(djc);
		first_node.push_back(n->node_id);
	}
	
#ifdef DRAW_ALL_COLORS	

	if(fname != NULL) {
		printf("Drawing with %d seperate sub-graphs from %d nodes and %d edges\n", cur_color-1, big->nodes.size(), big->edges.size());
		
		FILE *pngout;
		pngout = fopen(fname, "wb");
		if(pngout == NULL) {
			printf("Unable to open %s\n", fname);
			return;
		}
		
		gdImagePtr im;
		int minx = int(map->GetMinX());
		int maxx = int(map->GetMaxX());
		int miny = int(map->GetMinY());
		int maxy = int(map->GetMaxY());
		
		im = gdImageCreate((maxx - minx)/IMAGE_SCALE, (maxy - miny)/IMAGE_SCALE);
	//	im = gdImageCreate(maxx - minx, maxy - miny);
		
		//allocate this first, to make it the BG color.
		/*int black =*/ gdImageColorAllocate(im, 0, 0, 0);
		
	//	int grey = gdImageColorAllocate(im, 100, 100, 100);
		
		int *clist = new int[cur_color];
		int r;
		for(r = 0; r < cur_color; r++) {
			clist[r] = gdImageColorAllocate(im, rand()%255, rand()%255, rand()%255);
		}
		
		{
			list<PathEdge*>::iterator cur,end;
			PathEdge *e;
			
			cur = big->edges.begin();
			end = big->edges.end();
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
				gdImageLine(im, x1, y1, x2, y2, clist[e->from->color]);
			}
		}
		delete[] clist;
		
		gdImagePng(im, pngout);
		gdImageDestroy(im);
		
		fclose(pngout);
		
		printf("Wrote image: %s\n", fname);
	}
#endif 	//DRAW_ALL_COLORS
}

void calc_path_lengths(Map *map, MyGraph &vg, PathGraph *big, map<PathEdge *, EdgeDesc> &em, const char *fname) {

	vector<int> counts;
	vector<int> disjoint_counts;
	vector<int> first_node;
	vector< vector<int> > D;
	list<PathNode*>::iterator cur,end;
	PathNode *n;
	
	/*
		Node distances:
		1. find the root node.
			 - find the longest path from each node to any other node
			 - the node with the shortest of these longest paths is the root
		2. record each node's distance from that root node
	*/
	
	//color the graph and get us the info we need to do out job
	color_disjoint_graphs(big, vg, map, fname, D, counts, disjoint_counts, first_node);
	
	
	vector<int>::iterator curp,endp;
	
	int shortest_node = 0;
	int shortest = 0xFFFFFF;
	int the_longest = 0;
	int longest_node = 0;
	
	//stores the longest path from each node
	vector<int> longest_dists(D.size(), 0);
	
	//find the node with the longest path of all, so we know what
	//tree we are trying to find the root of
	cur = big->nodes.begin();
	end = big->nodes.end();
	for(; cur != end; cur++) {
		n = *cur;
		vector<int> &cv = D[n->node_id];
		curp = cv.begin();
		endp = cv.end();
		int longest = 0;
		int discount = 0;
		for(; curp != endp; curp++) {
			if(*curp == INT_LIMIT) {
				discount++;
				continue;
			}
			if(*curp > longest)
				longest = *curp;
		}
		
		longest_dists[n->node_id] = longest;
		
		if(longest > the_longest) {
			the_longest = longest;
			longest_node = n->node_id;
		}
	}
	
	//find the node with the shortest, longest path
	//the idea is to locate the 'root' of the tree.
	cur = big->nodes.begin();
	end = big->nodes.end();
	for(; cur != end; cur++) {
		n = *cur;
		vector<int> &cv = D[n->node_id];
		if(cv[longest_node] == INT_LIMIT)
			continue;	//this node cannot reach the root
		
		int longest = longest_dists[n->node_id];
		//n->longest_path = longest;
		
		if(longest < shortest) {
			shortest = longest;
			shortest_node = n->node_id;
		}
	}
	
	//now we have our root, set each node to their distance from the root
	vector<int> &root_dists = D[shortest_node];
printf("The tree's root is %d\n", shortest_node);
	cur = big->nodes.begin();
	end = big->nodes.end();
	for(; cur != end; cur++) {
		n = *cur;
		if(n->node_id == shortest_node)
			n->longest_path = 0;
		else
			n->longest_path = root_dists[n->node_id];
	}
	
	
	
	
	
	/*
	  	the reachability is the number of nodes which we could possibly get to
	  	by traveling each direction across an edge.
	  
		to find reachability:
		loop through each edge e
			Remove that edge from the graph
			color_disjoint_graphs
			fc = count number of nodes with the same color as 'e->from'
			tc = count number of nodes with the same color as 'e->to'
			e->normal_reach = tc;
			e->reverse_reach = fc;
	*/
	property_map<MyGraph, edge_weight_t>::type weightmap;
	weightmap = get(edge_weight, vg);
	
	int tc,fc;
	int from_color, to_color;
	
	printf("Finding lengths (%d dots)", 1+big->edges.size()/10);
	int pos = 0;
	
	PathEdge *e;
	list<PathEdge*>::iterator cur4,end4;
	cur4 = big->edges.begin();
	end4 = big->edges.end();
	for(; cur4 != end4; cur4++, pos++) {
		if(pos % 10 == 0) {
			printf(".");
			fflush(stdout);
		}
		e = *cur4;
		
		//remove this edge from the boost graph..
		remove_edge(em[e], vg);
		
		//color the graph and get us the info we need to do our job
		//char out[64];
		//sprintf(out, "lengraph-%d.png", pos);
		color_disjoint_graphs(big, vg, map, 
			NULL, 
			D, counts, disjoint_counts, first_node);
		
		//add the edge back in
		EdgeDesc ed;
		bool inserted;
		tie(ed, inserted) = add_edge(e->from->node_id, e->to->node_id, vg);
		em[e] = ed;
		weightmap[em[e]] = int(e->from->Dist2(e->to));	//cause its a new edge
		
		//make sure this stupid thing worked.
		if(e->from->color == e->to->color) {
			printf("Cycle detected in MST... WTF?\n");
			e->normal_reach = -1;
			e->reverse_reach = -1;
			continue;
		}
		from_color = e->from->color;
		to_color = e->to->color;
		
		//count our crap.
		tc = 0;
		fc = 0;
		cur = big->nodes.begin();
		end = big->nodes.end();
		for(; cur != end; cur++) {
			n = *cur;
			if(n->color == to_color)
				tc++;
			else if(n->color == from_color)
				fc++;
		}
		
		//put it on our edge
		e->normal_reach = tc;
		e->reverse_reach = fc;
	}
	
	printf("\n");
	
	//re-color the graph, since we dicked with it above
	color_disjoint_graphs(big, vg, map, fname, D, counts, disjoint_counts, first_node);
	choose_biggest_graph(big, counts, first_node);
}



void just_color_the_damned_thing(Map *map, PathGraph *big, const char *fname) {
	MyGraph vg(big->nodes.size());
	property_map<MyGraph, edge_weight_t>::type weightlist_mst;
	std::map<PathEdge *, EdgeDesc> edgemap_mst;
	build_boost_graph(vg, weightlist_mst, edgemap_mst, big, true);
	
	vector<int> counts;
	vector<int> disjoint_counts;
	vector<int> first_node;
	vector< vector<int> > D;
	list<PathNode*>::iterator cur,end;
	
	color_disjoint_graphs(big, vg, map, fname, D, counts, disjoint_counts, first_node);
}

/*
void calc_path_lengths(Map *map, MyGraph &vg, PathGraph *big, vector< vector<int> > &D, const char *fname) {

//	vector< vector<int> > D;
	vector<int> counts;
	vector<int> first_node;
	
	
	choose_biggest_graph(big, counts, first_node);
}
*/


//assumes that the graph is freshly colored disjoint, and counts/first_node are results from it
void choose_biggest_graph(PathGraph *big, vector<int> &counts, vector<int> &first_node) {
	int best_graph = 0;
	int best_count = counts[0];
	unsigned int r;
	for(r = 1; r < counts.size(); r++) {
		if(best_count < counts[r]) {
			best_count = counts[r];
			best_graph = r;
		}
		printf("Graph %d has %d edges\n", r, counts[r]);
	}
	
	printf("Best Tree: Detected %d graphs in the MST, selected #%d\n", counts.size(), best_graph);
	
	list<PathNode*> new_nodes;
	list<PathEdge*> new_edges;
	
	//rebuild the node list to include only nodes which are in
	//the best graph, also only keeping the edges which connect them.
	std::map<PathNode *, int> havenodelist;
	list<PathEdge*>::iterator cur4,end4;
	cur4 = big->edges.begin();
	end4 = big->edges.end();
	for(; cur4 != end4; cur4++) {
		PathEdge *e = *cur4;
		
		//remove the edge if it is not the main color
		//this also causes the removal of the nodes if they
		//are not used in any other edges.
		if(e->from->color != e->to->color) {
printf("Miscolor.\n");
			continue;
		}
		if(e->from->color != best_graph) {
			continue;
		}
		if(e->to->color != best_graph) {
			continue;
		}
		
		new_edges.push_back(e);
		
		if(havenodelist.count(e->from) != 1) {
			e->from->final_id = new_nodes.size();
			new_nodes.push_back(e->from);
			havenodelist[e->from] = 1;
		}
		if(havenodelist.count(e->to) != 1) {
			e->to->final_id = new_nodes.size();
			new_nodes.push_back(e->to);
			havenodelist[e->to] = 1;
		}
	}
	
	printf("Best Tree: Removed %d nodes and %d edges which were disconnected.\n", 
		big->nodes.size() - new_nodes.size(), big->edges.size() - new_edges.size());
	big->nodes = new_nodes;
	big->edges = new_edges;
}

void consolidate_cross_graphs(Map *map, PathGraph *big, PathGraph *excess, MyGraph &cross_graph, const char *fname) {
	
	vector< vector<int> > D;
	vector<int> counts;
	vector<int> disjoint_counts;
	vector<int> first_node;
	
	//color the graph and get us the info we need to do our job
	color_disjoint_graphs(big, cross_graph, map, fname, D, counts, disjoint_counts, first_node);
}

void find_path_info(Map *map, MyGraph &vg, vector< vector<PathEdge*> > &path_finding, PathGraph *big) {
	//make sure our path finding vector is big enough.
	{
		int size = big->nodes.size();
		vector<PathEdge*> tmp(size, (PathEdge*)NULL);
		path_finding.resize(0);
		path_finding.resize(size, tmp);
	}
	
	vector< vector<int> > D;
	vector<int> counts;
	vector<int> disjoint_counts;
	vector<int> first_node;
	
	//color the graph and get us the info we need to do our job
	color_disjoint_graphs(big, vg, map, NULL, D, counts, disjoint_counts, first_node);
	
	//figure out what edges link to each node.
	std::map<PathNode*, vector<PathEdge*> > node_edges;
	find_node_edges(big, node_edges);
	
	
	//for each node, find best edge to reach each other node.
	list<PathNode*>::iterator cur,end;
	PathNode *n;
	vector<int>::iterator curp,endp;
	vector<PathEdge *>::iterator cure,ende;
	int r;
	cur = big->nodes.begin();
	end = big->nodes.end();
	for(; cur != end; cur++) {
		n = *cur;
		//get all our vector refs that we need since this is kinda expensive
		vector<int> &cv = D[n->node_id];	//distance array
		vector<PathEdge *> &pf = path_finding[n->node_id];	//result
		vector<PathEdge *> &el = node_edges[n];	//our edges
		curp = cv.begin();
		endp = cv.end();
		for(r = 0; curp != endp; curp++, r++) {
			if(n->node_id == r) {
				//this is the end of the path, we cannot take an edge to reach ourself
				pf[r] = NULL;
				continue;
			}
			int my_dist = *curp;
			if(my_dist == INT_MAX) {
				//this node is unreachable.
				pf[r] = NULL;
				continue;
			}
			
			//else, node r reachable from us, find best edge.
			cure = el.begin();
			ende = el.end();
//			int shortest;
			PathEdge *ce;
			PathNode *cn;
			//for each edge
			for(; cure != ende; cure++) {
				ce = *cure;
				//find the node other than ourself on the edge
				if(ce->from == n)
					cn = ce->to;
				else
					cn = ce->from;
				//see how far away this node is
				int cdist = D[cn->node_id][r];
				if(cdist < my_dist) {
					//found one which is closer... due to min span tree, there
					//should only be one path possible so just go with it.
					pf[r] = ce;
					break;
				}
			}
			//assume that this node got assigned.. next
if(pf[r] == NULL) {
printf("Node id %d was not able to find a good path to node %d\n", n->node_id, r);
}
		}
	}
}






