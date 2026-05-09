#ifndef __AZONE_COMMON_ARCHIVE_H
#define __AZONE_COMMON_ARCHIVE_H

#include <list>
#include <string>
#include <stdint.h>

class Archive {
public:
    Archive() { }
    virtual ~Archive() { }
    
    virtual bool Open(std::string filename) = 0;
    virtual bool Close() = 0;
    virtual bool Get(std::string filename, char** buffer, size_t& buffer_size) = 0;
    virtual bool Exists(std::string filename) = 0;
    virtual bool GetFiles(std::string ext, std::list<std::string>& files) = 0;
};

#endif
