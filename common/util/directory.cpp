/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
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
