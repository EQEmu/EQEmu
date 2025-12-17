/*	EQEMu: Everquest Server Emulator

	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#pragma once

#include "common/loot.h"
#include "common/repositories/saylink_repository.h"
#include "common/types.h"

#include <string>

struct LootItem;

namespace EQ
{
	struct ItemData;
	class ItemInstance;
	struct SayLinkBody_Struct;

	namespace saylink {
		enum SayLinkType {
			SayLinkBlank = 0,
			SayLinkItemData,
			SayLinkLootItem,
			SayLinkItemInst
		};

		extern bool DegenerateLinkBody(SayLinkBody_Struct& say_Link_body_struct, const std::string& say_link_body);
		extern bool GenerateLinkBody(std::string& say_link_body, const SayLinkBody_Struct& say_link_body_struct);

	} /*saylink*/

	struct SayLinkBody_Struct {
		uint8 action_id;		/* %1X */
		uint32 item_id;			/* %05X */
		uint32 augment_1;		/* %05X */
		uint32 augment_2;		/* %05X */
		uint32 augment_3;		/* %05X */
		uint32 augment_4;		/* %05X */
		uint32 augment_5;		/* %05X */
		uint32 augment_6;		/* %05X */
		uint8 is_evolving;		/* %1X */
		uint32 evolve_group;	/* %04X */
		uint8 evolve_level;		/* %02X */
		uint32 ornament_icon;	/* %05X */
		uint32 hash;			/* %08X */
	};

	struct SayLinkProxy_Struct : SayLinkBody_Struct {
		const char* text;
	};

	class SayLinkEngine {
		// TODO: consider methods for direct 'saylink' assignments
	public:
		SayLinkEngine();

		void SetLinkType(saylink::SayLinkType link_type) { m_LinkType = link_type; }
		void SetItemData(const EQ::ItemData* item_data) { m_ItemData = item_data; }
		void SetLootData(const LootItem* loot_data) { m_LootData = loot_data; }
		void SetItemInst(const ItemInstance* item_inst) { m_ItemInst = item_inst; }

		// mainly for saylinks..but, not limited to
		void SetProxyActionID(uint8 proxy_action_id) { m_LinkProxyStruct.action_id = proxy_action_id; } // should always be '0'
		void SetProxyItemID(uint32 proxy_item_id) { m_LinkProxyStruct.item_id = proxy_item_id; }
		void SetProxyAugment1ID(uint32 proxy_augment_id) { m_LinkProxyStruct.augment_1 = proxy_augment_id; }
		void SetProxyAugment2ID(uint32 proxy_augment_id) { m_LinkProxyStruct.augment_2 = proxy_augment_id; }
		void SetProxyAugment3ID(uint32 proxy_augment_id) { m_LinkProxyStruct.augment_3 = proxy_augment_id; }
		void SetProxyAugment4ID(uint32 proxy_augment_id) { m_LinkProxyStruct.augment_4 = proxy_augment_id; }
		void SetProxyAugment5ID(uint32 proxy_augment_id) { m_LinkProxyStruct.augment_5 = proxy_augment_id; }
		void SetProxyAugment6ID(uint32 proxy_augment_id) { m_LinkProxyStruct.augment_6 = proxy_augment_id; }
		void SetProxyIsEvolving(uint8 proxy_is_evolving) { m_LinkProxyStruct.is_evolving = proxy_is_evolving; }
		void SetProxyEvolveGroup(uint32 proxy_evolve_group) { m_LinkProxyStruct.evolve_group = proxy_evolve_group; }
		void SetProxyEvolveLevel(uint8 proxy_evolve_level) { m_LinkProxyStruct.evolve_level = proxy_evolve_level; }
		void SetProxyOrnamentIcon(uint32 proxy_ornament_icon) { m_LinkProxyStruct.ornament_icon = proxy_ornament_icon; }
		void SetProxyHash(uint32 proxy_hash) { m_LinkProxyStruct.hash = proxy_hash; }

		void SetProxyText(const char* proxy_text) { m_LinkProxyStruct.text = proxy_text; } // overrides standard text use
		void SetTaskUse() { m_TaskUse = true; }

		const std::string& GenerateLink();
		bool LinkError() { return m_Error; }

		const std::string& Link() { return m_Link; }			// contains full string format: '\x12' '<LinkBody>' '<LinkText>' '\x12'
		const std::string& LinkBody() { return m_LinkBody; }	// contains string format: '<LinkBody>'
		const std::string& LinkText() { return m_LinkText; }	// contains string format: '<LinkText>'

		static std::string GenerateQuestSaylink(const std::string& saylink_text, bool silent, const std::string& link_name);

		void Reset();

		static std::string InjectSaylinksIfNotExist(const char *message);
		static void LoadCachedSaylinks();
	private:
		void generate_body();
		void generate_text();

		int m_LinkType;
		const ItemData     * m_ItemData;
		const LootItem     * m_LootData;
		const ItemInstance * m_ItemInst;
		SayLinkBody_Struct m_LinkBodyStruct;
		SayLinkProxy_Struct m_LinkProxyStruct;
		bool m_TaskUse;
		std::string m_Link;
		std::string m_LinkBody;
		std::string m_LinkText;
		bool m_Error;
		static SaylinkRepository::Saylink GetOrSaveSaylink(std::string saylink_text);
	};

} /*EQEmu*/

class Saylink {
public:
	static std::string Create(const std::string &saylink_text, bool silent = false, const std::string &link_name = "");
	static std::string Silent(const std::string &saylink_text, const std::string &link_name = "");
};
