/*

	Father Nitwit's Fear Pathing File Maker Thing
	Copyright (C) 2005 Father Nitwit (eqemu@8ass.com)
	
	I'll release thisunder the GPL, even though I hate the GPL.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.
  
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	
	  You should have received a copy of the GNU General Public License
	  along with this program; if not, write to the Free Software
	  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*

	This is a modified quadtree that stores a list of all
	nodes reachable within a defined distance from the bounds
	of that quadtree node. Therefor this distance should be
	the maximum distance used to search for a node.
	
	In case this distance is not big enough, we also store a
	complete list of reachable nodes at each quadtree level, so
	the root has all nodes for sure. This makes it possible to
	garuntee that we can find A node which is closest.

*/

#include "quadtree.h"
#include "apathing.h"
#include "../zone/map.h"

//#define SPLIT_DEBUG

QTNode::QTNode(Map *_map, float dist2, float Tminx, float Tmaxx, float Tminy, float Tmaxy) {
	node1 = NULL;
	node2 = NULL;
	node3 = NULL;
	node4 = NULL;
	minx = Tminx;
	maxx = Tmaxx;
	miny = Tminy;
	maxy = Tmaxy;
	map = _map;
	search_dist2 = dist2;	//this is a distance-squared
	final = false;
	buildVertexes();
	
}

QTNode::~QTNode() {
	clearNodes();
}

void QTNode::clearNodes() {
	if(node1 != NULL)
		delete node1;
	if(node2 != NULL)
		delete node2;
	if(node3 != NULL)
		delete node3;
	if(node4 != NULL)
		delete node4;
	node1 = NULL;
	node2 = NULL;
	node3 = NULL;
	node4 = NULL;
}


//assumes that both supplied arrays are big enough per countQTNodes/Facelists
void QTNode::fillBlocks(PathTree_Struct *heads, PathPointRef *flist, unsigned long &hindex, unsigned long &findex) {
	PathTree_Struct *head = &heads[hindex];
	hindex++;
	
	head->minx = minx;
	head->maxx = maxx;
	head->miny = miny;
	head->maxy = maxy;
	head->flags = 0;

	//rearranged to give all QT nodes a node list
	head->nodelist.count = nodes.size();
	head->nodelist.offset = findex;
	list<PathNode *>::iterator curs,end;
	curs = nodes.begin();
	end = nodes.end();
	for(; curs != end; curs++) {
		PathNode *cur = *curs;
		flist[findex] = cur->node_id;
		findex++;
	}
//	findex += head->nodes.count;


	if(final) {
		head->flags |= pathNodeFinal;
	} else {
		head->flags = 0;
		//branch node.
		
		if(node1 != NULL) {
			head->nodes[0] = hindex;
			node1->fillBlocks(heads, flist, hindex, findex);
		} else {
			head->nodes[0] = PATH_NODE_NONE;
		}
		if(node2 != NULL) {
			head->nodes[1] = hindex;
			node2->fillBlocks(heads, flist, hindex, findex);
		} else {
			head->nodes[1] = PATH_NODE_NONE;
		}
		if(node3 != NULL) {
			head->nodes[2] = hindex;
			node3->fillBlocks(heads, flist, hindex, findex);
		} else {
			head->nodes[2] = PATH_NODE_NONE;
		}
		if(node4 != NULL) {
			head->nodes[3] = hindex;
			node4->fillBlocks(heads, flist, hindex, findex);
		} else {
			head->nodes[3] = PATH_NODE_NONE;
		}
	}
}

unsigned long QTNode::countQTNodes() const {
	unsigned long c = 1;
	if(node1 != NULL)
		c += node1->countQTNodes();
	if(node2 != NULL)
		c += node2->countQTNodes();
	if(node3 != NULL)
		c += node3->countQTNodes();
	if(node4 != NULL)
		c += node4->countQTNodes();
	return(c);
}

/*unsigned long QTNode::countNodes() const {
	unsigned long c = nodes.size();
	if(node1 != NULL)
		c += node1->countNodes();
	if(node2 != NULL)
		c += node2->countNodes();
	if(node3 != NULL)
		c += node3->countNodes();
	if(node4 != NULL)
		c += node4->countNodes();
	return(c);
}*/

unsigned long QTNode::countPathNodes() const {
//	unsigned long c = final? nodes.size() : 0;
	unsigned long c = nodes.size();
	if(node1 != NULL)
		c += node1->countPathNodes();
	if(node2 != NULL)
		c += node2->countPathNodes();
	if(node3 != NULL)
		c += node3->countPathNodes();
	if(node4 != NULL)
		c += node4->countPathNodes();
	return(c);
}

