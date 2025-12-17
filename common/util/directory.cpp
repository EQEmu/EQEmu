#include "directory.h"

#ifdef _WIN32
#include "common/util/win_dirent.h"
#else
#include <dirent.h>
#endif

struct EQ::Directory::impl {
	DIR *m_dir;
};

EQ::Directory::Directory(const std::string &path)
{
	m_impl = new impl;
	m_impl->m_dir = opendir(path.c_str());
}

EQ::Directory::~Directory()
{
	if (m_impl->m_dir) {
		closedir(m_impl->m_dir);
	}

	delete m_impl;
}

bool EQ::Directory::Exists()
{
	return m_impl->m_dir != nullptr;
}

void EQ::Directory::GetFiles(std::vector<std::string>& files)
{
	if (m_impl->m_dir) {
		struct dirent *ent;
		while ((ent = readdir(m_impl->m_dir)) != nullptr) {
			switch (ent->d_type) {
			case DT_REG:
				files.push_back(ent->d_name);
				break;
			default:
				break;
			}
		}

		rewinddir(m_impl->m_dir);
	}
}
