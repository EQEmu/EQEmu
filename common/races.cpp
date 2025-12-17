/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemu.org)

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

#include "races.h"

#include "common/data_verification.h"

const char* GetRaceIDName(uint16 race_id)
{
	switch (race_id) {
		case Race::Abhorrent:
			return "Abhorrent";
		case Race::AirElemental:
		case Race::AirElemental2:
			return "Air Elemental";
		case Race::AirMephit:
			return "Air Mephit";
		case Race::Akhevan:
		case Race::Akheva:
			return "Akheva";
		case Race::Alaran:
			return "Alaran";
		case Race::AlaranGhost:
			return "Alaran Ghost";
		case Race::AlaranSentryStone:
			return "Alaran Sentry Stone";
		case Race::Alligator:
		case Race::Alligator2:
			return "Alligator";
		case Race::Denizen:
		case Race::Amygdalan:
			return "Amygdalan";
		case Race::Aneuk:
			return "Aneuk";
		case Race::AnimatedArmor:
			return "Animated Armor";
		case Race::AnimatedHand:
			return "Animated Hand";
		case Race::AnimatedStatue:
		case Race::AnimatedStatue2:
			return "Animated Statue";
		case Race::Apexus:
			return "Apexus";
		case Race::Arachnid:
			return "Arachnid";
		case Race::ArcanistOfHate:
			return "Arcanist of Hate";
		case Race::Armadillo:
			return "Armadillo";
		case Race::ArmorRack:
			return "Armor Rack";
		case Race::Aviak:
		case Race::Aviak2:
			return "Aviak";
		case Race::AviakPullAlong:
			return "Aviak Pull Along";
		case Race::AyonaeRo:
			return "Ayonae Ro";
		case Race::Banner:
		case Race::Banner2:
		case Race::Banner3:
		case Race::Banner4:
		case Race::Banner5:
		case Race::Banner6:
		case Race::Banner7:
			return "Banner";
		case Race::TenthAnniversaryBanner:
			return "10th Anniversary Banner";
		case Race::Banshee:
		case Race::Banshee2:
		case Race::Banshee3:
			return "Banshee";
		case Race::Barbarian:
		case Race::HalasCitizen:
			return "Barbarian";
		case Race::Barrel:
			return "Barrel";
		case Race::UndeadBoat:
			return "Barrel Barge Ship";
		case Race::Basilisk:
			return "Basilisk";
		case Race::GiantBat:
		case Race::Bat:
		case Race::Bat2:
			return "Bat";
		case Race::Bazu:
			return "Bazu";
		case Race::Bear:
		case Race::Bear2:
		case Race::Bear3:
			return "Bear";
		case Race::BearMount:
			return "Bear Mount";
		case Race::BearTrap:
			return "Bear Trap";
		case Race::Beetle:
		case Race::Beetle2:
		case Race::Beetle3:
			return "Beetle";
		case Race::HumanBeggar:
			return "Beggar";
		case Race::Bellikos:
			return "Bellikos";
		case Race::Bertoxxulous:
		case Race::BertoxxulousNew:
			return "Bertoxxulous";
		case Race::Bixie:
		case Race::Bixie2:
			return "Bixie";
		case Race::BlimpShip:
			return "Blimp Ship";
		case Race::BlindDreamer:
			return "Blind Dreamer";
		case Race::BloodRaven:
			return "Blood Raven";
		case Race::Boat:
		case Race::Boat2:
			return "Boat";
		case Race::Bolvirk:
			return "Bolvirk";
		case Race::BoneGolem:
		case Race::BoneGolem2:
			return "Bone Golem";
		case Race::Bones:
			return "Bones";
		case Race::BookDervish:
			return "Book Dervish";
		case Race::Portal:
			return "BoT Portal";
		case Race::Boulder:
			return "Boulder";
		case Race::Box:
			return "Box";
		case Race::Braxi:
			return "Braxi";
		case Race::BraxiMount:
			return "Braxi Mount";
		case Race::Brell:
			return "Brell";
		case Race::BrellsFirstCreation:
			return "Brell's First Creation";
		case Race::Bristlebane:
			return "Bristlebane";
		case Race::BrokenClockwork:
			return "Broken Clockwork";
		case Race::Brontotherium:
			return "Brontotherium";
		case Race::Brownie:
		case Race::Brownie2:
			return "Brownie";
		case Race::Bubonian:
			return "Bubonian";
		case Race::BubonianUnderling:
			return "Bubonian Underling";
		case Race::Burynai:
		case Race::Burynai2:
			return "Burynai";
		case Race::TenthAnniversaryCake:
			return "10th Anniversary Cake";
		case Race::Campfire:
			return "Campfire";
		case Race::CarrierHand:
			return "Carrier Hand";
		case Race::Cat:
			return "Cat";
		case Race::CazicThule:
		case Race::CazicThule2:
			return "Cazic Thule";
		case Race::Centaur:
		case Race::Centaur2:
			return "Centaur";
		case Race::Chest:
		case Race::Chest2:
		case Race::Chest3:
			return "Chest";
		case Race::Chimera:
		case Race::Chimera2:
			return "Chimera";
		case Race::Chokidai:
			return "Chokidai";
		case Race::Clam:
			return "Clam";
		case Race::CliknarMount:
			return "Cliknar Mount";
		case Race::CliknarQueen:
			return "Cliknar Queen";
		case Race::CliknarSoldier:
			return "Cliknar Soldier";
		case Race::CliknarWorker:
			return "Cliknar Worker";
		case Race::ClockworkBeetle:
			return "Clockwork Beetle";
		case Race::ClockworkBoar:
			return "Clockwork Boar";
		case Race::ClockworkBomb:
			return "Clockwork Bomb";
		case Race::ClockworkBrain:
			return "Clockwork Brain";
		case Race::ClockworkGnome:
			return "Clockwork Gnome";
		case Race::ClockworkGolem:
			return "Clockwork Golem";
		case Race::ClockworkGuardian:
			return "Clockwork Guardian";
		case Race::Cockatrice:
			return "Cockatrice";
		case Race::Coffin:
		case Race::Coffin2:
			return "Coffin";
		case Race::CoinPurse:
			return "Coin Purse";
		case Race::Coldain:
		case Race::Coldain2:
		case Race::Coldain3:
			return "Coldain";
		case Race::Coral:
			return "Coral";
		case Race::Corathus:
			return "Corathus";
		case Race::Crab:
			return "Crab";
		case Race::Cragbeast:
			return "Cragbeast";
		case Race::Cragslither:
			return "Cragslither";
		case Race::Crocodile:
			return "Crocodile";
		case Race::Crystal:
			return "Crystal";
		case Race::CrystalShard:
			return "Crystal Shard";
		case Race::CrystalSphere:
			return "Crystal Sphere";
		case Race::CrystalSpider:
			return "Crystal Spider";
		case Race::CrystalskinAmbuloid:
			return "Crystalskin Ambuloid";
		case Race::CrystalskinSessiloid:
			return "Crystalskin Sessiloid";
		case Race::DaisyMan:
			return "Daisy Man";
		case Race::DarkElf:
		case Race::NeriakCitizen:
			return "Dark Elf";
		case Race::DarkLord:
			return "Dark Lord";
		case Race::DemiLich:
			return "Demi Lich";
		case Race::DemonVulture:
			return "Demon Vulture";
		case Race::Dervish:
		case Race::Dervish2:
		case Race::Dervish3:
		case Race::Dervish4:
			return "Dervish";
		case Race::Dervish5:
			return "Dervish(Ver. 5)";
		case Race::Dervish6:
			return "Dervish(Ver. 6)";
		case Race::Devourer:
			return "Devourer";
		case Race::DireWolf:
			return "Dire Wolf";
		case Race::DiscordShip:
			return "Discord Ship";
		case Race::Discordling:
			return "Discordling";
		case Race::DiseasedFiend:
			return "Diseased Fiend";
		case Race::Djinn:
			return "Djinn";
		case Race::Drachnid:
		case Race::Drachnid2:
			return "Drachnid";
		case Race::DrachnidCocoon:
			return "Drachnid Cocoon";
		case Race::Dracolich:
			return "Dracolich";
		case Race::Draglock:
			return "Draglock";
		case Race::LavaDragon:
		case Race::DragonSkeleton:
		case Race::WaterDragon:
		case Race::VeliousDragon:
		case Race::ClockworkDragon:
		case Race::BlackAndWhiteDragon:
		case Race::GhostDragon:
		case Race::PrismaticDragon:
		case Race::Quarm:
		case Race::Dragon:
		case Race::Dragon2:
		case Race::Dragon3:
		case Race::Dragon4:
		case Race::Dragon5:
		case Race::Dragon6:
		case Race::Dragon7:
			return "Dragon";
		case Race::DragonBones:
			return "Dragon Bones";
		case Race::DragonEgg:
			return "Dragon Egg";
		case Race::DragonStatue:
			return "Dragon Statue";
		case Race::Dragorn:
			return "Dragorn";
		case Race::DragornBox:
			return "Dragorn Box";
		case Race::Drake:
		case Race::Drake2:
		case Race::Drake3:
			return "Drake";
		case Race::Drakkin:
			return "Drakkin";
		case Race::Drixie:
			return "Drixie";
		case Race::Drogmor:
			return "Drogmore";
		case Race::Drolvarg:
			return "Drolvarg";
		case Race::Dryad:
			return "Dryad";
		case Race::Dwarf:
		case Race::KaladimCitizen:
			return "Dwarf";
		case Race::DynamiteKeg:
			return "Dynamite Keg";
		case Race::Dynleth:
			return "Dyn'Leth";
		case Race::EarthElemental:
		case Race::EarthElemental2:
			return "Earth Elemental";
		case Race::EarthMephit:
			return "Earth Mephit";
		case Race::GiantEel:
			return "Eel";
		case Race::Efreeti:
		case Race::Efreeti2:
			return "Efreeti";
		case Race::Elddar:
			return "Elddar";
		case Race::Elemental:
			return "Elemental";
		case Race::ElkHead:
			return "Elk Head";
		case Race::ElvenBoat:
			return "Elven Boat";
		case Race::ElvenGhost:
			return "Elven Ghost";
		case Race::EnchantedArmor:
			return "Enchanted Armor";
		case Race::Erollisi:
			return "Erollisi";
		case Race::Erudite:
		case Race::EruditeCitizen:
		case Race::Erudite2:
			return "Erudite";
		case Race::EvanTest:
			return "Evan Test";
		case Race::EvilEye:
		case Race::EvilEye2:
		case Race::EvilEye3:
			return "Evil Eye";
		case Race::Exoskeleton:
			return "Exoskeleton";
		case Race::ExplosiveCart:
			return "Explosive Cart";
		case Race::EyeOfZomm:
			return "Eye";
		case Race::Fairy:
		case Race::Fairy2:
			return "Fairy";
		case Race::FallenKnight:
			return "Fallen Knight";
		case Race::Faun:
			return "Faun";
		case Race::FayDrake:
			return "Fay Drake";
		case Race::FenninRo:
			return "Fennin Ro";
		case Race::Feran:
			return "Feran";
		case Race::FeranMount:
			return "Feran Mount";
		case Race::Fiend:
			return "Fiend";
		case Race::FireElemental:
		case Race::FireElemental2:
			return "Fire Elemental";
		case Race::FireMephit:
			return "Fire Mephit";
		case Race::Fish:
		case Race::KunarkFish:
			return "Fish";
		case Race::Flag:
			return "Flag";
		case Race::FloatingIsland:
			return "Floating Island";
		case Race::FloatingSkull:
			return "Floating Skull";
		case Race::FloatingTower:
			return "Floating Tower";
		case Race::Fly:
			return "Fly";
		case Race::FlyingCarpet:
			return "Flying Carpet";
		case Race::ForestGiant2:
			return "Forest Giant";
		case Race::Frog:
		case Race::Frog2:
			return "Frog";
		case Race::Froglok:
		case Race::FroglokGhoul:
		case Race::Froglok2:
			return "Froglok";
		case Race::FroglokGhost:
			return "Froglok Ghost";
		case Race::FroglokSkeleton:
			return "Froglok Skeleton";
		case Race::FungalFiend:
			return "Fungal Fiend";
		case Race::FungusPatch:
			return "Fungus Patch";
		case Race::Fungusman:
			return "Fungusman";
		case Race::Galorian:
			return "Galorian";
		case Race::Gargoyle:
		case Race::Gargoyle2:
			return "Gargoyle";
		case Race::Gasbag:
			return "Gasbag";
		case Race::GelatinousCube:
		case Race::GelatinousCube2:
			return "Gelatinous Cube";
		case Race::Gelidran:
			return "Gelidran";
		case Race::Genari:
			return "Genari";
		case Race::Geonid:
			return "Geonid";
		case Race::Ghost:
		case Race::DwarfGhost:
		case Race::EruditeGhost:
		case Race::PirateGhost:
			return "Ghost";
		case Race::GhostShip:
		case Race::GhostShip2:
			return "Ghost Ship";
		case Race::Ghoul:
		case Race::Ghoul2:
			return "Ghoul";
		case Race::Giant:
		case Race::ForestGiant:
		case Race::FrostGiant:
		case Race::StormGiant:
		case Race::EarthGolem:
		case Race::IronGolem:
		case Race::StormGolem:
		case Race::AirGolem:
		case Race::WoodGolem:
		case Race::FireGolem:
		case Race::WaterGolem:
		case Race::Giant2:
		case Race::Giant3:
			return "Giant";
		case Race::GiantClockwork:
			return "Giant Clockwork";
		case Race::Giant4:
			return "Giant(Rallosian mats)";
		case Race::GiantShade:
			return "Giant Shade";
		case Race::Gigyn:
			return "Gigyn";
		case Race::GingerbreadMan:
			return "Gingerbread Man";
		case Race::Girplan:
			return "Girplan";
		case Race::Gnoll:
		case Race::Gnoll2:
		case Race::Gnoll3:
			return "Gnoll";
		case Race::Gnome:
			return "Gnome";
		case Race::Gnomework:
			return "Gnomework";
		case Race::GnomishBalloon:
			return "Gnomish Balloon";
		case Race::GnomishBoat:
			return "Gnomish Boat";
		case Race::GnomishHoveringTransport:
			return "Gnomish Hovering Transport";
		case Race::GnomishRocketPack:
			return "Gnomish Rocket Pack";
		case Race::Goblin:
		case Race::Bloodgill:
		case Race::KunarkGoblin:
		case Race::NewGoblin:
		case Race::Goblin2:
			return "Goblin";
		case Race::Luclin2:
			return "God - Luclin(Ver. 2)";
		case Race::Luclin3:
			return "God - Luclin(Ver. 3)";
		case Race::Luclin4:
			return "God - Luclin(Ver. 4)";
		case Race::GodOfDiscord:
			return "God of Discord";
		case Race::Golem:
		case Race::Golem2:
			return "Golem";
		case Race::Goo:
		case Race::Goo2:
		case Race::Goo3:
		case Race::Goo4:
			return "Goo";
		case Race::Goral:
			return "Goral";
		case Race::GoralMount:
			return "Goral Mount";
		case Race::Gorgon:
			return "Gorgon";
		case Race::Gorilla:
		case Race::Gorilla2:
			return "Gorilla";
		case Race::GrandfatherClock:
			return "Grandfather Clock";
		case Race::GrekenYoung:
			return "Greken - Young";
		case Race::GrekenYoungAdult:
			return "Greken - Young Adult";
		case Race::Grendlaen:
			return "Grendlaen";
		case Race::GriegVeneficus:
			return "Grieg Veneficus";
		case Race::Griffin:
		case Race::Griffin2:
			return "Griffin";
		case Race::Grimling:
			return "Grimling";
		case Race::GroundShaker:
			return "Ground Shaker";
		case Race::FreeportGuard:
		case Race::Felguard:
		case Race::Fayguard:
		case Race::VahShirGuard:
			return "Guard";
		case Race::GuardOfJustice:
			return "Guard of Justice";
		case Race::GuardianCpu:
			return "Guardian CPU";
		case Race::Hadal:
			return "Hadal";
		case Race::Hag:
			return "Hag";
		case Race::HalfElf:
			return "Half Elf";
		case Race::Halfling:
		case Race::RivervaleCitizen:
			return "Halfling";
		case Race::Harpy:
		case Race::Harpy2:
			return "Harpy";
		case Race::HighElf:
			return "High Elf";
		case Race::Hippogriff:
			return "Hippogriff";
		case Race::Holgresh:
		case Race::Holgresh2:
			return "Holgresh";
		case Race::HoneyPot:
			return "Honey Pot";
		case Race::Horse:
		case Race::Horse2:
		case Race::Horse3:
			return "Horse";
		case Race::HoveringPlatform:
			return "Hovering Platform";
		case Race::Hraquis:
			return "Hraquis";
		case Race::Human:
		case Race::HighpassCitizen:
		case Race::QeynosCitizen:
		case Race::Human2:
			return "Human";
		case Race::HumanGhost:
			return "Human Ghost";
		case Race::Huvul:
			return "Huvul";
		case Race::HydraCrystal:
			return "Hydra Crystal";
		case Race::HydraMount:
			return "Hydra Mount";
		case Race::Hydra:
			return "Hydra NPC";
		case Race::Hynid:
			return "Hynid";
		case Race::IceSpectre:
			return "Ice Spectre";
		case Race::Ikaav:
			return "Ikaav";
		case Race::Iksar:
		case Race::IksarCitizen:
			return "Iksar";
		case Race::IksarGhost:
			return "Iksar Ghost";
		case Race::IksarGolem:
			return "Iksar Golem";
		case Race::IksarSkeleton:
			return "Iksar Skeleton";
		case Race::IksarSpirit:
			return "Iksar Spirit";
		case Race::Imp:
			return "Imp";
		case Race::Innoruuk:
			return "Innoruuk";
		case Race::Insect:
			return "Insect";
		case Race::InteractiveObject:
			return "Interactive Object";
		case Race::InvisibleMan:
		case Race::InvisibleMan2:
		case Race::InvisibleMan3:
			return "Invisible Man";
		case Race::InvisibleManOfZomm:
			return "Invisible Man of Zomm";
		case Race::Ixt:
			return "Ixt";
		case Race::Jokester:
			return "Jokester";
		case Race::JumJumBucket:
			return "Jum Jum Bucket";
		case Race::JunkBeast:
			return "Junk Beast";
		case Race::Kangon:
			return "Kangon";
		case Race::KangonMount:
			return "Kangon Mount";
		case Race::Karana:
			return "Karana";
		case Race::PhinigelAutropos:
		case Race::Kedge:
			return "Kedge";
		case Race::Kerran:
		case Race::Kerran2:
			return "Kerran";
		case Race::Kirin:
		case Race::Kirin2:
			return "Kirin";
		case Race::KnightOfHate:
			return "Knight of Hate";
		case Race::KnightOfPestilence:
			return "Knight of Pestilence";
		case Race::Kobold:
		case Race::Kobold2:
			return "Kobold";
		case Race::Kraken:
			return "Kraken";
		case Race::Kyv:
			return "Kyv";
		case Race::Launch:
			return "Launch";
		case Race::LavaRock:
			return "Lava Rock";
		case Race::LavaSpider:
			return "Lava Spider";
		case Race::LavaSpiderQueen:
			return "Lava Spider Queen";
		case Race::Leech:
			return "Leech";
		case Race::Lepertoloth:
			return "Lepertoloth";
		case Race::Lightcrawler:
			return "Lightcrawler";
		case Race::LightingWarrior:
			return "Lightning Warrior";
		case Race::Lion:
			return "Lion";
		case Race::LizardMan:
			return "Lizard Man";
		case Race::Luclin:
			return "Luclin";
		case Race::Luggald:
		case Race::Luggald2:
			return "Luggald";
		case Race::Luggald3:
			return "Luggalds";
		case Race::Malarian:
			return "Malarian";
		case Race::Mammoth:
		case Race::Mammoth2:
			return "Mammoth";
		case Race::ManEatingPlant:
			return "Man - Eating Plant";
		case Race::Mansion:
			return "Mansion";
		case Race::Manticore:
			return "Manticore";
		case Race::Unknown5:
			return "Mantrap";
		case Race::Marionette:
			return "Marionette";
		case Race::Mastruq:
			return "Mastruq";
		case Race::MataMuram:
			return "Mata Muram";
		case Race::Toolbox:
			return "Medium Plant";
		case Race::Mephit:
			return "Mephit";
		case Race::MerchantShip:
			return "Merchant Ship";
		case Race::Mermaid:
			return "Mermaid";
		case Race::Mimic:
			return "Mimic";
		case Race::MiniPom:
			return "Mini POM";
		case Race::Minotaur:
		case Race::Minotaur2:
		case Race::Minotaur3:
		case Race::Minotaur4:
			return "Minotaur";
		case Race::MithanielMarr:
			return "Mithaniel Marr";
		case Race::MorellThule:
			return "Morell Thule";
		case Race::Mosquito:
			return "Mosquito";
		case Race::MouthOfInsanity:
			return "Mouth of Insanity";
		case Race::Muddite:
			return "Muddite";
		case Race::Mummy:
			return "Mummy";
		case Race::MuramiteArmorPile:
			return "Muramite Armor Pile";
		case Race::Murkglider:
			return "Murkglider";
		case Race::MurkgliderEggSack:
			return "Murkglider Egg Sac";
		case Race::Mutna:
			return "Mutna";
		case Race::Nekhon:
			return "Nekhon";
		case Race::Netherbian:
			return "Netherbian";
		case Race::Nightmare:
			return "Nightmare";
		case Race::NightmareGargoyle:
			return "Nightmare Gargoyle";
		case Race::NightmareGoblin:
			return "Nightmare Goblin";
		case Race::NightmareMephit:
			return "Nightmare Mephit";
		case Race::Unicorn2:
		case Race::Unicorn3:
			return "Nightmare / Unicorn";
		case Race::NightmareWraith:
			return "Nightmare Wraith";
		case Race::Nihil:
			return "Nihil";
		case Race::Nilborien:
			return "Nilborien";
		case Race::Noc:
			return "Noc";
		case Race::Nymph:
			return "Nymph";
		case Race::Ogre:
		case Race::OggokCitizen:
			return "Ogre";
		case Race::Ogre2:
			return "Ogre NPC - Male";
		case Race::Orb:
			return "Orb";
		case Race::Orc:
		case Race::Orc2:
			return "Orc";
		case Race::Othmir:
			return "Othmir";
		case Race::Owlbear:
			return "Owlbear";
		case Race::ParasiticScavenger:
			return "Parasitic Scavenger";
		case Race::Pegasus:
		case Race::Pegasus2:
		case Race::Pegasus3:
			return "Pegasus";
		case Race::Phoenix:
			return "Phoenix";
		case Race::Piranha:
			return "Piranha";
		case Race::OneArmedPirate:
		case Race::SpiritmasterNadox:
		case Race::BrokenSkullTaskmaster:
		case Race::GnomePirate:
		case Race::DarkElfPirate:
		case Race::OgrePirate:
		case Race::HumanPirate:
		case Race::EruditePirate:
			return "Pirate";
		case Race::PirateShip:
			return "Pirate Ship";
		case Race::Pixie:
			return "Pixie";
		case Race::PoisonFrog:
			return "Poison Frog";
		case Race::Portal2:
			return "Portal";
		case Race::PowderKeg:
			return "Powder Keg";
		case Race::PressurePlate:
			return "Pressure Plate";
		case Race::PufferSpore:
			return "Puffer Spore";
		case Race::Puma:
		case Race::Puma2:
		case Race::Puma3:
			return "Puma";
		case Race::Pusling:
			return "Pusling";
		case Race::Pyrilen:
			return "Pyrilen";
		case Race::Ratuk:
			return "Ra`tuk";
		case Race::Rabbit:
			return "Rabbit";
		case Race::StatueOfRallosZek:
		case Race::NewRallosZek:
			return "Rallos Zek";
		case Race::RallosOgre:
			return "Rallos Zek Minion";
		case Race::Raptor:
		case Race::Raptor2:
			return "Raptor";
		case Race::RaptorMount:
			return "Raptor Mount";
		case Race::GiantRat:
		case Race::Rat:
			return "Rat";
		case Race::RatMount:
			return "Rat Mount";
		case Race::Ratman:
		case Race::Ratman2:
			return "Ratman";
		case Race::ReanimatedHand:
			return "Reanimated Hand";
		case Race::Recuso:
			return "Recuso";
		case Race::RegenerationPool:
			return "Regeneration Pool";
		case Race::RelicCase:
			return "Relic case ";
		case Race::RhinoBeetle:
			return "Rhino Beetle";
		case Race::Rhinoceros:
			return "Rhinoceros";
		case Race::RobocopterOfZomm:
			return "Robocopter of Zomm";
		case Race::RockPile:
			return "Rock Pile";
		case Race::Rockhopper:
			return "Rockhopper";
		case Race::RonnieTest:
			return "Ronnie Test";
		case Race::RootTentacle:
			return "Root Tentacle";
		case Race::RotDogMount:
			return "Rot Dog Mount";
		case Race::RotDog:
			return "Rotdog";
		case Race::Rotocopter:
			return "Rotocopter";
		case Race::Rowboat:
			return "Rowboat";
		case Race::RoyalGuard:
			return "Royal Guard";
		case Race::RujarkianOrc:
		case Race::MasterOrc:
			return "Rujarkian Orc";
		case Race::RunedOrb:
			return "Runed Orb";
		case Race::RunicSymbol:
			return "Runic Symbol";
		case Race::Sabertooth:
			return "Saber - toothed Cat";
		case Race::SaltpetterBomb:
			return "Saltpetter Bomb";
		case Race::SandElf:
			return "Sand Elf";
		case Race::Sandman:
			return "Sandman";
		case Race::Sarnak:
		case Race::Sarnak2:
			return "Sarnak";
		case Race::SarnakGolem:
			return "Sarnak Golem";
		case Race::SarnakSpirit:
			return "Sarnak Spirit";
		case Race::Saryrn:
			return "Saryrn";
		case Race::Satyr:
			return "Satyr";
		case Race::ScaledWolf:
			return "Scaled Wolf";
		case Race::Scarecrow:
		case Race::Scarecrow2:
			return "Scarecrow";
		case Race::ScarletCheetah:
			return "Scarlet Cheetah";
		case Race::ScleraMount:
			return "Sclera Mount";
		case Race::Scorpion:
		case Race::IksarScorpion:
		case Race::Scorpion2:
			return "Scorpion";
		case Race::Scrykin:
			return "Scrykin";
		case Race::SeaTurtle:
			return "Sea Turtle";
		case Race::SeaHorse:
			return "Seahorse";
		case Race::Selyrah:
			return "Selyrah";
		case Race::SelyrahMount:
			return "Selyrah Mount";
		case Race::LordInquisitorSeru:
			return "Seru";
		case Race::ServantOfShadow:
			return "Servant of Shadow";
		case Race::SessiloidMount:
			return "Sessiloid Mount";
		case Race::Shade:
		case Race::Shade2:
		case Race::Shade3:
			return "Shade";
		case Race::KhatiSha:
			return "Shadel";
		case Race::ShamblingMound:
			return "Shambling Mound";
		case Race::Shark:
			return "Shark";
		case Race::Shiknar:
			return "Shik'Nar";
		case Race::Shiliskin:
			return "Shiliskin";
		case Race::Ship:
			return "Ship";
		case Race::ShipInABottle:
			return "Ship in a Bottle";
		case Race::Shissar:
		case Race::Shissar2:
			return "Shissar";
		case Race::Shrieker:
			return "Shrieker";
		case Race::Siren:
		case Race::Siren2:
			return "Siren";
		case Race::SkeletalHorse:
			return "Skeletal Horse";
		case Race::Skeleton:
		case Race::Skeleton2:
		case Race::Skeleton3:
			return "Skeleton";
		case Race::Skunk:
			return "Skunk";
		case Race::Skystrider:
			return "Skystrider";
		case Race::Plant2:
			return "Small Plant";
		case Race::GiantSnake:
		case Race::Snake:
			return "Snake";
		case Race::SnakeElemental:
			return "Snake Elemental";
		case Race::SnowDervish:
			return "Snow Dervish";
		case Race::SnowRabbit:
			return "Snow Rabbit";
		case Race::Sokokar:
			return "Sokokar";
		case Race::SokokarMount:
			return "Sokokar Mount";
		case Race::SokokarMount2:
			return "Sokokar(w saddle)";
		case Race::SolusekRo:
		case Race::SolusekRo2:
			return "Solusek Ro";
		case Race::SolusekRoGuard:
			return "Solusek Ro Guard";
		case Race::SonicWolf:
			return "Sonic Wolf";
		case Race::SoulDevourer:
			return "Soul Devourer";
		case Race::Spectre:
		case Race::Spectre2:
			return "Spectre";
		case Race::SpellParticle:
			return "Spell Particle 1";
		case Race::Sphinx:
		case Race::Sphinx2:
			return "Sphinx";
		case Race::GiantSpider:
		case Race::Spider:
			return "Spider";
		case Race::SpiderEggSack:
			return "Spider Egg Sack";
		case Race::SpiderMount:
			return "Spider Mount";
		case Race::SpiderQueen:
			return "Spider Queen";
		case Race::SpikeTrap:
			return "Spike Trap";
		case Race::SpiritWolf:
			return "Spirit Wolf";
		case Race::Sporali:
			return "Sporali";
		case Race::StoneJug:
			return "Stone Jug";
		case Race::StonePylon:
			return "Stone Pylon";
		case Race::StoneRing:
			return "Stone Ring";
		case Race::StoneWorker:
		case Race::StoneWorker2:
			return "Stone Worker";
		case Race::Stonegrabber:
			return "Stonegrabber";
		case Race::Stonemite:
			return "Stonemite";
		case Race::Stormrider:
			return "Stormrider";
		case Race::Succubus:
			return "Succubus";
		case Race::Succulent:
			return "Succulent";
		case Race::SullonZek:
			return "Sullon Zek";
		case Race::Shadel:
			return "Sun Revenant";
		case Race::Sunflower:
			return "Sunflower";
		case Race::Swinetor:
			return "Swinetor";
		case Race::Swordfish:
			return "Swordfish";
		case Race::Synarcana:
			return "Synarcana";
		case Race::Table:
			return "Table";
		case Race::FroglokTadpole:
			return "Tadpole";
		case Race::Taelosian:
			return "Taelosian";
		case Race::WineCask:
			return "Tall Plant";
		case Race::TallonZek:
			return "Tallon Zek";
		case Race::Taneth:
			return "Taneth";
		case Race::TarewMarr:
			return "Tarew Marr";
		case Race::Tegi:
			return "Tegi";
		case Race::TeleportMan:
			return "Teleport Man";
		case Race::TeleportationStand:
			return "Teleportation Stand";
		case Race::Telmira:
			return "Telmira";
		case Race::TentacleTerror:
		case Race::TentacleTerror2:
			return "Tentacle Terror";
		case Race::TerrisThule:
			return "Terris Thule";
		case Race::TestObject:
			return "Test Object";
		case Race::Rathe:
			return "The Rathe";
		case Race::TribunalNew:
			return "The Tribunal";
		case Race::ThoughtHorror:
			return "Thought Horror";
		case Race::Tiger:
			return "Tiger";
		case Race::TinSoldier:
			return "Tin Soldier";
		case Race::Plant:
			return "Toolbox";
		case Race::TopiaryLion:
			return "Topiary Lion";
		case Race::TopiaryLionMount:
			return "Topiary Lion Mount";
		case Race::Tormentor:
			return "Tormentor";
		case Race::Totem:
		case Race::Totem2:
			return "Totem";
		case Race::Trakanon:
			return "Trakanon";
		case Race::Tranquilion:
			return "Tranquilion";
		case Race::Treant:
		case Race::Treant2:
		case Race::Treant3:
			return "Treant";
		case Race::Tribunal:
			return "Tribunal";
		case Race::Triumvirate:
			return "Triumvirate";
		case Race::Troll:
		case Race::GrobbCitizen:
		case Race::TrollCrewMember:
		case Race::PirateDeckhand:
		case Race::BrokenSkullPirate:
			return "Troll";
		case Race::TrollZombie:
			return "Troll Zombie";
		case Race::Trusik:
			return "Trusik";
		case Race::Tsetsian:
			return "Tsetsian";
		case Race::Tumbleweed:
			return "Tumbleweed";
		case Race::Tunare:
			return "Tunare";
		case Race::Turepta:
			return "Turepta";
		case Race::Ukun:
			return "Ukun";
		case Race::Ulthork:
			return "Ulthork";
		case Race::UndeadChokidai:
			return "Undead Chokidai";
		case Race::UndeadFootman:
			return "Undead Footman";
		case Race::UndeadFroglok:
			return "Undead Froglok";
		case Race::UndeadIksar:
			return "Undead Iksar";
		case Race::UndeadKnight:
			return "Undead Knight";
		case Race::UndeadSarnak:
			return "Undead Sarnak";
		case Race::UndeadVeksar:
			return "Undead Veksar";
		case Race::Underbulk:
			return "Underbulk";
		case Race::Unicorn:
			return "Unicorn";
		case Race::Doug:
		case Race::MinorIllusion:
		case Race::Tree:
		case Race::Unknown:
		case Race::Unknown2:
		case Race::Unknown3:
		case Race::Unknown4:
			return "UNKNOWN RACE";
		case Race::VahShir:
		case Race::VahShirKing:
			return "Vah Shir";
		case Race::VahShirSkeleton:
			return "Vah Shir Skeleton";
		case Race::VallonZek:
			return "Vallon Zek";
		case Race::Valorian:
		case Race::Valorian2:
			return "Valorian";
		case Race::Vampire:
		case Race::ElfVampire:
		case Race::Vampire2:
		case Race::VampireVolatalis:
		case Race::UndeadVampire:
		case Race::Vampire3:
		case Race::MasterVampire:
		case Race::Vampire4:
			return "Vampire";
		case Race::Vase:
			return "Vase";
		case Race::Vegerog:
			return "Vegerog";
		case Race::Veksar:
		case Race::Veksar2:
		case Race::Veksar3:
			return "Veksar";
		case Race::VenrilSathir:
			return "Venril Sathir";
		case Race::VineMaw:
			return "Vine Maw";
		case Race::Wagon:
			return "Wagon";
		case Race::Walrus:
			return "Walrus";
		case Race::WarBoar:
		case Race::WarBoar2:
			return "War Boar";
		case Race::WarWraith:
			return "War Wraith";
		case Race::Wasp:
			return "Wasp";
		case Race::WaterElemental:
		case Race::WaterElemental2:
			return "Water Elemental";
		case Race::WaterMephit:
			return "Water Mephit";
		case Race::WaterSpout:
			return "Water Spout";
		case Race::WeaponRack:
		case Race::WeaponRack2:
			return "Weapon Rack";
		case Race::Web:
			return "Web";
		case Race::WeddingAltar:
			return "Wedding Altar";
		case Race::WeddingArbor:
			return "Wedding Arbor";
		case Race::WeddingFlowers:
			return "Wedding Flowers";
		case Race::Wereorc:
			return "Wereorc";
		case Race::Werewolf:
		case Race::Werewolf2:
		case Race::Werewolf3:
			return "Werewolf";
		case Race::WetfangMinnow:
			return "Wetfang Minnow";
		case Race::Whirligig:
			return "Whirligig";
		case Race::WickerBasket:
			return "Wicker Basket";
		case Race::Wisp:
			return "Will - O - Wisp";
		case Race::StoneJug2:
		case Race::WineCask2:
			return "Wine Cask";
		case Race::Witheran:
		case Race::Witheran2:
			return "Witheran";
		case Race::Wolf:
		case Race::WolfElemental:
		case Race::Wolf2:
			return "Wolf";
		case Race::WoodElf:
			return "Wood Elf";
		case Race::Worg:
		case Race::Worg2:
			return "Worg";
		case Race::Worm:
			return "Worm";
		case Race::Wretch:
			return "Wretch";
		case Race::Wrulon:
		case Race::Wrulon2:
			return "Wrulon";
		case Race::Wurm:
		case Race::Wurm2:
			return "Wurm";
		case Race::WurmMount:
			return "Wurm Mount";
		case Race::Wyvern:
		case Race::Wyvern2:
			return "Wyvern";
		case Race::Xalgoz:
			return "Xalgoz";
		case Race::Xaric:
			return "Xaric the Unspoken";
		case Race::Xegony:
			return "Xegony";
		case Race::Yakkar:
			return "Yakkar";
		case Race::Yeti:
			return "Yeti";
		case Race::Zebuxoruk:
			return "Zebuxoruk";
		case Race::ZebuxoruksCage:
			return "Zebuxoruk's Cage";
		case Race::Zelniak:
			return "Zelniak";
		case Race::Zombie:
		case Race::Zombie2:
			return "Zombie";
		default:
			return "UNKNOWN RACE";
	}
}