void QTNode::divideYourself(int depth) {
//	printf("Dividing in box (%.2f -> %.2f, %.2f -> %.2f) at depth %d with %d nodes.\n", 
//		minx, maxx, miny, maxy, depth, nodes.size());
	
	unsigned long cc;
	cc = nodes.size();
#ifdef MAX_QUADRENT_NODES
	if(cc <= MAX_QUADRENT_NODES) {
#ifdef SPLIT_DEBUG
printf("Stopping (nodecount) on box (%.2f -> %.2f, %.2f -> %.2f) at depth %d with %d nodes.\n", 
		minx, maxx, miny, maxy, depth, cc);
#endif
		final = true;
		return;
	}
#endif
	
#ifdef MIN_QUADRENT_SIZE
	if((maxx - minx) < MIN_QUADRENT_SIZE || (maxy - miny) < MIN_QUADRENT_SIZE) {
#ifdef SPLIT_DEBUG
printf("Stopping on box (size) (%.2f -> %.2f, %.2f -> %.2f) at depth %d with %d nodes.\n", 
		minx, maxx, miny, maxy, depth, cc);
#endif
		final = true;
		return;
	}
#endif
	
	doSplit();
	
	//get counts on our split
	float c1, c2, c3, c4;
	c1 = node1? node1->nodes.size() : 0;
	c2 = node2? node2->nodes.size() : 0;
	c3 = node3? node3->nodes.size() : 0;
	c4 = node4? node4->nodes.size() : 0;
	
#ifdef MIN_QUADRENT_GAIN
	int miss = 0;
	float gain1 = 1.0 - c1 / cc;
	float gain2 = 1.0 - c2 / cc;
	float gain3 = 1.0 - c3 / cc;
	float gain4 = 1.0 - c4 / cc;
	
	//see how many missed the gain mark
	if(gain1 < MIN_QUADRENT_GAIN)
		miss++;
	if(gain2 < MIN_QUADRENT_GAIN)
		miss++;
	if(gain3 < MIN_QUADRENT_GAIN)
		miss++;
	if(gain4 < MIN_QUADRENT_GAIN)
		miss++;
	
	if(miss > MAX_QUADRENT_MISSES) {
#ifdef SPLIT_DEBUG
printf("Stopping (gain) on box (%.2f -> %.2f, %.2f -> %.2f) at depth %d with %d nodes.\n", 
		minx, maxx, miny, maxy, depth, cc);
#endif
		final = true;
		return;
	}
#endif
	
	
	//if all nodes pass through all quadrents, then we are done
	//partially obsoleted by gain test.
	if(c1 == c2 && c1 == c3 && c1 == c4) {
#ifdef SPLIT_DEBUG
printf("Stopping (empty) on box (%.2f -> %.2f, %.2f -> %.2f) at depth %d with %d nodes.\n", 
		minx, maxx, miny, maxy, depth, cc);
printf("Our counts: %.3f, %.3f, %.3f, %.3f\n", c1, c2, c3, c4);
#endif
		final = true;
		return;
	}
	
	//there are prolly some more intelligent stopping criteria...
	
	depth++;
	
	if(node1 != NULL)
		node1->divideYourself(depth);
	if(node2 != NULL)
		node2->divideYourself(depth);
	if(node3 != NULL)
		node3->divideYourself(depth);
	if(node4 != NULL)
		node4->divideYourself(depth);
	
	
}

void QTNode::buildVertexes() {

	v[0].x = v[1].x = v[2].x = v[3].x = minx;
	v[4].x = v[5].x = v[6].x = v[7].x = maxx;
	
	v[0].y = v[1].y = v[4].y = v[5].y = miny;
	v[2].y = v[3].y = v[6].y = v[7].y = maxy;
	
	v[0].z = v[3].z = v[4].z = v[7].z = -999999;
	v[1].z = v[2].z = v[5].z = v[6].z = 9999999;
}

bool QTNode::IsInNode(const QTNode *n, const PathNode *o) {
	if(		o->x >= n->minx && o->x < n->maxx
		&&	o->y >= n->miny && o->y < n->maxy )
		return(true);
	
	//well its not inside the node, so see if it is reachable from it

	//4 points of this node
	GPoint	pt1(n->minx, n->miny, 0),
			pt2(n->minx, n->maxy, 0),
			pt3(n->maxx, n->miny, 0),
			pt4(n->maxx, n->maxy, 0);
	if(		o->Dist2(&pt1) < search_dist2
		||	o->Dist2(&pt2) < search_dist2
		||	o->Dist2(&pt3) < search_dist2
		||	o->Dist2(&pt4) < search_dist2)
		return(true);
	
	//not inside, and not reachable...
	
	return(false);
}

void QTNode::doSplit() {
	
	
	//find midpoints...
	float midx = minx + (maxx - minx) / 2.0;
	float midy = miny + (maxy - miny) / 2.0;
	
	//ordering following definitions in map.h
	node1 = new QTNode(map, search_dist2, midx, maxx, midy, maxy);
	node2 = new QTNode(map, search_dist2, minx, midx, midy, maxy);
	node3 = new QTNode(map, search_dist2, minx, midx, miny, midy);
	node4 = new QTNode(map, search_dist2, midx, maxx, miny, midy);
	if(node1 == NULL || node2 == NULL || node3 == NULL || node4 == NULL) {
		printf("Error: unable to allocate new QTNode, giving up.\n");
		return;
	}
	
//	unsigned long l;
//	l = faces.size();
//	for(r = 0; r < l; r++) {
//		PathNode *cur = faces[r];
	list<PathNode *>::iterator curs,end;
	curs = nodes.begin();
	end = nodes.end();
	for(; curs != end; curs++) {
		PathNode *cur = *curs;
		if(IsInNode(node1, cur))
			node1->nodes.push_back(cur);
		if(IsInNode(node2, cur))
			node2->nodes.push_back(cur);
		if(IsInNode(node3, cur))
			node3->nodes.push_back(cur);
		if(IsInNode(node4, cur))
			node4->nodes.push_back(cur);
	}
	
	//clean up empty sets.
	if(node1->nodes.size() == 0) {
		delete node1;
		node1 = NULL;
	}
	if(node2->nodes.size() == 0) {
		delete node2;
		node2 = NULL;
	}
	if(node3->nodes.size() == 0) {
		delete node3;
		node3 = NULL;
	}
	if(node4->nodes.size() == 0) {
		delete node4;
		node4 = NULL;
	}
	
}



