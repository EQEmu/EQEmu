/*

Fear Pathing generation utility.
(c) 2005 Father Nitwit



Settings table:

CREATE TABLE fear_settings (
	zone VARCHAR(16) NOT NULL PRIMARY KEY,
	
	#general settings:
	use_doors TINYINT NOT NULL DEFAULT 1,
	min_fix_z FLOAT NOT NULL DEFAULT 20,
	max_fear_distance FLOAT NOT NULL DEFAULT 250,
	image_scale TINYINT NOT NULL DEFAULT 4,
	
	#path related
	check_initial_los TINYINT NOT NULL DEFAULT 0,
	split_invalid_paths TINYINT NOT NULL DEFAULT 0,
	link_path_endpoints TINYINT NOT NULL DEFAULT 1,
	end_distance FLOAT NOT NULL DEFAULT 25,
	split_long_min FLOAT NOT NULL DEFAULT 300,
	split_long_step FLOAT NOT NULL DEFAULT 200,
	
	#node combining settings:
	same_dist FLOAT NOT NULL DEFAULT 2.5,
	node_combine_dist FLOAT NOT NULL DEFAULT 30,
	grid_combine_dist FLOAT NOT NULL DEFAULT 30,
	close_all_los TINYINT NOT NULL DEFAULT 0,
	
	#line-crossing reduction settings:
	cross_count INT NOT NULL DEFAULT 5,
	cross_min_length FLOAT NOT NULL DEFAULT 1,
	cross_max_z_diff FLOAT NOT NULL DEFAULT 20,
	cross_combine_dist FLOAT NOT NULL DEFAULT 120,
	
	#linking:
	second_link_dist FLOAT NOT NULL DEFAULT 100,
	link_max_dist FLOAT NOT NULL DEFAULT 400,
	link_count TINYINT NOT NULL DEFAULT 1
	
);

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




//parameters:
bool INCLUDE_DOORS = true;
float FEAR_MAXIMUM_DISTANCE = 250;
float ENDPOINT_CONNECT_MAX_DISTANCE = 25;
float MIN_FIX_Z = 20.0f;
float CLOSE_ENOUGH = 2.5;
bool COMBINE_CHECK_ALL_LOS = false;
float CLOSE_ENOUGH_COMBINE = 30;	//30;
float SPAWN_MIN_SECOND_DIST = 100;
bool SPLIT_INVALID_PATHS = false;
bool LINK_PATH_ENDPOINTS = true;
float MAX_LINK_SPAWN_DIST = 400;
float FINAL_LINK_POINTS_DIST = 30;
bool SPAWN_LINK_TWICE = true;
bool SPAWN_LINK_THRICE = true;
float MERGE_MIN_SECOND_DIST = 30;
float SPLIT_LINE_LENGTH = 300;
float SPLIT_LINE_INTERVAL = 200;
float LONG_PATH_CHECK_LOS = 0;   //0=disable
int CROSS_REDUCE_COUNT = 5;
float CROSS_MIN_LENGTH = 1;
float CROSS_MAX_Z_DIFF = 20;
float CLOSE_ENOUGH_CROSS = 120;
int IMAGE_SCALE = 4;




int main(int argc, char *argv[]) {
	
	srand(2038833498);
	
/*	const char *zone = 
		//"qeynos";
		"qeynos2";
		//"northkarana";
		*/
	if(argc != 2) {
		printf("Usage: %s [zone_short_name]\n", argv[0]);
		return(1);
	}
	const char *zone = argv[1];
	char buf[256];
	
	MYSQL m;
	
	list<PathGraph *> db_paths;
	list<PathNode *> db_spawns;

	mysql_init(&m);
	
	if(!mysql_real_connect(&m, DB_HOST, DB_LOGIN, DB_PASSWORD, DB_NAME, 0, NULL, 0)) {
		printf("Unable to connect: %s.\n", mysql_error(&m));
		return(1);
	}
	
	/*
		Data loading phase
	*/
	
	//load up our map file
	Map *map = Map::LoadMapfile(zone);
	if(map == NULL) {
		printf("Unable to load map file.");
		return(1);
	}
	
	//try to load the EQ map file to make our pictures prettier
	PathGraph eqmap;
	if(load_eq_map(zone, &eqmap)) {
		printf("Loaded EQ Client map: %d edges.\n", eqmap.edges.size());
	} else {
		printf("Unable to load EQ Client map, continuing without it.\n");
	}
	
	//load our crap from the DB...
	if(!load_paths_from_db(&m, map, zone, db_paths, db_spawns))
		return(1);
	
	if(db_paths.size() == 0)
		db_paths.push_back(new PathGraph());
	
	if(!load_spawns_from_db(&m, zone, db_spawns))
		return(1);

	if(INCLUDE_DOORS) {
		if(!load_doors_from_db(&m, zone, db_spawns))
			return(1);
	}
	
	if(!load_hints_from_db(&m, zone, db_spawns))
		return(1);

	
	/*{
		PathGraph *g;
		PathNode *cur = NULL, *last = NULL;
		g = new PathGraph();
		g->nodes.push_back(cur = new PathNode(200, 200, 5));
		last = cur;
		g->nodes.push_back(cur = new PathNode(GPoint(700, 200, 5)));
		g->add_edge(last, cur); last = cur;
		g->nodes.push_back(cur = new PathNode(GPoint(700, -300, 5)));
		g->add_edge(last, cur); last = cur;
		g->nodes.push_back(cur = new PathNode(GPoint(200, -300, 5)));
		g->add_edge(last, cur); last = cur;
		g->nodes.push_back(cur = new PathNode(GPoint(200, 200, 5)));
		g->add_edge(last, cur); last = cur;
		db_paths.push_back(g);
	}*/
	/*{
		PathGraph *g;
		PathNode *cur = NULL, *last = NULL;
		g = new PathGraph();
		g->nodes.push_back(cur = new PathNode(200, 200, 5));
		last = cur;
		g->nodes.push_back(cur = new PathNode(GPoint(385, 35, 23)));
		g->add_edge(last, cur); last = cur;
		g->nodes.push_back(cur = new PathNode(GPoint(450, -50, 23)));
		g->add_edge(last, cur); last = cur;
		g->nodes.push_back(cur = new PathNode(GPoint(535, -115, 23)));
		g->add_edge(last, cur); last = cur;
		g->nodes.push_back(cur = new PathNode(GPoint(700, -300, 5)));
		g->add_edge(last, cur); last = cur;
		db_paths.push_back(g);
		
		g = new PathGraph();
		g->nodes.push_back(cur = new PathNode(700, 200, 5));
		last = cur;
		g->nodes.push_back(cur = new PathNode(GPoint(535, 35, 23)));
		g->add_edge(last, cur); last = cur;
		g->nodes.push_back(cur = new PathNode(GPoint(450, -50, 23)));
		g->add_edge(last, cur); last = cur;
		g->nodes.push_back(cur = new PathNode(GPoint(385, -115, 23)));
		g->add_edge(last, cur); last = cur;
		g->nodes.push_back(cur = new PathNode(GPoint(200, -300, 5)));
		g->add_edge(last, cur); last = cur;
		db_paths.push_back(g);
	}*/
	
	
	//try to load settings, dont care if it fails
	if(load_settings_from_db(&m, zone))
		printf("Loaded zone settings from the database.\n");
	else
		printf("Unable to load settings from database. Using defaults.\n");
	
	
	printf("Load: got %d paths and %d spawn points from the database.\n", db_paths.size(), db_spawns.size());
	printf("Load: had to split up %d invalid paths.\n", load_split_paths);
	
	/*
		The make-the-db-suck-less phase
	*/
	
	//try to lower waypoints way in the sky:
	repair_high_waypoints(map, db_paths, db_spawns);
	printf("Fix Z: %d missed map, %d were not broken, %d were fixed.\n", z_no_map_count, z_not_fixed_count, z_fixed_count);
	printf("Fix Z: broken avg diff=%.3f, not broken avg diff=%.3f\n", z_fixed_diffs/z_fixed_count, z_not_fixed_diffs/z_not_fixed_count);
	
	//run a waypoint reduction algorithm in 3space:
	reduce_waypoints(db_paths);
	printf("WP Reduce: removed %d redundant waypoints.\n", wp_reduce_count);
	
	/*
		Graph connection and merging phase
	*/
	//make trivial connections of nodes at about the same spot on diff grids
	combine_trivial_grids(map, db_paths);
	printf("Trivial Merge: %d grids merged.\n", trivial_merge_count);
	
	//now do the 'closest with LOS' connection method
	combine_closest_grids(map, db_paths);
	printf("Closest Merge: %d grids linked, %d grids double-linked.\n", closest_merge_count, closest_merge2_count);
	PathGraph *big = db_paths.front();
	
	//now add in the spawn points, and link to closest with LOS
	link_spawns(map, big, db_spawns, MAX_LINK_SPAWN_DIST, NULL);
	printf("Link Spawns: %d linked once, %d linked twice, %d not linked, %d invalid.\n", link_spawn_count, link_spawn2_count, link_spawn_nocount, link_spawn_invalid);
	
	//combining close points might be causing small LOS obstacles... 
	//so we want to run this before we combine them.
	//this seems to do more harm than good right now
