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
#include <string.h>
#include <math.h>
//#include "EQWldData.h"
#include "awater.h"
#include "types.h"
#include "s3d.h"
#include "wld.h"

#include "archive.hpp"
#include "pfs.hpp"

#include "file_loader.hpp"
#include "zon.hpp"
#include "ter.hpp"


//this un-commented works with my map.cpp code correctly.
//with both of my inverts there commented.
#define INVERSEXY 1

#include <vector>
#include <map>
using namespace std;

//#define SPLIT_DEBUG


bool BuildWaterMap(const char *shortname, long DefaultRegionType);
void PrintBSP(BSP_Node *tree, long node_number);
long BSPFindRegion(BSP_Node *tree, long node_number, long region);
long BSPFindNode(BSP_Node *tree, long node_number, float x, float y, float z);
long BSPMarkRegion(BSP_Node *tree, long node_number, long region, int region_type);
long BSPCountNodes(BSP_Node *tree, long node_number);


int main(int argc, char *argv[]) {

	long DefaultRegionType=RegionTypeWater;
	
	if((argc < 2) ||
	   (argc > 3) ||
	   ((argc == 3)&&(strcasecmp(argv[1],"-dl")))) {

		printf("Usage: %s [-dl] (zone short name) to generate a water/lava map\n\n", argv[0]);
		printf("              -dl = mark special regions as lava if not tagged in .WLD\n");
		printf("\nIf -dl is omitted, untagged special regions will be marked as water\n");
		return(1);
	}
	
	char bufm[250];
	
	if(argc==3)
		DefaultRegionType=RegionTypeLava;
	sprintf(bufm, "%s.wtr", argv[argc-1]);
	if(!BuildWaterMap(argv[argc-1],DefaultRegionType))
		return(1);

	return(0);

}



bool BuildWaterMap(const char *shortname, long DefaultRegionType) {

//	char bufm[96];
	char bufs[96], bufw[96];
	long WaterOrLavaCount = 0;
	
	//TODO: clean up a LOT of memory that the freaku code does not
	
//	sprintf(bufm, "%s.map", shortname);
	sprintf(bufs, "%s.s3d", shortname);
	sprintf(bufw, "%s.wld", shortname);
	
	s3d_object s3d/*, s3d_obj, s3d_chr*/;
	wld_object wld/*, wld2, wld_obj, wld_chr*/;
	unsigned char *buf;
  
	FILE *s3df = fopen(bufs, "rb");
	if(s3df == NULL) {
		// One day we may try EQG, but not today.
		printf("Unable to open s3d file '%s'.\n", bufs);
		return(false);
	}

	printf("Loading %s...\n", bufs);
	
	
	S3D_Init(&s3d, s3df);
	S3D_GetFile(&s3d, bufw, &buf);
	WLD_Init(&wld, buf, &s3d, 1);

  	BSP_Node *tree = NULL;
	struct_Data29 *data29;

	for(int i=0; i<wld.fragCount; i++) {
		if(wld.frags[i]->type == 0x21) {
			printf("We have a type 0x21 fragment\n");
			tree = (BSP_Node *) wld.frags[i]->frag;
		}
		if(wld.frags[i]->type == 0x29) {
			printf("We have a type 0x29 fragment. ");
			data29 = (struct_Data29 *) wld.frags[i]->frag;
			printf("It has %ld regions and is marked as type %d. ",
				data29->region_count, data29->region_type);
			switch(data29->region_type) {
				case RegionTypeUnsupported: { printf("Unsupported\n"); break; }
				case RegionTypeUntagged: {
						printf("Untagged. We will set it to ");
						if(DefaultRegionType==RegionTypeWater) { 
							printf("Water\n");
						} else printf("Lava\n");
						data29->region_type = DefaultRegionType;
						WaterOrLavaCount++;
						break;
					}
				case RegionTypeWater:		{ printf("Water\n"); WaterOrLavaCount++; break; }
				case RegionTypeLava:		{ printf("Lava\n"); WaterOrLavaCount++; break; }
				case RegionTypeZoneLine: 	{ printf("Zoneline\n"); break; }
				default: printf("UNKNOWN\n");
			}
		}

	}
	if(tree==NULL) {
		printf("No BSP Tree. Bailing out\n");
		return(false);
	}
	if(WaterOrLavaCount==0) {
		printf("No water or lava in this zone. Nothing for us to do.\n");
		return(false);
	}


	long BSPTreeSize = BSPCountNodes(tree, 1);

	printf("There are %ld nodes in the BSP tree\n", BSPTreeSize);

	// Now we mark each leaf in the BSP tree that is in a 'special area' with what type the area is
	// Water, Lava, Zoneline etc
	
	for(int i=0; i<wld.fragCount; i++) {
		if(wld.frags[i]->type == 0x29) {
			data29 = (struct_Data29 *) wld.frags[i]->frag;
			for(long j=0; j<data29->region_count; j++) {
				BSPMarkRegion(tree, 1,data29->region_array[j]+1, data29->region_type); 
			}
		}
	}

	// Now write out the file

	char bufm[250];
	sprintf(bufm, "%s.wtr", shortname);

	FILE *WaterFile = fopen(bufm, "wb");
        if(WaterFile == NULL) {
		printf("Failed to open %s for writing\n", bufm);
		return(false);
	}
	const char *WFMagic = "EQEMUWATER";
	const long WFVersion = 1;

	if(fwrite(WFMagic, strlen(WFMagic), 1, WaterFile) != 1) {
		printf("Error writing output file\n");
		fclose(WaterFile);
		return(false);
	}
	if(fwrite(&WFVersion, sizeof(WFVersion), 1, WaterFile) != 1) {
		printf("Error writing output file\n");
	        fclose(WaterFile);
	        return(false);
	}

	if(fwrite(&BSPTreeSize, sizeof(BSPTreeSize), 1, WaterFile) != 1) {
		printf("Error writing output file\n");
	        fclose(WaterFile);
	        return(false);
	}

	if(fwrite(tree, sizeof(BSP_Node), BSPTreeSize, WaterFile) != BSPTreeSize) {
		printf("Error writing output file\n");
	        fclose(WaterFile);
	        return(false);
	}

	fclose(WaterFile);
	return(true);
}


