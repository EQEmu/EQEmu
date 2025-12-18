#include "account_management.h"

#include "common/event/task_scheduler.h"
#include "common/repositories/login_accounts_repository.h"
#include "loginserver/login_server.h"

EQ::Event::TaskScheduler task_runner;

uint64 AccountManagement::CreateLoginServerAccount(LoginAccountContext c)
{
	if (LoginAccountsRepository::GetAccountFromContext(database, c).id > 0) {
		LogWarning(
			"Attempting to create local login account for user [{}] but already exists!",
			c.username
		);

		return -1;
	}

	auto a = LoginAccountsRepository::CreateAccountFromContext(database, c);
	if (a.id > 0) {
		return (int64) a.id;
	}

	LogError("Failed to create local login account for user [{}] !", c.username);

	return 0;
}

uint64 AccountManagement::CheckLoginserverUserCredentials(LoginAccountContext c)
{
	auto mode = server.options.GetEncryptionMode();
	auto a    = LoginAccountsRepository::GetAccountFromContext(database, c);
	if (!a.id) {
		LogError(
			"account [{}] source_loginserver [{}] not found!",
			c.username,
			c.source_loginserver
		);

		return 0;
	}

	bool validated_credentials = eqcrypt_verify_hash(c.username, c.password, a.account_password, mode);
	if (!validated_credentials) {
		LogError(
			"account [{}] source_loginserver [{}] invalid credentials!",
			c.username,
			c.source_loginserver
		);

		return 0;
	}

	LogInfo(
		"account [{}] source_loginserver [{}] credentials validated success!",
		c.username,
		c.source_loginserver
	);

	return a.id;
}

bool AccountManagement::UpdateLoginserverUserCredentials(LoginAccountContext c)
{
	auto a = LoginAccountsRepository::GetAccountFromContext(database, c);
	if (!a.id) {
		LogError(
			"account [{}] source_loginserver [{}] not found!",
			c.username,
			c.source_loginserver
		);

		return false;
	}

	LoginAccountsRepository::UpdateAccountPassword(database, a, c.password);

	LogInfo(
		"account [{}] source_loginserver [{}] credentials updated!",
		c.username,
		c.source_loginserver
	);

	return true;
}

bool AccountManagement::UpdateLoginserverWorldAdminAccountPasswordByName(LoginAccountContext c)
{
	auto mode = server.options.GetEncryptionMode();
	auto hash = eqcrypt_hash(
		c.username,
		c.password,
		mode
	);

	auto a = LoginServerAdminsRepository::GetByName(database, c.username);
	if (!a.id) {
		LogError(
			"account_name [{}] not found!",
			c.username
		);

		return false;
	}

	a.account_password = hash;
	auto updated_account = LoginServerAdminsRepository::UpdateOne(database, a);

	LogInfo(
		"account_name [{}] status [{}]",
		c.username,
		(updated_account ? "success" : "failed")
	);

	return updated_account;
}

constexpr int REQUEST_TIMEOUT_MS = 1500;

uint64 AccountManagement::CheckExternalLoginserverUserCredentials(LoginAccountContext c)
{
	auto res = task_runner.Enqueue(
		[&]() -> uint32 {
			bool   running = true;
			uint32 ret     = 0;

			EQ::Net::ReliableStreamConnectionManager           mgr;
			std::shared_ptr<EQ::Net::ReliableStreamConnection> conn;

			mgr.OnNewConnection(
				[&](std::shared_ptr<EQ::Net::ReliableStreamConnection> connection) {
					conn = connection;
				}
			);

			mgr.OnConnectionStateChange(
				[&](
					std::shared_ptr<EQ::Net::ReliableStreamConnection> conn,
					EQ::Net::DbProtocolStatus from,
					EQ::Net::DbProtocolStatus to
				) {
					if (EQ::Net::StatusConnected == to) {
						EQ::Net::DynamicPacket p;
						p.PutUInt16(0, 1); //OP_SessionReady
						p.PutUInt32(2, 2);
						conn->QueuePacket(p);
					}
					else if (EQ::Net::StatusDisconnected == to) {
						running = false;
					}
				}
			);

			mgr.OnPacketRecv(
				[&](std::shared_ptr<EQ::Net::ReliableStreamConnection> conn, const EQ::Net::Packet &p) {
					auto opcode = p.GetUInt16(0);
					switch (opcode) {
						case 0x0017: //OP_ChatMessage
						{
							size_t buffer_len = c.username.length() + c.password.length() + 2;

							std::unique_ptr<char[]> buffer(new char[buffer_len]);

							strcpy(&buffer[0], c.username.c_str());
							strcpy(&buffer[c.username.length() + 1], c.password.c_str());

							size_t encrypted_len = buffer_len;

							if (encrypted_len % 8 > 0) {
								encrypted_len = ((encrypted_len / 8) + 1) * 8;
							}

							EQ::Net::DynamicPacket p;
							p.Resize(12 + encrypted_len);
							p.PutUInt16(0, 2); //OP_Login
							p.PutUInt32(2, 3);

							eqcrypt_block(&buffer[0], buffer_len, (char *) p.Data() + 12, true);
							conn->QueuePacket(p);
							break;
						}
						case 0x0018: {
							auto encrypt_size = p.Length() - 12;
							if (encrypt_size % 8 > 0) {
								encrypt_size = (encrypt_size / 8) * 8;
							}

							std::unique_ptr<char[]> decrypted(new char[encrypt_size]);

							eqcrypt_block((char *) p.Data() + 12, encrypt_size, &decrypted[0], false);

							EQ::Net::StaticPacket sp(&decrypted[0], encrypt_size);
							auto                  response_error = sp.GetUInt16(1);
							auto                  m_dbid         = sp.GetUInt32(8);

							{
								ret     = (response_error <= 101 ? m_dbid : 0);
								running = false;
							}
							break;
						}
					}
				}
			);

			auto s = Strings::Split(server.options.GetEQEmuLoginServerAddress(), ':');
			if (s.size() == 2) {
				auto address = s[0];
				auto port    = Strings::ToInt(s[1]);

				EQ::Net::DNSLookup(
					address, port, false, [&](const std::string &addr) {
						if (addr.empty()) {
							ret     = 0;
							running = false;
						}

						mgr.Connect(addr, port);
					}
				);
			}

			std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

			auto &loop = EQ::EventLoop::Get();
			while (running) {
				std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
				if (std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() > REQUEST_TIMEOUT_MS) {
					LogInfo("Deadline exceeded [{}]", REQUEST_TIMEOUT_MS);
					running = false;
				}

				loop.Process();
			}

			return ret;
		}
	);

	return res.get();
}

