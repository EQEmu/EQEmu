#include "common/multi_world_selector.h"
#include "common/multi_world_selector_notifier.h"
#include "world_selector/selector_config_path.h"
#include "world_selector/selector_state.h"
#include "world_selector/world_selector_server.h"

#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace EQ::Net::MultiWorldSelector;

namespace {

void Expect(bool condition, const std::string &message)
{
	if (!condition) {
		throw std::runtime_error(message);
	}
}

std::filesystem::path UniqueConfigPath()
{
	static std::uint64_t sequence = 0;
	return std::filesystem::temp_directory_path() /
		("eqemu_world_selector_tests_" +
			std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + "_" +
			std::to_string(++sequence) + ".json");
}

class TemporaryConfigFile {
public:
	explicit TemporaryConfigFile(const std::string &contents)
	:	path(UniqueConfigPath())
	{
		std::ofstream file(path, std::ofstream::binary);
		file << contents;
		if (!file.good()) {
			throw std::runtime_error("unable to create temporary selector config");
		}
	}

	~TemporaryConfigFile()
	{
		std::error_code error;
		std::filesystem::remove(path, error);
	}

	std::filesystem::path path;
};

Config TestConfig()
{
	Config config;
	config.enabled = true;
	config.selection_timeout_seconds = 60;
	config.session_timeout_seconds = 300;
	config.worlds.emplace("world_a", WorldBackend{"192.0.2.20", 9000});
	config.worlds.emplace("world_b", WorldBackend{"192.0.2.30", 9000});
	return config;
}

void TestConfigDefaults()
{
	const auto absent = ParseConfig(Json::Value{});
	Expect(!absent.enabled, "absent config must disable the selector");
	Expect(absent.control_bind == "127.0.0.1", "absent control bind changed");
	Expect(absent.world_bind == "0.0.0.0" && absent.world_port == 9000, "absent World defaults changed");
	Expect(absent.upstream_bind == "0.0.0.0", "absent upstream bind changed");
	Expect(IsLoopbackIPv4("127.0.0.1"), "valid loopback address was rejected");
	Expect(!IsLoopbackIPv4("127.bad"), "malformed loopback address was accepted");
	Expect(!IsLoopbackIPv4("192.0.2.1"), "non-loopback address was accepted as control bind");

	Json::Value document;
	document["multi_world_selector"]["enabled"] = false;
	document["multi_world_selector"]["world_bind"] = "192.0.2.10";
	document["multi_world_selector"]["upstream_bind"] = "192.0.2.20";
	const auto disabled = ParseConfig(document);
	Expect(!disabled.enabled, "explicitly disabled config became enabled");
	Expect(disabled.world_bind == "192.0.2.10", "explicit World bind did not parse");
	Expect(disabled.upstream_bind == "192.0.2.20", "explicit upstream bind did not parse");
}

void TestConfigValidation()
{
	auto config = TestConfig();
	std::string error;
	Expect(ValidateConfig(config, error), "valid selector config was rejected: " + error);

	config.upstream_bind = "not-an-ip";
	error.clear();
	Expect(!ValidateConfig(config, error), "invalid upstream bind was accepted");
	Expect(error.find("upstream_bind") != std::string::npos, "validation error omitted upstream_bind");

	config = TestConfig();
	config.worlds["world_a"] = {config.world_bind, config.world_port};
	error.clear();
	Expect(!ValidateConfig(config, error), "selector relay loop was accepted");
}

void TestDedicatedConfigFileLoading()
{
	TemporaryConfigFile valid(R"json({
  "multi_world_selector": {
    "enabled": true,
    "control_bind": "127.0.0.1",
    "world_bind": "192.0.2.10",
    "upstream_bind": "0.0.0.0",
    "worlds": {
      "world_a": { "backend_host": "192.0.2.20", "backend_port": 9000 }
    }
  }
})json");
	Config config;
	std::string error;
	Expect(LoadConfigFile(valid.path.string(), config, error) == ConfigFileLoadStatus::Loaded,
		"valid dedicated config did not load: " + error);
	Expect(config.enabled && config.worlds.count("world_a") == 1, "valid dedicated config values changed");

	const auto missing_path = UniqueConfigPath();
	Expect(LoadConfigFile(missing_path.string(), config, error) == ConfigFileLoadStatus::Unavailable,
		"missing dedicated config was not reported unavailable");
	Expect(!config.enabled, "missing dedicated config did not disable integration");

	TemporaryConfigFile malformed("{ \"multi_world_selector\": ");
	Expect(LoadConfigFile(malformed.path.string(), config, error) == ConfigFileLoadStatus::Malformed,
		"malformed dedicated config was accepted");
	Expect(!config.enabled, "malformed dedicated config did not disable integration");

	TemporaryConfigFile disabled(R"json({"multi_world_selector":{"enabled":false}})json");
	Expect(LoadConfigFile(disabled.path.string(), config, error) == ConfigFileLoadStatus::Loaded,
		"disabled dedicated config did not load");
	Expect(!config.enabled, "disabled dedicated config became enabled");
}

