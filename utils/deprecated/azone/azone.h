#ifndef AZONE_H
#define AZONE_H

//pull in datatypes from zone's map.h
#include "../../zone/map.h"

/*

	Father Nitwit's zone to map file converter.

*/

#include <stdio.h>
#include <vector>
#include <map>
using namespace std;


#define COUNT_MACTHES 1

//this is the version number to put in the map header
#undef MAP_VERSION  //override this from map.h with our version
#define MAP_VERSION 0x01000000

//quadtree stopping criteria, comment any to disable them
#define MAX_QUADRENT_FACES 50	//if box has fewer than this, stop
#define MIN_QUADRENT_SIZE 100.0f	//if box has a dimention smaller than this, stop
#define MIN_QUADRENT_GAIN 0.3f	//minimum split ratio before stopping
#define MAX_QUADRENT_MISSES 2	//maximum number of quads which can miss their gains
								//1 or 2 make sense, others are less useful

//attepmt to trim the data a little
#define MAX_Z	3000.0		//seems to be a lot of points above this
				//if all points on poly are, kill it.
/*
 This is used for the recursive node structure
 unsigned shorts are adequate because, worst case
 even in a zone that is 6000x6000 with a small node
 size of 30x30, there are only 40000 nodes.
 
 quadrent definitions:
 quad 1 (nodes[0]):
 x>=0, y>=0
 quad 2 (nodes[1]):
 x<0, y>=0
 quad 3 (nodes[2]):
 x<0, y<0
 quad 4 (nodes[3]):
 x>=0, y<0
 
 */
#define MAX_POLY_VTX 24		//arbitrary, im too lazy to figure it out
					//cut a triangle at most 6 times....

struct POLYGON {
   VERTEX /*w[MAX_POLY_VTX], */c[MAX_POLY_VTX];
   int count;             // w is world points, c is for camera points
};

class GPoint {
public:
	GPoint();
	GPoint(VERTEX &v);
	GPoint(float x, float y, float z);
	
	inline void operator()(float nx, float ny, float nz) { x = nx; y = ny; z = nz; }
	
	GPoint cross(const GPoint &them) const;
	float dot3(const GPoint &them) const;
	
	float x;
	float y;
	float z;

};
GPoint operator-(const GPoint &v1, const GPoint &v2);

class GVector : public GPoint {
public:
	GVector();
	GVector(const GPoint &them);
	GVector(float x, float y, float z, float w = 1.0f);
	
	inline void operator()(float nx, float ny, float nz, float nw) { x = nx; y = ny; z = nz; W = nw; }
	float dot4(const GVector &them) const;
	float dot4(const GPoint &them) const;
	void normalize();
	float length();
	
	float W;
};

struct FaceRecord {
	FACE *face;
	unsigned long index;
};

class QTNode;

class QTBuilder {
public:
	QTBuilder();
	~QTBuilder();
	
	bool build(const char *shortname);
	bool build_eqg(const char *shortname);
	bool writeMap(const char *file);
	
	bool FaceInNode(const QTNode *q, const FACE *f);
protected:
	
	void AddFace(VERTEX &v1, VERTEX &v2, VERTEX &v3);
	
	int ClipPolygon(POLYGON *poly, GVector *plane);
	
	//dynamic during load
//	vector<VERTEX> _VertexList;
	vector<FACE> _FaceList;
	
	//static once loaded
//	unsigned long vertexCount;
	unsigned long faceCount;
//	VERTEX * vertexBlock;
	FACE * faceBlock;
	
	VERTEX tempvtx[MAX_POLY_VTX];
	
	QTNode *_root;
	
	static void NormalizeN(FACE *p);

#ifdef COUNT_MACTHES
	unsigned long gEasyMatches;
	unsigned long gEasyExcludes;
	unsigned long gHardMatches;
	unsigned long gHardExcludes;
#endif

};

//quadtree node container
class QTNode {
public:
	QTNode(QTBuilder *builder, float Tminx, float Tmaxx, float Tminy, float Tmaxy);
	~QTNode();
	
	void clearNodes();
	
	void doSplit();
	void divideYourself(int depth);
	
	void buildVertexes();

//	bool writeFile(FILE *out);
	
	unsigned long countNodes() const;
	unsigned long countFacelists() const;
	
	void fillBlocks(nodeHeader *heads, unsigned long *flist, unsigned long &hindex, unsigned long &findex);
	
	float minx;
	float miny;
	float maxx;
	float maxy;
	unsigned long nfaces;
	vector<FaceRecord> faces;
	
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
	
protected:
	QTBuilder *builder;
};


#endif

