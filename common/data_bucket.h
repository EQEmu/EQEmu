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
#pragma once

#include "common/json/json_archive_single_line.h"
#include "common/repositories/data_buckets_repository.h"
#include "common/shareddb.h"
#include "common/types.h"

#include <string>

struct DataBucketKey {
	std::string key;
	std::string value;
	std::string expires;
	uint64_t    account_id   = 0;
	uint64_t    character_id = 0;
	uint32_t    npc_id       = 0;
	uint32_t    bot_id       = 0;
	uint16_t    zone_id      = 0;
	uint16_t    instance_id  = 0;
};

namespace DataBucketLoadType {
	enum Type : uint8 {
		Bot,
		Account,
		Client,
		Zone,
		MaxType
	};

	static const std::string Name[Type::MaxType] = {
		"Bot",
		"Account",
		"Client",
		"Zone"
	};
}

class DataBucket {
public:
	// non-scoped bucket methods (for global buckets)
	static void SetData(SharedDatabase *database, const std::string &bucket_key, const std::string &bucket_value, std::string expires_time = "");
	static bool DeleteData(SharedDatabase *database, const std::string &bucket_key);
	static std::string GetData(SharedDatabase *database, const std::string &bucket_key);
	static std::string GetDataExpires(SharedDatabase *database, const std::string &bucket_key);
	static std::string GetDataRemaining(SharedDatabase *database, const std::string &bucket_key);

	// scoped bucket methods
	static void SetData(SharedDatabase *database, const DataBucketKey &k_);
	static bool DeleteData(SharedDatabase *database, const DataBucketKey &k);
	static DataBucketsRepository::DataBuckets GetData(SharedDatabase *database, const DataBucketKey &k_, bool ignore_misses_cache = false);
	static std::string GetDataExpires(SharedDatabase *database, const DataBucketKey &k);
	static std::string GetDataRemaining(SharedDatabase *database, const DataBucketKey &k);
	static std::string GetScopedDbFilters(const DataBucketKey &k);

	// bucket repository versus key matching
	static bool CheckBucketMatch(const DataBucketsRepository::DataBuckets &dbe, const DataBucketKey &k);
	static bool ExistsInCache(const DataBucketsRepository::DataBuckets &entry);

	static void LoadZoneCache(SharedDatabase* database, uint16 zone_id, uint16 instance_id);
	static void BulkLoadEntitiesToCache(SharedDatabase* database, DataBucketLoadType::Type t, std::vector<uint32> ids);
	static void DeleteCachedBuckets(DataBucketLoadType::Type type, uint32 id, uint32 secondary_id = 0);

	static void DeleteFromMissesCache(DataBucketsRepository::DataBuckets e);
	static void ClearCache();
	static void DeleteFromCache(uint64 id, DataBucketLoadType::Type type);
	static void DeleteZoneFromCache(uint16 zone_id, uint16 instance_id, DataBucketLoadType::Type type);
	static bool CanCache(const DataBucketKey &key);
	static DataBucketsRepository::DataBuckets
	ExtractNestedValue(const DataBucketsRepository::DataBuckets &bucket, const std::string &full_key);
};
