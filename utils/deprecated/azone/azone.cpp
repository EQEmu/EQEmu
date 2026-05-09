/*

	Father Nitwit's Zone to map conversion program.
	Copyright (C) 2004 Father Nitwit (eqemu@8ass.com)
	
	This thing uses code from freaku, so whatever license that comes under
	is relavent, if you care.
	
	the rest of it is GPL, even though I hate the GPL.

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

typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef          short SHORT;
typedef unsigned long  DWORD;

#include <stdio.h>
#ifdef WIN32 //vc++ chokes here without this
#include <string>
#else
#include <string.h>
#endif
#include <math.h>
//#include "EQWldData.h"
#include "azone.h"
#include "types.h"
#include "s3d.h"
#include "wld.h"

#include "archive.hpp"
#include "pfs.hpp"

#include "file_loader.hpp"
#include "zon.hpp"
#include "ter.hpp"

//TODO: I am trimming faces for which all vertices are above MAX_Z
//	but I am not taking out the vertices to go with them.


//this un-commented works with my map.cpp code correctly.
//with both of my inverts there commented.
#define INVERSEXY 1

#include <vector>
#include <map>
using namespace std;

//#define SPLIT_DEBUG

#ifdef WIN32
#pragma comment( lib, "wsock32.lib" )
#endif

#ifndef MAX_QUADRENT_FACES
#ifndef MIN_QUADRENT_SIZE
#error Umm... your asking for trouble by turning off both stopping criteria
#endif
#endif

int main(int argc, char *argv[]) {
	
//	if(argc != 3) {
//		printf("Usage: %s (zone.wld) (out.map)\n", argv[0]);
	if(argc != 2) {
		printf("Usage: %s (zone short name)\n", argv[0]);
		return(1);
	}
	
	char bufm[250];
	
	sprintf(bufm, "%s.map", argv[1]);
	
	QTBuilder QT;
	
	if(!QT.build(argv[1]))
		return(1);
	
	if(!QT.writeMap(bufm))
		return(1);
	
	return(0);
}

QTBuilder::QTBuilder() {
	_root = NULL;
	
//	vertexCount = 0;
	faceCount = 0;
//	vertexBlock = NULL;
	faceBlock = NULL;
	
#ifdef COUNT_MACTHES
	gEasyMatches = 0;
	gEasyExcludes = 0;
	gHardMatches = 0;
	gHardExcludes = 0;
#endif
}

QTBuilder::~QTBuilder() {
	if(_root != NULL)
		delete _root;
	_root = NULL;
/*	if(vertexBlock != NULL)
		delete vertexBlock;
	vertexBlock = NULL;*/
	if(faceBlock != NULL)
		delete faceBlock;
	faceBlock = NULL;
}

