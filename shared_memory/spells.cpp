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
#include "spells.h"

#include "common/eqemu_exception.h"
#include "common/ipc_mutex.h"
#include "common/memory_mapped_file.h"
#include "common/shareddb.h"
#include "common/spdat.h"

void LoadSpells(SharedDatabase *database, const std::string &prefix) {
	EQ::IPCMutex mutex("spells");
	mutex.Lock();
	int records = database->GetMaxSpellID() + 1;
	if(records == 0) {
		EQ_EXCEPT("Shared Memory", "Unable to get any spells from the database.");
	}

	uint32 size = records * sizeof(SPDat_Spell_Struct) + sizeof(uint32);

	auto Config = EQEmuConfig::get();
	std::string file_name = Config->SharedMemDir + prefix + std::string("spells");
	EQ::MemoryMappedFile mmf(file_name, size);
	mmf.ZeroFile();

	void *ptr = mmf.Get();
	database->LoadSpells(ptr, records);
	mutex.Unlock();
}