uint32 GetPlayerRaceValue(uint16 race_id)
{
	switch (race_id) {
		case Race::Human:
		case Race::Barbarian:
		case Race::Erudite:
		case Race::WoodElf:
		case Race::HighElf:
		case Race::DarkElf:
		case Race::HalfElf:
		case Race::Dwarf:
		case Race::Troll:
		case Race::Ogre:
		case Race::Halfling:
		case Race::Gnome:
			return race_id;
		case Race::Iksar:
			return RaceIndex::Iksar;
		case Race::VahShir:
			return RaceIndex::VahShir;
		case Race::Froglok2:
			return RaceIndex::Froglok;
		case Race::Drakkin:
			return RaceIndex::Drakkin;
		default:
			return Race::Doug;
	}
}

uint16 GetPlayerRaceBit(uint16 race_id)
{
	switch (race_id) {
		case Race::Human:
			return RaceBitmask::Human;
		case Race::Barbarian:
			return RaceBitmask::Barbarian;
		case Race::Erudite:
			return RaceBitmask::Erudite;
		case Race::WoodElf:
			return RaceBitmask::WoodElf;
		case Race::HighElf:
			return RaceBitmask::HighElf;
		case Race::DarkElf:
			return RaceBitmask::DarkElf;
		case Race::HalfElf:
			return RaceBitmask::HalfElf;
		case Race::Dwarf:
			return RaceBitmask::Dwarf;
		case Race::Troll:
			return RaceBitmask::Troll;
		case Race::Ogre:
			return RaceBitmask::Ogre;
		case Race::Halfling:
			return RaceBitmask::Halfling;
		case Race::Gnome:
			return RaceBitmask::Gnome;
		case Race::Iksar:
			return RaceBitmask::Iksar;
		case Race::VahShir:
			return RaceBitmask::VahShir;
		case Race::Froglok2:
			return RaceBitmask::Froglok;
		case Race::Drakkin:
			return RaceBitmask::Drakkin;
		default:
			return RaceBitmask::Unknown;
	}
}

