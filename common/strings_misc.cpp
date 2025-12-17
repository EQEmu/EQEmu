#include "strings.h"

#include "fmt/format.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#ifdef _WINDOWS
#include <windows.h>
#define snprintf        _snprintf
#define strncasecmp     _strnicmp
#define strcasecmp      _stricmp
#endif

#ifndef va_copy
#define va_copy(d,s) ((d) = (s))
#endif

std::string SanitizeWorldServerName(std::string server_long_name)
{
	server_long_name.erase(
		std::remove_if(
			server_long_name.begin(),
			server_long_name.end(),
			[](char c) {
				return !(std::isalpha(c) || std::isalnum(c) || std::isspace(c) || IsAllowedWorldServerCharacterList(c));
			}
		), server_long_name.end()
	);

	server_long_name = Strings::Trim(server_long_name);

	// bad word filter
	for (auto &piece: Strings::Split(server_long_name, " ")) {
		for (auto &word: GetBadWords()) {
			// for shorter words that can actually be part of legitimate words
			// make sure that it isn't part of another word by matching on a space
			if (Strings::ToLower(piece) == word) {
				Strings::FindReplace(
					server_long_name,
					piece,
					Strings::Repeat("*", (int) word.length())
				);
				continue;
			}

			auto pos = Strings::ToLower(piece).find(word);
			if (Strings::ToLower(piece).find(word) != std::string::npos && piece.length() > 4 && word.length() > 4) {
				auto        found_word     = piece.substr(pos, word.length());
				std::string replaced_piece = piece.substr(pos, word.length());

				Strings::FindReplace(
					server_long_name,
					replaced_piece,
					Strings::Repeat("*", (int) word.length())
				);
			}
		}
	}

	return server_long_name;
}

std::vector<std::string> GetBadWords()
{
	return std::vector<std::string>{
		"2g1c",
		"acrotomophilia",
		"anal",
		"anilingus",
		"anus",
		"apeshit",
		"arsehole",
		"ass",
		"asshole",
		"assmunch",
		"autoerotic",
		"babeland",
		"bangbros",
		"bangbus",
		"bareback",
		"barenaked",
		"bastard",
		"bastardo",
		"bastinado",
		"bbw",
		"bdsm",
		"beaner",
		"beaners",
		"beaver",
		"beastiality",
		"bestiality",
		"bimbos",
		"birdlock",
		"bitch",
		"bitches",
		"blowjob",
		"blumpkin",
		"bollocks",
		"bondage",
		"boner",
		"boob",
		"boobs",
		"bukkake",
		"bulldyke",
		"bullshit",
		"bung",
		"bunghole",
		"busty",
		"butt",
		"buttcheeks",
		"butthole",
		"camel toe",
		"camgirl",
		"camslut",
		"camwhore",
		"carpetmuncher",
		"cialis",
		"circlejerk",
		"clit",
		"clitoris",
		"clusterfuck",
		"cock",
		"cocks",
		"coprolagnia",
		"coprophilia",
		"cornhole",
		"coon",
		"coons",
		"creampie",
		"cum",
		"cumming",
		"cumshot",
		"cumshots",
		"cunnilingus",
		"cunt",
		"darkie",
		"daterape",
		"deepthroat",
		"dendrophilia",
		"dick",
		"dildo",
		"dingleberry",
		"dingleberries",
		"doggiestyle",
		"doggystyle",
		"dolcett",
		"domination",
		"dominatrix",
		"dommes",
		"hump",
		"dvda",
		"ecchi",
		"ejaculation",
		"erotic",
		"erotism",
		"escort",
		"eunuch",
		"fag",
		"faggot",
		"fecal",
		"felch",
		"fellatio",
		"feltch",
		"femdom",
		"figging",
		"fingerbang",
		"fingering",
		"fisting",
		"footjob",
		"frotting",
		"fuck",
		"fuckin",
		"fucking",
		"fucktards",
		"fudgepacker",
		"futanari",
		"gangbang",
		"gangbang",
		"gaysex",
		"genitals",
		"goatcx",
		"goatse",
		"gokkun",
		"goodpoop",
		"goregasm",
		"grope",
		"g-spot",
		"guro",
		"handjob",
		"hentai",
		"homoerotic",
		"honkey",
		"hooker",
		"horny",
		"humping",
		"incest",
		"intercourse",
		"jailbait",
		"jigaboo",
		"jiggaboo",
		"jiggerboo",
		"jizz",
		"juggs",
		"kike",
		"kinbaku",
		"kinkster",
		"kinky",
		"knobbing",
		"livesex",
		"lolita",
		"lovemaking",
		"masturbate",
		"masturbating",
		"masturbation",
		"milf",
		"mong",
		"motherfucker",
		"muffdiving",
		"nambla",
		"nawashi",
		"negro",
		"neonazi",
		"nigga",
		"nigger",
		"nimphomania",
		"nipple",
		"nipples",
		"nsfw",
		"nude",
		"nudity",
		"nutten",
		"nympho",
		"nymphomania",
		"octopussy",
		"omorashi",
		"orgasm",
		"orgy",
		"paedophile",
		"paki",
		"panties",
		"panty",
		"pedobear",
		"pedophile",
		"pegging",
		"penis",
		"pikey",
		"pissing",
		"pisspig",
		"playboy",
		"ponyplay",
		"poof",
		"poon",
		"poontang",
		"punany",
		"poopchute",
		"porn",
		"porno",
		"pornography",
		"pthc",
		"pubes",
		"pussy",
		"queaf",
		"queef",
		"quim",
		"raghead",
		"rape",
		"raping",
		"rapist",
		"rectum",
		"rimjob",
		"rimming",
		"sadism",
		"santorum",
		"scat",
		"schlong",
		"scissoring",
		"semen",
		"sex",
		"sexcam",
		"sexo",
		"sexy",
		"sexual",
		"sexually",
		"sexuality",
		"shemale",
		"shibari",
		"shit",
		"shitblimp",
		"shitty",
		"shota",
		"shrimping",
		"skeet",
		"slanteye",
		"slut",
		"s&m",
		"smut",
		"snatch",
		"snowballing",
		"sodomize",
		"sodomy",
		"spastic",
		"spic",
		"splooge",
		"spooge",
		"spunk",
		"strapon",
		"strappado",
		"suck",
		"sucks",
		"swastika",
		"swinger",
		"threesome",
		"throating",
		"thumbzilla",
		"tight white",
		"tit",
		"tits",
		"titties",
		"titty",
		"topless",
		"tosser",
		"towelhead",
		"tranny",
		"tribadism",
		"tubgirl",
		"tushy",
		"twat",
		"twink",
		"twinkie",
		"undressing",
		"upskirt",
		"urophilia",
		"vagina",
		"viagra",
		"vibrator",
		"vorarephilia",
		"voyeur",
		"voyeurweb",
		"voyuer",
		"vulva",
		"wank",
		"wetback",
		"whore",
		"worldsex",
		"xx",
		"xxx",
		"yaoi",
		"yiffy",
		"zoophilia"
	};
}

void ParseAccountString(const std::string &s, std::string &account, std::string &loginserver)
{
	auto split = Strings::Split(s, ':');
	if (split.size() == 2) {
		loginserver = split[0];
		account     = split[1];
	}
	else if (split.size() == 1) {
		account = split[0];
	}
}