bool QTBuilder::build(const char *shortname) {
//	char bufm[96];
	char bufs[96], bufw[96];
	
	//TODO: clean up a LOT of memory that the freaku code does not
	
//	sprintf(bufm, "%s.map", shortname);
	sprintf(bufs, "%s.s3d", shortname);
	sprintf(bufw, "%s.wld", shortname);
	
	ZoneMesh meshi;
	ZoneMesh *mesh = &meshi;
	s3d_object s3d/*, s3d_obj, s3d_chr*/;
	wld_object wld/*, wld2, wld_obj, wld_chr*/;
	unsigned char *buf;
  
	FILE *s3df = fopen(bufs, "rb");
	if(s3df == NULL) {
		//try EQG.
		if(!build_eqg(shortname)) {
			printf("Unable to open s3d file '%s'.\n", bufs);
			return(false);
		} else {
			return(true);
		}
	}

	printf("Loading %s...\n", bufs);
	
	
	S3D_Init(&s3d, s3df);
	S3D_GetFile(&s3d, bufw, &buf);
	WLD_Init(&wld, buf, &s3d, 1);
	WLD_GetZoneMesh(&wld, &meshi);
	
	long i;
	VERTEX v1, v2, v3;
	for(i = 0; i < mesh->polygonCount; ++i) {
#ifdef INVERSEXY
		v1.y = mesh->verti[mesh->poly[i]->v1]->x;
		v1.x = mesh->verti[mesh->poly[i]->v1]->y;
#else
		v1.x = mesh->verti[mesh->poly[i]->v1]->x;
		v1.y = mesh->verti[mesh->poly[i]->v1]->y;
#endif
		v1.z = mesh->verti[mesh->poly[i]->v1]->z;
#ifdef INVERSEXY
		v2.y = mesh->verti[mesh->poly[i]->v2]->x;
		v2.x = mesh->verti[mesh->poly[i]->v2]->y;
#else
		v2.x = mesh->verti[mesh->poly[i]->v2]->x;
		v2.y = mesh->verti[mesh->poly[i]->v2]->y;
#endif
		v2.z = mesh->verti[mesh->poly[i]->v2]->z;
#ifdef INVERSEXY
		v3.y = mesh->verti[mesh->poly[i]->v3]->x;
		v3.x = mesh->verti[mesh->poly[i]->v3]->y;
#else
		v3.x = mesh->verti[mesh->poly[i]->v3]->x;
		v3.y = mesh->verti[mesh->poly[i]->v3]->y;
#endif
		v3.z = mesh->verti[mesh->poly[i]->v3]->z;
		
		AddFace(v1, v2, v3);
	}

	printf("	There are %lu vertices and %u faces.\n", _FaceList.size()*3, _FaceList.size());
	
	unsigned long r;
	
//	vertexCount = _VertexList.size();
	faceCount = _FaceList.size();
	
	
/*	vertexBlock = new VERTEX[vertexCount];
	//im not going to assume I know vectors are stored in contiguous blocks
	for(r = 0; r < vertexCount; r++) {
		vertexBlock[r] = _VertexList[r];
	}*/
	
	faceBlock = new FACE[faceCount];
	//im not going to assume I know vectors are stored in contiguous blocks
	for(r = 0; r < faceCount; r++) {
		faceBlock[r] = _FaceList[r];
	}
	
	//build quad tree... prolly much slower than it needs to be.
	float minx, miny, maxx, maxy;
	minx = 1e12;
	miny = 1e12;
	maxx = -1e12;
	maxy = -1e12;
	
	//find our limits.
	for(r = 0; r < faceCount; r++) {
		//a bit of lazyness going on here...
		{
		VERTEX &v = faceBlock[r].a;
		if(v.x > maxx)
			maxx = v.x;
		if(v.x < minx)
			minx = v.x;
		if(v.y > maxy)
			maxy = v.y;
		if(v.y < miny)
			miny = v.y;
		}
		{
		VERTEX &v = faceBlock[r].b;
		if(v.x > maxx)
			maxx = v.x;
		if(v.x < minx)
			minx = v.x;
		if(v.y > maxy)
			maxy = v.y;
		if(v.y < miny)
			miny = v.y;
		}
		{
		VERTEX &v = faceBlock[r].c;
		if(v.x > maxx)
			maxx = v.x;
		if(v.x < minx)
			minx = v.x;
		if(v.y > maxy)
			maxy = v.y;
		if(v.y < miny)
			miny = v.y;
		}
	}
	
	printf("	Bounding box: %.2f < x < %.2f, %.2f < y < %.2f\n", minx, maxx, miny, maxy);
	printf("World file loaded.\n");

	printf("Building quadtree.\n");
		
	_root = new QTNode(this, minx, maxx, miny, maxy);
	if(_root == NULL) {
		printf("Unable to allocate new QTNode.\n");
		return(false);
	}
	
	//build our initial set of faces... all of them:
	FACE *faceptr = faceBlock;
	_root->faces.resize(faceCount);
	for(r = 0; r < faceCount; r++) {
		_root->faces[r].face = faceptr;
		_root->faces[r].index = r;
		faceptr++;
	}
/*	_root->faces.resize(faceCount);
	for(r = 0; r < faceCount; r++) {
		_root->faces[r].face = &faceBlock[r];	//this is kinda bad, we dont own this 
											//memory, so we shouldent be taking addrs...
		_root->faces[r].index = r;
	}*/
	
	_root->divideYourself(0);
	
	printf("Done building quad tree...\n");

#ifdef COUNT_MACTHES
	fprintf(stderr, "Match counters: %lu easy in, %lu easy out, %lu hard in, %lu hard out.\n", gEasyMatches, gEasyExcludes, gHardMatches, gHardExcludes);
#endif
	
	
	return(true);
}