uint16 GetRaceIDFromPlayerRaceValue(uint32 player_race_value)
{
	switch (player_race_value) {
		case RaceIndex::Human:
		case RaceIndex::Barbarian:
		case RaceIndex::Erudite:
		case RaceIndex::WoodElf:
		case RaceIndex::HighElf:
		case RaceIndex::DarkElf:
		case RaceIndex::HalfElf:
		case RaceIndex::Dwarf:
		case RaceIndex::Troll:
		case RaceIndex::Ogre:
		case RaceIndex::Halfling:
		case RaceIndex::Gnome:
			return player_race_value;
		case RaceIndex::Iksar:
			return Race::Iksar;
		case RaceIndex::VahShir:
			return Race::VahShir;
		case RaceIndex::Froglok:
			return Race::Froglok2;
		case RaceIndex::Drakkin:
			return Race::Drakkin;
		default:
			return Race::Doug;
	}
}

float GetRaceGenderDefaultHeight(int race, int gender)
{
	static float male_height[] = {
	    6.0f,  6.0f,  7.0f,   6.0f,   5.0f,  6.0f,  5.0f,   5.5f,  4.0f,  8.0f,  9.0f,  3.5f,  3.0f,  6.0f,   6.0f,
	    2.0f,  8.5f,  8.0f,   21.0f,  20.0f, 6.0f,  6.0f,   3.5f,  3.0f,  6.0f,  2.0f,  5.0f,  5.0f,  6.0f,   6.0f,
	    6.0f,  7.5f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  5.0f,  6.0f,  6.0f,  7.0f,  4.0f,  4.7f,   6.0f,
	    8.0f,  3.0f,  12.0f,  5.0f,   21.0f, 6.0f,  6.0f,   3.0f,  9.0f,  6.0f,  6.0f,  2.0f,  6.0f,  3.0f,   6.0f,
	    4.0f,  20.0f, 5.0f,   5.0f,   6.0f,  9.0f,  25.0f,  6.0f,  6.0f,  10.0f, 6.0f,  6.0f,  6.0f,  6.0f,   2.5f,
	    7.0f,  6.0f,  5.0f,   6.0f,   1.5f,  1.0f,  3.5f,   7.0f,  6.0f,  6.0f,  6.0f,  6.0f,  7.0f,  3.0f,   3.0f,
	    7.0f,  12.0f, 8.0f,   9.0f,   4.0f,  11.5f, 8.0f,   6.0f,  6.0f,  12.0f, 6.0f,  6.0f,  6.0f,  20.0f,  10.0f,
	    6.5f,  6.0f,  17.0f,  1.0f,   4.0f,  6.0f,  8.0f,   5.0f,  1.0f,  6.0f,  6.0f,  5.0f,  5.0f,  5.0f,   9.0f,
	    3.0f,  8.0f,  2.0f,   24.0f,  6.0f,  10.0f, 6.0f,   6.0f,  6.0f,  3.0f,  7.0f,  9.0f,  6.0f,  11.0f,  2.5f,
	    14.0f, 8.0f,  7.0f,   12.0f,  6.0f,  27.0f, 6.0f,   6.0f,  6.0f,  6.0f,  2.0f,  9.0f,  9.0f,  6.0f,   9.0f,
	    3.0f,  3.0f,  6.0f,   6.0f,   10.0f, 6.0f,  6.0f,   15.0f, 15.0f, 9.0f,  7.0f,  6.0f,  6.0f,  7.0f,   8.0f,
	    3.0f,  3.0f,  6.0f,   7.0f,   13.0f, 6.0f,  6.0f,   9.0f,  5.0f,  7.0f,  9.0f,  6.0f,  6.0f,  8.0f,   6.0f,
	    6.0f,  5.5f,  6.0f,   4.0f,   25.0f, 6.0f,  6.0f,   6.0f,  22.0f, 20.0f, 6.0f,  10.0f, 13.5f, 12.0f,  3.0f,
	    30.0f, 6.0f,  6.0f,   35.0f,  1.5f,  8.0f,  3.0f,   6.0f,  2.0f,  6.0f,  6.0f,  5.0f,  2.0f,  7.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   4.0f,   6.0f,  6.0f,  6.0f,   8.0f,  8.0f,  7.0f,  8.0f,  6.0f,  7.0f,  6.0f,   7.0f,
	    6.0f,  10.0f, 3.0f,   6.0f,   8.0f,  9.0f,  15.0f,  5.0f,  10.0f, 7.0f,  6.0f,  7.0f,  6.0f,  7.0f,   7.0f,
	    12.0f, 6.0f,  4.0f,   6.0f,   5.0f,  3.0f,  30.0f,  30.0f, 15.0f, 20.0f, 6.0f,  10.0f, 6.0f,  14.0f,  14.0f,
	    16.0f, 15.0f, 30.0f,  15.0f,  7.5f,  5.0f,  4.0f,   6.0f,  15.0f, 6.5f,  3.0f,  12.0f, 10.0f, 10.5f,  10.0f,
	    7.5f,  6.0f,  6.0f,   12.5f,  9.0f,  20.0f, 2.0f,   10.0f, 25.0f, 8.0f,  6.0f,  6.0f,  10.0f, 18.0f,  45.0f,
	    13.0f, 15.0f, 8.0f,   30.0f,  25.0f, 25.0f, 10.0f,  13.0f, 5.0f,  3.5f,  15.0f, 35.0f, 11.0f, 15.0f,  50.0f,
	    13.0f, 6.0f,  7.0f,   6.0f,   60.0f, 6.0f,  22.0f,  22.0f, 21.0f, 22.0f, 15.0f, 25.0f, 23.0f, 8.0f,   15.0f,
	    10.0f, 6.0f,  7.0f,   6.0f,   12.0f, 9.5f,  6.0f,   12.0f, 12.0f, 12.0f, 15.0f, 4.0f,  5.0f,  105.0f, 20.0f,
	    5.0f,  10.0f, 10.0f,  10.0f,  20.0f, 13.5f, 8.0f,   10.0f, 3.0f,  5.0f,  9.0f,  6.0f,  6.0f,  6.0f,   10.0f,
	    8.0f,  8.0f,  8.0f,   6.0f,   6.0f,  5.0f,  5.0f,   5.0f,  9.0f,  9.0f,  9.0f,  6.0f,  8.5f,  6.0f,   7.0f,
	    8.0f,  7.0f,  11.0f,  6.0f,   7.0f,  9.0f,  8.0f,   6.0f,  8.0f,  6.0f,  6.0f,  6.0f,  6.0f,  9.0f,   10.0f,
	    6.0f,  3.0f,  4.0f,   3.0f,   3.0f,  4.0f,  10.0f,  10.0f, 2.0f,  8.0f,  6.0f,  6.0f,  14.0f, 7.0f,   5.0f,
	    9.0f,  7.0f,  7.0f,   10.0f,  10.0f, 12.0f, 9.0f,   7.0f,  12.0f, 13.0f, 16.0f, 6.0f,  9.0f,  6.0f,   6.0f,
	    10.0f, 25.0f, 15.0f,  6.0f,   25.0f, 6.0f,  6.0f,   8.0f,  11.0f, 6.0f,  9.0f,  2.0f,  6.0f,  5.0f,   4.0f,
	    8.5f,  6.0f,  6.0f,   6.0f,   4.0f,  6.0f,  15.0f,  1.0f,  2.0f,  6.0f,  40.0f, 8.0f,  12.0f, 3.0f,   8.0f,
	    99.0f, 9.0f,  100.0f, 100.0f, 10.0f, 6.0f,  27.5f,  20.0f, 6.0f,  6.0f,  5.0f,  6.0f,  8.0f,  5.0f,   3.0f,
	    11.5f, 25.0f, 80.0f,  20.0f,  9.0f,  8.0f,  5.0f,   4.0f,  7.0f,  10.0f, 6.0f,  11.0f, 8.0f,  5.0f,   6.0f,
	    6.0f,  30.0f, 7.0f,   15.0f,  9.0f,  6.0f,  9.0f,   6.0f,  3.0f,  32.5f, 15.0f, 7.5f,  10.0f, 10.0f,  6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  9.0f,  20.0f,  6.0f,  6.0f,  6.0f,  25.0f, 12.0f, 6.0f,  8.0f,   6.0f,
	    6.0f,  20.0f, 10.0f,  8.0f,   12.0f, 8.0f,  2.0f,   6.0f,  3.0f,  6.0f,  7.0f,  1.5f,  6.0f,  3.0f,   3.0f,
	    3.0f,  3.0f,  2.0f,   3.0f,   3.0f,  6.0f,  6.0f,   6.0f,  4.5f,  7.0f,  6.0f,  7.0f,  6.0f,  22.0f,  8.0f,
	    15.0f, 22.0f, 8.0f,   15.0f,  6.0f,  80.0f, 150.0f, 7.0f,  6.0f,  6.0f,  6.0f,  12.0f, 6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   35.0f, 20.0f, 9.0f,  6.0f,  6.0f,  6.0f,  20.0f,  20.0f,
	    20.0f, 20.0f, 20.0f,  9.0f,   4.0f,  4.0f,  10.0f,  5.0f,  8.0f,  6.0f,  10.0f, 6.0f,  6.0f,  2.0f,   36.0f,
	    14.0f, 7.0f,  250.0f, 6.0f,   9.0f,  6.0f,  7.0f,   4.0f,  6.0f,  8.0f,  6.0f,  23.0f, 6.0f,  6.0f,   6.0f,
	    70.0f, 6.0f,  7.0f,   6.0f,   6.0f,  6.0f,  20.0f,  6.0f,  6.0f,  6.0f,  5.0f,  1.0f,  6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,   6.0f,
	    4.0f,  4.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  10.0f, 6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  7.0f,  7.0f,   7.0f,  7.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,   8.0f,
	    6.0f,  6.0f,  6.0f,   7.0f,   6.0f,  6.0f,  6.0f,   7.5f,  6.0f,  6.0f,  4.0f,  6.0f,  3.0f,  6.0f,   6.0f,
	    1.0f,  9.0f,  7.0f,   8.0f,   7.0f,  8.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  8.0f,
	};

	static float female_height[] = {
	    6.0f,  6.0f,  7.0f,   6.0f,   5.0f,  6.0f,  5.0f,   5.5f,  4.0f,  8.0f,  9.0f,  3.5f,  3.0f,  6.0f,   6.0f,
	    2.0f,  8.5f,  8.0f,   21.0f,  20.0f, 6.0f,  6.0f,   3.5f,  3.0f,  6.0f,  2.0f,  5.0f,  5.0f,  6.0f,   6.0f,
	    6.0f,  7.5f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  5.0f,  6.0f,  6.0f,  7.0f,  4.0f,  4.7f,   6.0f,
	    8.0f,  3.0f,  12.0f,  5.0f,   21.0f, 6.0f,  6.0f,   3.0f,  9.0f,  6.0f,  6.0f,  2.0f,  6.0f,  3.0f,   6.0f,
	    4.0f,  20.0f, 5.0f,   5.0f,   6.0f,  9.0f,  25.0f,  6.0f,  6.0f,  10.0f, 6.0f,  6.0f,  6.0f,  6.0f,   2.5f,
	    7.0f,  6.0f,  5.0f,   6.0f,   1.5f,  1.0f,  3.5f,   7.0f,  6.0f,  6.0f,  6.0f,  6.0f,  7.0f,  3.0f,   3.0f,
	    7.0f,  12.0f, 8.0f,   9.0f,   4.0f,  11.5f, 8.0f,   6.0f,  6.0f,  12.0f, 6.0f,  6.0f,  6.0f,  20.0f,  10.0f,
	    6.5f,  6.0f,  17.0f,  1.0f,   4.0f,  6.0f,  8.0f,   5.0f,  1.0f,  6.0f,  6.0f,  5.0f,  5.0f,  5.0f,   9.0f,
	    3.0f,  8.0f,  2.0f,   24.0f,  6.0f,  10.0f, 6.0f,   6.0f,  6.0f,  3.0f,  7.0f,  9.0f,  6.0f,  11.0f,  2.5f,
	    14.0f, 8.0f,  7.0f,   12.0f,  6.0f,  27.0f, 6.0f,   6.0f,  6.0f,  6.0f,  2.0f,  9.0f,  9.0f,  6.0f,   9.0f,
	    3.0f,  3.0f,  6.0f,   6.0f,   10.0f, 6.0f,  6.0f,   15.0f, 15.0f, 9.0f,  7.0f,  6.0f,  6.0f,  7.0f,   8.0f,
	    3.0f,  3.0f,  6.0f,   7.0f,   13.0f, 6.0f,  6.0f,   9.0f,  5.0f,  7.0f,  9.0f,  6.0f,  6.0f,  8.0f,   6.0f,
	    6.0f,  5.5f,  6.0f,   4.0f,   25.0f, 6.0f,  6.0f,   6.0f,  22.0f, 20.0f, 6.0f,  10.0f, 13.5f, 12.0f,  3.0f,
	    30.0f, 6.0f,  6.0f,   35.0f,  1.5f,  8.0f,  3.0f,   6.0f,  2.0f,  6.0f,  6.0f,  5.0f,  2.0f,  7.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   4.0f,   6.0f,  6.0f,  6.0f,   8.0f,  8.0f,  7.0f,  8.0f,  6.0f,  7.0f,  6.0f,   7.0f,
	    6.0f,  10.0f, 3.0f,   6.0f,   8.0f,  9.0f,  15.0f,  5.0f,  10.0f, 7.0f,  6.0f,  7.0f,  6.0f,  7.0f,   7.0f,
	    12.0f, 6.0f,  4.0f,   6.0f,   5.0f,  3.0f,  30.0f,  30.0f, 15.0f, 20.0f, 6.0f,  10.0f, 6.0f,  14.0f,  14.0f,
	    16.0f, 15.0f, 30.0f,  15.0f,  7.5f,  5.0f,  4.0f,   6.0f,  15.0f, 6.5f,  3.0f,  12.0f, 10.0f, 10.5f,  10.0f,
	    7.5f,  6.0f,  6.0f,   12.5f,  9.0f,  20.0f, 2.0f,   10.0f, 25.0f, 8.0f,  6.0f,  6.0f,  10.0f, 18.0f,  45.0f,
	    13.0f, 15.0f, 8.0f,   30.0f,  25.0f, 25.0f, 10.0f,  13.0f, 5.0f,  3.5f,  15.0f, 35.0f, 11.0f, 15.0f,  50.0f,
	    13.0f, 6.0f,  7.0f,   6.0f,   60.0f, 6.0f,  22.0f,  22.0f, 21.0f, 22.0f, 15.0f, 25.0f, 23.0f, 8.0f,   15.0f,
	    10.0f, 6.0f,  7.0f,   6.0f,   12.0f, 9.5f,  6.0f,   12.0f, 12.0f, 12.0f, 15.0f, 4.0f,  5.0f,  105.0f, 20.0f,
	    5.0f,  10.0f, 10.0f,  10.0f,  20.0f, 13.5f, 8.0f,   10.0f, 3.0f,  5.0f,  9.0f,  6.0f,  6.0f,  6.0f,   10.0f,
	    8.0f,  8.0f,  8.0f,   6.0f,   6.0f,  5.0f,  5.0f,   5.0f,  9.0f,  9.0f,  9.0f,  6.0f,  8.5f,  6.0f,   7.0f,
	    8.0f,  7.0f,  11.0f,  6.0f,   7.0f,  9.0f,  8.0f,   6.0f,  8.0f,  6.0f,  6.0f,  6.0f,  6.0f,  9.0f,   10.0f,
	    6.0f,  3.0f,  4.0f,   3.0f,   3.0f,  4.0f,  10.0f,  10.0f, 2.0f,  8.0f,  6.0f,  6.0f,  14.0f, 7.0f,   5.0f,
	    9.0f,  7.0f,  7.0f,   10.0f,  10.0f, 12.0f, 9.0f,   7.0f,  12.0f, 13.0f, 16.0f, 6.0f,  9.0f,  6.0f,   6.0f,
	    10.0f, 25.0f, 15.0f,  6.0f,   25.0f, 6.0f,  6.0f,   8.0f,  11.0f, 6.0f,  9.0f,  2.0f,  6.0f,  5.0f,   4.0f,
	    8.5f,  6.0f,  6.0f,   6.0f,   4.0f,  6.0f,  15.0f,  1.0f,  2.0f,  6.0f,  40.0f, 8.0f,  12.0f, 3.0f,   8.0f,
	    99.0f, 9.0f,  100.0f, 100.0f, 10.0f, 6.0f,  27.5f,  20.0f, 6.0f,  6.0f,  5.0f,  6.0f,  8.0f,  5.0f,   3.0f,
	    11.5f, 25.0f, 80.0f,  20.0f,  9.0f,  8.0f,  5.0f,   4.0f,  7.0f,  10.0f, 6.0f,  11.0f, 8.0f,  5.0f,   6.0f,
	    6.0f,  30.0f, 7.0f,   15.0f,  9.0f,  6.0f,  9.0f,   6.0f,  3.0f,  32.5f, 15.0f, 7.5f,  10.0f, 10.0f,  6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  9.0f,  20.0f,  6.0f,  6.0f,  6.0f,  25.0f, 12.0f, 6.0f,  8.0f,   6.0f,
	    6.0f,  20.0f, 9.0f,   8.0f,   12.0f, 8.0f,  2.0f,   6.0f,  3.0f,  6.0f,  7.0f,  1.5f,  6.0f,  3.0f,   3.0f,
	    3.0f,  3.0f,  2.0f,   3.0f,   3.0f,  6.0f,  6.0f,   6.0f,  4.5f,  7.0f,  6.0f,  7.0f,  5.7f,  22.0f,  8.0f,
	    15.0f, 22.0f, 8.0f,   15.0f,  6.0f,  80.0f, 150.0f, 7.0f,  6.0f,  6.0f,  6.0f,  12.0f, 6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   35.0f, 20.0f, 9.0f,  6.0f,  6.0f,  6.0f,  20.0f,  20.0f,
	    20.0f, 20.0f, 20.0f,  9.0f,   4.0f,  4.0f,  10.0f,  5.0f,  8.0f,  6.0f,  10.0f, 5.7f,  6.0f,  2.0f,   36.0f,
	    14.0f, 7.0f,  250.0f, 6.0f,   9.0f,  6.0f,  7.0f,   4.0f,  6.0f,  8.0f,  6.0f,  23.0f, 6.0f,  6.0f,   6.0f,
	    70.0f, 6.0f,  7.0f,   6.0f,   6.0f,  6.0f,  20.0f,  6.0f,  6.0f,  6.0f,  5.0f,  1.0f,  6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,   6.0f,
	    4.0f,  4.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  10.0f, 6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  7.0f,  7.0f,   7.0f,  7.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,   8.0f,
	    6.0f,  6.0f,  6.0f,   7.0f,   6.0f,  6.0f,  6.0f,   7.5f,  6.0f,  6.0f,  4.0f,  6.0f,  3.0f,  6.0f,   6.0f,
	    1.0f,  9.0f,  7.0f,   8.0f,   7.0f,  8.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  8.0f,
	};

	const auto size = sizeof(male_height) / sizeof(male_height[0]);

	if (race >= size) {
		return 6.0f;
	}

	if (gender == Gender::Female) {
		return female_height[race];
	}

	return male_height[race];
}