//	check_edge_los(map, big);
//	printf("Bad Edges: removed %d no-LOS edges.\n", removed_edges_los);
	
#ifdef LONG_PATH_CHECK_LOS
	//check long paths LOS, we seem to have a problem with random long links
	//disable this if we check all edges above...
	check_long_edge_los(map, big);
	printf("Bad Edges: removed %d long no-LOS edges.\n", removed_long_edges_los);
#endif
	
	//clean up points close enough to eachother to be the same point.
	combine_grid_points(map, big, CLOSE_ENOUGH_COMBINE);
	printf("Point Combine: combined %d very close nodes (%d missed strict LOS).\n", combined_grid_points, combine_broke_los);
	printf("Point Combine: so far, %d LOS cache hits, %d LOS cache misses.\n", los_cache_hits, los_cache_misses);
	
	list<PathEdge *> all_edges = big->edges;
	printf("Big Graph: %d original nodes, %d original edges.\n", big->nodes.size(), big->edges.size());
	
#ifdef DRAW_PRETREE_GRAPH
	//draw out our graph before trimming
	sprintf(buf, "paths-%s-pretree.png", zone);
	draw_paths(map, big->edges, eqmap.edges, buf);
#endif
	
	
	/*
		Graph algorithm application (boost)
		run it on each disjoint graph
	*/
	
	vector<PathGraph *> disjoints;
	vector<int> start_nodes;
	
	//find all the disjoint graphs
	{
		//build the boost graph
		MyGraph boost_graph(big->nodes.size());
		property_map<MyGraph, edge_weight_t>::type weightlist;
		std::map<PathEdge *, EdgeDesc> edgemap;
		build_boost_graph(boost_graph, weightlist, edgemap, big, false);
		
		//find the grid which has most of the edges
		sprintf(buf, "paths-%s-colors.png", zone);
		find_disjoint_grids(map, boost_graph, big, buf, start_nodes, disjoints);
	}
	printf("\nSplit: There are %d valid disjoint graphs.\n", disjoints.size());
	
	//for each disjoint graph....
	int djnum = 0;
	PathGraph *tmpg; int start_node;
	vector<PathGraph *>::iterator cur,end;
	vector<int>::iterator curs,ends;
	cur = disjoints.begin(); curs = start_nodes.begin();
	end = disjoints.end(); ends = start_nodes.end();
	for(; cur != end; cur++,curs++) {
		big = *cur;
		start_node = *curs;
		
		printf("Disjoint %d: has %d edges and %d nodes.\n", djnum, big->edges.size(), big->nodes.size());
		
		//reset our stats...
		combine_broke_los = 0;
		combined_grid_points = 0;
		removed_edges_los = 0;
		removed_long_edges_los = 0;
		broke_paths = 0;
		cross_edge_count = 0;
		cross_add_count = 0;
		
		{
			//build the boost graph
			MyGraph boost_graph(big->nodes.size());
			property_map<MyGraph, edge_weight_t>::type weightlist;
			std::map<PathEdge *, EdgeDesc> edgemap;
			build_boost_graph(boost_graph, weightlist, edgemap, big);
			
			//calculate the MST
			run_min_spanning_tree(boost_graph, weightlist, edgemap, big, start_node);
			printf("Ran Min Spanning Tree: ended with %d edges\n", big->edges.size());
		}

		/*
			Now we have our minimal spanning tree, try to refine it.
			
			the goal of this crap is to fix newbie fields and open zones
		*/
		std::map<PathEdge*, vector<GPoint> > cross_list;
		PathGraph *cross_big = new PathGraph();
		PathGraph *cross_excess = new PathGraph();
		
		//count the number of times each edge crosses another edge, and record
		//the intersection points. Also seperate crossers from non-crossers
		count_crossing_lines(big->edges, cross_big, cross_excess, cross_list);
		printf("Cross Count: %d edges cross more than the specified number of other edges.\n", cross_edge_count);
		
		if(cross_edge_count > 2) {
			//Make waypoints at all points of intersection
			cut_crossed_grids(cross_big, cross_list);
			printf("Cross Cut: Created %d new nodes cutting intersections\n", cross_add_count);
			
			//combine close points with a somewhat big radius...
			combine_grid_points(map, cross_big, CLOSE_ENOUGH_CROSS);
			printf("Cross Combine: combined %d nodes.  (%d missed strict LOS)\n", combined_grid_points, combine_broke_los);
			printf("Cross Combine: so far, %d LOS cache hits, %d LOS cache misses.\n", los_cache_hits, los_cache_misses);
			
			//build our boost graph, so we can do reachability
			MyGraph cross_graph(cross_big->nodes.size());
			property_map<MyGraph, edge_weight_t>::type cross_weightlist;
			std::map<PathEdge *, EdgeDesc> cross_edgemap;
			build_boost_graph(cross_graph, cross_weightlist, cross_edgemap, cross_big, false);
			
			//isolate each disjoint graph and try to reduce it, gathering 
			//all non-cross points and edges while we are at it.
			sprintf(buf, "paths-%s-crosses.png", zone);
			consolidate_cross_graphs(map, cross_big, cross_excess, cross_graph, buf);
			
			//rebuild the big graph by merging cross_big and cross_excess
			//might be as simple as append the two arrays and run a combine on it.
			//leaks 'big'
			*cur = big = cross_excess;
			cross_excess->add_edges(cross_big->edges);
			rebuild_node_list(big->edges, big->nodes);
			
			//This is used to re-link the cross grids with the non-cross stuff
			combine_grid_points(map, big, CLOSE_ENOUGH_COMBINE);
			printf("Cross Merge: combined %d close nodes. (%d missed strict LOS)\n", combined_grid_points, combine_broke_los);
			printf("Cross Merge: so far, %d LOS cache hits, %d LOS cache misses.\n", los_cache_hits, los_cache_misses);
			
			//build yet another boost graph so we can run MST
			MyGraph cross_graph_final(big->nodes.size());
			property_map<MyGraph, edge_weight_t>::type cross_weightlist_final;
			std::map<PathEdge *, EdgeDesc> cross_edgemap_final;
			build_boost_graph(cross_graph_final, cross_weightlist_final, cross_edgemap_final, big);
			
			//run our MST to reduce the new cross-reduced graph
			run_min_spanning_tree(cross_graph_final, cross_weightlist_final, cross_edgemap_final, big, 0);
			printf("Ran Min Spanning Tree 2: ended with %d edges\n", big->edges.size());
		}	//end if there were some cross edges
		
		/*
			Final refinement phase, the graph has been reduced to our final
			form, this phase is for adding anything back in we might want
		*/

		//now that we reduced all our co-linear points, add a bunch back in for
		//long paths, so we have more points over space. Idea is that this way
		//we control how many colinear points there are, it isnt random
		//this counteracts any attempts to use line-crossing as a criteria for reduction
		//for some stupid reason, this just destroys the graph
	//	break_long_lines(db_paths);
	//	printf("WP Increase: created %d waypoints on long paths.\n", broke_paths);
		

		/*
		Things we might want to do:
	 - Try to create some cycles. Specifically large cycles.
	   - do this after reachability calculations
	   - use allready calculated reacahbility and distances, just adjust them as cycles added
	   - these will add more realism to the pathing
	   - might be implemented like this:
		 - run all pairs shortest path on the MST (is this a byproduct?)
		 - for each node N, for each other node K
		   - if path(N, K) is at least MIN_CYCLE_JOIN_PATH (to prevent making small cycles)
		   - and dist(N,K) is less than MAX_CYCLE_JOIN_DIST (do not want to invent long paths)
		   - and there is LOS from N to K
			 - connect N and K
			 - have to re-run all pairs again... that sucks
	   - another twist on the implementation would be to only look at paths
		 which were discarded by the MST. Or maybe run this first, then the other.
		 */
		 
		/*
			The final tree has been built, remove anything we dont need from
			it, and gather some information.
		*/
		
		//build a boost graph out of our minimal spanning tree.
		MyGraph boost_mst(big->nodes.size());
		property_map<MyGraph, edge_weight_t>::type weightlist_mst;
		std::map<PathEdge *, EdgeDesc> edgemap_mst;
		build_boost_graph(boost_mst, weightlist_mst, edgemap_mst, big, true);
		
		//determine the path lengths to all nodes from all others
		//including the longest path reachable by each node.
		//this also cleans up big by removing anything unreachable
	//	vector< vector<int> > AllPairs;
		sprintf(buf, "paths-%s-mstcolors%d.png", zone, djnum++);
	//	calc_path_lengths(map, boost_mst, big, AllPairs, buf);
		calc_path_lengths(map, boost_mst, big, edgemap_mst, buf);
		printf("Calculated the longest paths from each node.\n");
		printf("\n");
	}
	
	printf("Combining all disjoint graphs...\n");
	//now combine all our disjoint graphs into one big one...
	big = new PathGraph();
	cur = disjoints.begin();
	end = disjoints.end();
	big->nodes.clear();
	big->edges.clear();
	djnum = 1;
	for(; cur != end; cur++, djnum++) {
		tmpg = *cur;
		list<PathEdge *>::iterator cure,ende;
		cure = tmpg->edges.begin();
		ende = tmpg->edges.end();
		for(; cure != ende; cure++) {
			big->edges.push_back(*cure);
		}
//		sprintf(buf, "paths-%s-dj%d.png", zone, djnum-1);
//		just_color_the_damned_thing(map, tmpg, buf);
	}
	rebuild_node_list(big->edges, big->nodes, NULL);
	
	printf("Validating results...\n");
	validate_edges(map, big);
	
	//now we have our final node and edge set.
	//build a graph of all final nodes to find pathing info
