/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.
*/
#include "common/multi_world_selector.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <exception>
#include <fstream>
#include <utility>

namespace EQ::Net::MultiWorldSelector {

namespace {

bool ParseIPv4Octets(const std::string &address, std::array<std::uint8_t, 4> &octets)
{
	std::size_t start = 0;
	for (std::size_t index = 0; index < octets.size(); ++index) {
		const auto end = address.find('.', start);
		if ((end == std::string::npos) != (index == octets.size() - 1)) {
			return false;
		}

		const auto component = address.substr(start, end == std::string::npos ? end : end - start);
		if (component.empty() || component.size() > 3) {
			return false;
		}

		unsigned int value = 0;
		for (const auto c : component) {
			if (!std::isdigit(static_cast<unsigned char>(c))) {
				return false;
			}
			value = value * 10 + static_cast<unsigned int>(c - '0');
		}
		if (value > 255) {
			return false;
		}

		octets[index] = static_cast<std::uint8_t>(value);
		start = end == std::string::npos ? address.size() : end + 1;
	}

	return start == address.size();
}

void WriteUInt16(std::uint8_t *destination, std::uint16_t value)
{
	destination[0] = static_cast<std::uint8_t>((value >> 8) & 0xff);
	destination[1] = static_cast<std::uint8_t>(value & 0xff);
}

std::uint16_t ReadUInt16(const std::uint8_t *source)
{
	return static_cast<std::uint16_t>((static_cast<std::uint16_t>(source[0]) << 8) | source[1]);
}

} // namespace

bool IsValidWorldShortName(const std::string &name)
{
	if (name.empty() || name.size() > MaxWorldShortNameLength) {
		return false;
	}

	for (const auto c : name) {
		const auto value = static_cast<unsigned char>(c);
		if (!std::isalnum(value) && c != '_' && c != '-' && c != '.' && c != ' ') {
			return false;
		}
	}

	return true;
}

bool IsValidIPv4(const std::string &address)
{
	std::array<std::uint8_t, 4> octets{};
	return ParseIPv4Octets(address, octets);
}

bool IsLoopbackIPv4(const std::string &address)
{
	std::array<std::uint8_t, 4> octets{};
	return ParseIPv4Octets(address, octets) && octets[0] == 127;
}

Config ParseConfig(const Json::Value &document)
{
	Config config;
	const auto &root = document["multi_world_selector"];
	if (!root.isObject()) {
		return config;
	}

	try {
		auto read_port = [](const Json::Value &value, std::uint16_t default_value) {
			const auto parsed = value.asUInt();
			return parsed > 0 && parsed <= 65535 ? static_cast<std::uint16_t>(parsed) :
				(value.isNull() ? default_value : static_cast<std::uint16_t>(0));
		};

		config.enabled                   = root.get("enabled", false).asBool();
		config.control_bind              = root.get("control_bind", "127.0.0.1").asString();
		config.control_port              = read_port(root["control_port"], 9900);
		config.world_bind                = root.get("world_bind", "0.0.0.0").asString();
		config.upstream_bind             = root.get("upstream_bind", "0.0.0.0").asString();
		config.world_port                = read_port(root["world_port"], 9000);
		config.selection_timeout_seconds = root.get("selection_timeout_seconds", 60).asUInt();
		config.session_timeout_seconds   = root.get("session_timeout_seconds", 300).asUInt();

		const auto &worlds = root["worlds"];
		if (worlds.isObject()) {
			for (const auto &name : worlds.getMemberNames()) {
				const auto &entry = worlds[name];
				if (!entry.isObject()) {
					continue;
				}

				WorldBackend backend;
				backend.host = entry.get("backend_host", "").asString();
				backend.port = read_port(entry["backend_port"], 9000);
				config.worlds.emplace(name, std::move(backend));
			}
		}
	}
	catch (const std::exception &) {
		return Config{};
	}

	return config;
}

ConfigFileLoadStatus LoadConfigFile(const std::string &path, Config &config, std::string &error)
{
	config = Config{};
	error.clear();

	std::ifstream file(path, std::ifstream::binary);
	if (!file.good()) {
		error = "unable to open selector configuration: " + path;
		return ConfigFileLoadStatus::Unavailable;
	}

	Json::Value document;
	Json::CharReaderBuilder reader;
	std::string parse_error;
	if (!Json::parseFromStream(reader, file, &document, &parse_error)) {
		error = "unable to parse selector configuration " + path + ": " + parse_error;
		return ConfigFileLoadStatus::Malformed;
	}

	config = ParseConfig(document);
	return ConfigFileLoadStatus::Loaded;
}

bool EncodeControlPacket(
	const ControlSelection &selection,
	std::array<std::uint8_t, ControlPacketSize> &packet
)
{
	std::array<std::uint8_t, 4> address{};
	if (!ParseIPv4Octets(selection.client_ip, address) || selection.login_source_port == 0 ||
		!IsValidWorldShortName(selection.world_short_name)) {
		return false;
	}

	packet.fill(0);
	packet[0] = 'E';
	packet[1] = 'Q';
	packet[2] = 'W';
	packet[3] = 'S';
	packet[4] = 1;
	packet[5] = 0;
	WriteUInt16(packet.data() + 6, static_cast<std::uint16_t>(packet.size()));
	std::copy(address.begin(), address.end(), packet.begin() + 8);
	WriteUInt16(packet.data() + 12, selection.login_source_port);
	packet[14] = static_cast<std::uint8_t>(selection.world_short_name.size());
	packet[15] = 0;
	std::memcpy(packet.data() + 16, selection.world_short_name.data(), selection.world_short_name.size());
	return true;
}

bool DecodeControlPacket(const std::uint8_t *data, std::size_t size, ControlSelection &selection)
{
	if (data == nullptr || size != ControlPacketSize ||
		data[0] != 'E' || data[1] != 'Q' || data[2] != 'W' || data[3] != 'S' ||
		data[4] != 1 || data[5] != 0 || ReadUInt16(data + 6) != ControlPacketSize || data[15] != 0) {
		return false;
	}

	const auto name_length = static_cast<std::size_t>(data[14]);
	if (name_length == 0 || name_length > MaxWorldShortNameLength || ReadUInt16(data + 12) == 0) {
		return false;
	}

	for (std::size_t index = 16 + name_length; index < ControlPacketSize; ++index) {
		if (data[index] != 0) {
			return false;
		}
	}

	selection.client_ip = std::to_string(data[8]) + "." + std::to_string(data[9]) + "." +
		std::to_string(data[10]) + "." + std::to_string(data[11]);
	selection.login_source_port = ReadUInt16(data + 12);
	selection.world_short_name.assign(reinterpret_cast<const char *>(data + 16), name_length);
	return IsValidWorldShortName(selection.world_short_name);
}

} // namespace EQ::Net::MultiWorldSelector
