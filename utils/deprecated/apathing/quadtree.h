#ifndef FPQUADTREE_H
#define FPQUADTREE_H

//pull in datatypes from zone's map.h
#include "../zone/pathing.h"
#include "gpoint.h"

/*

	Father Nitwit's Fear Pathing File Maker Thing

*/

#include <stdio.h>
#include <vector>
#include <list>
using namespace std;


#define COUNT_MACTHES 1

//this is the version number to put in the map header
#undef PATHFILE_VERSION  //override this from fearpath.h with our version
#define PATHFILE_VERSION 0x02000000

//quadtree stopping criteria, comment any to disable them
//you want to keep the nodes small since the fear space is very sparse
#define MAX_QUADRENT_NODES 4	//if box has fewer than this, stop
#define MIN_QUADRENT_SIZE 50.0f	//if box has a dimention smaller than this, stop
#define MIN_QUADRENT_GAIN 0.1f	//minimum split ratio before stopping
#define MAX_QUADRENT_MISSES 2	//maximum number of quads which can miss their gains
								//1 or 2 make sense, others are less useful

class PathNode;
class Map;

//quadtree node container
class QTNode {
public:
	QTNode(Map *_map, float _dist2, float Tminx, float Tmaxx, float Tminy, float Tmaxy);
	~QTNode();
	
	void clearNodes();
	
	void doSplit();
	void divideYourself(int depth);
	
	void buildVertexes();
	
	unsigned long countQTNodes() const;	//counts QT nodes
	unsigned long countPathNodes() const;	//counts PathNodes
	
	void fillBlocks(PathTree_Struct *heads, PathPointRef *flist, unsigned long &hindex, unsigned long &findex);
	
	float minx;
	float miny;
	float maxx;
	float maxy;
	unsigned long nnodes;
	list<PathNode *> nodes;
	
	bool IsInNode(const QTNode *n, const PathNode *o);
	
	/*
	quadrent definitions:
		quad 1 (node1):
		x>=0, y>=0
		quad 2 (node2):
		x<0, y>=0
		quad 3 (node3):
		x<0, y<0
		quad 4 (node4):
		x>=0, y<0
	*/
	QTNode *node1;
	QTNode *node2;
	QTNode *node3;
	QTNode *node4;
	GPoint v[8];
	bool final;
	
	Map *map;
	float search_dist2;
};


#endif

