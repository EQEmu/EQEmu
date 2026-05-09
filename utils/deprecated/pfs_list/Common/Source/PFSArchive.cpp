#include "PFSArchive.h"

#define BufferRead(x, y) x = (y*)&_buffer[position]; position += sizeof(y);
#define BufferReadLength(x, y) memcpy(x, &_buffer[position], y); position += y;
#define MAX_FILENAME_SIZE 1024

PFSArchive::PFSArchive()
:_buffer(NULL), _buffer_size(0)
{
}

PFSArchive::~PFSArchive() {
    Close();
}

bool PFSArchive::Open(std::string filename)
{
    if(!ReadIntoBuffer(filename)) {
        Close();
        return false;
    }

    PFSHeader *header = NULL;
    PFSDirectoryHeader *directory_header = NULL;
    PFSDirectory *directory = NULL;
    PFSDataBlock *data_block = NULL;
    PFSFilenameHeader *filename_header = NULL;
    PFSFilenameEntry *filename_entry = NULL;
    size_t position = 0;

    BufferRead(header, PFSHeader);

    if(header->magic[0] != 'P' ||
        header->magic[1] != 'F' ||
        header->magic[2] != 'S' ||
        header->magic[3] != ' ') 
    {
        Close();
        return false;
    }

    position = header->offset;
    BufferRead(directory_header, PFSDirectoryHeader);

    std::vector<uint32_t> offsets(directory_header->count, 0);
    _filenames.resize(directory_header->count);
    _files.resize(directory_header->count);

    size_t i = 0;
    size_t j = 0;
    size_t running = 0;
    size_t temp_position = 0;
    size_t inflate = 0;
    char temp_buffer[32768];
    char temp_buffer2[32768];
    char temp_string[MAX_FILENAME_SIZE];
    for(; i < directory_header->count; ++i) {
        BufferRead(directory, PFSDirectory);
        if(directory->crc == ntohl(0xC90A5861)) {
            temp_position = position;
            position = directory->offset;
            memset(temp_buffer, 0, directory->size);
            inflate = 0;
            
            while(inflate < directory->size) {
                BufferRead(data_block, PFSDataBlock);
                BufferReadLength(temp_buffer2, data_block->deflate_length);
                decompress(temp_buffer2, data_block->deflate_length, temp_buffer + inflate, data_block->inflate_length);
				inflate += data_block->inflate_length;
            }

            position = temp_position;
            filename_header = (PFSFilenameHeader*)&temp_buffer[0];
            temp_position = sizeof(PFSFilenameHeader);
            
            for(j = 0; j < filename_header->filename_count; ++j)
			{
				filename_entry = (PFSFilenameEntry*)&temp_buffer[temp_position];
                if(filename_entry->filename_length + 1 >= MAX_FILENAME_SIZE) {
                    Close();
                    return false;
                }
				temp_string[filename_entry->filename_length] = 0;
				memcpy(temp_string, &temp_buffer[temp_position + sizeof(PFSFilenameEntry)], filename_entry->filename_length);
                _filenames[j] = temp_string;
				temp_position += sizeof(PFSFilenameEntry) + filename_entry->filename_length;
			}
        } else {
            _files[running] = position - 12;
			offsets[running] = directory->offset;
			++running;
        }
    }

    uint32_t temp = 0;
    for(i = directory_header->count - 2; i > 0; i--) {
        for(j = 0; j < i; j++) {
            if(offsets[j] > offsets[j + 1]) {
                temp = offsets[j];
				offsets[j] = offsets[j + 1];
				offsets[j + 1] = temp;
				temp = _files[j];
				_files[j] = _files[j + 1];
				_files[j + 1] = temp;
            }
        }
    }

    return true;
}

bool PFSArchive::Close()
{
    if(_buffer) {
        delete[] _buffer;
        _buffer = 0;
        _buffer_size = 0;
        _filenames.resize(0);
        _files.resize(0);
        return true;
    }
	return false;
}

//I would love to get rid of the allocations in the while loop below but sadly 
//I can't predict the size of data blocks well enough to be able to and feel 
//comfortable with it (I could however reduce the number of times I need to 
//reallocate with a little clever logic which i think I will do
bool PFSArchive::Get(std::string filename, char **buffer, size_t& buffer_size) {
    size_t sz = _filenames.size();
    for(size_t index = 0; index < sz; ++index) {
        if(!_filenames[index].compare(filename)) {
            PFSDirectory* directory = NULL;
            PFSDataBlock* data_block = NULL;
            char *temp = NULL;

            size_t position = _files[index];
            BufferRead(directory, PFSDirectory);
            position = directory->offset;

            *buffer = new char[directory->size];
            buffer_size = directory->size;

            size_t inflate = 0;
            while(inflate < directory->size) {
                BufferRead(data_block, PFSDataBlock);
                temp = new char[data_block->deflate_length];
                
                memcpy(temp, &_buffer[position], data_block->deflate_length);
                position += data_block->deflate_length;
                
                decompress(temp, data_block->deflate_length, *buffer + inflate, data_block->inflate_length);
                delete[] temp;
                inflate += data_block->inflate_length;
            }
            buffer_size = inflate;
            return true;
        }
    }

    return false;
}

bool PFSArchive::Exists(std::string filename) {
    size_t count = _filenames.size();
    for(size_t i = 0; i < count; ++i) {
        if(!_filenames[i].compare(filename.c_str()))
            return true;
    }
    return false;
}

bool PFSArchive::GetFiles(std::string ext, std::list<std::string>& files) {
    int elen = ext.length();
    bool all_files = !ext.compare("*");
    files.clear();

    size_t count = _filenames.size();
    for(size_t i = 0; i < count; ++i) {
        int flen = _filenames[i].length();
        if(flen <= elen)
    	    continue;

        if(!strcmp(_filenames[i].c_str() + (flen - elen), ext.c_str()) || all_files)
            files.push_back(_filenames[i]);
    }
    
    return files.size() > 0;
}

bool PFSArchive::ReadIntoBuffer(std::string filename) {
    FILE* f = fopen(filename.c_str(), "rb");
    if(!f) {
        return false;
    }

    fseek(f, 0, SEEK_END);
    size_t total_size = ftell(f);
    rewind(f);

    if(!total_size) {
        fclose(f);
        return false;
    }

    if(_buffer) {
        delete[] _buffer;
    }

    _buffer = new char[total_size];
    size_t bytes_read = fread(_buffer, 1, total_size, f);

    if(bytes_read != total_size) {
        return false;
    }

    _buffer_size = total_size;
    fclose(f);
    return true;
}