#define GetRaceGenderMask(race_id, gender_id) (((int)race_id << 8) | gender_id)

bool RaceAppearance::IsValidBeard(uint16 race_id, uint8 gender_id, uint8 beard_value, bool use_luclin)
{
	if (beard_value == std::numeric_limits<uint8>::max()) {
		return true;
	}

	if (use_luclin) {
		switch (GetRaceGenderMask(race_id, gender_id)) {
			case RaceAppearance::DwarfFemale:
				if (beard_value <= 1) {
					return true;
				}

				break;
			case RaceAppearance::HighElfMale:
			case RaceAppearance::DarkElfMale:
			case RaceAppearance::HalfElfMale:
			case RaceAppearance::DrakkinFemale:
				if (beard_value <= 3) {
					return true;
				}

				break;
			case RaceAppearance::HumanMale:
			case RaceAppearance::BarbarianMale:
			case RaceAppearance::EruditeMale:
			case RaceAppearance::DwarfMale:
			case RaceAppearance::HalflingMale:
			case RaceAppearance::GnomeMale:
				if (beard_value <= 5) {
					return true;
				}

				break;
			case RaceAppearance::DrakkinMale:
				if (beard_value <= 11) {
					return true;
				}

				break;
			default:
				break;
		}

		return false;
	} else {
		switch (GetRaceGenderMask(race_id, gender_id)) {
			case RaceAppearance::DrakkinFemale:
				if (beard_value <= 3) {
					return true;
				}

				break;
			case RaceAppearance::DrakkinMale:
				if (beard_value <= 11) {
					return true;
				}

				break;
			default:
				break;
		}

		return false;
	}
}

