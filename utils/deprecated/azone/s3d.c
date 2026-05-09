#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "s3d.h"

void decompress(char *p, char *p2, int len, int uLen) {
  int status;
  z_stream d_stream;

  d_stream.zalloc = (alloc_func)0;
  d_stream.zfree = (free_func)0;
  d_stream.opaque = (voidpf)0;
  
  d_stream.next_in  = p;
  d_stream.avail_in = len;
  d_stream.next_out = p2;
  d_stream.avail_out = uLen;
  
  inflateInit(&d_stream);
  status = inflate(&d_stream, Z_NO_FLUSH);
  inflateEnd(&d_stream);
}

int S3D_Init(s3d_object *obj, FILE *fp) {
  struct_header s3d_header;
  struct_directory_header s3d_dir_header;
  struct_directory s3d_dir;
  struct_data_block s3d_data;
  struct_fn_header *s3d_fn_header;
  struct_fn_entry *s3d_fn_entry;

  uint32 *offsets;
  
  char *temp, *temp2;
  int i, j, pos, inf, tmp, running = 0;

  obj->fp = fp;
  
  fread(&s3d_header, sizeof(struct_header), 1, fp);
  if(s3d_header.magicCookie[0] != 'P' || s3d_header.magicCookie[1] != 'F' || s3d_header.magicCookie[2] != 'S' || s3d_header.magicCookie[3] != ' ')
    return -1;
  fseek(fp, s3d_header.offset, SEEK_SET);
  fread(&s3d_dir_header, sizeof(struct_directory_header), 1, fp);
  
  obj->count = s3d_dir_header.count;
  obj->filenames = (char **) malloc(s3d_dir_header.count * sizeof(char *));
  obj->files = (uint32 *) malloc((s3d_dir_header.count - 1) * sizeof(uint32));
  offsets = (uint32 *) malloc((s3d_dir_header.count - 1) * sizeof(uint32));

  for(i = 0; i < s3d_dir_header.count; ++i) {
    fread(&s3d_dir, sizeof(struct_directory), 1, fp);

    if(s3d_dir.crc == 0x61580AC9) {
      pos = ftell(fp);
      fseek(fp, s3d_dir.offset, SEEK_SET);
      temp = (char *) malloc(s3d_dir.size);
      memset(temp, 0, s3d_dir.size);
      inf = 0;
      while(inf < s3d_dir.size) {
        fread(&s3d_data, sizeof(struct_data_block), 1, fp);
        temp2 = (char *) malloc(s3d_data.deflen);
        fread(temp2, s3d_data.deflen, 1, fp);
        decompress(temp2, temp + inf, s3d_data.deflen, s3d_data.inflen);
        free(temp2);
        inf += s3d_data.inflen;
      }
      fseek(fp, pos, SEEK_SET);
      s3d_fn_header = (struct_fn_header *) temp;
      pos = sizeof(struct_fn_header);
      for(j = 0; j < s3d_fn_header->fncount; ++j) {
        s3d_fn_entry = (struct_fn_entry *) &temp[pos];
        obj->filenames[j] = (char *) malloc(s3d_fn_entry->fnlen + 1);
        obj->filenames[j][s3d_fn_entry->fnlen] = 0;
        memcpy(obj->filenames[j], &temp[pos + sizeof(struct_fn_entry)], s3d_fn_entry->fnlen);
        pos += sizeof(struct_fn_entry) + s3d_fn_entry->fnlen;
      }
    }

    else {
      obj->files[running] = ftell(fp) - 12;
      offsets[running] = s3d_dir.offset;
      ++running;
    }

  }

  for(i = s3d_dir_header.count - 2; i > 0; i--) {
    for(j = 0; j < i; j++) {
      if(offsets[j] > offsets[j+1]) {
        tmp = offsets[j];
        offsets[j] = offsets[j + 1];
        offsets[j + 1] = tmp;
        tmp = obj->files[j];
        obj->files[j] = obj->files[j + 1];
        obj->files[j + 1] = tmp;
      }
    }
  }

  return 0;
}

size_t S3D_GetFile(s3d_object *obj, char *filename, uchar **out) {
  struct_directory s3d_dir;
  struct_data_block s3d_data;
  uchar *buf;
  char *temp2;
  long inf;
  int i;

  for(i = 0; i < obj->count; ++i) {
    if(!strcmp(obj->filenames[i], filename)) {
      fseek(obj->fp, obj->files[i], SEEK_SET);
      fread(&s3d_dir, sizeof(struct_directory), 1, obj->fp);
      fseek(obj->fp, s3d_dir.offset, SEEK_SET);
      buf = (uchar *) malloc(s3d_dir.size);
      
      inf = 0;
      while(inf < s3d_dir.size) {
        fread(&s3d_data, sizeof(struct_data_block), 1, obj->fp);
        temp2 = (char *) malloc(s3d_data.deflen);
        fread(temp2, s3d_data.deflen, 1, obj->fp);
        decompress(temp2, (char *) buf + inf, s3d_data.deflen, s3d_data.inflen);
        free(temp2);
        inf += s3d_data.inflen;
      }
      
      *out = buf;
      return inf;
    }
  }
  return -1;
}

