/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2003 EQEMu Development Team (http://eqemulator.net)

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

#include "client.h"

#include "common/eq_packet_structs.h"
#include "common/features.h"
#include "common/guild_base.h"
#include "common/repositories/guild_tributes_repository.h"
#include "zone/guild_mgr.h"
#include "zone/worldserver.h"

#include <map>

/*

The server periodicly sends tribute timer updates to the client on live,
but I dont see a point to that right now, so I dont do it.

*/
extern WorldServer worldserver;
std::map<uint32, TributeData> tribute_list{};

void Client::ToggleTribute(bool enabled) {
	if(enabled) {
		//make sure they have enough points to be activating this...
		int r;
		uint32 cost = 0;
		uint32 level = GetLevel();
		for (r = 0; r < EQ::invtype::TRIBUTE_SIZE; r++) {
			uint32 tid = m_pp.tributes[r].tribute;
			if(tid == TRIBUTE_NONE)
				continue;

			if(tribute_list.count(tid) != 1)
				continue;

			if(m_pp.tributes[r].tier >= MAX_TRIBUTE_TIERS) {
				m_pp.tributes[r].tier = 0;	//sanity check.
				continue;
			}

			TributeData &d = tribute_list[tid];

			TributeLevel_Struct &tier = d.tiers[m_pp.tributes[r].tier];

			if(level < tier.level) {
				Message(0, "You are not high enough level to activate this tribute!");
				ToggleTribute(false);
				continue;
			}

			cost += tier.cost;
		}

		if(cost > m_pp.tribute_points) {
			Message(Chat::Red, "You do not have enough tribute points to activate your tribute!");
			ToggleTribute(false);
			return;
		}
		AddTributePoints(0-cost);

		//reset their timer, since they just paid for a full duration
		m_pp.tribute_time_remaining = Tribute_duration;	//full duration
		tribute_timer.Start(m_pp.tribute_time_remaining);

		m_pp.tribute_active = 1;
	} else {
		m_pp.tribute_active = 0;
		tribute_timer.Disable();
	}
	DoTributeUpdate();
}

void Client::DoTributeUpdate() {
	EQApplicationPacket outapp(OP_TributeUpdate, sizeof(TributeInfo_Struct));
	TributeInfo_Struct *tis = (TributeInfo_Struct *) outapp.pBuffer;

	tis->active = m_pp.tribute_active ? 1 : 0;
	tis->tribute_master_id = tribute_master_id;	//Dont know what this is for

	int r;
	for (r = 0; r < EQ::invtype::TRIBUTE_SIZE; r++) {
		if(m_pp.tributes[r].tribute != TRIBUTE_NONE) {
			tis->tributes[r] = m_pp.tributes[r].tribute;
			tis->tiers[r] = m_pp.tributes[r].tier;
		} else {
			tis->tributes[r] = TRIBUTE_NONE;
			tis->tiers[r] = 0;
		}
	}
	QueuePacket(&outapp);

	SendTributeTimer();

	if(m_pp.tribute_active) {
		//send and equip tribute items...
		for (r = 0; r < EQ::invtype::TRIBUTE_SIZE; r++) {
			uint32 tid = m_pp.tributes[r].tribute;
			if(tid == TRIBUTE_NONE) {
				if (m_inv[EQ::invslot::TRIBUTE_BEGIN + r])
					DeleteItemInInventory(EQ::invslot::TRIBUTE_BEGIN + r);
				continue;
			}

			if(tribute_list.count(tid) != 1) {
				if (m_inv[EQ::invslot::TRIBUTE_BEGIN + r])
					DeleteItemInInventory(EQ::invslot::TRIBUTE_BEGIN + r);
				continue;
			}

			//sanity check
			if(m_pp.tributes[r].tier >= MAX_TRIBUTE_TIERS) {
				if (m_inv[EQ::invslot::TRIBUTE_BEGIN + r])
					DeleteItemInInventory(EQ::invslot::TRIBUTE_BEGIN + r);
				m_pp.tributes[r].tier = 0;
				continue;
			}

			TributeData &d = tribute_list[tid];
			TributeLevel_Struct &tier = d.tiers[m_pp.tributes[r].tier];
			uint32 item_id = tier.tribute_item_id;

			//summon the item for them
			const EQ::ItemInstance* inst = database.CreateItem(item_id, 1);
			if(inst == nullptr)
				continue;

			PutItemInInventory(EQ::invslot::TRIBUTE_BEGIN + r, *inst);
			SendItemPacket(EQ::invslot::TRIBUTE_BEGIN + r, inst, ItemPacketTributeItem);
			safe_delete(inst);
		}
	} else {
		//unequip tribute items...
		for (r = 0; r < EQ::invtype::TRIBUTE_SIZE; r++) {
			if (m_inv[EQ::invslot::TRIBUTE_BEGIN + r])
				DeleteItemInInventory(EQ::invslot::TRIBUTE_BEGIN + r);
		}
	}
	CalcBonuses();
}

void Client::SendTributeTimer() {
	//update their timer.
	EQApplicationPacket outapp2(OP_TributeTimer, sizeof(uint32));
	uint32 *timeleft = (uint32 *) outapp2.pBuffer;
	if(m_pp.tribute_active)
		*timeleft = m_pp.tribute_time_remaining;
	else
		*timeleft = Tribute_duration;	//full duration
	QueuePacket(&outapp2);
}

void Client::ChangeTributeSettings(TributeInfo_Struct *t) {
	int r;
	for (r = 0; r < EQ::invtype::TRIBUTE_SIZE; r++) {

		m_pp.tributes[r].tribute = TRIBUTE_NONE;

		uint32 tid = t->tributes[r];
		if(tid == TRIBUTE_NONE)
			continue;

		if(tribute_list.count(tid) != 1)
			continue;	//print a cheater warning?

		TributeData &d = tribute_list[tid];

		//make sure they chose a valid tier
		if(t->tiers[r] >= d.tier_count)
			continue;	//print a cheater warning?

		//might want to check required level, even though its checked before activate

		m_pp.tributes[r].tribute = tid;
		m_pp.tributes[r].tier = t->tiers[r];
	}

	DoTributeUpdate();
}

void Client::SendTributeDetails(uint32 client_id, uint32 tribute_id) {
	if(tribute_list.count(tribute_id) != 1) {
		LogError("Details request for invalid tribute [{}]", (unsigned long)tribute_id);
		return;
	}
	TributeData &td = tribute_list[tribute_id];

	int len = td.description.length();
	EQApplicationPacket outapp(OP_SelectTribute, sizeof(SelectTributeReply_Struct)+len+1);
	SelectTributeReply_Struct *t = (SelectTributeReply_Struct *) outapp.pBuffer;

	t->client_id = client_id;
	t->tribute_id = tribute_id;
	memcpy(t->description, td.description.c_str(), len);
	t->description[len] = '\0';

	QueuePacket(&outapp);
}

//returns the number of points received from the tribute
int32 Client::TributeItem(uint32 slot, uint32 quantity) {
	const EQ::ItemInstance*inst = m_inv[slot];

	if(inst == nullptr)
		return(0);

	//figure out what its worth
	int32 pts = inst->GetItem()->Favor;

	if(pts < 1) {
		Message(Chat::Red, "This item is worthless for favor.");
		return(0);
	}

	/*
		Make sure they have enough of them
		and remove it from inventory
	*/
	if(inst->IsStackable()) {
		if(inst->GetCharges() < (int32)quantity)	//dont have enough....
			return(0);
		DeleteItemInInventory(slot, quantity);
	} else {
		quantity = 1;
		DeleteItemInInventory(slot);
	}

	pts *= quantity;

	/* Add the tribute value in points */
	AddTributePoints(pts);
	return(pts);
}

//returns the number of points received from the tribute
int32 Client::TributeMoney(uint32 platinum) {
	if(!TakeMoneyFromPP(platinum * 1000)) {
		Message(Chat::Red, "You do not have that much money!");
		return(0);
	}

	/* Add the tribute value in points */
	AddTributePoints(platinum);
	return(platinum);
}

void Client::AddTributePoints(int32 ammount) {
	EQApplicationPacket outapp(OP_TributePointUpdate, sizeof(TributePoint_Struct));
	TributePoint_Struct *t = (TributePoint_Struct *) outapp.pBuffer;

	//change the point values.
	m_pp.tribute_points += ammount;

	//career only tracks points earned, not spent.
	if(ammount > 0)
		m_pp.career_tribute_points += ammount;

	//fill in the packet.
	t->career_tribute_points = m_pp.career_tribute_points;
	t->tribute_points = m_pp.tribute_points;

	QueuePacket(&outapp);
}

void Client::SendTributes() {

	std::map<uint32, TributeData>::iterator cur,end;
	cur = tribute_list.begin();
	end = tribute_list.end();

	for(; cur != end; ++cur) {
		if(cur->second.is_guild)
			continue;	//skip guild tributes here
		int len = cur->second.name.length();
		EQApplicationPacket outapp(OP_TributeInfo, sizeof(TributeAbility_Struct) + len + 1);
		TributeAbility_Struct* tas = (TributeAbility_Struct*)outapp.pBuffer;

		tas->tribute_id = htonl(cur->first);
		tas->tier_count = htonl(cur->second.unknown);

		//gotta copy over the data from tiers, and flip all the
		//byte orders, no idea why its flipped here
		uint32 r, c;
		c = cur->second.tier_count;
		TributeLevel_Struct *dest = tas->tiers;
		TributeLevel_Struct *src = cur->second.tiers;
		for(r = 0; r < c; r++, dest++, src++) {
			dest->cost = htonl(src->cost);
			dest->level = htonl(src->level);
			dest->tribute_item_id = htonl(src->tribute_item_id);
		}

		memcpy(tas->name, cur->second.name.c_str(), len);
		tas->name[len] = '\0';
		QueuePacket(&outapp);
	}
}

bool ZoneDatabase::LoadTributes() {

	TributeData tributeData;
	memset(&tributeData.tiers, 0, sizeof(tributeData.tiers));
	tributeData.tier_count = 0;

	tribute_list.clear();

	const std::string query = "SELECT id, name, descr, unknown, isguild FROM tributes";
	auto results = QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

    for (auto &row = results.begin(); row != results.end(); ++row) {
        uint32 id = Strings::ToUnsignedInt(row[0]);
		tributeData.name = row[1];
		tributeData.description = row[2];
		tributeData.unknown = strtoul(row[3], nullptr, 10);
		tributeData.is_guild = atol(row[4]) == 0? false: true;

		tribute_list[id] = tributeData;
    }

	LogInfo("Loaded [{}] tributes", Strings::Commify(results.RowCount()));

	const std::string query2 = "SELECT tribute_id, level, cost, item_id FROM tribute_levels ORDER BY tribute_id, level";
	results = QueryDatabase(query2);
	if (!results.Success()) {
		return false;
	}

	for (auto &row = results.begin(); row != results.end(); ++row) {
		uint32 id = Strings::ToUnsignedInt(row[0]);

		if (tribute_list.count(id) != 1) {
			LogError("Error in LoadTributes: unknown tribute [{}] in tribute_levels", (unsigned long) id);
			continue;
		}

		TributeData &cur = tribute_list[id];

		if (cur.tier_count >= MAX_TRIBUTE_TIERS) {
			LogError("Error in LoadTributes: on tribute [{}]: more tiers defined than permitted", (unsigned long) id);
			continue;
		}

		TributeLevel_Struct &s = cur.tiers[cur.tier_count];

		s.level           = Strings::ToUnsignedInt(row[1]);
		s.cost            = Strings::ToUnsignedInt(row[2]);
		s.tribute_item_id = Strings::ToUnsignedInt(row[3]);
		cur.tier_count++;
	}

	LogInfo("Loaded [{}] tribute levels", Strings::Commify(results.RowCount()));

	return true;
}

void Client::SendGuildTributes()
{
	for (auto const& t : tribute_list) {
		if (!t.second.is_guild)
			continue;	//skip non guild tributes here

		//guild tribute has an unknown uint32 at its begining, guild ID?
		int len = t.second.name.length() + 1;

		EQApplicationPacket* outapp = new EQApplicationPacket(OP_SendGuildTributes, sizeof(GuildTributeAbility_Struct) + len);
		GuildTributeAbility_Struct* gtas = (GuildTributeAbility_Struct*)outapp->pBuffer;

		auto tier_count = t.second.tier_count;
		for (int ti = 0; ti < t.second.tier_count; ti++) {
			if (RuleB(Guild, UseCharacterMaxLevelForGuildTributes) && t.second.tiers[ti].level > RuleI(Character, MaxLevel)) {
				tier_count -= 1;
				continue;
			}
			gtas->guild_id = GuildID();
			gtas->ability.tier_count = htonl(tier_count);
			gtas->ability.tribute_id = htonl(t.first);
			gtas->ability.tiers[ti].cost = htonl(t.second.tiers[ti].cost);
			gtas->ability.tiers[ti].tribute_item_id = htonl(t.second.tiers[ti].tribute_item_id);
			gtas->ability.tiers[ti].level = htonl(t.second.tiers[ti].level);
		}
		strn0cpy(gtas->ability.name, t.second.name.data(), t.second.name.length());
		FastQueuePacket(&outapp);
	}
}

void Client::SendGuildTributeDetails(uint32 tribute_id, uint32 tier)
{
	if (tribute_list.count(tribute_id) != 1) {
		LogGuilds("Details request for invalid tribute [{}]", tribute_id);
		return;
	}

	TributeData& td = tribute_list[tribute_id];

	int len = td.description.length();
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GuildSelectTribute, sizeof(GuildTributeSelectReply_Struct) + len + 1);
	GuildTributeSelectReply_Struct* t = (GuildTributeSelectReply_Struct*)outapp->pBuffer;

	t->tribute_id = tribute_id;
	t->tier = tier;
	t->tribute_id2 = tribute_id;
	strn0cpy(&t->description, td.description.c_str(), len);

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::DoGuildTributeUpdate()
{
	LogTribute("DoGuildTributeUpdate");
	auto guild = guild_mgr.GetGuildByGuildID(GuildID());

	if (guild && guild->tribute.enabled && GuildTributeOptIn()) {
		TributeData& d1 = tribute_list[guild->tribute.id_1];
		uint32 item_id1 = d1.tiers[guild->tribute.id_1_tier].tribute_item_id;
		TributeData& d2 = tribute_list[guild->tribute.id_2];
		uint32 item_id2 = d2.tiers[guild->tribute.id_2_tier].tribute_item_id;

		if (item_id1) {
			LogGuilds("Guild Tribute Item 1 is {}", item_id1);
			const EQ::ItemInstance* inst = database.CreateItem(item_id1, 1);
			if (!inst) {
				LogGuilds("Guild Tribute Item 1 was not found. {}", item_id1);
				return;
			}
			auto inst_level = d1.tiers[guild->tribute.id_1_tier].level;
			if (m_inv[EQ::invslot::GUILD_TRIBUTE_BEGIN]) {
				LogGuilds("Guild Tribute DELETE Item in Slot 450");
				DeleteItemInInventory(EQ::invslot::GUILD_TRIBUTE_BEGIN);
			}

			if ((RuleB(Guild,UseCharacterMaxLevelForGuildTributes) && RuleI(Character, MaxLevel) >= inst_level && GetLevel() >= inst_level) ||
				!RuleB(Guild, UseCharacterMaxLevelForGuildTributes)) {
				PutItemInInventory(EQ::invslot::GUILD_TRIBUTE_BEGIN, *inst);
				SendItemPacket(EQ::invslot::GUILD_TRIBUTE_BEGIN, inst, ItemPacketGuildTribute);
			}

			safe_delete(inst);
		}

		if (item_id2) {
			LogInfo("Guild Tribute Item 2 is {}", item_id2);
			const EQ::ItemInstance* inst = database.CreateItem(item_id2, 1);
			if (!inst) {
				LogGuilds("Guild Tribute Item 1 was not found. {}", item_id2);
				return;
			}
			auto inst_level = d2.tiers[guild->tribute.id_2_tier].level;
			if (m_inv[EQ::invslot::GUILD_TRIBUTE_BEGIN + 1]) {
				DeleteItemInInventory(EQ::invslot::GUILD_TRIBUTE_BEGIN + 1);
				LogGuilds("Guild Tribute DELETE Item in Slot 451");
			}

			if ((RuleB(Guild, UseCharacterMaxLevelForGuildTributes) && RuleI(Character, MaxLevel) >= inst_level && GetLevel() >= inst_level) ||
				!RuleB(Guild, UseCharacterMaxLevelForGuildTributes)) {
				PutItemInInventory(EQ::invslot::GUILD_TRIBUTE_BEGIN + 1, *inst);
				SendItemPacket(EQ::invslot::GUILD_TRIBUTE_BEGIN + 1, inst, ItemPacketGuildTribute);
			}

			safe_delete(inst);
		}
	}
	else {
		if (m_inv[EQ::invslot::GUILD_TRIBUTE_BEGIN]) {
			DeleteItemInInventory(EQ::invslot::GUILD_TRIBUTE_BEGIN);
		}
		if (m_inv[EQ::invslot::GUILD_TRIBUTE_BEGIN + 1]) {
			DeleteItemInInventory(EQ::invslot::GUILD_TRIBUTE_BEGIN + 1);
		}
	}
	CalcBonuses();
}

void Client::SendGuildActiveTributes(uint32 guild_id)
{
	auto guild = guild_mgr.GetGuildByGuildID(guild_id);

	auto outapp = new EQApplicationPacket(OP_GuildSendActiveTributes, sizeof(GuildTributeSendActive_Struct));
	auto gtsa   = (GuildTributeSendActive_Struct *) outapp->pBuffer;

	if (guild) {
		gtsa->guild_favor       = guild->tribute.favor;
		gtsa->tribute_timer     = guild->tribute.time_remaining;
		gtsa->tribute_enabled   = guild->tribute.enabled;
		gtsa->tribute_id_1      = guild->tribute.id_1;
		gtsa->tribute_id_1_tier = guild->tribute.id_1_tier;
		gtsa->tribute_id_2      = guild->tribute.id_2;
		gtsa->tribute_id_2_tier = guild->tribute.id_2_tier;
	}
	else {
		gtsa->guild_favor       = 0;
		gtsa->tribute_timer     = 0;
		gtsa->tribute_enabled   = 0;
		gtsa->tribute_id_1      = 0xffffffff;
		gtsa->tribute_id_1_tier = 0;
		gtsa->tribute_id_2      = 0xffffffff;
		gtsa->tribute_id_2_tier = 0;
	}

	QueuePacket(outapp);
	safe_delete(outapp)
}

void Client::SendGuildFavorAndTimer(uint32 guild_id)
{
	auto guild = guild_mgr.GetGuildByGuildID(guild_id);

	if (guild) {
		auto outapp = new EQApplicationPacket(OP_GuildTributeFavorAndTimer, sizeof(GuildTributeFavorTimer_Struct));
		auto gtsa   = (GuildTributeFavorTimer_Struct *) outapp->pBuffer;

		gtsa->guild_favor   = guild->tribute.favor;
		gtsa->tribute_timer = guild->tribute.time_remaining;
		gtsa->trophy_timer  = 0; //not yet implemented

		QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Client::SendGuildTributeOptInToggle(const GuildTributeMemberToggle* in)
{
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GuildOptInOut, sizeof(GuildTributeOptInOutReply_Struct));
	GuildTributeOptInOutReply_Struct* data = (GuildTributeOptInOutReply_Struct*)outapp->pBuffer;

	strn0cpy(data->player_name, in->player_name, sizeof(data->player_name));
	data->guild_id              = in->guild_id;
	data->no_donations          = in->no_donations;
	data->tribute_toggle        = in->tribute_toggle;
	data->tribute_trophy_toggle = 0; //not yet implemented
	data->time                  = time(nullptr);
	data->command               = in->command;

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::RequestGuildActiveTributes(uint32 guild_id)
{
	auto sp   = new ServerPacket(ServerOP_RequestGuildActiveTributes, sizeof(GuildTributeUpdate));
	auto data = (GuildTributeUpdate *) sp->pBuffer;

	data->guild_id = GuildID();

	worldserver.SendPacket(sp);
	safe_delete(sp);
}

void Client::RequestGuildFavorAndTimer(uint32 guild_id)
{
	auto sp   = new ServerPacket(ServerOP_RequestGuildFavorAndTimer, sizeof(GuildTributeUpdate));
	auto data = (GuildTributeUpdate *) sp->pBuffer;

	data->guild_id = GuildID();

	worldserver.SendPacket(sp);
	safe_delete(sp);
}

void Client::SendGuildTributeDonateItemReply(GuildTributeDonateItemRequest_Struct* in, uint32 favor) {

	auto outapp = new EQApplicationPacket(OP_GuildTributeDonateItem, sizeof(GuildTributeDonateItemReply_Struct));
	auto out = (GuildTributeDonateItemReply_Struct*)outapp->pBuffer;

	out->type      = in->type;
	out->slot      = in->slot;
	out->aug_index = in->aug_index;
	out->sub_index = in->sub_index;
	out->quantity  = in->quantity;
	out->unknown10 = in->unknown10;
	out->unknown20 = in->unknown20;
	out->favor     = favor;

	QueuePacket(outapp);
	safe_delete(outapp);

}

void Client::SendGuildTributeDonatePlatReply(GuildTributeDonatePlatRequest_Struct* in, uint32 favor) {

	auto outapp = new EQApplicationPacket(OP_GuildTributeDonatePlat, sizeof(GuildTributeDonatePlatReply_Struct));
	auto out = (GuildTributeDonatePlatReply_Struct*)outapp->pBuffer;

	out->favor    = favor;
	out->quantity = in->quantity;

	QueuePacket(outapp);
	safe_delete(outapp)

}

std::map<uint32, TributeData> Client::GetTributeList() {
	return tribute_list;
}

uint32 Client::LookupTributeItemID(uint32 tribute_id, uint32 tier)
{
	if (!tribute_id && !tier) {
		return 0;
	}

	if (tribute_list.contains(tribute_id)) {
		auto tribute = tribute_list.find(tribute_id);
		auto item_id = tribute->second.tiers[tier].tribute_item_id;
		if (item_id) {
			return item_id;
		}
	}
	return 0;
}

/*
64.37.149.6:1353 == server
66.90.221.245:3173 == client

84 01 00 00 == NPC ID of tribute master
08 0B 00 00 == Client ID

 [64.37.149.6:1353 ->
2004/08/19 04:09:07 GMT: 66.90.221.245:3173]
2004/08/19 04:09:07 GMT:  Child Packet:
  [OPCode: OP_Tribute] [Raw OPCode: OP_Tribute] [Size: 144]
   0: 00 00 00 00 00 00 00 05 - 00 00 00 14 00 00 DB EC  | ................
  16: 00 00 00 05 00 00 00 1E - 00 00 DB ED 00 00 00 0C  | ................
  32: 00 00 00 28 00 00 DB EE - 00 00 00 14 00 00 00 32  | ...(...........2
  48: 00 00 DB EF 00 00 00 1D - 00 00 00 3C 00 00 DB F0  | ...........<....
  64: 00 00 00 25 00 00 00 00 - 00 00 00 00 00 00 00 00  | ...%............
  80: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
  96: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
 112: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
 128: 41 75 72 61 20 6F 66 20 - 43 6C 61 72 69 74 79 00  | Aura of Clarity.

... many of those ...


2004/08/19 04:09:20 GMT: OPCode OP_StartTribute [Raw OPCode: OP_StartTribute] [Size: 12]
 [66.90.221.245:3173 ->
2004/08/19 04:09:20 GMT: 64.37.149.6:1353]
   0: 08 0B 00 00 84 01 00 00 - A9 E1 94 42              | ...........B


2004/08/19 04:09:20 GMT: OPCode 0x02f2 [Raw OPCode: 0x72f2] [Size: 0] [Compressed Size: 65]
 [64.37.149.6:1353 ->
2004/08/19 04:09:20 GMT: 66.90.221.245:3173]
 [Packet is compressed]
 [Packet is encrypted]
 [Packet is packed]
2004/08/19 04:09:20 GMT:  Child Packet:
  [OPCode: 0x02f2] [Raw OPCode: 0x02f2] [Size: 48]
   0: 00 00 00 00 FF FF FF FF - FF FF FF FF FF FF FF FF  | ................
  16: FF FF FF FF FF FF FF FF - 00 00 00 00 00 00 00 00  | ................
  32: 00 00 00 00 00 00 00 00 - 00 00 00 00 FF FF FF FF  | ................

2004/08/19 04:09:20 GMT:  Child Packet:
  [OPCode: 0x02f8] [Raw OPCode: 0x02f8] [Size: 4]
   0: C0 27 09 00                                        | .'..

2004/08/19 04:09:20 GMT:  Child Packet:
  [OPCode: OP_StartTribute] [Raw OPCode: OP_StartTribute] [Size: 12]
   0: 08 0B 00 00 84 01 00 00 - 01 00 00 00              | ............



2004/08/19 04:09:23 GMT: OPCode OP_SelectTribute [Raw OPCode: OP_SelectTribute] [Size: 12]
 [66.90.221.245:3173 ->
2004/08/19 04:09:23 GMT: 64.37.149.6:1353]
   0: 01 00 00 00 04 00 00 00 - E3 00 00 00              | ............

2004/08/19 04:09:23 GMT:  Child Packet:
  [OPCode: OP_SelectTribute] [Raw OPCode: OP_SelectTribute] [Size: 100]
   0: 01 00 00 00 04 00 00 00 - 4F 75 72 20 67 72 65 61  | ........Our grea
  16: 74 65 73 74 20 77 61 72 - 72 69 6F 72 73 20 66 6F  | test warriors fo
  32: 63 75 73 20 74 6F 20 69 - 6E 63 72 65 61 73 65 20  | cus to increase
  48: 79 6F 75 72 20 73 74 72 - 65 6E 67 74 68 2E 3C 62  | your strength.<b
  64: 72 3E 42 65 6E 65 66 69 - 74 20 2D 3C 62 72 3E 32  | r>Benefit -<br>2
  80: 20 53 74 72 65 6E 67 74 - 68 20 70 65 72 20 74 69  |  Strength per ti
  96: 65 72 2E 00                                        | er..

2004/08/19 04:09:28 GMT: OPCode OP_SelectTribute [Raw OPCode: OP_SelectTribute] [Size: 12]
 [66.90.221.245:3173 ->
2004/08/19 04:09:28 GMT: 64.37.149.6:1353]
   0: 01 00 00 00 17 00 00 00 - E3 00 00 00              | ............

2004/08/19 04:09:29 GMT: OPCode OP_SelectTribute [Raw OPCode: 0x72f7] [Size: 0] [Compressed Size: 205]
 [64.37.149.6:1353 ->
2004/08/19 04:09:29 GMT: 66.90.221.245:3173]
 [Packet is compressed]
 [Packet is encrypted]
 [Packet is packed]
2004/08/19 04:09:29 GMT:  Child Packet:
  [OPCode: OP_SelectTribute] [Raw OPCode: OP_SelectTribute] [Size: 232]
   0: 01 00 00 00 17 00 00 00 - 54 68 65 20 63 68 61 72  | ........The char
  16: 69 74 61 62 6C 65 20 74 - 61 6C 65 73 20 6F 66 20  | itable tales of
  32: 61 20 74 61 6C 65 6E 74 - 65 64 20 70 6F 65 74 20  | a talented poet
  48: 6C 65 6E 64 20 65 66 66 - 69 63 69 65 6E 63 79 20  | lend efficiency
  64: 74 6F 20 79 6F 75 72 20 - 62 65 6E 65 66 69 63 69  | to your benefici
  80: 61 6C 20 73 70 65 6C 6C - 73 2E 3C 62 72 3E 42 65  | al spells.<br>Be
  96: 6E 65 66 69 74 20 2D 3C - 62 72 3E 54 69 65 72 20  | nefit -<br>Tier
 112: 31 3A 20 45 6E 68 61 6E - 63 65 6D 65 6E 74 20 48  | 1: Enhancement H
 128: 61 73 74 65 20 49 3C 62 - 72 3E 54 69 65 72 20 32  | aste I<br>Tier 2
 144: 3A 20 45 6E 68 61 6E 63 - 65 6D 65 6E 74 20 48 61  | : Enhancement Ha
 160: 73 74 65 20 49 49 3C 62 - 72 3E 54 69 65 72 20 33  | ste II<br>Tier 3
 176: 3A 20 45 6E 68 61 6E 63 - 65 6D 65 6E 74 20 48 61  | : Enhancement Ha
 192: 73 74 65 20 49 49 49 3C - 62 72 3E 54 69 65 72 20  | ste III<br>Tier
 208: 34 3A 20 45 6E 68 61 6E - 63 65 6D 65 6E 74 20 48  | 4: Enhancement H
 224: 61 73 74 65 20 49 56 00                            | aste IV.




Donating shoulderpads:


2004/08/19 04:09:50 GMT: OPCode 0x02f3 [Raw OPCode: 0x02f3] [Size: 16]
 [66.90.221.245:3173 ->
2004/08/19 04:09:50 GMT: 64.37.149.6:1353]
   0: 1D 00 00 00 01 00 00 00 - 84 01 00 00 60 A9 D9 32  | ............`..2

 [64.37.149.6:1353 ->
2004/08/19 04:09:50 GMT: 66.90.221.245:3173]
2004/08/19 04:09:50 GMT:  Child Packet:
  [OPCode: 0x02f3] [Raw OPCode: 0x02f3] [Size: 16]
   0: 1D 00 00 00 01 00 00 00 - 84 01 00 00 02 00 00 00  | ................
2004/08/19 04:09:50 GMT:  Child Packet:
  [OPCode: 0x02f4] [Raw OPCode: 0x02f4] [Size: 16]
   0: 02 00 00 00 00 00 00 00 - 02 00 00 00 00 00 00 00  | ................





Donating Platinum:

2004/08/19 04:10:34 GMT: OPCode 0x02fe [Raw OPCode: 0x02fe] [Size: 12]
 [66.90.221.245:3173 ->
2004/08/19 04:10:34 GMT: 64.37.149.6:1353]
   0: 12 00 00 00 84 01 00 00 - 5F A6 E7 77              | ........_..w

 [64.37.149.6:1353 ->
2004/08/19 04:10:34 GMT: 66.90.221.245:3173]
2004/08/19 04:10:34 GMT:  Child Packet:
  [OPCode: 0x02fe] [Raw OPCode: 0x02fe] [Size: 12]
   0: 12 00 00 00 84 01 00 00 - 12 00 00 00              | ............
2004/08/19 04:10:34 GMT:  Child Packet:
  [OPCode: 0x02f4] [Raw OPCode: 0x02f4] [Size: 16]
   0: 14 00 00 00 00 00 00 00 - 14 00 00 00 00 00 00 00  | ................




Upgrading:

2004/08/19 04:10:37 GMT: OPCode OP_SelectTribute [Raw OPCode: OP_SelectTribute] [Size: 12]
 [66.90.221.245:3173 ->
2004/08/19 04:10:37 GMT: 64.37.149.6:1353]
   0: 01 00 00 00 28 00 00 00 - E3 00 00 00              | ....(.......

server -> client
2004/08/19 04:10:38 GMT:  Child Packet:
  [OPCode: OP_SelectTribute] [Raw OPCode: OP_SelectTribute] [Size: 128]
   0: 01 00 00 00 28 00 00 00 - 4F 75 72 20 68 65 61 6C  | ....(...Our heal
  16: 65 72 73 20 67 69 76 65 - 20 79 6F 75 20 61 6E 20  | ers give you an
  32: 61 6E 74 69 62 6F 64 79 - 2C 20 77 68 69 63 68 20  | antibody, which
  48: 68 65 6C 70 73 20 70 72 - 6F 74 65 63 74 20 79 6F  | helps protect yo
  64: 75 20 66 72 6F 6D 20 64 - 69 73 65 61 73 65 73 2E  | u from diseases.
  80: 3C 62 72 3E 42 65 6E 65 - 66 69 74 20 2D 3C 62 72  | <br>Benefit -<br
  96: 3E 35 20 44 69 73 65 61 - 73 65 20 52 65 73 69 73  | >5 Disease Resis
 112: 74 61 6E 63 65 20 70 65 - 72 20 74 69 65 72 2E 00  | tance per tier..

request tribute
2004/08/19 04:10:49 GMT: OPCode 0x02f2 [Raw OPCode: 0x02f2] [Size: 48]
 [66.90.221.245:3173 ->
2004/08/19 04:10:49 GMT: 64.37.149.6:1353]
   0: A1 F9 41 00 28 00 00 00 - FF FF FF FF FF FF FF FF  | ..A.(...........
  16: FF FF FF FF FF FF FF FF - 00 00 00 00 00 00 00 00  | ................
  32: 00 00 00 00 00 00 00 00 - 00 00 00 00 84 01 00 00  | ................

2004/08/19 04:10:49 GMT: OPCode 0x02fa [Raw OPCode: 0x02fa] [Size: 12]
 [66.90.221.245:3173 ->
2004/08/19 04:10:49 GMT: 64.37.149.6:1353]
   0: 08 0B 00 00 84 01 00 00 - F4 DE 12 00              | ............

ack tribute
 [64.37.149.6:1353 ->
2004/08/19 04:10:50 GMT: 66.90.221.245:3173]
2004/08/19 04:10:50 GMT:  Child Packet:
  [OPCode: 0x02f2] [Raw OPCode: 0x02f2] [Size: 48]
   0: 00 00 00 00 28 00 00 00 - FF FF FF FF FF FF FF FF  | ....(...........
  16: FF FF FF FF FF FF FF FF - 00 00 00 00 00 00 00 00  | ................
  32: 00 00 00 00 00 00 00 00 - 00 00 00 00 FF FF FF FF  | ................



Activate Tribute:

2004/08/19 04:11:05 GMT: OPCode 0x0365 [Raw OPCode: 0x0365] [Size: 4]
 [CLIENT:3173 ->
2004/08/19 04:11:05 GMT: SERVER:1353]
   0: 01 00 00 00                                        | ....


 [64.37.149.6:1353 ->
2004/08/19 04:11:05 GMT: 66.90.221.245:3173]
2004/08/19 04:11:05 GMT:  Child Packet:
  [OPCode: 0x02f8] [Raw OPCode: 0x02f8] [Size: 4]
   0: C0 27 09 00                                        | .'..

2004/08/19 04:11:05 GMT:  Child Packet:
  [OPCode: 0x02f4] [Raw OPCode: 0x02f4] [Size: 16]
   0: 0D 00 00 00 00 00 00 00 - 14 00 00 00 00 00 00 00  | ................

2004/08/19 04:11:05 GMT:  Child Packet:
  [OPCode: 0x02f2] [Raw OPCode: 0x02f2] [Size: 48]
   0: 01 00 00 00 28 00 00 00 - FF FF FF FF FF FF FF FF  | ....(...........
  16: FF FF FF FF FF FF FF FF - 00 00 00 00 00 00 00 00  | ................
  32: 00 00 00 00 00 00 00 00 - 00 00 00 00 FF FF FF FF  | ................

2004/08/19 04:11:05 GMT:  Child Packet:
  [OPCode: OP_ItemPacket] [Raw OPCode: OP_ItemPacket] [Size: 350]
   0: 6C 00 00 00 31 7C 30 7C - 34 30 30 7C 30 7C 30 7C  | l...1|0|400|0|0|
  16: 31 30 39 32 36 36 32 7C - 30 7C 2D 31 7C 22 30 7C  | 1092662|0|-1|"0|
  32: 52 65 73 69 73 74 20 44 - 69 73 65 61 73 65 20 35  | Resist Disease 5
  48: 20 42 65 6E 65 66 69 74 - 7C 52 65 73 69 73 74 20  |  Benefit|Resist
  64: 44 69 73 65 61 73 65 20 - 35 20 42 65 6E 65 66 69  | Disease 5 Benefi
  80: 74 7C 30 7C 35 36 34 36 - 35 7C 30 7C 32 35 35 7C  | t|0|56465|0|255|
  96: 32 35 35 7C 30 7C 30 7C - 31 7C 39 34 37 7C 2D 31  | 255|0|0|1|947|-1
 112: 7C 30 7C 31 7C 30 7C 30 - 7C 35 7C 30 7C 30 7C 30  | |0|1|0|0|5|0|0|0
 128: 7C 30 7C 30 7C 30 7C 30 - 7C 30 7C 30 7C 30 7C 30  | |0|0|0|0|0|0|0|0
 144: 7C 30 7C 30 7C 30 7C 30 - 7C 30 7C 30 7C 30 7C 30  | |0|0|0|0|0|0|0|0
 160: 7C 30 7C 30 7C 30 7C 30 - 7C 30 7C 30 7C 30 7C 30  | |0|0|0|0|0|0|0|0
 176: 7C 30 7C 30 7C 30 7C 30 - 7C 30 7C 30 7C 30 7C 30  | |0|0|0|0|0|0|0|0
 192: 7C 30 7C 30 7C 30 7C 2D - 31 7C 2D 31 7C 30 7C 30  | |0|0|0|-1|-1|0|0
 208: 7C 31 2E 30 30 30 30 30 - 30 7C 30 7C 30 7C 30 7C  | |1.000000|0|0|0|
 224: 30 7C 2D 31 7C 30 7C 30 - 7C 30 7C 30 7C 30 7C 30  | 0|-1|0|0|0|0|0|0
 240: 7C 30 7C 30 7C 30 7C 30 - 7C 30 7C 30 7C 30 7C 30  | |0|0|0|0|0|0|0|0
 256: 7C 30 7C 30 7C 30 7C 30 - 7C 7C 30 7C 30 7C 30 7C  | |0|0|0|0||0|0|0|
 272: 30 7C 30 7C 30 7C 30 7C - 30 7C 30 7C 30 7C 30 7C  | 0|0|0|0|0|0|0|0|
 288: 30 7C 30 7C 30 7C 30 7C - 7C 30 7C 30 7C 31 7C 30  | 0|0|0|0||0|0|1|0
 304: 7C 30 7C 30 7C 30 7C 30 - 7C 30 7C 30 7C 30 7C 30  | |0|0|0|0|0|0|0|0
 320: 7C 30 7C 30 7C 30 7C 30 - 7C 2D 31 7C 30 7C 30 7C  | |0|0|0|0|-1|0|0|
 336: 2D 31 22 7C 7C 7C 7C 7C - 7C 7C 7C 7C 7C 00        | -1"||||||||||.



Deactivate Tribute:


2004/08/19 04:11:08 GMT: OPCode 0x0365 [Raw OPCode: 0x0365] [Size: 4]
 [CLIENT:3173 ->
2004/08/19 04:11:08 GMT: SERVER:1353]
   0: 00 00 00 00                                        | ....

 [64.37.149.6:1353 ->
2004/08/19 04:11:09 GMT: 66.90.221.245:3173]
 [Packet is compressed]
 [Packet is encrypted]
 [Packet is packed]
2004/08/19 04:11:09 GMT:  Child Packet:
  [OPCode: 0x02f2] [Raw OPCode: 0x02f2] [Size: 48]
   0: 00 00 00 00 28 00 00 00 - FF FF FF FF FF FF FF FF  | ....(...........
  16: FF FF FF FF FF FF FF FF - 00 00 00 00 00 00 00 00  | ................
  32: 00 00 00 00 00 00 00 00 - 00 00 00 00 FF FF FF FF  | ................

2004/08/19 04:11:09 GMT:  Child Packet:
  [OPCode: 0x02f8] [Raw OPCode: 0x02f8] [Size: 4]
   0: C0 27 09 00                                        | .'..

*/