bool RaceAppearance::IsValidBeardColor(uint16 race_id, uint8 gender_id, uint8 beard_color_value, bool use_luclin)
{
	if (beard_color_value == std::numeric_limits<uint8>::max()) {
		return true;
	}

	switch (GetRaceGenderMask(race_id, gender_id)) {
		case RaceAppearance::GnomeMale:
			if (beard_color_value <= 24) {
				return true;
			}

			break;
		case RaceAppearance::HumanMale:
		case RaceAppearance::BarbarianMale:
		case RaceAppearance::EruditeMale:
		case RaceAppearance::HalfElfMale:
		case RaceAppearance::DwarfMale:
		case RaceAppearance::DwarfFemale:
		case RaceAppearance::HalflingMale:
			if (beard_color_value <= 19) {
				return true;
			}

			break;
		case RaceAppearance::DarkElfMale:
			if (EQ::ValueWithin(beard_color_value, 13, 18)) {
				return true;
			}

			break;
		case RaceAppearance::HighElfMale:
			if (beard_color_value <= 14) {
				return true;
			}

			break;
		case RaceAppearance::FroglokMale:
		case RaceAppearance::FroglokFemale:
		case RaceAppearance::DrakkinMale:
		case RaceAppearance::DrakkinFemale:
			if (beard_color_value <= 3) {
				return true;
			}

			break;
		default:
			break;
	}

	return false;
}