void TestSelectorConfigPath()
{
	char executable[] = "eqemu_world_selector.exe";
	char *default_arguments[] = {executable};
	Expect(ResolveConfigPath(1, default_arguments) == "world_selector.json", "default config path changed");

	TemporaryConfigFile explicit_config(R"json({"multi_world_selector":{"enabled":false}})json");
	auto explicit_path = explicit_config.path.string();
	char *explicit_arguments[] = {executable, explicit_path.data()};
	Expect(ResolveConfigPath(2, explicit_arguments) == explicit_path, "explicit config path was ignored");
}

void TestControlPacketValidation()
{
	ControlSelection input{"203.0.113.7", 45000, "world_b"};
	std::array<std::uint8_t, ControlPacketSize> packet{};
	Expect(EncodeControlPacket(input, packet), "valid control packet did not encode");

	ControlSelection decoded;
	Expect(DecodeControlPacket(packet.data(), packet.size(), decoded), "valid control packet did not decode");
	Expect(decoded.client_ip == input.client_ip && decoded.login_source_port == input.login_source_port &&
		decoded.world_short_name == input.world_short_name, "control packet round trip changed values");
	Expect(!DecodeControlPacket(packet.data(), packet.size() - 1, decoded), "short control packet was accepted");
	packet[0] = 'X';
	Expect(!DecodeControlPacket(packet.data(), packet.size(), decoded), "bad control magic was accepted");
	Expect(!EncodeControlPacket({"not-an-ip", 1, "world_a"}, packet), "bad IPv4 address was encoded");
	Expect(!EncodeControlPacket({"203.0.113.7", 0, "world_a"}, packet), "zero source port was encoded");
	Expect(!EncodeControlPacket({"203.0.113.7", 1, "../world_a"}, packet), "bad World name was encoded");
}

void TestWorldRoutingAndNatFallback()
{
	SelectorState state(TestConfig());
	const auto now = SelectorState::Clock::now();
	Expect(state.AddSelection({"198.51.100.10", 41000, "world_a"}, now), "world_a selection rejected");
	Expect(state.AddSelection({"198.51.100.11", 42000, "world_b"}, now), "world_b selection rejected");

	const auto world_a = state.AssignSession({"198.51.100.10", 41000}, now);
	const auto world_b = state.AssignSession({"198.51.100.11", 52000}, now);
	Expect(world_a && world_a->world_short_name == "world_a", "exact source-port route failed");
	Expect(world_b && world_b->world_short_name == "world_b", "NAT fallback route failed");
	Expect(!state.AddSelection({"198.51.100.12", 43000, "unknown"}, now), "unknown World was accepted");
}

void TestMultipleClientsAndReselection()
{
	SelectorState state(TestConfig());
	const auto now = SelectorState::Clock::now();
	Expect(state.AddSelection({"203.0.113.25", 44001, "world_a"}, now), "first NAT selection rejected");
	Expect(state.AddSelection({"203.0.113.25", 44002, "world_b"}, now), "second NAT selection rejected");
	Expect(state.AssignSession({"203.0.113.25", 44002}, now)->world_short_name == "world_b",
		"exact NAT source-port hint did not win");
	Expect(state.AssignSession({"203.0.113.25", 54001}, now)->world_short_name == "world_a",
		"FIFO NAT fallback failed");
	Expect(state.SessionCount() == 2, "simultaneous client sessions collided");

	const ClientEndpoint reselected{"203.0.113.30", 45000};
	Expect(state.AddSelection({reselected.address, reselected.port, "world_a"}, now), "initial route rejected");
	Expect(state.AssignSession(reselected, now)->world_short_name == "world_a", "initial route failed");
	Expect(state.AddSelection({reselected.address, reselected.port, "world_b"}, now), "reselection rejected");
	Expect(state.HasExactPendingSelection(reselected), "exact reselection was not detected");
	Expect(!state.HasExactPendingSelection({reselected.address, 45001}), "inexact reselection matched");
	state.RemoveSession(reselected);
	Expect(state.AssignSession(reselected, now)->world_short_name == "world_b", "reselection route failed");
}

void TestSessionRouteContinuation()
{
	SelectorState state(TestConfig());
	const auto now = SelectorState::Clock::now();
	const ClientEndpoint initial{"198.51.100.50", 46000};
	const ClientEndpoint zoning{"198.51.100.50", 46001};
	const ClientEndpoint second_client{"198.51.100.50", 46002};

	Expect(state.AddSelection({initial.address, initial.port, "world_a"}, now),
		"initial explicit selection rejected");
	const auto initial_route = state.AssignSession(initial, now);
	Expect(initial_route && initial_route->world_short_name == "world_a",
		"initial explicit selection did not create a route");

	const auto zoning_route = state.AssignSession(zoning, now + std::chrono::seconds(1));
	Expect(zoning_route && zoning_route->world_short_name == "world_a",
		"same-IP zoning reconnect did not inherit its route");

	const auto second_client_route = state.AssignSession(second_client, now + std::chrono::seconds(2));
	Expect(second_client_route && second_client_route->world_short_name == "world_a",
		"multiple same-IP sessions on one backend did not allow route inheritance");
	Expect(state.SessionCount() == 3, "continued sessions did not retain distinct UDP endpoints");
}