bool QTBuilder::build_eqg(const char *shortname) {
//	char bufm[96];
	char bufs[96];
  Archive *archive;
  FileLoader *fileloader;
  Zone_Model *zm;
	
	sprintf(bufs, "%s.eqg", shortname);

	archive = new PFSLoader();
	FILE *fff = fopen(bufs, "rb");
	if(fff == NULL) {
		printf("Failed to open '%s'\n", bufs);
		return(false);
	}
  if(archive->Open(fff) == 0) {
	  printf("Unable to open eqg file '%s'.\n", bufs);
	  return(false);
  }
  
  fileloader = new ZonLoader();
  if(fileloader->Open(NULL, (char *) shortname, archive) == 0) {
	  printf("Error reading ZON from %s\n", bufs);
	  return(false);
  }
  zm = fileloader->model_data.zone_model;

	long i;
	VERTEX v1, v2, v3;
	for(i = 0; i < zm->poly_count; ++i) {
#ifdef INVERSEXY
		v1.y = zm->verts[zm->polys[i]->v1]->x;
		v1.x = zm->verts[zm->polys[i]->v1]->y;
#else
		v1.x = zm->verts[zm->polys[i]->v1]->x;
		v1.y = zm->verts[zm->polys[i]->v1]->y;
#endif
		v1.z = zm->verts[zm->polys[i]->v1]->z;
#ifdef INVERSEXY
		v2.y = zm->verts[zm->polys[i]->v2]->x;
		v2.x = zm->verts[zm->polys[i]->v2]->y;
#else
		v2.x = zm->verts[zm->polys[i]->v2]->x;
		v2.y = zm->verts[zm->polys[i]->v2]->y;
#endif
		v2.z = zm->verts[zm->polys[i]->v2]->z;
#ifdef INVERSEXY
		v3.y = zm->verts[zm->polys[i]->v3]->x;
		v3.x = zm->verts[zm->polys[i]->v3]->y;
#else
		v3.x = zm->verts[zm->polys[i]->v3]->x;
		v3.y = zm->verts[zm->polys[i]->v3]->y;
#endif
		v3.z = zm->verts[zm->polys[i]->v3]->z;
		
		AddFace(v1, v2, v3);
	}

	printf("	There are %lu vertices and %u faces.\n", _FaceList.size()*3, _FaceList.size());
	
	unsigned long r;
	
//	vertexCount = _VertexList.size();
	faceCount = _FaceList.size();
	
	
/*	vertexBlock = new VERTEX[vertexCount];
	//im not going to assume I know vectors are stored in contiguous blocks
	for(r = 0; r < vertexCount; r++) {
		vertexBlock[r] = _VertexList[r];
	}*/
	
	faceBlock = new FACE[faceCount];
	//im not going to assume I know vectors are stored in contiguous blocks
	for(r = 0; r < faceCount; r++) {
		faceBlock[r] = _FaceList[r];
	}
	
	//build quad tree... prolly much slower than it needs to be.
	float minx, miny, maxx, maxy;
	minx = 1e12;
	miny = 1e12;
	maxx = -1e12;
	maxy = -1e12;
	
	//find our limits.
	for(r = 0; r < faceCount; r++) {
		//a bit of lazyness going on here...
		{
		VERTEX &v = faceBlock[r].a;
		if(v.x > maxx)
			maxx = v.x;
		if(v.x < minx)
			minx = v.x;
		if(v.y > maxy)
			maxy = v.y;
		if(v.y < miny)
			miny = v.y;
		}
		{
		VERTEX &v = faceBlock[r].b;
		if(v.x > maxx)
			maxx = v.x;
		if(v.x < minx)
			minx = v.x;
		if(v.y > maxy)
			maxy = v.y;
		if(v.y < miny)
			miny = v.y;
		}
		{
		VERTEX &v = faceBlock[r].c;
		if(v.x > maxx)
			maxx = v.x;
		if(v.x < minx)
			minx = v.x;
		if(v.y > maxy)
			maxy = v.y;
		if(v.y < miny)
			miny = v.y;
		}
	}
	
	printf("	Bounding box: %.2f < x < %.2f, %.2f < y < %.2f\n", minx, maxx, miny, maxy);
	printf("World file loaded.\n");

	printf("Building quadtree.\n");
		
	_root = new QTNode(this, minx, maxx, miny, maxy);
	if(_root == NULL) {
		printf("Unable to allocate new QTNode.\n");
		return(false);
	}
	
	//build our initial set of faces... all of them:
	FACE *faceptr = faceBlock;
	_root->faces.resize(faceCount);
	for(r = 0; r < faceCount; r++) {
		_root->faces[r].face = faceptr;
		_root->faces[r].index = r;
		faceptr++;
	}
/*	_root->faces.resize(faceCount);
	for(r = 0; r < faceCount; r++) {
		_root->faces[r].face = &faceBlock[r];	//this is kinda bad, we dont own this 
											//memory, so we shouldent be taking addrs...
		_root->faces[r].index = r;
	}*/
	
	_root->divideYourself(0);
	
	printf("Done building quad tree...\n");

#ifdef COUNT_MACTHES
	fprintf(stderr, "Match counters: %lu easy in, %lu easy out, %lu hard in, %lu hard out.\n", gEasyMatches, gEasyExcludes, gHardMatches, gHardExcludes);
#endif
	
	
	return(true);
}

