#include <stdio.h>
#ifdef WIN32 //vc++ chokes here without this
#include <string>
#else
#include <string.h>
#endif
#include <stdlib.h>
#include <iostream>

#include "file.hpp"
#include "ter.hpp"

using namespace std;

TERLoader::TERLoader() {
  this->buffer = NULL;
  this->buf_len = -1;
  this->archive = NULL;
  this->status = 0;
}

TERLoader::~TERLoader() {
  this->Close();
}



void build_hex_line(const char *buffer, unsigned long length, unsigned long offset, char *out_buffer, unsigned char padding)
{
char *ptr=out_buffer;
int i;
char printable[17];
	ptr+=sprintf(ptr,"%0*i:",padding,offset);
	for(i=0;i<16; i++) {
		if (i==8) {
			strcpy(ptr," -");
			ptr+=2;
		}
		if (i+offset < length) {
			unsigned char c=*(const unsigned char *)(buffer+offset+i);
			ptr+=sprintf(ptr," %02x",c);
			printable[i]=isprint(c) ? c : '.';
		} else {
			ptr+=sprintf(ptr,"   ");
			printable[i]=0;
		}
	}
	sprintf(ptr,"  | %.16s",printable);
}

void print_hex(const char *data, unsigned long length) {
	char buffer[80];
	uint32 offset;
	for(offset=0;offset<length;offset+=16) {
		build_hex_line(data,length,offset,buffer,5);
		printf("%s\n", buffer);	//%s is to prevent % escapes in the ascii
	}
}