bool RaceAppearance::IsValidDetail(uint16 race_id, uint8 gender_id, uint32 detail_value, bool use_luclin)
{
	if (detail_value == std::numeric_limits<uint32>::max()) {
		return true;
	}

	switch (GetRaceGenderMask(race_id, gender_id)) {
		case RaceAppearance::DrakkinMale:
		case RaceAppearance::DrakkinFemale:
			if (detail_value <= 7) {
				return true;
			}

			break;
		default:
			break;
	}

	return false;
}

bool RaceAppearance::IsValidEyeColor(uint16 race_id, uint8 gender_id, uint8 eye_color_value, bool use_luclin)
{
	if (eye_color_value == std::numeric_limits<uint8>::max()) {
		return true;
	}

	switch (GetRaceGenderMask(race_id, gender_id)) {
		case RaceAppearance::HumanMale:
		case RaceAppearance::HumanFemale:
		case RaceAppearance::BarbarianMale:
		case RaceAppearance::BarbarianFemale:
		case RaceAppearance::EruditeMale:
		case RaceAppearance::EruditeFemale:
		case RaceAppearance::WoodElfMale:
		case RaceAppearance::WoodElfFemale:
		case RaceAppearance::HighElfMale:
		case RaceAppearance::HighElfFemale:
		case RaceAppearance::DarkElfMale:
		case RaceAppearance::DarkElfFemale:
		case RaceAppearance::HalfElfMale:
		case RaceAppearance::HalfElfFemale:
		case RaceAppearance::DwarfMale:
		case RaceAppearance::DwarfFemale:
		case RaceAppearance::OgreMale:
		case RaceAppearance::OgreFemale:
		case RaceAppearance::HalflingMale:
		case RaceAppearance::HalflingFemale:
		case RaceAppearance::GnomeMale:
		case RaceAppearance::GnomeFemale:
		case RaceAppearance::IksarMale:
		case RaceAppearance::IksarFemale:
		case RaceAppearance::VahShirMale:
		case RaceAppearance::VahShirFemale:
			if (eye_color_value <= 9) {
				return true;
			}

			break;
		case RaceAppearance::TrollMale:
		case RaceAppearance::TrollFemale:
			if (eye_color_value <= 10) {
				return true;
			}

			break;
		case RaceAppearance::FroglokMale:
		case RaceAppearance::FroglokFemale:
		case RaceAppearance::DrakkinMale:
		case RaceAppearance::DrakkinFemale:
			if (eye_color_value <= 11) {
				return true;
			}

			break;
		default:
			break;
	}

	return false;
}

