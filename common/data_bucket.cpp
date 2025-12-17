#include "data_bucket.h"

#include "common/data_bucket.h"
#include "common/database.h"
#include "common/json/json.hpp"

#include <cctype>
#include <ctime>

using json = nlohmann::json;

const std::string                               NESTED_KEY_DELIMITER = ".";
std::vector<DataBucketsRepository::DataBuckets> g_data_bucket_cache  = {};

void DataBucket::SetData(SharedDatabase* database, const std::string &bucket_key, const std::string &bucket_value, std::string expires_time)
{
	auto k = DataBucketKey{
		.key = bucket_key,
		.value = bucket_value,
		.expires = expires_time,
	};

	DataBucket::SetData(database, k);
}

void DataBucket::SetData(SharedDatabase *database, const DataBucketKey &k_)
{
	DataBucketKey k = k_; // copy the key so we can modify it
	bool is_nested = k.key.find(NESTED_KEY_DELIMITER) != std::string::npos;
	if (is_nested) {
		k.key = Strings::Split(k.key, NESTED_KEY_DELIMITER).front();
	}

	auto b = DataBucketsRepository::NewEntity();
	auto r = GetData(database, k, true);
	// if we have an entry, use it
	if (r.id > 0) {
		b = r;
	}

	// add scoping to bucket
	if (k.character_id > 0) {
		b.character_id = k.character_id;
	}
	else if (k.account_id > 0) {
		b.account_id = k.account_id;
	}
	else if (k.npc_id > 0) {
		b.npc_id = k.npc_id;
	}
	else if (k.bot_id > 0) {
		b.bot_id = k.bot_id;
	} else if (k.zone_id > 0) {
		b.zone_id = k.zone_id;
		b.instance_id = k.instance_id;
	}

	const uint64 bucket_id         = b.id;
	int64        expires_time_unix = 0;

	if (!k.expires.empty()) {
		expires_time_unix = static_cast<int64>(std::time(nullptr)) + Strings::ToInt(k.expires);
		if (isalpha(k.expires[0]) || isalpha(k.expires[k.expires.length() - 1])) {
			expires_time_unix = static_cast<int64>(std::time(nullptr)) + Strings::TimeToSeconds(k.expires);
		}
		if (is_nested) {
			LogDataBuckets("Nested keys can't expire; set expiration on the parent key");
			expires_time_unix = 0;
		}
	}

	b.expires = expires_time_unix;
	b.value   = k.value;
	b.key_    = k.key;

	// Check for nested keys (keys with dots)
	if (k_.key.find(NESTED_KEY_DELIMITER) != std::string::npos) {
		// Retrieve existing JSON or create a new one
		std::string existing_value = r.id > 0 ? r.value : "{}";
		json json_value = json::object();

		// Check if the JSON is valid
		if (Strings::IsValidJson(existing_value)) {
			try {
				json_value = json::parse(existing_value);
			} catch (json::parse_error &e) {
				LogDataBuckets("Failed to parse JSON for key [{}] [{}]", k_.key, e.what());
				json_value = json::object(); // Reset to an empty object on error
			}
		}

		// Recursively merge new key-value pair into the JSON object
		auto nested_keys = Strings::Split(k_.key, NESTED_KEY_DELIMITER);
		auto top_key = nested_keys.front();
		// remove the top-level key
		nested_keys.erase(nested_keys.begin());

		json *current = &json_value;

		for (size_t i = 0; i < nested_keys.size(); ++i) {
			const std::string &key_part = nested_keys[i];

			if (i == nested_keys.size() - 1) {

				LogDataBucketsDetail("Setting key [{}] key_part [{}]", k.key, key_part);

				// If the key already exists and is an object or array, prevent overwriting to avoid data loss
				if (current->contains(key_part) &&
					((*current)[key_part].is_object() || (*current)[key_part].is_array())) {
					LogDataBuckets("Attempted to overwrite an existing object or array at key [{}] - skipping", k_.key);
					return;
				}

				// Set the value at the final key
				(*current)[key_part] = k_.value;
			} else {
				// Traverse or create nested objects
				if (!current->contains(key_part)) {
					(*current)[key_part] = json::object();
					LogDataBucketsDetail("Creating nested root key [{}] key_part [{}]", k.key, key_part);
				} else if (!(*current)[key_part].is_object()) {
					// If key exists but is not an object, reset to object to avoid conflicts
					(*current)[key_part] = json::object();
				}
				current = &(*current)[key_part];
			}
		}

		// Serialize JSON back to string
		b.value = json_value.dump();
		b.key_ = top_key; // Use the top-level key
	}

	if (bucket_id) {
		// update the cache if it exists
		if (CanCache(k)) {
			for (auto &e: g_data_bucket_cache) {
				if (CheckBucketMatch(e, k)) {
					e = b;
					break;
				}
			}
		}

		DataBucketsRepository::UpdateOne(*database, b);
	}
	else {
		b = DataBucketsRepository::InsertOne(*database, b);

		// add to cache if it doesn't exist
		if (CanCache(k) && !ExistsInCache(b)) {
			DeleteFromMissesCache(b);
			g_data_bucket_cache.emplace_back(b);
		}
	}
}

