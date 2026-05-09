#include <stdio.h>
#include <string.h>

#include "3d.hpp"
#include "zon.hpp"

ZonLoader::ZonLoader() {
  this->buffer = NULL;
  this->buf_len = -1;
  this->archive = NULL;
  this->status = 0;
}

ZonLoader::~ZonLoader() {
  this->Close();
}

int ZonLoader::Open(char *base_path, char *zone_name, Archive *archive) {
  uchar *buffer;
  int buf_len;

  Texture **tex;
  int tex_count, tex_tmp, *tex_map;

  int i, j, k, l, len;
  char **model_names;

  char *filename;

  uchar *zon_tmp, *zon_orig;

  float rot_change = 180.0f / 3.14159f;
  // float rot_change = 1.0f;
  float base[3];

//  filename = new char[strlen(zone_name) + 5];
//  sprintf(filename, "%s.zon", zone_name);
  const char *fn = archive->FindExtension(".zon");
  if(fn == NULL) {
	  printf("Unable to find a .zon file in %s\n", zone_name);
	  //try looking just for the .ter file...

	const char *tfn = archive->FindExtension(".ter");
	if(tfn == NULL)
		return(0);
	
	if(this->terloader.Open(NULL, (char *) tfn, archive) == 0) {
		printf("failed to direct load TER file in %s\n", zone_name);
		return(0);
	}
	printf("Got direct TER, it is %p\n", terloader.model_data.zone_model);
	this->model_data.zone_model = terloader.model_data.zone_model;
	  
    return 1;
  }

  filename = new char[strlen(fn)+1];
  strcpy(filename, fn);
  
  if(!archive->GetFile(filename, &buffer, &buf_len)) {
	  printf("Unable to load '%s' from '%s'\n", filename, zone_name);
    delete[] filename;
    return 0;
  }
  
printf("Writting ZON to '%s'\n", filename);
FILE *ttt = fopen(filename, "wb");
fwrite(buffer, 1, buf_len, ttt);
fclose(ttt);

  delete[] filename;

  zon_header *hdr = (zon_header *) buffer;
  zon_placeable *plac;

  if(hdr->magic[0] != 'E' || hdr->magic[1] != 'Q' || hdr->magic[2] != 'G' || hdr->magic[3] != 'Z') {
	  printf("Bad file header in ZON %s\n", zone_name);
    return 0;
  }

  buffer += sizeof(zon_header);

  //search for a nice TER file...
  uchar *buf_end = buffer + buf_len;
  while(buffer < buf_end) {
	if(strstr((const char *) buffer, ".ter") != NULL || strstr((const char *) buffer, ".TER") != NULL) {
		if(this->terloader.Open(NULL, (char *) buffer, archive) == 0) {
		  printf("failed to load TER file in %s\n", zone_name);
		  return(0);
		}
		printf("Got TER, it is %p\n", terloader.model_data.zone_model);
		this->model_data.zone_model = terloader.model_data.zone_model;
		break;
	}
	buffer += strlen((const char *) buffer)+1;
  }

  return 1;
/*******************************************/

  zon_orig = buffer;
  zon_tmp = buffer + hdr->list_len;
  buffer += strlen((char *) buffer) + 1;

  i = 0;
  while(buffer < zon_tmp) {
    len = strlen((char *) buffer);
    if(buffer[len - 4] == '.')
      ++i;
    buffer += len + 1;
  }
  this->model_data.model_count = i;
  model_names = new char *[this->model_data.model_count];
  this->model_data.models = new Model *[this->model_data.model_count];

  buffer = zon_orig;
  buffer += strlen((char *) buffer) + 1;
  i = 0;
  while(buffer < zon_tmp) {
    len = strlen((char *) buffer);
    if(buffer[len - 4] == '.') {
      model_names[i] = (char *) buffer;
      ++i;
    }
    buffer += len + 1;
  }

  buffer = zon_orig + hdr->list_len + hdr->unid * 4;

  this->model_data.plac_count = hdr->obj_count - 1;
  this->model_data.plac_count = 0;
  // this->model_data.placeable = new Placeable *[this->model_data.plac_count];
  this->model_data.placeable = 0;
  
  plac = (zon_placeable *) buffer;
  base[0] = plac->x;
  base[1] = plac->y;
  base[2] = plac->z;

  printf("(%f %f %f) (%f %f %f)\n", plac->x, plac->y, plac->z, plac->rz, plac->ry, plac->rx);

  buffer += sizeof(zon_placeable);

  for(i = 0; i < this->model_data.plac_count; ++i) {
    plac = (zon_placeable *) buffer;

    zon_tmp = zon_orig + plac->loc;

    this->model_data.placeable[i] = new Placeable;

    this->model_data.placeable[i]->x = plac->x;
    this->model_data.placeable[i]->y = plac->y;
    this->model_data.placeable[i]->z = plac->z;

    this->model_data.placeable[i]->rx = plac->rz * rot_change;
    this->model_data.placeable[i]->ry = plac->ry * rot_change;
    this->model_data.placeable[i]->rz = plac->rx * rot_change;

    this->model_data.placeable[i]->scale[0] = plac->scale;
    this->model_data.placeable[i]->scale[1] = plac->scale;
    this->model_data.placeable[i]->scale[2] = plac->scale;
    this->model_data.placeable[i]->model = -1;
#ifdef DEBUG_MODE
    printf("%s (%f %f %f) (%f %f %f) %f\n",
           zon_tmp,
           this->model_data.placeable[i]->x,
           this->model_data.placeable[i]->y,
           this->model_data.placeable[i]->z,
           this->model_data.placeable[i]->rx,
           this->model_data.placeable[i]->ry,
           this->model_data.placeable[i]->rz,
           this->model_data.placeable[i]->scale);
#endif
    while(zon_tmp[strlen((char *) zon_tmp) - 4] != '.')
      zon_tmp += strlen((char *) zon_tmp) + 1;

    for(j = 0; j < this->model_data.model_count; ++j) {
      if(!strcmp(model_names[j], (char *) zon_tmp)) {
        this->model_data.placeable[i]->model = j;
        break;
      }
    }

    buffer += sizeof(zon_placeable);
  }

  // model_loaders = new TERLoader[this->model_data.model_count];
  model_loaders = 0;

  tex_count = this->model_data.zone_model->tex_count;

  for(j = 0; j < this->model_data.model_count; ++j) {
    model_loaders[j].Open(NULL, model_names[j], archive);

    this->model_data.models[j] = new Model;
    this->model_data.models[j]->vert_count = model_loaders[j].model_data.zone_model->vert_count;
    this->model_data.models[j]->poly_count = model_loaders[j].model_data.zone_model->poly_count;
    this->model_data.models[j]->tex_count = model_loaders[j].model_data.zone_model->tex_count;
    this->model_data.models[j]->verts = model_loaders[j].model_data.zone_model->verts;
    this->model_data.models[j]->polys = model_loaders[j].model_data.zone_model->polys;
    this->model_data.models[j]->tex = model_loaders[j].model_data.zone_model->tex;

    tex_tmp = 1;
    for(i = 0; i < this->model_data.models[j]->tex_count; ++i) {
      for(k = 0; k < this->model_data.zone_model->tex_count; ++k) {
        if((!this->model_data.zone_model->tex[k]->filenames[0] && !this->model_data.models[j]->tex[i]->filenames[0]) ||
           (this->model_data.zone_model->tex[k]->filenames[0] &&
            this->model_data.models[j]->tex[i]->filenames[0] &&
            !strcmp(this->model_data.zone_model->tex[k]->filenames[0], this->model_data.models[j]->tex[i]->filenames[0]))) {
          tex_tmp = 0;
          break;
        }
      }
      if(tex_tmp)
        ++tex_count;
      else {
        for(k = 0; k < j; ++k) {
          for(l = 0; l < this->model_data.models[k]->tex_count; ++l) {
            if(this->model_data.models[k]->tex[l]->filenames[0] == this->model_data.models[j]->tex[i]->filenames[0] || 
               (this->model_data.models[k]->tex[l]->filenames[0] &&
                this->model_data.models[j]->tex[i]->filenames[0] &&
                !strcmp(this->model_data.models[k]->tex[l]->filenames[0], this->model_data.models[j]->tex[i]->filenames[0]))) {
              tex_tmp = 0;
              break;
            }
          }
        }
        if(tex_tmp)
          ++tex_count;
      }
    }
  }

  printf("%i\n", tex_count);
  tex = new Texture *[tex_count];
  tex_count = this->model_data.zone_model->tex_count;
  tex_tmp = 1;
  for(i = 0; i < tex_count; ++i)
    tex[i] = this->model_data.zone_model->tex[i];
  for(i = 0; i < this->model_data.model_count; ++i) {
    for(j = 0; j < this->model_data.models[i]->tex_count; ++j) {
      for(k = 0; k < tex_count; ++k) {
        if(tex[k]->filenames[0] == this->model_data.models[i]->tex[j]->filenames[0] ||
           (tex[k]->filenames[0] &&
            this->model_data.models[i]->tex[j]->filenames[0] &&
            !strcmp(tex[k]->filenames[0], this->model_data.models[i]->tex[j]->filenames[0]))) {
          tex_tmp = 0;
          break;
        }
      }
      if(tex_tmp) {
        tex[tex_count] = this->model_data.models[i]->tex[j];
        ++tex_count;
      }
    }
  }
  for(i = 0; i < this->model_data.model_count; ++i) {
    tex_map = new int[this->model_data.models[i]->tex_count];

    for(j = 0; j < this->model_data.models[i]->tex_count; ++j) {
      tex_map[j] = 0;
      for(k = 0; k < tex_count; ++k) {
        if(tex[k]->filenames[0] == this->model_data.models[i]->tex[j]->filenames[0] ||
           (tex[k]->filenames[0] &&
            this->model_data.models[i]->tex[j]->filenames[0] &&
            !strcmp(tex[k]->filenames[0], this->model_data.models[i]->tex[j]->filenames[0]))) {
          tex_map[j] = k;
          break;
        }
      }
    }

    for(j = 0; j < this->model_data.models[i]->poly_count; ++j)
      this->model_data.models[i]->polys[j]->tex = tex_map[this->model_data.models[i]->polys[j]->tex];
    delete[] tex_map;
  }
  delete[] this->model_data.zone_model->tex;
  this->model_data.zone_model->tex = tex;
  this->model_data.zone_model->tex_count = tex_count;

  this->status = 1;
  return 1;
}

// ... in my pants.

int ZonLoader::Close() {
  int i;

  if(!this->status)
    return 1;

  this->terloader.Close();
  for(i = 0; i < this->model_data.model_count; ++i)
    this->model_loaders[i].Close();
  delete[] this->model_loaders;

  return 1;
}
