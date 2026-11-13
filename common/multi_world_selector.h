/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.
*/
#pragma once

#include "common/json/json.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>

namespace EQ::Net::MultiWorldSelector {

constexpr std::size_t MaxWorldShortNameLength = 50;
constexpr std::size_t ControlPacketSize       = 66;
constexpr const char *ConfigFileName          = "world_selector.json";

enum class ConfigFileLoadStatus {
	Loaded,
	Unavailable,
	Malformed
};

struct WorldBackend {
	std::string host;
	std::uint16_t port{9000};
};

struct Config {
	bool enabled{false};
	std::string control_bind{"127.0.0.1"};
	std::uint16_t control_port{9900};
	std::string world_bind{"0.0.0.0"};
	std::string upstream_bind{"0.0.0.0"};
	std::uint16_t world_port{9000};
	std::uint32_t selection_timeout_seconds{60};
	std::uint32_t session_timeout_seconds{300};
	std::map<std::string, WorldBackend> worlds;
};

struct ControlSelection {
	std::string client_ip;
	std::uint16_t login_source_port{0};
	std::string world_short_name;
};

bool IsValidWorldShortName(const std::string &name);
bool IsValidIPv4(const std::string &address);
bool IsLoopbackIPv4(const std::string &address);
Config ParseConfig(const Json::Value &document);
ConfigFileLoadStatus LoadConfigFile(const std::string &path, Config &config, std::string &error);
bool EncodeControlPacket(const ControlSelection &selection, std::array<std::uint8_t, ControlPacketSize> &packet);
bool DecodeControlPacket(const std::uint8_t *data, std::size_t size, ControlSelection &selection);

} // namespace EQ::Net::MultiWorldSelector