std::string DataBucket::GetData(SharedDatabase* database, const std::string &bucket_key)
{
	return GetData(database, DataBucketKey{.key = bucket_key}).value;
}

DataBucketsRepository::DataBuckets DataBucket::ExtractNestedValue(
	const DataBucketsRepository::DataBuckets &bucket,
	const std::string &full_key)
{
	auto nested_keys = Strings::Split(full_key, NESTED_KEY_DELIMITER);
	auto top_key = nested_keys.front();
	nested_keys.erase(nested_keys.begin());
	json json_value;

	// Check if the JSON is valid
	if (!Strings::IsValidJson(bucket.value)) {
		LogDataBuckets("Invalid JSON for key [{}]", bucket.key_);
		return DataBucketsRepository::NewEntity();
	}

	try {
		json_value = json::parse(bucket.value); // Parse the JSON
	} catch (json::parse_error &ex) {
		LogDataBuckets("Failed to parse JSON for key [{}] [{}]", bucket.key_, ex.what());
		return DataBucketsRepository::NewEntity(); // Return empty entity on parse error
	}

	// Start from the top-level key (e.g., "progression")
	json *current = &json_value;

	// Traverse the JSON structure
	for (const auto &key_part: nested_keys) {
		LogDataBuckets("Looking for key part [{}] in JSON", key_part);

		if (!current->contains(key_part)) {
			LogDataBuckets("Key part [{}] not found in JSON for [{}]", key_part, full_key);
			return DataBucketsRepository::NewEntity();
		}

		current = &(*current)[key_part];
	}

	// Create a new entity with the extracted value
	DataBucketsRepository::DataBuckets result = bucket; // Copy the original bucket
	result.value = current->is_string() ? current->get<std::string>() : current->dump();
	return result;
}