void PrintBSP(BSP_Node *tree, long node_number) {
	printf("Node %ld, Left=%ld, Right=%ld\n",
	       node_number-1,
	       tree[node_number-1].left,
	       tree[node_number-1].right);
	printf("Normals are %4.3f, %4.3f, %4.3f, SplitD is %4.3f\n",
	       tree[node_number-1].normal[0],
	       tree[node_number-1].normal[1],
	       tree[node_number-1].normal[2],
	       tree[node_number-1].splitdistance);

	if(tree[node_number-1].left!=0) PrintBSP(tree, tree[node_number-1].left);
	if(tree[node_number-1].right!=0) PrintBSP(tree, tree[node_number-1].right);
	if((tree[node_number-1].left==0)&&
	   (tree[node_number-1].right==0)) {
	   printf("Region pointer is %ld\n", tree[node_number-1].region);
	}

}

long BSPCountNodes(BSP_Node *tree, long node_number) {

	long NodesInRightBranch = 0, NodesInLeftBranch = 0;

	if((tree[node_number-1].left==0)&&
		(tree[node_number-1].right==0)) return 1;

	if(tree[node_number-1].left!=0) NodesInLeftBranch = BSPCountNodes(tree, (tree[node_number-1].left));
	if(tree[node_number-1].right!=0) NodesInRightBranch = BSPCountNodes(tree, (tree[node_number-1].right));

	return(NodesInRightBranch + NodesInLeftBranch + 1);

}
	

long BSPFindRegion(BSP_Node *tree, long node_number, long region) {
	if(node_number<1) {
		printf("Something went wrong\n");
		exit(1);
	}
	if((tree[node_number-1].left==0)&&
	   (tree[node_number-1].right==0))  {
	   	if(tree[node_number-1].region==region) return node_number;
	}

	
	long retnode ;
	if(tree[node_number-1].left!=0) {
		retnode = BSPFindRegion(tree, tree[node_number-1].left, region);
		if(retnode != 0) return retnode ;
	}

	if(tree[node_number-1].right!=0) {
		return BSPFindRegion(tree, tree[node_number-1].right, region);
	}

	return 0;

}

long BSPFindNode(BSP_Node *tree, long node_number, float x, float y, float z) {

	float distance;


	printf("BSP Find Node, currently in Node %ld\n", node_number);
	// Are we at a leaf
	
	if((tree[node_number-1].left==0)&&
	   (tree[node_number-1].right==0))  {
	   	return tree[node_number-1].region;
	}
	
	// No, so determine which side of the split plane we are on
	//
	
	distance = (x * tree[node_number-1].normal[0]) +
	               (y * tree[node_number-1].normal[1]) +
		       (z * tree[node_number-1].normal[2]) +
                       tree[node_number-1].splitdistance;
	printf("Distance is %4.3f\n", distance);
	// Guess which side to go down
	if(distance == 0.0f) {
		printf("Distance is 0, don't know what to do!\n");
		exit(1);
	}
	if(distance >0.0f) {
		if(tree[node_number-1].left==0) {
			printf("Pos and no left node, abort!\n");
			exit(1);
		}
		return BSPFindNode(tree, tree[node_number-1].left,
				   x, y, z);
	}

	if(tree[node_number-1].right==0) {
		printf("Neg and no right node, abort!\n");
		exit(1);
	}

	return BSPFindNode(tree, tree[node_number-1].right,
			   x, y, z);

}

	
long BSPMarkRegion(BSP_Node *tree, long node_number, long region, int region_type) {
        if(node_number<1) {
                printf("Something went wrong\n");
                exit(1);
        }
        if((tree[node_number-1].left==0)&&
           (tree[node_number-1].right==0))  {
			   if(tree[node_number-1].region==region) {
				   tree[node_number-1].special=region_type;
				   return node_number;
			   }
        }


        long retnode ;
        if(tree[node_number-1].left!=0) {
                retnode = BSPMarkRegion(tree, tree[node_number-1].left, region, region_type);
                if(retnode != 0) return retnode ;
        }

        if(tree[node_number-1].right!=0) {
                return BSPMarkRegion(tree, tree[node_number-1].right, region, region_type);
        }

        return 0;

}
	



