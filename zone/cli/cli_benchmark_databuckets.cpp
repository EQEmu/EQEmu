#include <chrono>
#include <iostream>
#include <random>
#include "../zone_cli.h"
#include "../../common/cli/eqemu_command_handler.h"
#include "../../common/http/httplib.h"
#include "../../common/eqemu_logsys.h"
#include "../sidecar_api/sidecar_api.h"
#include "../../common/platform.h"
#include "../../common/data_bucket.h"
#include "../zonedb.h"
#include "../../common/repositories/data_buckets_repository.h"

void RunBenchmarkCycle(uint64_t target_rows)
{
	const size_t      OPERATIONS_PER_TEST = 5000;
	const std::string test_key_prefix     = "test_key_";

	std::cout << Strings::Repeat("-", 70) << "\n";
	std::cout << "📊 Running Benchmark at " << Strings::Commify(target_rows) << " Rows...\n";
	std::cout << Strings::Repeat("-", 70) << "\n";

	// 🧹 **Purge `test_key_*` Keys Before Each Run**
	std::cout << "🧹 Purging test keys (`test_key_*`)...\n";
	auto purge_start = std::chrono::high_resolution_clock::now();
	DataBucketsRepository::DeleteWhere(database, "`key` LIKE '" + test_key_prefix + "%'");
	auto                          purge_end  = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> purge_time = purge_end - purge_start;
	std::cout << "✅ Purged test keys in " << purge_time.count() << " seconds.\n";

	// 📊 **Ensure the Table Contains At Least `target_rows`**
	auto     populate_start = std::chrono::high_resolution_clock::now();
	uint64_t current_count  = DataBucketsRepository::Count(database);
	if (current_count < target_rows) {
		std::cout << "📌 Populating table to " << Strings::Commify(target_rows) << " rows...\n";
		std::mt19937                       rng(std::random_device{}());
		std::uniform_int_distribution<int> entity_type(1, 5);
		std::uniform_int_distribution<int> id_dist(1, 1000000);
		std::uniform_int_distribution<int> expiry_dist(0, 86400 * 30);  // Expiry up to 30 days

		while (current_count < target_rows) {
			std::vector<DataBucketsRepository::DataBuckets> batch;
			for (size_t                                     i = 0; i < 100000; ++i) {
				if (i > target_rows - current_count) {
					break;
				}

				int         entity_choice = entity_type(rng);
				int         entity_id     = id_dist(rng);
				std::string key           = "test_key_" + std::to_string(current_count + i);
				std::string value         = "value_" + std::to_string(current_count + i);
				int         expires       = static_cast<int>(std::time(nullptr)) + expiry_dist(rng);

				DataBucketsRepository::DataBuckets e{};
				e.key_         = key;
				e.value        = value;
				e.expires      = expires;
				e.account_id   = (entity_choice == 1) ? entity_id : 0;
				e.character_id = (entity_choice == 2) ? entity_id : 0;
				e.npc_id       = (entity_choice == 3) ? entity_id : 0;
				e.bot_id       = (entity_choice == 4) ? entity_id : 0;
				e.zone_id      = (entity_choice == 5) ? entity_id : 0;
				e.instance_id  = (entity_choice == 5) ? entity_id : 0;

				batch.emplace_back(e);
			}
			DataBucketsRepository::InsertMany(database, batch);
			current_count += batch.size();
		}
	}
	else {
		std::cout << "✅ Table already has " << current_count << " rows, proceeding with benchmark.\n";
	}

	auto                          populate_end  = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> populate_time = populate_end - populate_start;
	std::cout << "✅ Populated table in " << populate_time.count() << " seconds.\n";

	std::mt19937                       rng(std::random_device{}());
	std::uniform_int_distribution<int> id_dist(1, 1000);

	// 🚀 **Measure Insert Performance**
	std::vector<DataBucketKey> inserted_keys = {};
	auto                       insert_start  = std::chrono::high_resolution_clock::now();
	for (size_t                i             = 0; i < OPERATIONS_PER_TEST; ++i) {
		std::string key     = test_key_prefix + std::to_string(current_count + i);
		std::string value   = "value_" + std::to_string(current_count + i);
		int         expires = static_cast<int>(std::time(nullptr)) + 3600;

		DataBucketKey e{
			.key = key,
			.value = value,
			.expires = std::to_string(expires),
			.account_id = 0,
			.character_id = 0,
			.npc_id = 0,
			.bot_id = 0
		};

		// randomly set account_id, character_id, npc_id, or bot_id
		switch (i % 4) {
			case 0:
				e.account_id = id_dist(rng);
				break;
			case 1:
				e.character_id = id_dist(rng);
				break;
			case 2:
				e.npc_id = id_dist(rng);
				break;
			case 3:
				e.bot_id = id_dist(rng);
				break;
			case 4:
				int entity_choice = id_dist(rng);
				e.zone_id = entity_choice;
				e.instance_id = entity_choice;
				break;
		}

		DataBucket::SetData(&database, e);

		inserted_keys.emplace_back(e);
	}
	auto                          insert_end  = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> insert_time = insert_end - insert_start;
	std::cout << "✅ Completed " << Strings::Commify(OPERATIONS_PER_TEST) << " inserts in " << insert_time.count()
			  << " seconds. (Individual Insert Performance)\n";

	// ✏️ **Measure Update Performance Using DataBucket**
	auto      update_start = std::chrono::high_resolution_clock::now();
	for (auto &key: inserted_keys) {
		// 🔍 Retrieve existing bucket using scoped `GetData`
		auto e = DataBucket::GetData(&database, key);
		if (e.id > 0) {
			// create a new key object with the updated values
			DataBucketKey bucket_entry_key{
				.key = e.key_,
				.value = "some_new_value",
				.expires = std::to_string(e.expires),
				.account_id = e.account_id,
				.character_id = e.character_id,
				.npc_id = e.npc_id,
				.bot_id = e.bot_id,
				.zone_id = e.zone_id,
				.instance_id = e.instance_id
			};

			// 🔄 Update using DataBucket class
			DataBucket::SetData(&database, bucket_entry_key);
		}
	}
	auto                          update_end  = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> update_time = update_end - update_start;
	std::cout << "✅ Completed " << Strings::Commify(OPERATIONS_PER_TEST) << " updates in " << update_time.count()
			  << " seconds. (Scoped Update Performance)\n";


	// 🔍 **Measure Cached Read Performance**
	auto        read_cached_start = std::chrono::high_resolution_clock::now();
	for (size_t i                 = 0; i < OPERATIONS_PER_TEST; ++i) {
		std::string   key = test_key_prefix + std::to_string(current_count + i);
		DataBucketKey k{
			.key = key,
			.account_id = 0,
			.character_id = 0,
			.npc_id = 0,
			.bot_id = 0,
			.zone_id = 0,
			.instance_id = 0
		};

		// randomly set account_id, character_id, npc_id, or bot_id
		switch (i % 4) {
			case 0:
				k.account_id = id_dist(rng);
				break;
			case 1:
				k.character_id = id_dist(rng);
				break;
			case 2:
				k.npc_id = id_dist(rng);
				break;
			case 3:
				k.bot_id = id_dist(rng);
				break;
			case 4:
				int entity_choice = id_dist(rng);
				k.zone_id = entity_choice;
				k.instance_id = entity_choice;
		}

		DataBucket::GetData(&database, key);
	}
	auto                          read_cached_end  = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> read_cached_time = read_cached_end - read_cached_start;
	std::cout << "✅ Completed " << Strings::Commify(OPERATIONS_PER_TEST) << " cached reads in "
			  << read_cached_time.count() << " seconds. (DataBucket::GetData)\n";

	// 🔍 **Measure Client-Scoped Cache Miss Performance (Skips DB via CanCache)**
	auto read_client_cache_miss_start = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < OPERATIONS_PER_TEST; ++i) {
		// generate key that doesn't exist
		std::string key = "nonexistent_key_" + std::to_string(i);

		DataBucketKey k{
			.key = key,
			.character_id = 999999999, // use scoped value
		};

		DataBucket::GetData(&database, k);
	}
	auto read_client_cache_miss_end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> read_client_cache_miss_time = read_client_cache_miss_end - read_client_cache_miss_start;
	std::cout << "✅ Completed " << Strings::Commify(OPERATIONS_PER_TEST)
			  << " scoped cache-miss reads (no DB) in "
			  << read_client_cache_miss_time.count() << " seconds. (Client Scoped, Cache Miss, No DB)\n";

	// 🔍 **Measure Non-Cached Read Performance (Direct Query)**
	auto                          read_uncached_start = std::chrono::high_resolution_clock::now();
	for (size_t                   i                   = 0; i < OPERATIONS_PER_TEST; ++i) {
		std::string key = test_key_prefix + std::to_string(current_count + i);
		DataBucketsRepository::GetWhere(database, "`key` = '" + key + "'");
	}
	auto                          read_uncached_end   = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> read_uncached_time  = read_uncached_end - read_uncached_start;
	std::cout << "✅ Completed " << Strings::Commify(OPERATIONS_PER_TEST) << " non-cached reads in "
			  << read_uncached_time.count() << " seconds. (DataBucketsRepository::GetWhere)\n";

	// 🗑️ **Measure Delete Performance**
	auto        delete_start = std::chrono::high_resolution_clock::now();
	for (size_t i            = 0; i < OPERATIONS_PER_TEST; ++i) {
		std::string key = test_key_prefix + std::to_string(current_count + i);

		DataBucketKey k{
			.key = key,
			.account_id = 0,
			.character_id = 0,
			.npc_id = 0,
			.bot_id = 0,
			.zone_id = 0,
			.instance_id = 0
		};

		// randomly set account_id, character_id, npc_id, or bot_id
		switch (i % 4) {
			case 0:
				k.account_id = id_dist(rng);
				break;
			case 1:
				k.character_id = id_dist(rng);
				break;
			case 2:
				k.npc_id = id_dist(rng);
				break;
			case 3:
				k.bot_id = id_dist(rng);
				break;
			case 4:
				int entity_choice = id_dist(rng);
				k.zone_id = entity_choice;
				k.instance_id = entity_choice;
		}

		DataBucket::DeleteData(&database, k);
	}
	auto                          delete_end  = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> delete_time = delete_end - delete_start;
	std::cout << "✅ Completed " << Strings::Commify(OPERATIONS_PER_TEST) << " deletes in " << delete_time.count()
			  << " seconds.\n";
}

void ZoneCLI::BenchmarkDatabuckets(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Benchmark individual reads/writes/deletes in data_buckets at different table sizes.";

	if (cmd[{"-h", "--help"}]) {
		std::cout << "Usage: BenchmarkDatabuckets\n";
		return;
	}

	if (std::getenv("DEBUG")) {
		EQEmuLogSys::Instance()->SetDatabase(&database)->LoadLogDatabaseSettings();
	}

	auto start_time = std::chrono::high_resolution_clock::now();

	std::vector<uint64_t> benchmark_sizes = {10000, 100000, 1000000};

	for (auto size: benchmark_sizes) {
		RunBenchmarkCycle(size);
	}

	// 🚀 **Total Benchmark Time**
	auto                          end_time      = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> total_elapsed = end_time - start_time;
	std::cout << "\n🚀 Total Benchmark Time: " << total_elapsed.count() << " seconds\n";
}