// GetData fetches bucket data from the database or cache if it exists
// if the bucket doesn't exist, it will be added to the cache as a miss
// if ignore_misses_cache is true, the bucket will not be added to the cache as a miss
// the only place we should be ignoring the misses cache is on the initial read during SetData
DataBucketsRepository::DataBuckets DataBucket::GetData(SharedDatabase* database, const DataBucketKey &k_, bool ignore_misses_cache)
{
	DataBucketKey k = k_; // Copy the key so we can modify it

	bool is_nested_key = k.key.find(NESTED_KEY_DELIMITER) != std::string::npos;

	// Extract the top-level key for nested keys
	if (is_nested_key) {
		k.key = Strings::Split(k.key, NESTED_KEY_DELIMITER).front();
	}

	LogDataBuckets(
		"Getting bucket key [{}] bot_id [{}] account_id [{}] character_id [{}] npc_id [{}] zone_id [{}] instance_id [{}]",
		k.key,
		k.bot_id,
		k.account_id,
		k.character_id,
		k.npc_id,
		k.zone_id,
		k.instance_id
	);

	bool can_cache = CanCache(k);

	// Attempt to retrieve the value from the cache
	if (can_cache) {
		for (const auto &e : g_data_bucket_cache) {
			if (CheckBucketMatch(e, k)) {
				if (e.expires > 0 && e.expires < std::time(nullptr)) {
					LogDataBuckets("Attempted to read expired key [{}] removing from cache", e.key_);
					DeleteData(database, k);
					return DataBucketsRepository::NewEntity();
				}

				LogDataBuckets("Returning key [{}] value [{}] from cache", e.key_, e.value);

				if (is_nested_key && !k_.key.empty()) {
					return ExtractNestedValue(e, k_.key);
				}

				return e;
			}
		}
	}

	// Fetch the value from the database
	auto r = DataBucketsRepository::GetWhere(
		*database,
		fmt::format(
			" {} `key` = '{}' LIMIT 1",
			DataBucket::GetScopedDbFilters(k),
			k.key
		)
	);

	if (r.empty()) {
		// Handle cache misses
		if (!ignore_misses_cache && can_cache) {
			size_t size_before = g_data_bucket_cache.size();

			g_data_bucket_cache.emplace_back(
				DataBucketsRepository::DataBuckets{
					.id = 0,
					.key_ = k.key,
					.value = "",
					.expires = 0,
					.account_id = k.account_id,
					.character_id = k.character_id,
					.npc_id = k.npc_id,
					.bot_id = k.bot_id,
					.zone_id = k.zone_id,
					.instance_id = k.instance_id
				}
			);

			LogDataBuckets(
				"Key [{}] not found in database, adding to cache as a miss account_id [{}] character_id [{}] npc_id [{}] bot_id [{}] zone_id [{}] instance_id [{}] cache size before [{}] after [{}]",
				k.key,
				k.account_id,
				k.character_id,
				k.npc_id,
				k.bot_id,
				k.zone_id,
				k.instance_id,
				size_before,
				g_data_bucket_cache.size()
			);
		}

		return DataBucketsRepository::NewEntity();
	}

	auto bucket = r.front();

	// If the entry has expired, delete it
	if (bucket.expires > 0 && bucket.expires < static_cast<long long>(std::time(nullptr))) {
		DeleteData(database, k);
		return DataBucketsRepository::NewEntity();
	}

	// Add the value to the cache if it doesn't exist
	if (can_cache) {
		bool has_cache = false;
		for (const auto &e : g_data_bucket_cache) {
			if (e.id == bucket.id) {
				has_cache = true;
				break;
			}
		}

		if (!has_cache) {
			g_data_bucket_cache.emplace_back(bucket);
		}
	}

	// Handle nested key extraction
	if (is_nested_key && !k_.key.empty()) {
		return ExtractNestedValue(bucket, k_.key);
	}

	return bucket;
}

std::string DataBucket::GetDataExpires(SharedDatabase* database, const std::string &bucket_key)
{
	return GetDataExpires(database, DataBucketKey{.key = bucket_key});
}

std::string DataBucket::GetDataRemaining(SharedDatabase* database, const std::string &bucket_key)
{
	return GetDataRemaining(database, DataBucketKey{.key = bucket_key});
}

bool DataBucket::DeleteData(SharedDatabase* database, const std::string &bucket_key)
{
	return DeleteData(database, DataBucketKey{.key = bucket_key});
}