bool QTBuilder::writeMap(const char *file) {
	if(_root == NULL)
		return(false);
	
	printf("Writting map file.\n");
	
	//im too lazy to give reasons for errors
	FILE *out = fopen(file, "wb");
	if(out == NULL) {
		printf("Unable to open output file '%s'.\n", file);
		return(1);
	}
	
	mapHeader head;
	head.version = MAP_VERSION;
//	head.vertex_count = vertexCount;
	head.face_count = faceCount;
	head.node_count = _root->countNodes();
	head.facelist_count = _root->countFacelists();
	
	if(fwrite(&head, sizeof(head), 1, out) != 1) {
		printf("Error writting map file header.\n");
		fclose(out);
		return(1);
		
	}
	
	printf("	Map header: %lu faces, %u nodes, %lu facelists\n", head.face_count, head.node_count, head.facelist_count);

	
	//write vertexBlock
/*	if(fwrite(vertexBlock, sizeof(VERTEX), vertexCount, out) != vertexCount) {
		printf("Error writting map file vertices.\n");
		fclose(out);
		return(1);
		
	}*/
	
	//write faceBlock
	if(fwrite(faceBlock, sizeof(FACE), faceCount, out) != faceCount) {
		printf("Error writting map file faces.\n");
		fclose(out);
		return(1);
	}
	
	//make our node blocks to write out...
	nodeHeader *nodes = new nodeHeader[head.node_count];
	unsigned long *facelist = new unsigned long[head.facelist_count];
	if(nodes == NULL || facelist == NULL) {
		printf("Error allocating temporary memory for output.\n");
		fclose(out);
		return(1);  //no memory
	}
	
	unsigned long hindex = 0;
	unsigned long findex = 0;
	_root->fillBlocks(nodes, facelist, hindex, findex);
	
	if(fwrite(nodes, sizeof(nodeHeader), head.node_count, out) != head.node_count) {
		printf("Error writting map file nodes.\n");
		fclose(out);
		return(1);
	}
	if(fwrite(facelist, sizeof(unsigned long), head.facelist_count, out) != head.facelist_count) {
		printf("Error writting map file face list.\n");
		fclose(out);
		return(1);
	}
	
/*	if(!_root->writeFile(out)) {
		printf("Error writting map file quadtree nodes.\n");
		fclose(out);
		return(1);
	}*/
	
	fclose(out);
	delete[] nodes;
	delete[] facelist;
	
	printf("Done writting map.\n");
	
	return(0);
}


