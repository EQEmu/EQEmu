#include <string.h>
#include <zlib.h>
#include "pfs.hpp"

#ifdef WIN32
#include <winsock.h>
#endif

#pragma pack(1)

struct struct_header {
  uint32 offset;
  char magicCookie[4];
  uint32 unknown;
} typedef struct_header;

struct struct_directory_header {
  uint32 count;
} typedef struct_directory_header;

struct struct_directory {
  uint32 crc, offset, size;
} typedef struct_directory;

struct struct_data_block {
  uint32 deflen, inflen;
} typedef struct_data_block;

struct struct_fn_header {
  uint32 fncount;
} typedef struct_fn_header;

struct struct_fn_entry {
  uint32 fnlen;
} typedef struct_fn_entry;

#pragma pack()

inline void decompress(char *p, char *p2, int len, int uLen) {
  int status;
  z_stream d_stream;

  d_stream.zalloc = (alloc_func)0;
  d_stream.zfree = (free_func)0;
  d_stream.opaque = (voidpf)0;
  
  d_stream.next_in  = (Bytef *) p;
  d_stream.avail_in = len;
  d_stream.next_out = (Bytef *) p2;
  d_stream.avail_out = uLen;
  
  inflateInit(&d_stream);
  status = inflate(&d_stream, Z_NO_FLUSH);
  inflateEnd(&d_stream);
}

inline void Lower(char *str) {
  while(*str) {
    if(*str >= 'A' && *str <= 'Z')
      *str += 'a' - 'A';
    ++str;
  }
}

PFSLoader::PFSLoader() {
  // Set the status of the loader that nothing is loaded.
  this->buffer = NULL;
  this->buf_len = -1;
  this->status = 0;
}

PFSLoader::~PFSLoader() {
  this->Close();
}

int PFSLoader::Open(FILE *fp) {
  struct_header s3d_header;
  struct_directory_header s3d_dir_header;
  struct_directory s3d_dir;
  struct_data_block s3d_data;
  struct_fn_header *s3d_fn_header;
  struct_fn_entry *s3d_fn_entry;

  uint32 *offsets;

  char *temp, *temp2;
  int i, j, pos, inf, tmp, running = 0;

  fread(&s3d_header, sizeof(struct_header), 1, fp);
  if(s3d_header.magicCookie[0] != 'P' || s3d_header.magicCookie[1] != 'F' || s3d_header.magicCookie[2] != 'S' || s3d_header.magicCookie[3] != ' ')
    return 0;

  this->fp = fp;

  fseek(fp, s3d_header.offset, SEEK_SET);
  fread(&s3d_dir_header, sizeof(struct_directory_header), 1, fp);
  
  this->count = s3d_dir_header.count - 1;
  this->filenames = new char *[s3d_dir_header.count];
  this->files = new uint32[s3d_dir_header.count - 1];
  offsets = new uint32[s3d_dir_header.count - 1];

  for(i = 0; i < (int)s3d_dir_header.count; ++i) {
    fread(&s3d_dir, sizeof(struct_directory), 1, fp);

    if(s3d_dir.crc == ntohl(0xC90A5861)) {
      pos = ftell(fp);
      fseek(fp, s3d_dir.offset, SEEK_SET);
      temp = new char[s3d_dir.size];
      memset(temp, 0, s3d_dir.size);
      inf = 0;
      while(inf < (int)s3d_dir.size) {
        fread(&s3d_data, sizeof(struct_data_block), 1, fp);
        temp2 = new char[s3d_data.deflen];
        fread(temp2, s3d_data.deflen, 1, fp);
        decompress(temp2, temp + inf, s3d_data.deflen, s3d_data.inflen);
        delete[] temp2;
        inf += s3d_data.inflen;
      }
      fseek(fp, pos, SEEK_SET);
      s3d_fn_header = (struct_fn_header *) temp;
      pos = sizeof(struct_fn_header);
      for(j = 0; j < (int)s3d_fn_header->fncount; ++j) {
        s3d_fn_entry = (struct_fn_entry *) &temp[pos];
        this->filenames[j] = new char[s3d_fn_entry->fnlen + 1];
        this->filenames[j][s3d_fn_entry->fnlen] = 0;
        memcpy(this->filenames[j], &temp[pos + sizeof(struct_fn_entry)], s3d_fn_entry->fnlen);
        pos += sizeof(struct_fn_entry) + s3d_fn_entry->fnlen;
      }
    }

    else {
      this->files[running] = ftell(fp) - 12;
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
        tmp = this->files[j];
        this->files[j] = this->files[j + 1];
        this->files[j + 1] = tmp;
      }
    }
  }

  return 1;
}

int PFSLoader::Close() {
  if(this->status) {
    while(this->count > 0) {
      delete this->filenames[this->count - 1];
      --this->count;
    }
    delete[] this->filenames;
    delete[] this->files;
  }
  else
    return 0;

  this->buffer = NULL;
  this->buf_len = -1;
  this->status = 0;

  return 1;
}

const char *PFSLoader::FindExtension(const char *ext) {
  int i;

  int elen = strlen(ext);
  
  for(i = 0; i < this->count; ++i) {
    int flen = strlen(this->filenames[i]);
	if(flen <= elen)
		continue;
    if(!strcmp(this->filenames[i]+(flen-elen), ext))
		return(this->filenames[i]);
  }
  return(NULL);
}

int PFSLoader::GetFile(char *name, uchar **buf, int *len) {
  struct_directory s3d_dir;
  struct_data_block s3d_data;
  char *temp2;
  long inf;
  int i;
  Lower(name);

  for(i = 0; i < this->count; ++i) {
    if(!strcmp(this->filenames[i], name)) {
      fseek(this->fp, this->files[i], SEEK_SET);
      fread(&s3d_dir, sizeof(struct_directory), 1, this->fp);
      fseek(this->fp, s3d_dir.offset, SEEK_SET);
      *buf = new uchar[s3d_dir.size];
      
      inf = 0;
      while(inf < (int)s3d_dir.size) {
        fread(&s3d_data, sizeof(struct_data_block), 1, this->fp);
        temp2 = new char[s3d_data.deflen];
        fread(temp2, s3d_data.deflen, 1, this->fp);
        decompress(temp2, (char *) *buf + inf, s3d_data.deflen, s3d_data.inflen);
        delete[] temp2;
        inf += s3d_data.inflen;
      }

      *len = inf;
      return 1;
    }
  }
  return 0;
}
