#pragma once

#include "common/repositories/base/base_login_accounts_repository.h"

#include "common/database.h"
#include "common/strings.h"
#include "loginserver/encryption.h"
#include "loginserver/login_types.h"

class LoginAccountsRepository : public BaseLoginAccountsRepository {
public:
	static int64 GetFreeID(Database &db, const std::string &loginserver)
	{
		auto query = fmt::format(
			"SELECT IFNULL(MAX(id), 0) + 1 FROM login_accounts WHERE source_loginserver = '{}'",
			Strings::Escape(loginserver)
		);

		auto results = db.QueryDatabase(query);
		if (!results.Success() || results.RowCount() != 1) {
			return 0;
		}

		auto row = results.begin();

		return Strings::ToUnsignedInt(row[0]);
	}

	static LoginAccountsRepository::LoginAccounts CreateAccountFromContext(
		Database &db,
		LoginAccountContext c
	)
	{
		auto a = LoginAccountsRepository::NewEntity();

		if (!c.password_is_encrypted) {
			auto e = EncryptPasswordFromContext(c);
			a.account_password = e.password;
		}

		a.id                 = c.login_account_id > 0 ? c.login_account_id : GetFreeID(db, c.source_loginserver);
		a.account_name       = c.username;
		a.account_email      = !c.email.empty() ? c.email : "local_creation";
		a.source_loginserver = c.source_loginserver;
		a.last_ip_address    = "127.0.0.1";
		a.last_login_date    = std::time(nullptr);
		a.created_at         = std::time(nullptr);
		LoginAccountsRepository::InsertOne(db, a);

		return GetAccountFromContext(db, c).id > 0 ? a : NewEntity();
	}

	static LoginAccountsRepository::LoginAccounts GetAccountFromContext(
		Database &db,
		LoginAccountContext c
	)
	{
		std::string where = fmt::format(
			"account_name = '{}' AND source_loginserver = '{}'",
			Strings::Escape(c.username),
			Strings::Escape(c.source_loginserver)
		);

		if (!c.email.empty()) {
			where += fmt::format(" AND account_email = '{}'", Strings::Escape(c.email));
		}
		if (c.login_account_id > 0) {
			where += fmt::format(" AND id = {}", c.login_account_id);
		}

		where += " LIMIT 1";

		auto results = LoginAccountsRepository::GetWhere(db, where);

		auto e = LoginAccountsRepository::NewEntity();
		if (results.size() == 1) {
			e = results.front();
		}

		return e;
	}

	static LoginAccounts UpdateAccountPassword(Database &db, LoginAccounts a, std::string password)
	{
		LoginAccountContext c;
		c.username = a.account_name;
		c.password = password;
		auto e = EncryptPasswordFromContext(c);
		a.account_password = e.password;

		int success = LoginAccountsRepository::UpdateOne(db, a);

		return success ? a : NewEntity();
	}
};
