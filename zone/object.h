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

#pragma once

// Object Class:
// Represents Zone Objects (forges, ovens, brew barrels, items dropped to ground, etc)

#include "common/eq_packet_structs.h"
#include "common/timer.h"
#include "common/types.h"
#include "zone/entity.h"

class Client;
class EQApplicationPacket;

namespace EQ
{
	class ItemInstance;
}

/*
item icon numbers (showeq)
IT1_ACTORDEF=Long Sword
IT5_ACTORDEF=Throwing Knife
IT8_ACTORDEF=Basic Staff
IT10_ACTORDEF=Arrow
IT14_ACTORDEF=Basic Hammer
IT16_ACTORDEF=Basic Spear
IT27_ACTORDEF=Book
IT35_ACTORDEF=Mod Rod
IT62_ACTORDEF=Flaming Sword
IT63_ACTORDEF=Small Bag
IT64_ACTORDEF=Large Bag
IT65_ACTORDEF=Scroll
IT66_ACTORDEF=Forge
IT67_ACTORDEF=Voodoo Doll
IT68_ACTORDEF=Glowing Black Stone
IT69_ACTORDEF=Oven
IT70_ACTORDEF=Brew Barrel
IT73_ACTORDEF=Kiln
IT74_ACTORDEF=Pottery Wheel
IT78_ACTORDEF=Campfire (Oven)
IT128_ACTORDEF=Loom
IT177_ACTORDEF=Shattering Hammer
IT203_ACTORDEF=Round Shield
IT210_ACTORDEF=Shimmering Orb
IT400_ACTORDEF=Globe of Slush Water
IT401_ACTORDEF=Red Mushroom
IT402_ACTORDEF=Blue Mushroom
IT403_ACTORDEF=Yew Leaf
IT10511_ACTORDEF=A Soulstone Shard
IT10512_ACTORDEF=Orb of Exploration
IT10630_ACTORDEF=Fish Sword
IT10661_ACTORDEF=Blade of Walnan
IT10714_ACTORDEF=Augmentation Sealer
IT10725_ACTORDEF=Shuriken
*/

// Icon values:
//0x0453 a pie
//0x0454 cookies?
//0x0455 is a piece of meat?
//0x0456 is fletching sticks
//0x0457 looks like a burnt cookie or something :/
//0x0458 is a pottery wheel
//0x0459 is a oven
//0x045A is an oven
//0x045B is a forge
//0x045C is brewing barrel
//0x045D is a hammer
//0x045E is a wierd rope shape

enum ObjectTypes {
	StaticLocked = 0,
	Temporary = 1,
	ToolBox = 10,
	Research = 11,
	Mortar = 12,
	SelfDusting = 13,
	Baking1 = 14,
	Baking2 = 15,
	Tailoring = 16,
	Forge = 17,
	Fletching = 18,
	BrewBarrel = 19,
	Jewelcrafting = 20,
	PotteryWheel = 21,
	PotteryKiln = 22,
	WizardResearch = 24,
	MagicianResearch = 25,
	NecromancerResearch = 26,
	EnchanterResearch = 27,
	Invalid1 = 28,
	Invalid2 = 29,
	Experimental = 30,
	HighElfForge = 31,
	DarkElfForge = 32,
	OgreForge = 33,
	DwarfForge = 34,
	GnomeForge = 35,
	BarbarianForge = 36,
	IksarForge = 38,
	HumanForge = 39,
	HumanForge2 = 40,
	HalflingTailoring = 41,
	EruditeTailoring = 42,
	WoodElfTailoring = 43,
	WoodElfFletching = 44,
	IksarPotteryWheel = 45,
	TrollForge = 47,
	WoodElfForge = 48,
	HalflingForge = 49,
	EruditeForge = 50,
	AugmentationPool = 53,
	StaticUnlocked = 255
};

