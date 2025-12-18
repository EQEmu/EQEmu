#pragma once

#include "common/types.h"

#include <string>

#pragma pack(push)
#pragma pack(1)

// unencrypted base message header in all packets
struct LoginBaseMessage {
	int32_t sequence;     // request type/login sequence (2: handshake, 3: login, 4: serverlist, ...)
	bool    compressed;   // true: deflated
	int8_t  encrypt_type; // 1: invert (unused) 2: des (2 for encrypted player logins and order expansions) (client uses what it sent, ignores in reply)
	int32_t unk3;         // unused?
};

struct LoginBaseReplyMessage {
	bool    success;      // 0: failure (shows error string) 1: success
	int32_t error_str_id; // last error eqlsstr id, default: 101 (no error)
	char    str[1];       // variable length, unknown (may be unused, this struct is a common pattern elsewhere)
};

struct LoginHandShakeReply {
	LoginBaseMessage      base_header;
	LoginBaseReplyMessage base_reply;
	char                  unknown[1]; // variable length string
};

// variable length, can use directly if not serializing strings
struct PlayerLoginReply {
	// base header excluded to make struct data easier to encrypt
	//LoginBaseMessage base_header;
	LoginBaseReplyMessage base_reply;

	int8_t  unk1;                       // (default: 0)
	int8_t  unk2;                       // (default: 0)
	int32_t lsid;                       // (default: -1)
	char    key[11];                    // client reads until null (variable length)
	int32_t failed_attempts;
	bool    show_player_count;          // admin flag, enables admin button and shows server player counts (default: false)
	int32_t offer_min_days;             // guess, needs more investigation, maybe expansion offers (default: 99)
	int32_t offer_min_views;            // guess (default: -1)
	int32_t offer_cooldown_minutes;     // guess (default: 0)
	int32_t web_offer_number;           // web order view number, 0 nothing (default: 0)
	int32_t web_offer_min_days;         // number of days to show offer (based on first offer time in client eqls ini) (default: 99)
	int32_t web_offer_min_views;        // mininum views, -1 for no minimum, 0 for never shows (based on client eqls ini) (default: -1)
	int32_t web_offer_cooldown_minutes; // minimum minutes between offers (based on last offer time in client eqls ini) (default: 0)
	char    username[1];                // variable length, if not empty client attempts to re-login to server select when quitting from char select and sends this in a struct
	char    unknown[1];                 // variable length, password unlikely? client doesn't send this on re-login from char select
};

// variable length, for reference
struct LoginClientServerData {
	char    ip[1];
	int32_t server_type;      // legends, preferred, standard
	int32_t server_id;
	char    server_name[1];
	char    country_code[1];  // if doesn't match client locale then server is colored dark grey in list and joining is prevented (to block for "us" use one of "kr", "tw", "jp", "de", "fr", or "cn") (ISO 3166-1 alpha-2)
	char    language_code[1];
	int32_t server_status;    // see ServerStatusFlags
	int32_t player_count;
};

// variable length, for reference
struct ServerListReply {
	LoginBaseMessage      base_header;
	LoginBaseReplyMessage base_reply;

	int32_t               server_count;
	LoginClientServerData servers[0];
};

struct PlayEverquestRequest {
	LoginBaseMessage base_header;
	uint32           server_number;
};

// SCJoinServerReply
struct PlayEverquestResponse {
	LoginBaseMessage      base_header;
	LoginBaseReplyMessage base_reply;
	uint32                server_number;
};

#pragma pack()

enum LSClientVersion {
	cv_titanium,
	cv_sod,
	cv_larion
};

enum LSClientStatus {
	cs_not_sent_session_ready,
	cs_waiting_for_login,
	cs_creating_account,
	cs_failed_to_login,
	cs_logged_in
};

struct LoginWorldContext {
	std::string long_name;
	std::string short_name;
	std::string password;
	std::string password_hash;
	int64       admin_id = 0;
};

struct LoginWorldAdminAccountContext {
	int64 id;
	std::string username;
	std::string password;
	std::string password_hash;
	std::string email;
	std::string first_name;
	std::string last_name;
	std::string ip_address;
};

struct LoginAccountContext {
	std::string username;
	std::string password;
	std::string email;
	std::string source_loginserver = "local";
	uint32      login_account_id   = 0;
	bool        password_is_encrypted = false;
};

namespace LS {
	namespace ServerStatusFlags {
		enum eServerStatusFlags {
			Up     = 0, // default
			Down   = 1,
			Unused = 2,
			Locked = 4  // can be combined with Down to show "Locked (Down)"
		};
	}

	namespace ServerTypeFlags {
		enum eServerTypeFlags {
			None      = 0,
			Standard  = 1,
			Unknown2  = 2,
			Unknown4  = 4,
			Preferred = 8,
			Legends   = 16 // can be combined with Preferred flag to override color in Legends section with Preferred color (green)
		};
	}

	enum ServerType {
		Standard  = 3,
		Preferred = 2,
		Legends   = 1,
	};

	namespace ErrStr {
		constexpr static int ERROR_NONE               = 101; // No Error
		constexpr static int ERROR_UNKNOWN            = 102; // Error - Unknown Error Occurred
		constexpr static int ERROR_ACTIVE_CHARACTER   = 111; // Error 1018: You currently have an active character on that EverQuest Server, please allow a minute for synchronization and try again.
		constexpr static int ERROR_SERVER_UNAVAILABLE = 326; // That server is currently unavailable.  Please check the EverQuest webpage for current server status and try again later.
		constexpr static int ERROR_ACCOUNT_SUSPENDED  = 337; // This account is currently suspended.  Please contact customer service for more information.
		constexpr static int ERROR_ACCOUNT_BANNED     = 338; // This account is currently banned.  Please contact customer service for more information.
		constexpr static int ERROR_WORLD_MAX_CAPACITY = 339; // The world server is currently at maximum capacity and not allowing further logins until the number of players online decreases.  Please try again later.
	};
}

#pragma pack(pop)