bool DataBucket::DeleteData(SharedDatabase* database, const DataBucketKey &k)
{
	bool is_nested_key = k.key.find(NESTED_KEY_DELIMITER) != std::string::npos;

	if (!is_nested_key) {
		// Update cache
		if (CanCache(k)) {
			// delete from cache where contents match
			g_data_bucket_cache.erase(
				std::remove_if(
					g_data_bucket_cache.begin(),
					g_data_bucket_cache.end(),
					[&](DataBucketsRepository::DataBuckets &e) {
						return CheckBucketMatch(e, k);
					}
				),
				g_data_bucket_cache.end()
			);
		}

		// Regular key deletion, no nesting involved
		return DataBucketsRepository::DeleteWhere(
			*database,
			fmt::format("{} `key` = '{}'", DataBucket::GetScopedDbFilters(k), k.key)
		);
	}

	// If it's a nested key, retrieve the top-level JSON object
	auto top_level_key = Strings::Split(k.key, NESTED_KEY_DELIMITER).front();
	DataBucketKey top_level_k = k;
	top_level_k.key = top_level_key;

	auto r = GetData(database, top_level_k);
	if (r.id == 0 || r.value.empty() || !Strings::IsValidJson(r.value)) {
		LogDataBuckets("Attempted to delete nested key [{}] but parent key [{}] does not exist or is invalid JSON", k.key, top_level_key);
		return false;
	}

	json json_value;
	try {
		json_value = json::parse(r.value);
	} catch (json::parse_error &ex) {
		LogDataBuckets("Failed to parse JSON for key [{}] [{}]", top_level_key, ex.what());
		return false;
	}

	// Recursively remove the nested key
	auto nested_keys = Strings::Split(k.key, NESTED_KEY_DELIMITER);
	auto top_key = nested_keys.front();
	nested_keys.erase(nested_keys.begin());
	json *current = &json_value;

	for (size_t i = 0; i < nested_keys.size(); ++i) {
		const std::string &key_part = nested_keys[i];

		if (i == nested_keys.size() - 1) {
			// Last key in the hierarchy - delete it
			if (current->contains(key_part)) {
				current->erase(key_part);
				LogDataBuckets("Deleted nested key [{}] from [{}]", key_part, k.key);
			} else {
				LogDataBuckets("Key [{}] not found in JSON - nothing to delete", k.key);
				return false;
			}
		} else {
			if (!current->contains(key_part) || !(*current)[key_part].is_object()) {
				LogDataBuckets("Parent key [{}] does not exist or is not an object", key_part);
				return false;
			}
			current = &(*current)[key_part];
		}
	}

	// If the JSON object is now empty, delete the top-level key
	if (json_value.empty()) {
		LogDataBuckets("Top-level key [{}] is now empty, deleting entire entry", top_level_key);

		// delete cache
		if (CanCache(k)) {
			g_data_bucket_cache.erase(
				std::remove_if(
					g_data_bucket_cache.begin(),
					g_data_bucket_cache.end(),
					[&](DataBucketsRepository::DataBuckets &e) {
						return CheckBucketMatch(e, top_level_k);
					}
				),
				g_data_bucket_cache.end()
			);
		}

		return DataBucketsRepository::DeleteWhere(
			*database,
			fmt::format("{} `key` = '{}'", DataBucket::GetScopedDbFilters(k), top_level_key)
		);
	}

	// Otherwise, update the existing JSON without the deleted key
	r.value = json_value.dump();
	DataBucketsRepository::UpdateOne(*database, r);

	// Update cache
	if (CanCache(k)) {
		for (auto &e : g_data_bucket_cache) {
			if (CheckBucketMatch(e, top_level_k)) {
				e.value = r.value;
				break;
			}
		}
	}

	return true;
}

std::string DataBucket::GetDataExpires(SharedDatabase* database, const DataBucketKey &k)
{
	LogDataBuckets(
		"Getting bucket expiration key [{}] bot_id [{}] account_id [{}] character_id [{}] npc_id [{}]",
		k.key,
		k.bot_id,
		k.account_id,
		k.character_id,
		k.npc_id
	);

	auto r = GetData(database, k);
	if (r.id == 0) {
		return {};
	}

	return std::to_string(r.expires);
}

std::string DataBucket::GetDataRemaining(SharedDatabase* database, const DataBucketKey &k)
{
	LogDataBuckets(
		"Getting bucket remaining key [{}] bot_id [{}] account_id [{}] character_id [{}] npc_id [{}] bot_id [{}] zone_id [{}] instance_id [{}]",
		k.key,
		k.bot_id,
		k.account_id,
		k.character_id,
		k.npc_id,
		k.bot_id,
		k.zone_id,
		k.instance_id
	);

	auto r = GetData(database, k);
	if (r.id == 0) {
		return "0";
	}

	return fmt::format("{}", r.expires - (long long) std::time(nullptr));
}

std::string DataBucket::GetScopedDbFilters(const DataBucketKey &k)
{
	std::vector<std::string> q = {};
	if (k.character_id > 0) {
		q.emplace_back(fmt::format("character_id = {}", k.character_id));
	}
	else {
		q.emplace_back("character_id = 0");
	}

	if (k.account_id > 0) {
		q.emplace_back(fmt::format("account_id = {}", k.account_id));
	}
	else {
		q.emplace_back("account_id = 0");
	}

	if (k.npc_id > 0) {
		q.emplace_back(fmt::format("npc_id = {}", k.npc_id));
	}
	else {
		q.emplace_back("npc_id = 0");
	}

	if (k.bot_id > 0) {
		q.emplace_back(fmt::format("bot_id = {}", k.bot_id));
	}
	else {
		q.emplace_back("bot_id = 0");
	}

	if (k.zone_id > 0) {
		q.emplace_back(fmt::format("zone_id = {} AND instance_id = {}", k.zone_id, k.instance_id));
	}
	else {
		q.emplace_back("zone_id = 0 AND instance_id = 0");
	}

	return fmt::format(
		"{} {}",
		Strings::Join(q, " AND "),
		!q.empty() ? "AND" : ""
	);
}

