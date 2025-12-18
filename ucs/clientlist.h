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

#pragma once

#include "common/net/eqstream.h"
#include "common/opcodemgr.h"
#include "common/rulesys.h"
#include "ucs/chatchannel.h"

#include <list>
#include <vector>

#define MAX_JOINED_CHANNELS 10

enum {
	CommandJoin = 0, CommandLeaveAll, CommandLeave, CommandListAll, CommandList, CommandSet, CommandAnnounce, CommandSetOwner,
	CommandOPList, CommandInvite, CommandGrant, CommandModerate, CommandVoice, CommandKick,
	CommandPassword, CommandToggleInvites, CommandAFK, CommandUptime,
	CommandGetHeaders, CommandGetBody, CommandMailTo, CommandSetMessageStatus, CommandSelectMailBox,
	CommandSetMailForwarding, CommandBuddy, CommandIgnorePlayer,
	CommandEndOfList
};

struct CommandEntry {
	const char *CommandString;
	int CommandCode;
};

typedef enum { ConnectionTypeUnknown, ConnectionTypeCombined, ConnectionTypeMail, ConnectionTypeChat } ConnectionType;

static const CommandEntry Commands[] = {
					{ "join", CommandJoin },
					{ "leaveall", CommandLeaveAll },
					{ "leave", CommandLeave },
					{ "listall", CommandListAll },
					{ "list", CommandList },
					{ "set", CommandSet },
					{ "announce", CommandAnnounce },
					{ "setowner", CommandSetOwner },
					{ "oplist", CommandOPList },
					{ "invite", CommandInvite },
					{ "grant", CommandGrant },
					{ "moderate", CommandModerate },
					{ "voice", CommandVoice },
					{ "kick", CommandKick },
					{ "password", CommandPassword },
					{ "toggleinvites", CommandToggleInvites },
					{ "afk", CommandAFK },
					{ "uptime", CommandUptime },
					{ "getheaders", CommandGetHeaders },
					{ "getbody", CommandGetBody },
					{ "mailto", CommandMailTo },
					{ "setmessagestatus", CommandSetMessageStatus },
					{ "selectmailbox", CommandSelectMailBox },
					{ "setmailforwarding", CommandSetMailForwarding },
					{ "buddy", CommandBuddy },
					{ "ignoreplayer", CommandIgnorePlayer },
					{ "", CommandEndOfList } };

struct CharacterEntry {
	int CharID;
	int Level;
	std::string Name;
};

class Client {

public:
	Client(std::shared_ptr<EQStreamInterface> eqs);
	~Client();