QTNode::QTNode(QTBuilder *b, float Tminx, float Tmaxx, float Tminy, float Tmaxy) {
	node1 = NULL;
	node2 = NULL;
	node3 = NULL;
	node4 = NULL;
	minx = Tminx;
	maxx = Tmaxx;
	miny = Tminy;
	maxy = Tmaxy;
	final = false;
	buildVertexes();
	
	builder = b;
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

//assumes that both supplied arrays are big enough per countNodes/Facelists
void QTNode::fillBlocks(nodeHeader *heads, unsigned long *flist, unsigned long &hindex, unsigned long &findex) {
	nodeHeader *head = &heads[hindex];
	hindex++;
	
	head->minx = minx;
	head->maxx = maxx;
	head->miny = miny;
	head->maxy = maxy;
	head->flags = 0;
	if(final) {
		head->flags |= nodeFinal;
		head->faces.count = faces.size();
		head->faces.offset = findex;
		unsigned long r;
		for(r = 0; r < head->faces.count; r++) {
			flist[findex] = faces[r].index;
			findex++;
		}
//		findex += head->faces.count;
	} else {
		//branch node.
		head->flags = 0;
		
		if(node1 != NULL) {
			head->nodes[0] = hindex;
			node1->fillBlocks(heads, flist, hindex, findex);
		} else {
			head->nodes[0] = NODE_NONE;
		}
		if(node2 != NULL) {
			head->nodes[1] = hindex;
			node2->fillBlocks(heads, flist, hindex, findex);
		} else {
			head->nodes[1] = NODE_NONE;
		}
		if(node3 != NULL) {
			head->nodes[2] = hindex;
			node3->fillBlocks(heads, flist, hindex, findex);
		} else {
			head->nodes[2] = NODE_NONE;
		}
		if(node4 != NULL) {
			head->nodes[3] = hindex;
			node4->fillBlocks(heads, flist, hindex, findex);
		} else {
			head->nodes[3] = NODE_NONE;
		}
	}
}

unsigned long QTNode::countNodes() const {
	unsigned long c = 1;
	if(node1 != NULL)
		c += node1->countNodes();
	if(node2 != NULL)
		c += node2->countNodes();
	if(node3 != NULL)
		c += node3->countNodes();
	if(node4 != NULL)
		c += node4->countNodes();
	return(c);
}

unsigned long QTNode::countFacelists() const {
	unsigned long c = final? faces.size() : 0;
	if(node1 != NULL)
		c += node1->countFacelists();
	if(node2 != NULL)
		c += node2->countFacelists();
	if(node3 != NULL)
		c += node3->countFacelists();
	if(node4 != NULL)
		c += node4->countFacelists();
	return(c);
}

/*
Map Format:

1x mapHeader (head)
head.face_count x FACE
head.node_count x nodeHeader
head.facelist_count x unsigned long (indexes into face array)
 
		
*/
/*bool QTNode::writeFile(FILE *out) {
	static nodeHeader head;
	head.minx = minx;
	head.maxx = maxx;
	head.miny = miny;
	head.maxy = maxy;
	
	if(final) {
		head.faces.count = faces.size();
		if(fwrite(&head, sizeof(nodeHeader), 1, out) != 1) {
			return(false);
		}
		
		unsigned long *fblock = new unsigned long[head.faces.count];
		if(fblock == NULL)
			return(false);
		unsigned long r;
		for(r = 0; r < head.faces.count; r++) {
			fblock[r] = faces[r].index;
		}
		if(fwrite(fblock, sizeof(unsigned long), head.faces.count, out) != head.faces.count) {
			return(false);
		}
		
		delete[] fblock;
		
		return(true);
	}
	
	//not final, write mask and tell children to write themselves...
	
	head.faces.count = 0;

	if(fwrite(&head, sizeof(nodeHeader), 1, out) != 1) {
		return(false);
	}
	if(fwrite(&mask, 1, 1, out) != 1) {
		return(false);
	}
	
	if(node1 != NULL) {
		if(!node1->writeFile(out))
			return(false);
	}
	
	if(node2 != NULL) {
		if(!node2->writeFile(out))
			return(false);
	}
	
	if(node3 != NULL) {
		if(!node3->writeFile(out))
			return(false);
	}
	
	if(node4 != NULL) {
		if(!node4->writeFile(out))
			return(false);
	}
	
	
	return(true);
}*/

void QTNode::divideYourself(int depth) {
//	printf("Dividing in box (%.2f -> %.2f, %.2f -> %.2f) at depth %d with %d faces.\n", 
//		minx, maxx, miny, maxy, depth, faces.size());
	
	unsigned long cc;
	cc = faces.size();
#ifdef MAX_QUADRENT_FACES
	if(cc <= MAX_QUADRENT_FACES) {
#ifdef SPLIT_DEBUG
printf("Stopping (facecount) on box (%.2f -> %.2f, %.2f -> %.2f) at depth %d with %d faces.\n", 
		minx, maxx, miny, maxy, depth, cc);
#endif
		final = true;
		return;
	}
#endif
	
#ifdef MIN_QUADRENT_SIZE
	if((maxx - minx) < MIN_QUADRENT_SIZE || (maxy - miny) < MIN_QUADRENT_SIZE) {
#ifdef SPLIT_DEBUG
printf("Stopping on box (size) (%.2f -> %.2f, %.2f -> %.2f) at depth %d with %d faces.\n", 
		minx, maxx, miny, maxy, depth, cc);
#endif
		final = true;
		return;
	}
#endif
	
	doSplit();
	
	//get counts on our split
	float c1, c2, c3, c4;
	c1 = node1? node1->faces.size() : 0;
	c2 = node2? node2->faces.size() : 0;
	c3 = node3? node3->faces.size() : 0;
	c4 = node4? node4->faces.size() : 0;
	
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
printf("Stopping (gain) on box (%.2f -> %.2f, %.2f -> %.2f) at depth %d with %d faces.\n", 
		minx, maxx, miny, maxy, depth, cc);
#endif
		final = true;
		return;
	}
#endif
	
	
	//if all faces pass through all quadrents, then we are done
	//partially obsoleted by gain test.
	if(c1 == c2 && c1 == c3 && c1 == c4) {
#ifdef SPLIT_DEBUG
printf("Stopping (empty) on box (%.2f -> %.2f, %.2f -> %.2f) at depth %d with %d faces.\n", 
		minx, maxx, miny, maxy, depth, cc);
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


static const GVector gNormals[6] = {
	GVector(-1.0, 0.0, 0.0),
	GVector(0.0, 1.0, 0.0),
	GVector(1.0, 0.0, 0.0),
	GVector(0.0, -1.0, 0.0),
	GVector(0.0, 0.0, 1.0),
	GVector(0.0, 0.0, -1.0),
};

static const unsigned short gIntFaces[6][4] =
{
	{0, 1, 2, 3},
	{3, 2, 6, 7},
	{7, 6, 5, 4},
	{4, 5, 1, 0},
	{5, 6, 2, 1},
	{7, 4, 0, 3}
};


//stolen in haste from my fear pathing program, and untested in here...
bool edges_cross(GPoint *pt1, GPoint *pt2, const VERTEX *pt3, const VERTEX *pt4) {
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
(p1->dim > p2->dim? (coord >= p2->dim && coord <= p1->dim) : (coord >= p1->dim && coord <= p2->dim))

#define IntersectZfromX(p1, p2, inter) \
 (p2->x > p1->x? \
 (p1->z + ((inter - p1->x)/(p2->x - p1->x) * (p2->z - p1->z))) \
 :(p2->z + ((inter - p2->x)/(p1->x - p2->x) * (p1->z - p2->z))))
	
	float denom = IntersectDenom(pt1, pt2, pt3, pt4);
	if(denom != 0) {

		//the lines intersect, check segments now
		float xinter = IntersectX(pt1, pt2, pt3, pt4, denom);
		float yinter = IntersectY(pt1, pt2, pt3, pt4, denom);

		//now see if this point is on both segments
		if(		CoordOnLine(pt1, pt2, xinter, x)
			&&	CoordOnLine(pt1, pt2, yinter, y)
			&&	CoordOnLine(pt3, pt4, xinter, x)
			&&	CoordOnLine(pt3, pt4, yinter, y) ){
//			printf("Line (%.3f,%.3f,%.3f) -> (%.3f,%.3f,%.3f) d=%.3f\n", pt1->x, pt1->y, pt1->z, pt2->x, pt2->y, pt2->z, pt1->Dist2(pt2));
//			printf("Hits (%.3f,%.3f,%.3f) -> (%.3f,%.3f,%.3f) d=%.3f\n", pt3->x, pt3->y, pt3->z, pt4->x, pt4->y, pt4->z, pt3->Dist2(pt4));
//			printf("At (%.3f, %.3f), which IS on both segments.\n", xinter, yinter);

			return(true);
		}
//		printf("At (%.3f, %.3f), which is not on both segments.\n", xinter, yinter);
	}
	return(false);
}

#define MAX(x,y) ((x)<y?(y):(x))
#define MIN(x,y) ((x)<y?(x):(y))

bool PointInTriangle(VERTEX *polygon, float px, float py)

/*
  This code is from http://local.wasp.uwa.edu.au/~pbourke/geometry/
  Copyright Paul Bourke
  */

{
  int counter = 0;
  int i;
  double xinters;
  VERTEX p1,p2;

  p1 = polygon[0];
  for (i=1;i<=3;i++) {
    p2 = polygon[i % 3];
    if (py > MIN(p1.y,p2.y)) {
      if (py <= MAX(p1.y,p2.y)) {
        if (px <= MAX(p1.x,p2.x)) {
          if (p1.y != p2.y) {
            xinters = (py-p1.y)*(p2.x-p1.x)/(p2.y-p1.y)+p1.x;
            if (p1.x == p2.x || px <= xinters)
              counter++;
          }
        }
      }
    }
    p1 = p2;
  }

  if (counter % 2 == 0)
    return(false);
  else
    return(true);
}

//quick function which got too messy in the loop below.
bool QTBuilder::FaceInNode(const QTNode *q, const FACE *f) {
	const VERTEX *v1 = &f->a;
	const VERTEX *v2 = &f->b;
	const VERTEX *v3 = &f->c;
	
#ifdef COUNT_MACTHES
	gEasyMatches++;
#endif

	//Easy matches, points are within the quadrant.
	if( ! ( v1->x <= q->minx || v1->x > q->maxx 
			|| v1->y <= q->miny || v1->y > q->maxy ) ) {
		return(true);
	}
	if( ! ( v2->x <= q->minx || v2->x > q->maxx 
			|| v2->y <= q->miny || v2->y > q->maxy ) ) {
		return(true);
	}
	if( ! ( v3->x <= q->minx || v3->x > q->maxx 
			|| v3->y <= q->miny || v3->y > q->maxy ) ) {
		return(true);
	}
	
#ifdef COUNT_MACTHES
	gEasyMatches--;
	gEasyExcludes++;
#endif

	//make sure it is even possible to insersect:
	if( v1->x < q->minx && v2->x < q->minx && v3->x < q->minx )
		return(false);
	if( v1->x > q->maxx && v2->x > q->maxx && v3->x > q->maxx )
		return(false);
	if( v1->y < q->miny && v2->y < q->miny && v3->y < q->miny )
		return(false);
	if( v1->y > q->maxy && v2->y > q->maxy && v3->y > q->maxy )
		return(false);
	
	
#ifdef COUNT_MACTHES
	gEasyExcludes--;
#endif
	
#ifdef COUNT_MACTHES
	gHardMatches++;
#endif
//	return(true);
	
	//harder: no points are in the cube
	
	//4 points of this node
	GPoint	pt1(q->minx, q->miny, 0),
			pt2(q->minx, q->maxy, 0),
			pt3(q->maxx, q->miny, 0),
			pt4(q->maxx, q->maxy, 0);
	
	/*
	//box lines:
	pt1, pt2
	pt3, pt4
	pt1, pt3
	pt2, pt4
	
	//tri lines
	v1, v2
	v1, v3
	v2, v3
	*/
	
#define CheckIntersect(p1, p2, p3, p4) \
(((p4->y - p3->y)*(p2.x - p1.x) - (p4->x - p3->x)*(p2.y - p1.y)) != 0)
	
	int finaltest =
		   (edges_cross(&pt1, &pt2, v1, v2)
		|| edges_cross(&pt1, &pt2, v1, v3)
		|| edges_cross(&pt1, &pt2, v2, v3)
		|| edges_cross(&pt3, &pt4, v1, v2)
		|| edges_cross(&pt3, &pt4, v1, v3)
		|| edges_cross(&pt3, &pt4, v2, v3)
		|| edges_cross(&pt1, &pt3, v1, v2)
		|| edges_cross(&pt1, &pt3, v1, v3)
		|| edges_cross(&pt1, &pt3, v2, v3)
		|| edges_cross(&pt2, &pt4, v1, v2)
		|| edges_cross(&pt2, &pt4, v1, v3)
		|| edges_cross(&pt2, &pt4, v2, v3));

	if(finaltest) return finaltest;

	VERTEX Triangle[3];
	Triangle[0]=*v1; Triangle[1]=*v2; Triangle[2] = *v3;
	finaltest = PointInTriangle(Triangle, q->minx, q->miny) ||
	            PointInTriangle(Triangle, q->minx, q->maxy) ||
		    PointInTriangle(Triangle, q->maxx, q->maxy) ||
		    PointInTriangle(Triangle, q->maxx, q->miny);

	return finaltest;

	
/*	
	
	
	return(false);
	
	
	POLYGON it;
	it.c[0] = *v1;
	it.c[1] = *v2;
	it.c[2] = *v3;
	it.count = 3;
	
	GVector plane;
	GPoint pt1, pt2, pt3, pt4;
	//loop through each face of the cube.
	//chop off whatever is above that plane.
	int i;
	for(i = 0; i < 6; i++) {
		//q->v[gIntFaces[i][0]], q->v[gIntFaces[i][1]], q->v[gIntFaces[i][2]], q->v[gIntFaces[i][3]], gNormals[i]
		
		//get the 4 points on this face of the cube.
		pt1 = q->v[gIntFaces[i][0]];
		pt2 = q->v[gIntFaces[i][1]];
		pt3 = q->v[gIntFaces[i][2]];
		pt4 = q->v[gIntFaces[i][3]];
		
		//gNormals[i]
		
		//this is the plane containing this face of the cube.
		plane = (pt1 - pt3).cross(pt2 - pt3);
		plane.normalize();
		plane.W = -(plane.dot3(pt4));
		
		int res = ClipPolygon(&it, &plane);
		if(res == -1) {
			printf("Invalid plane for intersection test.\n");
			return(false);
		}
		if(res == 0) {
			break;	//nothing left in the polygon.
		}
	}
	
#ifdef COUNT_MACTHES
	if(it.count > 1) {
		gHardMatches++;
	} else {
		gHardExcludes++;
	}
#endif
	
	//if we have anything left, it intersects us.
	return(it.count > 1);
*/
}



void QTNode::doSplit() {
	
	
	//find midpoints...
	float midx = minx + (maxx - minx) / 2.0;
	float midy = miny + (maxy - miny) / 2.0;
	
	//ordering following definitions in map.h
	node1 = new QTNode(builder, midx, maxx, midy, maxy);
	node2 = new QTNode(builder, minx, midx, midy, maxy);
	node3 = new QTNode(builder, minx, midx, miny, midy);
	node4 = new QTNode(builder, midx, maxx, miny, midy);
	if(node1 == NULL || node2 == NULL || node3 == NULL || node4 == NULL) {
		printf("Error: unable to allocate new QTNode, giving up.\n");
		return;
	}
	
	unsigned long r,l;
	l = faces.size();
	for(r = 0; r < l; r++) {
		FaceRecord &cur = faces[r];
		if(builder->FaceInNode(node1, cur.face))
			node1->faces.push_back(cur);
		if(builder->FaceInNode(node2, cur.face))
			node2->faces.push_back(cur);
		if(builder->FaceInNode(node3, cur.face))
			node3->faces.push_back(cur);
		if(builder->FaceInNode(node4, cur.face))
			node4->faces.push_back(cur);
	}
	
	//clean up empty sets.
	if(node1->faces.size() == 0) {
		delete node1;
		node1 = NULL;
	}
	if(node2->faces.size() == 0) {
		delete node2;
		node2 = NULL;
	}
	if(node3->faces.size() == 0) {
		delete node3;
		node3 = NULL;
	}
	if(node4->faces.size() == 0) {
		delete node4;
		node4 = NULL;
	}
	
}

void QTBuilder::AddFace(VERTEX &v1, VERTEX &v2, VERTEX &v3) {
	FACE f;
	
#ifdef MAX_Z
	if(v1.z > MAX_Z && v2.z > MAX_Z && v3.z > MAX_Z)
		return;
#endif
	
	//this dosent work, trying new code below..
	/*
	//Make points, so I can use my functions, im lazy (:
	GPoint v1(v1);
	GPoint v2(v2);
	GPoint v3(v3);
	GVector res = (pt1 - pt3).cross(pt2 - pt3);
	float len = res.length();
#ifdef MIN_POLYGON_AREA
	if(len < MIN_POLYGON_AREA)	//skip small polygons..
		return;
#endif
	f.nx = res.x / len;
	f.ny = res.y / len;
	f.nz = res.z / len;
	f.nd = -( res.dot3(pt1) );
	*/
	
	//this still might not work
	f.nx = (v2.y - v1.y)*(v3.z - v1.z) - (v2.z - v1.z)*(v3.y - v1.y);
	f.ny = (v2.z - v1.z)*(v3.x - v1.x) - (v2.x - v1.x)*(v3.z - v1.z);
	f.nz = (v2.x - v1.x)*(v3.y - v1.y) - (v2.y - v1.y)*(v3.x - v1.x);
	NormalizeN(&f);
	f.nd = - f.nx * v1.x - f.ny * v1.y - f.nz * v1.z;
	
	f.a = v1;
	f.b = v2;
	f.c = v3;
	/*	f.a = _VertexList.size();
	_VertexList.push_back(v1);
	f.b = _VertexList.size();
	_VertexList.push_back(v2);
	f.c = _VertexList.size();
	_VertexList.push_back(v3);*/
	
	_FaceList.push_back(f);
}


GPoint::GPoint() {
	x = 0;
	y = 0;
	z = 0;
}

GPoint::GPoint(VERTEX &v) {
	x = v.x;
	y = v.y;
	z = v.z;
}

GPoint::GPoint(float nx, float ny, float nz) {
	x = nx;
	y = ny;
	z = nz;
}

//dot of x,y,z
float GPoint::dot3(const GPoint &them) const {
	return((x * them.x) + (y * them.y) +
			 (z * them.z));
}

//cross product
GPoint GPoint::cross(const GPoint &them) const {
	return(GPoint(y * them.z - z * them.y,
					 z * them.x - x * them.z,
					 x * them.y - y * them.x));
}

GPoint operator-(const GPoint &v1, const GPoint &v2) {
	return(GPoint(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z));
}


GVector::GVector() : GPoint() {
	W = 0;
}

GVector::GVector(const GPoint &them) : GPoint(them) {
	W = 1.0f;
}

GVector::GVector(float x, float y, float z, float w) : GPoint(x, y, z) {
	W = w;
}

//dot product of x,y,z,w
float GVector::dot4(const GVector &them) const {
	return((x * them.x) + (y * them.y) +
			 (z * them.z) + (W * them.W));
}

//dot product of x,y,z+w
float GVector::dot4(const GPoint &them) const {
	return((x * them.x) + (y * them.y) +
			 (z * them.z) + W);
}

float GVector::length() {
	return(sqrt((x * x) + (y * y) + (z * z)));
}

void GVector::normalize() {
	float len = length();	//stupid square roots take forever
	x /= len;
	y /= len;
	z /= len;
}

//stolen from: http://gamecode.tripod.com/tut/tut04.htm
int QTBuilder::ClipPolygon(POLYGON *poly, GVector *plane) { 
   /* Plan: cycle through the vertices, considering pairs of them.
      If both vertices are visible, add them to the new array.
      If both vertices are invisible, add neither to the new array.
      If one vertex is visible and the other is not, move the one
      that's not, and then add both vertices to the new array.
    */
   float dist1, dist2; // distances of points to plane 
   float distratio; // fraction of distance between two points 
     // new vertices might be created. Don't change 
     // polygon's vertices because we might still need 
     // them. Instead, use tempvtx array and update vertices 
     // array at the end. Create tempvtx array once only.
   int i, ii, j=0;    

   /* Check if plane is a valid plane */ 
   if (!(plane->x || plane->y || plane->z)) return -1; 
   // if not valid plane, don't change polygon and return an error; 

   /* The vertices should, as for all functions, be arranged in cyclic 
   order. That is, if a line was drawn from each vertex to the next 
   it would form the correct outline of the polygon in 3D space. 
   This routine might create new vertices because of the clipping, 
   but the cyclic order will be preserved. 
   */ 

   for (i=0; i<poly->count; i++) 
   { 
      ii = (i+1)%poly->count;
      dist1 = plane->x * poly->c[i].x + plane->y * poly->c[i].y + 
      plane->z * poly->c[i].z + plane->W; 
      dist2 = plane->x * poly->c[ii].x + plane->y * poly->c[ii].y + 
      plane->z * poly->c[ii].z + plane->W; 
      if (dist1<0 && dist2<0) // line unclipped and invisible 
         continue;
      if (dist1>0 && dist2>0) // line unclipped and visible
         tempvtx[j++]=poly->c[i];
      else // line partially visible 
      if (dist1>0) // first vertex is visible 
      { 
         distratio = dist1/(dist1-dist2); 
         tempvtx[j] = poly->c[i];
         j++; // Copied 1st vertex
         tempvtx[j].x = poly->c[i].x + 
            (poly->c[ii].x - poly->c[i].x) * distratio; 
         tempvtx[j].y = poly->c[i].y + 
            (poly->c[ii].y - poly->c[i].y) * distratio; 
         tempvtx[j].z = poly->c[i].z + 
            (poly->c[ii].z - poly->c[i].z) * distratio;
         j++; // Copied second vertex
      } 
      else // second vertex is visible 
      { 
         distratio = dist2/(dist2-dist1); 
         tempvtx[j].x = poly->c[ii].x + 
            (poly->c[i].x - poly->c[ii].x) * distratio; 
         tempvtx[j].y = poly->c[ii].y + 
            (poly->c[i].y - poly->c[ii].y) * distratio; 
         tempvtx[j].z = poly->c[ii].z + 
            (poly->c[i].z - poly->c[ii].z) * distratio; 
         j++; // Copy only first vertex. 2nd vertex will be copied
              // in next iteration of loop
      }
   } 

   for (i=0; i<j; i++) 
      poly->c[i] = tempvtx[i]; // Update the vertices in polygon
   poly->count = j;            // Update the vertex count
   return j; 
}


void QTBuilder::NormalizeN(FACE *p) {
	float len = sqrt(p->nx*p->nx + p->ny*p->ny + p->nz*p->nz);
	p->nx /= len;
	p->ny /= len;
	p->nz /= len;
}

