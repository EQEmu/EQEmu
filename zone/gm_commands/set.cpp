#include "../client.h"

void SetAAEXP(Client *c, const Seperator *sep);
void SetAAPoints(Client *c, const Seperator *sep);
void SetAdventurePoints(Client *c, const Seperator *sep);
void SetAlternateCurrency(Client *c, const Seperator *sep);
void SetAnimation(Client *c, const Seperator *sep);
void SetAnon(Client *c, const Seperator *sep);
void SetAutoLogin(Client *c, const Seperator *sep);
void SetBindPoint(Client *c, const Seperator *sep);
void SetChecksum(Client *c, const Seperator *sep);
void SetClassPermanent(Client *c, const Seperator *sep);
void SetCrystals(Client *c, const Seperator *sep);
void SetDate(Client *c, const Seperator *sep);
void SetEndurance(Client *c, const Seperator *sep);
void SetEnduranceFull(Client *c, const Seperator *sep);
void SetEXP(Client *c, const Seperator *sep);
void SetFlymode(Client *c, const Seperator *sep);
void SetFrozen(Client *c, const Seperator *sep);
void SetGender(Client *c, const Seperator *sep);
void SetGenderPermanent(Client *c, const Seperator *sep);
void SetGM(Client *c, const Seperator *sep);
void SetGMSpeed(Client *c, const Seperator *sep);
void SetGMStatus(Client *c, const Seperator *sep);
void SetGodMode(Client *c, const Seperator *sep);
void SetHaste(Client *c, const Seperator *sep);
void SetHideMe(Client *c, const Seperator *sep);
void SetHeroModel(Client *c, const Seperator *sep);
void SetHP(Client *c, const Seperator *sep);
void SetHPFull(Client *c, const Seperator *sep);
void SetInvulnerable(Client *c, const Seperator *sep);
void SetLanguage(Client *c, const Seperator *sep);
void SetLastName(Client *c, const Seperator *sep);
void SetLevel(Client *c, const Seperator *sep);
void SetLoginserverInfo(Client *c, const Seperator *sep);
void SetMana(Client *c, const Seperator *sep);
void SetManaFull(Client *c, const Seperator *sep);
void SetMOTD(Client *c, const Seperator *sep);
void SetName(Client *c, const Seperator *sep);
void SetOOCMute(Client *c, const Seperator *sep);
void SetPassword(Client *c, const Seperator *sep);
void SetPVP(Client *c, const Seperator *sep);
void SetPVPPoints(Client *c, const Seperator *sep);
void SetRace(Client *c, const Seperator *sep);
void SetRacePermanent(Client *c, const Seperator *sep);
void SetServerLocked(Client *c, const Seperator *sep);
void SetSkill(Client *c, const Seperator *sep);
void SetSkillAll(Client *c, const Seperator *sep);
void SetSkillAllMax(Client *c, const Seperator *sep);
void SetStartZone(Client *c, const Seperator *sep);
void SetTemporaryName(Client *c, const Seperator *sep);
void SetTexture(Client *c, const Seperator *sep);
void SetTime(Client *c, const Seperator *sep);
void SetTimeZone(Client *c, const Seperator *sep);
void SetTitle(Client *c, const Seperator *sep);
void SetTitleSuffix(Client *c, const Seperator *sep);
void SetWeather(Client *c, const Seperator *sep);
void SetZoneData(Client *c, const Seperator *sep);