void TestAmbiguousRouteContinuation()
{
	SelectorState state(TestConfig());
	const auto now = SelectorState::Clock::now();
	const std::string address = "203.0.113.50";

	Expect(state.AddSelection({address, 47000, "world_a"}, now), "world_a selection rejected");
	Expect(state.AssignSession({address, 47000}, now)->world_short_name == "world_a",
		"world_a session assignment failed");
	Expect(state.AddSelection({address, 47001, "world_b"}, now), "world_b selection rejected");
	Expect(state.AssignSession({address, 47001}, now)->world_short_name == "world_b",
		"explicit world_b selection did not override same-IP inheritance");

	Expect(!state.AssignSession({address, 47002}, now + std::chrono::seconds(1)),
		"ambiguous same-IP routes were inherited");
}

void TestExplicitSelectionPrecedesContinuation()
{
	SelectorState state(TestConfig());
	const auto now = SelectorState::Clock::now();
	const std::string address = "203.0.113.60";

	Expect(state.AddSelection({address, 48000, "world_a"}, now), "initial world_a selection rejected");
	Expect(state.AssignSession({address, 48000}, now)->world_short_name == "world_a",
		"initial world_a route failed");
	Expect(state.AddSelection({address, 48001, "world_b"}, now), "explicit world_b selection rejected");
	const auto selected = state.AssignSession({address, 48001}, now + std::chrono::seconds(1));
	Expect(selected && selected->world_short_name == "world_b",
		"same-IP route inheritance overrode an explicit selection");
}

void TestExpiredAndExactSessionContinuation()
{
	auto config = TestConfig();
	config.session_timeout_seconds = 3;
	SelectorState state(config);
	const auto now = SelectorState::Clock::now();
	const ClientEndpoint exact{"198.51.100.60", 49000};

	Expect(state.AddSelection({exact.address, exact.port, "world_a"}, now), "exact route selection rejected");
	Expect(state.AssignSession(exact, now)->world_short_name == "world_a", "exact route assignment failed");
	const auto stable = state.AssignSession(exact, now + std::chrono::seconds(1));
	Expect(stable && stable->world_short_name == "world_a", "exact active endpoint route changed");
	Expect(state.SessionCount() == 1, "exact active endpoint created a duplicate session");

	Expect(!state.AssignSession({exact.address, 49001}, now + std::chrono::seconds(4)),
		"expired session allowed same-IP route inheritance");
}

void TestExpiryAndRestart()
{
	auto config = TestConfig();
	config.selection_timeout_seconds = 2;
	config.session_timeout_seconds = 3;
	const auto now = SelectorState::Clock::now();
	SelectorState state(config);
	Expect(state.AddSelection({"198.51.100.20", 43000, "world_a"}, now), "selection rejected");
	state.Expire(now + std::chrono::seconds(2));
	Expect(!state.AssignSession({"198.51.100.20", 43000}, now), "stale selection did not expire");

	Expect(state.AddSelection({"198.51.100.21", 43001, "world_a"}, now), "session selection rejected");
	Expect(state.AssignSession({"198.51.100.21", 43001}, now).has_value(), "session assignment failed");
	const auto expired = state.Expire(now + std::chrono::seconds(3));
	Expect(expired.size() == 1, "stale session did not expire");

	SelectorState restarted(config);
	Expect(restarted.PendingSelectionCount() == 0 && restarted.SessionCount() == 0,
		"clean restart unexpectedly persisted routes");
}

void TestNotificationFailureIsolation()
{
	auto config = TestConfig();
	std::string error;
	config.enabled = false;
	Expect(Notify(config, {"203.0.113.40", 45000, "world_a"}, error),
		"disabled notification did not return harmlessly");

	config.enabled = true;
	config.control_bind = "192.0.2.1";
	Expect(!Notify(config, {"203.0.113.40", 45000, "world_a"}, error),
		"non-loopback control destination was accepted");

	config.control_bind = "127.0.0.1";
	config.control_port = 65534; // UDP intentionally has no listener acknowledgement.
	Expect(Notify(config, {"203.0.113.40", 45000, "world_a"}, error),
		"unavailable selector blocked or failed the local UDP send: " + error);
}

} // namespace

int main()
{
	try {
		TestConfigDefaults();
		TestConfigValidation();
		TestDedicatedConfigFileLoading();
		TestSelectorConfigPath();
		TestControlPacketValidation();
		TestWorldRoutingAndNatFallback();
		TestMultipleClientsAndReselection();
		TestSessionRouteContinuation();
		TestAmbiguousRouteContinuation();
		TestExplicitSelectionPrecedesContinuation();
		TestExpiredAndExactSessionContinuation();
		TestExpiryAndRestart();
		TestNotificationFailureIsolation();
	}
	catch (const std::exception &exception) {
		std::cerr << "Multi-World Selector test failure: " << exception.what() << '\n';
		return 1;
	}

	std::cout << "All Multi-World Selector tests passed\n";
	return 0;
}