//	MyGraph final(big->nodes.size());
//	property_map<MyGraph, edge_weight_t>::type weightlist_final;
//	std::map<PathEdge *, EdgeDesc> edgemap_final;
//	build_boost_graph(final, weightlist_final, edgemap_final, big, true);
	
//		sprintf(buf, "paths-%s-mstcolors%d-ppi.png", zone, djnum-1);
//		just_color_the_damned_thing(map, big, buf);
		
	vector< vector<PathEdge*> > path_finding;
	find_path_info(map, big, path_finding);
	printf("Calculated pathing information...\n");
		
	//write out a nice image of our MST
	sprintf(buf, "paths-%s-mstree.png", zone);
	draw_paths2(map, all_edges, big->edges, eqmap.edges, db_spawns, buf);
	
	//write out a map for inside the EQ client, not as pretty as the PNG
	sprintf(buf, "eqmaps_out/%s_2.txt", zone);
	write_eq_map(big->edges, buf);
	
	/*
		Build structures, and write out the path file.
	*/
	RDTSC_Timer t1, t2;
	QTNode *root;
	root = build_quadtree(map, big);
	if(root == NULL) {
		printf("Failed to build quadtree, quitting.\n");
		return(1);
	}
	
	t2.start();
	sprintf(buf, "%s.path", zone);
	if(!write_path_file(root, big, buf, path_finding)) {
		printf("Unable to write path file.\n");
		return(1);
	}
	
	printf("Everything completed successfully. %s.path has been generated.\n", zone);
	