void command_set(Client *c, const Seperator *sep)
{
	struct Cmd {
		std::string cmd{}; // command
		std::string u{}; // usage
		void (*fn)(Client *c, const Seperator *sep) = nullptr; // function
		std::vector<std::string> a{}; // aliases
	};

	std::vector<Cmd> commands = {
		Cmd{.cmd = "aa_exp", .u = "aa_exp [aa|group|raid] [Amount]", .fn = SetAAEXP, .a = {"#setaaxp"}},
		Cmd{.cmd = "aa_points", .u = "aa_points [aa|group|raid] [Amount]", .fn = SetAAPoints, .a = {"#setaapts"}},
		Cmd{.cmd = "adventure_points", .u = "adventure_points [Theme ID] [Amount]", .fn = SetAdventurePoints, .a = {"#set_adventure_points"}},
		Cmd{.cmd = "alternate_currency", .u = "alternate_currency [Currency ID] [Amount]", .fn = SetAlternateCurrency, .a = {"#setaltcurrency"}},
		Cmd{.cmd = "animation", .u = "animation [Animation ID]", .fn = SetAnimation, .a = {"#setanim"}},
		Cmd{.cmd = "anon", .u = "anon [Character ID] [Anonymous Flag] or #set anon [Anonymous Flag]", .fn = SetAnon, .a = {"#setanon"}},
		Cmd{.cmd = "auto_login", .u = "auto_login [0|1]", .fn = SetAutoLogin, .a = {"#setautologin"}},
		Cmd{.cmd = "bind_point", .u = "bind_point", .fn = SetBindPoint, .a = {"#setbind"}},
		Cmd{.cmd = "checksum", .u = "checksum", .fn = SetChecksum, .a = {"#updatechecksum"}},
		Cmd{.cmd = "class_permanent", .u = "class_permanent [Class ID]", .fn = SetClassPermanent, .a = {"#permaclass"}},
		Cmd{.cmd = "crystals", .u = "crystals [ebon|radiant] [Amount]", .fn = SetCrystals, .a = {"#setcrystals"}},
		Cmd{.cmd = "date", .u = "date [Year] [Month] [Day] [Hour] [Minute] (Hour and Minute are optional)", .fn = SetDate, .a = {"#date"}},
		Cmd{.cmd = "endurance", .u = "endurance [Amount]", .fn = SetEndurance, .a = {"#setendurance"}},
		Cmd{.cmd = "endurance_full", .u = "endurance_full", .fn = SetEnduranceFull, .a = {"#endurance"}},
		Cmd{.cmd = "exp", .u = "exp [aa|exp] [Amount]", .fn = SetEXP, .a = {"#setxp"}},
		Cmd{.cmd = "flymode", .u = "flymode [Flymode ID]", .fn = SetFlymode, .a = {"#flymode"}},
		Cmd{.cmd = "frozen", .u = "frozen [on|off]", .fn = SetFrozen, .a = {"#freeze", "#unfreeze"}},
		Cmd{.cmd = "gender", .u = "gender [Gender ID]", .fn = SetGender, .a = {"#gender"}},
		Cmd{.cmd = "gender_permanent", .u = "gender_permanent [Gender ID]", .fn = SetGenderPermanent, .a = {"#permagender"}},
		Cmd{.cmd = "gm", .u = "gm [on|off]", .fn = SetGM, .a = {"#gm"}},
		Cmd{.cmd = "gm_speed", .u = "gm_speed [on|off]", .fn = SetGMSpeed, .a = {"#gmspeed"}},
		Cmd{.cmd = "gm_status", .u = "gm_status [GM Status] [Account]", .fn = SetGMStatus, .a = {"#flag"}},
		Cmd{.cmd = "god_mode", .u = "god_mode [on|off]", .fn = SetGodMode, .a = {"#godmode"}},
		Cmd{.cmd = "haste", .u = "haste [Percentage]", .fn = SetHaste, .a = {"#haste"}},
		Cmd{.cmd = "hide_me", .u = "hide_me [on|off]", .fn = SetHideMe, .a = {"#hideme"}},
		Cmd{.cmd = "hero_model", .u = "hero_model [Hero Model] [Slot] (Slot is optional)", .fn = SetHeroModel, .a = {"#heromodel"}},
		Cmd{.cmd = "hp", .u = "hp [Amount]", .fn = SetHP, .a = {"#sethp"}},
		Cmd{.cmd = "hp_full", .u = "hp_full", .fn = SetHPFull, .a = {"#heal"}},
		Cmd{.cmd = "invulnerable", .u = "invulnerable", .fn = SetInvulnerable, .a = {"#invul"}},
		Cmd{.cmd = "language", .u = "language [Language ID] [Language Level]", .fn = SetLanguage, .a = {"#setlanguage"}},
		Cmd{.cmd = "last_name", .u = "last_name [Last Name]", .fn = SetLastName, .a = {"#lastname"}},
		Cmd{.cmd = "level", .u = "level [Level]", .fn = SetLevel, .a = {"#level"}},
		Cmd{.cmd = "loginserver_info", .u = "loginserver_info [Email] [Password]", .fn = SetLoginserverInfo, .a = {"#setlsinfo"}},
		Cmd{.cmd = "mana", .u = "mana [Amount]", .fn = SetMana, .a = {"#setmana"}},
		Cmd{.cmd = "mana_full", .u = "mana_full", .fn = SetManaFull, .a = {"#mana"}},
		Cmd{.cmd = "motd", .u = "motd", .fn = SetMOTD, .a = {"#motd"}},
		Cmd{.cmd = "name", .u = "name", .fn = SetName, .a = {"#name"}},
		Cmd{.cmd = "ooc_mute", .u = "ooc_mute", .fn = SetOOCMute, .a = {"#oocmute"}},
		Cmd{.cmd = "password", .u = "password [Account Name] [Password] (account table password)", .fn = SetPassword, .a = {"#setpass"}},
		Cmd{.cmd = "pvp", .u = "pvp [on|off]", .fn = SetPVP, .a = {"#pvp"}},
		Cmd{.cmd = "pvp_points", .u = "pvp_points [Amount]", .fn = SetPVPPoints, .a = {"#setpvppoints"}},
		Cmd{.cmd = "race", .u = "race [Race ID]", .fn = SetRace, .a = {"#race"}},
		Cmd{.cmd = "race_permanent", .u = "race_permanent [Race ID]", .fn = SetRacePermanent, .a = {"#permarace"}},
		Cmd{.cmd = "server_locked", .u = "server_locked [on|off]", .fn = SetServerLocked, .a = {"#lock", "#serverlock", "#serverunlock", "#unlock"}},
		Cmd{.cmd = "skill", .u = "skill [Skill ID] [Skill Level]", .fn = SetSkill, .a = {"#setskill"}},
		Cmd{.cmd = "skill_all", .u = "skill_all [Skill Level]", .fn = SetSkillAll, .a = {"#setskillall"}},
		Cmd{.cmd = "skill_all_max", .u = "skill_all_max", .fn = SetSkillAllMax, .a = {"#maxskills"}},
		Cmd{.cmd = "start_zone", .u = "endurance [Amount]", .fn = SetStartZone, .a = {"#setstartzone"}},
		Cmd{.cmd = "temporary_name", .u = "temporary_name [Name]", .fn = SetTemporaryName, .a = {"#tempname"}},
		Cmd{.cmd = "texture", .u = "texture [Texture ID]", .fn = SetTexture, .a = {"#texture"}},
		Cmd{.cmd = "time", .u = "time [Hour] [Minute]", .fn = SetTime, .a = {"#time"}},
		Cmd{.cmd = "time_zone", .u = "time_zone [Hour] [Minute]", .fn = SetTimeZone, .a = {"#timezone"}},
		Cmd{.cmd = "title", .u = "title [Title]", .fn = SetTitle, .a = {"#title"}},
		Cmd{.cmd = "title_suffix", .u = "title_suffix [Title Suffix]", .fn = SetTitleSuffix, .a = {"#titlesuffix"}},
		Cmd{.cmd = "temporary_name", .u = "temporary_name [Name]", .fn = SetTemporaryName, .a = {"#tempname"}},
		Cmd{.cmd = "weather", .u = "weather [0|1|2|3]", .fn = SetWeather, .a = {"#weather"}},
		Cmd{.cmd = "zone", .u = "zone [option]", .fn = SetZoneData, .a = {"#zclip", "#zcolor", "#zheader", "#zonelock", "#zsafecoords", "#zsky", "#zunderworld"}},
	};

	// Check for arguments
	const auto arguments = sep->argnum;

	// look for alias or command
	for (const auto &cmd: commands) {
		// Check for alias first
		for (const auto &alias: cmd.a) {
			if (!alias.empty() && Strings::EqualFold(alias, sep->arg[0])) {
				// build string from sep args
				std::vector<std::string> args = {};

				// skip the first arg
				for (auto i = 1; i <= arguments; i++) {
					if (sep->arg[i]) {
						args.emplace_back(sep->arg[i]);
					}
				}

				// build the rewrite string
				const std::string& rewrite = fmt::format("#set {} {}", cmd.cmd, Strings::Join(args, " "));

				// rewrite to #set <sub-command <args>>
				c->SetEntityVariable("old_command", Strings::Replace(alias, "#", ""));
				c->SendGMCommand(rewrite);
				c->DeleteEntityVariable("old_command");

				c->Message(
					Chat::Gray,
					fmt::format(
						"{} is now located under {}, using {}.",
						sep->arg[0],
						Saylink::Silent("#set"),
						Saylink::Silent(rewrite)
					).c_str()
				);

				return;
			}
		}

		// Check for command
		if (cmd.cmd == Strings::ToLower(sep->arg[1])) {
			cmd.fn(c, sep);
			return;
		}
	}

	// Command not found
	c->Message(Chat::White, "Command not found. Usage: #set [command]");
	for (const auto &cmd: commands) {
		c->Message(Chat::White, fmt::format("Usage: #set {}", cmd.u).c_str());
	}
}