uint64 AccountManagement::HealthCheckUserLogin()
{
	std::string in_account_username = "healthcheckuser";
	std::string in_account_password = "healthcheckpassword";

	auto res = task_runner.Enqueue(
		[&]() -> uint64 {
			bool   running = true;
			uint64 ret     = 0;

			EQ::Net::ReliableStreamConnectionManager           mgr;
			std::shared_ptr<EQ::Net::ReliableStreamConnection> c;

			mgr.OnNewConnection(
				[&](std::shared_ptr<EQ::Net::ReliableStreamConnection> connection) {
					c = connection;
				}
			);

			mgr.OnConnectionStateChange(
				[&](
					std::shared_ptr<EQ::Net::ReliableStreamConnection> conn,
					EQ::Net::DbProtocolStatus from,
					EQ::Net::DbProtocolStatus to
				) {
					if (EQ::Net::StatusConnected == to) {
						EQ::Net::DynamicPacket p;
						p.PutUInt16(0, 1); //OP_SessionReady
						p.PutUInt32(2, 2);
						c->QueuePacket(p);
					}
					else if (EQ::Net::StatusDisconnected == to) {
						running = false;
					}
				}
			);

			mgr.OnPacketRecv(
				[&](std::shared_ptr<EQ::Net::ReliableStreamConnection> conn, const EQ::Net::Packet &p) {
					auto opcode = p.GetUInt16(0);
					switch (opcode) {
						case 0x0017: //OP_ChatMessage
						{
							size_t buffer_len =
									   in_account_username.length() + in_account_password.length() + 2;

							std::unique_ptr<char[]> buffer(new char[buffer_len]);

							strcpy(&buffer[0], in_account_username.c_str());
							strcpy(&buffer[in_account_username.length() + 1], in_account_password.c_str());

							size_t encrypted_len = buffer_len;

							if (encrypted_len % 8 > 0) {
								encrypted_len = ((encrypted_len / 8) + 1) * 8;
							}

							EQ::Net::DynamicPacket p;
							p.Resize(12 + encrypted_len);
							p.PutUInt16(0, 2); //OP_Login
							p.PutUInt32(2, 3);

							eqcrypt_block(&buffer[0], buffer_len, (char *) p.Data() + 12, true);
							c->QueuePacket(p);
							break;
						}
						case 0x0018: {
							auto encrypt_size = p.Length() - 12;
							if (encrypt_size % 8 > 0) {
								encrypt_size = (encrypt_size / 8) * 8;
							}

							std::unique_ptr<char[]> decrypted(new char[encrypt_size]);

							eqcrypt_block((char *) p.Data() + 12, encrypt_size, &decrypted[0], false);

							EQ::Net::StaticPacket sp(&decrypted[0], encrypt_size);
							auto                  response_error = sp.GetUInt16(1);

							{
								// we only care to see the response code
								ret     = response_error;
								running = false;
							}
							break;
						}
					}
				}
			);

			mgr.Connect("127.0.0.1", 5999);

			std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
			auto                                  &loop = EQ::EventLoop::Get();
			while (running) {
				std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
				if (std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() > 2000) {
					ret     = 0;
					running = false;
				}

				loop.Process();
			}

			return ret;
		}
	);

	return res.get();
}

bool AccountManagement::CreateLoginserverWorldAdminAccount(
	const std::string &username,
	const std::string &password,
	const std::string &email,
	const std::string &first_name,
	const std::string &last_name,
	const std::string &ip_address
)
{
	auto mode = server.options.GetEncryptionMode();
	auto hash = eqcrypt_hash(username, password, mode);

	LogInfo(
		"Attempting to create world admin account | username [{}] encryption algorithm [{}] ({})",
		username,
		GetEncryptionByModeId(mode),
		mode
	);

	auto a = LoginServerAdminsRepository::GetByName(database, username);
	if (a.id > 0) {
		LogWarning(
			"Attempting to create world admin account for user [{}] but already exists!",
			username
		);

		return false;
	}

	a = LoginServerAdminsRepository::NewEntity();
	a.account_name            = username;
	a.account_password        = hash;
	a.first_name              = first_name;
	a.last_name               = last_name;
	a.email                   = email;
	a.registration_ip_address = ip_address;
	a.registration_date       = std::time(nullptr);

	a = LoginServerAdminsRepository::InsertOne(database, a);

	if (a.id > 0) {
		LogInfo(
			"Account creation success for user [{}] encryption algorithm [{}] ({}) new admin id [{}]",
			username,
			GetEncryptionByModeId(mode),
			mode,
			a.id
		);
		return true;
	}

	LogError("Failed to create world admin account account for user [{}] !", username);

	return false;
}
