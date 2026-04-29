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
#include "encryption.h"
#include "common/compiler_macros.h"

#ifdef EQEMU_USE_OPENSSL
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/provider.h>
#endif
#ifdef EQEMU_USE_MBEDTLS
#include <mbedtls/des.h>
#include <mbedtls/md5.h>
#include <mbedtls/sha1.h>
#include <mbedtls/sha512.h>
#endif

#include <cstring>
#include <string>

#include <memory>

#ifdef ENABLE_SECURITY

#include <sodium.h>

#endif

std::string GetEncryptionByModeId(uint32 mode)
{
	switch (mode) {
		case EncryptionModeMD5:
			return "MD5";
		case EncryptionModeMD5PassUser:
			return "MD5PassUser";
		case EncryptionModeMD5UserPass:
			return "MD5UserPass";
		case EncryptionModeMD5Triple:
			return "MD5Triple";
		case EncryptionModeSHA:
			return "SHA";
		case EncryptionModeSHAPassUser:
			return "SHAPassUser";
		case EncryptionModeSHAUserPass:
			return "SHAUserPass";
		case EncryptionModeSHATriple:
			return "SHATriple";
		case EncryptionModeSHA512:
			return "SHA512";
		case EncryptionModeSHA512PassUser:
			return "SHA512PassUser";
		case EncryptionModeSHA512UserPass:
			return "SHA512UserPass";
		case EncryptionModeSHA512Triple:
			return "SHA512Triple";
		case EncryptionModeArgon2:
			return "Argon2";
		case EncryptionModeSCrypt:
			return "SCrypt";
		default:
			return "";
	}
}

const char *eqcrypt_block(const char *buffer_in, size_t buffer_in_sz, char *buffer_out, bool enc)
{
#ifdef EQEMU_USE_MBEDTLS
	if (enc) {
		if (buffer_in_sz % 8 != 0) {
			auto temp_buffer_sz = ((buffer_in_sz / 8) + 1) * 8;
			unsigned char *temp_buffer = new unsigned char[temp_buffer_sz];
			unsigned char *temp_buffer_in = &temp_buffer[0];
			unsigned char *temp_buffer_out = &temp_buffer[temp_buffer_sz];

			memset(temp_buffer, 0, temp_buffer_sz * 2);
			memcpy(temp_buffer_in, buffer_in, buffer_in_sz);

			unsigned char key[MBEDTLS_DES_KEY_SIZE];
			unsigned char iv[8];
			memset(&key, 0, MBEDTLS_DES_KEY_SIZE);
			memset(&iv, 0, 8);

			mbedtls_des_context context;
			mbedtls_des_setkey_enc(&context, key);
			mbedtls_des_crypt_cbc(&context, MBEDTLS_DES_ENCRYPT, temp_buffer_sz, iv, (const unsigned char*)temp_buffer_in, (unsigned char*)temp_buffer_out);

			memcpy(buffer_out, temp_buffer_out, temp_buffer_sz);
			delete[] temp_buffer;
		}
		else {
			unsigned char key[MBEDTLS_DES_KEY_SIZE];
			unsigned char iv[8];
			memset(&key, 0, MBEDTLS_DES_KEY_SIZE);
			memset(&iv, 0, 8);

			mbedtls_des_context context;
			mbedtls_des_setkey_enc(&context, key);
			mbedtls_des_crypt_cbc(&context, MBEDTLS_DES_ENCRYPT, buffer_in_sz, iv, (const unsigned char*)buffer_in, (unsigned char*)buffer_out);
		}
	}
	else {
		if (buffer_in_sz && buffer_in_sz % 8 != 0) {
			return nullptr;
		}

		unsigned char key[MBEDTLS_DES_KEY_SIZE];
		unsigned char iv[8];
		memset(&key, 0, MBEDTLS_DES_KEY_SIZE);
		memset(&iv, 0, 8);

		mbedtls_des_context context;
		mbedtls_des_setkey_dec(&context, key);
		mbedtls_des_crypt_cbc(&context, MBEDTLS_DES_DECRYPT, buffer_in_sz, iv, (const unsigned char*)buffer_in, (unsigned char*)buffer_out);
	}
#endif

#ifdef EQEMU_USE_OPENSSL
	// Decrypt requires block-aligned input; encrypt zero-pads a trailing
	// partial block to match the legacy DES_ncbc_encrypt semantics the
	// game protocol expects.
	if (!enc && buffer_in_sz && buffer_in_sz % 8 != 0) {
		return nullptr;
	}

	unsigned char key[8] = {0};
	unsigned char iv[8]  = {0};

	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	if (!ctx) {
		return nullptr;
	}

	bool result = EVP_CipherInit_ex2(ctx, EVP_des_cbc(), key, iv, enc, nullptr) == 1;
	if (result) {
		EVP_CIPHER_CTX_set_padding(ctx, 0);

		const unsigned char* src = reinterpret_cast<const unsigned char*>(buffer_in);
		size_t src_len = buffer_in_sz;
		std::unique_ptr<unsigned char[]> padded;

		if (enc && buffer_in_sz % 8 != 0) {
			src_len = ((buffer_in_sz / 8) + 1) * 8;
			padded.reset(new unsigned char[src_len]());
			memcpy(padded.get(), buffer_in, buffer_in_sz);
			src = padded.get();
		}

		int outl = 0;
		int final_len = 0;
		result = EVP_CipherUpdate(ctx, reinterpret_cast<unsigned char*>(buffer_out), &outl, src, static_cast<int>(src_len)) == 1
			&& EVP_CipherFinal_ex(ctx, reinterpret_cast<unsigned char*>(buffer_out) + outl, &final_len) == 1;
	}

	EVP_CIPHER_CTX_free(ctx);
	if (!result) {
		return nullptr;
	}
#endif
	return buffer_out;
}

