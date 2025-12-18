#pragma once

#include "common/eqemu_logsys.h"
#include "common/strings.h"
#include "common/types.h"
#include "loginserver/login_types.h"

#include <string>

enum EncryptionMode {
	EncryptionModeMD5            = 1,
	EncryptionModeMD5PassUser    = 2,
	EncryptionModeMD5UserPass    = 3,
	EncryptionModeMD5Triple      = 4,
	EncryptionModeSHA            = 5,
	EncryptionModeSHAPassUser    = 6,
	EncryptionModeSHAUserPass    = 7,
	EncryptionModeSHATriple      = 8,
	EncryptionModeSHA512         = 9,
	EncryptionModeSHA512PassUser = 10,
	EncryptionModeSHA512UserPass = 11,
	EncryptionModeSHA512Triple   = 12,
	EncryptionModeArgon2         = 13,
	EncryptionModeSCrypt         = 14
};

namespace CryptoHash {
	const int md5_hash_length    = 32;
	const int sha1_hash_length   = 40;
	const int sha512_hash_length = 128;
}

std::string GetEncryptionByModeId(uint32 mode);
const char *eqcrypt_block(const char *buffer_in, size_t buffer_in_sz, char *buffer_out, bool enc);
std::string eqcrypt_hash(const std::string &username, const std::string &password, int mode);
bool eqcrypt_verify_hash(const std::string &username, const std::string &password, const std::string &pwhash, int mode);

struct EncryptionResult {
	std::string password;
	int         mode = 0;
	std::string mode_name;
};

static EncryptionResult EncryptPasswordFromContext(LoginAccountContext c, int mode = EncryptionModeSCrypt)
{
	if (mode == 0) {
		LogError("Encryption mode not set!");
		return {};
	}

	EncryptionResult r;
	r.password  = eqcrypt_hash(
		c.username,
		c.password,
		mode
	);
	r.mode      = mode;
	r.mode_name = GetEncryptionByModeId(r.mode);

	LogInfo("Encrypted password for user [{}] using mode [{}] ({})", c.username, r.mode_name, r.mode);

	return r;
}
