#ifndef __EQCLIENT_WLD_H_
#define __EQCLIENT_WLD_H_

#include "s3d.h"
#include "types.h"

#define FRAGMENT(name) status = name(&frag_obj, obj, (wld + pos + sizeof(struct_wld_basic_frag)), frag->size, frag->nameRef)
#define FRAGMENT_FUNC(name) int name(void **obj, wld_object *wld, uchar *buf, int len, int frag_name)

typedef struct struct_wld_header {
  long magic;
  long version;
  long fragmentCount;
  long header3;
  long header4;
  long stringHashSize;
  long header6;
} struct_wld_header;

typedef struct struct_wld_basic_frag {
  long size;
  long id;
  long nameRef;
} struct_wld_basic_frag;


typedef struct struct_Data36 {
  long flags;
  long fragment1;
  long fragment2;
  long fragment3;
  long fragment4;
  float centerX;
  float centerY;
  float centerZ;
  long params2[3]; // 48
  float maxDist;
  float minX;
  float minY;
  float minZ;
  float maxX;
  float maxY;
  float maxZ; // 24
  short int vertexCount;
  short int texCoordsCount;
  short int normalsCount;
  short int colorCount;
  short int polygonsCount;
  short int size6;
  short int polygonTexCount;
  short int vertexTexCount;
  short int size9;
  short int scale; // 20
} struct_Data36;

struct Data10 {
  long flags, size1, fragment;
};

typedef struct WLDVertex {
  double x, y, z;
  float u, v;
  short skin;
} WLDVertex;

typedef struct Vert {
  signed short int x, y, z;
} Vert;

typedef struct TexCoordsNew {
  signed long tx, tz;
} TexCoordsNew;

typedef struct TexCoordsOld {
  signed short int tx, tz;
} TexCoordsOld;

typedef struct VertexNormal {
  signed char nx, ny, nz;
} VertexNormal;

typedef struct VertexColor {
  char color[4];
} VertexColor;

typedef struct WLDPolygon {
  short int flags, v1, v2, v3;
} WLDPolygon;

typedef struct Poly {
  long int flags, v1, v2, v3, tex;
} Poly;

typedef struct WLDTexture {
  int count;
  int *flags;
  int params;
  char **filenames;
  uint32 *textures;
} WLDTexture;

typedef struct Mesh {
  long name;
  long polygonCount;
  long vertexCount;
  WLDVertex *verti;
  Poly *poly;
  WLDTexture *tex;
} Mesh;

typedef struct ZoneMesh {
  long name;
  long polygonCount;
  long vertexCount;
  WLDVertex **verti;
  Poly **poly;
  WLDTexture *tex;
} ZoneMesh;

typedef struct struct_frag {
  long type;
  long name;
  void *frag;
}struct_frag;

typedef struct struct_Data21 {
  float normal[3], splitdistance;
  long region, node[2];
} struct_Data21;

typedef struct struct_Data22 {
  long flags, fragment1, size1, size2, params1, size3, size4, params2, size5, size6;
} struct_Data22;

typedef struct struct_Data29 {
  long region_count;
  long *region_array;
  long strlen;
  char *str;
  int region_type; // We fill this in with -1 for unknown, 1 for water, 2 for lava
} struct_Data29;

/*typedef struct BSP_Region {
  
} BSP_Region;*/

typedef struct BSP_Node {
  long node_number;
  float normal[3], splitdistance;
  long region;
  int special;
 long left, right;
} BSP_Node;

typedef struct struct_Data15 {
  uint32 ref, flags, fragment1;
  float trans[3], rot[3];
  float scale[3];
  uint32 fragment2, flags2;
} struct_Data15;

typedef struct WLDPlaceable {
  float trans[3], rot[3], scale[3];
  Mesh *mesh;
} WLDPlaceable;

typedef struct Placeable_LL {
  WLDPlaceable *obj;
  struct Placeable_LL *next;
} Placeable_LL;

typedef struct wld_object {
  int fragCount;
  char loadBSP;
  uchar *wld;
  uchar *sHash;
  uint8 _new;
  s3d_object *s3d;
  Placeable_LL *placeable, *placeable_cur;
  struct wld_object *objs;
  struct_frag **frags;
} wld_object;

#ifdef __cplusplus
extern "C" {
#endif

int WLD_Init(wld_object *obj, uchar *wld, s3d_object *s3d, char loadBSP);
int WLD_GetZoneMesh(wld_object *obj, ZoneMesh *mesh);
#ifdef __cplusplus
}
#endif


#endif
