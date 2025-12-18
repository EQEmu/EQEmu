/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2013 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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

