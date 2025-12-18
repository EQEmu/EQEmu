#include "client_manager.h"

#include "common/file.h"
#include "common/misc.h"
#include "common/path_manager.h"
#include "loginserver/login_server.h"

extern LoginServer server;
extern bool        run_server;

void CheckTitaniumOpcodeFile(const std::string &path)
{
	if (File::Exists(path)) {
		return;
	}

	auto f = fopen(path.c_str(), "w");
	if (f) {
		fprintf(f, "#EQEmu Public Login Server OPCodes\n");
		fprintf(f, "OP_SessionReady=0x0001\n");
		fprintf(f, "OP_Login=0x0002\n");
		fprintf(f, "OP_ServerListRequest=0x0004\n");
		fprintf(f, "OP_PlayEverquestRequest=0x000d\n");
		fprintf(f, "OP_PlayEverquestResponse=0x0021\n");
		fprintf(f, "OP_ChatMessage=0x0016\n");
		fprintf(f, "OP_LoginAccepted=0x0017\n");
		fprintf(f, "OP_ServerListResponse=0x0018\n");
		fprintf(f, "OP_Poll=0x0029\n");
		fprintf(f, "OP_EnterChat=0x000f\n");
		fprintf(f, "OP_PollResponse=0x0011\n");
		fclose(f);
	}
}

void CheckSoDOpcodeFile(const std::string &path)
{
	if (File::Exists(path)) {
		return;
	}

	auto f = fopen(path.c_str(), "w");
	if (f) {
		fprintf(f, "#EQEmu Public Login Server OPCodes\n");
		fprintf(f, "OP_SessionReady=0x0001\n");
		fprintf(f, "OP_Login=0x0002\n");
		fprintf(f, "OP_ServerListRequest=0x0004\n");
		fprintf(f, "OP_PlayEverquestRequest=0x000d\n");
		fprintf(f, "OP_PlayEverquestResponse=0x0022\n");
		fprintf(f, "OP_ChatMessage=0x0017\n");
		fprintf(f, "OP_LoginAccepted=0x0018\n");
		fprintf(f, "OP_ServerListResponse=0x0019\n");
		fprintf(f, "OP_Poll=0x0029\n");
		fprintf(f, "OP_LoginExpansionPacketData=0x0031\n");
		fprintf(f, "OP_EnterChat=0x000f\n");
		fprintf(f, "OP_PollResponse=0x0011\n");
		fclose(f);
	}
}

void CheckLarionOpcodeFile(const std::string &path)
{
	if (File::Exists(path)) {
		return;
	}

	auto f = fopen(path.c_str(), "w");
	if (f) {
		fprintf(f, "#EQEmu Public Login Server OPCodes\n");
		fprintf(f, "OP_SessionReady=0x0001\n");
		fprintf(f, "OP_Login=0x0002\n");
		fprintf(f, "OP_ServerListRequest=0x0004\n");
		fprintf(f, "OP_PlayEverquestRequest=0x000d\n");
		fprintf(f, "OP_PlayEverquestResponse=0x0022\n");
		fprintf(f, "OP_ChatMessage=0x0017\n");
		fprintf(f, "OP_LoginAccepted=0x0018\n");
		fprintf(f, "OP_ServerListResponse=0x0019\n");
		fprintf(f, "OP_Poll=0x0029\n");
		fprintf(f, "OP_EnterChat=0x000f\n");
		fprintf(f, "OP_PollResponse=0x0011\n");
		fprintf(f, "OP_SystemFingerprint=0x0016\n");
		fprintf(f, "OP_ExpansionList=0x0030\n");
		fclose(f);
	}
}

