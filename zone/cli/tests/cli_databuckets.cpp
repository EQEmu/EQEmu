#include "../../zone_cli.h"
#include "../../common/cli/eqemu_command_handler.h"
#include "../../common/http/httplib.h"
#include "../../common/eqemu_logsys.h"
#include "../../common/platform.h"
#include "../../zone.h"
#include "../../client.h"
#include "../../common/net/eqstream.h"

extern Zone* zone;

void RunTest(const std::string& test_name, const std::string& expected, const std::string& actual);
void RunTest(const std::string& test_name, bool expected, bool actual);
void RunTest(const std::string& test_name, int expected, int actual);

void ZoneCLI::TestDataBuckets(int argc, char** argv, argh::parser& cmd, std::string& description)
{
	if (cmd[{"-h", "--help"}]) {
		return;
	}

	uint32 break_length = 50;
	int    failed_count = 0;

	EQEmuLogSys::Instance()->SilenceConsoleLogging();

	// boot shell zone for testing
	Zone::Bootup(ZoneID("qrg"), 0, false);
	zone->StopShutdownTimer();

	entity_list.Process();
	entity_list.MobProcess();

	EQEmuLogSys::Instance()->EnableConsoleLogging();

	Client* client = new Client();
	client->SetCharacterId(1); // Set a dummy character ID for testing

	EQEmuLogSys::Instance()->log_settings[Logs::MySQLQuery].is_category_enabled = std::getenv("DEBUG") ? 1 : 0;
	EQEmuLogSys::Instance()->log_settings[Logs::MySQLQuery].log_to_console      = std::getenv("DEBUG") ? 3 : 0;

	// 🧹 Delete all test keys before running tests
	std::vector<std::string> test_keys_to_clear = {
		"basic_key", "expiring_key", "cache_key", "json_key", "non_existent_key", "simple_key",
		"nested", "nested.test1", "nested.test2", "nested.test1.a", "nested.test2.a",
		"exp_test", "cache_test", "full_json", "full_json.key2", "complex", "complex.nested.obj1",
		"complex.nested.obj2", "plain_string", "json_array", "nested_partial",
		"nested_override", "empty_json", "json_string", "deep_nested",
		"nested_expire", "scoped_miss_test", "scoped_nested_miss.key",
		"cache_miss_overwrite", "missed_nested_set", "account_client_test", "ac_nested.test",
		"scoped_db_only_key"
	};

	DataBucketsRepository::DeleteWhere(
		database,
		fmt::format("`key` IN ('{}')", Strings::Join(test_keys_to_clear, "','"))
	);
	DataBucket::ClearCache();

	std::cout << "===========================================\n";
	std::cout << "⚙\uFE0F> Running DataBuckets Tests...\n";
	std::cout << "===========================================\n\n";

	// Basic Key-Value Set/Get
	client->DeleteBucket("basic_key");
	client->SetBucket("basic_key", "simple_value");
	std::string value = client->GetBucket("basic_key");
	RunTest("Basic Key-Value Set/Get", "simple_value", value);

	// Overwriting a Key
	client->SetBucket("basic_key", "new_value");
	value = client->GetBucket("basic_key");
	RunTest("Overwriting a Key", "new_value", value);

	// Deleting a Key
	client->DeleteBucket("basic_key");
	value = client->GetBucket("basic_key");
	RunTest("Deleting a Key", "", value);

	// Setting a Key with an Expiration
	client->SetBucket("expiring_key", "expires_soon", "S1");
	value = client->GetBucket("expiring_key");
	RunTest("Setting a Key with an Expiration", "expires_soon", value);

	// Ensure Expired Key is Deleted
	std::this_thread::sleep_for(std::chrono::seconds(2));
	value = client->GetBucket("expiring_key");
	RunTest("Ensure Expired Key is Deleted", "", value);

	// Cache Read/Write Consistency
	client->SetBucket("cache_key", "cached_value");
	value = client->GetBucket("cache_key");
	RunTest("Cache Read/Write Consistency", "cached_value", value);

	// Cache Clears on Key Deletion
	client->DeleteBucket("cache_key");
	value = client->GetBucket("cache_key");
	RunTest("Cache Clears on Key Deletion", "", value);

	// Setting a Full JSON String
	client->SetBucket("json_key", R"({"key1":"value1","key2":"value2"})");
	value = client->GetBucket("json_key");
	RunTest("Setting a Full JSON String", R"({"key1":"value1","key2":"value2"})", value);

	// Overwriting JSON with a Simple String
	client->SetBucket("json_key", "string_value");
	value = client->GetBucket("json_key");
	RunTest("Overwriting JSON with a Simple String", "string_value", value);

	// Deleting Non-Existent Key
	client->DeleteBucket("non_existent_key");
	value = client->GetBucket("non_existent_key");
	RunTest("Deleting Non-Existent Key", "", value);

	// Basic Key-Value Storage**
	client->DeleteBucket("simple_key"); // Reset
	client->SetBucket("simple_key", "simple_value");
	value = client->GetBucket("simple_key");
	RunTest("Basic Key-Value Set/Get", "simple_value", value);

	// Nested Key Storage**
	client->DeleteBucket("nested");
	client->SetBucket("nested.test1", "value1");
	client->SetBucket("nested.test2", "value2");
	value = client->GetBucket("nested");
	RunTest("Nested Key Set/Get", R"({"test1":"value1","test2":"value2"})", value);

	// Prevent Overwriting Objects**
	client->DeleteBucket("nested");
	client->SetBucket("nested.test1.a", "value1");
	client->SetBucket("nested.test2.a", "value2");
	client->SetBucket("nested.test2", "new_value"); // Should be **rejected**
	value = client->GetBucket("nested");
	RunTest("Prevent Overwriting Objects", R"({"test1":{"a":"value1"},"test2":{"a":"value2"}})", value);

	// Deleting a Specific Nested Key**
	client->DeleteBucket("nested");
	client->SetBucket("nested.test1", "value1");
	client->SetBucket("nested.test2", "value2");
	client->DeleteBucket("nested.test1");
	value = client->GetBucket("nested");
	RunTest("Delete Nested Key", R"({"test2":"value2"})", value);

	// Deleting the Entire Parent Key**
	client->DeleteBucket("nested");
	value = client->GetBucket("nested");
	RunTest("Delete Parent Key", "", value);

	// Expiration is Ignored for Nested Keys**
	client->DeleteBucket("exp_test");
	client->SetBucket("exp_test.nested", "data", "S20"); // Expiration ignored
	value = client->GetBucket("exp_test");
	RunTest("Expiration Ignored for Nested Keys", R"({"nested":"data"})", value);

	// Cache Behavior**
	client->DeleteBucket("cache_test");
	client->SetBucket("cache_test", "cache_value");
	value = client->GetBucket("cache_test");
	RunTest("Cache Read/Write Consistency", "cache_value", value);

	// Ensure Deleting Parent Key Clears Cache**
	client->DeleteBucket("cache_test");
	value = client->GetBucket("cache_test");
	RunTest("Cache Clears on Parent Delete", "", value);

	// Setting an Entire JSON Object**
	client->DeleteBucket("full_json");
	client->SetBucket("full_json", R"({"key1":"value1","key2":{"subkey":"subvalue"}})");
	value = client->GetBucket("full_json");
	RunTest("Set and Retrieve Full JSON Structure", R"({"key1":"value1","key2":{"subkey":"subvalue"}})", value);

	// Partial Nested Key Deletion within JSON**
	client->DeleteBucket("full_json");
	client->SetBucket("full_json", R"({"key1":"value1","key2":{"subkey":"subvalue"}})");
	client->DeleteBucket("full_json.key2");
	value = client->GetBucket("full_json");
	RunTest("Delete Nested Key within JSON", R"({"key1":"value1"})", value);

	// Ensure Object Protection on Overwrite Attempt**
	client->DeleteBucket("complex");
	client->SetBucket("complex.nested.obj1", "data1");
	client->SetBucket("complex.nested.obj2", "data2");
	client->SetBucket("complex.nested", "overwrite_attempt"); // Should be rejected
	value = client->GetBucket("complex");
	RunTest("Ensure Object Protection on Overwrite Attempt", R"({"nested":{"obj1":"data1","obj2":"data2"}})", value);

	// Deleting Non-Existent Key Doesn't Break Existing Data**
	client->DeleteBucket("complex");
	client->SetBucket("complex.nested.obj1", "data1");
	client->SetBucket("complex.nested.obj2", "data2");
	client->DeleteBucket("does_not_exist"); // Should do nothing
	value = client->GetBucket("complex");
	RunTest("Deleting Non-Existent Key Doesn't Break Existing Data", R"({"nested":{"obj1":"data1","obj2":"data2"}})",
	        value);

	// Get nested key value one level up **
	client->DeleteBucket("complex");
	client->SetBucket("complex.nested.obj1", "data1");
	client->SetBucket("complex.nested.obj2", "data2");
	value = client->GetBucket("complex.nested");
	RunTest("Get nested key value", R"({"obj1":"data1","obj2":"data2"})", value);

	// Get nested key value deep **
	client->DeleteBucket("complex");
	client->SetBucket("complex.nested.obj1", "data1");
	client->SetBucket("complex.nested.obj2", "data2");
	value = client->GetBucket("complex.nested.obj2");
	RunTest("Get nested key value deep", R"(data2)", value);

	// Retrieve Nested Key from Plain String**
	client->DeleteBucket("plain_string");
	client->SetBucket("plain_string", "some_value");
	value = client->GetBucket("plain_string.nested");
	RunTest("Retrieve Nested Key from Plain String", "", value);

	// Store and Retrieve JSON Array**
	client->DeleteBucket("json_array");
	client->SetBucket("json_array", R"(["item1", "item2"])");
	value = client->GetBucket("json_array");
	RunTest("Store and Retrieve JSON Array", R"(["item1", "item2"])", value);

	//	// Prevent Overwriting Array with Object**
	//	client->DeleteBucket("json_array");
	//	client->SetBucket("json_array", R"(["item1", "item2"])");
	//	client->SetBucket("json_array.item", "new_value"); // Should be rejected
	//	value = client->GetBucket("json_array");
	//	RunTest("Prevent Overwriting Array with Object", R"(["item1", "item2"])", value);

	// Retrieve Non-Existent Nested Key**
	client->DeleteBucket("nested_partial");
	client->SetBucket("nested_partial.level1", R"({"exists": "yes"})");
	value = client->GetBucket("nested_partial.level1.non_existent");
	RunTest("Retrieve Non-Existent Nested Key", "", value);

	// Overwriting Parent Key Deletes Children**
	client->DeleteBucket("nested_override");
	client->SetBucket("nested_override.child", "data");
	client->SetBucket("nested_override", "new_parent_value"); // Should remove `child`
	value = client->GetBucket("nested_override");
	RunTest("Overwriting Parent Key Deletes Children", "new_parent_value", value);

	// Store and Retrieve Empty JSON Object**
	client->DeleteBucket("empty_json");
	client->SetBucket("empty_json", R"({})");
	value = client->GetBucket("empty_json");
	RunTest("Store and Retrieve Empty JSON Object", R"({})", value);

	// Store and Retrieve JSON String**
	client->DeleteBucket("json_string");
	client->SetBucket("json_string", R"("this is a string")");
	value = client->GetBucket("json_string");
	RunTest("Store and Retrieve JSON String", R"("this is a string")", value);

	// Deeply Nested Key Retrieval**
	client->DeleteBucket("deep_nested");
	client->SetBucket("deep_nested.level1.level2.level3.level4.level5", "final_value");
	value = client->GetBucket("deep_nested.level1.level2.level3.level4.level5");
	RunTest("Deeply Nested Key Retrieval", "final_value", value);

	// Setting a Key with an Expiration
	client->SetBucket("nested_expire.test.test", "shouldnt_expire", "S1");
	value = client->GetBucket("nested_expire");
	std::this_thread::sleep_for(std::chrono::seconds(2));
	RunTest("Setting a nested key with an expiration protection test", R"({"test":{"test":"shouldnt_expire"}})", value);

	// Delete Deep Nested Key Keeps Parent**
	//	client->DeleteBucket("deep_nested");
	//	client->SetBucket("deep_nested.level1.level2.level3", R"({"key": "value"})");
	//	client->DeleteBucket("deep_nested.level1.level2.level3.key");
	//	value = client->GetBucket("deep_nested.level1.level2.level3");
	//	RunTest("Delete Deep Nested Key Keeps Parent", "{}", value);

	// ================================
	// 🧪 Scoped Cache-Miss Behavior Tests
	// ================================

	// Ensure a scoped key (character ID) that doesn't exist is not fetched from DB if not in cache
	client->DeleteBucket("scoped_miss_test"); // Ensure not in DB
	DataBucket::ClearCache();                 // Clear all caches
	std::string scoped_miss_value = client->GetBucket("scoped_miss_test");
	RunTest("Scoped Missing Key Returns Empty (Skips DB)", "", scoped_miss_value);

	// Ensure nested scoped key that isn't in cache returns empty immediately
	client->DeleteBucket("scoped_nested_miss.key");
	DataBucket::ClearCache(); // Clear cache again
	std::string scoped_nested_miss = client->GetBucket("scoped_nested_miss.key");
	RunTest("Nested Scoped Key Miss Returns Empty (Skips DB)", "", scoped_nested_miss);

	// Write to a key that was previously missed (0-id cached miss)
	client->DeleteBucket("cache_miss_overwrite");
	DataBucket::ClearCache(); // Ensure clean slate
	std::string missed_value = client->GetBucket("cache_miss_overwrite");
	RunTest("Initial Cache Miss Returns Empty", "", missed_value);
	client->SetBucket("cache_miss_overwrite", "new_value");
	std::string new_val = client->GetBucket("cache_miss_overwrite");
	RunTest("Overwrite After Cache Miss Works", "new_value", new_val);

	// Write a nested key that previously missed
	client->DeleteBucket("missed_nested_set.test");
	DataBucket::ClearCache();
	std::string initial = client->GetBucket("missed_nested_set.test");
	RunTest("Missed Nested Key Returns Empty", "", initial);
	client->SetBucket("missed_nested_set.test", "set_value");
	std::string after_write = client->GetBucket("missed_nested_set.test");
	RunTest("Nested Set After Miss Works", "set_value", after_write);

	// ================================
	// 🧪 Scoped Cache Preload Tests (Account + Client)
	// ================================

	// Clear everything for a clean test
	// Insert directly into the DB without touching cache
	const std::string scoped_key = "scoped_db_only_key";
	client->DeleteBucket(scoped_key);
	DataBucket::ClearCache();

	// ✅ Scoped insert
	DataBucketsRepository::InsertOne(
		database, {
			.key_ = scoped_key,
			.value = "cached_value",
			.character_id = client->CharacterID()
		}
	);

	// Cold cache test — should return ""
	// std::string cold_value = client->GetBucket(scoped_key);
	// RunTest("Cold Cache Scoped Key Returns Empty (Due to Skip DB)", "", cold_value);
	//
	// // ✅ Reload cache
	// client->LoadDataBucketsCache();
	//
	// // Cache should now return the value
	// std::string hot_value = client->GetBucket(scoped_key);
	// RunTest("Post-BulkLoad Scoped Key Returns Value", "cached_value", hot_value);
	//
	// // Also test nested key after preload
	// client->DeleteBucket("ac_nested.test");
	// client->SetBucket("ac_nested.test", "nested_val");
	//
	// // Clear cache, then preload
	// DataBucket::ClearCache();
	// client->LoadDataBucketsCache();
	//
	// std::string nested_value = client->GetBucket("ac_nested.test");
	// RunTest("Post-BulkLoad Nested Scoped Key Returns Value", "nested_val", nested_value);
	//
	// // Remove and check that cache misses properly again
	// client->DeleteBucket("ac_nested.test");
	// DataBucket::ClearCache();
	// std::string post_delete_check = client->GetBucket("ac_nested.test");
	// RunTest("Post-Delete Nested Scoped Key Returns Empty", "", post_delete_check);


	std::cout << "\n===========================================\n";
	std::cout << "✅ All DataBucket Tests Completed!\n";
	std::cout << "===========================================\n";
}