/*

** now we have a minimal connected graph such that any mob can get anywhere
   by only a single path.



** now we have our final pathing grid. determine some useful info for each node.

look for dead ends/node preference:
 - at each node, sum up the length of all edges reachable by using that link
   - for each node as N
   -- for each edge leaving N as E
   ---- reset marks on all edges
   ---- mark all edges leaving N
   ---- perform a depth first search of all reachable nodes
   ------ Only traverse unmarked edges. Mark each edge as it is traversed.
   ---- unmark all edges leaving N, except E
   ---- sum the length of all marked edges
   ---- store this reachable length as a weight for this edge of this node.
   -- for each edge leaving N as E
   ---- determine highest edge weight in this node
   -- for each edge leaving N as E
   ---- count number of edges within RANDOM_WALK % of the highest node.
   -- sort the edge list for this node to have all random nodes first
   -- store the random walkable counter for this node
 


** Finally we have all the calculation

things we need to store:
nodes
edge lists
quadtree containing nodes

 - We do not really need to store edges which are not on the random walk
   list since the pathing code will never consider using them.

Node {
 x, y, z
 edge list pointer
 edge list length
 random walkable counter
}

edge list entry {
 ending node pointer
 reachable edge weight? (not used by pathing, might have other purpose)
}

quadtree node {
 minx, maxx, miny, maxy
 union {
  node pointers: q1, q2, q3, q4
  node list offset and length
 }
}




how to do fear pathing...
when a mob is feared, it uses the quadtree to find the node closest to it
that it can see to. The LOS requirement will make this more difficult, but feasible.

Once it has found its first node, the mob will set this as its waypoint and go there.

Once the mob reaches a node, it will look at the node's random counter (RC)
 if RC is 1, take the first edge and use its terminal as next waypoint
 if RC is >1, randomly choose an edge from 0 to RC-1, and walk that edge.
*/
	
	
	
	
	mysql_close(&m);
}