int TERLoader::Open(char *base_path, char *zone_name, Archive *archive) {
  ter_header *thdr;
  ter_vertex *tver;
  ter_vertex_v3 *tver3;
  ter_triangle *ttri;
  uchar *ter_orig, *ter_tmp;
  int model = 0;
  int i, j, mat_count = 0;

  Zone_Model *zm;

  material *mlist;
  char var_len, val_len;
  char *var, *val;

  uchar *buffer;
  int buf_len, bone_count;

  char *filename;

  if(zone_name[strlen(zone_name) - 4] == '.')
    filename = zone_name;
  else {
    filename = new char[strlen(zone_name) + 5];
    sprintf(filename, "%s.ter", zone_name);
  }

  if(!GetFile(&buffer, &buf_len, base_path, filename, archive)) {
    if(filename != zone_name)
      delete[] filename;
    return 0;
  }

printf("Writting TER to '%s'\n", filename);
FILE *ttt = fopen(filename, "wb");
fwrite(buffer, 1, buf_len, ttt);
fclose(ttt);
  
  if(filename != zone_name)
    delete[] filename;

  thdr = (ter_header *) buffer;

  mlist = new material[thdr->mat_count];

  ter_orig = buffer;

  if(thdr->magic[0] != 'E' || thdr->magic[1] != 'Q' || thdr->magic[2] != 'G')
    return 0;

  if(thdr->magic[3] == 'M') {
	  printf("Model File encountered in TER loader!\n");
	  return(0);
  }
  else if(thdr->magic[3] == 'T') {
    buffer += sizeof(ter_header);
  } else
    return 0;
  
  ter_tmp = buffer + thdr->list_len;

  if(thdr->version == 3)
  {
	if(sizeof(ter_header) + thdr->list_len + (thdr->vert_count * sizeof(ter_vertex_v3)) + (thdr->tri_count * sizeof(ter_triangle)) > (unsigned int)buf_len)
	{
		printf("Buffer Range Check Failed.\n");
		return 0;
	}
  }
  else
  {
	if(sizeof(ter_header) + thdr->list_len + (thdr->vert_count * sizeof(ter_vertex)) + (thdr->tri_count * sizeof(ter_triangle)) > (unsigned int)buf_len)
	{
		printf("Buffer Range Check Failed(v3).\n");
		return 0;
	}
  }

  for(i = 0; i < (int)thdr->mat_count; ++i) {
    mlist[i].name = NULL;
    mlist[i].basetex = NULL;
  }

int line_count = 0;
  j = 0;
  while(buffer < ter_tmp) {
    val_len = strlen((char *) buffer);
    val = (char *) buffer;

    buffer += val_len + 1;
    var_len = strlen((char *) buffer);
    var = (char *) buffer;
	line_count++;


    if(strlen(var) == 0 || strlen(val) == 0) {
      ++buffer;
      continue;
    }

    if(!strcmp(val, "e_fShininess0")) {
      // Shiny!
      continue;
    }
    else if(!strcmp(var, "e_TextureDiffuse0")) {
      mlist[j].basetex = new char[val_len + 1];
      memcpy(mlist[j].basetex, val, val_len + 1);
      ++mat_count;
      ++j;
    }
    else if(val[0] != 'e' && val[1] != '_' && ((var[0] != 'e' && var[1] != '_') || !strcmp(val, "e_fShininess0"))) {
      if(val[val_len - 3] == '.') {
        mlist[j].name = new char[var_len + 1];
        memcpy(mlist[j].name, var, var_len + 1);
      }
      else {
        mlist[j].name = new char[val_len + 1];
        memcpy(mlist[j].name, val, val_len + 1);
        continue;
      }
    }

    buffer += var_len + 1;
  }

printf("%s: Material count: %d, list len=%d, line count=%d\n", zone_name, thdr->mat_count, thdr->list_len, line_count);

  this->model_data.zone_model = new Zone_Model;
  zm = this->model_data.zone_model;
  
  zm->vert_count = thdr->vert_count;
  zm->poly_count = thdr->tri_count;
  
  zm->verts = new Vertex *[zm->vert_count];
  zm->polys = new Polygon *[zm->poly_count];

  this->model_data.plac_count = 0;
  this->model_data.model_count = 0;

  //this is not always correct!
  // Derision. Fix offset calculation for EQG version 2
  //
//#define DEBUGTER
  buffer = ter_orig + thdr->list_len + sizeof(ter_header);
  if(thdr->magic[3] == 'M') buffer = buffer + 4;
#ifdef DEBUGTER
     printf("Starting offset is %8X\n", buffer-ter_orig);
#endif
     for(int b=0; b<thdr->mat_count; b++) {
        unsigned long property_count = *((unsigned long *)(buffer+12));
#ifdef DEBUGTER
        printf("Property count is %d\n", property_count); fflush(stdout);
#endif
        buffer += 16;
        for (int a=0; a<property_count; a++)
            buffer += 12;
     }
#ifdef DEBUGTER
     printf("Offset is %8X\n", buffer - ter_orig);
#endif

  printf("Orig Delta %d 0x%x\n", buffer-ter_tmp, buffer-ter_tmp);

  /*
  * This is the biggest hack pile of crap ever... but I am tired of trying to
  * figure out this file format, so im just looking at the files for the offsets
  * myself.
  *
  */
  /*if(string("ter_broodlands.ter") == zone_name) {
	  buffer = ter_orig + 0x382BB - 4;
//	  zm->vert_count = (0x2ABF90-0x382BB-4)/sizeof(ter_vertex);
  } else if(string("ter_volcano.ter") == zone_name) {
	  if(thdr->mat_count == 19320) {
		  buffer = ter_orig + 0x39BB66;
	  } else {
		  buffer = ter_orig + 0x38BF0;
	  }
  } else if(string("ter_guildhall.ter") == zone_name) {
	  buffer = ter_orig + 0x3080 - 2;
  } else if(string("ter_guildlobby.ter") == zone_name) {
	  buffer = ter_orig + 0x4190 + 19;
  } else if(string("ter_harbingers.ter") == zone_name) {
	  buffer = ter_orig + 0x1178;
  } else if(string("ter_main.ter") == zone_name) {
	  //stillmoona
	  buffer = ter_orig + 0x71944+4;
  } else if(string("ter_easterntemple.ter") == zone_name) {
	  //stillmoonb
	  buffer = ter_orig + 0xE340-2;
  } else if(string("ter_abyss01.ter") == zone_name) {
	  //thenest
	  buffer = ter_orig + 0xCA244;
  } else if(string("ter_stormtower01.ter") == zone_name) {
	  //thundercrest
	  buffer = ter_orig + 0x62230-4;
  }*/
 
/*  buffer = ter_orig + 0x382BB - 4;

  printf("Good Delta %d 0x%x\n", buffer-ter_tmp, buffer-ter_tmp);*/
  
/*
  uchar *pptr2 =  (buffer - (sizeof(ter_vertex)*1));
printf("v dump start: 0x%x\n", (pptr2-ter_orig));
  print_hex((const char *)pptr2, (buffer-pptr2)+sizeof(ter_vertex)*10);
*/
  
printf("%d verts start at %d (0x%x)\n", zm->vert_count, (buffer-ter_orig), (buffer-ter_orig));
print_hex((const char *)(buffer-16), sizeof(ter_vertex)*3+16);

  for(i = 0; i < zm->vert_count; ++i) {
	  if(thdr->version == 3)
	  {
		  tver3 = (ter_vertex_v3 *) buffer;
		  zm->verts[i] = new Vertex;
		  zm->verts[i]->x = tver3->x;
		  zm->verts[i]->y = tver3->y;
		  zm->verts[i]->z = tver3->z;
		  zm->verts[i]->u = tver3->u;
		  zm->verts[i]->v = tver3->v;
		  buffer += sizeof(ter_vertex_v3);
	  }
	  else
	  {
		  tver = (ter_vertex *) buffer;
		  zm->verts[i] = new Vertex;
		  zm->verts[i]->x = tver->x;
		  zm->verts[i]->y = tver->y;
		  zm->verts[i]->z = tver->z;
		  zm->verts[i]->u = tver->u;
		  zm->verts[i]->v = tver->v;
		  buffer += sizeof(ter_vertex);
	  }
  }

/*  uchar *pptr =  (buffer);

printf("dump tri start: 0x%x\n", (pptr-ter_orig));
  print_hex((const char *)(pptr-16), 16+sizeof(ter_triangle)*10);

int skipped = 0;
  j = 0;
printf("%d tris start at %d (0x%x)\n", zm->poly_count, (buffer-ter_orig), (buffer-ter_orig));*/

	printf("dump tri start: 0x%x\n", (buffer));
	print_hex((const char *)(buffer-16), 16+sizeof(ter_triangle)*10);
	printf("%d tris start at %d (0x%x)\n", zm->poly_count, (buffer), (buffer));

	uint8 errored = 0;
	int skipped = 0;
	j = 0;
	for(i = 0; i < zm->poly_count; ++i) {
		ttri = (ter_triangle *) buffer;
		if(ttri->group == -1) {
			skipped++;
			buffer += sizeof(ter_triangle);
			continue;
		}
		zm->polys[j] = new Polygon;


		if(ttri->v1 >= zm->vert_count && errored < 10) {
			printf("Tri %d/%d (s %d) @0x%x: invalid v1: %d >= %d\n", i, zm->poly_count, skipped, (buffer-ter_orig), ttri->v1, zm->vert_count);
			errored++;
		}
		if(ttri->v2 >= zm->vert_count && errored < 10) {
			printf("Tri %d/%d (s %d) @0x%x: invalid v2: %d >= %d\n", i, zm->poly_count, skipped, (buffer-ter_orig), ttri->v2, zm->vert_count);
			errored++;
		}
		if(ttri->v3 >= zm->vert_count && errored < 10) {
			printf("Tri %d/%d (s %d) @0x%x: invalid v3: %d >= %d\n", i, zm->poly_count, skipped, (buffer-ter_orig), ttri->v3, zm->vert_count);
			errored++;
		}

		if(ttri->v1 == 0x80000 && ttri->v1 >= zm->vert_count) {
			zm->polys[j]->v1 = 0;
			zm->polys[j]->v2 = 0;
			zm->polys[j]->v3 = 0;
		} else {
			zm->polys[j]->v1 = ttri->v1;
			zm->polys[j]->v2 = ttri->v2;
			zm->polys[j]->v3 = ttri->v3;

			if(ttri->group == -1)
				zm->polys[j]->tex = thdr->mat_count;
			else
				zm->polys[j]->tex = ttri->group;

		}

		++j;
		buffer += sizeof(ter_triangle);
	}
	printf("Skipped %d, %d total\n", skipped, zm->poly_count);
  
  zm->poly_count = j;
  
  zm->tex_count = thdr->mat_count;
  zm->tex = new Texture *[thdr->mat_count];
  
  for(i = 0; i < (int)thdr->mat_count; ++i) {
    zm->tex[i] = new Texture;
    zm->tex[i]->frame_count = 1;
    zm->tex[i]->filenames = new char *[1];
    if(mlist[i].basetex) {
      zm->tex[i]->filenames[0] = new char[strlen(mlist[i].basetex) + 1];
      memcpy(zm->tex[i]->filenames[0], mlist[i].basetex, strlen(mlist[i].basetex) + 1);

      delete[] mlist[i].basetex;
    }
    else if(mlist[i].name) {
      zm->tex[i]->filenames[0] = new char[strlen(mlist[i].name) + 1];
      memcpy(zm->tex[i]->filenames[0], mlist[i].name, strlen(mlist[i].name) + 1);
      delete[] mlist[i].name;
    }
  }
  // zm->tex[thdr->mat_count] = new Texture;
  // zm->tex[thdr->mat_count]->filename = NULL;
  
  delete[] mlist;

  this->status = 1;
  return 1;
}

int TERLoader::Close() {
  Zone_Model *zm = this->model_data.zone_model;
  int i;

  if(!this->status)
    return 1;

  for(i = 0; i < zm->vert_count; ++i)
    delete zm->verts[i];
  for(i = 0; i < zm->poly_count; ++i)
    delete zm->polys[i];
  for(i = 0; i < zm->tex_count; ++i) {
    delete[] zm->tex[i]->filenames[0];
    delete[] zm->tex[i]->filenames;
    delete zm->tex[i];
  }

  delete[] zm->verts;
  delete[] zm->polys;

  delete this->model_data.zone_model;

  return 1;
}
