#pragma once

class ServerPacket;

class SharedTaskZoneMessaging {
public:
	static void HandleWorldMessage(ServerPacket *pack);
};