bool RaceAppearance::IsValidFace(uint16 race_id, uint8 gender_id, uint8 face_value, bool use_luclin)
{
	if (face_value == std::numeric_limits<uint8>::max()) {
		return true;
	}

	switch (GetRaceGenderMask(race_id, gender_id)) {
		case RaceAppearance::DrakkinMale:
		case RaceAppearance::DrakkinFemale:
			if (face_value <= 6) {
				return true;
			}

			break;
		case RaceAppearance::HumanMale:
		case RaceAppearance::HumanFemale:
		case RaceAppearance::BarbarianMale:
		case RaceAppearance::BarbarianFemale:
		case RaceAppearance::EruditeMale:
		case RaceAppearance::EruditeFemale:
		case RaceAppearance::WoodElfMale:
		case RaceAppearance::WoodElfFemale:
		case RaceAppearance::HighElfMale:
		case RaceAppearance::HighElfFemale:
		case RaceAppearance::DarkElfMale:
		case RaceAppearance::DarkElfFemale:
		case RaceAppearance::HalfElfMale:
		case RaceAppearance::HalfElfFemale:
		case RaceAppearance::DwarfMale:
		case RaceAppearance::DwarfFemale:
		case RaceAppearance::TrollMale:
		case RaceAppearance::TrollFemale:
		case RaceAppearance::OgreMale:
		case RaceAppearance::OgreFemale:
		case RaceAppearance::HalflingMale:
		case RaceAppearance::HalflingFemale:
		case RaceAppearance::GnomeMale:
		case RaceAppearance::GnomeFemale:
		case RaceAppearance::IksarMale:
		case RaceAppearance::IksarFemale:
		case RaceAppearance::VahShirMale:
		case RaceAppearance::VahShirFemale:
			if (face_value <= 7) {
				return true;
			}

			break;
		case RaceAppearance::FroglokMale:
		case RaceAppearance::FroglokFemale:
			if (face_value <= 9) {
				return true;
			}

			break;
		default:
			break;
	}

	return false;
}

