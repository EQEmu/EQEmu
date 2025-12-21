
#include "database.h"

#include "common/eqemu_logsys.h"
#include "common/strings.h"

// this function does not delete the ServerPacket, so it must be handled at call site
void QSDatabase::GeneralQueryReceive(ServerPacket *pack)
{
	/*
		These are general queries passed from anywhere in zone instead of packing structures and breaking them down again and again
	*/
	auto queryBuffer = new char[pack->ReadUInt32() + 1];
	pack->ReadString(queryBuffer);

	std::string query(queryBuffer);
	auto        results = QueryDatabase(query);
	if (!results.Success()) {
		LogInfo("Failed Delete Log Record Insert: [{}]", results.ErrorMessage().c_str());
		LogInfo("[{}]", query.c_str());
	}

	safe_delete_array(queryBuffer);
}