class Object: public Entity
{
public:
	// Loading object from database
	Object(uint32 id, uint32 type, uint32 icon, const Object_Struct& data, const EQ::ItemInstance* inst = nullptr, bool fix_z = true);
	Object(const EQ::ItemInstance* inst, const std::string& name, float max_x, float min_x, float max_y, float min_y, float z, float heading, uint32 respawn_timer, bool fix_z);
	// Loading object from client dropping item on ground
	Object(Client* client, const EQ::ItemInstance* inst);
	Object(const EQ::ItemInstance *inst, float x, float y, float z, float heading, uint32 decay_time = 300000, bool fix_z = true);
	Object(const std::string& model, float x, float y, float z, float heading, uint8 type, uint32 decay_time = 0);

// Destructor
	~Object();
	bool Process();
	bool IsGroundSpawn() { return m_ground_spawn; }
	// Event handlers
	bool HandleClick(Client* sender, const ClickObject_Struct* click_object);
	void Close();
	void Delete(bool reset_state=false); // Object itself
	static void HandleCombine(Client* user, const NewCombine_Struct* in_combine, Object *worldo);
	static void HandleAugmentation(Client* user, const AugmentItem_Struct* in_augment, Object *worldo);
	static void HandleAutoCombine(Client* user, const RecipeAutoCombine_Struct* rac);

	// Packet functions
	void CreateSpawnPacket(EQApplicationPacket* app);
	void CreateDeSpawnPacket(EQApplicationPacket* app);
	void Depop();
	void Repop();

	// Floating
	inline bool IsFixZEnabled() const { return m_fix_z; };
	inline void SetFixZ(bool fix_z) { m_fix_z = fix_z; };

	//Decay functions
	void StartDecay() {decay_timer.Start();}

	// Container functions
	const EQ::ItemInstance* GetItem(uint8 index);
	void PutItem(uint8 index, const EQ::ItemInstance* inst);
	void DeleteItem(uint8 index); // Item inside container
	EQ::ItemInstance* PopItem(uint8 index); // Pop item out of container

	// Override base class implementations
	virtual bool IsObject()	const { return true; }
	virtual bool Save();
	virtual uint16 VarSave();
	virtual void SetID(uint16 set_id);

	void ClearUser() { user = nullptr; }

	uint32 GetDBID();
	uint32 GetType();
	void SetType(uint32 type);
	uint32 GetIcon();
	void SetIcon(uint32 icon);
	uint32 GetItemID();
	void SetItemID(uint32 itemid);
	void GetObjectData(Object_Struct* Data);
	void SetObjectData(Object_Struct* Data);
	void GetLocation(float* x, float* y, float* z);
	void SetLocation(float x, float y, float z);
	void GetHeading(float* heading);
	void SetHeading(float heading);
	float GetX();
	float GetY();
	float GetZ();
	float GetHeadingData();
	void SetX(float pos);
	void SetY(float pos);
	void SetZ(float pos);
	void SetTiltX(float pos);
	void SetTiltY(float pos);
	float GetTiltX();
	float GetTiltY();
	void SetModelName(const char* modelname);
	const char* GetModelName();
	float GetSize();
	void SetSize(float size);
	uint16 GetSolidType();
	void SetSolidType(uint16 size);
	void SetDisplayName(const char *in_name);
	const char *GetDisplayName() const { return m_display_name; }

	bool ClearEntityVariables();
	bool DeleteEntityVariable(std::string variable_name);
	std::string GetEntityVariable(std::string variable_name);
	std::vector<std::string> GetEntityVariables();
	void SetEntityVariable(std::string variable_name, std::string variable_value);
	bool EntityVariableExists(std::string variable_name);

protected:
	void	ResetState();	// Set state back to original
	void	RandomSpawn(bool send_packet = false);		//spawn this ground spawn at a random place

	Object_Struct m_data;        // Packet data
	EQ::ItemInstance *m_inst;        // Item representing object
	uint32 m_id;        // Database key, different than drop_id
	uint32 m_type;        // Object Type, ie, forge, oven, dropped item, etc (ref: ContainerUseTypes)
	uint32 m_icon;        // Icon to use for forge, oven, etc
	float  m_max_x;
	float  m_max_y;
	float  m_min_x;
	float  m_min_y;
	bool   m_ground_spawn;
	char   m_display_name[64];
	bool   m_fix_z;
protected:

	std::map<std::string, std::string> o_EntityVariables;

	Client *user;
	Client *last_user;

	Timer respawn_timer;
	Timer decay_timer;
	void FixZ();
};

namespace ObjectType {
	constexpr uint32 SmallBag             = 0;
	constexpr uint32 LargeBag             = 1;
	constexpr uint32 Quiver               = 2;
	constexpr uint32 BeltPouch            = 3;
	constexpr uint32 WristPouch           = 4;
	constexpr uint32 Backpack             = 5;
	constexpr uint32 SmallChest           = 6;
	constexpr uint32 LargeChest           = 7;
	constexpr uint32 Bandolier            = 8;
	constexpr uint32 Medicine             = 9;
	constexpr uint32 Tinkering            = 10;
	constexpr uint32 Lexicon              = 11;
	constexpr uint32 PoisonMaking         = 12;
	constexpr uint32 Quest                = 13;
	constexpr uint32 MixingBowl           = 14;
	constexpr uint32 Baking               = 15;
	constexpr uint32 Tailoring            = 16;
	constexpr uint32 Blacksmithing        = 17;
	constexpr uint32 Fletching            = 18;
	constexpr uint32 Brewing              = 19;
	constexpr uint32 JewelryMaking        = 20;
	constexpr uint32 Pottery              = 21;
	constexpr uint32 Kiln                 = 22;
	constexpr uint32 KeyMaker             = 23;
	constexpr uint32 ResearchWIZ          = 24;
	constexpr uint32 ResearchMAG          = 25;
	constexpr uint32 ResearchNEC          = 26;
	constexpr uint32 ResearchENC          = 27;
	constexpr uint32 Unknown              = 28;
	constexpr uint32 ResearchPractice     = 29;
	constexpr uint32 Alchemy              = 30;
	constexpr uint32 HighElfForge         = 31;
	constexpr uint32 DarkElfForge         = 32;
	constexpr uint32 OgreForge            = 33;
	constexpr uint32 DwarfForge           = 34;
	constexpr uint32 GnomeForge           = 35;
	constexpr uint32 BarbarianForge       = 36;
	constexpr uint32 IksarForge           = 37;
	constexpr uint32 HumanForgeOne        = 38;
	constexpr uint32 HumanForgeTwo        = 39;
	constexpr uint32 HalflingTailoringOne = 40;
	constexpr uint32 HalflingTailoringTwo = 41;
	constexpr uint32 EruditeTailoring     = 42;
	constexpr uint32 WoodElfTailoring     = 43;
	constexpr uint32 WoodElfFletching     = 44;
	constexpr uint32 IksarPottery         = 45;
	constexpr uint32 Fishing              = 46;
	constexpr uint32 TrollForge           = 47;
	constexpr uint32 WoodElfForge         = 48;
	constexpr uint32 HalflingForge        = 49;
	constexpr uint32 EruditeForge         = 50;
	constexpr uint32 Merchant             = 51;
	constexpr uint32 FroglokForge         = 52;
	constexpr uint32 Augmenter            = 53;
	constexpr uint32 Churn                = 54;
	constexpr uint32 TransformationMold   = 55;
	constexpr uint32 DetransformationMold = 56;
	constexpr uint32 Unattuner            = 57;
	constexpr uint32 TradeskillBag        = 58;
	constexpr uint32 CollectibleBag       = 59;
	constexpr uint32 NoDeposit            = 60;

	std::string GetName(uint32 object_type);
	bool IsValid(uint32 object_type);
}