bool RaceAppearance::IsValidHair(uint16 race_id, uint8 gender_id, uint8 hair_value, bool use_luclin)
{
	if (hair_value == std::numeric_limits<uint8>::max()) {
		return true;
	}

	if (use_luclin) {
		switch (GetRaceGenderMask(race_id, gender_id)) {
			case RaceAppearance::HumanMale:
			case RaceAppearance::HumanFemale:
			case RaceAppearance::BarbarianMale:
			case RaceAppearance::BarbarianFemale:
			case RaceAppearance::WoodElfMale:
			case RaceAppearance::WoodElfFemale:
			case RaceAppearance::HighElfMale:
			case RaceAppearance::HighElfFemale:
			case RaceAppearance::DarkElfMale:
			case RaceAppearance::DarkElfFemale:
			case RaceAppearance::HalfElfMale:
			case RaceAppearance::HalfElfFemale:
			case RaceAppearance::DwarfMale:
			case RaceAppearance::DwarfFemale:
			case RaceAppearance::TrollFemale:
			case RaceAppearance::OgreFemale:
			case RaceAppearance::HalflingMale:
			case RaceAppearance::HalflingFemale:
			case RaceAppearance::GnomeMale:
			case RaceAppearance::GnomeFemale:
				if (hair_value <= 3) {
					return true;
				}

				break;
			case RaceAppearance::EruditeMale:
				if (hair_value <= 5) {
					return true;
				}

				break;
			case RaceAppearance::DrakkinFemale:
				if (hair_value <= 7) {
					return true;
				}

				break;
			case RaceAppearance::EruditeFemale:
			case RaceAppearance::DrakkinMale:
				if (hair_value <= 8) {
					return true;
				}

				break;
			default:
				break;
		}

		return false;
	} else {
		switch (GetRaceGenderMask(race_id, gender_id)) {
			case RaceAppearance::DrakkinFemale:
				if (hair_value <= 7) {
					return true;
				}

				break;
			case RaceAppearance::DrakkinMale:
				if (hair_value <= 8) {
					return true;
				}

				break;
			default:
				break;
		}

		return false;
	}
}

bool RaceAppearance::IsValidHairColor(uint16 race_id, uint8 gender_id, uint8 hair_color_value, bool use_luclin)
{
	if (hair_color_value == std::numeric_limits<uint8>::max()) {
		return true;
	}

	switch (GetRaceGenderMask(race_id, gender_id)) {
		case RaceAppearance::GnomeMale:
		case RaceAppearance::GnomeFemale:
			if (hair_color_value <= 24) {
				return true;
			}

			break;
		case RaceAppearance::TrollFemale:
		case RaceAppearance::OgreFemale:
			if (hair_color_value <= 23) {
				return true;
			}

			break;
		case RaceAppearance::HumanMale:
		case RaceAppearance::HumanFemale:
		case RaceAppearance::BarbarianMale:
		case RaceAppearance::BarbarianFemale:
		case RaceAppearance::WoodElfMale:
		case RaceAppearance::WoodElfFemale:
		case RaceAppearance::HalfElfMale:
		case RaceAppearance::HalfElfFemale:
		case RaceAppearance::DwarfMale:
		case RaceAppearance::DwarfFemale:
		case RaceAppearance::HalflingMale:
		case RaceAppearance::HalflingFemale:
			if (hair_color_value <= 19) {
				return true;
			}

			break;
		case RaceAppearance::DarkElfMale:
		case RaceAppearance::DarkElfFemale:
			if (EQ::ValueWithin(hair_color_value, 13, 18)) {
				return true;
			}

			break;
		case RaceAppearance::HighElfMale:
		case RaceAppearance::HighElfFemale:
			if (hair_color_value <= 14) {
				return true;
			}

			break;
		case RaceAppearance::FroglokMale:
		case RaceAppearance::FroglokFemale:
		case RaceAppearance::DrakkinMale:
		case RaceAppearance::DrakkinFemale:
			if (hair_color_value <= 3) {
				return true;
			}

			break;
		default:
			break;
	}

	return false;
}

bool RaceAppearance::IsValidHeritage(uint16 race_id, uint8 gender_id, uint32 heritage_value, bool use_luclin)
{
	if (heritage_value == std::numeric_limits<uint32>::max()) {
		return true;
	}

	switch (GetRaceGenderMask(race_id, gender_id)) {
		case RaceAppearance::DrakkinMale:
		case RaceAppearance::DrakkinFemale:
			if (heritage_value <= 7) {
				return true;
			}

			break;
		default:
			break;
	}

	return false;
}

bool RaceAppearance::IsValidTattoo(uint16 race_id, uint8 gender_id, uint32 tattoo_value, bool use_luclin)
{
	if (tattoo_value == std::numeric_limits<uint32>::max()) {
		return true;
	}

	switch (GetRaceGenderMask(race_id, gender_id)) {
		case RaceAppearance::DrakkinMale:
		case RaceAppearance::DrakkinFemale:
			if (tattoo_value <= 7) {
				return true;
			}

			break;
		default:
			break;
	}

	return false;
}

bool RaceAppearance::IsValidWoad(uint16 race_id, uint8 gender_id, uint8 woad_value, bool use_luclin)
{
	if (woad_value == std::numeric_limits<uint8>::max()) {
		return true;
	}

	if (use_luclin) {
		switch (GetRaceGenderMask(race_id, gender_id)) {
			case RaceAppearance::BarbarianMale:
			case RaceAppearance::BarbarianFemale:
				if (woad_value <= 8) {
					return true;
				}

				break;
			default:
				break;
		}
	}

	return false;
}

const char* GetGenderName(uint32 gender_id) {
	const char* gender_name = "Unknown";
	if (gender_id == Gender::Male) {
		gender_name = "Male";
	} else if (gender_id == Gender::Female) {
		gender_name = "Female";
	} else if (gender_id == Gender::Neuter) {
		gender_name = "Neuter";
	}
	return gender_name;
}

const std::string GetPlayerRaceAbbreviation(uint16 race_id)
{
	if (!IsPlayerRace(race_id)) {
		return std::string("UNK");
	}

	switch (race_id) {
		case Race::Human:
			return "HUM";
		case Race::Barbarian:
			return "BAR";
		case Race::Erudite:
			return "ERU";
		case Race::WoodElf:
			return "ELF";
		case Race::HighElf:
			return "HIE";
		case Race::DarkElf:
			return "DEF";
		case Race::HalfElf:
			return "HEF";
		case Race::Dwarf:
			return "DWF";
		case Race::Troll:
			return "TRL";
		case Race::Ogre:
			return "OGR";
		case Race::Halfling:
			return "HFL";
		case Race::Gnome:
			return "GNM";
		case Race::Iksar:
			return "IKS";
		case Race::VahShir:
			return "VAH";
		case Race::Froglok2:
			return "FRG";
		case Race::Drakkin:
			return "DRK";
	}

	return std::string("UNK");
}

bool IsPlayerRace(uint16 race_id) {
	return (
		EQ::ValueWithin(race_id, Race::Human, Race::Gnome) ||
		race_id == Race::Iksar ||
		race_id == Race::VahShir ||
		race_id == Race::Froglok2 ||
		race_id == Race::Drakkin
	);
}
