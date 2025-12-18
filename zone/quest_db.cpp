#include "quest_db.h"

#include "zone/zone_config.h"
#include "zone/zonedb.h"

// New connections avoid concurrency issues and allow use of unbuffered results
// with prepared statements. Using zone connections w/o buffering would cause
// "Commands out of sync" errors if any queries occur before results consumed.
QuestDB::QuestDB(Connection type, bool connect)
{
	if (connect)
	{
		m_db = std::unique_ptr<Database, Deleter>(new Database(), Deleter(true));

		const auto config = EQEmuConfig::get();

		if (type == Connection::Default || type == Connection::Content && config->ContentDbHost.empty())
		{
			m_db->Connect(config->DatabaseHost, config->DatabaseUsername, config->DatabasePassword,
				config->DatabaseDB, config->DatabasePort, "questdb");
		}
		else if (type == Connection::Content)
		{
			m_db->Connect(config->ContentDbHost, config->ContentDbUsername, config->ContentDbPassword,
				config->ContentDbName, config->ContentDbPort, "questdb");
		}
	}
	else if (type == Connection::Default)
	{
		m_db = std::unique_ptr<Database, Deleter>(&database, Deleter(false));
	}
	else if (type == Connection::Content)
	{
		m_db = std::unique_ptr<Database, Deleter>(&content_db, Deleter(false));
	}

	if (!m_db || (connect && m_db->GetStatus() != DBcore::Connected))
	{
		throw std::runtime_error(fmt::format("Failed to connect to db type [{}]", static_cast<int>(type)));
	}
}

QuestDB::QuestDB(const char* host, const char* user, const char* pass, const char* db, uint32_t port)
	: m_db(new Database(), Deleter(true))
{
	if (!m_db->Connect(host, user, pass, db, port, "questdb"))
	{
		throw std::runtime_error(fmt::format("Failed to connect to db [{}:{}]", host, port));
	}
}

void QuestDB::Deleter::operator()(Database* ptr) noexcept
{
	if (owner)
	{
		delete ptr;
	}
};