#ifdef EQEMU_USE_OPENSSL
static OSSL_PROVIDER *s_legacy_provider  = nullptr;
static OSSL_PROVIDER *s_default_provider = nullptr;
#endif

bool eqcrypt_init()
{
#ifdef EQEMU_USE_OPENSSL
	if (!s_default_provider) {
		s_default_provider = OSSL_PROVIDER_load(nullptr, "default");
	}
	if (!s_legacy_provider) {
		s_legacy_provider = OSSL_PROVIDER_load(nullptr, "legacy");
	}

	if (!s_default_provider || !s_legacy_provider) {
		char buf[256];
		while (auto err = ERR_get_error()) {
			ERR_error_string_n(err, buf, sizeof(buf));
			LogError("OpenSSL provider load failure: {}", buf);
		}
		return false;
	}
#endif

	return true;
}

void eqcrypt_shutdown()
{
#ifdef EQEMU_USE_OPENSSL
	if (s_legacy_provider) {
		OSSL_PROVIDER_unload(s_legacy_provider);
		s_legacy_provider = nullptr;
	}
	if (s_default_provider) {
		OSSL_PROVIDER_unload(s_default_provider);
		s_default_provider = nullptr;
	}
#endif
}

std::string eqcrypt_md5(const std::string &msg)
{
	std::string ret;
	ret.reserve(32);

#ifdef EQEMU_USE_MBEDTLS
	unsigned char digest[16];
	char temp[4];

	if (0 == mbedtls_md5_ret((const unsigned char*)msg.c_str(), msg.length(), digest)) {
		for (int i = 0; i < 16; ++i) {
			sprintf(&temp[0], "%02x", digest[i]);
			ret.push_back(temp[0]);
			ret.push_back(temp[1]);
		}
	}
#endif

#ifdef EQEMU_USE_OPENSSL
	unsigned char md5_digest[16];
	char tmp[4];

	if (EVP_Digest(msg.data(), msg.length(), md5_digest, nullptr, EVP_md5(), nullptr) == 1) {
		for (int i = 0; i < 16; ++i) {
			sprintf(&tmp[0], "%02x", md5_digest[i]);
			ret.push_back(tmp[0]);
			ret.push_back(tmp[1]);
		}
	}
#endif

	return ret;
}

std::string eqcrypt_sha1(const std::string &msg)
{
	std::string ret;
	ret.reserve(40);

#ifdef EQEMU_USE_MBEDTLS
	unsigned char digest[20];
	char temp[4];

	if (0 == mbedtls_sha1_ret((const unsigned char*)msg.c_str(), msg.length(), digest)) {
		for (int i = 0; i < 20; ++i) {
			sprintf(&temp[0], "%02x", digest[i]);
			ret.push_back(temp[0]);
			ret.push_back(temp[1]);
		}
	}
#endif

#ifdef EQEMU_USE_OPENSSL
	unsigned char sha_digest[20];
	char tmp[4];

	if (EVP_Digest(msg.data(), msg.length(), sha_digest, nullptr, EVP_sha1(), nullptr) == 1) {
		for (int i = 0; i < 20; ++i) {
			sprintf(&tmp[0], "%02x", sha_digest[i]);
			ret.push_back(tmp[0]);
			ret.push_back(tmp[1]);
		}
	}
#endif

	return ret;
}