ClientManager::ClientManager()
{
	int titanium_port = server.config.GetVariableInt("client_configuration", "titanium_port", 5998);

	EQStreamManagerInterfaceOptions titanium_opts(titanium_port, false, false);

	m_titanium_stream = new EQ::Net::EQStreamManager(titanium_opts);
	m_titanium_ops    = new RegularOpcodeManager;

	std::string opcodes_path = fmt::format(
		"{}/{}",
		PathManager::Instance()->GetOpcodePath(),
		"login_opcodes.conf"
	);

	CheckTitaniumOpcodeFile(opcodes_path);

	if (!m_titanium_ops->LoadOpcodes(opcodes_path.c_str())) {
		LogError(
			"ClientManager fatal error: couldn't load opcodes for Titanium file [{}]",
			server.config.GetVariableString("client_configuration", "titanium_opcodes", "login_opcodes.conf")
		);

		run_server = false;
	}

	m_titanium_stream->OnNewConnection(
		[this](std::shared_ptr<EQ::Net::EQStream> stream) {
			LogInfo(
				"New Titanium client connection from [{}:{}]",
				long2ip(stream->GetRemoteIP()),
				stream->GetRemotePort()
			);

			stream->SetOpcodeManager(&m_titanium_ops);
			Client *c = new Client(stream, cv_titanium);
			m_clients.push_back(c);
		}
	);

	int sod_port = server.config.GetVariableInt("client_configuration", "sod_port", 5999);

	EQStreamManagerInterfaceOptions sod_opts(sod_port, false, false);
	m_sod_stream = new EQ::Net::EQStreamManager(sod_opts);
	m_sod_ops    = new RegularOpcodeManager;

	opcodes_path = fmt::format(
		"{}/{}",
		PathManager::Instance()->GetOpcodePath(),
		"login_opcodes_sod.conf"
	);

	CheckSoDOpcodeFile(opcodes_path);

	if (!m_sod_ops->LoadOpcodes(opcodes_path.c_str())) {
		LogError(
			"ClientManager fatal error: couldn't load opcodes for SoD file {}",
			server.config.GetVariableString("client_configuration", "sod_opcodes", "login_opcodes.conf").c_str()
		);

		run_server = false;
	}

	m_sod_stream->OnNewConnection(
		[this](std::shared_ptr<EQ::Net::EQStream> stream) {
			LogInfo(
				"New SoD+ client connection from [{}:{}]",
				long2ip(stream->GetRemoteIP()),
				stream->GetRemotePort()
			);

			stream->SetOpcodeManager(&m_sod_ops);
			auto *c = new Client(stream, cv_sod);
			m_clients.push_back(c);
		}
	);

	int larion_port = server.config.GetVariableInt("client_configuration", "larion_port", 15900);

	EQStreamManagerInterfaceOptions larion_opts(larion_port, false, false);

	m_larion_stream = new EQ::Net::EQStreamManager(larion_opts);
	m_larion_ops    = new RegularOpcodeManager;

	opcodes_path = fmt::format(
		"{}/{}",
		PathManager::Instance()->GetOpcodePath(),
		"login_opcodes_larion.conf"
	);

	CheckLarionOpcodeFile(opcodes_path);

	if (!m_larion_ops->LoadOpcodes(opcodes_path.c_str())) {
		LogError(
			"ClientManager fatal error: couldn't load opcodes for Larion file [{}]",
			server.config.GetVariableString("client_configuration", "larion_opcodes", "login_opcodes.conf")
		);

		run_server = false;
	}

	m_larion_stream->OnNewConnection(
		[this](std::shared_ptr<EQ::Net::EQStream> stream) {
			LogInfo(
				"New Larion client connection from [{}:{}]",
				long2ip(stream->GetRemoteIP()),
				stream->GetRemotePort()
			);

			stream->SetOpcodeManager(&m_larion_ops);
			Client *c = new Client(stream, cv_larion);
			m_clients.push_back(c);
		}
	);
}

ClientManager::~ClientManager()
{
	delete m_titanium_stream;
	delete m_titanium_ops;
	delete m_sod_stream;
	delete m_sod_ops;
}

void ClientManager::Process()
{
	ProcessDisconnect();

	for (auto it = m_clients.begin(); it != m_clients.end();) {
		Client *c = *it;
		if (!c->Process()) {
			LogWarning("Client had a fatal error and had to be removed from the login");
			delete c;
			it = m_clients.erase(it);
		}
		else {
			++it;
		}
	}
}

void ClientManager::ProcessDisconnect()
{
	auto it = m_clients.begin();
	while (it != m_clients.end()) {
		Client *c = *it;
		if (c->GetConnection()->CheckState(CLOSED)) {
			LogInfo("Client disconnected from the server, removing client");
			delete c;
			it = m_clients.erase(it);
		}
		else {
			++it;
		}
	}
}

void ClientManager::RemoveExistingClient(unsigned int account_id, const std::string &loginserver)
{
	auto it = m_clients.begin();
	while (it != m_clients.end()) {
		Client *c = *it;
		if (c->GetAccountID() == account_id && c->GetLoginServerName() == loginserver) {
			LogInfo("Client attempting to log in existing client already logged in, removing existing client");
			delete c;
			it = m_clients.erase(it);
		}
		else {
			++it;
		}
	}
}

Client *ClientManager::GetClient(unsigned int account_id, const std::string &loginserver)
{
	auto iter = std::find_if(
		m_clients.begin(), m_clients.end(),
		[&](Client *c) {
			return c->GetAccountID() == account_id && c->GetLoginServerName() == loginserver;
		}
	);

	return (iter != m_clients.end()) ? *iter : nullptr;
}
