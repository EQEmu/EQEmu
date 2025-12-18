#pragma once

#include "common/linked_list.h"
#include "common/timer.h"

#include <string>
#include <vector>

class Client;

#define SYSTEM_OWNER std::string("*System*")

class ChatChannel {

public:

	ChatChannel(const std::string& inName, const std::string& inOwner, const std::string& inPassword, bool inPermanent, int inMinimumStatus = 0);
	~ChatChannel();

	void AddClient(Client *c);
	bool RemoveClient(Client *c);
	bool IsClientInChannel(Client *c);

	int MemberCount(int Status);
	const std::string &GetName() { return m_name; }
	void SendMessageToChannel(const std::string& Message, Client* Sender);
	bool CheckPassword(const std::string& in_password) { return m_password.empty() || m_password == in_password; }
	void SetPassword(const std::string& in_password);
	bool IsOwner(const std::string& name) { return (m_owner == name); }
	const std::string& GetPassword() { return m_password; }
	void SetOwner(const std::string& in_owner);
	std::string& GetOwnerName();
	void SetTemporary();
	void SetPermanent();
	void SendChannelMembers(Client *c);
	int GetMinStatus() { return m_minimum_status; }
	bool ReadyToDelete() { return m_delete_timer.Check(); }
	void SendOPList(Client *c);
	void AddInvitee(const std::string &Invitee);
	void RemoveInvitee(std::string Invitee);
	bool IsInvitee(std::string Invitee);
	void AddModerator(const std::string &Moderator);
	void RemoveModerator(const std::string &Moderator);
	bool IsModerator(std::string Moderator);
	void AddVoice(const std::string &Voiced);
	void RemoveVoice(const std::string &Voiced);
	bool HasVoice(std::string Voiced);
	inline bool IsModerated() { return m_moderated; }
	void SetModerated(bool inModerated);

	friend class ChatChannelList;

private:

	std::string m_name;
	std::string m_owner;
	std::string m_password;

	bool m_permanent;
	bool m_moderated;

	int m_minimum_status;

	Timer m_delete_timer;

	LinkedList<Client*> m_clients_in_channel;

	std::vector<std::string> m_moderators;
	std::vector<std::string> m_invitees;
	std::vector<std::string> m_voiced;

};

class ChatChannelList {

public:
	ChatChannel* CreateChannel(const std::string& name, const std::string& owner, const std::string& password, bool permanent, int minimum_status, bool save_to_database = false);
	ChatChannel* FindChannel(const std::string& name);
	ChatChannel* AddClientToChannel(std::string channel_name, Client* c, bool command_directed = false);
	ChatChannel* RemoveClientFromChannel(const std::string& in_channel_name, Client* c, bool command_directed = false);
	void RemoveChannel(ChatChannel *Channel);
	void RemoveAllChannels();
	void SendAllChannels(Client *c);
	void Process();
	static inline std::vector<std::string> GetBlockedChannelNames() { return m_blocked_channel_names; }
	static inline std::vector<std::string> GetFilteredNames() { return m_filtered_names; }
	static inline void ClearChannelBlockList() { m_blocked_channel_names.clear(); };
	static inline void ClearFilteredNameList() { m_filtered_names.clear(); };
	static void AddToChannelBlockList(const std::string& channel_name);
	static void AddToFilteredNames(const std::string& name);
	static bool IsOnChannelBlockList(const std::string& channel_name);
	static bool IsOnFilteredNameList(const std::string& channel_name);
	static inline void SetChannelBlockList(const std::vector<std::string>& new_list) { m_blocked_channel_names = new_list; }
	static inline void SetFilteredNameList(const std::vector<std::string>& new_list) { m_filtered_names = new_list; }
private:

	LinkedList<ChatChannel*> ChatChannels;
	static inline std::vector<std::string> m_blocked_channel_names;
	static inline std::vector<std::string> m_filtered_names;

};

std::string CapitaliseName(const std::string& inString);
