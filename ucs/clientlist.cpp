/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2008 EQEMu Development Team (http://eqemulator.net)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY except by those people which sell it, which
are required to give you total support for your newly bought product;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

*/

#include "common/eqemu_logsys.h"
#include "common/misc_functions.h"
#include "common/path_manager.h"
#include "common/strings.h"
#include "ucs/chatchannel.h"
#include "ucs/clientlist.h"
#include "ucs/database.h"
#include "ucs/ucsconfig.h"

#include <algorithm>
#include <cstdlib>
#include <list>
#include <string>
#include <vector>

extern UCSDatabase database;
extern std::string WorldShortName;
extern std::string GetMailPrefix();
extern ChatChannelList *ChannelList;
extern Clientlist *g_Clientlist;
extern uint32 ChatMessagesSent;
extern uint32 MailMessagesSent;

int LookupCommand(const char *ChatCommand) {

	if (!ChatCommand) return -1;

	for (int i = 0; i < CommandEndOfList; i++) {

		if (!strcasecmp(Commands[i].CommandString, ChatCommand)) {
			return Commands[i].CommandCode;
		}

	}

	return -1;
}

void Client::SendUptime() {

	uint32 ms = Timer::GetCurrentTime();
	uint32 d = ms / 86400000;
	ms -= d * 86400000;
	uint32 h = ms / 3600000;
	ms -= h * 3600000;
	uint32 m = ms / 60000;
	ms -= m * 60000;
	uint32 s = ms / 1000;

	auto message = fmt::format("UCS has been up for {:02}d {:02}h {:02}m {:02}s", d, h, m, s);
	GeneralChannelMessage(message);
	message = fmt::format("Chat Messages Sent: {}, Mail Messages Sent: {}", ChatMessagesSent, MailMessagesSent);
	GeneralChannelMessage(message);
}

std::vector<std::string> ParseRecipients(std::string RecipientString) {

	// This method parses the Recipient List in the mailto command, which can look like this example:
	//
	// "Baalinor <SOE.EQ.BTG2.Baalinor>,
	// -Friends <SOE.EQ.BTG2.Playedtest SOE.EQ.BTG2.Dyetest>,
	// Guild <SOE.EQ.BTG2.Dsfvxcbcx SOE.EQ.BTG2.Necronor>, SOE.EQ.BTG2.luccerathe, SOE.EQ.BTG2.codsas
	//
	// First, it splits it up at commas, so it looks like this:
	//
	// Baalinor <SOE.EQ.BTG2.Baalinor>
	// -Friends <SOE.EQ.BTG2.Playedtest SOE.EQ.BTG2.Dyetest>
	// Guild <SOE.EQ.BTG2.Dsfvxcbcx SOE.EQ.BTG2.Necronor>
	// SOE.EQ.BTG2.luccerathe
	// SOE.EQ.BTG2
	//
	// Then, if an entry has a '<' in it, it extracts the text between the < and >
	// If the text between the < and > has a space in it, then there are multiple addresses in there, so those are extracted.
	//
	// The prefix (SOE.EQ.<Shortname>) is discarded, the names are normalised so they begin with a single upper case character
	// followed by lower case.
	//
	// The vector is sorted and any duplicates discarded, so the vector we return, in our example, looks like this:
	//
	// Baalinor
	// -Playedtest
	// -Dyetest
	// Dsfvxcbcx
	// Necronor
	// Luccerathe
	// Codsas
	//
	// The '-' prefix indicates 'Secret To' (like BCC:)
	//
	std::vector<std::string> RecipientList;

	std::string Secret;

	std::string::size_type CurrentPos, Comma, FirstLT, LastGT, Space, LastPeriod;

	CurrentPos = 0;

	while (CurrentPos != std::string::npos) {

		Comma = RecipientString.find_first_of(",", CurrentPos);

		if (Comma == std::string::npos) {

			RecipientList.emplace_back(RecipientString.substr(CurrentPos));

			break;
		}
		RecipientList.emplace_back(RecipientString.substr(CurrentPos, Comma - CurrentPos));

		CurrentPos = Comma + 2;
	}

	std::vector<std::string>::iterator Iterator;

	Iterator = RecipientList.begin();

	while (Iterator != RecipientList.end()) {

		if ((*Iterator)[0] == '-') {

			Secret = "-";

			while ((*Iterator)[0] == '-')
				(*Iterator) = (*Iterator).substr(1);
		}
		else
			Secret = "";

		FirstLT = (*Iterator).find_first_of("<");

		if (FirstLT != std::string::npos) {

			LastGT = (*Iterator).find_last_of(">");

			if (LastGT != std::string::npos) {

				(*Iterator) = (*Iterator).substr(FirstLT + 1, LastGT - FirstLT - 1);

				if ((*Iterator).find_first_of(" ") != std::string::npos) {

					std::string Recips = (*Iterator);

					RecipientList.erase(Iterator);

					CurrentPos = 0;

					while (CurrentPos != std::string::npos) {

						Space = Recips.find_first_of(" ", CurrentPos);

						if (Space == std::string::npos) {

							RecipientList.push_back(Secret + Recips.substr(CurrentPos));

							break;
						}
						RecipientList.push_back(Secret + Recips.substr(CurrentPos,
							Space - CurrentPos));
						CurrentPos = Space + 1;
					}
					Iterator = RecipientList.begin();

					continue;
				}
			}
		}


		(*Iterator) = Secret + (*Iterator);

		++Iterator;

	}

	for (Iterator = RecipientList.begin(); Iterator != RecipientList.end(); ++Iterator) {

		if ((*Iterator).length() > 0) {

			if ((*Iterator)[0] == '-')
				Secret = "-";
			else
				Secret = "";

			LastPeriod = (*Iterator).find_last_of(".");

			if (LastPeriod != std::string::npos) {

				(*Iterator) = (*Iterator).substr(LastPeriod + 1);

				for (unsigned int i = 0; i < (*Iterator).length(); i++) {

					if (i == 0)
						(*Iterator)[i] = toupper((*Iterator)[i]);
					else
						(*Iterator)[i] = tolower((*Iterator)[i]);
				}

				(*Iterator) = Secret + (*Iterator);
			}
		}

	}

	sort(RecipientList.begin(), RecipientList.end());

	auto new_end_pos = unique(RecipientList.begin(), RecipientList.end());

	RecipientList.erase(new_end_pos, RecipientList.end());

	return RecipientList;

}

static void ProcessMailTo(Client *c, std::string MailMessage) {

	LogDebug("MAILTO: From [{}], [{}]", c->MailBoxName().c_str(), MailMessage.c_str());

	std::vector<std::string> Recipients;

	std::string::size_type FirstQuote = MailMessage.find_first_of("\"", 0);

	std::string::size_type NextQuote = MailMessage.find_first_of("\"", FirstQuote + 1);

	std::string RecipientsString = MailMessage.substr(FirstQuote + 1, NextQuote - FirstQuote - 1);

	Recipients = ParseRecipients(RecipientsString);

	// Now extract the subject field. This is in quotes if it is more than one word
	//
	std::string Subject;

	std::string::size_type SubjectStart = NextQuote + 2;

	std::string::size_type SubjectEnd;

	if (MailMessage.substr(SubjectStart, 1) == "\"") {

		SubjectEnd = MailMessage.find_first_of("\"", SubjectStart + 1);

		Subject = MailMessage.substr(SubjectStart + 1, SubjectEnd - SubjectStart - 1);

		SubjectEnd += 2;

	}
	else {
		SubjectEnd = MailMessage.find_first_of(" ", SubjectStart);

		Subject = MailMessage.substr(SubjectStart, SubjectEnd - SubjectStart);

		SubjectEnd++;

	}
	std::string Body = MailMessage.substr(SubjectEnd);

	bool Success = true;

	RecipientsString.clear();

	int VisibleRecipients = 0;

	for (auto &Recipient : Recipients) {

		if (Recipient[0] == '-') {

			Recipient = Recipient.substr(1);

		}
		else {
			if (VisibleRecipients > 0)

				RecipientsString += ", ";

			VisibleRecipients++;

			RecipientsString = RecipientsString + GetMailPrefix() + Recipient;
		}
	}
	if (VisibleRecipients == 0)
		RecipientsString = "<UNDISCLOSED RECIPIENTS>";

	for (auto &Recipient : Recipients) {

		if (!database.SendMail(Recipient, c->MailBoxName(), Subject, Body, RecipientsString)) {

			LogError("Failed in SendMail([{}], [{}], [{}], [{}])", Recipient.c_str(),
				c->MailBoxName().c_str(), Subject.c_str(), RecipientsString.c_str());

			int PacketLength = 10 + Recipient.length() + Subject.length();

			// Failure
			auto outapp = new EQApplicationPacket(OP_MailDeliveryStatus, PacketLength);

			char *PacketBuffer = (char *)outapp->pBuffer;

			VARSTRUCT_ENCODE_STRING(PacketBuffer, "1");
			VARSTRUCT_ENCODE_TYPE(uint8, PacketBuffer, 0x20);
			VARSTRUCT_ENCODE_STRING(PacketBuffer, Recipient.c_str());
			VARSTRUCT_ENCODE_STRING(PacketBuffer, Subject.c_str());
			VARSTRUCT_ENCODE_STRING(PacketBuffer, "0");
			VARSTRUCT_ENCODE_TYPE(uint16, PacketBuffer, 0x3237);
			VARSTRUCT_ENCODE_TYPE(uint8, PacketBuffer, 0x0);


			c->QueuePacket(outapp);

			safe_delete(outapp);

			Success = false;
		}
	}

	if (Success) {
		// Success
		auto outapp = new EQApplicationPacket(OP_MailDeliveryStatus, 10);

		char *PacketBuffer = (char *)outapp->pBuffer;

		VARSTRUCT_ENCODE_STRING(PacketBuffer, "1");
		VARSTRUCT_ENCODE_TYPE(uint8, PacketBuffer, 0);
		VARSTRUCT_ENCODE_STRING(PacketBuffer, "test"); // Doesn't matter what we send in this text field.
		VARSTRUCT_ENCODE_STRING(PacketBuffer, "1");


		c->QueuePacket(outapp);

		safe_delete(outapp);
	}
}

static void ProcessMailTo(Client *c, const std::string& from, const std::string& subject, const std::string& message) {
}

static void ProcessSetMessageStatus(std::string SetMessageCommand) {

	int MessageNumber;

	int Status;

	switch (SetMessageCommand[0]) {

	case 'R': // READ
		Status = 3;
		break;

	case 'T': // TRASH
		Status = 4;
		break;

	default: // DELETE
		Status = 0;

	}
	std::string::size_type NumStart = SetMessageCommand.find_first_of("123456789");

	while (NumStart != std::string::npos) {

		std::string::size_type NumEnd = SetMessageCommand.find_first_of(" ", NumStart);

		if (NumEnd == std::string::npos) {

			MessageNumber = Strings::ToInt(SetMessageCommand.substr(NumStart));

			database.SetMessageStatus(MessageNumber, Status);

			break;
		}

		MessageNumber = Strings::ToInt(SetMessageCommand.substr(NumStart, NumEnd - NumStart));

		database.SetMessageStatus(MessageNumber, Status);

		NumStart = SetMessageCommand.find_first_of("123456789", NumEnd);
	}
}

static void ProcessCommandBuddy(Client *c, std::string Buddy) {

	LogInfo("Received buddy command with parameters [{}]", Buddy.c_str());
	c->GeneralChannelMessage("Buddy list modified");

	uint8 SubAction = 1;

	if (Buddy.substr(0, 1) == "-")
		SubAction = 0;

	auto outapp = new EQApplicationPacket(OP_Buddy, Buddy.length() + 2);
	char *PacketBuffer = (char *)outapp->pBuffer;
	VARSTRUCT_ENCODE_TYPE(uint8, PacketBuffer, SubAction);

	if (SubAction == 1) {
		VARSTRUCT_ENCODE_STRING(PacketBuffer, Buddy.c_str());
		database.AddFriendOrIgnore(c->GetCharID(), 1, Buddy);
	}
	else {
		VARSTRUCT_ENCODE_STRING(PacketBuffer, Buddy.substr(1).c_str());
		database.RemoveFriendOrIgnore(c->GetCharID(), 1, Buddy.substr(1));
	}

	c->QueuePacket(outapp);

	safe_delete(outapp);

}

static void ProcessCommandIgnore(Client *c, std::string Ignoree) {

	LogInfo("Received ignore command with parameters [{}]", Ignoree.c_str());
	c->GeneralChannelMessage("Ignore list modified");

	uint8 SubAction = 0;

	if (Ignoree.substr(0, 1) == "-") {
		SubAction = 1;
		Ignoree = Ignoree.substr(1);

		// Strip off the SOE.EQ.<shortname>.
		//
		std::string CharacterName;

		std::string::size_type LastPeriod = Ignoree.find_last_of(".");

		if (LastPeriod == std::string::npos)
			CharacterName = Ignoree;
		else
			CharacterName = Ignoree.substr(LastPeriod + 1);

		database.RemoveFriendOrIgnore(c->GetCharID(), 0, CharacterName);

	}
	else
	{
		database.AddFriendOrIgnore(c->GetCharID(), 0, Ignoree);
		Ignoree = "SOE.EQ." + WorldShortName + "." + Ignoree;
	}

	auto outapp = new EQApplicationPacket(OP_Ignore, Ignoree.length() + 2);
	char *PacketBuffer = (char *)outapp->pBuffer;
	VARSTRUCT_ENCODE_TYPE(uint8, PacketBuffer, SubAction);

	VARSTRUCT_ENCODE_STRING(PacketBuffer, Ignoree.c_str());

	c->QueuePacket(outapp);

	safe_delete(outapp);

}

Clientlist::Clientlist(int ChatPort) {
	EQStreamManagerInterfaceOptions chat_opts(ChatPort, false, false);
	chat_opts.opcode_size = 1;
	chat_opts.reliable_stream_options.stale_connection_ms = 600000;
	chat_opts.reliable_stream_options.resend_delay_ms = RuleI(Network, ResendDelayBaseMS);
	chat_opts.reliable_stream_options.resend_delay_factor = RuleR(Network, ResendDelayFactor);
	chat_opts.reliable_stream_options.resend_delay_min = RuleI(Network, ResendDelayMinMS);
	chat_opts.reliable_stream_options.resend_delay_max = RuleI(Network, ResendDelayMaxMS);

	chatsf = new EQ::Net::EQStreamManager(chat_opts);

	ChatOpMgr = new RegularOpcodeManager;

	const ucsconfig *Config = ucsconfig::get();


	std::string opcodes_file = fmt::format("{}/{}", PathManager::Instance()->GetServerPath(), Config->MailOpCodesFile);

	LogInfo("Loading [{}]", opcodes_file);
	if (!ChatOpMgr->LoadOpcodes(opcodes_file.c_str()))
		exit(1);

	chatsf->OnNewConnection([this](std::shared_ptr<EQ::Net::EQStream> stream) {
		LogInfo("New Client UDP connection from [{0}] [{1}]", stream->GetRemoteIP(), stream->GetRemotePort());
		stream->SetOpcodeManager(&ChatOpMgr);

		auto c = new Client(stream);
		ClientChatConnections.push_back(c);
	});
}

Client::Client(std::shared_ptr<EQStreamInterface> eqs) {

	ClientStream = eqs;

	CurrentMailBox = 0;

	Announce = false;

	Status = 0;

	HideMe = 0;

	AccountID = 0;

	AllowInvites = true;
	Revoked = false;

	for (auto &elem : JoinedChannels)
		elem = nullptr;

	TotalKarma = 0;
	AttemptedMessages = 0;
	ForceDisconnect = false;

	AccountGrabUpdateTimer = new Timer(60000); //check every minute
	GlobalChatLimiterTimer = new Timer(RuleI(Chat, IntervalDurationMS));

	TypeOfConnection = ConnectionTypeUnknown;
	ClientVersion_ = EQ::versions::ClientVersion::Unknown;

	UnderfootOrLater = false;
}

Client::~Client() {

	CloseConnection();

	LeaveAllChannels(false);

	if (AccountGrabUpdateTimer)
	{
		delete AccountGrabUpdateTimer;
		AccountGrabUpdateTimer = nullptr;
	}

	if (GlobalChatLimiterTimer)
	{
		delete GlobalChatLimiterTimer;
		GlobalChatLimiterTimer = nullptr;
	}
}

void Client::CloseConnection() {

	ClientStream->RemoveData();

	ClientStream->Close();

	ClientStream->ReleaseFromUse();
}

void Clientlist::CheckForStaleConnectionsAll()
{
	LogDebug("Checking for stale connections");

	auto it = ClientChatConnections.begin();
	while (it != ClientChatConnections.end()) {
		(*it)->SendKeepAlive();
		++it;
	}
}

void Clientlist::CheckForStaleConnections(Client *c) {

	if (!c) {
		return;
	}

	std::list<Client*>::iterator Iterator;

	for (Iterator = ClientChatConnections.begin(); Iterator != ClientChatConnections.end(); ++Iterator) {

		if (((*Iterator) != c) && ((c->GetName() == (*Iterator)->GetName())
			&& (c->GetConnectionType() == (*Iterator)->GetConnectionType()))) {

			LogInfo("Removing old connection for [{}]", c->GetName().c_str());

			struct in_addr in;

			in.s_addr = (*Iterator)->ClientStream->GetRemoteIP();

			LogInfo("Client connection from [{}]:[{}] closed", inet_ntoa(in),
				ntohs((*Iterator)->ClientStream->GetRemotePort()));

			safe_delete((*Iterator));

			Iterator = ClientChatConnections.erase(Iterator);
		}
	}
}

std::string RemoveDuplicateChannels(const std::string& in_channels) {
	// Split the string by ", " and store the names in a vector
	std::vector<std::string> channel_names = Strings::Split(in_channels, ", ");

	// Remove duplicates by inserting the names of the channels into an unordered set
	// and then copying the unique elements back into the original vector
	std::unordered_set<std::string> unique_channels;
	channel_names.erase(
		std::remove_if(
			channel_names.begin(), channel_names.end(),
			[&unique_channels](const std::string &channel) {
				return !unique_channels.insert(channel).second;
			}
		), channel_names.end()
	);

	// Concatenate the names of the unique channels into a single string
	std::string unique_channels_string = Strings::Implode(", ", channel_names);

	return unique_channels_string;
}

void Clientlist::Process()
{
	auto it = ClientChatConnections.begin();
	while (it != ClientChatConnections.end()) {
		(*it)->AccountUpdate();
		if ((*it)->ClientStream->CheckState(CLOSED)) {
			struct in_addr in;
			in.s_addr = (*it)->ClientStream->GetRemoteIP();

			LogInfo("Client connection from [{}]:[{}] closed", inet_ntoa(in),
				ntohs((*it)->ClientStream->GetRemotePort()));

			safe_delete((*it));

			it = ClientChatConnections.erase(it);
			continue;
		}

		EQApplicationPacket *app = nullptr;

		bool KeyValid = true;

		while (KeyValid && !(*it)->GetForceDisconnect() && (app = (*it)->ClientStream->PopPacket())) {
			EmuOpcode opcode = app->GetOpcode();

			auto o = (*it)->ClientStream->GetOpcodeManager();
			LogPacketClientServer(
				"[{}] [{:#06x}] Size [{}] {}",
				OpcodeManager::EmuToName(app->GetOpcode()),
				o->EmuToEQ(app->GetOpcode()) == 0 ? app->GetProtocolOpcode() : o->EmuToEQ(app->GetOpcode()),
				app->Size(),
				(EQEmuLogSys::Instance()->IsLogEnabled(Logs::Detail, Logs::PacketClientServer) ? DumpPacketToString(app) : "")
			);

			switch (opcode) {
			case OP_MailLogin: {
				char *PacketBuffer = (char *)app->pBuffer + 1;
				char MailBox[64];
				char Key[64];
				char ConnectionTypeIndicator;

				VARSTRUCT_DECODE_STRING(MailBox, PacketBuffer);

				if (strlen(PacketBuffer) != 9) {
					LogInfo("Mail key is the wrong size. Version of world incompatible with UCS.");
					KeyValid = false;
					break;
				}
				ConnectionTypeIndicator = VARSTRUCT_DECODE_TYPE(char, PacketBuffer);

				(*it)->SetConnectionType(ConnectionTypeIndicator);

				VARSTRUCT_DECODE_STRING(Key, PacketBuffer);

				std::string MailBoxString = MailBox, CharacterName;

				// Strip off the SOE.EQ.<shortname>.
				//
				std::string::size_type LastPeriod = MailBoxString.find_last_of(".");

				if (LastPeriod == std::string::npos) {
					CharacterName = MailBoxString;
				}
				else {
					CharacterName = MailBoxString.substr(LastPeriod + 1);
				}

				LogInfo("Received login for user [{}] with key [{}]",
					MailBox, Key);

				if (!database.VerifyMailKey(CharacterName, (*it)->ClientStream->GetRemoteIP(), Key)) {
					LogError("Chat Key for [{}] does not match, closing connection.", MailBox);
					KeyValid = false;
					break;
				}

				(*it)->SetAccountID(database.FindAccount(CharacterName.c_str(), (*it)));

				database.GetAccountStatus((*it));

				if ((*it)->GetConnectionType() == ConnectionTypeCombined) {
					(*it)->SendFriends();
				}

				(*it)->SendMailBoxes();

				CheckForStaleConnections((*it));
				break;
			}
			case OP_Mail: {
				std::string command_string = (const char *)app->pBuffer + 1;
				bool command_directed = false;
				if (command_string.empty()) {
					break;
				}

				if (Strings::Contains(Strings::ToLower(command_string), "leave")) {
					command_directed = true;
				}

				ProcessOPMailCommand((*it), command_string, command_directed);
				break;
			}

			default: {
				LogInfo("Unhandled chat opcode {:#04x}", opcode);
				break;
			}
			}
			safe_delete(app);
		}
		if (!KeyValid || (*it)->GetForceDisconnect()) {
			struct in_addr in;
			in.s_addr = (*it)->ClientStream->GetRemoteIP();

			LogInfo("Force disconnecting client: [{}]:[{}], KeyValid=[{}], GetForceDisconnect()=[{}]",
					inet_ntoa(in), ntohs((*it)->ClientStream->GetRemotePort()), KeyValid,
					(*it)->GetForceDisconnect());

			(*it)->ClientStream->Close();

			safe_delete((*it));

			it = ClientChatConnections.erase(it);
			continue;
		}

		++it;
	}
}

void Clientlist::ProcessOPMailCommand(Client *c, std::string command_string, bool command_directed)
{

	if (command_string.length() == 0)
		return;

	if (isdigit(command_string[0]))
	{

		c->SendChannelMessageByNumber(command_string);

		return;
	}

	if (command_string[0] == '#') {

		c->SendChannelMessage(command_string);

		return;
	}

	std::string command, parameters;

	std::string::size_type Space = command_string.find_first_of(" ");

	if (Space != std::string::npos) {

		command = command_string.substr(0, Space);

		std::string::size_type parameters_start = command_string.find_first_not_of(" ", Space);

		if (parameters_start != std::string::npos)
			parameters = command_string.substr(parameters_start);
	}
	else {
		command = command_string;
	}

	auto command_code = LookupCommand(command.c_str());
	switch (command_code) {

	case CommandJoin:
		if (!command_directed) {
			//Append saved channels to params
			const auto saved_channels = database.CurrentPlayerChannels(c->GetName());
			if (!saved_channels.empty()) {
				parameters += fmt::format(", {}", Strings::Join(saved_channels, ", "));
			}
			parameters = RemoveDuplicateChannels(parameters);
		}
		c->JoinChannels(parameters, command_directed);
		break;

	case CommandLeaveAll:
		c->LeaveAllChannels(true, true);
		break;

	case CommandLeave:
		c->LeaveChannels(parameters, command_directed);
		break;

	case CommandListAll:

		ChannelList->SendAllChannels(c);
		break;

	case CommandList:
		c->ProcessChannelList(parameters);
		break;

	case CommandSet:
		c->LeaveAllChannels(false);
		c->JoinChannels(parameters, command_directed);
		break;

	case CommandAnnounce:
		c->ToggleAnnounce(parameters);
		break;

	case CommandSetOwner:
		c->SetChannelOwner(parameters);
		break;

	case CommandOPList:
		c->OPList(parameters);
		break;

	case CommandInvite:
		c->ChannelInvite(parameters);
		break;

	case CommandGrant:
		c->ChannelGrantModerator(parameters);
		break;

	case CommandModerate:
		c->ChannelModerate(parameters);
		break;

	case CommandVoice:
		c->ChannelGrantVoice(parameters);
		break;

	case CommandKick:
		c->ChannelKick(parameters);
		break;

	case CommandPassword:
		c->SetChannelPassword(parameters);
		break;

	case CommandToggleInvites:
		c->ToggleInvites();
		break;

	case CommandAFK:
		break;

	case CommandUptime:
		c->SendUptime();
		break;

	case CommandGetHeaders:
		database.SendHeaders(c);
		break;

	case CommandGetBody:
		database.SendBody(c, Strings::ToInt(parameters));
		break;

	case CommandMailTo:
		ProcessMailTo(c, parameters);
		break;

	case CommandSetMessageStatus:
		LogInfo("Set Message Status, Params: [{}]", parameters.c_str());
		ProcessSetMessageStatus(parameters);
		break;

	case CommandSelectMailBox:
	{
		std::string::size_type NumStart = parameters.find_first_of("0123456789");
		c->ChangeMailBox(Strings::ToInt(parameters.substr(NumStart)));
		break;
	}
	case CommandSetMailForwarding:
		break;

	case CommandBuddy:
		RemoveApostrophes(parameters);
		ProcessCommandBuddy(c, parameters);
		break;

	case CommandIgnorePlayer:
		RemoveApostrophes(parameters);
		ProcessCommandIgnore(c, parameters);
		break;

	default:
		c->SendHelp();
		LogInfo("Unhandled OP_Mail command: [{}]", command_string.c_str());
	}
}


void Clientlist::CloseAllConnections() {


	std::list<Client*>::iterator Iterator;

	for (Iterator = ClientChatConnections.begin(); Iterator != ClientChatConnections.end(); ++Iterator) {

		LogInfo("Removing client [{}]", (*Iterator)->GetName().c_str());

		(*Iterator)->CloseConnection();
	}
}

void Client::AddCharacter(int CharID, const char *CharacterName, int Level) {

	if (!CharacterName) return;

	LogDebug("Adding character [{}] with ID [{}] for [{}]", CharacterName, CharID, GetName().c_str());

	CharacterEntry NewCharacter;
	NewCharacter.CharID = CharID;
	NewCharacter.Name = CharacterName;
	NewCharacter.Level = Level;

	Characters.push_back(NewCharacter);
}

void Client::SendKeepAlive() {
	EQApplicationPacket outapp(OP_SessionReady, 0);
	QueuePacket(&outapp);
}

void Client::SendMailBoxes() {

	int Count = Characters.size();

	int PacketLength = 10;

	std::string s;

	for (int i = 0; i < Count; i++) {

		s += GetMailPrefix() + Characters[i].Name;

		if (i != (Count - 1))
			s = s + ",";
	}

	PacketLength += s.length() + 1;

	auto outapp = new EQApplicationPacket(OP_MailLogin, PacketLength);

	char *PacketBuffer = (char *)outapp->pBuffer;

	VARSTRUCT_ENCODE_TYPE(uint8, PacketBuffer, 1);
	VARSTRUCT_ENCODE_TYPE(uint32, PacketBuffer, Count);
	VARSTRUCT_ENCODE_TYPE(uint32, PacketBuffer, 0);

	VARSTRUCT_ENCODE_STRING(PacketBuffer, s.c_str());
	VARSTRUCT_ENCODE_TYPE(uint8, PacketBuffer, 0);


	QueuePacket(outapp);

	safe_delete(outapp);
}

Client *Clientlist::FindCharacter(const std::string& CharacterName) {

	std::list<Client*>::iterator Iterator;

	for (Iterator = ClientChatConnections.begin(); Iterator != ClientChatConnections.end(); ++Iterator) {

		if ((*Iterator)->GetName() == CharacterName)
			return ((*Iterator));

	}

	return nullptr;
}

void Client::AddToChannelList(ChatChannel *JoinedChannel) {

	if (!JoinedChannel) return;

	for (int i = 0; i < MAX_JOINED_CHANNELS; i++)
		if (JoinedChannels[i] == nullptr) {
			JoinedChannels[i] = JoinedChannel;
			LogDebug("Added Channel [{}] to slot [{}] for [{}]", JoinedChannel->GetName().c_str(), i + 1, GetName().c_str());
			return;
		}
}

void Client::RemoveFromChannelList(ChatChannel *JoinedChannel) {

	for (int i = 0; i < MAX_JOINED_CHANNELS; i++)
		if (JoinedChannels[i] == JoinedChannel) {

			// Shuffle all the channels down. Client likes them all nice and consecutive.
			//
			for (int j = i; j < (MAX_JOINED_CHANNELS - 1); j++)
				JoinedChannels[j] = JoinedChannels[j + 1];

			JoinedChannels[MAX_JOINED_CHANNELS - 1] = nullptr;

			break;
		}
}

int Client::ChannelCount() {

	int NumberOfChannels = 0;

	for (auto &elem : JoinedChannels)
		if (elem)
			NumberOfChannels++;

	return NumberOfChannels;

}

void Client::JoinChannels(std::string& channel_name_list, bool command_directed) {

	for (auto &elem : channel_name_list) {
		if (elem == '%') {
			elem = '/';
		}
	}

	LogInfo("Client: [{}] joining channels [{}]", GetName().c_str(), channel_name_list.c_str());

	auto number_of_channels = ChannelCount();

	auto current_pos = channel_name_list.find_first_not_of(" ");

	while (current_pos != std::string::npos) {

		if (number_of_channels == MAX_JOINED_CHANNELS) {

			GeneralChannelMessage("You have joined the maximum number of channels. /leave one before trying to join another.");

			break;
		}

		auto comma = channel_name_list.find_first_of(", ", current_pos);

		if (comma == std::string::npos) {
			auto* joined_channel = ChannelList->AddClientToChannel(channel_name_list.substr(current_pos), this, command_directed);

			if (joined_channel) {
				AddToChannelList(joined_channel);
			}

			break;
		}

		auto* joined_channel = ChannelList->AddClientToChannel(channel_name_list.substr(current_pos, comma - current_pos), this, command_directed);

		if (joined_channel) {

			AddToChannelList(joined_channel);

			number_of_channels++;
		}

		current_pos = channel_name_list.find_first_not_of(", ", comma);
	}

	std::string JoinedChannelsList, ChannelMessage;

	ChannelMessage = "Channels: ";

	char tmp[200];

	int ChannelCount = 0;

	for (int i = 0; i < MAX_JOINED_CHANNELS; i++) {

		if (JoinedChannels[i] != nullptr) {

			if (ChannelCount) {

				JoinedChannelsList = JoinedChannelsList + ",";

				ChannelMessage = ChannelMessage + ",";

			}

			JoinedChannelsList = JoinedChannelsList + JoinedChannels[i]->GetName();

			sprintf(tmp, "%i=%s(%i)", i + 1, JoinedChannels[i]->GetName().c_str(), JoinedChannels[i]->MemberCount(Status));

			ChannelMessage += tmp;

			ChannelCount++;
		}
	}

	auto outapp = new EQApplicationPacket(OP_Mail, JoinedChannelsList.length() + 1);

	char *PacketBuffer = (char *)outapp->pBuffer;

	sprintf(PacketBuffer, "%s", JoinedChannelsList.c_str());


	QueuePacket(outapp);

	safe_delete(outapp);

	if (ChannelCount == 0)
		ChannelMessage = "You are not on any channels.";

	outapp = new EQApplicationPacket(OP_ChannelMessage, ChannelMessage.length() + 3);

	PacketBuffer = (char *)outapp->pBuffer;

	VARSTRUCT_ENCODE_TYPE(uint8, PacketBuffer, 0x00);
	VARSTRUCT_ENCODE_TYPE(uint8, PacketBuffer, 0x00);
	VARSTRUCT_ENCODE_STRING(PacketBuffer, ChannelMessage.c_str());


	QueuePacket(outapp);

	safe_delete(outapp);
}

void Client::LeaveChannels(std::string& channel_name_list, bool command_directed) {
	LogInfo("Client: [{}] leaving channels [{}]", GetName().c_str(), channel_name_list.c_str());

	auto current_pos = 0;

	while (current_pos != std::string::npos) {

		std::string::size_type Comma = channel_name_list.find_first_of(", ", current_pos);

		if (Comma == std::string::npos) {

			auto* joined_channel = ChannelList->RemoveClientFromChannel(channel_name_list.substr(current_pos), this, command_directed);

			if (joined_channel)
				RemoveFromChannelList(joined_channel);

			break;
		}

		auto* joined_channel = ChannelList->RemoveClientFromChannel(channel_name_list.substr(current_pos, Comma - current_pos), this, command_directed);

		if (joined_channel)
			RemoveFromChannelList(joined_channel);

		current_pos = channel_name_list.find_first_not_of(", ", Comma);
	}

	std::string joined_channels_list, channel_message;

	channel_message = "Channels: ";

	char tmp[200];

	int ChannelCount = 0;

	for (int i = 0; i < MAX_JOINED_CHANNELS; i++) {

		if (JoinedChannels[i] != nullptr) {

			if (ChannelCount) {

				joined_channels_list = joined_channels_list + ",";

				channel_message = channel_message + ",";
			}

			joined_channels_list = joined_channels_list + JoinedChannels[i]->GetName();

			sprintf(tmp, "%i=%s(%i)", i + 1, JoinedChannels[i]->GetName().c_str(), JoinedChannels[i]->MemberCount(Status));

			channel_message += tmp;

			ChannelCount++;
		}
	}

	auto outapp = new EQApplicationPacket(OP_Mail, joined_channels_list.length() + 1);

	char *PacketBuffer = (char *)outapp->pBuffer;

	sprintf(PacketBuffer, "%s", joined_channels_list.c_str());


	QueuePacket(outapp);

	safe_delete(outapp);

	if (ChannelCount == 0)
		channel_message = "You are not on any channels.";

	outapp = new EQApplicationPacket(OP_ChannelMessage, channel_message.length() + 3);

	PacketBuffer = (char *)outapp->pBuffer;

	VARSTRUCT_ENCODE_TYPE(uint8, PacketBuffer, 0x00);
	VARSTRUCT_ENCODE_TYPE(uint8, PacketBuffer, 0x00);
	VARSTRUCT_ENCODE_STRING(PacketBuffer, channel_message.c_str());


	QueuePacket(outapp);

	safe_delete(outapp);
}

void Client::LeaveAllChannels(bool send_updated_channel_list, bool command_directed) {

	for (auto &elem : JoinedChannels) {

		if (elem) {

			ChannelList->RemoveClientFromChannel(elem->GetName(), this, command_directed);

			elem = nullptr;
		}
	}

	if (send_updated_channel_list)
		SendChannelList();
}


void Client::ProcessChannelList(const std::string& Input) {

	if (Input.length() == 0) {

		SendChannelList();

		return;
	}

	std::string ChannelName = Input;

	if (isdigit(ChannelName[0]))
		ChannelName = ChannelSlotName(Strings::ToInt(ChannelName));

	ChatChannel *RequiredChannel = ChannelList->FindChannel(ChannelName);

	if (RequiredChannel)
		RequiredChannel->SendChannelMembers(this);
	else
		GeneralChannelMessage("Channel " + Input + " not found.");
}



void Client::SendChannelList() {

	std::string ChannelMessage;

	ChannelMessage = "Channels: ";

	char tmp[200];

	int ChannelCount = 0;

	for (int i = 0; i < MAX_JOINED_CHANNELS; i++) {

		if (JoinedChannels[i] != nullptr) {

			if (ChannelCount)
				ChannelMessage = ChannelMessage + ",";

			sprintf(tmp, "%i=%s(%i)", i + 1, JoinedChannels[i]->GetName().c_str(), JoinedChannels[i]->MemberCount(Status));

			ChannelMessage += tmp;

			ChannelCount++;
		}
	}

	if (ChannelCount == 0)
		ChannelMessage = "You are not on any channels.";

	auto outapp = new EQApplicationPacket(OP_ChannelMessage, ChannelMessage.length() + 3);

	char *PacketBuffer = (char *)outapp->pBuffer;

	VARSTRUCT_ENCODE_TYPE(uint8, PacketBuffer, 0x00);
	VARSTRUCT_ENCODE_TYPE(uint8, PacketBuffer, 0x00);
	VARSTRUCT_ENCODE_STRING(PacketBuffer, ChannelMessage.c_str());


	QueuePacket(outapp);

	safe_delete(outapp);
}

void Client::SendChannelMessage(std::string Message)
{

	std::string::size_type MessageStart = Message.find_first_of(" ");

	if (MessageStart == std::string::npos)
		return;

	std::string ChannelName = Message.substr(1, MessageStart - 1);

	LogInfo("[{}] tells [{}], [[{}]]", GetName().c_str(), ChannelName.c_str(), Message.substr(MessageStart + 1).c_str());

	ChatChannel *RequiredChannel = ChannelList->FindChannel(ChannelName);

	if (IsRevoked())
	{
		GeneralChannelMessage("You are Revoked, you cannot chat in global channels.");
		return;
	}

	if (ChannelName.compare("Newplayers") != 0)
	{
		if (GetKarma() < RuleI(Chat, KarmaGlobalChatLimit))
		{
			CharacterEntry *char_ent = nullptr;
			for (auto &elem : Characters) {
				if (elem.Name.compare(GetName()) == 0) {
					char_ent = &elem;
					break;
				}
			}
			if (char_ent)
			{
				if (char_ent->Level < RuleI(Chat, GlobalChatLevelLimit))
				{
					GeneralChannelMessage("You are either not high enough level or high enough karma to talk in this channel right now.");
					return;
				}
			}
		}
	}

	if (RequiredChannel) {
		if (RuleB(Chat, EnableAntiSpam))
		{
			if (!RequiredChannel->IsModerated() || RequiredChannel->HasVoice(GetName()) || RequiredChannel->IsOwner(GetName()) ||
				RequiredChannel->IsModerator(GetName()) || IsChannelAdmin())
			{
				if (GlobalChatLimiterTimer)
				{
					if (GlobalChatLimiterTimer->Check(false))
					{
						GlobalChatLimiterTimer->Start(RuleI(Chat, IntervalDurationMS));
						AttemptedMessages = 0;
					}
				}
				int AllowedMessages = RuleI(Chat, MinimumMessagesPerInterval) + GetKarma();
				AllowedMessages = AllowedMessages > RuleI(Chat, MaximumMessagesPerInterval) ? RuleI(Chat, MaximumMessagesPerInterval) : AllowedMessages;

				if (RuleI(Chat, MinStatusToBypassAntiSpam) <= Status)
					AllowedMessages = 10000;

				AttemptedMessages++;
				if (AttemptedMessages > AllowedMessages)
				{
					if (AttemptedMessages > RuleI(Chat, MaxMessagesBeforeKick))
					{
						ForceDisconnect = true;
					}
					if (GlobalChatLimiterTimer)
					{
						char TimeLeft[256];
						sprintf(TimeLeft, "You are currently rate limited, you cannot send more messages for %i seconds.",
							(GlobalChatLimiterTimer->GetRemainingTime() / 1000));
						GeneralChannelMessage(TimeLeft);
					}
					else
					{
						GeneralChannelMessage("You are currently rate limited, you cannot send more messages for up to 60 seconds.");
					}
				}
				else
				{
					RequiredChannel->SendMessageToChannel(Message.substr(MessageStart + 1), this);
				}
			}
			else
				GeneralChannelMessage("Channel " + ChannelName + " is moderated and you have not been granted a voice.");
		}
		else
		{
			if (!RequiredChannel->IsModerated() || RequiredChannel->HasVoice(GetName()) || RequiredChannel->IsOwner(GetName()) ||
				RequiredChannel->IsModerator(GetName()) || IsChannelAdmin())
				RequiredChannel->SendMessageToChannel(Message.substr(MessageStart + 1), this);
			else
				GeneralChannelMessage("Channel " + ChannelName + " is moderated and you have not been granted a voice.");
		}
	}
}

void Client::SendChannelMessageByNumber(std::string Message) {

	std::string::size_type MessageStart = Message.find_first_of(" ");

	if (MessageStart == std::string::npos)
		return;

	int ChannelNumber = Strings::ToInt(Message.substr(0, MessageStart));

	if ((ChannelNumber < 1) || (ChannelNumber > MAX_JOINED_CHANNELS)) {

		GeneralChannelMessage("Invalid channel name/number specified.");

		return;
	}

	ChatChannel *RequiredChannel = JoinedChannels[ChannelNumber - 1];

	if (!RequiredChannel) {

		GeneralChannelMessage("Invalid channel name/number specified.");

		return;
	}

	if (IsRevoked())
	{
		GeneralChannelMessage("You are Revoked, you cannot chat in global channels.");
		return;
	}

	if (RequiredChannel->GetName().compare("Newplayers") != 0)
	{
		if (GetKarma() < RuleI(Chat, KarmaGlobalChatLimit))
		{
			CharacterEntry *char_ent = nullptr;
			for (auto &elem : Characters) {
				if (elem.Name.compare(GetName()) == 0) {
					char_ent = &elem;
					break;
				}
			}
			if (char_ent)
			{
				if (char_ent->Level < RuleI(Chat, GlobalChatLevelLimit))
				{
					GeneralChannelMessage("You are either not high enough level or high enough karma to talk in this channel right now.");
					return;
				}
			}
		}
	}

	LogInfo("[{}] tells [{}], [[{}]]", GetName().c_str(), RequiredChannel->GetName().c_str(),
		Message.substr(MessageStart + 1).c_str());

	if (RuleB(Chat, EnableAntiSpam))
	{
		if (!RequiredChannel->IsModerated() || RequiredChannel->HasVoice(GetName()) || RequiredChannel->IsOwner(GetName()) ||
			RequiredChannel->IsModerator(GetName()))
		{
			if (GlobalChatLimiterTimer)
			{
				if (GlobalChatLimiterTimer->Check(false))
				{
					GlobalChatLimiterTimer->Start(RuleI(Chat, IntervalDurationMS));
					AttemptedMessages = 0;
				}
			}
			int AllowedMessages = RuleI(Chat, MinimumMessagesPerInterval) + GetKarma();
			AllowedMessages = AllowedMessages > RuleI(Chat, MaximumMessagesPerInterval) ? RuleI(Chat, MaximumMessagesPerInterval) : AllowedMessages;
			if (RuleI(Chat, MinStatusToBypassAntiSpam) <= Status)
				AllowedMessages = 10000;

			AttemptedMessages++;
			if (AttemptedMessages > AllowedMessages)
			{
				if (AttemptedMessages > RuleI(Chat, MaxMessagesBeforeKick))
				{
					ForceDisconnect = true;
				}
				if (GlobalChatLimiterTimer)
				{
					char TimeLeft[256];
					sprintf(TimeLeft, "You are currently rate limited, you cannot send more messages for %i seconds.",
						(GlobalChatLimiterTimer->GetRemainingTime() / 1000));
					GeneralChannelMessage(TimeLeft);
				}
				else
				{
					GeneralChannelMessage("You are currently rate limited, you cannot send more messages for up to 60 seconds.");
				}
			}
			else
			{
				RequiredChannel->SendMessageToChannel(Message.substr(MessageStart + 1), this);
			}
		}
		else
			GeneralChannelMessage("Channel " + RequiredChannel->GetName() + " is moderated and you have not been granted a voice.");
	}
	else
	{
		if (!RequiredChannel->IsModerated() || RequiredChannel->HasVoice(GetName()) || RequiredChannel->IsOwner(GetName()) ||
			RequiredChannel->IsModerator(GetName()))
			RequiredChannel->SendMessageToChannel(Message.substr(MessageStart + 1), this);
		else
			GeneralChannelMessage("Channel " + RequiredChannel->GetName() + " is moderated and you have not been granted a voice.");
	}

}

void Client::SendChannelMessage(const std::string& ChannelName, const std::string& Message, Client *Sender) {

	if (!Sender) return;

	std::string FQSenderName = WorldShortName + "." + Sender->GetName();

	int PacketLength = ChannelName.length() + Message.length() + FQSenderName.length() + 3;

	if (UnderfootOrLater)
		PacketLength += 8;

	auto outapp = new EQApplicationPacket(OP_ChannelMessage, PacketLength);

	char *PacketBuffer = (char *)outapp->pBuffer;

	VARSTRUCT_ENCODE_STRING(PacketBuffer, ChannelName.c_str());
	VARSTRUCT_ENCODE_STRING(PacketBuffer, FQSenderName.c_str());
	VARSTRUCT_ENCODE_STRING(PacketBuffer, Message.c_str());

	if (UnderfootOrLater)
		VARSTRUCT_ENCODE_STRING(PacketBuffer, "SPAM:0:");

	QueuePacket(outapp);

	safe_delete(outapp);
}

void Client::ToggleAnnounce(const std::string& State)
{
	if (State == "")
		Announce = !Announce;
	else if (State == "on")
		Announce = true;
	else
		Announce = false;

	std::string Message = "Announcing now ";

	if (Announce)
		Message += "on";
	else
		Message += "off";

	GeneralChannelMessage(Message);
}

void Client::AnnounceJoin(ChatChannel *Channel, Client *c) {

	if (!Channel || !c) return;

	int PacketLength = Channel->GetName().length() + c->GetName().length() + 2;

	auto outapp = new EQApplicationPacket(OP_ChannelAnnounceJoin, PacketLength);

	char *PacketBuffer = (char *)outapp->pBuffer;

	VARSTRUCT_ENCODE_STRING(PacketBuffer, Channel->GetName().c_str());
	VARSTRUCT_ENCODE_STRING(PacketBuffer, c->GetName().c_str());


	QueuePacket(outapp);

	safe_delete(outapp);
}

void Client::AnnounceLeave(ChatChannel *Channel, Client *c) {

	if (!Channel || !c) return;

	int PacketLength = Channel->GetName().length() + c->GetName().length() + 2;

	auto outapp = new EQApplicationPacket(OP_ChannelAnnounceLeave, PacketLength);

	char *PacketBuffer = (char *)outapp->pBuffer;

	VARSTRUCT_ENCODE_STRING(PacketBuffer, Channel->GetName().c_str());
	VARSTRUCT_ENCODE_STRING(PacketBuffer, c->GetName().c_str());


	QueuePacket(outapp);

	safe_delete(outapp);
}

void Client::GeneralChannelMessage(const char *Characters) {

	if (!Characters) return;

	std::string Message = Characters;

	GeneralChannelMessage(Message);

}

void Client::GeneralChannelMessage(const std::string& Message) {

	auto outapp = new EQApplicationPacket(OP_ChannelMessage, Message.length() + 3);
	char *PacketBuffer = (char *)outapp->pBuffer;
	VARSTRUCT_ENCODE_TYPE(uint8, PacketBuffer, 0x00);
	VARSTRUCT_ENCODE_TYPE(uint8, PacketBuffer, 0x00);
	VARSTRUCT_ENCODE_STRING(PacketBuffer, Message.c_str());

	QueuePacket(outapp);

	safe_delete(outapp);
}

void Client::SetChannelPassword(std::string ChannelPassword) {

	std::string::size_type PasswordStart = ChannelPassword.find_first_not_of(" ");

	if (PasswordStart == std::string::npos) {
		std::string Message = "Incorrect syntax: /chat password <new password> <channel name>";
		GeneralChannelMessage(Message);
		return;
	}

	std::string::size_type Space = ChannelPassword.find_first_of(" ", PasswordStart);

	if (Space == std::string::npos) {
		std::string Message = "Incorrect syntax: /chat password <new password> <channel name>";
		GeneralChannelMessage(Message);
		return;
	}

	std::string Password = ChannelPassword.substr(PasswordStart, Space - PasswordStart);

	std::string::size_type ChannelStart = ChannelPassword.find_first_not_of(" ", Space);

	if (ChannelStart == std::string::npos) {
		std::string Message = "Incorrect syntax: /chat password <new password> <channel name>";
		GeneralChannelMessage(Message);
		return;
	}

	std::string ChannelName = ChannelPassword.substr(ChannelStart);

	if ((ChannelName.length() > 0) && isdigit(ChannelName[0]))
		ChannelName = ChannelSlotName(Strings::ToInt(ChannelName));

	std::string Message;

	if (!strcasecmp(Password.c_str(), "remove")) {
		Password.clear();
		Message = "Password REMOVED on channel " + ChannelName;
	}
	else
		Message = "Password change on channel " + ChannelName;

	LogInfo("Set password of channel [[{}]] to [[{}]] by [{}]", ChannelName.c_str(), Password.c_str(), GetName().c_str());

	ChatChannel *RequiredChannel = ChannelList->FindChannel(ChannelName);

	if (!RequiredChannel) {
		std::string Message = "Channel not found.";
		GeneralChannelMessage(Message);
		return;
	}

	if (!RequiredChannel->IsOwner(GetName()) && !RequiredChannel->IsModerator(GetName()) && !IsChannelAdmin()) {
		std::string Message = "You do not own or have moderator rights on channel " + ChannelName;
		GeneralChannelMessage(Message);
		return;
	}

	RequiredChannel->SetPassword(Password);
	database.SaveChatChannel(RequiredChannel->GetName(), RequiredChannel->GetOwnerName(), Password, RequiredChannel->GetMinStatus()); // Update DB with new password

	GeneralChannelMessage(Message);

}

void Client::SetChannelOwner(std::string CommandString) {

	std::string::size_type PlayerStart = CommandString.find_first_not_of(" ");

	if (PlayerStart == std::string::npos) {
		std::string Message = "Incorrect syntax: /chat setowner <player> <channel>";
		GeneralChannelMessage(Message);
		return;
	}

	std::string::size_type Space = CommandString.find_first_of(" ", PlayerStart);

	if (Space == std::string::npos) {
		std::string Message = "Incorrect syntax: /chat setowner <player> <channel>";
		GeneralChannelMessage(Message);
		return;
	}

	std::string NewOwner = CapitaliseName(CommandString.substr(PlayerStart, Space - PlayerStart));

	std::string::size_type ChannelStart = CommandString.find_first_not_of(" ", Space);

	if (ChannelStart == std::string::npos) {
		std::string Message = "Incorrect syntax: /chat setowner <player> <channel>";
		GeneralChannelMessage(Message);
		return;
	}

	std::string ChannelName = CapitaliseName(CommandString.substr(ChannelStart));

	if ((ChannelName.length() > 0) && isdigit(ChannelName[0]))
		ChannelName = ChannelSlotName(Strings::ToInt(ChannelName));

	LogInfo("Set owner of channel [[{}]] to [[{}]]", ChannelName.c_str(), NewOwner.c_str());

	ChatChannel *RequiredChannel = ChannelList->FindChannel(ChannelName);

	if (!RequiredChannel) {
		GeneralChannelMessage("Channel " + ChannelName + " not found.");
		return;
	}

	if (!RequiredChannel->IsOwner(GetName()) && !IsChannelAdmin()) {
		std::string Message = "You do not own channel " + ChannelName;
		GeneralChannelMessage(Message);
		return;
	}

	if (database.FindCharacter(NewOwner.c_str()) < 0) {

		GeneralChannelMessage("Player " + NewOwner + " does not exist.");
		return;
	}

	RequiredChannel->SetOwner(NewOwner);
	database.SaveChatChannel(RequiredChannel->GetName(), NewOwner, RequiredChannel->GetPassword(), RequiredChannel->GetMinStatus()); // Update DB with new owner

	if (RequiredChannel->IsModerator(NewOwner))
		RequiredChannel->RemoveModerator(NewOwner);

	GeneralChannelMessage("Channel owner changed.");

}

void Client::OPList(std::string CommandString) {

	std::string::size_type ChannelStart = CommandString.find_first_not_of(" ");

	if (ChannelStart == std::string::npos) {
		std::string Message = "Incorrect syntax: /chat oplist <channel>";
		GeneralChannelMessage(Message);
		return;
	}

	std::string ChannelName = CapitaliseName(CommandString.substr(ChannelStart));

	if ((ChannelName.length() > 0) && isdigit(ChannelName[0]))
		ChannelName = ChannelSlotName(Strings::ToInt(ChannelName));

	ChatChannel *RequiredChannel = ChannelList->FindChannel(ChannelName);

	if (!RequiredChannel) {
		GeneralChannelMessage("Channel " + ChannelName + " not found.");
		return;
	}

	RequiredChannel->SendOPList(this);
}

void Client::ChannelInvite(std::string CommandString) {

	std::string::size_type PlayerStart = CommandString.find_first_not_of(" ");

	if (PlayerStart == std::string::npos) {
		std::string Message = "Incorrect syntax: /chat invite <player> <channel>";
		GeneralChannelMessage(Message);
		return;
	}

	std::string::size_type Space = CommandString.find_first_of(" ", PlayerStart);

	if (Space == std::string::npos) {
		std::string Message = "Incorrect syntax: /chat invite <player> <channel>";
		GeneralChannelMessage(Message);
		return;
	}

	std::string Invitee = CapitaliseName(CommandString.substr(PlayerStart, Space - PlayerStart));

	std::string::size_type ChannelStart = CommandString.find_first_not_of(" ", Space);

	if (ChannelStart == std::string::npos) {
		std::string Message = "Incorrect syntax: /chat invite <player> <channel>";
		GeneralChannelMessage(Message);
		return;
	}

	std::string ChannelName = CapitaliseName(CommandString.substr(ChannelStart));

	if ((ChannelName.length() > 0) && isdigit(ChannelName[0]))
		ChannelName = ChannelSlotName(Strings::ToInt(ChannelName));

	LogInfo("[[{}]] invites [[{}]] to channel [[{}]]", GetName().c_str(), Invitee.c_str(), ChannelName.c_str());

	Client *RequiredClient = g_Clientlist->FindCharacter(Invitee);

	if (!RequiredClient) {

		GeneralChannelMessage(Invitee + " is not online.");
		return;
	}

	if (RequiredClient == this) {

		GeneralChannelMessage("You cannot invite yourself to a channel.");
		return;
	}

	if (!RequiredClient->InvitesAllowed()) {

		GeneralChannelMessage("That player is not currently accepting channel invitations.");
		return;
	}

	ChatChannel *RequiredChannel = ChannelList->FindChannel(ChannelName);

	if (!RequiredChannel) {

		GeneralChannelMessage("Channel " + ChannelName + " not found.");
		return;
	}

	if (!RequiredChannel->IsOwner(GetName()) && !RequiredChannel->IsModerator(GetName())) {

		std::string Message = "You do not own or have moderator rights to channel " + ChannelName;

		GeneralChannelMessage(Message);
		return;
	}

	if (RequiredChannel->IsClientInChannel(RequiredClient)) {

		GeneralChannelMessage(Invitee + " is already in that channel");

		return;
	}

	RequiredChannel->AddInvitee(Invitee);

	RequiredClient->GeneralChannelMessage(GetName() + " has invited you to join channel " + ChannelName);

	GeneralChannelMessage("Invitation sent to " + Invitee + " to join channel " + ChannelName);

}

void Client::ChannelModerate(std::string CommandString) {

	std::string::size_type ChannelStart = CommandString.find_first_not_of(" ");

	if (ChannelStart == std::string::npos) {

		std::string Message = "Incorrect syntax: /chat moderate <channel>";

		GeneralChannelMessage(Message);
		return;
	}

	std::string ChannelName = CapitaliseName(CommandString.substr(ChannelStart));

	if ((ChannelName.length() > 0) && isdigit(ChannelName[0]))
		ChannelName = ChannelSlotName(Strings::ToInt(ChannelName));

	ChatChannel *RequiredChannel = ChannelList->FindChannel(ChannelName);

	if (!RequiredChannel) {

		GeneralChannelMessage("Channel " + ChannelName + " not found.");
		return;
	}

	if (!RequiredChannel->IsOwner(GetName()) && !RequiredChannel->IsModerator(GetName()) && !IsChannelAdmin()) {

		GeneralChannelMessage("You do not own or have moderator rights to channel " + ChannelName);
		return;
	}

	RequiredChannel->SetModerated(!RequiredChannel->IsModerated());

	if (!RequiredChannel->IsClientInChannel(this)) {
		if (RequiredChannel->IsModerated())
			GeneralChannelMessage("Channel " + ChannelName + " is now moderated.");
		else
			GeneralChannelMessage("Channel " + ChannelName + " is no longer moderated.");
	}

}

void Client::ChannelGrantModerator(std::string CommandString) {

	std::string::size_type PlayerStart = CommandString.find_first_not_of(" ");

	if (PlayerStart == std::string::npos) {

		GeneralChannelMessage("Incorrect syntax: /chat grant <player> <channel>");
		return;
	}

	std::string::size_type Space = CommandString.find_first_of(" ", PlayerStart);

	if (Space == std::string::npos) {

		GeneralChannelMessage("Incorrect syntax: /chat grant <player> <channel>");
		return;
	}

	std::string Moderator = CapitaliseName(CommandString.substr(PlayerStart, Space - PlayerStart));

	std::string::size_type ChannelStart = CommandString.find_first_not_of(" ", Space);

	if (ChannelStart == std::string::npos) {

		GeneralChannelMessage("Incorrect syntax: /chat grant <player> <channel>");
		return;
	}

	std::string ChannelName = CapitaliseName(CommandString.substr(ChannelStart));

	if ((ChannelName.length() > 0) && isdigit(ChannelName[0]))
		ChannelName = ChannelSlotName(Strings::ToInt(ChannelName));

	LogInfo("[[{}]] gives [[{}]] moderator rights to channel [[{}]]", GetName().c_str(), Moderator.c_str(), ChannelName.c_str());

	Client *RequiredClient = g_Clientlist->FindCharacter(Moderator);

	if (!RequiredClient && (database.FindCharacter(Moderator.c_str()) < 0)) {

		GeneralChannelMessage("Player " + Moderator + " does not exist.");
		return;
	}

	if (RequiredClient == this) {

		GeneralChannelMessage("You cannot grant yourself moderator rights to a channel.");
		return;
	}

	ChatChannel *RequiredChannel = ChannelList->FindChannel(ChannelName);

	if (!RequiredChannel) {

		GeneralChannelMessage("Channel " + ChannelName + " not found.");
		return;
	}

	if (!RequiredChannel->IsOwner(GetName()) && !IsChannelAdmin()) {

		GeneralChannelMessage("You do not own channel " + ChannelName);
		return;
	}

	if (RequiredChannel->IsModerator(Moderator)) {

		RequiredChannel->RemoveModerator(Moderator);

		if (RequiredClient)
			RequiredClient->GeneralChannelMessage(GetName() + " has removed your moderator rights to channel " + ChannelName);

		GeneralChannelMessage("Removing moderator rights from " + Moderator + " to channel " + ChannelName);
	}
	else {
		RequiredChannel->AddModerator(Moderator);

		if (RequiredClient)
			RequiredClient->GeneralChannelMessage(GetName() + " has made you a moderator of channel " + ChannelName);

		GeneralChannelMessage(Moderator + " is now a moderator on channel " + ChannelName);
	}

}

void Client::ChannelGrantVoice(std::string CommandString) {

	std::string::size_type PlayerStart = CommandString.find_first_not_of(" ");

	if (PlayerStart == std::string::npos) {

		GeneralChannelMessage("Incorrect syntax: /chat voice <player> <channel>");
		return;
	}

	std::string::size_type Space = CommandString.find_first_of(" ", PlayerStart);

	if (Space == std::string::npos) {
		GeneralChannelMessage("Incorrect syntax: /chat voice <player> <channel>");
		return;
	}

	std::string Voicee = CapitaliseName(CommandString.substr(PlayerStart, Space - PlayerStart));

	std::string::size_type ChannelStart = CommandString.find_first_not_of(" ", Space);

	if (ChannelStart == std::string::npos) {
		GeneralChannelMessage("Incorrect syntax: /chat voice <player> <channel>");
		return;
	}

	std::string ChannelName = CapitaliseName(CommandString.substr(ChannelStart));

	if ((ChannelName.length() > 0) && isdigit(ChannelName[0]))
		ChannelName = ChannelSlotName(Strings::ToInt(ChannelName));

	LogInfo("[[{}]] gives [[{}]] voice to channel [[{}]]", GetName().c_str(), Voicee.c_str(), ChannelName.c_str());

	Client *RequiredClient = g_Clientlist->FindCharacter(Voicee);

	if (!RequiredClient && (database.FindCharacter(Voicee.c_str()) < 0)) {

		GeneralChannelMessage("Player " + Voicee + " does not exist.");
		return;
	}

	if (RequiredClient == this) {

		GeneralChannelMessage("You cannot grant yourself voice to a channel.");
		return;
	}

	ChatChannel *RequiredChannel = ChannelList->FindChannel(ChannelName);

	if (!RequiredChannel) {

		GeneralChannelMessage("Channel " + ChannelName + " not found.");
		return;
	}

	if (!RequiredChannel->IsOwner(GetName()) && !RequiredChannel->IsModerator(GetName()) && !IsChannelAdmin()) {

		GeneralChannelMessage("You do not own or have moderator rights to channel " + ChannelName);
		return;
	}

	if (RequiredClient && (RequiredChannel->IsOwner(RequiredClient->GetName()) || RequiredChannel->IsModerator(RequiredClient->GetName()))) {

		GeneralChannelMessage("The channel owner and moderators automatically have voice.");
		return;
	}

	if (RequiredChannel->HasVoice(Voicee)) {

		RequiredChannel->RemoveVoice(Voicee);

		if (RequiredClient)
			RequiredClient->GeneralChannelMessage(GetName() + " has removed your voice rights to channel " + ChannelName);

		GeneralChannelMessage("Removing voice from " + Voicee + " in channel " + ChannelName);
	}
	else {
		RequiredChannel->AddVoice(Voicee);

		if (RequiredClient)
			RequiredClient->GeneralChannelMessage(GetName() + " has given you voice in channel " + ChannelName);

		GeneralChannelMessage(Voicee + " now has voice in channel " + ChannelName);
	}

}

void Client::ChannelKick(std::string CommandString) {

	std::string::size_type PlayerStart = CommandString.find_first_not_of(" ");

	if (PlayerStart == std::string::npos) {

		GeneralChannelMessage("Incorrect syntax: /chat kick <player> <channel>");
		return;
	}

	std::string::size_type Space = CommandString.find_first_of(" ", PlayerStart);

	if (Space == std::string::npos) {

		GeneralChannelMessage("Incorrect syntax: /chat kick <player> <channel>");
		return;
	}

	std::string Kickee = CapitaliseName(CommandString.substr(PlayerStart, Space - PlayerStart));

	std::string::size_type ChannelStart = CommandString.find_first_not_of(" ", Space);

	if (ChannelStart == std::string::npos) {

		GeneralChannelMessage("Incorrect syntax: /chat kick <player> <channel>");
		return;
	}
	std::string ChannelName = CapitaliseName(CommandString.substr(ChannelStart));

	if ((ChannelName.length() > 0) && isdigit(ChannelName[0]))
		ChannelName = ChannelSlotName(Strings::ToInt(ChannelName));

	LogInfo("[[{}]] kicks [[{}]] from channel [[{}]]", GetName().c_str(), Kickee.c_str(), ChannelName.c_str());

	Client *RequiredClient = g_Clientlist->FindCharacter(Kickee);

	if (!RequiredClient) {

		GeneralChannelMessage("Player " + Kickee + " is not online.");
		return;
	}

	if (RequiredClient == this) {

		GeneralChannelMessage("You cannot kick yourself out of a channel.");
		return;
	}

	ChatChannel *RequiredChannel = ChannelList->FindChannel(ChannelName);

	if (!RequiredChannel) {

		GeneralChannelMessage("Channel " + ChannelName + " not found.");
		return;
	}

	if (!RequiredChannel->IsOwner(GetName()) && !RequiredChannel->IsModerator(GetName()) && !IsChannelAdmin()) {

		GeneralChannelMessage("You do not own or have moderator rights to channel " + ChannelName);
		return;
	}

	if (RequiredChannel->IsOwner(RequiredClient->GetName())) {

		GeneralChannelMessage("You cannot kick the owner out of the channel.");
		return;
	}

	if (RequiredChannel->IsModerator(Kickee) && !RequiredChannel->IsOwner(GetName())) {

		GeneralChannelMessage("Only the channel owner can kick a moderator out of the channel.");
		return;
	}

	if (RequiredChannel->IsModerator(Kickee)) {

		RequiredChannel->RemoveModerator(Kickee);

		RequiredClient->GeneralChannelMessage(GetName() + " has removed your moderator rights to channel " + ChannelName);

		GeneralChannelMessage("Removing moderator rights from " + Kickee + " to channel " + ChannelName);
	}

	RequiredClient->GeneralChannelMessage(GetName() + " has kicked you from channel " + ChannelName);

	GeneralChannelMessage("Kicked " + Kickee + " from channel " + ChannelName);

	RequiredClient->LeaveChannels(ChannelName, false);
}

void Client::ToggleInvites() {

	AllowInvites = !AllowInvites;

	if (AllowInvites)
		GeneralChannelMessage("You will now receive channel invitations.");
	else
		GeneralChannelMessage("You will no longer receive channel invitations.");

}

std::string Client::ChannelSlotName(int SlotNumber) {

	if ((SlotNumber < 1) || (SlotNumber > MAX_JOINED_CHANNELS))
		return "";

	if (JoinedChannels[SlotNumber - 1] == nullptr)
		return "";

	return JoinedChannels[SlotNumber - 1]->GetName();

}

void Client::SendHelp() {

	GeneralChannelMessage("Chat Channel Commands:");

	GeneralChannelMessage("/join, /leave, /leaveall, /list, /announce, /autojoin, ;set");
	GeneralChannelMessage(";oplist, ;grant, ;invite, ;kick, ;moderate, ;password, ;voice");
	GeneralChannelMessage(";setowner, ;toggleinvites");
}

void Client::AccountUpdate()
{
	if (AccountGrabUpdateTimer)
	{
		if (AccountGrabUpdateTimer->Check(false))
		{
			AccountGrabUpdateTimer->Start(60000);
			database.GetAccountStatus(this);
		}
	}
}

void Client::SetConnectionType(char c) {

	switch (c)
	{
	case EQ::versions::ucsTitaniumChat:
	{
		TypeOfConnection = ConnectionTypeChat;
		ClientVersion_ = EQ::versions::ClientVersion::Titanium;
		LogInfo("Connection type is Chat (Titanium)");
		break;
	}
	case EQ::versions::ucsTitaniumMail:
	{
		TypeOfConnection = ConnectionTypeMail;
		ClientVersion_ = EQ::versions::ClientVersion::Titanium;
		LogInfo("Connection type is Mail (Titanium)");
		break;
	}
	case EQ::versions::ucsSoFCombined:
	{
		TypeOfConnection = ConnectionTypeCombined;
		ClientVersion_ = EQ::versions::ClientVersion::SoF;
		LogInfo("Connection type is Combined (SoF)");
		break;
	}
	case EQ::versions::ucsSoDCombined:
	{
		TypeOfConnection = ConnectionTypeCombined;
		ClientVersion_ = EQ::versions::ClientVersion::SoD;
		LogInfo("Connection type is Combined (SoD)");
		break;
	}
	case EQ::versions::ucsUFCombined:
	{
		TypeOfConnection = ConnectionTypeCombined;
		ClientVersion_ = EQ::versions::ClientVersion::UF;
		UnderfootOrLater = true;
		LogInfo("Connection type is Combined (Underfoot)");
		break;
	}
	case EQ::versions::ucsRoFCombined:
	{
		TypeOfConnection = ConnectionTypeCombined;
		ClientVersion_ = EQ::versions::ClientVersion::RoF;
		UnderfootOrLater = true;
		LogInfo("Connection type is Combined (RoF)");
		break;
	}
	case EQ::versions::ucsRoF2Combined:
	{
		TypeOfConnection = ConnectionTypeCombined;
		ClientVersion_ = EQ::versions::ClientVersion::RoF2;
		UnderfootOrLater = true;
		LogInfo("Connection type is Combined (RoF2)");
		break;
	}
	default:
	{
		TypeOfConnection = ConnectionTypeUnknown;
		ClientVersion_ = EQ::versions::ClientVersion::Unknown;
		LogInfo("Connection type is unknown");
	}
	}
}

Client *Clientlist::IsCharacterOnline(const std::string& CharacterName) {

	// This method is used to determine if the character we are a sending an email to is connected to the mailserver,
	// so we can send them a new email notification.
	//
	// The way live works is that it sends a notification if a player receives an email for their 'primary' mailbox,
	// i.e. for the character they are logged in as, or for the character whose mailbox they have selected in the
	// mail window.
	//
	std::list<Client*>::iterator Iterator;

	for (Iterator = ClientChatConnections.begin(); Iterator != ClientChatConnections.end(); ++Iterator) {

		if (!(*Iterator)->IsMailConnection())
			continue;

		int MailBoxNumber = (*Iterator)->GetMailBoxNumber(CharacterName);

		// If the mail is destined for the primary mailbox for this character, or the one they have selected
		//
		if ((MailBoxNumber == 0) || (MailBoxNumber == (*Iterator)->GetMailBoxNumber()))
			return (*Iterator);

	}

	return nullptr;
}

int Client::GetMailBoxNumber(const std::string& CharacterName) {

	for (unsigned int i = 0; i < Characters.size(); i++)
		if (Characters[i].Name == CharacterName)
			return i;

	return -1;
}

void Client::SendNotification(int MailBoxNumber, const std::string& Subject, const std::string& From, int MessageID) {

	char TimeStamp[100];

	char sMessageID[100];

	char Sequence[100];

	sprintf(TimeStamp, "%i", (int)time(nullptr));

	sprintf(sMessageID, "%i", MessageID);

	sprintf(Sequence, "%i", 1);

	int PacketLength = 8 + strlen(sMessageID) + strlen(TimeStamp) + From.length() + Subject.length();

	auto outapp = new EQApplicationPacket(OP_MailNew, PacketLength);

	char *PacketBuffer = (char *)outapp->pBuffer;

	VARSTRUCT_ENCODE_INTSTRING(PacketBuffer, MailBoxNumber);
	VARSTRUCT_ENCODE_STRING(PacketBuffer, sMessageID);
	VARSTRUCT_ENCODE_STRING(PacketBuffer, TimeStamp);
	VARSTRUCT_ENCODE_STRING(PacketBuffer, "1");
	VARSTRUCT_ENCODE_STRING(PacketBuffer, From.c_str());
	VARSTRUCT_ENCODE_STRING(PacketBuffer, Subject.c_str());


	QueuePacket(outapp);

	safe_delete(outapp);
}

void Client::ChangeMailBox(int NewMailBox)
{
	LogInfo("[{}] Change to mailbox [{}]", MailBoxName().c_str(), NewMailBox);

	SetMailBox(NewMailBox);
	auto id = std::to_string(NewMailBox);

	LogInfo("New mailbox is [{}]", MailBoxName().c_str());

	auto outapp = new EQApplicationPacket(OP_MailboxChange, id.length() + 1);

	char *buf = (char *)outapp->pBuffer;

	VARSTRUCT_ENCODE_STRING(buf, id.c_str());

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendFriends() {

	std::vector<std::string> Friends, Ignorees;

	database.GetFriendsAndIgnore(GetCharID(), Friends, Ignorees);

	EQApplicationPacket *outapp;

	std::vector<std::string>::iterator Iterator;

	Iterator = Friends.begin();

	while (Iterator != Friends.end()) {

		outapp = new EQApplicationPacket(OP_Buddy, (*Iterator).length() + 2);

		char *PacketBuffer = (char *)outapp->pBuffer;

		VARSTRUCT_ENCODE_TYPE(uint8, PacketBuffer, 1);

		VARSTRUCT_ENCODE_STRING(PacketBuffer, (*Iterator).c_str());


		QueuePacket(outapp);

		safe_delete(outapp);

		++Iterator;
	}

	Iterator = Ignorees.begin();

	while (Iterator != Ignorees.end()) {

		std::string Ignoree = "SOE.EQ." + WorldShortName + "." + (*Iterator);

		outapp = new EQApplicationPacket(OP_Ignore, Ignoree.length() + 2);

		char *PacketBuffer = (char *)outapp->pBuffer;

		VARSTRUCT_ENCODE_TYPE(uint8, PacketBuffer, 0);

		VARSTRUCT_ENCODE_STRING(PacketBuffer, Ignoree.c_str());


		QueuePacket(outapp);

		safe_delete(outapp);

		++Iterator;
	}
}

std::string Client::MailBoxName()
{
	if ((Characters.empty()) || (CurrentMailBox > (Characters.size() - 1))) {
		LogDebug("MailBoxName() called with CurrentMailBox set to [{}] and Characters.size() is [{}]",
				 CurrentMailBox, Characters.size());

		return std::string();
	}

	LogDebug("MailBoxName() called with CurrentMailBox set to [{}] and Characters.size() is [{}]",
			 CurrentMailBox, Characters.size());

	return Characters[CurrentMailBox].Name;

}

int Client::GetCharID() {

	if (Characters.empty())
		return 0;

	return Characters[0].CharID;
}