static std::map<uint32, std::string> object_types = {
	{ ObjectType::SmallBag,             "Small Bag" },
	{ ObjectType::LargeBag,             "Large Bag" },
	{ ObjectType::Quiver,               "Quiver" },
	{ ObjectType::BeltPouch,            "Belt Pouch" },
	{ ObjectType::WristPouch,           "Wrist Pouch" },
	{ ObjectType::Backpack,             "Backpack" },
	{ ObjectType::SmallChest,           "Small Chest" },
	{ ObjectType::LargeChest,           "Large Chest" },
	{ ObjectType::Bandolier,            "Bandolier" },
	{ ObjectType::Medicine,             "Medicine" },
	{ ObjectType::Tinkering,            "Tinkering" },
	{ ObjectType::Lexicon,              "Lexicon" },
	{ ObjectType::PoisonMaking,         "Mortar and Pestle" },
	{ ObjectType::Quest,                "Quest" },
	{ ObjectType::MixingBowl,           "Mixing Bowl" },
	{ ObjectType::Baking,               "Baking" },
	{ ObjectType::Tailoring,            "Tailoring" },
	{ ObjectType::Blacksmithing,        "Blacksmithing" },
	{ ObjectType::Fletching,            "Fletching" },
	{ ObjectType::Brewing,              "Brewing" },
	{ ObjectType::JewelryMaking,        "Jewelry Making" },
	{ ObjectType::Pottery,              "Pottery" },
	{ ObjectType::Kiln,                 "Kiln" },
	{ ObjectType::KeyMaker,             "Key Maker" },
	{ ObjectType::ResearchWIZ,          "Lexicon" },
	{ ObjectType::ResearchMAG,          "Lexicon" },
	{ ObjectType::ResearchNEC,          "Lexicon" },
	{ ObjectType::ResearchENC,          "Lexicon" },
	{ ObjectType::Unknown,              "Unknown" },
	{ ObjectType::ResearchPractice,     "Lexicon" },
	{ ObjectType::Alchemy,              "Alchemy" },
	{ ObjectType::HighElfForge,         "High Elf Forge" },
	{ ObjectType::DarkElfForge,         "Dark Elf Forge" },
	{ ObjectType::OgreForge,            "Ogre Forge" },
	{ ObjectType::DwarfForge,           "Dwarf Forge" },
	{ ObjectType::GnomeForge,           "Gnome Forge" },
	{ ObjectType::BarbarianForge,       "Barbarian Forge" },
	{ ObjectType::IksarForge,           "Iksar Forge" },
	{ ObjectType::HumanForgeOne,        "Human Forge" },
	{ ObjectType::HumanForgeTwo,        "Human Forge" },
	{ ObjectType::HalflingTailoringOne, "Halfling Tailoring" },
	{ ObjectType::HalflingTailoringTwo, "Halfling Tailoring" },
	{ ObjectType::EruditeTailoring,     "Erudite Tailoring" },
	{ ObjectType::WoodElfTailoring,     "Wood Elf Tailoring" },
	{ ObjectType::WoodElfFletching,     "Wood Elf Fletching" },
	{ ObjectType::IksarPottery,         "Iksar Pottery" },
	{ ObjectType::Fishing,              "Fishing" },
	{ ObjectType::TrollForge,           "Troll Forge" },
	{ ObjectType::WoodElfForge,         "Wood Elf Forge" },
	{ ObjectType::HalflingForge,        "Halfling Forge" },
	{ ObjectType::EruditeForge,         "Erudite Forge" },
	{ ObjectType::Merchant,             "Merchant" },
	{ ObjectType::FroglokForge,         "Froglok Forge" },
	{ ObjectType::Augmenter,            "Augmenter" },
	{ ObjectType::Churn,                "Churn" },
	{ ObjectType::TransformationMold,   "Transformation Mold" },
	{ ObjectType::DetransformationMold, "Detransformation Mold" },
	{ ObjectType::Unattuner,            "Unattuner" },
	{ ObjectType::TradeskillBag,        "Tradeskill Bag" },
	{ ObjectType::CollectibleBag,       "Collectible Bag" },
	{ ObjectType::NoDeposit,            "No Deposit" }
};