bool DataBucket::CheckBucketMatch(const DataBucketsRepository::DataBuckets &dbe, const DataBucketKey &k)
{
	return (
		dbe.key_ == k.key &&
		dbe.bot_id == k.bot_id &&
		dbe.account_id == k.account_id &&
		dbe.character_id == k.character_id &&
		dbe.npc_id == k.npc_id &&
		dbe.zone_id == k.zone_id &&
		dbe.instance_id == k.instance_id
	);
}

void DataBucket::LoadZoneCache(SharedDatabase* database, uint16 zone_id, uint16 instance_id)
{
	const auto &l = DataBucketsRepository::GetWhere(
		*database,
		fmt::format(
			"zone_id = {} AND instance_id = {} AND (`expires` > {} OR `expires` = 0)",
			zone_id,
			instance_id,
			(long long) std::time(nullptr)
		)
	);

	if (l.empty()) {
		return;
	}

	LogDataBucketsDetail("cache size before [{}] l size [{}]", g_data_bucket_cache.size(), l.size());

	uint32 added_count = 0;

	for (const auto &e: l) {
		if (!ExistsInCache(e)) {
			added_count++;
		}
	}

	for (const auto &e: l) {
		if (!ExistsInCache(e)) {
			LogDataBucketsDetail("bucket id [{}] bucket key [{}] bucket value [{}]", e.id, e.key_, e.value);

			g_data_bucket_cache.emplace_back(e);
		}
	}

	LogDataBucketsDetail("cache size after [{}]", g_data_bucket_cache.size());

	LogDataBuckets(
		"Loaded [{}] zone keys new cache size is [{}]",
		l.size(),
		g_data_bucket_cache.size()
	);
}

void DataBucket::BulkLoadEntitiesToCache(SharedDatabase* database, DataBucketLoadType::Type t, std::vector<uint32> ids)
{
	if (ids.empty()) {
		return;
	}

	if (ids.size() == 1) {
		bool has_cache = false;

		for (const auto &e: g_data_bucket_cache) {
			if (t == DataBucketLoadType::Bot) {
				has_cache = e.bot_id == ids[0];
			}
			else if (t == DataBucketLoadType::Account) {
				has_cache = e.account_id == ids[0];
			}
			else if (t == DataBucketLoadType::Client) {
				has_cache = e.character_id == ids[0];
			}
		}

		if (has_cache) {
			LogDataBucketsDetail("LoadType [{}] ID [{}] has cache", DataBucketLoadType::Name[t], ids[0]);
			return;
		}
	}

	std::string column;

	switch (t) {
		case DataBucketLoadType::Bot:
			column = "bot_id";
			break;
		case DataBucketLoadType::Client:
			column = "character_id";
			break;
		case DataBucketLoadType::Account:
			column = "account_id";
			break;
		default:
			LogError("Incorrect LoadType [{}]", static_cast<int>(t));
			break;
	}

	const auto &l = DataBucketsRepository::GetWhere(
		*database,
		fmt::format(
			"{} IN ({}) AND (`expires` > {} OR `expires` = 0)",
			column,
			Strings::Join(ids, ", "),
			(long long) std::time(nullptr)
		)
	);

	if (l.empty()) {
		return;
	}

	LogDataBucketsDetail("cache size before [{}] l size [{}]", g_data_bucket_cache.size(), l.size());

	uint32 added_count = 0;

	for (const auto &e: l) {
		if (!ExistsInCache(e)) {
			added_count++;
		}
	}

	for (const auto &e: l) {
		if (!ExistsInCache(e)) {
			LogDataBucketsDetail("bucket id [{}] bucket key [{}] bucket value [{}]", e.id, e.key_, e.value);

			g_data_bucket_cache.emplace_back(e);
		}
	}

	LogDataBucketsDetail("cache size after [{}]", g_data_bucket_cache.size());

	LogDataBuckets(
		"Bulk Loaded ids [{}] column [{}] new cache size is [{}]",
		ids.size(),
		column,
		g_data_bucket_cache.size()
	);
}