std::string eqcrypt_sha512(const std::string &msg)
{
	std::string ret;
	ret.reserve(128);

#ifdef EQEMU_USE_MBEDTLS
	unsigned char digest[64];
	char temp[4];

	if (0 == mbedtls_sha512_ret((const unsigned char*)msg.c_str(), msg.length(), digest, 0)) {
		for (int i = 0; i < 64; ++i) {
			sprintf(&temp[0], "%02x", digest[i]);
			ret.push_back(temp[0]);
			ret.push_back(temp[1]);
		}
	}
#endif

#ifdef EQEMU_USE_OPENSSL
	unsigned char sha_digest[64];
	char tmp[4];

	if (EVP_Digest(msg.data(), msg.length(), sha_digest, nullptr, EVP_sha512(), nullptr) == 1) {
		for (int i = 0; i < 64; ++i) {
			sprintf(&tmp[0], "%02x", sha_digest[i]);
			ret.push_back(tmp[0]);
			ret.push_back(tmp[1]);
		}
	}
#endif

	return ret;
}

#ifdef ENABLE_SECURITY

std::string eqcrypt_argon2(const std::string &msg)
{
	char        buffer[crypto_pwhash_STRBYTES] = {0};
	std::string ret;

	if (crypto_pwhash_str(
		&buffer[0],
		&msg[0],
		msg.length(),
		crypto_pwhash_OPSLIMIT_INTERACTIVE,
		crypto_pwhash_MEMLIMIT_INTERACTIVE
	) != 0) {
		return "";
	}

	ret = buffer;
	return ret;
}

std::string eqcrypt_scrypt(const std::string &msg)
{
	char        buffer[crypto_pwhash_scryptsalsa208sha256_STRBYTES] = {0};
	std::string ret;

	if (crypto_pwhash_scryptsalsa208sha256_str(
		&buffer[0],
		&msg[0],
		msg.length(),
		crypto_pwhash_scryptsalsa208sha256_OPSLIMIT_INTERACTIVE,
		crypto_pwhash_scryptsalsa208sha256_MEMLIMIT_INTERACTIVE
	) != 0) {
		return "";
	}

	ret = buffer;
	return ret;
}

#endif

std::string eqcrypt_hash(const std::string &username, const std::string &password, int mode)
{
	switch (mode) {
		case EncryptionModeMD5:
			return eqcrypt_md5(password);
		case EncryptionModeMD5PassUser:
			return eqcrypt_md5(password + ":" + username);
		case EncryptionModeMD5UserPass:
			return eqcrypt_md5(username + ":" + password);
		case EncryptionModeMD5Triple:
			return eqcrypt_md5(eqcrypt_md5(username) + eqcrypt_md5(password));
		case EncryptionModeSHA:
			return eqcrypt_sha1(password);
		case EncryptionModeSHAPassUser:
			return eqcrypt_sha1(password + ":" + username);
		case EncryptionModeSHAUserPass:
			return eqcrypt_sha1(username + ":" + password);
		case EncryptionModeSHATriple:
			return eqcrypt_sha1(eqcrypt_sha1(username) + eqcrypt_sha1(password));
		case EncryptionModeSHA512:
			return eqcrypt_sha512(password);
		case EncryptionModeSHA512PassUser:
			return eqcrypt_sha512(password + ":" + username);
		case EncryptionModeSHA512UserPass:
			return eqcrypt_sha512(username + ":" + password);
		case EncryptionModeSHA512Triple:
			return eqcrypt_sha512(eqcrypt_sha512(username) + eqcrypt_sha512(password));
#ifdef ENABLE_SECURITY
		case EncryptionModeArgon2:
			return eqcrypt_argon2(password);
		case EncryptionModeSCrypt:
			return eqcrypt_scrypt(password);
#endif
			//todo bcrypt? pbkdf2?
		default:
			return "";
			break;
	}
}

bool eqcrypt_verify_hash(const std::string &username, const std::string &password, const std::string &pwhash, int mode)
{
	switch (mode) {
#ifdef ENABLE_SECURITY
		case 13:
			return crypto_pwhash_str_verify(&pwhash[0], &password[0], password.length()) == 0;
		case 14:
			return crypto_pwhash_scryptsalsa208sha256_str_verify(&pwhash[0], &password[0], password.length()) == 0;
#endif
		default: {
			auto hash = eqcrypt_hash(username, password, mode);
			return hash.compare(pwhash) == 0;
		}
	}

	return false;
}