	std::shared_ptr<EQStreamInterface> ClientStream;
	void AddCharacter(int CharID, const char *CharacterName, int Level);
	void ClearCharacters() { Characters.clear(); }
	void SendMailBoxes();
	inline void QueuePacket(const EQApplicationPacket *p, bool ack_req=true) { ClientStream->QueuePacket(p, ack_req); }
	std::string GetName() { if(Characters.size()) return Characters[0].Name; else return ""; }
	void JoinChannels(std::string& channel_name_list, bool command_directed = false);
	void LeaveChannels(std::string& channel_name_list, bool command_directed = false);
	void LeaveAllChannels(bool send_updated_channel_list = true, bool command_directed = false);
	void AddToChannelList(ChatChannel *JoinedChannel);
	void RemoveFromChannelList(ChatChannel *JoinedChannel);
	void SendChannelMessage(std::string Message);
	void SendChannelMessage(const std::string& ChannelName, const std::string& Message, Client *Sender);
	void SendChannelMessageByNumber(std::string Message);
	void SendChannelList();
	void CloseConnection();
	void ToggleAnnounce(const std::string& State);
	bool IsAnnounceOn() { return (Announce == true); }
	void AnnounceJoin(ChatChannel *Channel, Client *c);
	void AnnounceLeave(ChatChannel *Channel, Client *c);
	void GeneralChannelMessage(const std::string& Message);
	void GeneralChannelMessage(const char *Characters);
	void SetChannelPassword(std::string ChannelPassword);
	void ProcessChannelList(const std::string& Input);
	void AccountUpdate();
	int ChannelCount();
	std::string RemoveDuplicateChannels(std::string& in_channels);
	inline void SetAccountID(int inAccountID) { AccountID = inAccountID; }
	inline int GetAccountID() { return AccountID; }
	inline void SetAccountStatus(int inStatus) { Status = inStatus; }
	inline void SetHideMe(bool inHideMe) { HideMe = inHideMe; }
	inline void SetKarma(uint32 inKarma) { TotalKarma = inKarma; }
	inline int GetAccountStatus() { return Status; }
	inline bool GetHideMe() { return HideMe; }
	inline uint32 GetKarma() { return TotalKarma; }
	void SetChannelOwner(std::string CommandString);
	void OPList(std::string CommandString);
	void ChannelInvite(std::string CommandString);
	void ChannelGrantModerator(std::string CommandString);
	void ChannelGrantVoice(std::string CommandString);
	void ChannelKick(std::string CommandString);
	void ChannelModerate(std::string CommandString);
	std::string ChannelSlotName(int ChannelNumber);
	void ToggleInvites();
	bool InvitesAllowed() { return AllowInvites; }
	bool IsRevoked() { return Revoked; }
	void SetRevoked(bool r) { Revoked = r; }
	inline bool IsChannelAdmin() { return (Status >= RuleI(Channels, RequiredStatusAdmin)); }
	inline bool CanListAllChannels() { return (Status >= RuleI(Channels, RequiredStatusListAll)); }
	void SendHelp();
	inline bool GetForceDisconnect() { return ForceDisconnect; }
	std::string MailBoxName();
	int GetMailBoxNumber() { return CurrentMailBox; }
	int GetMailBoxNumber(const std::string& CharacterName);

	void SetConnectionType(char c);
	ConnectionType GetConnectionType() { return TypeOfConnection; }
	EQ::versions::ClientVersion GetClientVersion() { return ClientVersion_; }

	inline bool IsMailConnection() { return (TypeOfConnection == ConnectionTypeMail) || (TypeOfConnection == ConnectionTypeCombined); }
	void SendNotification(int MailBoxNumber, const std::string& Subject, const std::string& From, int MessageID);
	void ChangeMailBox(int NewMailBox);
	inline void SetMailBox(int NewMailBox) { CurrentMailBox = NewMailBox; }
	void SendFriends();
	int GetCharID();
	void SendUptime();
	void SendKeepAlive();

private:
	unsigned int CurrentMailBox;
	std::vector<CharacterEntry> Characters;
	ChatChannel *JoinedChannels[MAX_JOINED_CHANNELS];
	bool Announce;
	int AccountID;
	int Status;
	bool HideMe;
	bool AllowInvites;
	bool Revoked;

	//Anti Spam Stuff
	Timer *AccountGrabUpdateTimer;
	uint32 TotalKarma;

	Timer *GlobalChatLimiterTimer; //60 seconds
	int AttemptedMessages;
	bool ForceDisconnect;

	ConnectionType TypeOfConnection;
	EQ::versions::ClientVersion ClientVersion_;
	bool UnderfootOrLater;
};

class Clientlist {

public:
	Clientlist(int MailPort);
	void	Process();
	void	CloseAllConnections();
	Client *FindCharacter(const std::string& CharacterName);
	void	CheckForStaleConnectionsAll();
	void	CheckForStaleConnections(Client *c);
	Client *IsCharacterOnline(const std::string& CharacterName);
	void ProcessOPMailCommand(Client* c, std::string command_string, bool command_directed = false);

private:
	EQ::Net::EQStreamManager *chatsf;

	std::list<Client*> ClientChatConnections;

	OpcodeManager *ChatOpMgr;
};