void DataBucket::DeleteCachedBuckets(DataBucketLoadType::Type type, uint32 id, uint32 secondary_id)
{
	size_t size_before = g_data_bucket_cache.size();

	g_data_bucket_cache.erase(
		std::remove_if(
			g_data_bucket_cache.begin(),
			g_data_bucket_cache.end(),
			[&](DataBucketsRepository::DataBuckets &e) {
				return (
					(type == DataBucketLoadType::Bot && e.bot_id == id) ||
					(type == DataBucketLoadType::Account && e.account_id == id) ||
					(type == DataBucketLoadType::Client && e.character_id == id) ||
					(type == DataBucketLoadType::Zone && e.zone_id == id && e.instance_id == secondary_id)
				);
			}
		),
		g_data_bucket_cache.end()
	);

	LogDataBuckets(
		"LoadType [{}] id [{}] cache size before [{}] after [{}]",
		DataBucketLoadType::Name[type],
		id,
		size_before,
		g_data_bucket_cache.size()
	);
}

bool DataBucket::ExistsInCache(const DataBucketsRepository::DataBuckets &entry)
{
	for (const auto &e: g_data_bucket_cache) {
		if (e.id == entry.id) {
			return true;
		}
	}

	return false;
}

void DataBucket::DeleteFromMissesCache(DataBucketsRepository::DataBuckets e)
{
	// delete from cache where there might have been a written bucket miss to the cache
	// this is to prevent the cache from growing too large
	size_t size_before = g_data_bucket_cache.size();

	g_data_bucket_cache.erase(
		std::remove_if(
			g_data_bucket_cache.begin(),
			g_data_bucket_cache.end(),
			[&](DataBucketsRepository::DataBuckets &ce) {
				return ce.id == 0 && ce.key_ == e.key_ &&
					   ce.account_id == e.account_id &&
					   ce.character_id == e.character_id &&
					   ce.npc_id == e.npc_id &&
					   ce.bot_id == e.bot_id &&
					   ce.zone_id == e.zone_id &&
					   ce.instance_id == e.instance_id;
			}
		),
		g_data_bucket_cache.end()
	);
	LogDataBucketsDetail(
		"Deleted bucket misses from cache where key [{}] size before [{}] after [{}]",
		e.key_,
		size_before,
		g_data_bucket_cache.size()
	);
}

void DataBucket::ClearCache()
{
	g_data_bucket_cache.clear();
	LogInfo("Cleared data buckets cache");
}

void DataBucket::DeleteFromCache(uint64 id, DataBucketLoadType::Type type)
{
	size_t size_before = g_data_bucket_cache.size();

	g_data_bucket_cache.erase(
		std::remove_if(
			g_data_bucket_cache.begin(),
			g_data_bucket_cache.end(),
			[&](DataBucketsRepository::DataBuckets &e) {
				switch (type) {
					case DataBucketLoadType::Bot:
						return e.bot_id == id;
					case DataBucketLoadType::Client:
						return e.character_id == id;
					case DataBucketLoadType::Account:
						return e.account_id == id;
					default:
						return false;
				}
			}
		),
		g_data_bucket_cache.end()
	);

	LogDataBuckets(
		"Deleted [{}] id [{}] from cache size before [{}] after [{}]",
		DataBucketLoadType::Name[type],
		id,
		size_before,
		g_data_bucket_cache.size()
	);
}

void DataBucket::DeleteZoneFromCache(uint16 zone_id, uint16 instance_id, DataBucketLoadType::Type type)
{
	size_t size_before = g_data_bucket_cache.size();

	g_data_bucket_cache.erase(
		std::remove_if(
			g_data_bucket_cache.begin(),
			g_data_bucket_cache.end(),
			[&](DataBucketsRepository::DataBuckets &e) {
				switch (type) {
					case DataBucketLoadType::Zone:
						return e.zone_id == zone_id && e.instance_id == instance_id;
					default:
						return false;
				}
			}
		),
		g_data_bucket_cache.end()
	);

	LogDataBuckets(
		"Deleted zone [{}] instance [{}] from cache size before [{}] after [{}]",
		zone_id,
		instance_id,
		size_before,
		g_data_bucket_cache.size()
	);
}

// CanCache returns whether a bucket can be cached or not
// characters are only in one zone at a time so we can cache locally to the zone
// bots (not implemented) are only in one zone at a time so we can cache locally to the zone
// npcs (ids) can be in multiple zones so we can't cache locally to the zone
bool DataBucket::CanCache(const DataBucketKey &key)
{
	if (key.character_id > 0 || key.account_id > 0 || key.bot_id > 0 || key.zone_id > 0) {
		return true;
	}

	return false;
}
