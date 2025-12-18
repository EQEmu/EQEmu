#include "common/repositories/npc_types_repository.h"
#include "zone/client.h"
#include "zone/groups.h"
#include "zone/raids.h"

void SendNPCEditSubCommands(Client *c)
{
	c->Message(Chat::White, "Usage: #npcedit name [Name] - Sets an NPC's Name");
	c->Message(Chat::White, "Usage: #npcedit lastname [Last Name] - Sets an NPC's Last Name");
	c->Message(Chat::White, "Usage: #npcedit level [Level] - Sets an NPC's Level");
	c->Message(Chat::White, "Usage: #npcedit race [Race ID] - Sets an NPC's Race");
	c->Message(Chat::White, "Usage: #npcedit class [Class ID] - Sets an NPC's Class");
	c->Message(Chat::White, "Usage: #npcedit bodytype [Body Type ID] - Sets an NPC's Bodytype");
	c->Message(Chat::White, "Usage: #npcedit hp [HP] - Sets an NPC's HP");
	c->Message(Chat::White, "Usage: #npcedit mana [Mana] - Sets an NPC's Mana");
	c->Message(Chat::White, "Usage: #npcedit gender [Gender ID] - Sets an NPC's Gender");
	c->Message(Chat::White, "Usage: #npcedit texture [Texture] - Sets an NPC's Texture");
	c->Message(Chat::White, "Usage: #npcedit helmtexture [Helmet Texture] - Sets an NPC's Helmet Texture");
	c->Message(Chat::White, "Usage: #npcedit herosforgemodel [Model Number] - Sets an NPC's Hero's Forge Model");
	c->Message(Chat::White, "Usage: #npcedit size [Size] - Sets an NPC's Size");
	c->Message(Chat::White, "Usage: #npcedit hpregen [HP Regen] - Sets an NPC's HP Regen Rate Per Tick");
	c->Message(Chat::White, "Usage: #npcedit hp_regen_per_second [HP Regen] - Sets an NPC's HP Regen Rate Per Second");
	c->Message(Chat::White, "Usage: #npcedit manaregen [Mana Regen] - Sets an NPC's Mana Regen Rate Per Tick");
	c->Message(Chat::White, "Usage: #npcedit loottable [Loottable ID] - Sets an NPC's Loottable ID");
	c->Message(Chat::White, "Usage: #npcedit merchantid [Merchant ID] - Sets an NPC's Merchant ID");
	c->Message(Chat::White, "Usage: #npcedit alt_currency_id [Alternate Currency ID] - Sets an NPC's Alternate Currency ID");
	c->Message(Chat::White, "Usage: #npcedit spell [Spell List ID] - Sets an NPC's Spells List ID");
	c->Message(Chat::White, "Usage: #npcedit npc_spells_effects_id [Spell Effects ID] - Sets an NPC's Spell Effects ID");
	c->Message(Chat::White, "Usage: #npcedit faction [Faction ID] - Sets an NPC's Faction ID");
	c->Message(Chat::White, "Usage: #npcedit adventure_template_id [Template ID] - Sets an NPC's Adventure Template ID");
	c->Message(Chat::White, "Usage: #npcedit trap_template [Template ID] - Sets an NPC's Trap Template ID");
	c->Message(Chat::White, "Usage: #npcedit damage [Minimum] [Maximum] - Sets an NPC's Damage");
	c->Message(Chat::White, "Usage: #npcedit attackcount [Attack Count] - Sets an NPC's Attack Count");
	c->Message(Chat::White, "Usage: #npcedit special_attacks [Special Attacks] - Sets an NPC's Special Attacks");
	c->Message(Chat::White, "Usage: #npcedit special_abilities [Special Abilities] - Sets an NPC's Special Abilities");
	c->Message(Chat::White, "Usage: #npcedit aggroradius [Radius] - Sets an NPC's Aggro Radius");
	c->Message(Chat::White, "Usage: #npcedit assistradius [Radius] - Sets an NPC's Assist Radius");
	c->Message(Chat::White, "Usage: #npcedit featuresave - Saves an NPC's current facial features to the database");
	c->Message(Chat::White, "Usage: #npcedit armortint_id [Armor Tint ID] - Sets an NPC's Armor Tint ID");
	c->Message(Chat::White, "Usage: #npcedit color [Red] [Green] [Blue] - Sets an NPC's Red, Green, and Blue armor tint");
	c->Message(Chat::White, "Usage: #npcedit ammoidfile [ID File] - Sets an NPC's Ammo ID File");
	c->Message(Chat::White, "Usage: #npcedit weapon [Primary Model] [Secondary Model] - Sets an NPC's Primary and Secondary Weapon Model");
	c->Message(Chat::White, "Usage: #npcedit meleetype [Primary Type] [Secondary Type] - Sets an NPC's Melee Skill Types");
	c->Message(Chat::White, "Usage: #npcedit rangedtype [Type] - Sets an NPC's Ranged Skill Type");
	c->Message(Chat::White, "Usage: #npcedit runspeed [Run Speed] - Sets an NPC's Run Speed");
	c->Message(Chat::White, "Usage: #npcedit mr [Resistance] - Sets an NPC's Magic Resistance");
	c->Message(Chat::White, "Usage: #npcedit pr [Resistance] - Sets an NPC's Poison Resistance");
	c->Message(Chat::White, "Usage: #npcedit dr [Resistance] - Sets an NPC's Disease Resistance");
	c->Message(Chat::White, "Usage: #npcedit fr [Resistance] - Sets an NPC's Fire Resistance");
	c->Message(Chat::White, "Usage: #npcedit cr [Resistance] - Sets an NPC's Cold Resistance");
	c->Message(Chat::White, "Usage: #npcedit corrup [Resistance] - Sets an NPC's Corruption Resistance");
	c->Message(Chat::White, "Usage: #npcedit phr [Resistance] - Sets and NPC's Physical Resistance");
	c->Message(Chat::White, "Usage: #npcedit seeinvis [Flag] - Sets an NPC's See Invisible Flag [0 = Cannot See Invisible, 1 = Can See Invisible]");
	c->Message(Chat::White, "Usage: #npcedit seeinvisundead [Flag] - Sets an NPC's See Invisible vs. Undead Flag  [0 = Cannot See Invisible vs. Undead, 1 = Can See Invisible vs. Undead]");
	c->Message(Chat::White, "Usage: #npcedit qglobal [Flag] - Sets an NPC's Quest Global Flag [0 = Quest Globals Off, 1 = Quest Globals On]");
	c->Message(Chat::White, "Usage: #npcedit ac [Armor Class] - Sets an NPC's Armor Class");
	c->Message(Chat::White, "Usage: #npcedit npcaggro [Flag] - Sets an NPC's NPC Aggro Flag [0 = Aggro NPCs Off, 1 = Aggro NPCs On]");
	c->Message(Chat::White, "Usage: #npcedit spawn_limit [Limit] - Sets an NPC's Spawn Limit Counter");
	c->Message(Chat::White, "Usage: #npcedit attackspeed [Attack Speed] - Sets an NPC's Attack Speed Modifier");
	c->Message(Chat::White, "Usage: #npcedit attackdelay [Attack Delay] - Sets an NPC's Attack Delay");
	c->Message(Chat::White, "Usage: #npcedit findable [Flag] - Sets an NPC's Findable Flag [0 = Not Findable, 1 = Findable]");
	c->Message(Chat::White, "Usage: #npcedit str [Strength] - Sets an NPC's Strength");
	c->Message(Chat::White, "Usage: #npcedit sta [Stamina] - Sets an NPC's Stamina");
	c->Message(Chat::White, "Usage: #npcedit agi [Agility] - Sets an NPC's Agility");
	c->Message(Chat::White, "Usage: #npcedit dex [Dexterity] - Sets an NPC's Dexterity");
	c->Message(Chat::White, "Usage: #npcedit int [Intelligence] - Sets an NPC's Intelligence");
	c->Message(Chat::White, "Usage: #npcedit wis [Wisdom] - Sets an NPC's Wisdom");
	c->Message(Chat::White, "Usage: #npcedit cha [Charisma] - Sets an NPC's Charisma");
	c->Message(Chat::White, "Usage: #npcedit seehide [Flag] - Sets an NPC's See Hide Flag [0 = Cannot See Hide, 1 = Can See Hide]");
	c->Message(Chat::White, "Usage: #npcedit seeimprovedhide [Flag] - Sets an NPC's See Improved Hide Flag [0 = Cannot See Improved Hide, 1 = Can See Improved Hide]");
	c->Message(Chat::White, "Usage: #npcedit trackable [Flag] - Sets an NPC's Trackable Flag [0 = Not Trackable, 1 = Trackable]");
	c->Message(Chat::White, "Usage: #npcedit atk [Attack] - Sets an NPC's Attack");
	c->Message(Chat::White, "Usage: #npcedit accuracy [Accuracy] - Sets an NPC's Accuracy");
	c->Message(Chat::White, "Usage: #npcedit avoidance [Avoidance] - Sets an NPC's Avoidance");
	c->Message(Chat::White, "Usage: #npcedit slow_mitigation [Slow Mitigation] - Sets an NPC's Slow Mitigation");
	c->Message(Chat::White, "Usage: #npcedit version [Version] - Sets an NPC's Version");
	c->Message(Chat::White, "Usage: #npcedit maxlevel [Max Level] - Sets an NPC's Maximum Level");
	c->Message(Chat::White, "Usage: #npcedit scalerate [Scale Rate] - Sets an NPC's Scaling Rate [50 = 50%, 100 = 100%, 200 = 200%]");
	c->Message(Chat::White, "Usage: #npcedit emoteid [Emote ID] - Sets an NPC's Emote ID");
	c->Message(Chat::White, "Usage: #npcedit spellscale [Scale Rate] - Sets an NPC's Spell Scaling Rate [50 = 50%, 100 = 100%, 200 = 200%]");
	c->Message(Chat::White, "Usage: #npcedit healscale [Scale Rate] - Sets an NPC's Heal Scaling Rate [50 = 50%, 100 = 100%, 200 = 200%]");
	c->Message(Chat::White, "Usage: #npcedit no_target [Flag] - Sets an NPC's No Target Hotkey Flag [0 = Not Targetable with Target Hotkey, 1 = Targetable with Target Hotkey]");
	c->Message(Chat::White, "Usage: #npcedit raidtarget [Flag] - Sets an NPC's Raid Target Flag [0 = Not a Raid Target, 1 = Raid Target]");
	c->Message(Chat::White, "Usage: #npcedit armtexture [Texture] - Sets an NPC's Arm Texture");
	c->Message(Chat::White, "Usage: #npcedit bracertexture [Texture] - Sets an NPC's Bracer Texture");
	c->Message(Chat::White, "Usage: #npcedit handtexture [Texture] - Sets an NPC's Hand Texture");
	c->Message(Chat::White, "Usage: #npcedit legtexture [Texture] - Sets an NPC's Leg Texture");
	c->Message(Chat::White, "Usage: #npcedit feettexture [Texture] - Sets an NPC's Feet Texture");
	c->Message(Chat::White, "Usage: #npcedit walkspeed [Walk Speed] - Sets an NPC's Walk Speed");
	c->Message(Chat::White, "Usage: #npcedit show_name [Flag] - Sets an NPC's Show Name Flag [0 = Hidden, 1 = Shown]");
	c->Message(Chat::White, "Usage: #npcedit untargetable [Flag] - Sets an NPC's Untargetable Flag [0 = Targetable, 1 = Untargetable]");
	c->Message(Chat::White, "Usage: #npcedit charm_ac [Armor Class] - Sets an NPC's Armor Class while Charmed");
	c->Message(Chat::White, "Usage: #npcedit charm_min_dmg [Damage] - Sets an NPC's Minimum Damage while Charmed");
	c->Message(Chat::White, "Usage: #npcedit charm_max_dmg [Damage] - Sets an NPC's Maximum Damage while Charmed");
	c->Message(Chat::White, "Usage: #npcedit charm_attack_delay [Attack Delay] - Sets an NPC's Attack Delay while Charmed");
	c->Message(Chat::White, "Usage: #npcedit charm_accuracy_rating [Accuracy] - Sets an NPC's Accuracy Rating while Charmed");
	c->Message(Chat::White, "Usage: #npcedit charm_avoidance_rating [Avoidance] - Sets an NPC's Avoidance Rating while Charmed");
	c->Message(Chat::White, "Usage: #npcedit charm_atk [Attack] - Sets an NPC's Attack while Charmed");
	c->Message(Chat::White, "Usage: #npcedit skip_global_loot [Flag] - Sets an NPC's Skip Global Loot Flag [0 = Don't Skip, 1 = Skip");
	c->Message(Chat::White, "Usage: #npcedit rarespawn [Flag] - Sets an NPC's Rare Spawn Flag [0 = Not a Rare Spawn, 1 = Rare Spawn]");
	c->Message(Chat::White, "Usage: #npcedit stuck_behavior [Stuck Behavior] - Sets an NPC's Stuck Behavior [0 = Run to Target, 1 = Warp to Target, 2 = Take No Action, 3 = Evade Combat]");
	c->Message(Chat::White, "Usage: #npcedit model [Race ID] - Sets an NPC's Race Model");
	c->Message(Chat::White, "Usage: #npcedit flymode [Fly Mode] - Sets an NPC's Fly Mode [0 = Ground, 1 = Flying, 2 = Levitating, 3 = Water, 4 = Floating, 5 = Levitating While Running]");
	c->Message(Chat::White, "Usage: #npcedit always_aggro [Flag] - Sets an NPC's Always Aggro Flag [0 = Does not Always Aggro, 1 = Always Aggro]");
	c->Message(Chat::White, "Usage: #npcedit exp_mod [Modifier] - Sets an NPC's Experience Modifier [50 = 50%, 100 = 100%, 200 = 200%]");
	c->Message(Chat::White, "Usage: #npcedit heroic_strikethrough [Heroic Strikethrough] - Sets an NPC's Heroic Strikethrough");
	c->Message(Chat::White, "Usage: #npcedit faction_amount [Faction Amount] - Sets an NPC's Faction Amount");
	c->Message(Chat::White, "Usage: #npcedit keeps_sold_items [Flag] - Sets an NPC's Keeps Sold Items Flag [0 = False, 1 = True]");
	c->Message(Chat::White, "Usage: #npcedit is_parcel_merchant [Flag] - Sets an NPC's Parcel Merchant Flag [0 = False, 1 = True]");
	c->Message(Chat::White, "Usage: #npcedit setanimation [Animation ID] - Sets an NPC's Animation on Spawn (Stored in spawn2 table)");
	c->Message(Chat::White, "Usage: #npcedit respawntime [Respawn Time] - Sets an NPC's Respawn Timer in Seconds (Stored in spawn2 table)");
	c->Message(Chat::White, "Usage: #npcedit set_grid [Grid ID] - Sets an NPC's Grid ID");
	c->Message(Chat::White, "Usage: #npcedit npc_tint_id [NPC Tint ID] - Sets an NPC's Tint ID (0 to 78 for RoF2)");
}

