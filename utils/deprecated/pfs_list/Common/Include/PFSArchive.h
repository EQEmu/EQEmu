#ifndef __AZONE_COMMON_PFSARCHIVE_H
#define __AZONE_COMMON_PFSARCHIVE_H

#include <stdio.h>
#include <string.h>
#include <list>
#include <vector>
#ifdef WIN32
#include <Winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include "Archive.h"
#include "PFSDataStructs.h"
#include "Compression.h"

class PFSArchive : public Archive {
public:
    PFSArchive();
    virtual ~PFSArchive();
    
    virtual bool Open(std::string filename);
    virtual bool Close();
    virtual bool Get(std::string filename, char **buffer, size_t& buffer_size);
    virtual bool Exists(std::string filename);
    virtual bool GetFiles(std::string ext, std::list<std::string>& files);
private:
    bool ReadIntoBuffer(std::string filename);
    std::vector<std::string> _filenames;
    std::vector<size_t> _files;
    char* _buffer;
    size_t _buffer_size;
};

#endif