void command_npcedit(Client *c, const Seperator *sep)
{
	if (!c->GetTarget() || !c->GetTarget()->IsNPC()) {
		c->Message(Chat::White, "You must target an NPC to use this command.");
		return;
	}

	int arguments = sep->argnum;

	bool is_help = (!arguments || !strcasecmp(sep->arg[1], "help"));

	if (is_help) {
		SendNPCEditSubCommands(c);
		return;
	}

	std::string sub_command = sep->arg[1];

	auto t             = c->GetTarget()->CastToNPC();
	auto npc_id        = t->GetNPCTypeID();
	auto npc_id_string = fmt::format(
		"NPC ID {}",
		npc_id
	);

	auto n = NpcTypesRepository::FindOne(content_db, npc_id);
	if (!n.id) {
		c->Message(Chat::White, "You must target a valid NPC to use this command.");
		return;
	}

	std::string d;

	if (!strcasecmp(sep->arg[1], "name")) {
		std::string name = sep->argplus[2];
		if (!name.empty()) {
			n.name = name;
			d = fmt::format(
				"{} is now named '{}'.",
				npc_id_string,
				name
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit name [Name] - Sets an NPC's Name");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "lastname")) {
		std::string last_name = sep->argplus[2];
		n.lastname = last_name;
		d = fmt::format(
			"{} now has the lastname '{}'.",
			npc_id_string,
			last_name
		);
	} else if (!strcasecmp(sep->arg[1], "level")) {
		if (sep->IsNumber(2)) {
			auto level = static_cast<uint8_t>(Strings::ToUnsignedInt(sep->arg[2]));
			n.level = level;
			d = fmt::format(
				"{} is now level {}.",
				npc_id_string,
				level
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit level [Level] - Sets an NPC's Level");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "race")) {
		if (sep->IsNumber(2)) {
			auto race_id = static_cast<uint16_t>(Strings::ToUnsignedInt(sep->arg[2]));
			n.race = race_id;
			d = fmt::format(
				"{} is now a(n) {} ({}).",
				npc_id_string,
				GetRaceIDName(race_id),
				race_id
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit race [Race ID] - Sets an NPC's Race");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "class")) {
		if (sep->IsNumber(2)) {
			auto class_id = static_cast<uint8_t>(Strings::ToUnsignedInt(sep->arg[2]));
			n.class_ = class_id;
			d = fmt::format(
				"{} is now a(n) {} ({}).",
				npc_id_string,
				GetClassIDName(class_id),
				class_id
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit class [Class ID] - Sets an NPC's Class");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "bodytype")) {
		if (sep->IsNumber(2)) {
			const uint8 body_type_id = static_cast<uint8>(Strings::ToUnsignedInt(sep->arg[2]));
			const std::string& body_type_name = BodyType::GetName(body_type_id);
			if (Strings::EqualFold(body_type_name, "UNKNOWN BODY TYPE")) {
				c->Message(
					Chat::White,
					fmt::format(
						"Body Type {} does not exist.",
						body_type_id
					).c_str()
				);
				return;
			}

			n.bodytype = body_type_id;
			d = fmt::format(
				"{} is now using Body Type {} ({}).",
				npc_id_string,
				body_type_name,
				body_type_id
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit bodytype [Body Type ID] - Sets an NPC's Bodytype");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "hp")) {
		if (sep->IsNumber(2)) {
			auto hp = Strings::ToBigInt(sep->arg[2]);
			n.hp = hp;
			d = fmt::format(
				"{} now has {} Health.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit hp [HP] - Sets an NPC's HP");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "mana")) {
		if (sep->IsNumber(2)) {
			auto mana = Strings::ToBigInt(sep->arg[2]);
			n.mana = mana;
			d = fmt::format(
				"{} now has {} Mana.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit mana [Mana] - Sets an NPC's Mana");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "gender")) {
		if (sep->IsNumber(2)) {
			auto gender_id = static_cast<uint8_t>(Strings::ToUnsignedInt(sep->arg[2]));
			n.gender = gender_id;
			d = fmt::format(
				"{} is now a {} ({}).",
				npc_id_string,
				gender_id,
				GetGenderName(gender_id)
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit gender [Gender ID] - Sets an NPC's Gender");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "texture")) {
		if (sep->IsNumber(2)) {
			auto texture = static_cast<uint8_t>(Strings::ToUnsignedInt(sep->arg[2]));
			n.texture = texture;
			d = fmt::format(
				"{} is now using Texture {}.",
				npc_id_string,
				texture
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit texture [Texture] - Sets an NPC's Texture");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "helmtexture")) {
		if (sep->IsNumber(2)) {
			auto helmet_texture = static_cast<uint8_t>(Strings::ToUnsignedInt(sep->arg[2]));
			n.helmtexture = helmet_texture;
			d = fmt::format(
				"{} is now using Helmet Texture {}.",
				npc_id_string,
				helmet_texture
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit helmtexture [Helmet Texture] - Sets an NPC's Helmet Texture");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "herosforgemodel")) {
		if (sep->IsNumber(2)) {
			auto heros_forge_model = Strings::ToInt(sep->arg[2]);
			n.herosforgemodel = heros_forge_model;
			d = fmt::format(
				"{} is now using Hero's Forge Model {}.",
				npc_id_string,
				heros_forge_model
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit herosforgemodel [Model Number] - Sets an NPC's Hero's Forge Model"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "size")) {
		if (sep->IsNumber(2)) {
			auto size = Strings::ToFloat(sep->arg[2]);
			n.size = size;
			d = fmt::format(
				"{} is now Size {:.2f}.",
				npc_id_string,
				size
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit size [Size] - Sets an NPC's Size");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "hpregen")) {
		if (sep->IsNumber(2)) {
			auto hp_regen = Strings::ToBigInt(sep->arg[2]);
			n.hp_regen_rate = hp_regen;
			d = fmt::format(
				"{} now regenerates {} Health per Tick.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit hpregen [HP Regen] - Sets an NPC's HP Regen Rate Per Tick");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "hp_regen_per_second")) {
		if (sep->IsNumber(2)) {
			auto hp_regen_per_second = Strings::ToBigInt(sep->arg[2]);
			n.hp_regen_per_second = hp_regen_per_second;
			d = fmt::format(
				"{} now regenerates {} HP per Second.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit hp_regen_per_second [HP Regen] - Sets an NPC's HP Regen Rate Per Second"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "manaregen")) {
		if (sep->IsNumber(2)) {
			auto mana_regen = Strings::ToBigInt(sep->arg[2]);
			n.mana_regen_rate = mana_regen;
			d = fmt::format(
				"{} now regenerates {} Mana per Tick.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit manaregen [Mana Regen] - Sets an NPC's Mana Regen Rate Per Tick");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "loottable")) {
		if (sep->IsNumber(2)) {
			auto loottable_id = Strings::ToUnsignedInt(sep->arg[2]);
			n.loottable_id = loottable_id;
			d = fmt::format(
				"{} is now using Loottable ID {}.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit loottable [Loottable ID] - Sets an NPC's Loottable ID");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "merchantid")) {
		if (sep->IsNumber(2)) {
			auto merchant_id = Strings::ToUnsignedInt(sep->arg[2]);
			n.merchant_id = merchant_id;
			d = fmt::format(
				"{} is now using Merchant ID {}.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit merchantid [Merchant ID] - Sets an NPC's Merchant ID");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "alt_currency_id")) {
		if (sep->IsNumber(2)) {
			auto alternate_currency_id      = Strings::ToUnsignedInt(sep->arg[2]);
			auto alternate_currency_item_id = zone->GetCurrencyItemID(alternate_currency_id);
			n.alt_currency_id = alternate_currency_id;
			d = fmt::format(
				"{} is now using Alternate Currency {}.",
				npc_id_string,
				(
					alternate_currency_item_id ?
					fmt::format(
						"{} ({})",
						database.CreateItemLink(alternate_currency_item_id),
						alternate_currency_id
					) :
					std::to_string(alternate_currency_id)
				)
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit alt_currency_id [Alternate Currency ID] - Sets an NPC's Alternate Currency ID"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "spell")) {
		if (sep->IsNumber(2)) {
			auto spell_list_id = Strings::ToUnsignedInt(sep->arg[2]);
			n.npc_spells_id = spell_list_id;
			d = fmt::format(
				"{} is now using Spell List ID {}.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit spell [Spell List ID] - Sets an NPC's Spells List ID");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "npc_spells_effects_id")) {
		if (sep->IsNumber(2)) {
			auto spell_effects_id = Strings::ToUnsignedInt(sep->arg[2]);
			n.npc_spells_effects_id = spell_effects_id;
			d = fmt::format(
				"{} is now using Spells Effects ID {}.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit npc_spells_effects_id [Spell Effects ID] - Sets an NPC's Spell Effects ID"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "faction")) {
		if (sep->IsNumber(2)) {
			auto npc_faction_id   = Strings::ToInt(sep->arg[2]);
			const auto f = zone->GetNPCFaction(npc_faction_id);
			if (f) {
				auto faction_id = f->primaryfaction;
				auto faction_name = content_db.GetFactionName(faction_id);

				n.npc_faction_id = npc_faction_id;
				d = fmt::format(
					"{} is now using Faction {}.",
					npc_id_string,
					(
						!faction_name.empty() ?
						fmt::format(
							"{} ({})",
							faction_name,
							faction_id
						) :
						Strings::Commify(sep->arg[2])
					)
				);
			}
			else {
				c->Message(Chat::White, "Need to provide a valid, existing, npc_faction_id");
				return;
			}
		} else {
			c->Message(Chat::White, "Usage: #npcedit faction [npc_faction_id] - Sets an NPC's npc Faction ID (not primary faction) but lookup into table.");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "adventure_template_id")) {
		if (sep->IsNumber(2)) {
			auto adventure_template_id = Strings::ToUnsignedInt(sep->arg[2]);
			n.adventure_template_id = adventure_template_id;
			d = fmt::format(
				"{} is now using Adventure Template ID {}.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit adventure_template_id [Template ID] - Sets an NPC's Adventure Template ID"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "trap_template")) {
		if (sep->IsNumber(2)) {
			auto trap_template = Strings::ToUnsignedInt(sep->arg[2]);
			n.trap_template = trap_template;
			d = fmt::format(
				"{} is now using Trap Template ID {}.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit trap_template [Template ID] - Sets an NPC's Trap Template ID");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "damage")) {
		if (sep->IsNumber(2) && sep->IsNumber(3)) {
			auto minimum_damage = Strings::ToUnsignedInt(sep->arg[2]);
			auto maximum_damage = Strings::ToUnsignedInt(sep->arg[3]);
			n.mindmg = minimum_damage;
			n.maxdmg = maximum_damage;
			d = fmt::format(
				"{} now hits from {} to {} damage.",
				npc_id_string,
				Strings::Commify(sep->arg[2]),
				Strings::Commify(sep->arg[3])
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit damage [Minimum] [Maximum] - Sets an NPC's Damage");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "attackcount")) {
		if (sep->IsNumber(2)) {
			auto attack_count = static_cast<int16>(Strings::ToInt(sep->arg[2]));
			n.attack_count = attack_count;
			d = fmt::format(
				"{} now has an Attack Count of {}.",
				npc_id_string,
				attack_count
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit attackcount [Attack Count] - Sets an NPC's Attack Count");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "special_attacks")) {
		std::string special_attacks = sep->argplus[2];
		n.npcspecialattks = special_attacks;
		d =  fmt::format(
			"{} is now using the following Special Attacks '{}'.",
			npc_id_string,
			special_attacks
		);
	} else if (!strcasecmp(sep->arg[1], "special_abilities")) {
		std::string special_abilities = sep->argplus[2];
		n.special_abilities = special_abilities;
		d = fmt::format(
			"{} is now using the following Special Abilities '{}'.",
			npc_id_string,
			special_abilities
		);
	} else if (!strcasecmp(sep->arg[1], "aggroradius")) {
		if (sep->IsNumber(2)) {
			auto aggro_radius = Strings::ToUnsignedInt(sep->arg[2]);
			n.aggroradius = aggro_radius;
			d = fmt::format(
				"{} now has an Aggro Radius of {}.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit aggroradius [Radius] - Sets an NPC's Aggro Radius");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "assistradius")) {
		if (sep->IsNumber(2)) {
			auto assist_radius = Strings::ToUnsignedInt(sep->arg[2]);
			n.assistradius = assist_radius;
			d = fmt::format(
				"{} now has an Assist Radius of {}",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit assistradius [Radius] - Sets an NPC's Assist Radius");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "featuresave")) {
		d = fmt::format(
			"{} saved with all current body and facial feature settings.",
			npc_id
		);

		n.gender = t->GetGender();
		n.texture = t->GetTexture();
		n.helmtexture = t->GetHelmTexture();
		n.size = t->GetSize();
		n.face = t->GetLuclinFace();
		n.luclin_hairstyle = t->GetHairStyle();
		n.luclin_haircolor = t->GetHairColor();
		n.luclin_eyecolor = t->GetEyeColor1();
		n.luclin_eyecolor2 = t->GetEyeColor2();
		n.luclin_beardcolor = t->GetBeardColor();
		n.luclin_beard = t->GetBeard();
		n.drakkin_heritage = t->GetDrakkinHeritage();
		n.drakkin_tattoo = t->GetDrakkinTattoo();
		n.drakkin_details = t->GetDrakkinDetails();
	} else if (!strcasecmp(sep->arg[1], "armortint_id")) {
		if (sep->IsNumber(2)) {
			auto armor_tint_id = Strings::ToUnsignedInt(sep->arg[2]);
			n.armortint_id = armor_tint_id;
			d = fmt::format(
				"{} is now using Armor Tint ID {}.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit armortint_id [Armor Tint ID] - Sets an NPC's Armor Tint ID");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "color")) {
		if (sep->IsNumber(2)) {
			auto  red   = static_cast<uint8_t>(Strings::ToUnsignedInt(sep->arg[2]));
			uint8_t green = sep->IsNumber(3) ? static_cast<uint8_t>(Strings::ToUnsignedInt(sep->arg[3])) : 0;
			uint8_t blue  = sep->IsNumber(4) ? static_cast<uint8_t>(Strings::ToUnsignedInt(sep->arg[4])) : 0;
			n.armortint_red = red;
			n.armortint_green = green;
			n.armortint_blue = blue;
			d = fmt::format(
				"{} now has {} Red, {} Green, and {} Blue tinting on their armor.",
				npc_id_string,
				red,
				green,
				blue
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit color [Red] [Green] [Blue] - Sets an NPC's Red, Green, and Blue armor tint"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "ammoidfile")) {
		if (sep->IsNumber(2)) {
			auto ammo_id_file = Strings::ToUnsignedInt(sep->arg[2]);
			n.ammo_idfile = ammo_id_file;
			d = fmt::format(
				"{} is now using Ammo ID File {}.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit ammoidfile [ID File] - Sets an NPC's Ammo ID File");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "weapon")) {
		if (sep->IsNumber(2)) {
			auto     primary_model   = Strings::ToUnsignedInt(sep->arg[2]);
			uint32_t secondary_model = sep->IsNumber(3) ? Strings::ToUnsignedInt(sep->arg[3]) : 0;
			n.d_melee_texture1 = primary_model;
			n.d_melee_texture2 = secondary_model;
			d = fmt::format(
				"{} will have Model {} set to their Primary and Model {} set to their Secondary on repop.",
				npc_id_string,
				Strings::Commify(primary_model),
				Strings::Commify(secondary_model)
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit weapon [Primary Model] [Secondary Model] - Sets an NPC's Primary and Secondary Weapon Model"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "meleetype")) {
		if (sep->IsNumber(2)) {
			auto     primary_type   = Strings::ToUnsignedInt(sep->arg[2]);
			uint32_t secondary_type = sep->IsNumber(3) ? Strings::ToUnsignedInt(sep->arg[3]) : EQ::skills::SkillHandtoHand;

			auto primary_skill   = EQ::skills::GetSkillName(static_cast<EQ::skills::SkillType>(primary_type));
			auto secondary_skill = EQ::skills::GetSkillName(static_cast<EQ::skills::SkillType>(secondary_type));

			n.prim_melee_type = primary_type;
			n.sec_melee_type = secondary_type;

			d = fmt::format(
				"{} now has a Primary Melee Type of {} and a Secondary Melee Type of {}.",
				npc_id_string,
				(
					!primary_skill.empty() ?
					fmt::format(
						"{} ({})",
						primary_skill,
						primary_type
					) :
					std::to_string(primary_type)
				),
				(
					!secondary_skill.empty() ?
					fmt::format(
						"{} ({})",
						secondary_skill,
						secondary_type
					) :
					std::to_string(secondary_type)
				)
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit meleetype [Primary Type] [Secondary Type] - Sets an NPC's Melee Skill Types"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "rangedtype")) {
		if (sep->IsNumber(2)) {
			auto ranged_type = Strings::ToUnsignedInt(sep->arg[2]);

			auto ranged_skill = EQ::skills::GetSkillName(static_cast<EQ::skills::SkillType>(ranged_type));

			n.ranged_type = ranged_type;

			d = fmt::format(
				"{} now has a Ranged Type of {}.",
				npc_id_string,
				(
					!ranged_skill.empty() ?
					fmt::format(
						"{} ({})",
						ranged_skill,
						ranged_type
					) :
					std::to_string(ranged_type)
				)
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit rangedtype [Type] - Sets an NPC's Ranged Skill Type");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "runspeed")) {
		if (sep->IsNumber(2)) {
			auto run_speed = Strings::ToFloat(sep->arg[2]);
			n.runspeed = run_speed;
			d = fmt::format(
				"{} now runs at a Run Speed of {:.2f}.",
				npc_id_string,
				run_speed
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit runspeed [Run Speed] - Sets an NPC's Run Speed");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "mr")) {
		if (sep->IsNumber(2)) {
			auto magic_resist = static_cast<int16_t>(Strings::ToUnsignedInt(sep->arg[2]));
			n.MR = magic_resist;
			d = fmt::format(
				"{} now has a Magic Resistance of {}.",
				npc_id_string,
				magic_resist
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit mr [Resistance] - Sets an NPC's Magic Resistance");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "pr")) {
		if (sep->IsNumber(2)) {
			auto poison_resist = static_cast<int16_t>(Strings::ToUnsignedInt(sep->arg[2]));
			n.PR = poison_resist;
			d = fmt::format(
				"{} now has a Poison Resistance of {}.",
				npc_id_string,
				poison_resist
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit pr [Resistance] - Sets an NPC's Poison Resistance");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "dr")) {
		if (sep->IsNumber(2)) {
			auto disease_resist = static_cast<int16_t>(Strings::ToUnsignedInt(sep->arg[2]));
			n.DR = disease_resist;
			d = fmt::format(
				"{} now has a Disease Resistance of {}.",
				npc_id_string,
				disease_resist
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit dr [Resistance] - Sets an NPC's Disease Resistance");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "fr")) {
		if (sep->IsNumber(2)) {
			auto fire_resist = static_cast<int16_t>(Strings::ToUnsignedInt(sep->arg[2]));
			n.FR = fire_resist;
			d = fmt::format(
				"{} now has a Fire Resistance of {}.",
				npc_id_string,
				fire_resist
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit fr [Resistance] - Sets an NPC's Fire Resistance");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "cr")) {
		if (sep->IsNumber(2)) {
			auto cold_resist = static_cast<int16_t>(Strings::ToUnsignedInt(sep->arg[2]));
			n.CR = cold_resist;
			d = fmt::format(
				"{} now has a Cold Resistance of {}.",
				npc_id_string,
				cold_resist
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit cr [Resistance] - Sets an NPC's Cold Resistance");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "corrup")) {
		if (sep->IsNumber(2)) {
			auto corruption_resist = static_cast<int16_t>(Strings::ToUnsignedInt(sep->arg[2]));
			n.Corrup = corruption_resist;
			d = fmt::format(
				"{} now has a Corruption Resistance of {}.",
				npc_id_string,
				corruption_resist
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit corrup [Resistance] - Sets an NPC's Corruption Resistance");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "phr")) {
		if (sep->IsNumber(2)) {
			auto physical_resist = static_cast<uint16_t>(Strings::ToUnsignedInt(sep->arg[2]));
			n.PhR = physical_resist;
			d = fmt::format(
				"{} now has a Physical Resistance of {}.",
				npc_id_string,
				physical_resist
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit phr [Resistance] - Sets and NPC's Physical Resistance");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "seeinvis")) {
		if (sep->IsNumber(2)) {
			auto see_invisible = static_cast<int16_t>(Strings::ToUnsignedInt(sep->arg[2]));
			n.see_invis = see_invisible;
			d = fmt::format(
				"{} can {} See Invisible.",
				npc_id_string,
				see_invisible ? "now" : "no longer"
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit seeinvis [Flag] - Sets an NPC's See Invisible Flag [0 = Cannot See Invisible, 1 = Can See Invisible]"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "seeinvisundead")) {
		if (sep->IsNumber(2)) {
			auto see_invisible_undead = static_cast<int16_t>(Strings::ToUnsignedInt(sep->arg[2]));
			n.see_invis_undead = see_invisible_undead;
			d = fmt::format(
				"{} can {} See Invisible vs. Undead.",
				npc_id_string,
				see_invisible_undead ? "now" : "no longer"
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit seeinvisundead [Flag] - Sets an NPC's See Invisible vs. Undead Flag  [0 = Cannot See Invisible vs. Undead, 1 = Can See Invisible vs. Undead]"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "qglobal")) {
		if (sep->IsNumber(2)) {
			auto use_qglobals = Strings::ToUnsignedInt(sep->arg[2]);
			n.qglobal = use_qglobals;
			d = fmt::format(
				"{} can {} use Quest Globals.",
				npc_id_string,
				use_qglobals ? "now" : "no longer"
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit qglobal [Flag] - Sets an NPC's Quest Global Flag [0 = Quest Globals Off, 1 = Quest Globals On]"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "ac")) {
		if (sep->IsNumber(2)) {
			auto armor_class = static_cast<int16_t>(Strings::ToUnsignedInt(sep->arg[2]));
			n.AC = armor_class;
			d = fmt::format(
				"{} now has {} Armor Class.",
				npc_id_string,
				armor_class
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit ac [Armor Class] - Sets an NPC's Armor Class");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "npcaggro")) {
		if (sep->IsNumber(2)) {
			auto aggro_npcs = static_cast<int8_t>(Strings::ToUnsignedInt(sep->arg[2]));
			n.npc_aggro = aggro_npcs;
			d = fmt::format(
				"{} will {} aggro other NPCs that have a hostile faction.",
				npc_id_string,
				aggro_npcs ? "now" : "no longer"
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit npcaggro [Flag] - Sets an NPC's NPC Aggro Flag [0 = Aggro NPCs Off, 1 = Aggro NPCs On]"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "spawn_limit")) {
		if (sep->IsNumber(2)) {
			auto spawn_limit = static_cast<int8_t>(Strings::ToUnsignedInt(sep->arg[2]));
			n.spawn_limit = spawn_limit;
			d = fmt::format(
				"{} now has a Spawn Limit of {}.",
				npc_id_string,
				spawn_limit
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit spawn_limit [Limit] - Sets an NPC's Spawn Limit Counter");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "attackspeed")) {
		if (sep->IsNumber(2)) {
			auto attack_speed = Strings::ToFloat(sep->arg[2]);
			n.attack_speed = attack_speed;
			d = fmt::format(
				"{} now has an Attack Speed of {:.2f}.",
				npc_id_string,
				attack_speed
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit attackspeed [Attack Speed] - Sets an NPC's Attack Speed Modifier");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "attackdelay")) {
		if (sep->IsNumber(2)) {
			auto attack_delay = static_cast<uint8_t>(Strings::ToUnsignedInt(sep->arg[2]));
			n.attack_delay = attack_delay;
			d = fmt::format(
				"{} now has an Attack Delay of {}.",
				npc_id_string,
				attack_delay
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit attackdelay [Attack Delay] - Sets an NPC's Attack Delay");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "findable")) {
		if (sep->IsNumber(2)) {
			auto is_findable = static_cast<int8_t>(Strings::ToUnsignedInt(sep->arg[2]));
			n.findable = is_findable;
			d = fmt::format(
				"{} is {} Findable.",
				npc_id_string,
				is_findable ? "now" : "no longer"
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit findable [Flag] - Sets an NPC's Findable Flag [0 = Not Findable, 1 = Findable]"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "str")) {
		if (sep->IsNumber(2)) {
			auto strength = Strings::ToUnsignedInt(sep->arg[2]);
			n.STR = strength;
			d = fmt::format(
				"{} now has {} Strength.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit str [Strength] - Sets an NPC's Strength");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "sta")) {
		if (sep->IsNumber(2)) {
			auto stamina = Strings::ToUnsignedInt(sep->arg[2]);
			n.STA = stamina;
			d = fmt::format(
				"{} now has {} Stamina.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit sta [Stamina] - Sets an NPC's Stamina");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "agi")) {
		if (sep->IsNumber(2)) {
			auto agility = Strings::ToUnsignedInt(sep->arg[2]);
			n.AGI = agility;
			d = fmt::format(
				"{} now has {} Agility.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit agi [Agility] - Sets an NPC's Agility");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "dex")) {
		if (sep->IsNumber(2)) {
			auto dexterity = Strings::ToUnsignedInt(sep->arg[2]);
			n.DEX = dexterity;
			d = fmt::format(
				"{} now has {} Dexterity.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit dex [Dexterity] - Sets an NPC's Dexterity");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "int")) {
		if (sep->IsNumber(2)) {
			auto intelligence = Strings::ToUnsignedInt(sep->arg[2]);
			n._INT = intelligence;
			d = fmt::format(
				"{} now has {} Intelligence.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit int [Intelligence] - Sets an NPC's Intelligence");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "wis")) {
		if (sep->IsNumber(2)) {
			auto wisdom = Strings::ToUnsignedInt(sep->arg[2]);
			n.WIS = wisdom;
			d = fmt::format(
				"{} now has {} Wisdom.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit wis [Wisdom] - Sets an NPC's Wisdom");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "cha")) {
		if (sep->IsNumber(2)) {
			auto charisma = Strings::ToUnsignedInt(sep->arg[2]);
			n.CHA = charisma;
			d = fmt::format(
				"{} now has {} Charisma.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit cha [Charisma] - Sets an NPC's Charisma");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "seehide")) {
		if (sep->IsNumber(2)) {
			auto see_hide = static_cast<int8_t>(Strings::ToInt(sep->arg[2]));
			n.see_hide = see_hide;
			d = fmt::format(
				"{} can {} See Hide.",
				npc_id_string,
				see_hide ? "now" : "no longer"
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit seehide [Flag] - Sets an NPC's See Hide Flag [0 = Cannot See Hide, 1 = Can See Hide]"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "seeimprovedhide")) {
		if (sep->IsNumber(2)) {
			auto see_improved_hide = static_cast<int8>(Strings::ToInt(sep->arg[2]));
			n.see_improved_hide = see_improved_hide;
			d = fmt::format(
				"{} can {} See Improved Hide.",
				npc_id_string,
				see_improved_hide ? "now" : "no longer"
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit seeimprovedhide [Flag] - Sets an NPC's See Improved Hide Flag [0 = Cannot See Improved Hide, 1 = Can See Improved Hide]"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "trackable")) {
		if (sep->IsNumber(2)) {
			auto is_trackable = static_cast<int8_t>(Strings::ToInt(sep->arg[2]));
			n.trackable = is_trackable;
			d = fmt::format(
				"{} is {} Trackable.",
				npc_id_string,
				is_trackable ? "now" : "no longer"
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit trackable [Flag] - Sets an NPC's Trackable Flag [0 = Not Trackable, 1 = Trackable]"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "atk")) {
		if (sep->IsNumber(2)) {
			auto attack = Strings::ToInt(sep->arg[2]);
			n.ATK = attack;
			d = fmt::format(
				"{} now has {} Attack.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit atk [Attack] - Sets an NPC's Attack");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "accuracy")) {
		if (sep->IsNumber(2)) {
			auto accuracy = Strings::ToInt(sep->arg[2]);
			n.Accuracy = accuracy;
			d = fmt::format(
				"{} now has {} Accuracy.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit accuracy [Accuracy] - Sets an NPC's Accuracy");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "avoidance")) {
		if (sep->IsNumber(2)) {
			auto avoidance = Strings::ToUnsignedInt(sep->arg[2]);
			n.Avoidance = avoidance;
			d = fmt::format(
				"{} now has {} Avoidance.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit avoidance [Avoidance] - Sets an NPC's Avoidance");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "slow_mitigation")) {
		if (sep->IsNumber(2)) {
			auto slow_mitigation = static_cast<int16_t>(Strings::ToInt(sep->arg[2]));
			n.slow_mitigation = slow_mitigation;
			d = fmt::format(
				"{} now has {} Slow Mitigation.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit slow_mitigation [Slow Mitigation] - Sets an NPC's Slow Mitigation"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "version")) {
		if (sep->IsNumber(2)) {
			auto version = static_cast<uint16_t>(Strings::ToUnsignedInt(sep->arg[2]));
			n.version = version;
			d = fmt::format(
				"{} is now using Version {}.",
				npc_id_string,
				version
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit version [Version] - Sets an NPC's Version");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "maxlevel")) {
		if (sep->IsNumber(2)) {
			auto max_level = static_cast<int8_t>(Strings::ToInt(sep->arg[2]));
			n.maxlevel = max_level;
			d = fmt::format(
				"{} now has a Maximum Level of {}.",
				npc_id_string,
				max_level
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit maxlevel [Max Level] - Sets an NPC's Maximum Level");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "scalerate")) {
		if (sep->IsNumber(2)) {
			auto scale_rate = Strings::ToInt(sep->arg[2]);
			n.scalerate = scale_rate;
			d = fmt::format(
				"{} now has a Scaling Rate of {}%%.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit scalerate [Scale Rate] - Sets an NPC's Scaling Rate [50 = 50%, 100 = 100%, 200 = 200%]"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "emoteid")) {
		if (sep->IsNumber(2)) {
			const uint32 emote_id = Strings::ToUnsignedInt(sep->arg[2]);
			n.emoteid = emote_id;
			d = fmt::format(
				"{} now has an Emote ID of {}.",
				npc_id_string,
				Strings::Commify(emote_id)
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit emoteid [Emote ID] - Sets an NPC's Emote ID"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "spellscale")) {
		if (sep->IsNumber(2)) {
			auto spell_scale = Strings::ToUnsignedInt(sep->arg[2]);
			n.spellscale = static_cast<float>(spell_scale);
			d = fmt::format(
				"{} now has a Spell Scaling Rate of {}%%.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit spellscale [Scale Rate] - Sets an NPC's Spell Scaling Rate [50 = 50%, 100 = 100%, 200 = 200%]"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "healscale")) {
		if (sep->IsNumber(2)) {
			auto heal_scale = Strings::ToUnsignedInt(sep->arg[2]);
			n.healscale = static_cast<float>(heal_scale);
			d = fmt::format(
				"{} now has a Heal Scaling Rate of {}%%.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit healscale [Scale Rate] - Sets an NPC's Heal Scaling Rate [50 = 50%, 100 = 100%, 200 = 200%]"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "no_target")) {
		if (sep->IsNumber(2)) {
			auto is_no_target = static_cast<uint8_t>(Strings::ToUnsignedInt(sep->arg[2]));
			n.no_target_hotkey = is_no_target;
			d = fmt::format(
				"{} is {} Targetable with Target Hotkey.",
				npc_id_string,
				is_no_target ? "now" : "no longer"
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit no_target [Flag] - Sets an NPC's No Target Hotkey Flag [0 = Not Targetable with Target Hotkey, 1 = Targetable with Target Hotkey]"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "raidtarget")) {
		if (sep->IsNumber(2)) {
			auto is_raid_target = static_cast<uint8_t>(Strings::ToUnsignedInt(sep->arg[2]));
			n.raid_target = is_raid_target;
			d = fmt::format(
				"{} is {} designated as a Raid Target.",
				npc_id_string,
				is_raid_target ? "now" : "no longer"
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit raidtarget [Flag] - Sets an NPC's Raid Target Flag [0 = Not a Raid Target, 1 = Raid Target]"
			);
		}
		return;
	} else if (!strcasecmp(sep->arg[1], "armtexture")) {
		if (sep->IsNumber(2)) {
			auto arm_texture = static_cast<int8_t>(Strings::ToInt(sep->arg[2]));
			n.armtexture = arm_texture;
			d = fmt::format(
				"{} is now using Arm Texture {}.",
				npc_id_string,
				arm_texture
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit armtexture [Texture] - Sets an NPC's Arm Texture");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "bracertexture")) {
		if (sep->IsNumber(2)) {
			auto bracer_texture = static_cast<int8_t>(Strings::ToInt(sep->arg[2]));
			n.bracertexture = bracer_texture;
			d = fmt::format(
				"{} is now using Bracer Texture {}.",
				npc_id_string,
				bracer_texture
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit bracertexture [Texture] - Sets an NPC's Bracer Texture");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "handtexture")) {
		if (sep->IsNumber(2)) {
			auto hand_texture = static_cast<int8_t>(Strings::ToInt(sep->arg[2]));
			n.handtexture = hand_texture;
			d = fmt::format(
				"{} is now using Hand Texture {}.",
				npc_id_string,
				hand_texture
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit handtexture [Texture] - Sets an NPC's Hand Texture");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "legtexture")) {
		if (sep->IsNumber(2)) {
			auto leg_texture = static_cast<int8_t>(Strings::ToInt(sep->arg[2]));
			n.legtexture = leg_texture;
			d = fmt::format(
				"{} is now using Leg Texture {}.",
				npc_id_string,
				leg_texture
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit legtexture [Texture] - Sets an NPC's Leg Texture");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "feettexture")) {
		if (sep->IsNumber(2)) {
			auto feet_texture = static_cast<int8_t>(Strings::ToInt(sep->arg[2]));
			n.feettexture = feet_texture;
			d = fmt::format(
				"{} is now using Feet Texture {}.",
				npc_id_string,
				feet_texture
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit feettexture [Texture] - Sets an NPC's Feet Texture");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "walkspeed")) {
		if (sep->IsNumber(2)) {
			auto walk_speed = static_cast<int8_t>(Strings::ToInt(sep->arg[2]));
			n.walkspeed = walk_speed;
			d = fmt::format(
				"{} now walks at a Walk Speed of {}.",
				npc_id_string,
				walk_speed
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit walkspeed [Walk Speed] - Sets an NPC's Walk Speed");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "show_name")) {
		if (sep->IsNumber(2)) {
			auto show_name = static_cast<int8_t>(Strings::ToInt(sep->arg[2]));
			n.show_name = show_name;
			d = fmt::format(
				"{} will {} show their name.",
				npc_id_string,
				show_name ? "now" : "no longer"
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit show_name [Flag] - Sets an NPC's Show Name Flag [0 = Hidden, 1 = Shown]"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "untargetable")) {
		if (sep->IsNumber(2)) {
			auto is_untargetable = static_cast<int8_t>(Strings::ToInt(sep->arg[2]));
			n.untargetable = is_untargetable;
			d = fmt::format(
				"{} will {} be untargetable.",
				npc_id_string,
				is_untargetable ? "now" : "no longer"
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit untargetable [Flag] - Sets an NPC's Untargetable Flag [0 = Targetable, 1 = Untargetable]"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "charm_ac")) {
		if (sep->IsNumber(2)) {
			auto charm_armor_class = static_cast<int16_t>(Strings::ToInt(sep->arg[2]));
			n.charm_ac = charm_armor_class;
			d = fmt::format(
				"{} now has {} Armor Class while Charmed.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit charm_ac [Armor Class] - Sets an NPC's Armor Class while Charmed");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "charm_min_dmg")) {
		if (sep->IsNumber(2)) {
			auto charm_minimum_damage = Strings::ToInt(sep->arg[2]);
			n.charm_min_dmg = charm_minimum_damage;
			d = fmt::format(
				"{} now does {} Minimum Damage while Charmed.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit charm_min_dmg [Damage] - Sets an NPC's Minimum Damage while Charmed"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "charm_max_dmg")) {
		if (sep->IsNumber(2)) {
			auto charm_maximum_damage = Strings::ToInt(sep->arg[2]);
			n.charm_max_dmg = charm_maximum_damage;
			d = fmt::format(
				"{} now does {} Maximum Damage while Charmed.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit charm_max_dmg [Damage] - Sets an NPC's Maximum Damage while Charmed"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "charm_attack_delay")) {
		if (sep->IsNumber(2)) {
			auto charm_attack_delay = static_cast<int8_t>(Strings::ToInt(sep->arg[2]));
			n.charm_attack_delay = charm_attack_delay;
			d = fmt::format(
				"{} now has {} Attack Delay while Charmed.",
				npc_id_string,
				charm_attack_delay
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit charm_attack_delay [Attack Delay] - Sets an NPC's Attack Delay while Charmed"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "charm_accuracy_rating")) {
		if (sep->IsNumber(2)) {
			auto charm_accuracy_rating = Strings::ToInt(sep->arg[2]);
			n.charm_accuracy_rating = charm_accuracy_rating;
			d = fmt::format(
				"{} now has {} Accuracy Rating while Charmed.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit charm_accuracy_rating [Accuracy] - Sets an NPC's Accuracy Rating while Charmed"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "charm_avoidance_rating")) {
		if (sep->IsNumber(2)) {
			auto charm_avoidance_rating = Strings::ToInt(sep->arg[2]);
			n.charm_avoidance_rating = charm_avoidance_rating;
			d = fmt::format(
				"{} now has {} Avoidance Rating while Charmed.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit charm_avoidance_rating [Avoidance] - Sets an NPC's Avoidance Rating while Charmed"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "charm_atk")) {
		if (sep->IsNumber(2)) {
			auto charm_attack = Strings::ToInt(sep->arg[2]);
			n.charm_atk = charm_attack;
			d = fmt::format(
				"{} now has {} Attack while Charmed.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(Chat::White, "Usage: #npcedit charm_atk [Attack] - Sets an NPC's Attack while Charmed");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "skip_global_loot")) {
		if (sep->IsNumber(2)) {
			auto skip_global_loot = static_cast<int8_t>(Strings::ToInt(sep->arg[2]));
			n.skip_global_loot = skip_global_loot;
			d = fmt::format(
				"{} will {} skip Global Loot.",
				npc_id_string,
				skip_global_loot ? "now" : "no longer"
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit skip_global_loot [Flag] - Sets an NPC's Skip Global Loot Flag [0 = Don't Skip, 1 = Skip"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "rarespawn")) {
		if (sep->IsNumber(2)) {
			auto is_rare_spawn = static_cast<int8_t>(Strings::ToInt(sep->arg[2]));
			n.rare_spawn = is_rare_spawn;
			d = fmt::format(
				"{} is {} designated as a Rare Spawn.",
				npc_id_string,
				is_rare_spawn ? "now" : "no longer"
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit rarespawn [Flag] - Sets an NPC's Rare Spawn Flag [0 = Not a Rare Spawn, 1 = Rare Spawn]"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "stuck_behavior")) {
		if (sep->IsNumber(2)) {
			auto behavior_id   = static_cast<int8_t>(Strings::ToInt(sep->arg[2]));
			if (behavior_id > EQ::constants::StuckBehavior::EvadeCombat) {
				behavior_id = EQ::constants::StuckBehavior::EvadeCombat;
			}

			auto behavior_name = EQ::constants::GetStuckBehaviorName(behavior_id);
			n.stuck_behavior = behavior_id;
			d = fmt::format(
				"{} is now using Stuck Behavior {}.",
				npc_id_string,
				(
					!behavior_name.empty() ?
					fmt::format(
						"{} ({})",
						behavior_name,
						behavior_id
					) :
					std::to_string(behavior_id)
				)
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit stuck_behavior [Stuck Behavior] - Sets an NPC's Stuck Behavior [0 = Run to Target, 1 = Warp to Target, 2 = Take No Action, 3 = Evade Combat]"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "model")) {
		if (sep->IsNumber(2)) {
			auto race_id = static_cast<uint16_t>(Strings::ToUnsignedInt(sep->arg[2]));
			n.model = race_id;
			if (race_id) {
				d = fmt::format(
					"{} is now using a(n) {} ({}) as their race model.",
					npc_id_string,
					GetRaceIDName(race_id),
					race_id
				);
			} else {
				d = fmt::format(
					"{} is no longer using a race model.",
					npc_id_string
				);
			}
		} else {
			c->Message(Chat::White, "Usage: #npcedit model [Race ID] - Sets an NPC's Race Model");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "flymode")) {
		if (sep->IsNumber(2)) {
			auto flymode_id = static_cast<int8_t>(Strings::ToInt(sep->arg[2]));
			if (flymode_id > GravityBehavior::LevitateWhileRunning) {
				flymode_id = GravityBehavior::LevitateWhileRunning;
			}

			auto flymode_name = EQ::constants::GetFlyModeName(flymode_id);
			n.flymode = flymode_id;
			d = fmt::format(
				"{} is now using Fly Mode {}.",
				npc_id_string,
				(
					!flymode_name.empty() ?
					fmt::format(
						"{} ({})",
						flymode_name,
						flymode_id
					) :
					std::to_string(flymode_id)
				)
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit flymode [Fly Mode] - Sets an NPC's Fly Mode [0 = Ground, 1 = Flying, 2 = Levitating, 3 = Water, 4 = Floating, 5 = Levitating While Running]"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "always_aggro")) {
		if (sep->IsNumber(2)) {
			auto always_aggro = static_cast<int8_t>(Strings::ToInt(sep->arg[2]));
			n.always_aggro = always_aggro;
			d = fmt::format(
				"{} will {} Always Aggro.",
				npc_id_string,
				always_aggro ? "now" : "no longer"
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit always_aggro [Flag] - Sets an NPC's Always Aggro Flag [0 = Does not Always Aggro, 1 = Always Aggro]"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "exp_mod")) {
		if (sep->IsNumber(2)) {
			auto experience_modifier = Strings::ToInt(sep->arg[2]);
			n.exp_mod = experience_modifier;
			d = fmt::format(
				"{} now has an Experience Modifier of {}%%.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit exp_mod [Modifier] - Sets an NPC's Experience Modifier [50 = 50%, 100 = 100%, 200 = 200%]"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "heroic_strikethrough")) {
		if (sep->IsNumber(2)) {
			auto heroic_strikethrough = Strings::ToInt(sep->arg[2]);
			n.heroic_strikethrough = heroic_strikethrough;
			d = fmt::format(
				"{} now has {} Heroic Strikethrough.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit heroic_strikethrough [Heroic Strikethrough] - Sets an NPC's Heroic Strikethrough"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "faction_amount")) {
		if (sep->IsNumber(2)) {
			auto faction_amount = Strings::ToInt(sep->arg[2]);
			n.faction_amount = faction_amount;
			d = fmt::format(
				"{} now has a Faction Amount of {}.",
				npc_id_string,
				Strings::Commify(sep->arg[2])
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit faction_amount [Faction Amount] - Sets an NPC's Faction Amount"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "keeps_sold_items")) {
		if (sep->IsNumber(2)) {
			auto keeps_sold_items = static_cast<uint8_t>(Strings::ToUnsignedInt(sep->arg[2]));
			n.keeps_sold_items = keeps_sold_items;
			d = fmt::format(
				"{} will {} Keep Sold Items.",
				npc_id_string,
				keeps_sold_items ? "now" : "no longer"
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit keeps_sold_items [Flag] - Sets an NPC's Keeps Sold Items Flag [0 = False, 1 = True]"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "is_parcel_merchant")) {
		if (sep->IsNumber(2)) {
			const bool is_parcel_merchant = static_cast<uint8_t>(Strings::ToUnsignedInt(sep->arg[2]));

			n.is_parcel_merchant = is_parcel_merchant;

			d = fmt::format(
				"{} will {} be a Parcel Merchant.",
				npc_id_string,
				is_parcel_merchant ? "now" : "no longer"
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit is_parcel_merchant [Flag] - Sets an NPC's Parcel Merchant Flag [0 = False, 1 = True]"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "setanimation")) {
		if (sep->IsNumber(2)) {
			auto animation_id   = Strings::ToUnsignedInt(sep->arg[2]);
			if (animation_id > EQ::constants::SpawnAnimations::Looting) {
				animation_id = EQ::constants::SpawnAnimations::Looting;
			}

			auto animation_name = EQ::constants::GetSpawnAnimationName(animation_id);
			d = fmt::format(
				"{} is now using Spawn Animation {} on Spawn Group ID {}.",
				npc_id_string,
				(
					!animation_name.empty() ?
					fmt::format(
						"{} ({})",
						animation_name,
						animation_id
					) :
					std::to_string(animation_id)
				),
				Strings::Commify(std::to_string(c->GetTarget()->CastToNPC()->GetSpawnGroupId()))
			);
			auto query = fmt::format(
				"UPDATE spawn2 SET animation = {} WHERE spawngroupID = {}",
				animation_id,
				c->GetTarget()->CastToNPC()->GetSpawnGroupId()
			);
			content_db.QueryDatabase(query);

			c->GetTarget()->SetAppearance(EmuAppearance(animation_id));
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit setanimation [Animation ID] - Sets an NPC's Animation on Spawn (Stored in spawn2 table)"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "respawntime")) {
		if (sep->IsNumber(2)) {
			auto respawn_time = Strings::ToUnsignedInt(sep->arg[2]);
			if (respawn_time) {
				d = fmt::format(
					"{} now has a Respawn Timer of {} ({}) on Spawn Group ID {}.",
					npc_id_string,
					Strings::SecondsToTime(respawn_time),
					respawn_time,
					Strings::Commify(std::to_string(t->GetSpawnGroupId()))
				);
				auto query = fmt::format(
					"UPDATE spawn2 SET respawntime = {} WHERE spawngroupID = {} AND version = {}",
					respawn_time,
					t->GetSpawnGroupId(),
					zone->GetInstanceVersion()
				);
				content_db.QueryDatabase(query);
			} else {
				c->Message(Chat::White, "Respawn Timer must be greater than 0 seconds.");
				return;
			}
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit respawntime [Respawn Time] - Sets an NPC's Respawn Timer in Seconds (Stored in spawn2 table)"
			);
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "set_grid")) {
		if (sep->IsNumber(2)) {
			const uint32 grid_id = Strings::ToUnsignedInt(sep->arg[2]);
			if (grid_id >= 0) {
				d = fmt::format(
					"{} now has a Grid ID of {} on Spawn Group ID {}.",
					npc_id_string,
					grid_id,
					Strings::Commify(std::to_string(t->GetSpawnGroupId()))
				);
				auto query = fmt::format(
					"UPDATE spawn2 SET pathgrid = {} WHERE spawngroupID = {} AND version = {} AND zone = '{}'",
					grid_id,
					t->GetSpawnGroupId(),
					zone->GetInstanceVersion(),
					zone->GetShortName()
				);
				content_db.QueryDatabase(query);
			} else {
				c->Message(Chat::White, "Grid ID must be greater than or equal to 0.");
				return;
			}
		} else {
			c->Message(Chat::White, "Usage: #npcedit set_grid [Grid ID] - Sets an NPC's Grid ID");
			return;
		}
	} else if (!strcasecmp(sep->arg[1], "npc_tint_id")) {
		if (sep->IsNumber(2)) {
			const uint32 npc_tint_id = (Strings::ToUnsignedInt(sep->arg[2]));

			n.npc_tint_id = npc_tint_id;

			d = fmt::format(
				"Set NPCTintID {} for {}",
				npc_tint_id,
				npc_id_string
			);
		} else {
			c->Message(
				Chat::White,
				"Usage: #npcedit npc_tint_id [id] - Sets an NPC's NPCTintID [0 - 78 for RoF2]"
			);
			return;
		}
	} else {
		SendNPCEditSubCommands(c);
		return;
	}

	if (!NpcTypesRepository::UpdateOne(content_db, n)) {
		c->Message(
			Chat::White,
			fmt::format(
				"Failed to update {}.",
				npc_id_string
			).c_str()
		);
	}

	c->Message(Chat::White, d.c_str());
}
