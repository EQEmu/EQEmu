## [Unreleased]

### Multiclass

* Strengthen `SetClassMask` validation to reject empty masks and masks with invalid/non-playable class bits (MC-002)

## [23.10.3] 9/16/2025

### Hotfix

* Hotfix crashes occurring in #4987. @Akkadius 2025-09-17

## [23.10.2] 9/16/2025

### Hotfix

* Revert #4996 as it was causing critical issues with spells that needs to be further investigated. @Akkadius 2025-09-17

## [23.10.1] 9/16/2025

### Hotfix
* Fixed Mail Key Bug ([#5015](https://github.com/EQEmu/Server/pull/5015)) @Kinglykrab 2025-09-16

## [23.10.0] 9/15/2025

### Build

* Fix Linking with GCC ([#4969](https://github.com/EQEmu/Server/pull/4969)) @solar984 2025-08-03

### Code

* Add #npcedit npc_tint_id Help Message ([#4982](https://github.com/EQEmu/Server/pull/4982)) @Kinglykrab 2025-08-17
* Cleanup #show ip_lookup Message ([#5005](https://github.com/EQEmu/Server/pull/5005)) @Kinglykrab 2025-08-30
* Fix #set race 0 Message ([#5004](https://github.com/EQEmu/Server/pull/5004)) @Kinglykrab 2025-08-30
* Fix Issues with Strings::Commify and Mob::SendStatsWindow ([#4984](https://github.com/EQEmu/Server/pull/4984)) @Kinglykrab 2025-08-17
* Remove Attributions ([#4988](https://github.com/EQEmu/Server/pull/4988)) @KimLS 2025-08-16
* Remove Unused errorname Variable ([#5001](https://github.com/EQEmu/Server/pull/5001)) @Kinglykrab 2025-08-29

### Commands

* Add #find account Subcommand ([#4981](https://github.com/EQEmu/Server/pull/4981)) @Kinglykrab 2025-08-17
* Add #show keyring Subcommand ([#4973](https://github.com/EQEmu/Server/pull/4973)) @Kinglykrab 2025-08-03
* Add #task complete Saylink to #task show ([#4985](https://github.com/EQEmu/Server/pull/4985)) @Kinglykrab 2025-08-17

### Constants

* Change Race Changes to Race Namespace ([#5000](https://github.com/EQEmu/Server/pull/5000)) @Kinglykrab 2025-08-30
* Convert SE Defines to SpellEffect Namespace ([#4999](https://github.com/EQEmu/Server/pull/4999)) @Kinglykrab 2025-08-30

### Database

* Add `heal_amount` to `character_stats_record` ([#4986](https://github.com/EQEmu/Server/pull/4986)) @Kinglykrab 2025-08-17

### Fixes

* Fix #show recipe uint16 Cap ([#4978](https://github.com/EQEmu/Server/pull/4978)) @Kinglykrab 2025-08-11
* Fix Race 474 for Titanium ([#4979](https://github.com/EQEmu/Server/pull/4979)) @regneq 2025-08-11
* Fix Recipe Inspect Bug ([#4994](https://github.com/EQEmu/Server/pull/4994)) @Kinglykrab 2025-08-30
* Fix Several Evolving Item Bugs ([#4992](https://github.com/EQEmu/Server/pull/4992)) @neckkola 2025-09-08
* Fix Task Reloading ([#5002](https://github.com/EQEmu/Server/pull/5002)) @Kinglykrab 2025-08-29

### Loginserver

* Fix Legacy World When Using Local DB ([#4970](https://github.com/EQEmu/Server/pull/4970)) @solar984 2025-08-03

### Pets

* Add Pet Constants and Methods ([#4987](https://github.com/EQEmu/Server/pull/4987)) @Kinglykrab 2025-08-17

### Quest API

* Add EVENT_CHARM_START and EVENT_CHARM_END ([#5013](https://github.com/EQEmu/Server/pull/5013)) @Kinglykrab 2025-09-15
* Add GetKeyRing() to Perl/Lua ([#4980](https://github.com/EQEmu/Server/pull/4980)) @Kinglykrab 2025-08-17
* Add GetNPCTintIndex() to Perl/Lua ([#4983](https://github.com/EQEmu/Server/pull/4983)) @Kinglykrab 2025-08-17
* Add GetTimers() and GetPausedTimers() to Perl/Lua ([#4965](https://github.com/EQEmu/Server/pull/4965)) @Kinglykrab 2025-08-03
* Add Identifiers to Get/Modify NPC Stat Methods ([#5012](https://github.com/EQEmu/Server/pull/5012)) @Kinglykrab 2025-09-15

### Repositories

* Convert Character Inspect Messages to Repositories ([#4997](https://github.com/EQEmu/Server/pull/4997)) @Kinglykrab 2025-08-30
* Convert Damage Shield Types to Repositories ([#4995](https://github.com/EQEmu/Server/pull/4995)) @Kinglykrab 2025-08-30
* Convert Item Loading to Repositories ([#4998](https://github.com/EQEmu/Server/pull/4998)) @Kinglykrab 2025-08-30
* Convert Mail Key to Repositories ([#5007](https://github.com/EQEmu/Server/pull/5007)) @Kinglykrab 2025-09-15
* Convert Shared Bank Platinum to Repositories ([#5006](https://github.com/EQEmu/Server/pull/5006)) @Kinglykrab 2025-09-02
* Convert Spell Loading to Repositories ([#4996](https://github.com/EQEmu/Server/pull/4996)) @Kinglykrab 2025-08-30
* Convert Total Time Played to Repositories ([#5008](https://github.com/EQEmu/Server/pull/5008)) @Kinglykrab 2025-09-15

## [23.9.1] 8/2/2025

### Hotfix

* Fix Quest Ownership Edge Case ([#4977](https://github.com/EQEmu/Server/pull/4977)) @Kinglykrab 2025-08-02

## [23.9.0] 8/2/2025

### Bots

* Fix FinishBuffing rule ([#4961](https://github.com/EQEmu/Server/pull/4961)) @nytmyr 2025-07-01
* Fix ^cast resurrects ([#4958](https://github.com/EQEmu/Server/pull/4958)) @nytmyr 2025-06-29

### Build

* Fix Linking with GCC ([#4969](https://github.com/EQEmu/Server/pull/4969)) @solar984 2025-08-03
* More Build Speed Improvements ([#4959](https://github.com/EQEmu/Server/pull/4959)) @Akkadius 2025-06-30

### Commands

* Add #show keyring Subcommand ([#4973](https://github.com/EQEmu/Server/pull/4973)) @Kinglykrab 2025-08-03

### Database

* Add Indexes to NPC's Spawns Loot ([#4972](https://github.com/EQEmu/Server/pull/4972)) @Akkadius 2025-07-30

### Feature

* Zone Scripting ([#4908](https://github.com/EQEmu/Server/pull/4908)) @Kinglykrab 2025-07-10

### Fixes

* Add a missing Froglok starting area for Titanium Startzone. ([#4962](https://github.com/EQEmu/Server/pull/4962)) @regneq 2025-07-04
* Fix Hero's Forge Ingame and Character Select ([#4966](https://github.com/EQEmu/Server/pull/4966)) @Kinglykrab 2025-07-30
* Show player count on the server list status. ([#4971](https://github.com/EQEmu/Server/pull/4971)) @regneq 2025-07-30

### Loginserver

* Fix Legacy World When Using Local DB ([#4970](https://github.com/EQEmu/Server/pull/4970)) @solar984 2025-08-03

### Performance

* Clear Wearchange Deduplication Cache ([#4960](https://github.com/EQEmu/Server/pull/4960)) @Akkadius 2025-06-30

### Quest API

* Add GetMemberRole() to Perl/Lua ([#4963](https://github.com/EQEmu/Server/pull/4963)) @Barathos 2025-07-10
* Add GetTimers() and GetPausedTimers() to Perl/Lua ([#4965](https://github.com/EQEmu/Server/pull/4965)) @Kinglykrab 2025-08-03

## [23.8.1] 6/28/2025

### Crash Fix

* Fix Possible Crashes with Raid Methods ([#4955](https://github.com/EQEmu/Server/pull/4955)) @Kinglykrab 2025-06-26

### Databuckets

* Revert Caching Changes of #4917 ([#4957](https://github.com/EQEmu/Server/pull/4957)) @Akkadius 2025-06-28

### Fixes

* Fix FindCharacter Using content_db ([#4956](https://github.com/EQEmu/Server/pull/4956)) @Kinglykrab 2025-06-26
* Fix Hero Forge on Character Select ([#4954](https://github.com/EQEmu/Server/pull/4954)) @Kinglykrab 2025-06-26

## [23.8.0] 6/25/2025

### API

* Add wwmarquee <type> <message> ([#4919](https://github.com/EQEmu/Server/pull/4919)) @Akkadius 2025-06-23

### Build

* Significantly Improve Build Times Using Unity Builds ([#4948](https://github.com/EQEmu/Server/pull/4948)) @Akkadius 2025-06-22

### Code

* AdventureManager Global to Singleton Cleanup ([#4931](https://github.com/EQEmu/Server/pull/4931)) @Kinglykrab 2025-06-25
* Cleanup Strings Header ([#4950](https://github.com/EQEmu/Server/pull/4950)) @Akkadius 2025-06-22
* ClientList Global to Singleton Cleanup ([#4942](https://github.com/EQEmu/Server/pull/4942)) @Kinglykrab 2025-06-25
* DatabaseUpdate Global to Singleton Cleanup ([#4943](https://github.com/EQEmu/Server/pull/4943)) @Kinglykrab 2025-06-25
* DiscordManager Global to Singleton Cleanup ([#4926](https://github.com/EQEmu/Server/pull/4926)) @Kinglykrab 2025-06-25
* EQ::Random Global to Singleton Cleanup ([#4936](https://github.com/EQEmu/Server/pull/4936)) @Kinglykrab 2025-06-22
* EQEmuLogSys Global to Singleton Cleanup ([#4925](https://github.com/EQEmu/Server/pull/4925)) @Akkadius 2025-06-25
* EvolvingItemsManager Global to Singleton Cleanup ([#4929](https://github.com/EQEmu/Server/pull/4929)) @Kinglykrab 2025-06-25
* LFGuildManager Global to Singleton Cleanup ([#4927](https://github.com/EQEmu/Server/pull/4927)) @Kinglykrab 2025-06-25
* LoginServerList Global to Singleton Cleanup ([#4941](https://github.com/EQEmu/Server/pull/4941)) @Kinglykrab 2025-06-25
* Optimize PCH Config (Faster Builds) ([#4951](https://github.com/EQEmu/Server/pull/4951)) @Akkadius 2025-06-22
* PCH Cleanup ([#4952](https://github.com/EQEmu/Server/pull/4952)) @Akkadius 2025-06-25
* PathManager Global to Singleton Cleanup ([#4924](https://github.com/EQEmu/Server/pull/4924)) @Akkadius 2025-06-22
* PetitionList Global to Singleton Cleanup ([#4944](https://github.com/EQEmu/Server/pull/4944)) @Kinglykrab 2025-06-25
* PlayerEventLogs Global to Singleton Cleanup ([#4928](https://github.com/EQEmu/Server/pull/4928)) @Kinglykrab 2025-06-25
* QueryServConnection Global to Singleton Cleanup ([#4938](https://github.com/EQEmu/Server/pull/4938)) @Kinglykrab 2025-06-25
* Remove Lua Rule Constants ([#4949](https://github.com/EQEmu/Server/pull/4949)) @Akkadius 2025-06-22
* Remove Regex Compile Bloat ([#4947](https://github.com/EQEmu/Server/pull/4947)) @Akkadius 2025-06-22
* Remove Unused MZoneShutdown Mutex ([#4946](https://github.com/EQEmu/Server/pull/4946)) @Kinglykrab 2025-06-22
* SharedTaskManager Global to Singleton Cleanup ([#4930](https://github.com/EQEmu/Server/pull/4930)) @Kinglykrab 2025-06-25
* SkillCaps Global to Singleton Cleanup ([#4933](https://github.com/EQEmu/Server/pull/4933)) @Kinglykrab 2025-06-22
* TaskManager Global to Singleton Cleanup ([#4945](https://github.com/EQEmu/Server/pull/4945)) @Kinglykrab 2025-06-22
* UCSConnection Global to Singleton Cleanup ([#4937](https://github.com/EQEmu/Server/pull/4937)) @Kinglykrab 2025-06-25
* WebInterfaceList Global to Singleton Cleanup ([#4935](https://github.com/EQEmu/Server/pull/4935)) @Kinglykrab 2025-06-25
* WorldContentServer Global to Singleton Cleanup ([#4939](https://github.com/EQEmu/Server/pull/4939)) @Kinglykrab 2025-06-25
* WorldEventScheduler and ZoneEventScheduler Global to Singleton Cleanup ([#4932](https://github.com/EQEmu/Server/pull/4932)) @Kinglykrab 2025-06-25
* ZSList Global to Singleton Cleanup ([#4940](https://github.com/EQEmu/Server/pull/4940)) @Kinglykrab 2025-06-25
* ZoneStore Global to Singleton Cleanup ([#4934](https://github.com/EQEmu/Server/pull/4934)) @Kinglykrab 2025-06-23

### Corpses

* Add corpse entity variables to DB ([#4911](https://github.com/EQEmu/Server/pull/4911)) @fryguy503 2025-06-25

### Databuckets

* Move Databuckets to Common ([#4918](https://github.com/EQEmu/Server/pull/4918)) @Akkadius 2025-06-16
* Nested Caching ([#4917](https://github.com/EQEmu/Server/pull/4917)) @Akkadius 2025-06-25

### Doors

* Fix door saving for versions ([#4905](https://github.com/EQEmu/Server/pull/4905)) @nytmyr 2025-06-09

### Fixes

* Bulk Send Corpses after Idle State ([#4910](https://github.com/EQEmu/Server/pull/4910)) @Akkadius 2025-06-09
* Fix ClientList Singleton Shortcomings of #4942  @Akkadius 2025-06-25
* Fix breaking changes to Titanium, SoF, SoD patches causes by big bags update ([#4922](https://github.com/EQEmu/Server/pull/4922)) @regneq 2025-06-25

### Logs

* Fix output for tasks in logs ([#4907](https://github.com/EQEmu/Server/pull/4907)) @joligario 2025-06-09

### Performance

* Auto Idle / AFK ([#4903](https://github.com/EQEmu/Server/pull/4903)) @Akkadius 2025-05-22
* Wearchange Packet Send Deduplication ([#4916](https://github.com/EQEmu/Server/pull/4916)) @Akkadius 2025-06-25

### Player Event Logs

* Don't Clear Event Data on ETL Events ([#4913](https://github.com/EQEmu/Server/pull/4913)) @Akkadius 2025-06-25

### Player Events

* Don't Write Empty Data ([#4912](https://github.com/EQEmu/Server/pull/4912)) @Akkadius 2025-06-25
* Zone Fetch Settings from QS if Enabled ([#4915](https://github.com/EQEmu/Server/pull/4915)) @Akkadius 2025-06-25

### Quest API

* Add CompleteTask and UncompleteTask methods to Perl/Lua ([#4921](https://github.com/EQEmu/Server/pull/4921)) @Barathos 2025-06-25
* Lua Client Scoped EnableTitleSets ([#4914](https://github.com/EQEmu/Server/pull/4914)) @fryguy503 2025-06-25

### Quests

* Support Multiple Quest, Plugin, and Lua Module Paths ([#4906](https://github.com/EQEmu/Server/pull/4906)) @Akkadius 2025-06-09

### World API

* Input Validation ([#4904](https://github.com/EQEmu/Server/pull/4904)) @Akkadius 2025-05-22

## [23.7.0] 5/19/2025

### CLI

* Add custom database version output ([#4901](https://github.com/EQEmu/Server/pull/4901)) @joligario 2025-05-18
* Fix MySQL check in database dumper ([#4897](https://github.com/EQEmu/Server/pull/4897)) @joligario 2025-05-16

### Commands

* Add #zonevariable Command ([#4882](https://github.com/EQEmu/Server/pull/4882)) @Kinglykrab 2025-05-15

### Database

* Add Custom Database Migrations for Operators ([#4892](https://github.com/EQEmu/Server/pull/4892)) @Akkadius 2025-05-16
* Remove Transaction Wrapped Character Save ([#4894](https://github.com/EQEmu/Server/pull/4894)) @Akkadius 2025-05-16

### Fixes

* Deadlock on failed #copycharacter commands ([#4887](https://github.com/EQEmu/Server/pull/4887)) @Akkadius 2025-05-16

### Logging

* Auto Update Log Category Names ([#4890](https://github.com/EQEmu/Server/pull/4890)) @Akkadius 2025-05-16

### Netcode

* Resend Logic Adjustments ([#4900](https://github.com/EQEmu/Server/pull/4900)) @Akkadius 2025-05-18

### Player Events

* Add rule to ignore configured GM commands ([#4888](https://github.com/EQEmu/Server/pull/4888)) @Akkadius 2025-05-16

### Rules

* Auto Update Rule Notes from Source ([#4891](https://github.com/EQEmu/Server/pull/4891)) @Akkadius 2025-05-16

### World

* Fix Rarer Reload Deadlock ([#4893](https://github.com/EQEmu/Server/pull/4893)) @Akkadius 2025-05-16

### Zone State

* Load New Spawn2 Data When Present ([#4889](https://github.com/EQEmu/Server/pull/4889)) @Akkadius 2025-05-16

## [23.6.0] 5/14/2025

### Bots

* Correct ^pull logic and add checks for Enchanter pets ([#4827](https://github.com/EQEmu/Server/pull/4827)) @nytmyr 2025-05-15
* Fix creation limit, spawn limit, level requirement checks ([#4868](https://github.com/EQEmu/Server/pull/4868)) @nytmyr 2025-05-15
* Move all spell_id instances to uint16 ([#4876](https://github.com/EQEmu/Server/pull/4876)) @nytmyr 2025-05-15
* Prevent non-taunters from potentially fleeing mob on TargetReflection ([#4859](https://github.com/EQEmu/Server/pull/4859)) @nytmyr 2025-04-28

### CLI

* ETL Settings Output ([#4873](https://github.com/EQEmu/Server/pull/4873)) @joligario 2025-05-15

### Code

* Fix typo in QueryNameAvailablity ([#4869](https://github.com/EQEmu/Server/pull/4869)) @nytmyr 2025-04-28

### Crash

* Fix crash bug with pbae and quest scripts spawning mobs ([#4884](https://github.com/EQEmu/Server/pull/4884)) @carolus21rex 2025-05-15

### Feature

* Add Character:TradeskillUpMinChance rule ([#4867](https://github.com/EQEmu/Server/pull/4867)) @zrix-eq 2025-05-15
* Enable spawn attribute for NPCTintID ([#4871](https://github.com/EQEmu/Server/pull/4871)) @neckkola 2025-05-15

### Fixes

* Add trader/buyer cleanup actions ([#4843](https://github.com/EQEmu/Server/pull/4843)) @neckkola 2025-05-15
* Fix #copycharacter command ([#4860](https://github.com/EQEmu/Server/pull/4860)) @nytmyr 2025-04-28
* Fix Crash with #task ([#4874](https://github.com/EQEmu/Server/pull/4874)) @Kinglykrab 2025-04-30
* Fix Object Name Init, User Refs, and Client Sync on Close ([#4861](https://github.com/EQEmu/Server/pull/4861)) @zimp-wow 2025-05-15
* Fix breaking change to UF patches caused by Big Bags update ([#4883](https://github.com/EQEmu/Server/pull/4883)) @hbingram 2025-05-15
* Prevent Ranged Attack from being triggered at arbitrary rate ([#4879](https://github.com/EQEmu/Server/pull/4879)) @catapultam-habeo 2025-05-15

### Performance

* Store Player Title Sets in Client Memory ([#4836](https://github.com/EQEmu/Server/pull/4836)) @Kinglykrab 2025-05-15

### Quest API

* Add Last Login and First Login Flags to EVENT_CONNECT ([#4866](https://github.com/EQEmu/Server/pull/4866)) @Kinglykrab 2025-05-15

## [23.5.0] 4/10/2025

### API

* World API Optimizations ([#4850](https://github.com/EQEmu/Server/pull/4850)) @Akkadius 2025-04-10

### Bots

* Add valid state checks to ^clickitem ([#4830](https://github.com/EQEmu/Server/pull/4830)) @nytmyr 2025-04-10
* Flag all buffs with SE_DamageShield as Damage Shield ([#4833](https://github.com/EQEmu/Server/pull/4833)) @nytmyr 2025-04-10
* Positioning rewrite ([#4856](https://github.com/EQEmu/Server/pull/4856)) @nytmyr 2025-04-10
* Restore old buff overwrite blocking ([#4832](https://github.com/EQEmu/Server/pull/4832)) @nytmyr 2025-04-10

### Bugfix

* Load zone variables before encounter_load. ([#4846](https://github.com/EQEmu/Server/pull/4846)) @zimp-wow 2025-04-10
* Prevent depops from blocking new spawns. ([#4841](https://github.com/EQEmu/Server/pull/4841)) @zimp-wow 2025-04-10
* Prevent final shutdown from persisting incomplete state. ([#4849](https://github.com/EQEmu/Server/pull/4849)) @zimp-wow 2025-04-10

### Code

* Remove queryserv dump flag ([#4842](https://github.com/EQEmu/Server/pull/4842)) @joligario 2025-04-10
* Update link for legacy EQEmu loginserver account setup ([#4826](https://github.com/EQEmu/Server/pull/4826)) @joligario 2025-04-10

### Crash

* Fix rarer exception crash issue in PlayerEventLogs::ProcessBatchQueue ([#4835](https://github.com/EQEmu/Server/pull/4835)) @Akkadius 2025-04-03

### Database

* Fix manifest for `helmtexture` in `horses` table ([#4852](https://github.com/EQEmu/Server/pull/4852)) @joligario 2025-04-10

### Feature

* Add rule to consume command text from any channel ([#4839](https://github.com/EQEmu/Server/pull/4839)) @catapultam-habeo 2025-04-10

### Fixes

* Add the bazaar search limit to query ([#4829](https://github.com/EQEmu/Server/pull/4829)) @neckkola 2025-04-10
* Backfill expire_at (not sure why this didn't make it in there to begin with)  @Akkadius 2025-03-31
* Bazaar Search window not working in a DZ ([#4828](https://github.com/EQEmu/Server/pull/4828)) @neckkola 2025-04-10
* Databuckets Account Cache Loading ([#4855](https://github.com/EQEmu/Server/pull/4855)) @Akkadius 2025-04-10
* Fix missing timer_name check on Mob::StopTimer ([#4840](https://github.com/EQEmu/Server/pull/4840)) @zimp-wow 2025-04-04
* FixHeading Infinite Loop Fix ([#4854](https://github.com/EQEmu/Server/pull/4854)) @KimLS 2025-04-10
* Make sure we don't expire default value instances  @Akkadius 2025-03-31
* Regression in World SendEmoteMessageRaw ([#4837](https://github.com/EQEmu/Server/pull/4837)) @Akkadius 2025-04-03
* Remove QS Tables From Export  @Akkadius 2025-04-10
* Zone State Spawn2 Location Restore ([#4844](https://github.com/EQEmu/Server/pull/4844)) @Akkadius 2025-04-10

### Netcode

* Fix Stale Client Edge Case ([#4853](https://github.com/EQEmu/Server/pull/4853)) @Akkadius 2025-04-10

### Performance

* Character Save Optimizations ([#4851](https://github.com/EQEmu/Server/pull/4851)) @Akkadius 2025-04-10
* Network Ring Buffers ([#4857](https://github.com/EQEmu/Server/pull/4857)) @Akkadius 2025-04-10
* Pre-Compute CLE Server Lists ([#4838](https://github.com/EQEmu/Server/pull/4838)) @Akkadius 2025-04-10

### Spells

* Fear resistance effects edge case fixes and support for SPA 102 as an AA ([#4848](https://github.com/EQEmu/Server/pull/4848)) @KayenEQ 2025-04-10
* Update to SPA 180 SE_ResistSpellChance to not block unresistable spells. ([#4847](https://github.com/EQEmu/Server/pull/4847)) @KayenEQ 2025-04-10
* Update to SPA 378 SE_SpellEffectResistChance ([#4845](https://github.com/EQEmu/Server/pull/4845)) @KayenEQ 2025-04-10

## [23.4.0] 3/30/2025

### API

* Expose Zoneserver Compile Metadata ([#4815](https://github.com/EQEmu/Server/pull/4815)) @Akkadius 2025-03-29

### Bots

* Charmed Pets were breaking Mob respawns ([#4780](https://github.com/EQEmu/Server/pull/4780)) @nytmyr 2025-03-16
* Enraged positioning ([#4789](https://github.com/EQEmu/Server/pull/4789)) @nytmyr 2025-03-29
* Fix IsValidSpellTypeBySpellID to account for all types ([#4764](https://github.com/EQEmu/Server/pull/4764)) @nytmyr 2025-03-19
* Fix Rule ZonesWithSpawnLimits/ZonesWithForcedSpawnLimits errors ([#4791](https://github.com/EQEmu/Server/pull/4791)) @nytmyr 2025-03-29
* Fix rule Bots:FinishBuffing ([#4788](https://github.com/EQEmu/Server/pull/4788)) @nytmyr 2025-03-29
* Line of Sight and Mez optimizations and cleanup ([#4746](https://github.com/EQEmu/Server/pull/4746)) @nytmyr 2025-03-29
* Prevent bot pets from despawning on #repop ([#4790](https://github.com/EQEmu/Server/pull/4790)) @nytmyr 2025-03-29

### Code

* Control flow defaults missed in recent bot updates ([#4817](https://github.com/EQEmu/Server/pull/4817)) @joligario 2025-03-30
* Remove Extraneous Time Type in ShowZoneData ([#4806](https://github.com/EQEmu/Server/pull/4806)) @Kinglykrab 2025-03-29
* Remove Unused Command Methods ([#4805](https://github.com/EQEmu/Server/pull/4805)) @Kinglykrab 2025-03-29
* UCS Member Count ([#4819](https://github.com/EQEmu/Server/pull/4819)) @joligario 2025-03-30

### Commands

* Add #show zone_variables ([#4812](https://github.com/EQEmu/Server/pull/4812)) @Akkadius 2025-03-29
* Add Instance Support to #zoneshutdown ([#4807](https://github.com/EQEmu/Server/pull/4807)) @Kinglykrab 2025-03-29

### Crash

* Fix Rarer World Crash with Player Event Thread Processor ([#4800](https://github.com/EQEmu/Server/pull/4800)) @Akkadius 2025-03-29
* Fix Repop Race Condition Crash ([#4814](https://github.com/EQEmu/Server/pull/4814)) @Akkadius 2025-03-29

### Database

* Fix Respawn Times Table ([#4802](https://github.com/EQEmu/Server/pull/4802)) @Akkadius 2025-03-29
* Wrap PurgeExpiredInstances in a Transaction ([#4824](https://github.com/EQEmu/Server/pull/4824)) @Akkadius 2025-03-30

### Feature

* Implement /changename & related script bindings. Clean up #set name ([#4770](https://github.com/EQEmu/Server/pull/4770)) @catapultam-habeo 2025-03-20

### Fixes

* AllowFVNoDrop Flag trades ([#4809](https://github.com/EQEmu/Server/pull/4809)) @neckkola 2025-03-27
* Fix Instance Creation Race Condition ([#4803](https://github.com/EQEmu/Server/pull/4803)) @Akkadius 2025-03-29
* Fix zone crash when attempting to add a disappearing client to hate list. ([#4782](https://github.com/EQEmu/Server/pull/4782)) @zimp-wow 2025-03-19
* Globally Reloading Quests when not loaded ([#4813](https://github.com/EQEmu/Server/pull/4813)) @Akkadius 2025-03-29
* Instance DZ Creation ([#4823](https://github.com/EQEmu/Server/pull/4823)) @Akkadius 2025-03-30
* Zone State Entity Variable Load Pre-Spawn ([#4785](https://github.com/EQEmu/Server/pull/4785)) @Akkadius 2025-03-19
* Zone State Position Fix ([#4784](https://github.com/EQEmu/Server/pull/4784)) @Akkadius 2025-03-19
* Zone State Variables Load First ([#4798](https://github.com/EQEmu/Server/pull/4798)) @Akkadius 2025-03-29
* Zone state edge case with 0 hp ([#4787](https://github.com/EQEmu/Server/pull/4787)) @Akkadius 2025-03-29

### Instance

* Clear Respawn Timers on Creation ([#4801](https://github.com/EQEmu/Server/pull/4801)) @Akkadius 2025-03-29

### Instances

* Add `expire_at` Column ([#4820](https://github.com/EQEmu/Server/pull/4820)) @Akkadius 2025-03-30

### Performance

* Add several database indexes ([#4811](https://github.com/EQEmu/Server/pull/4811)) @Akkadius 2025-03-29
* Have World Send Smarter Guild Updates ([#4796](https://github.com/EQEmu/Server/pull/4796)) @Akkadius 2025-03-29
* Improve Character Select DB Performance ([#4799](https://github.com/EQEmu/Server/pull/4799)) @Akkadius 2025-03-29
* Reduce Adventure S2S chatter ([#4793](https://github.com/EQEmu/Server/pull/4793)) @Akkadius 2025-03-29
* Reduce CorpseOwnerOnline S2S Chatter to World ([#4795](https://github.com/EQEmu/Server/pull/4795)) @Akkadius 2025-03-29
* Reduce LFGuild Chatter ([#4794](https://github.com/EQEmu/Server/pull/4794)) @Akkadius 2025-03-29
* Reduce UpdateWho S2S Chatter to World ([#4792](https://github.com/EQEmu/Server/pull/4792)) @Akkadius 2025-03-29
* Send Smarter Emote Packets ([#4818](https://github.com/EQEmu/Server/pull/4818)) @Akkadius 2025-03-30

### Quest API

* Add Support for NPC ID and NPC Name Specificity ([#4781](https://github.com/EQEmu/Server/pull/4781)) @Kinglykrab 2025-03-19

### Reload

* Add Reload for Maps / Navs ([#4816](https://github.com/EQEmu/Server/pull/4816)) @Akkadius 2025-03-29

### Zone

* Zone State Automated Testing and Improvements ([#4808](https://github.com/EQEmu/Server/pull/4808)) @Akkadius 2025-03-30
* Zone State Improvements Part 3 ([#4773](https://github.com/EQEmu/Server/pull/4773)) @Akkadius 2025-03-13

## [23.3.4] 3/14/2025

### Fixes

* Add check for simultaneous direct vendor and parcel Trader/Buyer Purchase ([#4778](https://github.com/EQEmu/Server/pull/4778)) @neckkola 2025-03-14
* Fix for rare circumstance where NPC's would have 0 health on restore @Akkadius

## [23.3.3] 3/13/2025

### Database

* Add indexes for data_buckets and zone_state_spawns ([#4771](https://github.com/EQEmu/Server/pull/4771)) @Akkadius 2025-03-11

### Fixes

* Update GuildBank to correctly handle items with charges equal to zero ([#4774](https://github.com/EQEmu/Server/pull/4774)) @neckkola 2025-03-13

### Networking

* Fix "port in use" error ([#4772](https://github.com/EQEmu/Server/pull/4772)) @Akkadius 2025-03-12

### Zone

* Zone State Improvements Part 3 ([#4773](https://github.com/EQEmu/Server/pull/4773)) @Akkadius 2025-03-13

## [23.3.2] 3/11/2025

### DynamicZones

* Bulk request dz member statuses on zone boot ([#4769](https://github.com/EQEmu/Server/pull/4769)) @hgtw 2025-03-11

### Zone

* Zone State Improvements (Continued) ([#4768](https://github.com/EQEmu/Server/pull/4768)) @Akkadius 2025-03-11

## [23.3.0] 3/8/2025

### Bots

* Fix buffs not overwriting lesser buffs ([#4756](https://github.com/EQEmu/Server/pull/4756)) @nytmyr 2025-03-06
* Fix taunting bots positioning ([#4754](https://github.com/EQEmu/Server/pull/4754)) @nytmyr 2025-03-06
* Move commanded spell map to zone ([#4755](https://github.com/EQEmu/Server/pull/4755)) @nytmyr 2025-03-06

### Code

* Fix typo in GM tradeskill combine message ([#4762](https://github.com/EQEmu/Server/pull/4762)) @nytmyr 2025-03-08

### Crash

* Bot aura crash fix ([#4752](https://github.com/EQEmu/Server/pull/4752)) @nytmyr 2025-03-06

### Databuckets

* Nested Databuckets Protections and Improvements ([#4748](https://github.com/EQEmu/Server/pull/4748)) @Akkadius 2025-03-04

### Feature

* Add Rule for dealing with augments when an item evolves ([#4758](https://github.com/EQEmu/Server/pull/4758)) @neckkola 2025-03-08
* Allow assigning Helm Texture independently of Body Texture for Horses ([#4759](https://github.com/EQEmu/Server/pull/4759)) @catapultam-habeo 2025-03-08

### Fixes

* Add crash checks for certain PlayerEventLogs ([#4761](https://github.com/EQEmu/Server/pull/4761)) @neckkola 2025-03-07
* Correct incorrectly calculated stat caps with Heroic Stats ([#4760](https://github.com/EQEmu/Server/pull/4760)) @catapultam-habeo 2025-03-08
* Fix sigabort crash from invalid JSON  @Akkadius 2025-03-03
* Forgot to push up some changes for test output  @Akkadius 2025-03-04
* Parcel Delivery Updates for two edge cases ([#4753](https://github.com/EQEmu/Server/pull/4753)) @neckkola 2025-03-06
* Remove one port check in world  @Akkadius 2025-03-03
* Zero out currentnpcid whenever spawn is reset. ([#4763](https://github.com/EQEmu/Server/pull/4763)) @zimp-wow 2025-03-08

### Logging

* Convert JSON Error to Data Buckets Logging Category ([#4747](https://github.com/EQEmu/Server/pull/4747)) @Kinglykrab 2025-03-04

### Pets

* Fix renamed pets loading as blank names ([#4751](https://github.com/EQEmu/Server/pull/4751)) @nytmyr 2025-03-05

### Rules

* Fix EvolvingItems:PercentOfRaidExperience Description ([#4757](https://github.com/EQEmu/Server/pull/4757)) @Kinglykrab 2025-03-07

### Tests

* Cleanup Hand-in Tests ([#4749](https://github.com/EQEmu/Server/pull/4749)) @Akkadius 2025-03-04

### Zone

* Make zone controller less likely to be visible, immune to all forms of combat ([#4750](https://github.com/EQEmu/Server/pull/4750)) @Akkadius 2025-03-06
* State Save Improvements ([#4765](https://github.com/EQEmu/Server/pull/4765)) @Akkadius 2025-03-08

## [23.2.0] 3/3/2025

### Crash

* Database SetMutex crash fix ([#4741](https://github.com/EQEmu/Server/pull/4741)) @Akkadius 2025-03-03
* Fix Aura process crash with bots ([#4743](https://github.com/EQEmu/Server/pull/4743)) @Akkadius 2025-03-03
* Fix crash in add loot code path ([#4745](https://github.com/EQEmu/Server/pull/4745)) @Akkadius 2025-03-03
* Fix world repop crash ([#4742](https://github.com/EQEmu/Server/pull/4742)) @Akkadius 2025-03-03
* Potential crash fix in scan close mobs ([#4744](https://github.com/EQEmu/Server/pull/4744)) @Akkadius 2025-03-03

### Fixes

* Cleanup zone buckets on instance purge. ([#4739](https://github.com/EQEmu/Server/pull/4739)) @zimp-wow 2025-03-02
* Fix an error causing Endurance Regen to not be applied by items. ([#4738](https://github.com/EQEmu/Server/pull/4738)) @catapultam-habeo 2025-03-02

### World

* Check if port in use to avoid double booting mistakes ([#4740](https://github.com/EQEmu/Server/pull/4740)) @Akkadius 2025-03-03

## [23.1.0] 3/1/2025

### Bots

* Fix unresponsive bots in groups upon group wipe ([#4712](https://github.com/EQEmu/Server/pull/4712)) @nytmyr 2025-02-28

### Code

* More login <-> world code cleanup ([#4724](https://github.com/EQEmu/Server/pull/4724)) @Akkadius 2025-02-28

### Crash

* Check for directory existence before traversing in CheckForCompatibleQuestPlugins ([#4730](https://github.com/EQEmu/Server/pull/4730)) @Akkadius 2025-03-02
* Fix filesystem crash / exception in DatabaseDumpService::RemoveCredentialsFile() ([#4731](https://github.com/EQEmu/Server/pull/4731)) @Akkadius 2025-03-01
* Fix large file size crash in File::GetContents for windows ([#4735](https://github.com/EQEmu/Server/pull/4735)) @Akkadius 2025-03-02
* Fix reload concurrency crash when ran from Spire ([#4733](https://github.com/EQEmu/Server/pull/4733)) @Akkadius 2025-03-02
* Validate item in SE_SummonItemIntoBag ([#4734](https://github.com/EQEmu/Server/pull/4734)) @Akkadius 2025-03-02
* World CLI validation ([#4728](https://github.com/EQEmu/Server/pull/4728)) @Akkadius 2025-03-01

### Database

* Remove force_interactive from big bag updates ([#4727](https://github.com/EQEmu/Server/pull/4727)) @Akkadius 2025-03-01

### Feature

* Add a rule for spells to bypass stacking rules ([#4716](https://github.com/EQEmu/Server/pull/4716)) @catapultam-habeo 2025-02-28
* Evolving items Additions ([#4725](https://github.com/EQEmu/Server/pull/4725)) @neckkola 2025-03-01

### Fixes

* Add character_pet_name to player tables schema  @Akkadius 2025-03-02
* Add client packets to questmanager:setguild ([#4732](https://github.com/EQEmu/Server/pull/4732)) @neckkola 2025-03-01
* Clear m_completed_shared_tasks before reloading  @Akkadius 2025-02-24
* Fix AA Reset Error Message ([#4720](https://github.com/EQEmu/Server/pull/4720)) @Kinglykrab 2025-02-28
* Fix Issue with Suffixes/Prefixes ([#4723](https://github.com/EQEmu/Server/pull/4723)) @Kinglykrab 2025-02-28
* Fix Trading Items to Bot Pets ([#4721](https://github.com/EQEmu/Server/pull/4721)) @MortimerGreenwald 2025-02-28
* Refactor ApplyItemBonuses to fix double-counting of ATK and recommended levels not correctly applying ([#4713](https://github.com/EQEmu/Server/pull/4713)) @catapultam-habeo 2025-03-01

### Loginserver

* Minor cleanup ([#4729](https://github.com/EQEmu/Server/pull/4729)) @Akkadius 2025-03-01

### Quest API

* Add Key Ring Methods to Perl and Lua ([#4719](https://github.com/EQEmu/Server/pull/4719)) @Kinglykrab 2025-02-28
* Implement eq.handin() and quest::handin() ([#4718](https://github.com/EQEmu/Server/pull/4718)) @Akkadius 2025-02-28

### Tasks

* Extend IsTaskCompleted to also be aware of shared task completion ([#4714](https://github.com/EQEmu/Server/pull/4714)) @Akkadius 2025-02-24

### Zone

* Implement Zone State Saving on Shutdown ([#4715](https://github.com/EQEmu/Server/pull/4715)) @Akkadius 2025-02-28

### Zone State

* Wrap all serialization/deserialization in try/catch ([#4726](https://github.com/EQEmu/Server/pull/4726)) @Akkadius 2025-03-01

## [23.0.2] 2/21/2025

### Bots

* Add checks to ensure bots and pets do not engage on ^pull ([#4708](https://github.com/EQEmu/Server/pull/4708)) @nytmyr 2025-02-22
* Improve positioning ([#4709](https://github.com/EQEmu/Server/pull/4709)) @nytmyr 2025-02-22
* Prevent medding in combat if any mob has bot targeted ([#4707](https://github.com/EQEmu/Server/pull/4707)) @nytmyr 2025-02-22

### Client Mod

* Adds a hacked fast camp rule for GMs ([#4697](https://github.com/EQEmu/Server/pull/4697)) @KimLS 2025-02-20

### Fixes

* Fix Lua Zone ID Exports ([#4700](https://github.com/EQEmu/Server/pull/4700)) @Kinglykrab 2025-02-22
* Fix bad Mob reference in QuestManager::resumetimer() ([#4710](https://github.com/EQEmu/Server/pull/4710)) @zimp-wow 2025-02-22
* Fix cursor load on zone ([#4704](https://github.com/EQEmu/Server/pull/4704)) @nytmyr 2025-02-22
* Fix infinite loop in QuestManager::stoptimer() ([#4703](https://github.com/EQEmu/Server/pull/4703)) @zimp-wow 2025-02-21

### Quest API

* Add GetSpawn() to Perl and Lua ([#4702](https://github.com/EQEmu/Server/pull/4702)) @Kinglykrab 2025-02-22

## [23.0.1] 2/20/2025

### Fixes

* Player event ordering merge fix ([#4699](https://github.com/EQEmu/Server/pull/4699)) @Akkadius 2025-02-20

### Quest API

* Add DisableRespawnTimers to Perl and Lua ([#4691](https://github.com/EQEmu/Server/pull/4691)) @Kinglykrab 2025-02-20

## [23.0.0] 2/19/2025

### Bots

* Add AEHateLine to HateLine ParentType ([#4678](https://github.com/EQEmu/Server/pull/4678)) @nytmyr 2025-02-15
* Add IsInRaidOrGroup checks to ^attack and ^pull ([#4654](https://github.com/EQEmu/Server/pull/4654)) @nytmyr 2025-02-07
* Add missing stance options ([#4681](https://github.com/EQEmu/Server/pull/4681)) @nytmyr 2025-02-15
* Bot Overhaul ([#4580](https://github.com/EQEmu/Server/pull/4580)) @nytmyr 2025-02-03
* Command Cleanup ([#4676](https://github.com/EQEmu/Server/pull/4676)) @nytmyr 2025-02-15
* Correct camp count on ^camp ([#4650](https://github.com/EQEmu/Server/pull/4650)) @nytmyr 2025-02-06
* Correct helper message for forced casts ([#4656](https://github.com/EQEmu/Server/pull/4656)) @nytmyr 2025-02-07
* Crash fixes related to GetNumberNeedingHealedInGroup ([#4684](https://github.com/EQEmu/Server/pull/4684)) @nytmyr 2025-02-15
* Fix AE range calculation ([#4683](https://github.com/EQEmu/Server/pull/4683)) @nytmyr 2025-02-15
* Fix Bards not casting ([#4638](https://github.com/EQEmu/Server/pull/4638)) @nytmyr 2025-02-03
* Fix a couple potential crashes with GetNumberNeedingHealedInGroup ([#4652](https://github.com/EQEmu/Server/pull/4652)) @nytmyr 2025-02-07
* Fix crash related to GetTempSpellType() ([#4649](https://github.com/EQEmu/Server/pull/4649)) @nytmyr 2025-02-06
* Fix pets causing aggro ([#4677](https://github.com/EQEmu/Server/pull/4677)) @nytmyr 2025-02-15
* Fix spell priority commands ([#4660](https://github.com/EQEmu/Server/pull/4660)) @nytmyr 2025-02-08
* Fix typo in positioning ([#4659](https://github.com/EQEmu/Server/pull/4659)) @nytmyr 2025-02-08
* Move BotGetSpellsByType to cache ([#4655](https://github.com/EQEmu/Server/pull/4655)) @nytmyr 2025-02-07
* Prevents casting on ineligible targets due to target type, stacking, etc. ([#4680](https://github.com/EQEmu/Server/pull/4680)) @nytmyr 2025-02-15
* Sanity checks for spell type updates ([#4641](https://github.com/EQEmu/Server/pull/4641)) @nytmyr 2025-02-05

### Bug

* Item Purchase Offset when multiple buyers are buying at the same time. ([#4628](https://github.com/EQEmu/Server/pull/4628)) @fryguy503 2025-02-06

### CI

* Fix database race condition ([#4646](https://github.com/EQEmu/Server/pull/4646)) @Akkadius 2025-02-06

### Client Mod

* Adds a hacked fast camp rule for GMs ([#4697](https://github.com/EQEmu/Server/pull/4697)) @KimLS 2025-02-20

### Code

* Bot RaidGroupSay ([#4653](https://github.com/EQEmu/Server/pull/4653)) @nytmyr 2025-02-07
* Cleanup logic in cursor bag check ([#4642](https://github.com/EQEmu/Server/pull/4642)) @nytmyr 2025-02-04
* Use Repositories for Titles ([#4608](https://github.com/EQEmu/Server/pull/4608)) @Kinglykrab 2025-02-07

### Commands

* Fix #goto not accepting proper heading ([#4685](https://github.com/EQEmu/Server/pull/4685)) @nytmyr 2025-02-15
* Fix Illusion Block ([#4666](https://github.com/EQEmu/Server/pull/4666)) @nytmyr 2025-02-12

### Crash

* Fix raid/group crash regression ([#4671](https://github.com/EQEmu/Server/pull/4671)) @Akkadius 2025-02-12
* Fix zone crash caused by NPC::MoveTo ([#4639](https://github.com/EQEmu/Server/pull/4639)) @catapultam-habeo 2025-02-03

### Databuckets

* Add Zone Scoped Databuckets ([#4690](https://github.com/EQEmu/Server/pull/4690)) @Akkadius 2025-02-18

### Expeditions

* Move expedition code into DynamicZone ([#4672](https://github.com/EQEmu/Server/pull/4672)) @hgtw 2025-02-16

### Feature

* Add Support for Tradeskill Recipe Inspect ([#4648](https://github.com/EQEmu/Server/pull/4648)) @Kinglykrab 2025-02-06
* Add rule to allow /changepetname to function without being enabled by scripts.  @catapultam-habeo 2025-02-05
* GuildBank Updates ([#4674](https://github.com/EQEmu/Server/pull/4674)) @neckkola 2025-02-15
* Implement Big Bags ([#4606](https://github.com/EQEmu/Server/pull/4606)) @Kinglykrab 2025-02-03

### Fixes

* #rq and #reload quest alias ([#4694](https://github.com/EQEmu/Server/pull/4694)) @Akkadius 2025-02-18
* Always spawn zone controller first ([#4669](https://github.com/EQEmu/Server/pull/4669)) @Akkadius 2025-02-12
* Big Bag Cleanup ([#4643](https://github.com/EQEmu/Server/pull/4643)) @fryguy503 2025-02-05
* Big Bag additional fixes ([#4644](https://github.com/EQEmu/Server/pull/4644)) @fryguy503 2025-02-05
* Change logging level for no items found in a bazaar search to reduce spam logs. ([#4675](https://github.com/EQEmu/Server/pull/4675)) @neckkola 2025-02-13
* Find Zone - Expansion Settings ([#4692](https://github.com/EQEmu/Server/pull/4692)) @MortimerGreenwald 2025-02-18
* Fix Beastlord Warder Size Modifier ([#4665](https://github.com/EQEmu/Server/pull/4665)) @Kinglykrab 2025-02-12
* Fix CI since hand-ins are merged  @Akkadius 2025-02-03
* Fix Illusion Fade Texture Bug ([#4673](https://github.com/EQEmu/Server/pull/4673)) @Kinglykrab 2025-02-14
* Fix Item Discovery ([#4663](https://github.com/EQEmu/Server/pull/4663)) @Kinglykrab 2025-02-10
* Fix ST_GroupNoPets and ST_GroupClientAndPet ([#4667](https://github.com/EQEmu/Server/pull/4667)) @nytmyr 2025-02-10
* Fix SendStatsWindow Mod2 Value Display ([#4658](https://github.com/EQEmu/Server/pull/4658)) @Kinglykrab 2025-02-07
* Fix Tradeskill Queries ([#4661](https://github.com/EQEmu/Server/pull/4661)) @Kinglykrab 2025-02-09
* Fix error in update manifest ([#4637](https://github.com/EQEmu/Server/pull/4637)) @nytmyr 2025-02-03
* Fix issue with getting an unset nested databucket ([#4693](https://github.com/EQEmu/Server/pull/4693)) @Akkadius 2025-02-19
* Fix non-error in player_event_logs ([#4695](https://github.com/EQEmu/Server/pull/4695)) @Akkadius 2025-02-18
* GMMove Update Edge Case With Clients ([#4686](https://github.com/EQEmu/Server/pull/4686)) @Akkadius 2025-02-15
* Item Handins to Pets ([#4687](https://github.com/EQEmu/Server/pull/4687)) @Akkadius 2025-02-15
* Parcel Delivery Updates ([#4688](https://github.com/EQEmu/Server/pull/4688)) @neckkola 2025-02-16
* Prevent zone from loading ETL ID's on bootup ([#4696](https://github.com/EQEmu/Server/pull/4696)) @Akkadius 2025-02-18
* Update pre big bag corpse slot_id's to support big bags ([#4679](https://github.com/EQEmu/Server/pull/4679)) @nytmyr 2025-02-15

### Inventory

* Fix cursor bag saving to invalid slot_ids ([#4640](https://github.com/EQEmu/Server/pull/4640)) @nytmyr 2025-02-04

### Items

* Overhaul Item Hand-in System ([#4593](https://github.com/EQEmu/Server/pull/4593)) @Akkadius 2025-02-03

### Loginserver

* Fix iterator crash ([#4670](https://github.com/EQEmu/Server/pull/4670)) @Akkadius 2025-02-12
* Modernize codebase ([#4647](https://github.com/EQEmu/Server/pull/4647)) @Akkadius 2025-02-06

### NPC Handins

* Fix MultiQuest Handins ([#4651](https://github.com/EQEmu/Server/pull/4651)) @Akkadius 2025-02-07

### Performance

* Server Reload Overhaul ([#4689](https://github.com/EQEmu/Server/pull/4689)) @Akkadius 2025-02-18

### Player Event Logs

* Migrate and Deprecate QS Legacy Logging ([#4542](https://github.com/EQEmu/Server/pull/4542)) @neckkola 2025-02-05

### Quest API

* Add Bandolier Methods ([#4635](https://github.com/EQEmu/Server/pull/4635)) @Kinglykrab 2025-02-03
* Add Potion Belt Methods ([#4634](https://github.com/EQEmu/Server/pull/4634)) @Kinglykrab 2025-02-04
* Add Zone Support to Perl and Lua ([#4662](https://github.com/EQEmu/Server/pull/4662)) @Kinglykrab 2025-02-09

### Spells

* Add all types to checks for max_targets_allowed rule for AEs ([#4682](https://github.com/EQEmu/Server/pull/4682)) @nytmyr 2025-02-15

## [22.62.2] 2/1/2025

### Fixes

* Add price change check to the Bazaar Search Window purchase mechanics ([#4632](https://github.com/EQEmu/Server/pull/4632)) @neckkola 2025-02-01
* NewBazaar Search Consumables ([#4631](https://github.com/EQEmu/Server/pull/4631)) @neckkola 2025-02-01
* Update the shard bazaar search feature ([#4630](https://github.com/EQEmu/Server/pull/4630)) @neckkola 2025-02-01

### Memory Leak

* Revert " Change raw pointer to unique_ptr to avoid potential leak in dbg stream " ([#4616](https://github.com/EQEmu/Server/pull/4616)) @Akkadius 2025-01-27

### Performance

* Significantly Improve Client Network Resends ([#4629](https://github.com/EQEmu/Server/pull/4629)) @Akkadius 2025-02-01

## [22.62.1] 1/27/2025

### Memory Leak

* Revert "Change raw pointer to unique_ptr to avoid potential leak in dbg stream" ([#4616](https://github.com/EQEmu/Server/pull/4616)) @Akkadius 2025-01-27

### Performance

* Re-use ClientUpdate packet memory ([#4619](https://github.com/EQEmu/Server/pull/4619)) @Akkadius 2025-01-27
* Re-use OP_Animation packet ([#4621](https://github.com/EQEmu/Server/pull/4621)) @Akkadius 2025-01-27
* Re-use OP_Damage packet memory ([#4625](https://github.com/EQEmu/Server/pull/4625)) @Akkadius 2025-01-27
* Re-use OP_HPUpdate packet memory ([#4622](https://github.com/EQEmu/Server/pull/4622)) @Akkadius 2025-01-27
* Re-use OP_PlayerStateAdd packet memory ([#4626](https://github.com/EQEmu/Server/pull/4626)) @Akkadius 2025-01-27
* Re-use OP_SendFindableNPCs packet memory ([#4623](https://github.com/EQEmu/Server/pull/4623)) @Akkadius 2025-01-27

### Repop

* Make #repop instant ([#4620](https://github.com/EQEmu/Server/pull/4620)) @Akkadius 2025-01-27

## [22.62.0] 1/26/2025

### Bazaar

* Improve Bazaar Search Performance ([#4615](https://github.com/EQEmu/Server/pull/4615)) @neckkola 2025-01-27

### CLI

* Add --skip-backup to world database:updates ([#4605](https://github.com/EQEmu/Server/pull/4605)) @Akkadius 2025-01-22

### Database

* Change npc_types walkspeed to be of type float ([#4589](https://github.com/EQEmu/Server/pull/4589)) @Akkadius 2025-01-07

### Databuckets

* Add Account Scoped Databuckets ([#4603](https://github.com/EQEmu/Server/pull/4603)) @Akkadius 2025-01-21
* Implement Nested Databuckets ([#4604](https://github.com/EQEmu/Server/pull/4604)) @Akkadius 2025-01-27

### Feature

* Add Alternate Bazaar Search Approach ([#4600](https://github.com/EQEmu/Server/pull/4600)) @neckkola 2025-01-20
* Add Support for Item Previews ([#4599](https://github.com/EQEmu/Server/pull/4599)) @Kinglykrab 2025-01-20
* Evolving Item Support for RoF2 ([#4496](https://github.com/EQEmu/Server/pull/4496)) @neckkola 2025-01-20
* Implement Custom Pet Names ([#4594](https://github.com/EQEmu/Server/pull/4594)) @catapultam-habeo 2025-01-22

### Fixes

* Add Bazaar BulkSendTrader Limit for RoF2 ([#4590](https://github.com/EQEmu/Server/pull/4590)) @neckkola 2025-01-08
* CLI help menu from parsing correctly in World  @Akkadius 2025-01-22
* Delete later in RemoveItem second case  @Akkadius 2025-01-25
* Fix query error in character_evolving_items  @Akkadius 2025-01-21
* Repair a memory leak in #summonitem ([#4591](https://github.com/EQEmu/Server/pull/4591)) @neckkola 2025-01-08
* Repair an incorrect safe_delete call memory leak. ([#4588](https://github.com/EQEmu/Server/pull/4588)) @neckkola 2025-01-07
* Repair levers opening the Evolving XP Transfer Window ([#4607](https://github.com/EQEmu/Server/pull/4607)) @neckkola 2025-01-23
* Update a few Bazaar RoF2 routines for memory leaks ([#4592](https://github.com/EQEmu/Server/pull/4592)) @neckkola 2025-01-08
* Update database version to match manifest  @Akkadius 2025-01-21
* Update trader add/remove packets to limits for RoF2 ([#4595](https://github.com/EQEmu/Server/pull/4595)) @neckkola 2025-01-19

### Linux

* Implement KSM Kernel Samepage Merging with Maps ([#4601](https://github.com/EQEmu/Server/pull/4601)) @Akkadius 2025-01-21

### Memory Leak

* Change raw pointer to unique_ptr to avoid potential leak in dbg stream ([#4616](https://github.com/EQEmu/Server/pull/4616)) @KimLS 2025-01-27
* Fix leak in BuyTraderItemOutsideBazaar ([#4609](https://github.com/EQEmu/Server/pull/4609)) @Akkadius 2025-01-24
* Fix leak in Client::RemoveDuplicateLore ([#4614](https://github.com/EQEmu/Server/pull/4614)) @Akkadius 2025-01-24
* Fix leak in NPC::RemoveItem ([#4611](https://github.com/EQEmu/Server/pull/4611)) @Akkadius 2025-01-24
* Fix leak in QuestManager::varlink ([#4610](https://github.com/EQEmu/Server/pull/4610)) @Akkadius 2025-01-24
* Fix leaks in Client::Handle_OP_AugmentItem ([#4612](https://github.com/EQEmu/Server/pull/4612)) @Akkadius 2025-01-24
* Fix memory leak in Client::Handle_OP_MoveMultipleItems ([#4613](https://github.com/EQEmu/Server/pull/4613)) @Akkadius 2025-01-24

### Performance

* Client / NPC Position Update Optimizations ([#4602](https://github.com/EQEmu/Server/pull/4602)) @Akkadius 2025-01-21

### Quest API

* Add SetAAEXPPercentage to Perl/Lua ([#4597](https://github.com/EQEmu/Server/pull/4597)) @Kinglykrab 2025-01-19

### Zone

* Implement zone player count sharding ([#4536](https://github.com/EQEmu/Server/pull/4536)) @Akkadius 2025-01-08

## [22.61.0] 1/6/2025

### Bots

* Fix AA ranks to account for level ([#4567](https://github.com/EQEmu/Server/pull/4567)) @nytmyr 2024-12-07

### Code

* Convert Event Parses to Single Line ([#4569](https://github.com/EQEmu/Server/pull/4569)) @Kinglykrab 2024-12-12
* Fix GM Flag Spell Restriction Bypasses ([#4571](https://github.com/EQEmu/Server/pull/4571)) @Kinglykrab 2025-01-06
* Remove Unused Group Methods ([#4559](https://github.com/EQEmu/Server/pull/4559)) @Kinglykrab 2024-12-12

### Commands

* Add #find bot Subcommand ([#4563](https://github.com/EQEmu/Server/pull/4563)) @Kinglykrab 2024-12-12
* Add #find ldon_theme Subcommand ([#4564](https://github.com/EQEmu/Server/pull/4564)) @Kinglykrab 2024-12-12
* Fix #copycharacter ([#4582](https://github.com/EQEmu/Server/pull/4582)) @Akkadius 2025-01-06

### Databuckets

* Improved Reliability and Performance of Databuckets ([#4562](https://github.com/EQEmu/Server/pull/4562)) @Akkadius 2024-12-12

### Feature

* Enable bazaar window 'Find Trader' functionality ([#4560](https://github.com/EQEmu/Server/pull/4560)) @neckkola 2024-12-12

### Filesystem

* Path Manager Improvements ([#4557](https://github.com/EQEmu/Server/pull/4557)) @Akkadius 2025-01-06

### Fixes

* Allow Items in ROF2 to Stack to 32,767 ([#4556](https://github.com/EQEmu/Server/pull/4556)) @Kinglykrab 2024-12-12
* Fix EVENT_COMBAT on NPC Death ([#4558](https://github.com/EQEmu/Server/pull/4558)) @Kinglykrab 2024-11-28
* Guild Membership Update Fix ([#4581](https://github.com/EQEmu/Server/pull/4581)) @neckkola 2025-01-06
* Guild creation to propagate across zones ([#4575](https://github.com/EQEmu/Server/pull/4575)) @neckkola 2025-01-06
* Repair a EQEMUConfig Memory Leak ([#4584](https://github.com/EQEmu/Server/pull/4584)) @neckkola 2025-01-06
* Repair a LoadNPCEmote MemoryLeak ([#4586](https://github.com/EQEmu/Server/pull/4586)) @neckkola 2025-01-06
* Repair a memory leak in GuildsList ([#4585](https://github.com/EQEmu/Server/pull/4585)) @neckkola 2025-01-06
* Resolve a client crash when logging in or zoning ([#4572](https://github.com/EQEmu/Server/pull/4572)) @neckkola 2024-12-14

### Groups

* Fix AmIMainAssist incorrectly checking for MainTankName ([#4565](https://github.com/EQEmu/Server/pull/4565)) @nytmyr 2024-12-04

### Inventory

* Add GetInventorySlots() Method ([#4566](https://github.com/EQEmu/Server/pull/4566)) @Kinglykrab 2025-01-06

### Logs

* Improve Crash log defaults ([#4579](https://github.com/EQEmu/Server/pull/4579)) @Akkadius 2025-01-06

### Maps

* Fix broken Map MMFS implementation ([#4576](https://github.com/EQEmu/Server/pull/4576)) @Akkadius 2025-01-06

### Network

* Prune / disconnect TCP connections gracefully ([#4574](https://github.com/EQEmu/Server/pull/4574)) @Akkadius 2025-01-06

### Rules

* Add rules for requiring custom files from client ([#4561](https://github.com/EQEmu/Server/pull/4561)) @knervous 2024-12-12

## [22.60.0] 11/25/2024

### Bazaar

* Further refinements for instanced bazaar ([#4544](https://github.com/EQEmu/Server/pull/4544)) @neckkola 2024-11-16

### Code

* Fix build with older C++ libraries ([#4549](https://github.com/EQEmu/Server/pull/4549)) @hgtw 2024-11-24

### Config

* Fix World TCP Address Configuration Default ([#4551](https://github.com/EQEmu/Server/pull/4551)) @Akkadius 2024-11-24

### Fixes

* Fix Issue with Perl EVENT_PAYLOAD ([#4545](https://github.com/EQEmu/Server/pull/4545)) @Kinglykrab 2024-11-24
* Fix Possible Item Loss in Trades ([#4554](https://github.com/EQEmu/Server/pull/4554)) @Kinglykrab 2024-11-24
* Fix Strings::Commify bug with #mystats ([#4547](https://github.com/EQEmu/Server/pull/4547)) @carolus21rex 2024-11-22
* Fix an edge case with augmented items inside parceled containers ([#4546](https://github.com/EQEmu/Server/pull/4546)) @neckkola 2024-11-21
* Fix for bazaar search of containers. ([#4540](https://github.com/EQEmu/Server/pull/4540)) @neckkola 2024-11-15
* Fix for mult-instanced bazaar zones ([#4541](https://github.com/EQEmu/Server/pull/4541)) @neckkola 2024-11-15
* Fix for sending money via Parcel, then changing your mind ([#4552](https://github.com/EQEmu/Server/pull/4552)) @neckkola 2024-11-24
* Fix issue where NPC's are being hidden as traders ([#4539](https://github.com/EQEmu/Server/pull/4539)) @Akkadius 2024-11-15
* Players could become flagged as a Trader when they were not trading ([#4553](https://github.com/EQEmu/Server/pull/4553)) @neckkola 2024-11-24

### Rules

* Add Rule to Disable NPCs Facing Target ([#4543](https://github.com/EQEmu/Server/pull/4543)) @Kinglykrab 2024-11-24

### Tasks

* Update tasks in all zones if invalid zone set ([#4550](https://github.com/EQEmu/Server/pull/4550)) @hgtw 2024-11-25

## [22.59.1] 11/13/2024

### Hotfix

* Fix faulty database migration condition with databuckets (9285)

## [22.59.0] 11/13/2024

### Databuckets

* Add database index to data_buckets ([#4535](https://github.com/EQEmu/Server/pull/4535)) @Akkadius 2024-11-09

### Fixes

* Bazaar two edge case issues resolved ([#4533](https://github.com/EQEmu/Server/pull/4533)) @neckkola 2024-11-09
* Check if the mob is already in the close mobs list before inserting  @Akkadius 2024-11-11
* ScanCloseMobs - Ensure scanning mob has an entity ID  @Akkadius 2024-11-10

### Performance

* Improvements to ScanCloseMobs logic ([#4534](https://github.com/EQEmu/Server/pull/4534)) @Akkadius 2024-11-08

### Quest API

* Add Native Database Querying Interface ([#4531](https://github.com/EQEmu/Server/pull/4531)) @hgtw 2024-11-13

### Rules

* Add Rule for restricting client versions to world server ([#4527](https://github.com/EQEmu/Server/pull/4527)) @knervous 2024-11-12

## [22.58.0] 11/5/2024

### Code

* Add mysql prepared statement support ([#4530](https://github.com/EQEmu/Server/pull/4530)) @hgtw 2024-11-06
* Update perlbind to 1.1.0 ([#4529](https://github.com/EQEmu/Server/pull/4529)) @hgtw 2024-11-06

### Feature

* Focus Skill Attack Spells ([#4528](https://github.com/EQEmu/Server/pull/4528)) @mmcgarvey 2024-10-31

### Fixes

* Add Missing Lua Registers ([#4525](https://github.com/EQEmu/Server/pull/4525)) @Kinglykrab 2024-10-24
* Fix cross_zone_set_entity_variable_by_char_id in Lua ([#4526](https://github.com/EQEmu/Server/pull/4526)) @Kinglykrab 2024-10-24

### Loginserver

* Automatifc Opcode File Creation ([#4521](https://github.com/EQEmu/Server/pull/4521)) @KimLS 2024-10-22

### Quest API

* Add Spawn Circle/Grid Methods to Perl/Lua ([#4524](https://github.com/EQEmu/Server/pull/4524)) @Kinglykrab 2024-10-24

## [22.57.1] 10/22/2024

### Bots

* Enable Bot Commands Only if Rule Enabled ([#4519](https://github.com/EQEmu/Server/pull/4519)) @Kinglykrab 2024-10-22
* Fix pet buffs from saving duplicates every save ([#4520](https://github.com/EQEmu/Server/pull/4520)) @nytmyr 2024-10-22

### Loginserver

* Automatic Opcode File Creation ([#4521](https://github.com/EQEmu/Server/pull/4521)) @KimLS 2024-10-22

## [22.57.0] 10/20/2024

### Bots

* Add "silent" option to ^spawn and mute raid spawn ([#4494](https://github.com/EQEmu/Server/pull/4494)) @nytmyr 2024-10-05
* Add attack flag when told to attack ([#4490](https://github.com/EQEmu/Server/pull/4490)) @nytmyr 2024-09-29
* Fix timers loading on spawn and zone ([#4516](https://github.com/EQEmu/Server/pull/4516)) @nytmyr 2024-10-20

### Code

* Fixed a typo in Zoning.cpp ([#4515](https://github.com/EQEmu/Server/pull/4515)) @carolus21rex 2024-10-20
* Optimization Code Cleanup ([#4489](https://github.com/EQEmu/Server/pull/4489)) @Akkadius 2024-09-30
* Remove Extra Skill in EQ::skills::GetExtraDamageSkills() ([#4486](https://github.com/EQEmu/Server/pull/4486)) @Kinglykrab 2024-10-03

### Crash

* Fixes a crash when the faction_list db table is empty. ([#4511](https://github.com/EQEmu/Server/pull/4511)) @KimLS 2024-10-14

### Fixes

* Add character_instance_safereturns to tables_to_zero_id ([#4485](https://github.com/EQEmu/Server/pull/4485)) @Morzain 2024-09-26
* Correctly limit max targets of PBAOE ([#4507](https://github.com/EQEmu/Server/pull/4507)) @catapultam-habeo 2024-10-11
* FindBestZ selecting false zone floor as bestz - Results in roambox failures ([#4504](https://github.com/EQEmu/Server/pull/4504)) @fryguy503 2024-10-13
* Fix #set motd Crash ([#4495](https://github.com/EQEmu/Server/pull/4495)) @Kinglykrab 2024-10-05
* Fix `character_exp_modifiers` Default Values ([#4502](https://github.com/EQEmu/Server/pull/4502)) @Kinglykrab 2024-10-09
* Fix a display error regarding a few trader/buyer query errors ([#4514](https://github.com/EQEmu/Server/pull/4514)) @neckkola 2024-10-17
* Fix Group ID 0 in Group::SaveGroupLeaderAA() ([#4487](https://github.com/EQEmu/Server/pull/4487)) @Kinglykrab 2024-10-03
* Fix Mercenary Encounter Crash ([#4509](https://github.com/EQEmu/Server/pull/4509)) @Kinglykrab 2024-10-12
* Fix NPC::CanTalk() Crash ([#4499](https://github.com/EQEmu/Server/pull/4499)) @Kinglykrab 2024-10-07
* Fix Spells:DefaultAOEMaxTargets Default Value ([#4508](https://github.com/EQEmu/Server/pull/4508)) @Kinglykrab 2024-10-12
* Fix Targeted AOE Max Targets Rule ([#4488](https://github.com/EQEmu/Server/pull/4488)) @Kinglykrab 2024-10-03
* fixed a bug where it would use npc value instead of faction value in the database. ([#4491](https://github.com/EQEmu/Server/pull/4491)) @regneq 2024-09-29
* Master of Disguise should apply to illusions casted by others. ([#4506](https://github.com/EQEmu/Server/pull/4506)) @fryguy503 2024-10-11
* Spells - Self Only (Yellow) cast when non group member is targeted ([#4503](https://github.com/EQEmu/Server/pull/4503)) @fryguy503 2024-10-11

### Loginserver

* Larion loginserver support ([#4492](https://github.com/EQEmu/Server/pull/4492)) @KimLS 2024-10-03
* Login Fatal Error Spamming ([#4476](https://github.com/EQEmu/Server/pull/4476)) @KimLS 2024-10-09

### Logs

* Add NPC Trades to Player Events ([#4505](https://github.com/EQEmu/Server/pull/4505)) @Kinglykrab 2024-10-13

### Quest API

* Add Buff Fade Methods to Perl/Lua ([#4501](https://github.com/EQEmu/Server/pull/4501)) @Kinglykrab 2024-10-09
* Add EVENT_READ_ITEM to Perl/Lua ([#4497](https://github.com/EQEmu/Server/pull/4497)) @Kinglykrab 2024-10-08
* Add NPC List Filter Methods to Perl/Lua ([#4493](https://github.com/EQEmu/Server/pull/4493)) @Kinglykrab 2024-10-04
* Add Scripting Support to Mercenaries ([#4500](https://github.com/EQEmu/Server/pull/4500)) @Kinglykrab 2024-10-11

### Rules

* Add Rule to disable PVP Regions ([#4513](https://github.com/EQEmu/Server/pull/4513)) @Kinglykrab 2024-10-17

## [22.56.3] 9/23/2024

### Fixes

* Fix issue with Client::SaveDisciplines() not specifying character ID ([#4481](https://github.com/EQEmu/Server/pull/4477)) @Kinglykrab 2024-09-23

## [22.56.2] 9/20/2024

### Fixes

* Fix Issue with Database::ReserveName ([#4477](https://github.com/EQEmu/Server/pull/4477)) @Kinglykrab 2024-09-20

### Quest API

* Add GrantAllAAPoints() Overload To Perl/Lua ([#4474](https://github.com/EQEmu/Server/pull/4474)) @Kinglykrab 2024-09-20

## [22.56.1] 9/20/2024

### Fixes

* Fix Untrained Disciplines in Client::SaveDisciplines() ([#4472](https://github.com/EQEmu/Server/pull/4472)) @Kinglykrab 2024-09-13
* Fix Infinite Loop in Adventure::Finished() ([#4473](https://github.com/EQEmu/Server/pull/4473)) @oddx2k 2024-09-13

## [22.56.0] 9/12/2024

### Code

* Add IsCloseToBanker method ([#4462](https://github.com/EQEmu/Server/pull/4462)) @Akkadius 2024-08-27

### Feature

* Add Rule to Limit Task Update Messages ([#4459](https://github.com/EQEmu/Server/pull/4459)) @Kinglykrab 2024-08-28
* Allow NPCs to cast Sacrifice ([#4470](https://github.com/EQEmu/Server/pull/4470)) @fuzzlecutter 2024-09-12
* Lazy Load Bank Contents ([#4453](https://github.com/EQEmu/Server/pull/4453)) @catapultam-habeo 2024-08-27

### Fixes

* Add RULE_STRING to RuleManager::ResetRules ([#4467](https://github.com/EQEmu/Server/pull/4467)) @Kinglykrab 2024-09-07
* Fix Bard Effect in Migration 9237 ([#4468](https://github.com/EQEmu/Server/pull/4468)) @Kinglykrab 2024-09-09
* ModernAAScalingEnabled() Calculation Error ([#4469](https://github.com/EQEmu/Server/pull/4469)) @carolus21rex 2024-09-11

### Performance

* Move Discipline Loading to Client::CompleteConnect() ([#4466](https://github.com/EQEmu/Server/pull/4466)) @Kinglykrab 2024-09-09

### Rules

* Add a Bandolier Swap Delay Rule ([#4465](https://github.com/EQEmu/Server/pull/4465)) @Kinglykrab 2024-09-08

## [22.55.1] 8/26/2024

### Code

* Remove unused methods ([#4449](https://github.com/EQEmu/Server/pull/4449)) @Kinglykrab 2024-08-22

### Feature

* Add Character:DefaultGuildRank Rule ([#4438](https://github.com/EQEmu/Server/pull/4438)) @Kinglykrab 2024-08-04
* Add Optional Return to EVENT_DAMAGE_TAKEN ([#4454](https://github.com/EQEmu/Server/pull/4454)) @Kinglykrab 2024-08-27
* Extend Spell Buckets Functionality ([#4441](https://github.com/EQEmu/Server/pull/4441)) @Kinglykrab 2024-08-22

### Fixes

* Apply Race & Class restrictions to Auto-Combines ([#4452](https://github.com/EQEmu/Server/pull/4452)) @catapultam-habeo 2024-08-20
* Attune Augments when Equipped ([#4446](https://github.com/EQEmu/Server/pull/4446)) @fryguy503 2024-08-10
* Correct missed maxlevel reference in exp.cpp ([#4463](https://github.com/EQEmu/Server/pull/4463)) @N0ctrnl 2024-08-27
* Ensure close of Tribute Item search ([#4439](https://github.com/EQEmu/Server/pull/4439)) @joligario 2024-08-04
* Fix AddCrystals() in Perl/Lua ([#4445](https://github.com/EQEmu/Server/pull/4445)) @Kinglykrab 2024-08-10
* Fix Bot Spell Entries IDs Capping at 32,767 ([#4444](https://github.com/EQEmu/Server/pull/4444)) @Kinglykrab 2024-08-27
* Fix Character ID of 0 being inserted into character_stats_record ([#4458](https://github.com/EQEmu/Server/pull/4458)) @Kinglykrab 2024-08-22
* Fix Issue with Removed #setfaction Command ([#4448](https://github.com/EQEmu/Server/pull/4448)) @Kinglykrab 2024-08-11
* Fix Lua Client FilteredMessage ([#4437](https://github.com/EQEmu/Server/pull/4437)) @Kinglykrab 2024-07-31
* Fix client hotbar exchanging items when zoning ([#4460](https://github.com/EQEmu/Server/pull/4460)) @neckkola 2024-08-27
* Fix issue with killed mob coordinates ([#4457](https://github.com/EQEmu/Server/pull/4457)) @Kinglykrab 2024-08-22
* Imitate Death should also clear zone feign aggro ([#4436](https://github.com/EQEmu/Server/pull/4436)) @fryguy503 2024-07-31
* client_max_level allow leveling to end of level ([#4455](https://github.com/EQEmu/Server/pull/4455)) @fryguy503 2024-08-20

### Improvement

* Filtered Messages Extension ([#4435](https://github.com/EQEmu/Server/pull/4435)) @fryguy503 2024-07-31

### Quest API

* Add AreTasksCompleted() to Perl/Lua. ([#4456](https://github.com/EQEmu/Server/pull/4456)) @Kinglykrab 2024-08-23
* Add Area-Based Quest Methods to Perl/Lua ([#4447](https://github.com/EQEmu/Server/pull/4447)) @Kinglykrab 2024-08-27
* Add Several Door Methods to Perl/Lua ([#4451](https://github.com/EQEmu/Server/pull/4451)) @Kinglykrab 2024-08-16

### World

* Fix slow world bootup bug ([#4461](https://github.com/EQEmu/Server/pull/4461)) @Akkadius 2024-08-27

## [22.54.0] 7/30/2024

### Code

* Cleanup Client File Exporting ([#4348](https://github.com/EQEmu/Server/pull/4348)) @Kinglykrab 2024-07-31
* Cleanup Stance Code ([#4368](https://github.com/EQEmu/Server/pull/4368)) @Kinglykrab 2024-07-03
* Mask GM Show Buff message behind EntityVariable ([#4419](https://github.com/EQEmu/Server/pull/4419)) @nytmyr 2024-07-22

### Commands

* Extend #devtools Functionality ([#4425](https://github.com/EQEmu/Server/pull/4425)) @Kinglykrab 2024-07-23

### Databuckets

* Remove memory reserve from bulk load ([#4427](https://github.com/EQEmu/Server/pull/4427)) @Akkadius 2024-07-23

### Feature

* Add Barter/Buyer Features ([#4405](https://github.com/EQEmu/Server/pull/4405)) @neckkola 2024-07-30
* Add Parcel notification for online players when using the Quest API ([#4418](https://github.com/EQEmu/Server/pull/4418)) @neckkola 2024-07-22
* Implement Move Multiple Items ([#4259](https://github.com/EQEmu/Server/pull/4259)) @catapultam-habeo 2024-07-30

### Fixes

* Aegolism Spell line stacking ([#4399](https://github.com/EQEmu/Server/pull/4399)) @KayenEQ 2024-07-07
* AllowRaidTargetBlind logic backwards ([#4400](https://github.com/EQEmu/Server/pull/4400)) @fryguy503 2024-07-01
* AutoSplit unknown bug and cleanup. ([#4401](https://github.com/EQEmu/Server/pull/4401)) @fryguy503 2024-07-07
* Corpse Call removing Resurrection Effects ([#4410](https://github.com/EQEmu/Server/pull/4410)) @fryguy503 2024-07-22
* Fix #parcels add subcommand ([#4431](https://github.com/EQEmu/Server/pull/4431)) @neckkola 2024-07-29
* Fix #setlevel Allowing Skills Above Max ([#4423](https://github.com/EQEmu/Server/pull/4423)) @Kinglykrab 2024-07-23
* Fix Bot::SetBotStance ([#4426](https://github.com/EQEmu/Server/pull/4426)) @Kinglykrab 2024-07-23
* Fix Client::RemoveTitle ([#4421](https://github.com/EQEmu/Server/pull/4421)) @Kinglykrab 2024-07-23
* Fix EVENT_USE_SKILL with Sense Heading ([#4424](https://github.com/EQEmu/Server/pull/4424)) @Kinglykrab 2024-07-23
* Fix for random disconnects when a large number of guild members zone or disconnect ([#4402](https://github.com/EQEmu/Server/pull/4402)) @neckkola 2024-07-10
* Fix issue with quest::echo and quest::me ([#4433](https://github.com/EQEmu/Server/pull/4433)) @Kinglykrab 2024-07-30
* Personal tributes for bard items were not applying correctly ([#4416](https://github.com/EQEmu/Server/pull/4416)) @neckkola 2024-07-16
* Potential fix for some undesired ranged explotative behavior. ([#4413](https://github.com/EQEmu/Server/pull/4413)) @fryguy503 2024-07-22
* Proximity Aggro for Frustrated and Undead ([#4411](https://github.com/EQEmu/Server/pull/4411)) @fryguy503 2024-07-22
* Slay Adjustments ([#4389](https://github.com/EQEmu/Server/pull/4389)) @fryguy503 2024-07-07
* Stop DOSing ourselves with OP_WearChange ([#4432](https://github.com/EQEmu/Server/pull/4432)) @catapultam-habeo 2024-07-30
* [Quest API] Fix getraididbycharid and getgroupidbycharid ([#4417](https://github.com/EQEmu/Server/pull/4417)) @nytmyr 2024-07-16

### Improvement

* Flee Overhaul ([#4407](https://github.com/EQEmu/Server/pull/4407)) @fryguy503 2024-07-30

### Rules

* Add HasteCap and Hastev3Cap rules for NPCs, Bots and Mercs ([#4406](https://github.com/EQEmu/Server/pull/4406)) @nytmyr 2024-07-22

### Zone Instances

* Revert " Handle routing to instances when using evac/succor " (#4429) ([#4297](https://github.com/EQEmu/Server/pull/4297)) @Akkadius 2024-07-30

### Zoning

* Improve zone routing ([#4428](https://github.com/EQEmu/Server/pull/4428)) @Akkadius 2024-07-30

## [22.53.1] 6/16/2024

### Fixes

* Fix trader mode ([#4397](https://github.com/EQEmu/Server/pull/4397)) @joligario 2024-06-17

## [22.53.0] 6/14/2024

### Bug

* Anon players should not show in /who all ([#4392](https://github.com/EQEmu/Server/pull/4392)) @fryguy503 2024-06-14
* Escape should put player into SOS if owned. ([#4388](https://github.com/EQEmu/Server/pull/4388)) @fryguy503 2024-06-07
* Prevent Resurrection Spells from being resisted ([#4393](https://github.com/EQEmu/Server/pull/4393)) @fryguy503 2024-06-14

### Code

* Cleanup Account Status Code ([#4376](https://github.com/EQEmu/Server/pull/4376)) @Kinglykrab 2024-06-02
* Cleanup Body Type Code ([#4366](https://github.com/EQEmu/Server/pull/4366)) @Kinglykrab 2024-06-02
* Cleanup Object Type Code ([#4375](https://github.com/EQEmu/Server/pull/4375)) @Kinglykrab 2024-06-14
* Remove unused code in emu_constants.h ([#4384](https://github.com/EQEmu/Server/pull/4384)) @Kinglykrab 2024-06-14

### Fixes

* Fix #goto Target ([#4382](https://github.com/EQEmu/Server/pull/4382)) @Kinglykrab 2024-06-03
* Fix Swarm Pet Damage Messages ([#4383](https://github.com/EQEmu/Server/pull/4383)) @Kinglykrab 2024-06-04
* Fix for players having empty bazaar window dropdown list, even though trader is tagged as a trader. ([#4391](https://github.com/EQEmu/Server/pull/4391)) @neckkola 2024-06-14
* Fix potential trader crash when serialized item not found ([#4386](https://github.com/EQEmu/Server/pull/4386)) @joligario 2024-06-14

### Rules

* Add Invisible Augment Rules ([#4385](https://github.com/EQEmu/Server/pull/4385)) @Kinglykrab 2024-06-14
* Classic Harm Touch Formula ([#4394](https://github.com/EQEmu/Server/pull/4394)) @fryguy503 2024-06-14
* Mend/Sneak allow success tuning ([#4390](https://github.com/EQEmu/Server/pull/4390)) @fryguy503 2024-06-14
* Snare Override Movement Bonus ([#4381](https://github.com/EQEmu/Server/pull/4381)) @fryguy503 2024-06-02

## [22.52.0] 6/1/2024

### Code

* Cleanup Bucket Comparison Code ([#4374](https://github.com/EQEmu/Server/pull/4374)) @Kinglykrab 2024-06-02
* Cleanup Bug Category Code ([#4367](https://github.com/EQEmu/Server/pull/4367)) @Kinglykrab 2024-06-01
* Cleanup Deity Code ([#4363](https://github.com/EQEmu/Server/pull/4363)) @Kinglykrab 2024-06-01
* Cleanup Special Ability Code ([#4365](https://github.com/EQEmu/Server/pull/4365)) @Kinglykrab 2024-06-01
* Remove unused code in common/eq_constants.h ([#4364](https://github.com/EQEmu/Server/pull/4364)) @Kinglykrab 2024-06-01

### Combat

* Adjustments to Crippling Blows/Slay Undead and Confirmed Critical Code ([#4354](https://github.com/EQEmu/Server/pull/4354)) @fryguy503 2024-05-27

### Fixes

* Add protection to ensure adventure points award are only attempted on players ([#4371](https://github.com/EQEmu/Server/pull/4371)) @joligario 2024-05-31
* Adjust Kick/RoundKick Damage Lower levels ([#4355](https://github.com/EQEmu/Server/pull/4355)) @fryguy503 2024-05-28
* Bazaar Search not working correctly for Iksar, Vashir, Drakkin and Froglok races ([#4379](https://github.com/EQEmu/Server/pull/4379)) @neckkola 2024-06-02
* Fix Unescaped String in Client::GotoPlayer ([#4373](https://github.com/EQEmu/Server/pull/4373)) @Kinglykrab 2024-06-01

### NPC Spells

* Fixed an issue where the repository spell adj value was overriding the spell difficulty default value ([#4370](https://github.com/EQEmu/Server/pull/4370)) @regneq 2024-06-01

### Quest API

* Add Item Link Methods to Perl/Lua ([#4359](https://github.com/EQEmu/Server/pull/4359)) @Kinglykrab 2024-06-01

### Quests

* Fix Lua encounter double register ([#4369](https://github.com/EQEmu/Server/pull/4369)) @Akkadius 2024-05-31
* Fix issue with Lua encounters loading in certain circumstances ([#4378](https://github.com/EQEmu/Server/pull/4378)) @Akkadius 2024-06-01

### Rules

* Add Skill Base Damage Rules ([#4360](https://github.com/EQEmu/Server/pull/4360)) @Kinglykrab 2024-06-01

### Skills

* Fix caps out of bounds issue ([#4377](https://github.com/EQEmu/Server/pull/4377)) @Akkadius 2024-06-01

## [22.51.1] 5/27/2024

### Fixes

* Adjust return for perl release check  @Akkadius 2024-05-26
* Corrected issue with bazaar purchase via parcels where an incorrect quantity would be calculated. ([#4352](https://github.com/EQEmu/Server/pull/4352)) @neckkola 2024-05-27

### Performance

* Improve SkillCaps::GetTrainLevel() Efficiency ([#4350](https://github.com/EQEmu/Server/pull/4350)) @Kinglykrab 2024-05-26

### Rules

* Legacy Compute Defense against modern agi based defense. ([#4349](https://github.com/EQEmu/Server/pull/4349)) @fryguy503 2024-05-27

## [22.51.0] 5/26/2024

### Commands

* #npcspawn Changes ([#4311](https://github.com/EQEmu/Server/pull/4311)) @twincannon 2024-05-16
* Cleanup #resetaa Command ([#4310](https://github.com/EQEmu/Server/pull/4310)) @Kinglykrab 2024-05-22

### Crash

* Add validation to RemoveXTarget ([#4324](https://github.com/EQEmu/Server/pull/4324)) @Akkadius 2024-05-25
* Fix Zone deconstructor crashes ([#4325](https://github.com/EQEmu/Server/pull/4325)) @Akkadius 2024-05-25
* Fix crash issue when dividing by zero in CalcHPRegen ([#4320](https://github.com/EQEmu/Server/pull/4320)) @Akkadius 2024-05-25
* Fix crash when map name is null ([#4322](https://github.com/EQEmu/Server/pull/4322)) @Akkadius 2024-05-25
* Fix player event crash in ITEM_DESTROY ([#4326](https://github.com/EQEmu/Server/pull/4326)) @Akkadius 2024-05-25
* Fix player events reload when out of bounds ([#4321](https://github.com/EQEmu/Server/pull/4321)) @Akkadius 2024-05-25
* Fix rarer crash in EntityList::MobProcess ([#4319](https://github.com/EQEmu/Server/pull/4319)) @Akkadius 2024-05-25

### Feature

* Add RoF2 Bazaar Support ([#4315](https://github.com/EQEmu/Server/pull/4315)) @neckkola 2024-05-26
* Add SE_IncreaseArchery and rules to tune archery ([#4335](https://github.com/EQEmu/Server/pull/4335)) @fryguy503 2024-05-26
* Add parcel container support ([#4305](https://github.com/EQEmu/Server/pull/4305)) @neckkola 2024-05-17

### Fixes

* Accuracy, Avoidance and Atk adjustments ([#4336](https://github.com/EQEmu/Server/pull/4336)) @fryguy503 2024-05-26
* Fix Crash with null Argument in #modifynpcstat ([#4318](https://github.com/EQEmu/Server/pull/4318)) @Kinglykrab 2024-05-24
* Fix RemoveAlternateCurrencyValue not updating Client ([#4317](https://github.com/EQEmu/Server/pull/4317)) @Kinglykrab 2024-05-23
* Fix Using Bind Wound Above 70% Health ([#4340](https://github.com/EQEmu/Server/pull/4340)) @Kinglykrab 2024-05-26
* Fix issue with #hotfix ([#4316](https://github.com/EQEmu/Server/pull/4316)) @Kinglykrab 2024-05-22
* Fix issue with #suspend ([#4314](https://github.com/EQEmu/Server/pull/4314)) @Kinglykrab 2024-05-23
* Fix issue with KeepOneRecordPerCompletedTask ([#4313](https://github.com/EQEmu/Server/pull/4313)) @Kinglykrab 2024-05-23
* Fix mistaken removed RULE_CATEGORY_END() ([#4341](https://github.com/EQEmu/Server/pull/4341)) @fryguy503 2024-05-26
* Missed a mob offense section for PR #4328 ([#4331](https://github.com/EQEmu/Server/pull/4331)) @fryguy503 2024-05-26
* Raid Targets should not be Blindable as this will break all spell casting AI. ([#4334](https://github.com/EQEmu/Server/pull/4334)) @fryguy503 2024-05-26
* When Mounts are allowed to zone, block them from zoning to disallowed zones. ([#4330](https://github.com/EQEmu/Server/pull/4330)) @fryguy503 2024-05-25
* When refreshing buffs, attempt to use the same buffslot if the buff still exists. ([#4338](https://github.com/EQEmu/Server/pull/4338)) @fryguy503 2024-05-26

### Lua Mod

* Fix issue with SetAAEXP and SetEXP firing when uninitialized ([#4345](https://github.com/EQEmu/Server/pull/4345)) @Akkadius 2024-05-26

### Merchants

* Add New Classic Greed/Faction/Charisma Prices Rule ([#4301](https://github.com/EQEmu/Server/pull/4301)) @noudess 2024-05-17

### Mobs

* Remove entity type checks from ScanCloseMobs ([#4323](https://github.com/EQEmu/Server/pull/4323)) @Akkadius 2024-05-25

### NPC Spells

* Fix an issue where procs wouldn't fire if no spell entries in list ([#4344](https://github.com/EQEmu/Server/pull/4344)) @Akkadius 2024-05-26

### Perl

* Linux /opt/eqemu-perl checks when using release binaries ([#4346](https://github.com/EQEmu/Server/pull/4346)) @Akkadius 2024-05-26

### Quest API

* Add Zone Uptime Exports to Perl/Lua ([#4339](https://github.com/EQEmu/Server/pull/4339)) @Kinglykrab 2024-05-26

### Rules

* Added MeleeMitigation Level Difference Roll Adjusted for level diffs ([#4332](https://github.com/EQEmu/Server/pull/4332)) @fryguy503 2024-05-26
* Allow maximum per kill AA amount ([#4329](https://github.com/EQEmu/Server/pull/4329)) @fryguy503 2024-05-25
* Allow servers to adjust the filtering threshold for heals from damage (e.g. Mark of Kings). ([#4327](https://github.com/EQEmu/Server/pull/4327)) @fryguy503 2024-05-25
* Backstab Haste Correction ([#4337](https://github.com/EQEmu/Server/pull/4337)) @fryguy503 2024-05-26
* Mob Offensive and Weapon Skill static tables ([#4328](https://github.com/EQEmu/Server/pull/4328)) @fryguy503 2024-05-25
* Remove hard coded initial aggro in favor or an adjustable Rule ([#4333](https://github.com/EQEmu/Server/pull/4333)) @fryguy503 2024-05-26

### Scripts

* Fix zone data load ordering issue ([#4343](https://github.com/EQEmu/Server/pull/4343)) @Akkadius 2024-05-26

### Spells

* Add content filtering to NPC spells ([#4309](https://github.com/EQEmu/Server/pull/4309)) @Akkadius 2024-05-17

## [22.50.1] 5/12/2024

### Fixes

* Clear GuildOnlineStatus on world boot ([#4306](https://github.com/EQEmu/Server/pull/4306)) @neckkola 2024-05-12

## [22.50.0] 5/9/2024

### Code

* Fix Message in #find task ([#4270](https://github.com/EQEmu/Server/pull/4270)) @Kinglykrab 2024-04-19

### Combat

* Add rule to set stun duration.  Add rule to add stun messages. ([#4284](https://github.com/EQEmu/Server/pull/4284)) @noudess 2024-04-25

### Commands

* Adjust #kill and #list Commands ([#4271](https://github.com/EQEmu/Server/pull/4271)) @Kinglykrab 2024-05-04

### Compiling

* Add profiler flag ([#4293](https://github.com/EQEmu/Server/pull/4293)) @xackery 2024-05-04

### Corpses

* Corpse searches for items did not search augs ([#4289](https://github.com/EQEmu/Server/pull/4289)) @noudess 2024-04-27

### Database

* Add Extra Haste to Bots/Character Tables ([#4286](https://github.com/EQEmu/Server/pull/4286)) @Kinglykrab 2024-05-04
* Break out changes to `npc_types` from parcels ([#4295](https://github.com/EQEmu/Server/pull/4295)) @joligario 2024-05-03
* Move pets query to content database ([#4296](https://github.com/EQEmu/Server/pull/4296)) @joligario 2024-05-04

### Doors

* Replace magic # in code with a field in doors table for when door closes. ([#4288](https://github.com/EQEmu/Server/pull/4288)) @noudess 2024-04-25

### Experience

* Add SetExp/SetAAExp lua mods ([#4292](https://github.com/EQEmu/Server/pull/4292)) @xackery 2024-05-04

### Feature

* Add Augmentation Support for Parcels ([#4285](https://github.com/EQEmu/Server/pull/4285)) @neckkola 2024-04-24
* Add Character Auto Login ([#4216](https://github.com/EQEmu/Server/pull/4216)) @Kinglykrab 2024-04-15
* Add Parcel Feature for RoF2 Clients ([#4198](https://github.com/EQEmu/Server/pull/4198)) @neckkola 2024-04-21
* Add `is_parcel_merchant` to #npcedit ([#4290](https://github.com/EQEmu/Server/pull/4290)) @Kinglykrab 2024-05-04
* Add devcontainer support ([#4294](https://github.com/EQEmu/Server/pull/4294)) @xackery 2024-05-09
* Fix ignore_default on lua mod damage ([#4228](https://github.com/EQEmu/Server/pull/4228)) @xackery 2024-04-15

### Fixes

* "GM flag" vs "GM Flag"  @Akkadius 2024-05-04
* All NPC classes could stun with kick/only warriors could be stunned ([#4273](https://github.com/EQEmu/Server/pull/4273)) @noudess 2024-04-24
* Fix Account Flags Loading ([#4243](https://github.com/EQEmu/Server/pull/4243)) @Kinglykrab 2024-04-15
* Fix Corpses "Disappearing" ([#4275](https://github.com/EQEmu/Server/pull/4275)) @joligario 2024-04-21
* Fix Issue With Auto Login ([#4267](https://github.com/EQEmu/Server/pull/4267)) @Kinglykrab 2024-04-21
* Fix crash from #4266  @Akkadius 2024-05-05
* Fixes Issue 4257 - Double Taunt Message ([#4258](https://github.com/EQEmu/Server/pull/4258)) @fryguy503 2024-04-18
* GLAA fix after database.cpp updates ([#4277](https://github.com/EQEmu/Server/pull/4277)) @neckkola 2024-04-21
* GetSkill default return 0  @Akkadius 2024-05-05
* Hero forge armor bug on login and show helm toggle. ([#4246](https://github.com/EQEmu/Server/pull/4246)) @noudess 2024-04-15
* Perl__send_parcel GCC compile error  @Akkadius 2024-05-05
* Remove food or water GM message  @Akkadius 2024-05-05
* Revert GetSkill from #4234  @Akkadius 2024-05-05
* Fix for world crash with over 1500 guilds ([#4299](https://github.com/EQEmu/Server/pull/4299)) @neckkola 2024-05-09

### Hot Fix

* Update database_update_manifest.cpp ([#4291](https://github.com/EQEmu/Server/pull/4291)) @noudess 2024-04-29

### Loot

* Add content filtering to lootdrop_entries ([#4229](https://github.com/EQEmu/Server/pull/4229)) @Akkadius 2024-04-15
* Consolidate filtering logic ([#4280](https://github.com/EQEmu/Server/pull/4280)) @Akkadius 2024-04-21

### Lua

* Add IsImmuneToSpell Lua Mod ([#4278](https://github.com/EQEmu/Server/pull/4278)) @xackery 2024-04-28
* Add UpdatePersonalFaction Lua Mod ([#4279](https://github.com/EQEmu/Server/pull/4279)) @xackery 2024-05-04

### Messages

* Add GM Status and Invulnerability Messages ([#4266](https://github.com/EQEmu/Server/pull/4266)) @Kinglykrab 2024-05-04
* Fix bug where DoT messages stop coming out when mob dies. ([#4249](https://github.com/EQEmu/Server/pull/4249)) @noudess 2024-04-15
* Fix multiple errors in spell damage at death. ([#4264](https://github.com/EQEmu/Server/pull/4264)) @noudess 2024-04-28

### Performance

* Change skill_cap from vector to map ([#4252](https://github.com/EQEmu/Server/pull/4252)) @xackery 2024-04-15

### Pets

* Fix errors in sync with client of sit/stand ([#4245](https://github.com/EQEmu/Server/pull/4245)) @noudess 2024-04-15

### Quest API

* Add DescribeSpecialAbilities() to Perl/Lua ([#4269](https://github.com/EQEmu/Server/pull/4269)) @Kinglykrab 2024-04-21
* Add GetConsiderColor() to Perl/Lua ([#4253](https://github.com/EQEmu/Server/pull/4253)) @Kinglykrab 2024-04-15
* Add Parcel Sending to Perl/Lua ([#4287](https://github.com/EQEmu/Server/pull/4287)) @Kinglykrab 2024-05-04
* Add Zone Methods to Perl/Lua ([#4268](https://github.com/EQEmu/Server/pull/4268)) @Kinglykrab 2024-04-21

### Quests

* Hot Reload Changes ([#4302](https://github.com/EQEmu/Server/pull/4302)) @Akkadius 2024-05-09
* Reload Quests on Bootup, Init earlier ([#4298](https://github.com/EQEmu/Server/pull/4298)) @Akkadius 2024-05-05

### Spells

* Implemented SPA 122 SE_ReduceSkill ([#4234](https://github.com/EQEmu/Server/pull/4234)) @KayenEQ 2024-05-04
* Normal Group Spells (non-raid) landed twice on caster ([#4240](https://github.com/EQEmu/Server/pull/4240)) @noudess 2024-04-15
* SPA69 TotalHP can be used in Worn Slot, Fixes/Updates to Max HP related variables. ([#4244](https://github.com/EQEmu/Server/pull/4244)) @KayenEQ 2024-04-15

### Zone Instances

* Handle routing to instances when using evac/succor ([#4297](https://github.com/EQEmu/Server/pull/4297)) @joligario 2024-05-04

## [22.49.1] 4/15/2024

### Zoning

* Fix zone routing edge case ([#4255](https://github.com/EQEmu/Server/pull/4255)) @Akkadius 2024-04-15

## [22.49.0] 4/15/2024

### Code

* Avoid unnecessary copies in database functions ([#4220](https://github.com/EQEmu/Server/pull/4220)) @joligario 2024-03-29
* Reference type in `GetRaidLeaderName` ([#4218](https://github.com/EQEmu/Server/pull/4218)) @joligario 2024-03-28
* Remove unnecessary reference types ([#4212](https://github.com/EQEmu/Server/pull/4212)) @joligario 2024-03-28

### Crash

* Check mob pointer before trying to remove it ([#4230](https://github.com/EQEmu/Server/pull/4230)) @Akkadius 2024-04-01

### Feature

* Add Character Auto Login ([#4216](https://github.com/EQEmu/Server/pull/4216)) @Kinglykrab 2024-04-15
* Add LuaMod functions for CommonDamage and HealDamage ([#4227](https://github.com/EQEmu/Server/pull/4227)) @xackery 2024-03-31
* Add RegisterBug LuaMod ([#4209](https://github.com/EQEmu/Server/pull/4209)) @xackery 2024-03-30
* Additive Spell Focus from Worn slot with Limit Checks ([#4208](https://github.com/EQEmu/Server/pull/4208)) @KayenEQ 2024-03-30
* Fix ignore_default on lua mod damage ([#4228](https://github.com/EQEmu/Server/pull/4228)) @xackery 2024-04-15

### Fixes

* Client not updating HP bar when an HP Buff with a Heal is applied. ([#4237](https://github.com/EQEmu/Server/pull/4237)) @KayenEQ 2024-04-02
* Fix Account Flags Loading ([#4243](https://github.com/EQEmu/Server/pull/4243)) @Kinglykrab 2024-04-15
* Fix Auto Login Issue ([#4213](https://github.com/EQEmu/Server/pull/4213)) @Kinglykrab 2024-03-28
* Fix Bot Creation Issue ([#4235](https://github.com/EQEmu/Server/pull/4235)) @Kinglykrab 2024-04-02
* Fix Bot/Character ID Overlap in Groups ([#4093](https://github.com/EQEmu/Server/pull/4093)) @Kinglykrab 2024-03-23
* Fix Group Leadership ([#4214](https://github.com/EQEmu/Server/pull/4214)) @Kinglykrab 2024-03-28
* Fix Issue With Bot Raid Aggro ([#4222](https://github.com/EQEmu/Server/pull/4222)) @Kinglykrab 2024-03-31
* Fix Lua Crash with Spell Blocked Event ([#4236](https://github.com/EQEmu/Server/pull/4236)) @Kinglykrab 2024-04-01
* Fix Luabind Double Class Register ([#4219](https://github.com/EQEmu/Server/pull/4219)) @Kinglykrab 2024-03-29
* Fix crash in SendEnterWorld ([#4204](https://github.com/EQEmu/Server/pull/4204)) @Akkadius 2024-03-24
* Fix event_consider any_cast error ([#4210](https://github.com/EQEmu/Server/pull/4210)) @xackery 2024-03-27
* Fix fishing chances ([#4203](https://github.com/EQEmu/Server/pull/4203)) @joligario 2024-03-24
* Fix manifest for skill caps schema type ([#4231](https://github.com/EQEmu/Server/pull/4231)) @Akkadius 2024-04-02
* Hero forge armor bug on login and show helm toggle. ([#4246](https://github.com/EQEmu/Server/pull/4246)) @noudess 2024-04-15
* Radiant/Ebon Crystals should only extract to 1000 ([#4195](https://github.com/EQEmu/Server/pull/4195)) @fryguy503 2024-03-23
* SPA214 SE_MaxHPChange calculation errors corrected. ([#4238](https://github.com/EQEmu/Server/pull/4238)) @KayenEQ 2024-04-02
* Shared Tasks - charid is now character_id ([#4233](https://github.com/EQEmu/Server/pull/4233)) @fryguy503 2024-04-01
* Using %T in channel messages on fresh corpse yields mob, not corpse name. ([#4168](https://github.com/EQEmu/Server/pull/4168)) @noudess 2024-04-05

### Hot Fix

* Fix Group::AddToGroup ([#4201](https://github.com/EQEmu/Server/pull/4201)) @Kinglykrab 2024-03-23
* Hot Fix for Group::AddToGroup Hot Fix ([#4202](https://github.com/EQEmu/Server/pull/4202)) @Kinglykrab 2024-03-23

### Loot

* Add content filtering to lootdrop_entries ([#4229](https://github.com/EQEmu/Server/pull/4229)) @Akkadius 2024-04-15

### Lua

* Add Zone and Language Constants ([#4211](https://github.com/EQEmu/Server/pull/4211)) @fryguy503 2024-03-28

### Messages

* Fix bug where DoT messages stop coming out when mob dies. ([#4249](https://github.com/EQEmu/Server/pull/4249)) @noudess 2024-04-15

### Performance

* Change skill_cap from vector to map ([#4252](https://github.com/EQEmu/Server/pull/4252)) @xackery 2024-04-15

### Pets

* Fix errors in sync with client of sit/stand ([#4245](https://github.com/EQEmu/Server/pull/4245)) @noudess 2024-04-15

### Quest API

* Add Archetype Methods to Perl/Lua ([#4181](https://github.com/EQEmu/Server/pull/4181)) @Kinglykrab 2024-03-23
* Add Class/Deity/Race Methods to Perl/Lua ([#4215](https://github.com/EQEmu/Server/pull/4215)) @Kinglykrab 2024-03-28
* Add GetConsiderColor() to Perl/Lua ([#4253](https://github.com/EQEmu/Server/pull/4253)) @Kinglykrab 2024-04-15
* Add Spell Blocked Event to Perl/Lua ([#4217](https://github.com/EQEmu/Server/pull/4217)) @Kinglykrab 2024-04-01

### Quests

* Avoid Player and Bot quests in unloaded zone ([#4232](https://github.com/EQEmu/Server/pull/4232)) @hgtw 2024-04-01

### Repositories

* Convert database.cpp to Repositories ([#4054](https://github.com/EQEmu/Server/pull/4054)) @Kinglykrab 2024-03-24

### Skill Caps

* Further improvements ([#4205](https://github.com/EQEmu/Server/pull/4205)) @Akkadius 2024-03-24
* Remove from shared memory and simplify ([#4069](https://github.com/EQEmu/Server/pull/4069)) @Kinglykrab 2024-03-23

### Spells

* Implemented SPA 463 SE_SHIELD_TARGET ([#4224](https://github.com/EQEmu/Server/pull/4224)) @KayenEQ 2024-03-30
* Normal Group Spells (non-raid) landed twice on caster ([#4240](https://github.com/EQEmu/Server/pull/4240)) @noudess 2024-04-15
* SPA148 Stacking Fix ([#4206](https://github.com/EQEmu/Server/pull/4206)) @KayenEQ 2024-03-27
* SPA69 TotalHP can be used in Worn Slot, Fixes/Updates to Max HP related variables. ([#4244](https://github.com/EQEmu/Server/pull/4244)) @KayenEQ 2024-04-15

## [22.48.0] 3/23/2024

### Bots

* IsValidTarget Crash Fix ([#4187](https://github.com/EQEmu/Server/pull/4187)) @nytmyr 2024-03-12
* Move BotGroupSay to Pet Response ([#4171](https://github.com/EQEmu/Server/pull/4171)) @nytmyr 2024-03-08

### Code

* Cleanup Zone Get Methods ([#4169](https://github.com/EQEmu/Server/pull/4169)) @Kinglykrab 2024-03-09

### Fixes

* An Update to Xtarget to exclude Bot owned Temp/Swarm Pets ([#4172](https://github.com/EQEmu/Server/pull/4172)) @MortimerGreenwald 2024-03-08
* Fix #serverrules Command ([#4193](https://github.com/EQEmu/Server/pull/4193)) @Kinglykrab 2024-03-20
* Fix Bot Cloning ([#4186](https://github.com/EQEmu/Server/pull/4186)) @Kinglykrab 2024-03-17
* Fix Crash in ClientList::GetCLEIP ([#4173](https://github.com/EQEmu/Server/pull/4173)) @Kinglykrab 2024-03-10
* Fix Default Value in `rule_values` table ([#4166](https://github.com/EQEmu/Server/pull/4166)) @Kinglykrab 2024-03-07
* Fix EVENT_KILLED_MERIT firing before NPC removal ([#4185](https://github.com/EQEmu/Server/pull/4185)) @Kinglykrab 2024-03-17
* Fix Empty Groups When Removing Bots ([#4178](https://github.com/EQEmu/Server/pull/4178)) @Kinglykrab 2024-03-14
* Fix GetLeaderName() for Groups ([#4184](https://github.com/EQEmu/Server/pull/4184)) @Kinglykrab 2024-03-14
* Fix Mob::CalculateDistance(mob) Typo ([#4183](https://github.com/EQEmu/Server/pull/4183)) @Kinglykrab 2024-03-10
* Fix Proximity Say ([#4189](https://github.com/EQEmu/Server/pull/4189)) @Kinglykrab 2024-03-15
* Fix ScaleNPC() in Perl ([#4196](https://github.com/EQEmu/Server/pull/4196)) @Kinglykrab 2024-03-23
* Fix range_percent ([#4197](https://github.com/EQEmu/Server/pull/4197)) @Kinglykrab 2024-03-22
* Fix reusing timers ([#4199](https://github.com/EQEmu/Server/pull/4199)) @joligario 2024-03-23

### Hot Fix

* Add bool return to fix Client::RemoveAAPoints ([#4176](https://github.com/EQEmu/Server/pull/4176)) @Kinglykrab 2024-03-09

### Loot

* Fix issue with nested data being loaded multiple times ([#4192](https://github.com/EQEmu/Server/pull/4192)) @Akkadius 2024-03-23

### Misc

* Windows preprocessor define in crash.cpp ([#4191](https://github.com/EQEmu/Server/pull/4191)) @joligario 2024-03-23

### Quest API

* Add Buff Support to Perl/Lua ([#4182](https://github.com/EQEmu/Server/pull/4182)) @Kinglykrab 2024-03-14
* Add DeleteBot() to Perl/Lua ([#4167](https://github.com/EQEmu/Server/pull/4167)) @nytmyr 2024-03-07
* Add GetDeityName() to Perl/Lua ([#4180](https://github.com/EQEmu/Server/pull/4180)) @Kinglykrab 2024-03-14
* Add RemoveAAPoints() and AA Loss Event to Perl/Lua ([#4174](https://github.com/EQEmu/Server/pull/4174)) @Kinglykrab 2024-03-09
* Add RemoveAlternateCurrencyValue() to Perl/Lua ([#4190](https://github.com/EQEmu/Server/pull/4190)) @Kinglykrab 2024-03-17
* Add Restore Methods for Health, Mana, and Endurance to Perl/Lua ([#4179](https://github.com/EQEmu/Server/pull/4179)) @Kinglykrab 2024-03-23
* Add Silent Saylink Methods to Perl/Lua ([#4177](https://github.com/EQEmu/Server/pull/4177)) @Kinglykrab 2024-03-14

### Rules

* Add World:Rules Rule ([#4194](https://github.com/EQEmu/Server/pull/4194)) @Kinglykrab 2024-03-23

### Tradeskills

* Implement learning recipes from books ([#4170](https://github.com/EQEmu/Server/pull/4170)) @hgtw 2024-03-23

## [22.47.0] 3/5/2024

### Crash Fix

* Added a guild_mgr check ([#4163](https://github.com/EQEmu/Server/pull/4163)) @neckkola 2024-03-06
* Goto Command could crash using Developer Tools ([#4158](https://github.com/EQEmu/Server/pull/4158)) @neckkola 2024-03-04
* Groundspawn Memory Corruption ([#4157](https://github.com/EQEmu/Server/pull/4157)) @neckkola 2024-03-04
* Update to location of qGlobals initialization ([#4144](https://github.com/EQEmu/Server/pull/4144)) @neckkola 2024-03-02

### Feature

* Adds rules to control level requirements for Double Backstab, Assassinate, and Double Bowshot  (#4159) ([#29](https://github.com/EQEmu/Server/pull/29)) @catapultam-habeo 2024-03-04
* Adjust String-based Rules Length ([#4138](https://github.com/EQEmu/Server/pull/4138)) @Kinglykrab 2024-03-06
* Exempt a zone from IP-limit checks. ([#4137](https://github.com/EQEmu/Server/pull/4137)) @catapultam-habeo 2024-03-02

### Fixes

* Add id to the guild_bank table ([#4155](https://github.com/EQEmu/Server/pull/4155)) @neckkola 2024-03-05
* Fix Bots/Bot Pets ending up on XTargets ([#4132](https://github.com/EQEmu/Server/pull/4132)) @Kinglykrab 2024-03-02
* Fix Character EXP Modifiers default ([#4161](https://github.com/EQEmu/Server/pull/4161)) @Kinglykrab 2024-03-06
* Fix Spawns Not Parsing Quest on Zone Bootup ([#4149](https://github.com/EQEmu/Server/pull/4149)) @Kinglykrab 2024-03-05
* Fix typo when updating spawn events in spawn condition manager ([#4160](https://github.com/EQEmu/Server/pull/4160)) @joligario 2024-03-05
* GetBotNameByID Temporary Reference Warning ([#4145](https://github.com/EQEmu/Server/pull/4145)) @Kinglykrab 2024-03-02
* Prevent NPE when creating DZ using ad-hoc version IDs ([#4141](https://github.com/EQEmu/Server/pull/4141)) @catapultam-habeo 2024-03-05
* Update FreeGuildID Routine ([#4143](https://github.com/EQEmu/Server/pull/4143)) @neckkola 2024-03-02

### Quest API

* Add Bot Special Attacks for Immune Aggro/Damage ([#4108](https://github.com/EQEmu/Server/pull/4108)) @Kinglykrab 2024-03-02
* Add GetHeroicStrikethrough() to Perl/Lua ([#4150](https://github.com/EQEmu/Server/pull/4150)) @Kinglykrab 2024-03-03
* Add IsAlwaysAggro() to Perl/Lua ([#4152](https://github.com/EQEmu/Server/pull/4152)) @Kinglykrab 2024-03-04
* Add IsBoat()/IsControllableBoat() to Perl/Lua ([#4151](https://github.com/EQEmu/Server/pull/4151)) @Kinglykrab 2024-03-03
* Add IsDestructibleObject() to Perl/Lua ([#4153](https://github.com/EQEmu/Server/pull/4153)) @Kinglykrab 2024-03-03

### Zone

* Zone Routing Improvements ([#4142](https://github.com/EQEmu/Server/pull/4142)) @Akkadius 2024-03-02

### Zoning

* Zone routing adjustment ([#4162](https://github.com/EQEmu/Server/pull/4162)) @Akkadius 2024-03-06

## [22.46.1] 3/2/2024

### Fixes

* Change `UnburyCorpse` to use repository methods ([#4147](https://github.com/EQEmu/Server/pull/4147)) @joligario 2024-03-03

## [22.46.0] 3/2/2024

### Commands

* Add #fish Command ([#4136](https://github.com/EQEmu/Server/pull/4136)) @Kinglykrab 2024-03-01

### Crash Fix

* Raid::UpdateGroupAAs ([#4139](https://github.com/EQEmu/Server/pull/4139)) @neckkola 2024-03-02
* Update to location of qGlobals initialization ([#4144](https://github.com/EQEmu/Server/pull/4144)) @neckkola 2024-03-02

### Feature

* Exempt a zone from IP-limit checks. ([#4137](https://github.com/EQEmu/Server/pull/4137)) @catapultam-habeo 2024-03-02

### Fixes

* Cleanup NPC Mana Tap Logic ([#4134](https://github.com/EQEmu/Server/pull/4134)) @noudess 2024-03-02
* Fix Bots/Bot Pets ending up on XTargets ([#4132](https://github.com/EQEmu/Server/pull/4132)) @Kinglykrab 2024-03-02
* Fix issue with NPC Secondary Textures ([#4129](https://github.com/EQEmu/Server/pull/4129)) @Kinglykrab 2024-03-01
* GetBotNameByID Temporary Reference Warning ([#4145](https://github.com/EQEmu/Server/pull/4145)) @Kinglykrab 2024-03-02
* Update FreeGuildID Routine ([#4143](https://github.com/EQEmu/Server/pull/4143)) @neckkola 2024-03-02
* Use std::clamp for Mob::ChangeSize ([#4140](https://github.com/EQEmu/Server/pull/4140)) @joligario 2024-03-02

### Quest API

* Add Bot Special Attacks for Immune Aggro/Damage ([#4108](https://github.com/EQEmu/Server/pull/4108)) @Kinglykrab 2024-03-02

### Zone

* Zone Routing Improvements ([#4142](https://github.com/EQEmu/Server/pull/4142)) @Akkadius 2024-03-02

## [22.45.1] 2/29/2024

### Character Creation

* Improved Random Name Generator ([#4081](https://github.com/EQEmu/Server/pull/4081)) @catapultam-habeo 2024-02-27

### Code

* Fix Server Rules Documentation Generation ([#4125](https://github.com/EQEmu/Server/pull/4125)) @Kinglykrab 2024-02-26
* Remove unnecessary stoptimer logs ([#4128](https://github.com/EQEmu/Server/pull/4128)) @Kinglykrab 2024-02-28

### Commands

* Add `#forage` command ([#4133](https://github.com/EQEmu/Server/pull/4133)) @joligario 2024-02-29

### Crash

* Fix crash issue during database dump ([#4127](https://github.com/EQEmu/Server/pull/4127)) @Akkadius 2024-02-29

### Crash Fix

* D20 crash if mitigation average resulted in 0 ([#4131](https://github.com/EQEmu/Server/pull/4131)) @nytmyr 2024-02-29

### Fixes

* Fix forage returning first result from table ([#4130](https://github.com/EQEmu/Server/pull/4130)) @nytmyr 2024-02-29
* Who /all displays incorrect guild name ([#4123](https://github.com/EQEmu/Server/pull/4123)) @neckkola 2024-02-25

### Quest API

* Add Pet Owner Methods to Perl/Lua ([#4115](https://github.com/EQEmu/Server/pull/4115)) @Kinglykrab 2024-02-25

## [22.45.0] 2/24/2024

### Beacon

* Revert a few lines of PR #4024 that broke beacons ([#4118](https://github.com/EQEmu/Server/pull/4118)) @noudess 2024-02-24

### Bots

* Cleanup empty `bot_commands` files ([#4095](https://github.com/EQEmu/Server/pull/4095)) @Kinglykrab 2024-02-19
* Remove Alt Combat Functionality ([#4067](https://github.com/EQEmu/Server/pull/4067)) @Kinglykrab 2024-02-12

### Code

* Update fmt to 10.2.1 ([#4071](https://github.com/EQEmu/Server/pull/4071)) @hgtw 2024-02-16
* Use explicit conversions for enum formatting ([#4064](https://github.com/EQEmu/Server/pull/4064)) @hgtw 2024-02-12

### Database

* Add content_database flag for zone changes ([#4104](https://github.com/EQEmu/Server/pull/4104)) @joligario 2024-02-20
* Remove "item_tick" from database schema per https://github.com/EQEmu/Server/pull/3985 ([#4083](https://github.com/EQEmu/Server/pull/4083)) @Akkadius 2024-02-16

### Expansions

* Zone Expansion Version Routing ([#4084](https://github.com/EQEmu/Server/pull/4084)) @Akkadius 2024-02-20

### Factions

* Fix issue with npcedit and cached factions ([#4103](https://github.com/EQEmu/Server/pull/4103)) @noudess 2024-02-20

### Feature

* Add Augments to Starting Items for Bots/Players ([#4100](https://github.com/EQEmu/Server/pull/4100)) @Kinglykrab 2024-02-21
* Hot Zone Bonus XP Notice ([#4091](https://github.com/EQEmu/Server/pull/4091)) @fryguy503 2024-02-18

### Fixes

* Add safety check to SummonAllCharacterCorpses. ([#4107](https://github.com/EQEmu/Server/pull/4107)) @fryguy503 2024-02-21
* Added additional Guild-related crash checks ([#4105](https://github.com/EQEmu/Server/pull/4105)) @neckkola 2024-02-22
* Aura effect spell id from #3964 ([#4094](https://github.com/EQEmu/Server/pull/4094)) @joligario 2024-02-18
* Crash on Ubuntu 22.04 ([#4119](https://github.com/EQEmu/Server/pull/4119)) @neckkola 2024-02-25
* Fix #wpadd Query ([#4078](https://github.com/EQEmu/Server/pull/4078)) @Kinglykrab 2024-02-14
* Fix Bot Weapons with No Races ([#4110](https://github.com/EQEmu/Server/pull/4110)) @Kinglykrab 2024-02-25
* Fix ClearSpawnTimers() ([#4073](https://github.com/EQEmu/Server/pull/4073)) @Kinglykrab 2024-02-13
* Fix Issue with ClearSpawnTimers() ([#4070](https://github.com/EQEmu/Server/pull/4070)) @Kinglykrab 2024-02-13
* Fix ObjectContentsRepository using content_db ([#4076](https://github.com/EQEmu/Server/pull/4076)) @Kinglykrab 2024-02-14
* Fix Sympathetic Procs for Bots/Mercenariess ([#4086](https://github.com/EQEmu/Server/pull/4086)) @Kinglykrab 2024-02-19
* Fix fix_z Query ([#4080](https://github.com/EQEmu/Server/pull/4080)) @Kinglykrab 2024-02-15
* Fix hotzone string error ([#4109](https://github.com/EQEmu/Server/pull/4109)) @fryguy503 2024-02-22
* Fix issue with IsEntOnHateList ([#4097](https://github.com/EQEmu/Server/pull/4097)) @Kinglykrab 2024-02-19
* Fix manifest check on `base_data` change ([#4082](https://github.com/EQEmu/Server/pull/4082)) @joligario 2024-02-16
* Revert 2df7d19 ([#4101](https://github.com/EQEmu/Server/pull/4101)) @joligario 2024-02-20
* Update database_schema to add guild_permissions ([#4085](https://github.com/EQEmu/Server/pull/4085)) @neckkola 2024-02-16

### Guilds

* Fixes for Guild Bank ([#4120](https://github.com/EQEmu/Server/pull/4120)) @neckkola 2024-02-25

### Logs

* Fix log in BaseGuildManager::IsGuildLeader() ([#4102](https://github.com/EQEmu/Server/pull/4102)) @joligario 2024-02-20

### Opcodes

* Fix opcode reloading ([#4075](https://github.com/EQEmu/Server/pull/4075)) @Akkadius 2024-02-14

### Quest API

* Add Bot Methods to Perl/Lua ([#4113](https://github.com/EQEmu/Server/pull/4113)) @Kinglykrab 2024-02-25
* Add Entity Variable Events to Perl/Lua ([#4092](https://github.com/EQEmu/Server/pull/4092)) @Kinglykrab 2024-02-20
* Add GetAugmentIDs() to Perl/Lua ([#4114](https://github.com/EQEmu/Server/pull/4114)) @Kinglykrab 2024-02-25
* Add GetNPCSpellsEffectsID() to Perl/Lua ([#4117](https://github.com/EQEmu/Server/pull/4117)) @Kinglykrab 2024-02-25
* Add Hatelist Count Methods to Perl/Lua ([#4106](https://github.com/EQEmu/Server/pull/4106)) @Kinglykrab 2024-02-21
* Add IsInAGuild() to Perl/Lua ([#4066](https://github.com/EQEmu/Server/pull/4066)) @Kinglykrab 2024-02-14
* Add Pet Methods to Perl/Lua ([#4116](https://github.com/EQEmu/Server/pull/4116)) @Kinglykrab 2024-02-25
* Add Timer Events to Perl/Lua ([#4099](https://github.com/EQEmu/Server/pull/4099)) @Kinglykrab 2024-02-19
* Add spell cast methods to Lua ([#4096](https://github.com/EQEmu/Server/pull/4096)) @Kinglykrab 2024-02-19
* Add tracebacks to Lua error messages ([#4079](https://github.com/EQEmu/Server/pull/4079)) @hgtw 2024-02-16
* Export Combat Record to Death Events ([#4112](https://github.com/EQEmu/Server/pull/4112)) @Kinglykrab 2024-02-25

### Repositories

* Cleanup and Convert Character Creation to Repositories ([#4053](https://github.com/EQEmu/Server/pull/4053)) @Kinglykrab 2024-02-20
* Convert Zone Flags to Repositories ([#4077](https://github.com/EQEmu/Server/pull/4077)) @Kinglykrab 2024-02-16
* Update `zone` base repository from #4084 ([#4111](https://github.com/EQEmu/Server/pull/4111)) @joligario 2024-02-23

## [22.44.5] - 2/12/2024

### Fixes

* [Hotfix] Fix issues where we're using the wrong database pointers @Akkadius

## [22.44.4] - 2/12/2024

### Fixes

* Fix ClearSpawnTimers() ([#4073](https://github.com/EQEmu/Server/pull/4073)) @Kinglykrab 2024-02-13

## [22.44.3] - 2/12/2024

### Fixes

* Fix Issue with ClearSpawnTimers() ([#4070](https://github.com/EQEmu/Server/pull/4070)) @Kinglykrab 2024-02-13

## [22.44.2] - 2/12/2024

### Bots

* Remove Alt Combat Functionality ([#4067](https://github.com/EQEmu/Server/pull/4067)) @Kinglykrab 2024-02-12

### Code

* Use explicit conversions for enum formatting ([#4064](https://github.com/EQEmu/Server/pull/4064)) @hgtw 2024-02-12

### Database

* Fix default value for `time_of_death` in `character_corpses` ([#4060](https://github.com/EQEmu/Server/pull/4060)) @joligario 2024-02-11
* Fix table name in manifest ([#4063](https://github.com/EQEmu/Server/pull/4063)) @joligario 2024-02-11
* Proper default for `droptime` from `object_contents` ([#4061](https://github.com/EQEmu/Server/pull/4061)) @joligario 2024-02-11

### Fixes

* Guard against crash condition ([#4062](https://github.com/EQEmu/Server/pull/4062)) @neckkola 2024-02-11

### Guilds

* Clean up `GUILD_RANK_NONE` references ([#4059](https://github.com/EQEmu/Server/pull/4059)) @joligario 2024-02-11

## [22.44.1] - 2/11/2024

### Fixes

* Fix File::Exists(file_name) to be resilient to Chinese characters ([#4058](https://github.com/EQEmu/Server/pull/4058)) @Akkadius 2024-02-11

## [22.44.0] - 2/10/2024

### Base Data

* Remove from shared memory and simplify ([#4045](https://github.com/EQEmu/Server/pull/4045)) @Kinglykrab 2024-02-05

### Bot Commands

* Separate Bot Commands into Individual Files ([#4035](https://github.com/EQEmu/Server/pull/4035)) @Kinglykrab 2024-02-01

### Code

* Convert Quest Ornament Methods to Repositories ([#4048](https://github.com/EQEmu/Server/pull/4048)) @Kinglykrab 2024-02-10
* Remove redundant check in Object::HandleCombine ([#4041](https://github.com/EQEmu/Server/pull/4041)) @joligario 2024-02-03

### Commands

* Add #show special_abilities ([#4043](https://github.com/EQEmu/Server/pull/4043)) @Kinglykrab 2024-02-05

### Crash Reporting

* Update Spire to use SSL ([#4055](https://github.com/EQEmu/Server/pull/4055)) @Akkadius 2024-02-10

### Database

* Correct manifest migration entries that should be content ([#4047](https://github.com/EQEmu/Server/pull/4047)) @Akkadius 2024-02-05

### Deprecation

* Remove eqemu_server.pl script and ties to server ([#4049](https://github.com/EQEmu/Server/pull/4049)) @Akkadius 2024-02-10

### Feature

* Add RoF2 Guild features ([#3699](https://github.com/EQEmu/Server/pull/3699)) @neckkola 2024-02-10
* Add optional `is_forced` parameter to Zone::Repop ([#4046](https://github.com/EQEmu/Server/pull/4046)) @Kinglykrab 2024-02-05
* Add support to crosszone/worldwide spells to affect bot pets ([#4036](https://github.com/EQEmu/Server/pull/4036)) @Kinglykrab 2024-02-01
* Classic Taunt (Pre 2006) style of taunt ([#3942](https://github.com/EQEmu/Server/pull/3942)) @fryguy503 2024-02-10
* Corpse Overhaul ([#3938](https://github.com/EQEmu/Server/pull/3938)) @fryguy503 2024-02-08

### Fixes

* Fix Bot Non-Melee Damage Messages ([#4038](https://github.com/EQEmu/Server/pull/4038)) @Kinglykrab 2024-02-01
* Fix Froglok Starting Languages ([#4050](https://github.com/EQEmu/Server/pull/4050)) @Kinglykrab 2024-02-09
* Fix Issue with Bot Dual wield ([#4037](https://github.com/EQEmu/Server/pull/4037)) @Kinglykrab 2024-02-01
* Fix for potential crash of DS damage on death ([#4040](https://github.com/EQEmu/Server/pull/4040)) @nytmyr 2024-02-03

### GM Commands

* Fix typo in #rules help. ([#4042](https://github.com/EQEmu/Server/pull/4042)) @noudess 2024-02-04

### Loot

* Remove from shared memory, simplification ([#3988](https://github.com/EQEmu/Server/pull/3988)) @Akkadius 2024-02-05

### Objects

* Add `fix_z` column to ground spawns ([#3992](https://github.com/EQEmu/Server/pull/3992)) @Kinglykrab 2024-02-01

### Quest API

* Add GetAAEXPPercentage() and GetEXPPercentage() to Perl/Lua ([#4044](https://github.com/EQEmu/Server/pull/4044)) @Kinglykrab 2024-02-04

### Quests

* Cleanup Quest Parser Logic ([#4025](https://github.com/EQEmu/Server/pull/4025)) @Kinglykrab 2024-02-06

### Tasks

* Avoid removing client tasks from memory on load ([#4052](https://github.com/EQEmu/Server/pull/4052)) @hgtw 2024-02-10
* Send active elements in task select packet ([#4051](https://github.com/EQEmu/Server/pull/4051)) @hgtw 2024-02-10

## [22.43.3] - 1/30/2024

### API

* Add lock_status method to world API ([#4028](https://github.com/EQEmu/Server/pull/4028)) @Akkadius 2024-01-29

### Alternate Currency

* Convert Load of Alternate Currencies to Repositories ([#3993](https://github.com/EQEmu/Server/pull/3993)) @Kinglykrab 2024-01-29

### Beacon

* Cleanup Beacons Code ([#4024](https://github.com/EQEmu/Server/pull/4024)) @Kinglykrab 2024-01-29

### Bots

* Convert Bot Database Methods to Repositories ([#4023](https://github.com/EQEmu/Server/pull/4023)) @Kinglykrab 2024-01-29
* Missing boolean returns on DeletePetStats and SaveInspectMessage ([#4031](https://github.com/EQEmu/Server/pull/4031)) @joligario 2024-01-30

### Code

* Add Raid Loot Type Constants ([#4022](https://github.com/EQEmu/Server/pull/4022)) @Kinglykrab 2024-01-29
* Cleanup cross-zone/world-wide OPCode handling ([#4003](https://github.com/EQEmu/Server/pull/4003)) @Kinglykrab 2024-01-29

### Commands

* Cleanup #petitems Command ([#4005](https://github.com/EQEmu/Server/pull/4005)) @Kinglykrab 2024-01-29

### Crash Fix

* Fix Possible Crash in HateList::Find ([#4027](https://github.com/EQEmu/Server/pull/4027)) @Kinglykrab 2024-01-28

### Fixes

* Fix NPC After Death Emotes ([#4021](https://github.com/EQEmu/Server/pull/4021)) @Kinglykrab 2024-01-29
* Fix database loading routines ([#4030](https://github.com/EQEmu/Server/pull/4030)) @Akkadius 2024-01-29
* Fix issue in character_corpses Repository Query ([#4029](https://github.com/EQEmu/Server/pull/4029)) @Kinglykrab 2024-01-29
* Limit Pet Taunt Distance ([#4018](https://github.com/EQEmu/Server/pull/4018)) @fryguy503 2024-01-26
* Limit Player Taunt Distance ([#4019](https://github.com/EQEmu/Server/pull/4019)) @fryguy503 2024-01-26
* Patch for recent change to DEATH ([#4032](https://github.com/EQEmu/Server/pull/4032)) @noudess 2024-01-30
* spawn_conditions map was being emptied by mistake ([#4033](https://github.com/EQEmu/Server/pull/4033)) @noudess 2024-01-30

### Grids

* Convert Grid/Grid Entries to Repositories ([#4011](https://github.com/EQEmu/Server/pull/4011)) @Kinglykrab 2024-01-29

### Mercenaries

* Add expansion settings update to Mercs:Enable ([#4001](https://github.com/EQEmu/Server/pull/4001)) @Kinglykrab 2024-01-29

### Merchants

* Convert Merchant Load to Repositories ([#4007](https://github.com/EQEmu/Server/pull/4007)) @Kinglykrab 2024-01-29

### Pets

* Convert Load of Pets Beastlord Data to Repositories ([#3995](https://github.com/EQEmu/Server/pull/3995)) @Kinglykrab 2024-01-29

### Quests

* Use separate variable for quest idle override. ([#4026](https://github.com/EQEmu/Server/pull/4026)) @noudess 2024-01-27

### Rules

* Add rules for cross-zone/world-wide casts to affect Bots/Mercenaries/Pets ([#4002](https://github.com/EQEmu/Server/pull/4002)) @Kinglykrab 2024-01-29

### Spawn2

* Convert Spawn2 Methods to Repositories ([#4014](https://github.com/EQEmu/Server/pull/4014)) @Kinglykrab 2024-01-29

### Traps

* Convert Load/Set of Traps to Repositories ([#3994](https://github.com/EQEmu/Server/pull/3994)) @Kinglykrab 2024-01-29

### Zone

* Convert Fishing/Foraging to Repositories ([#4008](https://github.com/EQEmu/Server/pull/4008)) @Kinglykrab 2024-01-30

## [22.43.2] - 1/25/2024

### Bots

* ^mez command spell list fix ([#3998](https://github.com/EQEmu/Server/pull/3998)) @dariusuknuis 2024-01-19

### Code

* Cleanup position methods ([#4015](https://github.com/EQEmu/Server/pull/4015)) @Kinglykrab 2024-01-25

### Commands

* Add `#npcedit set_grid [Grid ID]` to `#npcedit` ([#4004](https://github.com/EQEmu/Server/pull/4004)) @Kinglykrab 2024-01-22

### Crash Fix

* Fix crash when creating Frogloks/Drakkin ([#4016](https://github.com/EQEmu/Server/pull/4016)) @Kinglykrab 2024-01-25
* Reverting PR #3877 ([#3997](https://github.com/EQEmu/Server/pull/3997)) @fryguy503 2024-01-17

### Database

* Drop deprecated item_tick table ([#3977](https://github.com/EQEmu/Server/pull/3977)) @Akkadius 2024-01-14
* Drop item_tick if exists tweak in manifest ([#3985](https://github.com/EQEmu/Server/pull/3985)) @Akkadius 2024-01-15
* Increase max spawngroup name from 50 to 200 ([#3991](https://github.com/EQEmu/Server/pull/3991)) @Akkadius 2024-01-22

### Fixes

* Fix Mercenaries Buffs/Zoning Issues ([#4000](https://github.com/EQEmu/Server/pull/4000)) @Kinglykrab 2024-01-22
* Fix zone database update manifest ([#3972](https://github.com/EQEmu/Server/pull/3972)) @Kinglykrab 2024-01-13
* Fixes to zone idle while empty changes. ([#4006](https://github.com/EQEmu/Server/pull/4006)) @noudess 2024-01-23
* Reversed logic on InLiquid ([#3979](https://github.com/EQEmu/Server/pull/3979)) @fryguy503 2024-01-14

### Instances

* Convert Instance Quest Methods to Repositories ([#4012](https://github.com/EQEmu/Server/pull/4012)) @Kinglykrab 2024-01-25

### Logging

* Force Info category to be always on in file/console logs ([#3990](https://github.com/EQEmu/Server/pull/3990)) @Akkadius 2024-01-22

### Merchants

* Change database structure for merchant slots ([#3974](https://github.com/EQEmu/Server/pull/3974)) @joligario 2024-01-14

### Messages

* Fix disciple message added by previous patch. ([#3986](https://github.com/EQEmu/Server/pull/3986)) @noudess 2024-01-15

### Quest API

* QuestReward should now summon item to the inventory instead of the cursor. ([#3996](https://github.com/EQEmu/Server/pull/3996)) @regneq 2024-01-22

### Repositories

* Add `rank` to reserved words ([#3982](https://github.com/EQEmu/Server/pull/3982)) @Akkadius 2024-01-15
* Fix datetime zero-value save behavior ([#3976](https://github.com/EQEmu/Server/pull/3976)) @Akkadius 2024-01-14

### Spawn2

* Spawn condition value should default spawn_conditions value ([#3980](https://github.com/EQEmu/Server/pull/3980)) @noudess 2024-01-14

### Zoning

* Additional logs for zoning under instance checks ([#3989](https://github.com/EQEmu/Server/pull/3989)) @Akkadius 2024-01-22

## [22.43.1] - 1/14/2024

### Repositories

*  Revert #3976 - fix character corpses @Akkadius 2024-01-14

## [22.43.0] - 1/14/2024

### Database

* Drop deprecated item_tick table ([#3977](https://github.com/EQEmu/Server/pull/3977)) @Akkadius 2024-01-14

### Merchants

* Change database structure for merchant slots ([#3974](https://github.com/EQEmu/Server/pull/3974)) @joligario 2024-01-14

### Repositories

* Fix datetime zero-value save behavior ([#3976](https://github.com/EQEmu/Server/pull/3976)) @Akkadius 2024-01-14

### Zones

* Convert IDLE_WHEN_EMPTY to a Zone Column ([#3891](https://github.com/EQEmu/Server/pull/3891)) @Kinglykrab 2024-01-13

## [22.42.1] - 1/13/2024

### Fixes

* Fix zone database update manifest ([#3972](https://github.com/EQEmu/Server/pull/3972)) @Kinglykrab 2024-01-13
* quest::processmobswhilezoneempty() fix. ([#3971](https://github.com/EQEmu/Server/pull/3971)) @noudess 2024-01-13

## [22.42.0] - 1/13/2024

### AAs

* Convert Loading of AAs to Repositories ([#3963](https://github.com/EQEmu/Server/pull/3963)) @Kinglykrab 2024-01-13

### Auras

* Convert Get of Auras to Repositories ([#3964](https://github.com/EQEmu/Server/pull/3964)) @Kinglykrab 2024-01-13

### Character

* Character EXP Modifiers in Memory ([#3934](https://github.com/EQEmu/Server/pull/3934)) @Kinglykrab 2024-01-13
* Convert Character Corpses to Repositories ([#3941](https://github.com/EQEmu/Server/pull/3941)) @Kinglykrab 2024-01-13
* Convert Save of Character Languages to Repositories ([#3948](https://github.com/EQEmu/Server/pull/3948)) @Kinglykrab 2024-01-12

### Code

* Cleanup AddCash() and RemoveCash() NPC Methods ([#3957](https://github.com/EQEmu/Server/pull/3957)) @Kinglykrab 2024-01-12
* Cleanup ChangeSize() ([#3959](https://github.com/EQEmu/Server/pull/3959)) @Kinglykrab 2024-01-12
* Cleanup Owner Related Mob Methods ([#3960](https://github.com/EQEmu/Server/pull/3960)) @Kinglykrab 2024-01-13
* Cleanup RandomizeFeatures() ([#3958](https://github.com/EQEmu/Server/pull/3958)) @Kinglykrab 2024-01-12
* Remove GroupCount() and RaidGroupCount() ([#3943](https://github.com/EQEmu/Server/pull/3943)) @Kinglykrab 2024-01-13
* Remove unused Grid methods ([#3944](https://github.com/EQEmu/Server/pull/3944)) @Kinglykrab 2024-01-13

### Commands

* Convert NPC Spawn Methods to Repositories ([#3956](https://github.com/EQEmu/Server/pull/3956)) @Kinglykrab 2024-01-13

### Crash Fix

* Character Creation Class/Race out of Range. ([#3920](https://github.com/EQEmu/Server/pull/3920)) @fryguy503 2024-01-09

### Feat

* Alt Sinister Strike Formula ([#3921](https://github.com/EQEmu/Server/pull/3921)) @fryguy503 2024-01-09

### Feature

* Add Lore Item Trade Error ([#3932](https://github.com/EQEmu/Server/pull/3932)) @fryguy503 2024-01-13
* Rule: UseLiveBlockedMessage Bard Spam ([#3933](https://github.com/EQEmu/Server/pull/3933)) @fryguy503 2024-01-09

### Fixes

* Attack Corpse Skill Up Fix ([#3924](https://github.com/EQEmu/Server/pull/3924)) @fryguy503 2024-01-09
* Fix Issue with SetFlyMode() ([#3961](https://github.com/EQEmu/Server/pull/3961)) @Kinglykrab 2024-01-12
* Harm Touch Critical Ratio ([#3915](https://github.com/EQEmu/Server/pull/3915)) @fryguy503 2024-01-09
* Increase Precision in CheckDoubleAttack ([#3928](https://github.com/EQEmu/Server/pull/3928)) @fryguy503 2024-01-09
* IsOfClientBotMerc() for Discipline Messages ([#3940](https://github.com/EQEmu/Server/pull/3940)) @fryguy503 2024-01-09
* Limit MeleeMitigationEffect to defender if only client. ([#3918](https://github.com/EQEmu/Server/pull/3918)) @fryguy503 2024-01-09
* Rampage Number of Hits Limit ([#3929](https://github.com/EQEmu/Server/pull/3929)) @fryguy503 2024-01-09
* Remove duplicate character_data repository in zonedb.cpp ([#3935](https://github.com/EQEmu/Server/pull/3935)) @Kinglykrab 2024-01-09
* Undead Should Never Flee ([#3926](https://github.com/EQEmu/Server/pull/3926)) @fryguy503 2024-01-09

### Global Loot

* Convert Global Loot to Repositories ([#3962](https://github.com/EQEmu/Server/pull/3962)) @Kinglykrab 2024-01-13

### Ground Spawns

* Convert Ground Spawns to Repositories ([#3967](https://github.com/EQEmu/Server/pull/3967)) @Kinglykrab 2024-01-13

### Horses

* Convert Horses to Repositories ([#3965](https://github.com/EQEmu/Server/pull/3965)) @Kinglykrab 2024-01-13

### Item Ticks

* Remove Item Ticks Code ([#3955](https://github.com/EQEmu/Server/pull/3955)) @Kinglykrab 2024-01-13

### Mercenaries

* Convert Mercenaries to Repositories ([#3947](https://github.com/EQEmu/Server/pull/3947)) @Kinglykrab 2024-01-13

### Merchants

* Convert Clear/Delete/Save of Temporary Merchant Lists to Repositories ([#3945](https://github.com/EQEmu/Server/pull/3945)) @Kinglykrab 2024-01-13

### NPCs

* Convert Load of NPC Emotes to Repositories ([#3954](https://github.com/EQEmu/Server/pull/3954)) @Kinglykrab 2024-01-13

### Objects

* Convert Add/Delete/Update of Objects to Repositories ([#3966](https://github.com/EQEmu/Server/pull/3966)) @Kinglykrab 2024-01-13
* Convert World Containers to Repositories ([#3951](https://github.com/EQEmu/Server/pull/3951)) @Kinglykrab 2024-01-13

### Pets

* Convert Pets to Repositories ([#3968](https://github.com/EQEmu/Server/pull/3968)) @Kinglykrab 2024-01-13

### Repositories

* Add null integer column support, instance_list notes migration, regenerate repositories ([#3969](https://github.com/EQEmu/Server/pull/3969)) @Akkadius 2024-01-13

### Respawns

* Convert Respawn Times to Repositories ([#3949](https://github.com/EQEmu/Server/pull/3949)) @Kinglykrab 2024-01-13

### Rules

* Add rule for bard aggro cap ([#3909](https://github.com/EQEmu/Server/pull/3909)) @fryguy503 2024-01-09
* Allow GMs to silently summon ([#3910](https://github.com/EQEmu/Server/pull/3910)) @fryguy503 2024-01-09
* Bash Two Hander use Shoulders ([#3925](https://github.com/EQEmu/Server/pull/3925)) @fryguy503 2024-01-09
* Classic Spell Data SPA Calc variability ([#3931](https://github.com/EQEmu/Server/pull/3931)) @fryguy503 2024-01-09
* Classic Tracking Skillups ([#3923](https://github.com/EQEmu/Server/pull/3923)) @fryguy503 2024-01-09
* Separate Tradeskill Max Train from Research. ([#3916](https://github.com/EQEmu/Server/pull/3916)) @fryguy503 2024-01-09

### Tasks

* Add Support for Task Window Element Groups ([#3902](https://github.com/EQEmu/Server/pull/3902)) @hgtw 2024-01-09

### Traps

* Convert Load of LDoN Traps/Trap Entries to Repositories ([#3953](https://github.com/EQEmu/Server/pull/3953)) @Kinglykrab 2024-01-13

### Zones

* Convert Get/Set of Zone Timezone to Repositories ([#3946](https://github.com/EQEmu/Server/pull/3946)) @Kinglykrab 2024-01-13
* Convert IDLE_WHEN_EMPTY to a Zone Column ([#3891](https://github.com/EQEmu/Server/pull/3891)) @Kinglykrab 2024-01-13
* Convert SaveZoneCFG to Repositories ([#3950](https://github.com/EQEmu/Server/pull/3950)) @Kinglykrab 2024-01-13

## [22.41.0] - 1/8/2024

### Bug

* DI Buff Fade ([#3919](https://github.com/EQEmu/Server/pull/3919)) @fryguy503 2024-01-08
* NPCs will now only proc on hit ([#3913](https://github.com/EQEmu/Server/pull/3913)) @fryguy503 2024-01-08
* Pets should not ignore Z axis ([#3912](https://github.com/EQEmu/Server/pull/3912)) @fryguy503 2024-01-08

### Fixes

* Disciplines should show when someone casts them. ([#3901](https://github.com/EQEmu/Server/pull/3901)) @fryguy503 2024-01-08
* Fix Typo in Character Skills loading ([#3937](https://github.com/EQEmu/Server/pull/3937)) @Kinglykrab 2024-01-09
* Fix for HasLockoutByCharacterID ([#3927](https://github.com/EQEmu/Server/pull/3927)) @fryguy503 2024-01-08
* Harm Touch, Improved Harm Touch, and Unholy Touch ([#3904](https://github.com/EQEmu/Server/pull/3904)) @fryguy503 2024-01-08
* Legacy Manaburn should have hard cap. ([#3905](https://github.com/EQEmu/Server/pull/3905)) @fryguy503 2024-01-08
* TGB - Added logic to stop bard errors on group songs. ([#3906](https://github.com/EQEmu/Server/pull/3906)) @fryguy503 2024-01-08
* World Shutdown Filter ([#3930](https://github.com/EQEmu/Server/pull/3930)) @fryguy503 2024-01-08

### Info

* Adding textual feedback when trying to sell alt items back to … ([#3917](https://github.com/EQEmu/Server/pull/3917)) @fryguy503 2024-01-08

### Rules

* Backstab Damage Modifier ([#3908](https://github.com/EQEmu/Server/pull/3908)) @fryguy503 2024-01-08
* Classic Tradeskill Skill Clamp ([#3914](https://github.com/EQEmu/Server/pull/3914)) @fryguy503 2024-01-08
* Classic Triple Attack ([#3903](https://github.com/EQEmu/Server/pull/3903)) @fryguy503 2024-01-08
* Ensure mana taps only effect NPC's that have mana. ([#3907](https://github.com/EQEmu/Server/pull/3907)) @fryguy503 2024-01-08
* Over Taunt Hate ([#3900](https://github.com/EQEmu/Server/pull/3900)) @fryguy503 2024-01-08
* Stun Chance Percent Rule ([#3922](https://github.com/EQEmu/Server/pull/3922)) @fryguy503 2024-01-08

## [22.40.0] - 1/7/2024

### Account

* Convert Get/Update Account Karma to Repositories ([#3858](https://github.com/EQEmu/Server/pull/3858)) @Kinglykrab 2024-01-07
* Convert UpdateGMStatus to Repositories ([#3859](https://github.com/EQEmu/Server/pull/3859)) @Kinglykrab 2024-01-07

### Bots

* Resist Spell Fix ([#3840](https://github.com/EQEmu/Server/pull/3840)) @dariusuknuis 2024-01-07

### Bugfix

* Negative Aggro Fix ([#3866](https://github.com/EQEmu/Server/pull/3866)) @fryguy503 2024-01-07

### Character

* Convert Clear/Delete/Get/Update of Character Item Recast to Repositories ([#3857](https://github.com/EQEmu/Server/pull/3857)) @Kinglykrab 2024-01-07
* Convert Delete/Load/Remove/Save of Character AA to Repositories ([#3849](https://github.com/EQEmu/Server/pull/3849)) @Kinglykrab 2024-01-07
* Convert Delete/Load/Save of Character Bandolier to Repositories ([#3845](https://github.com/EQEmu/Server/pull/3845)) @Kinglykrab 2024-01-07
* Convert Delete/Load/Save of Character Disciplines to Repositories ([#3850](https://github.com/EQEmu/Server/pull/3850)) @Kinglykrab 2024-01-07
* Convert Delete/Load/Save of Character Leadership Abilities to Repositories ([#3847](https://github.com/EQEmu/Server/pull/3847)) @Kinglykrab 2024-01-07
* Convert Delete/Load/Save of Character Material to Repositories ([#3846](https://github.com/EQEmu/Server/pull/3846)) @Kinglykrab 2024-01-07
* Convert Delete/Load/Save of Character Spells to Repositories ([#3842](https://github.com/EQEmu/Server/pull/3842)) @Kinglykrab 2024-01-07
* Convert Delete/Save of Character Memmed Spells to Repositories ([#3841](https://github.com/EQEmu/Server/pull/3841)) @Kinglykrab 2024-01-07
* Convert Load/Save of Character Bind to Repositories ([#3851](https://github.com/EQEmu/Server/pull/3851)) @Kinglykrab 2024-01-07
* Convert Load/Save of Character Buffs to Repositories ([#3855](https://github.com/EQEmu/Server/pull/3855)) @Kinglykrab 2024-01-07
* Convert Load/Save of Character Currency to Repositories ([#3848](https://github.com/EQEmu/Server/pull/3848)) @Kinglykrab 2024-01-07
* Convert Load/Save of Character Data to Repositories ([#3839](https://github.com/EQEmu/Server/pull/3839)) @Kinglykrab 2024-01-07
* Convert Load/Save of Character Potion Belt to Repositories ([#3844](https://github.com/EQEmu/Server/pull/3844)) @Kinglykrab 2024-01-07
* Convert Load/Save of Character Skills to Repositories ([#3843](https://github.com/EQEmu/Server/pull/3843)) @Kinglykrab 2024-01-07
* Convert Load/Update of Character Alternate Currencies to Repositories ([#3856](https://github.com/EQEmu/Server/pull/3856)) @Kinglykrab 2024-01-07
* Convert NoRentExpired to Repositories ([#3860](https://github.com/EQEmu/Server/pull/3860)) @Kinglykrab 2024-01-07

### Characters

* Convert Load/Save of Character Auras to Repositories ([#3854](https://github.com/EQEmu/Server/pull/3854)) @Kinglykrab 2024-01-07

### Code

* Remove bot-based saylink method ([#3852](https://github.com/EQEmu/Server/pull/3852)) @Kinglykrab 2024-01-07

### Commands

* Add #clearxtargets Command ([#3833](https://github.com/EQEmu/Server/pull/3833)) @Kinglykrab 2024-01-07
* Add scoped buckets and editing to #databuckets ([#3826](https://github.com/EQEmu/Server/pull/3826)) @Kinglykrab 2024-01-07
* Cleanup #appearance Command ([#3827](https://github.com/EQEmu/Server/pull/3827)) @Kinglykrab 2024-01-07
* Cleanup #fixmob  Command ([#3828](https://github.com/EQEmu/Server/pull/3828)) @Kinglykrab 2024-01-07
* Cleanup #petname Command ([#3829](https://github.com/EQEmu/Server/pull/3829)) @Kinglykrab 2024-01-07
* Cleanup #shutdown Command ([#3830](https://github.com/EQEmu/Server/pull/3830)) @Kinglykrab 2024-01-07
* Remove #zopp Command ([#3831](https://github.com/EQEmu/Server/pull/3831)) @Kinglykrab 2024-01-07

### Crash

* Fix crash where Raid invite could be accepted after forming group with the Raid invitor. ([#3837](https://github.com/EQEmu/Server/pull/3837)) @Aeadoin 2024-01-06

### Feature

* Break Trader if moved ([#3862](https://github.com/EQEmu/Server/pull/3862)) @fryguy503 2024-01-07
* Formula Addition (40+ Harm Touch) ([#3870](https://github.com/EQEmu/Server/pull/3870)) @fryguy503 2024-01-07
* Legacy Fizzle Code ([#3868](https://github.com/EQEmu/Server/pull/3868)) @fryguy503 2024-01-07
* Legacy Manaburn Rule ([#3872](https://github.com/EQEmu/Server/pull/3872)) @fryguy503 2024-01-07

### Fixes

* Add locations where melee can be bound outside of a city. ([#3887](https://github.com/EQEmu/Server/pull/3887)) @fryguy503 2024-01-07
* Amplification should not benefit from instrument mods ([#3898](https://github.com/EQEmu/Server/pull/3898)) @fryguy503 2024-01-07
* Bard Caster Level Fixes ([#3883](https://github.com/EQEmu/Server/pull/3883)) @fryguy503 2024-01-07
* Buff Sync ([#3896](https://github.com/EQEmu/Server/pull/3896)) @fryguy503 2024-01-07
* Cancel Magic SE fix ([#3890](https://github.com/EQEmu/Server/pull/3890)) @fryguy503 2024-01-07
* Class Trainers dont steal your money! ([#3864](https://github.com/EQEmu/Server/pull/3864)) @fryguy503 2024-01-07
* Clear Ramp when Clearing hate ([#3892](https://github.com/EQEmu/Server/pull/3892)) @fryguy503 2024-01-07
* DI/Death Pact Fix ([#3867](https://github.com/EQEmu/Server/pull/3867)) @fryguy503 2024-01-07
* Depop Charm Pet and Detach Debuffs on Evacuate ([#3888](https://github.com/EQEmu/Server/pull/3888)) @fryguy503 2024-01-07
* Dire Charm Reset ([#3875](https://github.com/EQEmu/Server/pull/3875)) @fryguy503 2024-01-07
* Disciplines Getting Focuses Fix ([#3884](https://github.com/EQEmu/Server/pull/3884)) @fryguy503 2024-01-07
* Fix issue with 9th/10th inventory slot ([#3835](https://github.com/EQEmu/Server/pull/3835)) @Kinglykrab 2024-01-03
* Fix typo on ZoneDatabase::LoadPetInfo ([#3871](https://github.com/EQEmu/Server/pull/3871)) @Kinglykrab 2024-01-07
* Harm Touch Unholy Disc Type ([#3874](https://github.com/EQEmu/Server/pull/3874)) @fryguy503 2024-01-07
* Prevent QS Crashes ([#3877](https://github.com/EQEmu/Server/pull/3877)) @fryguy503 2024-01-07
* Rez Effects Stacking ([#3882](https://github.com/EQEmu/Server/pull/3882)) @fryguy503 2024-01-07
* Rez in zone clear aggro ([#3895](https://github.com/EQEmu/Server/pull/3895)) @fryguy503 2024-01-07
* Rune Invis Break ([#3893](https://github.com/EQEmu/Server/pull/3893)) @fryguy503 2024-01-07
* Snare and DOT Stacking ([#3897](https://github.com/EQEmu/Server/pull/3897)) @fryguy503 2024-01-07
* Swim Skillup and Underwater Fall Damage Fix ([#3885](https://github.com/EQEmu/Server/pull/3885)) @fryguy503 2024-01-07
* Swimming Rules Adjustment and Racial ([#3889](https://github.com/EQEmu/Server/pull/3889)) @fryguy503 2024-01-07
* Target Locked Pet Taunt ([#3894](https://github.com/EQEmu/Server/pull/3894)) @fryguy503 2024-01-07
* Vampiric Embrace Fixes ([#3873](https://github.com/EQEmu/Server/pull/3873)) @fryguy503 2024-01-07

### Languages

* Cleanup language constants, use repositories ([#3838](https://github.com/EQEmu/Server/pull/3838)) @Kinglykrab 2024-01-07

### Pets

* Convert Load/Save of Pet Info to Repositories ([#3853](https://github.com/EQEmu/Server/pull/3853)) @Kinglykrab 2024-01-07

### Rules

* Casting Charm on over level = Aggro ([#3886](https://github.com/EQEmu/Server/pull/3886)) @fryguy503 2024-01-07
* Classic Invite Requires Target ([#3878](https://github.com/EQEmu/Server/pull/3878)) @fryguy503 2024-01-07
* Evac Aggro Wipe ([#3880](https://github.com/EQEmu/Server/pull/3880)) @fryguy503 2024-01-07
* Mounts will wear off on zone ([#3865](https://github.com/EQEmu/Server/pull/3865)) @fryguy503 2024-01-07
* PC Push and NPCtoNPC Push ([#3879](https://github.com/EQEmu/Server/pull/3879)) @fryguy503 2024-01-07
* RequireMnemonicRetention for Spells 9-12 Rule ([#3876](https://github.com/EQEmu/Server/pull/3876)) @fryguy503 2024-01-07
* Resist Softcap rules ([#3863](https://github.com/EQEmu/Server/pull/3863)) @fryguy503 2024-01-07
* Restrict Finishing Blow to only Fleeing NPC's. ([#3869](https://github.com/EQEmu/Server/pull/3869)) @fryguy503 2024-01-07
* Undead Aggro ([#3881](https://github.com/EQEmu/Server/pull/3881)) @fryguy503 2024-01-07

### Tuning

* FD and Sneak break when cast on adjustments. ([#3861](https://github.com/EQEmu/Server/pull/3861)) @fryguy503 2024-01-07

### UCS

* Consolidate configuration block ([#3768](https://github.com/EQEmu/Server/pull/3768)) @Akkadius 2024-01-07

## [22.39.1] - 12/31/2023

### Code

* Appearance not appearence ([#3819](https://github.com/EQEmu/Server/pull/3819)) @Kinglykrab 2023-12-30
* Delete errmsg.h in common and zone ([#3821](https://github.com/EQEmu/Server/pull/3821)) @Kinglykrab 2023-12-30
* Gender constants cleanup ([#3817](https://github.com/EQEmu/Server/pull/3817)) @Kinglykrab 2023-12-30
* Remove MakeSpawnUpdateNoDelta from mob.cpp/mob.h ([#3816](https://github.com/EQEmu/Server/pull/3816)) @Kinglykrab 2023-12-31
* Remove SendStunAppearance from mob.cpp/mob.h ([#3818](https://github.com/EQEmu/Server/pull/3818)) @Kinglykrab 2023-12-31
* Remove unused PlotPosition methods from mob.cpp/mob.h ([#3820](https://github.com/EQEmu/Server/pull/3820)) @Kinglykrab 2023-12-31

### Database

* Database update improvements, content db and terminal checks ([#3814](https://github.com/EQEmu/Server/pull/3814)) @Akkadius 2023-12-31

### Fixes

* Client:SetBucket Overload Incorrectly Named ([#3825](https://github.com/EQEmu/Server/pull/3825)) @fryguy503 2023-12-30
* Fix crash in Client::Handle_OP_GMGoto ([#3832](https://github.com/EQEmu/Server/pull/3832)) @Kinglykrab 2023-12-31

### Quest API

* Add HasItemOnCorpse() to Perl/Lua ([#3824](https://github.com/EQEmu/Server/pull/3824)) @Kinglykrab 2023-12-31
* Fix issue with death events. ([#3823](https://github.com/EQEmu/Server/pull/3823)) @Kinglykrab 2023-12-31

### Repositories

* Protected extended repositories from being overwritten if exists ([#3815](https://github.com/EQEmu/Server/pull/3815)) @Akkadius 2023-12-31

## [22.39.0] - 12/27/2023

### Character

* Fix character copier due to schema change ([#3805](https://github.com/EQEmu/Server/pull/3805)) @Akkadius 2023-12-28

### Combat

* Disarm was not dropping item to ground due to bug ([#3811](https://github.com/EQEmu/Server/pull/3811)) @noudess 2023-12-27

### Logs

* Bulk insert new log settings ([#3810](https://github.com/EQEmu/Server/pull/3810)) @Akkadius 2023-12-28
* Reclassify unhelpful Info message ([#3809](https://github.com/EQEmu/Server/pull/3809)) @Akkadius 2023-12-28

### MySQL

* Fix MySQL Query error formatting ([#3808](https://github.com/EQEmu/Server/pull/3808)) @Akkadius 2023-12-28

### Objects

* Remove "No objects to load for zone" error message ([#3807](https://github.com/EQEmu/Server/pull/3807)) @Akkadius 2023-12-28

### Player Events

* Bulk replace settings on boot ([#3806](https://github.com/EQEmu/Server/pull/3806)) @Akkadius 2023-12-28

### Tasks

* Add enabled column ([#3804](https://github.com/EQEmu/Server/pull/3804)) @Akkadius 2023-12-28

## [22.38.0] - 12/26/2023

### Bots

* Remove unnecessary error on SetItemReuse ([#3795](https://github.com/EQEmu/Server/pull/3795)) @nytmyr 2023-12-20

### Code

* Consolidate GetHateRandom(), GetHateRandomBot(), GetHateRandomClient(), and GetHateRandomNPC() ([#3794](https://github.com/EQEmu/Server/pull/3794)) @Kinglykrab 2023-12-25
* Race constants refactor ([#3782](https://github.com/EQEmu/Server/pull/3782)) @Akkadius 2023-12-23

### Database

* Fix issue with saylinks query in MySQL 8.0+ ([#3800](https://github.com/EQEmu/Server/pull/3800)) @Akkadius 2023-12-24
* Update faction mods with Live data ([#3799](https://github.com/EQEmu/Server/pull/3799)) @joligario 2023-12-23

### Fixes

* Disable Hide/Improved Hide on Trap damage ([#3791](https://github.com/EQEmu/Server/pull/3791)) @Kinglykrab 2023-12-19
* Fix Bard Invisibility Songs breaking every 4 ticks ([#3783](https://github.com/EQEmu/Server/pull/3783)) @Kinglykrab 2023-12-19
* Fix can_riposte parameter in DoMeleeSkillAttackDmg ([#3792](https://github.com/EQEmu/Server/pull/3792)) @Kinglykrab 2023-12-19

### Forage

* Add a rule to disabled using common_food_ids from the list in forage.cpp.  currently set to enabled. ([#3796](https://github.com/EQEmu/Server/pull/3796)) @regneq 2023-12-22

### NPC

* Support for multiple emotes per type, emote variables ([#3801](https://github.com/EQEmu/Server/pull/3801)) @regneq 2023-12-25

### Quest API

* Add GetHateTopBot(), GetHateTopClient(), and GetHateTopNPC() to Perl/Lua ([#3793](https://github.com/EQEmu/Server/pull/3793)) @Kinglykrab 2023-12-22
* Add SummonItemIntoInventory() to Perl/Lua ([#3797](https://github.com/EQEmu/Server/pull/3797)) @Kinglykrab 2023-12-22

### Repositories

* Add ReplaceOne and ReplaceMany ([#3802](https://github.com/EQEmu/Server/pull/3802)) @Akkadius 2023-12-26

## [22.37.0] - 12/18/2023

### Bots

* Add ScanCloseMobs support to fix AEs ([#3786](https://github.com/EQEmu/Server/pull/3786)) @nytmyr 2023-12-18
* Expand ^itemuse options ([#3756](https://github.com/EQEmu/Server/pull/3756)) @nytmyr 2023-12-17
* Fix ^defensive from checking aggressive disciplines. ([#3787](https://github.com/EQEmu/Server/pull/3787)) @nytmyr 2023-12-18
* Fix ^oo autodefend from sending bots/pets to invalid haters ([#3772](https://github.com/EQEmu/Server/pull/3772)) @nytmyr 2023-12-16
* Fix unnecessary failed to save timer error ([#3788](https://github.com/EQEmu/Server/pull/3788)) @nytmyr 2023-12-18
* [Quest API] Add ^clickitem, ^timer, fix GetBestBotSpellForCure ([#3755](https://github.com/EQEmu/Server/pull/3755)) @nytmyr 2023-12-17

### CI

* Switch to use clang for Linux builds (speed) ([#3777](https://github.com/EQEmu/Server/pull/3777)) @Akkadius 2023-12-17

### Compilation

* Use pre-compiled headers for Windows (speed) ([#3778](https://github.com/EQEmu/Server/pull/3778)) @Akkadius 2023-12-18

### Fixes

* Drop Invisibility when hit by traps ([#3785](https://github.com/EQEmu/Server/pull/3785)) @Kinglykrab 2023-12-18
* Fix NPCs routing to 0.0, 0.0 on #summon ([#3780](https://github.com/EQEmu/Server/pull/3780)) @Kinglykrab 2023-12-18
* Fix bad merge  @Akkadius 2023-12-17
* Fix issue with HOTBonusHealingSplitOverDuration Rule ([#3776](https://github.com/EQEmu/Server/pull/3776)) @Kinglykrab 2023-12-17
* Fixed the discrepacy with time using command #time and in quests. ([#3767](https://github.com/EQEmu/Server/pull/3767)) @regneq 2023-12-17
* Send Entity ID in Death Events to resolve #3721 ([#3779](https://github.com/EQEmu/Server/pull/3779)) @Kinglykrab 2023-12-18

### Quest API

* Add EVENT_ALT_CURRENCY_GAIN and EVENT_ALT_CURRENCY_LOSS to Perl/Lua ([#3734](https://github.com/EQEmu/Server/pull/3734)) @Kinglykrab 2023-12-17
* Add EVENT_CRYSTAL_GAIN and EVENT_CRYSTAL_LOSS to Perl/Lua ([#3735](https://github.com/EQEmu/Server/pull/3735)) @Kinglykrab 2023-12-17
* Add EVENT_LDON_POINTS_GAIN and EVENT_LDON_POINTS_LOSS to Perl/Lua ([#3742](https://github.com/EQEmu/Server/pull/3742)) @Kinglykrab 2023-12-17
* Add EVENT_LEVEL_UP and EVENT_LEVEL_DOWN to Bots ([#3750](https://github.com/EQEmu/Server/pull/3750)) @Kinglykrab 2023-12-17
* Add EVENT_LOOT_ADDED to Perl/Lua ([#3739](https://github.com/EQEmu/Server/pull/3739)) @Kinglykrab 2023-12-17
* Add GetNPCAggro() and SetNPCAggro() to Perl/Lua ([#3781](https://github.com/EQEmu/Server/pull/3781)) @Kinglykrab 2023-12-18

## [22.36.0] - 12/16/2023

### Bots

* Add rule to toggle DT hitting owner ([#3757](https://github.com/EQEmu/Server/pull/3757)) @nytmyr 2023-12-11
* Enable auto-saving of bots. ([#3758](https://github.com/EQEmu/Server/pull/3758)) @nytmyr 2023-12-13

### Code

* Cleanup classes.cpp/classes.h ([#3752](https://github.com/EQEmu/Server/pull/3752)) @Kinglykrab 2023-12-13

### Corpse

* Fix /corpse command regression from #3727 ([#3770](https://github.com/EQEmu/Server/pull/3770)) @Akkadius 2023-12-16

### Database

* Make it clearer to users that a database backup is occurring ([#3769](https://github.com/EQEmu/Server/pull/3769)) @Akkadius 2023-12-16
* When database version is greater than binary, we are up to date ([#3771](https://github.com/EQEmu/Server/pull/3771)) @Akkadius 2023-12-16

### Fixes

* Fix Starting Items SQL ([#3766](https://github.com/EQEmu/Server/pull/3766)) @Kinglykrab 2023-12-16

### Logging

* Change empty object loading to warning ([#3759](https://github.com/EQEmu/Server/pull/3759)) @nytmyr 2023-12-11

### Rules

* Add DOT and HOT Rules ([#3760](https://github.com/EQEmu/Server/pull/3760)) @Kinglykrab 2023-12-16

## [22.35.0] - 12/10/2023

### Bots

* Add BotHealOnLevel to fully heal/mana on level. ([#3745](https://github.com/EQEmu/Server/pull/3745)) @nytmyr 2023-12-08
* Fix bots learning spells on level ([#3744](https://github.com/EQEmu/Server/pull/3744)) @nytmyr 2023-12-08

### Bug

* Fix blocked spells regression from #3638 ([#3753](https://github.com/EQEmu/Server/pull/3753)) @joligario 2023-12-11
* PR 3638 Missed the blocked spells repository updates ([#3748](https://github.com/EQEmu/Server/pull/3748)) @fryguy503 2023-12-08

### CMake

* Update minimum version of CMake ([#3743](https://github.com/EQEmu/Server/pull/3743)) @joligario 2023-12-08

### Code

* Remove hard-coded Status Checks ([#3727](https://github.com/EQEmu/Server/pull/3727)) @Kinglykrab 2023-12-03

### Commands

* #guild set CharName 0 did not remove char from guild. ([#3717](https://github.com/EQEmu/Server/pull/3717)) @noudess 2023-11-25
* #petname changes PC to Nobody if selected. ([#3720](https://github.com/EQEmu/Server/pull/3720)) @noudess 2023-11-26
* Add #show aas Command ([#3710](https://github.com/EQEmu/Server/pull/3710)) @Kinglykrab 2023-11-26
* Add #task complete Command ([#3711](https://github.com/EQEmu/Server/pull/3711)) @Kinglykrab 2023-11-26
* Cleanup #acceptrules Command ([#3716](https://github.com/EQEmu/Server/pull/3716)) @Kinglykrab 2023-11-26
* Cleanup #disarmtrap Command ([#3713](https://github.com/EQEmu/Server/pull/3713)) @Kinglykrab 2023-11-26
* Cleanup #list Command ([#3714](https://github.com/EQEmu/Server/pull/3714)) @Kinglykrab 2023-11-26
* Cleanup #movement Command ([#3715](https://github.com/EQEmu/Server/pull/3715)) @Kinglykrab 2023-11-26
* Cleanup #object Command ([#3722](https://github.com/EQEmu/Server/pull/3722)) @Kinglykrab 2023-12-03
* Cleanup #zonebootup and #zoneshutdown Commands ([#3729](https://github.com/EQEmu/Server/pull/3729)) @Kinglykrab 2023-12-03
* Fix formatting of #wpinfo output. ([#3728](https://github.com/EQEmu/Server/pull/3728)) @noudess 2023-12-01

### Database

* Add primary key to keyring table ([#3746](https://github.com/EQEmu/Server/pull/3746)) @Kinglykrab 2023-12-08
* Consolidate Starting Items Table ([#3723](https://github.com/EQEmu/Server/pull/3723)) @Kinglykrab 2023-11-30
* Extra whitespace in #3723 ([#3730](https://github.com/EQEmu/Server/pull/3730)) @joligario 2023-12-02
* Minor adjustment to #3726 ([#3732](https://github.com/EQEmu/Server/pull/3732)) @joligario 2023-12-03
* Modify `updated` column in `items` table with proper default. ([#3726](https://github.com/EQEmu/Server/pull/3726)) @joligario 2023-12-02
* Pull Spell Group Cache from Content DB ([#3749](https://github.com/EQEmu/Server/pull/3749)) @fryguy503 2023-12-08

### Faction

* Alliance line is only allowed 1 faction change at a time. ([#3718](https://github.com/EQEmu/Server/pull/3718)) @noudess 2023-11-26

### Fixes

* Changing Group Leader Invalidated GetLeaderName() ([#3712](https://github.com/EQEmu/Server/pull/3712)) @Kinglykrab 2023-11-26
* Fix 9245 SQL ([#3740](https://github.com/EQEmu/Server/pull/3740)) @Kinglykrab 2023-12-05
* Fix Swarm Pets Requiring NPC Aggros Flag ([#3738](https://github.com/EQEmu/Server/pull/3738)) @Kinglykrab 2023-12-05
* Guild Message Limits ([#3724](https://github.com/EQEmu/Server/pull/3724)) @neckkola 2023-11-29

### Quest API

* Add EVENT_AA_GAIN to AddAAPoints() ([#3733](https://github.com/EQEmu/Server/pull/3733)) @Kinglykrab 2023-12-03
* Add GMMove Overloads to Perl/Lua ([#3719](https://github.com/EQEmu/Server/pull/3719)) @Kinglykrab 2023-11-25

### Scripts

* Import items into `items_new` table instead of writing directly to the existing `items` table. ([#3725](https://github.com/EQEmu/Server/pull/3725)) @joligario 2023-11-30
* Revert database engine change from #3702. ([#3736](https://github.com/EQEmu/Server/pull/3736)) @joligario 2023-12-03
* Update 13th Floor Import Tool ([#3702](https://github.com/EQEmu/Server/pull/3702)) @joligario 2023-11-26

## [22.34.2] - 11/23/2023

### Admin

* Update date in changelog ([#3698](https://github.com/EQEmu/Server/pull/3698)) @joligario 2023-11-19

### Code

* Fix typo in #giveitem ([#3704](https://github.com/EQEmu/Server/pull/3704)) @Kinglykrab 2023-11-22

### Fixes

* Add "IgnoreLevelBasedHasteCaps" rule to GetHaste() ([#3705](https://github.com/EQEmu/Server/pull/3705)) @jcr4990 2023-11-23
* Fix bots/Mercenaries being removed from hatelist ([#3708](https://github.com/EQEmu/Server/pull/3708)) @Kinglykrab 2023-11-23
* Fix some spell types failing IsValidSpellRange check ([#3707](https://github.com/EQEmu/Server/pull/3707)) @nytmyr 2023-11-23

### Loginserver

* Update ticket login table structure ([#3703](https://github.com/EQEmu/Server/pull/3703)) @KimLS 2023-11-22

## [22.34.1] - 11/20/2023

### EQTime

Hotfix for world not spamming save messages by setting to detail level logging @Akkadius 2023-11-20

## [22.34.0] - 11/19/2023

### Bots

* Add ownerraid, byclass and byrace actionables and fix group-based arguments for raids. ([#3680](https://github.com/EQEmu/Server/pull/3680)) @nytmyr 2023-11-19

### Code

* Cleanup #giveitem and #summonitem ([#3692](https://github.com/EQEmu/Server/pull/3692)) @Kinglykrab 2023-11-19
* Cleanup #show currencies Command ([#3693](https://github.com/EQEmu/Server/pull/3693)) @Kinglykrab 2023-11-19

### Commands

* Add #show aa_points Command ([#3695](https://github.com/EQEmu/Server/pull/3695)) @Kinglykrab 2023-11-19

### Database

* Pull pet power from content database ([#3689](https://github.com/EQEmu/Server/pull/3689)) @joligario 2023-11-18

### GM Commands

* Add `#takeplatinum` ([#3690](https://github.com/EQEmu/Server/pull/3690)) @joligario 2023-11-19
* Remove duplicate comment ([#3691](https://github.com/EQEmu/Server/pull/3691)) @joligario 2023-11-19

### Illusions

* RandomizeFeastures erased texture. ([#3686](https://github.com/EQEmu/Server/pull/3686)) @noudess 2023-11-12

### Spawn

* (imported from takp) Added min_time and max_time to spawnentry. This will prevent a NPC from… ([#3685](https://github.com/EQEmu/Server/pull/3685)) @regneq 2023-11-18

## [22.33.0] - 11/11/2023

### Feature

* Add Comment to Item Data/Quest API ([#3669](https://github.com/EQEmu/Server/pull/3669)) @Kinglykrab 2023-11-07

### Spawn2

* Fix edge case with instances not copying disabled spawn state ([#3688](https://github.com/EQEmu/Server/pull/3688)) @Akkadius 2023-11-12

## [22.32.1] - 11/6/2023

### Hotfix

* Adjust spawn2_disabled migration to copy data over

## [22.32.0] - 11/6/2023

### Bots

* Fix invalid races from being created ([#3681](https://github.com/EQEmu/Server/pull/3681)) @nytmyr 2023-11-06

### Crash

* Fix crash on CentOS when forming a raid with PCs or BOTs ([#3676](https://github.com/EQEmu/Server/pull/3676)) @neckkola 2023-11-06

### Fixes

* Add IsTGBCompatibleSpell() to package.add ([#3675](https://github.com/EQEmu/Server/pull/3675)) @Kinglykrab 2023-11-04
* Fix Perl__worldwideremovetask package ([#3670](https://github.com/EQEmu/Server/pull/3670)) @Kinglykrab 2023-11-04
* Revert " Fix Killed XYZH support in EVENT_DEATH in Perl. " (#3682) ([#3591](https://github.com/EQEmu/Server/pull/3591)) @fryguy503 2023-11-06
CRASH

### GCC

* Compatibility fix for GCC 13 ([#3677](https://github.com/EQEmu/Server/pull/3677)) @joligario 2023-11-05

### Parser

* Cleanup Spire Parsing for crosszonemoveplayerbycharid ([#3674](https://github.com/EQEmu/Server/pull/3674)) @Kinglykrab 2023-11-04
* Cleanup Spire Parsing for crosszonemoveplayerbyexpeditionid ([#3671](https://github.com/EQEmu/Server/pull/3671)) @Kinglykrab 2023-11-04
* Cleanup Spire Parsing for crosszonemoveplayerbygroupid ([#3673](https://github.com/EQEmu/Server/pull/3673)) @Kinglykrab 2023-11-04
* Cleanup Spire Parsing for crosszonemoveplayerbyguildid ([#3672](https://github.com/EQEmu/Server/pull/3672)) @Kinglykrab 2023-11-04

### Quest API

* Add GetBaseRaceName() to Perl and Lua ([#3668](https://github.com/EQEmu/Server/pull/3668)) @joligario 2023-11-01
* Add details to Lua event dispatch errors ([#3679](https://github.com/EQEmu/Server/pull/3679)) @hgtw 2023-11-06

### Spawn

* Split spawn2 enabled into its own state table ([#3664](https://github.com/EQEmu/Server/pull/3664)) @Akkadius 2023-11-06

### Spells

* Added IsNightTime() for Dance of the Fireflies ([#3667](https://github.com/EQEmu/Server/pull/3667)) @regneq 2023-11-04

## [22.31.3] - 10/31/2023

### Bug

* Force raids off content database ([#3665](https://github.com/EQEmu/Server/pull/3665)) @joligario 2023-10-31

### Crash

* Revert " Fix spell in AESpell related to beacons " ([#3659](https://github.com/EQEmu/Server/pull/3659)) @Akkadius 2023-10-31

### Fixes

* Fix issue with blocked spells not loading properly  @Akkadius 2023-10-31

### Logs

* Convert Loot Messages to Error Logs ([#3663](https://github.com/EQEmu/Server/pull/3663)) @Kinglykrab 2023-10-31

## [22.31.2] - 10/31/2023

### Fixes

* Hotfix issue with beacon spells crashing @Akkadius 2023-10-31

## [22.31.1] - 10/31/2023

### Fixes

* Hotfix issue with blocked spells not loading properly @Akkadius 2023-10-31

## [22.31.0] - 10/29/2023

### Crash

* Fix crash when client pointer does not exist during #hotfix ([#3661](https://github.com/EQEmu/Server/pull/3661)) @Akkadius 2023-10-29
* Fix spell in AESpell related to beacons ([#3659](https://github.com/EQEmu/Server/pull/3659)) @Akkadius 2023-10-29

### Database

* Add id to variables table ([#3658](https://github.com/EQEmu/Server/pull/3658)) @Akkadius 2023-10-29

### Linux

* Add symbols to release builds ([#3660](https://github.com/EQEmu/Server/pull/3660)) @Akkadius 2023-10-29

### Perl

* Revert " Reload perl quests on zone bootup " ([#3648](https://github.com/EQEmu/Server/pull/3648)) @Akkadius 2023-10-26

### Trading

* Fix part 3 of Issue 932. ([#3654](https://github.com/EQEmu/Server/pull/3654)) @noudess 2023-10-29

## [22.30.2] - 10/26/2023

### Fixes

Revert Perl regression in #3648 causing scripts to not reliably initialize on zone bootup. @Akkadius 2023-10-26

## [22.30.1] - 10/24/2023

### Fixes

* Fix empty InsertMany in bot starting items. ([#3653](https://github.com/EQEmu/Server/pull/3653)) @Kinglykrab 2023-10-24

## [22.30.0] - 10/23/2023

### API

* Implement Zone Sidecar ([#3635](https://github.com/EQEmu/Server/pull/3635)) @Akkadius 2023-10-24

### Commands

* Move #suspend from content database ([#3651](https://github.com/EQEmu/Server/pull/3651)) @joligario 2023-10-24

### Fixes

* Fix Bot Starting Items SQL ([#3649](https://github.com/EQEmu/Server/pull/3649)) @Kinglykrab 2023-10-23

### Perl

* Implement eqemu-perl for Linux ([#3652](https://github.com/EQEmu/Server/pull/3652)) @Akkadius 2023-10-24
* Reload perl quests on zone bootup ([#3648](https://github.com/EQEmu/Server/pull/3648)) @hgtw 2023-10-24

### Pets

* Disallow effect of alliance line when cast on pets. ([#3650](https://github.com/EQEmu/Server/pull/3650)) @noudess 2023-10-24

## [22.29.1] - 10/21/2023

### DB

* Fix manifest for blocked spells ([#3646](https://github.com/EQEmu/Server/pull/3646)) @joligario 2023-10-21

### Fixes

* Fix issue with subcommand settings not working ([#3643](https://github.com/EQEmu/Server/pull/3643)) @Kinglykrab 2023-10-21
* Hotfix command without hotfix name ([#3644](https://github.com/EQEmu/Server/pull/3644)) @joligario 2023-10-21
* Verifying mail keys when none exist ([#3645](https://github.com/EQEmu/Server/pull/3645)) @joligario 2023-10-21

## [22.29.0] - 10/20/2023

### Feature

* Add Expansion and Content Flag support to Blocked Spells ([#3638](https://github.com/EQEmu/Server/pull/3638)) @Kinglykrab 2023-10-20

### Fixes

* Fix crash when checking Bot Group/Raid membership ([#3641](https://github.com/EQEmu/Server/pull/3641)) @Aeadoin 2023-10-20

### Perl

* Static linker fix on Linux ([#3642](https://github.com/EQEmu/Server/pull/3642)) @Akkadius 2023-10-20

### Rules

* Add rule to configure max number of procs per round Combat:MaxProcs ([#3640](https://github.com/EQEmu/Server/pull/3640)) @Akkadius 2023-10-20

## [22.28.1] - 10/20/2023

### Build

* Perl Linux build fix

## [22.28.0] - 10/15/2023

### Bots

* Adjust Bot Movement Speed ([#3615](https://github.com/EQEmu/Server/pull/3615)) @Kinglykrab 2023-10-14
* Fix bot removal on zone, regression from #3611 ([#3631](https://github.com/EQEmu/Server/pull/3631)) @Akkadius 2023-10-16

### Crash

* Fix Crash with #summon ([#3618](https://github.com/EQEmu/Server/pull/3618)) @Kinglykrab 2023-10-14
* Fix crash in Mob::ShowBuffs ([#3632](https://github.com/EQEmu/Server/pull/3632)) @Akkadius 2023-10-16
* Resolve crash when assigning empty raid note. ([#3628](https://github.com/EQEmu/Server/pull/3628)) @Aeadoin 2023-10-15

### Feature

* Add Extra Kick Classes ([#3613](https://github.com/EQEmu/Server/pull/3613)) @Kinglykrab 2023-10-11
* Add Immune to Assassinate Special Ability ([#3622](https://github.com/EQEmu/Server/pull/3622)) @Kinglykrab 2023-10-14
* Add Immune to Headshot Special Ability ([#3624](https://github.com/EQEmu/Server/pull/3624)) @Kinglykrab 2023-10-14
* Update Raid Functions for Titanium and Underfoot ([#3524](https://github.com/EQEmu/Server/pull/3524)) @neckkola 2023-10-14

### Fixes

* Fix #cast defaulting to cast time ([#3617](https://github.com/EQEmu/Server/pull/3617)) @Kinglykrab 2023-10-14

### Parser Fix

* Fix SendIllusion Spire parsing ([#3623](https://github.com/EQEmu/Server/pull/3623)) @Kinglykrab 2023-10-14

### Quest API

* Add GrantAllAAPoints() to Perl/Lua and Modify #grantaa ([#3616](https://github.com/EQEmu/Server/pull/3616)) @Kinglykrab 2023-10-14
* Add target ID and spell exports to events ([#3620](https://github.com/EQEmu/Server/pull/3620)) @Kinglykrab 2023-10-15

### Scripts

* Update 13th Floor importer ([#3630](https://github.com/EQEmu/Server/pull/3630)) @joligario 2023-10-16
* Update 13th Floor script for legacy research tome bagtypes ([#3621](https://github.com/EQEmu/Server/pull/3621)) @joligario 2023-10-14

## [22.27.0] - 10/07/2023

### Crash

* Bot member zoned crash fix ([#3607](https://github.com/EQEmu/Server/pull/3607)) @Akkadius 2023-10-07
* Fix #summon crash ([#3608](https://github.com/EQEmu/Server/pull/3608)) @Akkadius 2023-10-07
* Fix CanUseAlternateAdvancementRank crash ([#3609](https://github.com/EQEmu/Server/pull/3609)) @Akkadius 2023-10-07
* Fix crash in #movechar ([#3612](https://github.com/EQEmu/Server/pull/3612)) @Akkadius 2023-10-07
* Fix crash in CastSpell Quest API input cast ([#3610](https://github.com/EQEmu/Server/pull/3610)) @Akkadius 2023-10-07
* Fix dangling pointer crash observed in SendHPPacketsFrom ([#3611](https://github.com/EQEmu/Server/pull/3611)) @Akkadius 2023-10-07
* Fix rarer crash with File::Makedir ([#3606](https://github.com/EQEmu/Server/pull/3606)) @Akkadius 2023-10-07

### Fixes

* Add Validation to #find, #set, and #show args ([#3598](https://github.com/EQEmu/Server/pull/3598)) @Kinglykrab 2023-09-18
* Ensure Linux builds report failures  @Akkadius 2023-10-03
* Fix #show group_info Popup ([#3605](https://github.com/EQEmu/Server/pull/3605)) @Kinglykrab 2023-10-04
* Fix swarm pet names to use '_' instead of ' ' ([#3601](https://github.com/EQEmu/Server/pull/3601)) @noudess 2023-09-19
* Invis vs. Undead/Animal Breaks Charm for Pets ([#3587](https://github.com/EQEmu/Server/pull/3587)) @crdunwel 2023-09-19

### Logs

* Change pathing log messages from Error to Pathing. ([#3604](https://github.com/EQEmu/Server/pull/3604)) @joligario 2023-09-29

### Quest API

* Add Caster ID Parameter to FindBuff in Perl/Lua ([#3590](https://github.com/EQEmu/Server/pull/3590)) @Kinglykrab 2023-09-29

## [22.26.2] - 09/18/2023

### Fixes

* Fix an issue with schema versioning for the AA update

## [22.26.1] - 09/17/2023

### Fixes

* Add Validation to #find, #set, and #show args ([#3598](https://github.com/EQEmu/Server/pull/3598)) @Kinglykrab 2023-09-17

## [22.26.0] - 09/17/2023

### Bug

* Additional Wild Ramp param was causing an overflow it appears at least on local testing. ([#3589](https://github.com/EQEmu/Server/pull/3589)) @fryguy503 2023-09-11

### Commands

* npc_edit faction and #setfaction duplicate and incorrect. ([#3577](https://github.com/EQEmu/Server/pull/3577)) @noudess 2023-09-03

### Feature

* Add Defensive Proc Rules for Level Gap Penalty ([#3580](https://github.com/EQEmu/Server/pull/3580)) @Kinglykrab 2023-09-17
* Add opcodes for Cast and Scribe book buttons ([#3578](https://github.com/EQEmu/Server/pull/3578)) @hgtw 2023-09-17
* Cleanup Group Split Money Logic ([#3583](https://github.com/EQEmu/Server/pull/3583)) @crdunwel 2023-09-17

### Fixes

* AA System Fixes ([#3572](https://github.com/EQEmu/Server/pull/3572)) @KimLS 2023-09-17
* Check for Song Skill Increase on Bard Pulse ([#3586](https://github.com/EQEmu/Server/pull/3586)) @crdunwel 2023-09-17
* Fix #set faction/#setfaction Command ([#3575](https://github.com/EQEmu/Server/pull/3575)) @Kinglykrab 2023-09-02
* Fix Killed XYZH support in EVENT_DEATH in Perl. ([#3591](https://github.com/EQEmu/Server/pull/3591)) @Kinglykrab 2023-09-17
* Fix entity ID on death with #castspell ([#3592](https://github.com/EQEmu/Server/pull/3592)) @Kinglykrab 2023-09-17
* Zoning logging edge case safety  @Akkadius 2023-08-29

### Logging

* Add logging to track down Wild Ramp issue ([#3588](https://github.com/EQEmu/Server/pull/3588)) @fryguy503 2023-09-11

### Messages

* Remove duplicate message on tracking begin ([#3574](https://github.com/EQEmu/Server/pull/3574)) @noudess 2023-09-11
* Swarm pet normal damage messages were missing ([#3594](https://github.com/EQEmu/Server/pull/3594)) @noudess 2023-09-17

### Quest API

* Add ApplySpell() and SetBuffDuration() overloads to Perl/Lua ([#3576](https://github.com/EQEmu/Server/pull/3576)) @Kinglykrab 2023-09-17
* Add XYZ/XYZH Overloads to Cross Zone Move Methods ([#3581](https://github.com/EQEmu/Server/pull/3581)) @Kinglykrab 2023-09-17

## [22.25.0] - 08/28/2023

### Bots

* Add Support for Mana Pool AA Bonuses. ([#3571](https://github.com/EQEmu/Server/pull/3571)) @Aeadoin 2023-08-21
* Remove In-Game Command References to Bot Groups ([#3545](https://github.com/EQEmu/Server/pull/3545)) @Aeadoin 2023-08-13

### Bug

* Escape item name in trader audit. ([#3540](https://github.com/EQEmu/Server/pull/3540)) @fryguy503 2023-08-13

### CMake

* Add cmake option to re-enable MSVC warnings ([#3537](https://github.com/EQEmu/Server/pull/3537)) @hgtw 2023-08-13
* Define perlbind option for all targets ([#3538](https://github.com/EQEmu/Server/pull/3538)) @hgtw 2023-08-13

### Charm

* Fix to update target windows on charm on/off ([#3549](https://github.com/EQEmu/Server/pull/3549)) @noudess 2023-08-20

### Combat Messages

* Fix issue where pet proc damage was not showing up ([#3551](https://github.com/EQEmu/Server/pull/3551)) @noudess 2023-08-24

### Database

* Change primary key entry to NOT NULL ([#3559](https://github.com/EQEmu/Server/pull/3559)) @joligario 2023-08-20
* Extend dumper CLI utility to export static instance data ([#3562](https://github.com/EQEmu/Server/pull/3562)) @Akkadius 2023-08-20

### Expansions

* Expansion settings tweaks ([#3556](https://github.com/EQEmu/Server/pull/3556)) @Akkadius 2023-08-20

### Feature

* Add adjustability for AERampage Range. ([#3548](https://github.com/EQEmu/Server/pull/3548)) @fryguy503 2023-08-20
* Change #reload zone to reload zone headers globally. ([#3557](https://github.com/EQEmu/Server/pull/3557)) @Kinglykrab 2023-08-20
* Change money type to all lower case as EQ live money shares and split has it that way. ([#3550](https://github.com/EQEmu/Server/pull/3550)) @regneq 2023-08-18

### Fixes

* Add character_stats_record to player tables  @Akkadius 2023-08-11
* Bots no longer drop group on death, and raid fixes. ([#3542](https://github.com/EQEmu/Server/pull/3542)) @Aeadoin 2023-08-13
* Bots will now load AAs properly when spawned. ([#3544](https://github.com/EQEmu/Server/pull/3544)) @Aeadoin 2023-08-13
* Clearing target window on CHARM wear off had a side effect ([#3570](https://github.com/EQEmu/Server/pull/3570)) @noudess 2023-08-21
* Fix #motd/#set motd Command ([#3558](https://github.com/EQEmu/Server/pull/3558)) @Kinglykrab 2023-08-20
* Fix Bot::CheckDataBucket to work with Owner Buckets. ([#3552](https://github.com/EQEmu/Server/pull/3552)) @Aeadoin 2023-08-18
* Fix to zoning logging exception  @Akkadius 2023-08-29
* Instance GetUnusedInstanceID crash fox  @Akkadius 2023-08-21
* Minor adjustment to formula calc position to fix modifier bug. ([#3565](https://github.com/EQEmu/Server/pull/3565)) @Valorith 2023-08-21

### Instances

* Honor reserved instances ([#3563](https://github.com/EQEmu/Server/pull/3563)) @joligario 2023-08-21
* Refine id selection ([#3568](https://github.com/EQEmu/Server/pull/3568)) @joligario 2023-08-29

### Logging

* Add detailed zoning logging ([#3555](https://github.com/EQEmu/Server/pull/3555)) @Akkadius 2023-08-20

### Quest API

* Reload content flags globally when a content flag is set ([#3564](https://github.com/EQEmu/Server/pull/3564)) @Akkadius 2023-08-21

### Rules

* Add FinalRaidExpMultiplier Rule ([#3554](https://github.com/EQEmu/Server/pull/3554)) @Valorith 2023-08-20
* Add a rule to adjust the randomization range for Wizard\Caster Merc innate critical ratio. ([#3543](https://github.com/EQEmu/Server/pull/3543)) @catapultam-habeo 2023-08-13
* Add rule to restrict hand in of quest items to quest flagged NPCs. ([#3536](https://github.com/EQEmu/Server/pull/3536)) @Valorith 2023-08-13
* Add rule to toggle pets accepting quest items ([#3533](https://github.com/EQEmu/Server/pull/3533)) @Valorith 2023-08-06
* Also reload rules in world when #reload rules invoked ([#3566](https://github.com/EQEmu/Server/pull/3566)) @Akkadius 2023-08-21
* Correct explanation of Bots:ManaRegen ([#3569](https://github.com/EQEmu/Server/pull/3569)) @Aeadoin 2023-08-21

### Spawns

* Fixes a rarer issue where spawn2 is not being properly content filtered  @Akkadius 2023-08-13

## [22.24.0] - 08/05/2023

### Character

* Record character stats to `character_stats_record` table ([#3522](https://github.com/EQEmu/Server/pull/3522)) @Akkadius 2023-08-05

### Code

* Cleanup #view zone_loot Command ([#3523](https://github.com/EQEmu/Server/pull/3523)) @Kinglykrab 2023-08-02
* Remove Strings::Commify from all identifier values ([#3528](https://github.com/EQEmu/Server/pull/3528)) @Kinglykrab 2023-08-02

### Feature

* Add Support for Drakkin Heritage Illusions ([#3521](https://github.com/EQEmu/Server/pull/3521)) @Kinglykrab 2023-08-01

### Fixes

* Bug fix for raid mark NPC across zones ([#3525](https://github.com/EQEmu/Server/pull/3525)) @neckkola 2023-08-05
* Bugs table should not target content database ([#3535](https://github.com/EQEmu/Server/pull/3535)) @Akkadius 2023-08-05
* Fix +/- 0.1 XYZ Door Manipulation ([#3527](https://github.com/EQEmu/Server/pull/3527)) @Kinglykrab 2023-08-02
* Fix issue with mob scanning when trying to use EVENT_SPAWN ([#3529](https://github.com/EQEmu/Server/pull/3529)) @Akkadius 2023-08-04

### Quest API

* Adjust GetCloseMobList calls internally ([#3530](https://github.com/EQEmu/Server/pull/3530)) @Akkadius 2023-08-04

## [22.23.0] - 07/31/2023

### Databuckets

* Improvements to distributed cache, reload commands ([#3519](https://github.com/EQEmu/Server/pull/3519)) @Akkadius 2023-08-01

### Fixes

* Fix #gm top level alias for #set gm ([#3517](https://github.com/EQEmu/Server/pull/3517)) @Kinglykrab 2023-07-30
* Fix Appearance Issues ([#3520](https://github.com/EQEmu/Server/pull/3520)) @Kinglykrab 2023-08-01
* Fix NPC Cast Events not parsing properly. ([#3518](https://github.com/EQEmu/Server/pull/3518)) @Kinglykrab 2023-08-01

### Scaling/Bug Fix

* Scaling where min and max damage was bugged ([#3514](https://github.com/EQEmu/Server/pull/3514)) @noudess 2023-08-01

## [22.22.1] - 07/30/2023

### Database

* Hotfix: Add command_subsettings to server tables  @Akkadius 2023-07-29

### Doors

* Add door blacklist ([#3516](https://github.com/EQEmu/Server/pull/3516)) @Akkadius 2023-07-30

## [22.22.0] - 07/27/2023

### Code

* Fix casing in corpse money and decay time. ([#3511](https://github.com/EQEmu/Server/pull/3511)) @Kinglykrab 2023-07-23

### Crash Fix

* Guard against Spells:MaxTotalSlotsPET being set above client allowed maximum. ([#3507](https://github.com/EQEmu/Server/pull/3507)) @Valorith 2023-07-22

### Data Buckets

* Distributed Databucket Caching ([#3500](https://github.com/EQEmu/Server/pull/3500)) @Kinglykrab 2023-07-24

### Database

* Fix console output in database:dump --dump-output-to-console  @Akkadius 2023-07-25

### Fixes

* Escape search string in #find item ([#3510](https://github.com/EQEmu/Server/pull/3510)) @Kinglykrab 2023-07-22

### Quest API

* Add GetMobTypeIdentifier() to Perl/Lua ([#3512](https://github.com/EQEmu/Server/pull/3512)) @Kinglykrab 2023-07-28

### Saylink

* Fix cases where saylinks were not being cached ([#3508](https://github.com/EQEmu/Server/pull/3508)) @Akkadius 2023-07-20

## [22.21.2] - 07/19/2023

### Databuckets

* Fix rarer same bucket name scoping overlap issue ([#3509](https://github.com/EQEmu/Server/pull/3509)) @Akkadius 2023-07-20

## [22.21.1] - 07/18/2023

### Bug

* Show Petition and Show Petition_Info fix ([#3503](https://github.com/EQEmu/Server/pull/3503)) @fryguy503 2023-07-18

### Code

* Remove arbitrary teleport blocking in Tutorial and Load zones ([#3506](https://github.com/EQEmu/Server/pull/3506)) @Valorith 2023-07-19

### Databuckets

* Fix issue with expired databuckets not being expired and returned properly ([#3504](https://github.com/EQEmu/Server/pull/3504)) @Akkadius 2023-07-18

### Fixes

* #set title_suffix Argument Position ([#3505](https://github.com/EQEmu/Server/pull/3505)) @Kinglykrab 2023-07-18

## [22.21.0] - 07/18/2023

### Data Buckets

* Implement scoped data buckets ([#3498](https://github.com/EQEmu/Server/pull/3498)) @Akkadius 2023-07-16

### Fixes

* Fix rule check and add rule for pickpocket command ([#3492](https://github.com/EQEmu/Server/pull/3492)) @tuday2 2023-07-16

### Pathing

* Improvements to roambox logic, pathing ([#3502](https://github.com/EQEmu/Server/pull/3502)) @Akkadius 2023-07-18

### Quest API

* Add SetLDoNPoints() to Perl/Lua ([#3496](https://github.com/EQEmu/Server/pull/3496)) @Kinglykrab 2023-07-16

### Z Clipping

* Don't issue zclip adjustments when NPC is not moving ([#3499](https://github.com/EQEmu/Server/pull/3499)) @Akkadius 2023-07-16

## [22.20.1] - 07/15/2023

### Database

* Fix database manifest entry for #3443  @neckkola 2023-07-15

## [22.20.0] - 07/15/2023

### Bots

* Remove orphaned commands related to botgroup ([#3489](https://github.com/EQEmu/Server/pull/3489)) @tuday2 2023-07-09

### Commands

* Consolidate #set-like commands into a singular #set command ([#3486](https://github.com/EQEmu/Server/pull/3486)) @Kinglykrab 2023-07-15

### Feature

* Add Support for item textures higher than 65,535 ([#3494](https://github.com/EQEmu/Server/pull/3494)) @Kinglykrab 2023-07-14
* Update raid features ([#3443](https://github.com/EQEmu/Server/pull/3443)) @neckkola 2023-07-13

### Fixes

* Fix Tradeskill Combines with augmented items ([#3490](https://github.com/EQEmu/Server/pull/3490)) @Kinglykrab 2023-07-15
* Fix charmed pets to follow when charmed. ([#3488](https://github.com/EQEmu/Server/pull/3488)) @noudess 2023-07-08
* Update bot naming check and add more explanation ([#3491](https://github.com/EQEmu/Server/pull/3491)) @tuday2 2023-07-13

### Quest API

* Add Mob/Entity type check methods to Perl/Lua ([#3493](https://github.com/EQEmu/Server/pull/3493)) @Kinglykrab 2023-07-13

## [22.19.0] - 07/08/2023

### Bots

* Add Pickpocket Command ([#3484](https://github.com/EQEmu/Server/pull/3484)) @tuday2 2023-07-08

### Code

* Move #find item summon links to front ([#3483](https://github.com/EQEmu/Server/pull/3483)) @Kinglykrab 2023-07-05
* Remove LoadItemDBFieldNames() from common/misc.cpp and common/misc.h ([#3473](https://github.com/EQEmu/Server/pull/3473)) @Kinglykrab 2023-07-04

### Commands

* Add #itemsearch alias to #find aliases ([#3485](https://github.com/EQEmu/Server/pull/3485)) @Kinglykrab 2023-07-08
* Consolidate #show commands into a singular #show command ([#3478](https://github.com/EQEmu/Server/pull/3478)) @Kinglykrab 2023-07-08

### Fixes

* Fix _PutItem having a slot_id of -1 on mobs with no items ([#3474](https://github.com/EQEmu/Server/pull/3474)) @Kinglykrab 2023-07-04

### Rules

* Add Skills:TrivialTradeskillCombinesNoFail Rule ([#3481](https://github.com/EQEmu/Server/pull/3481)) @JasXSL 2023-07-05

## [22.18.0] - 07/04/2023

### Commands

* Add #emotesearch to #find command ([#3480](https://github.com/EQEmu/Server/pull/3480)) @Kinglykrab 2023-07-04
* Cleanup #copycharacter Command ([#3479](https://github.com/EQEmu/Server/pull/3479)) @Kinglykrab 2023-07-04

### Fixes

* Add chatchannel_reserved_names to a new manifest ([#3482](https://github.com/EQEmu/Server/pull/3482)) @Kinglykrab 2023-07-04
* Fix issue in zone store of returning reference to local variable ([#3477](https://github.com/EQEmu/Server/pull/3477)) @Kinglykrab 2023-07-04

## [22.17.0] - 07/03/2023

### Cleanup/Feature

* Add support for bots to #showstats/#mystats ([#3427](https://github.com/EQEmu/Server/pull/3427)) @Kinglykrab 2023-07-01

### Code

* Remove LoadItemDBFieldNames() from common/misc.cpp and common/misc.h ([#3473](https://github.com/EQEmu/Server/pull/3473)) @Kinglykrab 2023-07-04
* Remove handle_npc_single_npc from zone/lua_parser_events.cpp and zone/lua_parser_events.h ([#3467](https://github.com/EQEmu/Server/pull/3467)) @Kinglykrab 2023-07-03

### Database

* Set multi statements off when returning early ([#3462](https://github.com/EQEmu/Server/pull/3462)) @Akkadius 2023-07-01

### Feature

* Add Strings::BeginsWith() and Strings::EndsWith() ([#3471](https://github.com/EQEmu/Server/pull/3471)) @Kinglykrab 2023-07-03

### Fixes

* Add check for underscores in botcreate command ([#3458](https://github.com/EQEmu/Server/pull/3458)) @tuday2 2023-06-29
* EVENT_LANGUAGE_SKILL_UP in Lua was using EVENT_SKILL_UP logic ([#3466](https://github.com/EQEmu/Server/pull/3466)) @Kinglykrab 2023-07-03
* Fix _PutItem having a slot_id of -1 on mobs with no items ([#3474](https://github.com/EQEmu/Server/pull/3474)) @Kinglykrab 2023-07-04
* Fix data type of GetAggroCount() ([#3470](https://github.com/EQEmu/Server/pull/3470)) @Kinglykrab 2023-07-03

### Logging

* Fix logging crash when % are sent through query logs ([#3461](https://github.com/EQEmu/Server/pull/3461)) @Akkadius 2023-07-01

### Quest API

* Add ClearAccountFlag() and GetAccountFlags() to Perl/Lua ([#3469](https://github.com/EQEmu/Server/pull/3469)) @Kinglykrab 2023-07-03
* Add GetClassAbbreviation() and GetRaceAbbreviation() to Perl/Lua ([#3463](https://github.com/EQEmu/Server/pull/3463)) @Kinglykrab 2023-07-02
* Add GetClassPlural() and GetRacePlural() to Perl/Lua ([#3468](https://github.com/EQEmu/Server/pull/3468)) @Kinglykrab 2023-07-03
* Add GetCloseMobList() and CalculateDistance() overload to Perl/Lua ([#3455](https://github.com/EQEmu/Server/pull/3455)) @Kinglykrab 2023-07-02
* Add Hate Entry Methods to Perl ([#3459](https://github.com/EQEmu/Server/pull/3459)) @Kinglykrab 2023-07-02
* Add ItemData Class to Perl ([#3465](https://github.com/EQEmu/Server/pull/3465)) @Kinglykrab 2023-07-02
* Add Spawn2 Class to Perl ([#3456](https://github.com/EQEmu/Server/pull/3456)) @Kinglykrab 2023-07-02
* Add StatBonuses Class to Perl ([#3460](https://github.com/EQEmu/Server/pull/3460)) @Kinglykrab 2023-07-02
* Add missing Item Methods to Perl/Lua. ([#3464](https://github.com/EQEmu/Server/pull/3464)) @Kinglykrab 2023-07-02

## [22.16.0] - 06/27/2023

### Code

* Default skill type to Hand to Hand in #npcedit meleetype ([#3422](https://github.com/EQEmu/Server/pull/3422)) @Kinglykrab 2023-06-19
* Delete common/worldconn.cpp ([#3436](https://github.com/EQEmu/Server/pull/3436)) @Kinglykrab 2023-06-24
* Remove DatabaseCastAccepted() from zone/npc.cpp and zone/npc.h ([#3449](https://github.com/EQEmu/Server/pull/3449)) @Kinglykrab 2023-06-24
* Remove GetACAvoid() from zone/merc.h ([#3447](https://github.com/EQEmu/Server/pull/3447)) @Kinglykrab 2023-06-25
* Remove GetACMit() from zone/merc.h ([#3446](https://github.com/EQEmu/Server/pull/3446)) @Kinglykrab 2023-06-24
* Remove _ClearWaypints() from zone/npc.h ([#3445](https://github.com/EQEmu/Server/pull/3445)) @Kinglykrab 2023-06-24
* Remove acmod() from zone/merc.h ([#3448](https://github.com/EQEmu/Server/pull/3448)) @Kinglykrab 2023-06-24
* Remove command_packetprofile from zone/command.h ([#3432](https://github.com/EQEmu/Server/pull/3432)) @Kinglykrab 2023-06-24
* Remove command_showpetspell in zone/command.h ([#3430](https://github.com/EQEmu/Server/pull/3430)) @Kinglykrab 2023-06-24
* Remove command_unlock from zone/command.h ([#3431](https://github.com/EQEmu/Server/pull/3431)) @Kinglykrab 2023-06-24

### Commands

* Add #finddeity Command ([#3435](https://github.com/EQEmu/Server/pull/3435)) @Kinglykrab 2023-06-26
* Add #findlanguage Command ([#3434](https://github.com/EQEmu/Server/pull/3434)) @Kinglykrab 2023-06-25
* Add #showspells Command ([#3429](https://github.com/EQEmu/Server/pull/3429)) @Kinglykrab 2023-06-24
* Add missing subcommands to #npcedit ([#3423](https://github.com/EQEmu/Server/pull/3423)) @Kinglykrab 2023-06-19
* Cleanup #showbuffs Command ([#3439](https://github.com/EQEmu/Server/pull/3439)) @Kinglykrab 2023-06-26
* Cleanup #shownpcgloballoot and #showzonegloballoot Commands ([#3440](https://github.com/EQEmu/Server/pull/3440)) @Kinglykrab 2023-06-24
* Cleanup #viewcurrencies Command ([#3441](https://github.com/EQEmu/Server/pull/3441)) @Kinglykrab 2023-06-25
* Consolidate #findX commands to a singular #find Command ([#3452](https://github.com/EQEmu/Server/pull/3452)) @Kinglykrab 2023-06-28
* Consolidate #merchant_close_shop and #merchant_open_shop to #merchantshop ([#3433](https://github.com/EQEmu/Server/pull/3433)) @Kinglykrab 2023-06-24
* Delete #showbonusstats Command ([#3437](https://github.com/EQEmu/Server/pull/3437)) @Kinglykrab 2023-06-24
* Delete #spellinfo Command ([#3438](https://github.com/EQEmu/Server/pull/3438)) @Kinglykrab 2023-06-24

### Database

* Fix database version checking edge case issue ([#3428](https://github.com/EQEmu/Server/pull/3428)) @Akkadius 2023-06-22
* Fix multi-statement error reporting ([#3425](https://github.com/EQEmu/Server/pull/3425)) @Akkadius 2023-06-19
* Implement native database migrations in server ([#2857](https://github.com/EQEmu/Server/pull/2857)) @Akkadius 2023-06-19

### Fixes

* Fix NPC Item Stat Bonuses ([#3444](https://github.com/EQEmu/Server/pull/3444)) @Kinglykrab 2023-06-26
* Fix error in 023_01_21_bots_raid_members.sql ([#3453](https://github.com/EQEmu/Server/pull/3453)) @mibastian 2023-06-28
* Fix possible crash with #npcedit weapon ([#3421](https://github.com/EQEmu/Server/pull/3421)) @Kinglykrab 2023-06-19
* Merchant Open Flag set only for regular Merchants ([#3454](https://github.com/EQEmu/Server/pull/3454)) @Kinglykrab 2023-06-27

### Readme

* Update new location of database updates ([#3424](https://github.com/EQEmu/Server/pull/3424)) @joligario 2023-06-19

### Rules

* Add ClientPetsUserOwnerNameInLastName rule ([#3442](https://github.com/EQEmu/Server/pull/3442)) @Kinglykrab 2023-06-25

## [22.15.3] - 06/19/2023

### Fixes

* Fix improper condition in Water LOS checks ([#3426](https://github.com/EQEmu/Server/pull/3426)) @Kinglykrab 2023-06-19

## [22.15.2] - 06/19/2023

### Database

* Fix multi-statement error reporting ([#3425](https://github.com/EQEmu/Server/pull/3425)) @Akkadius 2023-06-19

## [22.15.1] - 06/19/2023

### Schema

* Add `chatchannel_reserved_names` table to `database_schema.h`

## [22.15.0] - 06/19/2023

### Code

* Default skill type to Hand to Hand in #npcedit meleetype ([#3422](https://github.com/EQEmu/Server/pull/3422)) @Kinglykrab 2023-06-19

### Commands

* Add missing subcommands to #npcedit ([#3423](https://github.com/EQEmu/Server/pull/3423)) @Kinglykrab 2023-06-19

### Database

* Implement native database migrations in server ([#2857](https://github.com/EQEmu/Server/pull/2857)) @Akkadius 2023-06-19

### Fixes

* Fix possible crash with #npcedit weapon ([#3421](https://github.com/EQEmu/Server/pull/3421)) @Kinglykrab 2023-06-19

## [22.14.1] - 06/18/2023

### Backups

* Move world database:dump to use MySQL credentials file ([#3410](https://github.com/EQEmu/Server/pull/3410)) @Akkadius 2023-06-17

### Binaries

* Add support for static linking (portable) binaries ([#3417](https://github.com/EQEmu/Server/pull/3417)) @Akkadius 2023-06-18

### CI

* Build static linux binaries ([#3419](https://github.com/EQEmu/Server/pull/3419)) @Akkadius 2023-06-18

### CLI

* Add `bots:enable` and `bots:disable` commands ([#3415](https://github.com/EQEmu/Server/pull/3415)) @Akkadius 2023-06-17
* Add `mercs:enable` and `mercs:disable` commands ([#3416](https://github.com/EQEmu/Server/pull/3416)) @Akkadius 2023-06-18
* Console menu validation fixes ([#3413](https://github.com/EQEmu/Server/pull/3413)) @Akkadius 2023-06-17

### Database

* Add query multi statement execution support ([#3414](https://github.com/EQEmu/Server/pull/3414)) @Akkadius 2023-06-17

### Feature

* Add Water Line of Sight Checks ([#3408](https://github.com/EQEmu/Server/pull/3408)) @nytmyr 2023-06-17

### Logging

* Logging improvements, console silencing, terminal coloring ([#3412](https://github.com/EQEmu/Server/pull/3412)) @Akkadius 2023-06-17

### Quest API

* Add convert_money_to_string() to Perl/Lua ([#3418](https://github.com/EQEmu/Server/pull/3418)) @Kinglykrab 2023-06-18

### Strings

* Add more test cases for string utils ([#3411](https://github.com/EQEmu/Server/pull/3411)) @Akkadius 2023-06-17

### Targeting

* Fix bug when using /tar on invalid target ([#3407](https://github.com/EQEmu/Server/pull/3407)) @noudess 2023-06-17

### Telnet

* Add cross zone/world wide cast and move functionality to Telnet ([#3409](https://github.com/EQEmu/Server/pull/3409)) @Kinglykrab 2023-06-17

## [22.13.1] - 06/13/2023

### Targeting

* Revert #3383 ([#3405](https://github.com/EQEmu/Server/pull/3405)) @noudess 2023-06-13

## [22.13.0] - 06/12/2023

### Code

* Add GMFind_Struct to packet structures ([#3402](https://github.com/EQEmu/Server/pull/3402)) @Kinglykrab 2023-06-12
* Remove CHECK_LOS_STEP from zone/common.h ([#3398](https://github.com/EQEmu/Server/pull/3398)) @Kinglykrab 2023-06-12
* Remove CheckCoordLosNoZLeaps() from zone/entity.cpp and zone/entity.h ([#3384](https://github.com/EQEmu/Server/pull/3384)) @Kinglykrab 2023-06-12
* Remove CountTempPets() from zone/entity.cpp and zone/entity.h ([#3390](https://github.com/EQEmu/Server/pull/3390)) @Kinglykrab 2023-06-12
* Remove Evade() from zone/entity.cpp and zone/entity.h ([#3394](https://github.com/EQEmu/Server/pull/3394)) @Kinglykrab 2023-06-12
* Remove GateAllClients() from zone/entity.cpp and zone/entity.h ([#3391](https://github.com/EQEmu/Server/pull/3391)) @Kinglykrab 2023-06-12
* Remove GetClient(ip, port) from zone/entity.h ([#3386](https://github.com/EQEmu/Server/pull/3386)) @Kinglykrab 2023-06-12
* Remove GetClientCount() from zone/entity.cpp and zone/entity.h ([#3392](https://github.com/EQEmu/Server/pull/3392)) @Kinglykrab 2023-06-12
* Remove GetGroupByBot(), GetRaidByMob(), and GetRaidByLeaderName() from zone/entity.cpp and zone/entity.h ([#3387](https://github.com/EQEmu/Server/pull/3387)) @Kinglykrab 2023-06-12
* Remove InteractiveChat() and TakenAction() from zone/npc.h ([#3382](https://github.com/EQEmu/Server/pull/3382)) @Kinglykrab 2023-06-12
* Remove LimitCheckBoth() from zone/entity.cpp and zone/entity.h ([#3393](https://github.com/EQEmu/Server/pull/3393)) @Kinglykrab 2023-06-12
* Remove NPC::AddCash() from npc.cpp/npc.h ([#3380](https://github.com/EQEmu/Server/pull/3380)) @Kinglykrab 2023-06-09
* Remove RemoveMob() and RemoveRaid() from zone/entity.cpp and zone/entity.h ([#3389](https://github.com/EQEmu/Server/pull/3389)) @Kinglykrab 2023-06-12
* Remove SPECIALIZE_MANA_REDUCE from zone/common.h ([#3400](https://github.com/EQEmu/Server/pull/3400)) @Kinglykrab 2023-06-12
* Remove SendAATimer() from zone/entity.h ([#3388](https://github.com/EQEmu/Server/pull/3388)) @Kinglykrab 2023-06-12
* Remove WriteEntityIDs() from zone/entity.cpp and zone/entity.h ([#3395](https://github.com/EQEmu/Server/pull/3395)) @Kinglykrab 2023-06-12
* Remove _BECOMENPCPET() and _NPCPET() from zone/common.h ([#3399](https://github.com/EQEmu/Server/pull/3399)) @Kinglykrab 2023-06-12
* Remove pDBAsyncWorkID from zone/entity.h ([#3385](https://github.com/EQEmu/Server/pull/3385)) @Kinglykrab 2023-06-12
* Remove struct DynamicZoneSafeReturn from zone/entity.h ([#3396](https://github.com/EQEmu/Server/pull/3396)) @Kinglykrab 2023-06-12
* Remove struct TradeEntity from zone/common.h ([#3397](https://github.com/EQEmu/Server/pull/3397)) @Kinglykrab 2023-06-12

### Commands

* Assign #opcode to a #reload alias ([#3401](https://github.com/EQEmu/Server/pull/3401)) @Kinglykrab 2023-06-12

### Illusions

* RandomizeFeatures and SetGender were killing db texture ([#3376](https://github.com/EQEmu/Server/pull/3376)) @noudess 2023-06-06

### Logging

* Fixed statements that logged incorrect data ([#3381](https://github.com/EQEmu/Server/pull/3381)) @noudess 2023-06-07

### Quest API

* Add GetEXPForLevel() to Perl/Lua ([#3403](https://github.com/EQEmu/Server/pull/3403)) @Kinglykrab 2023-06-12
* Add SendChannelMessage() to Perl/Lua ([#3378](https://github.com/EQEmu/Server/pull/3378)) @Kinglykrab 2023-06-04
* Add several spell methods to Perl/Lua ([#3379](https://github.com/EQEmu/Server/pull/3379)) @Kinglykrab 2023-06-12

### Targeting

* /tar <bad target> should not untarget existing target ([#3383](https://github.com/EQEmu/Server/pull/3383)) @noudess 2023-06-12

## [22.12.0] - 05/29/2023

### Code

* Cleanup #setskill and #setskillall Commands ([#3367](https://github.com/EQEmu/Server/pull/3367)) @Kinglykrab 2023-05-25
* Delete message.h ([#3348](https://github.com/EQEmu/Server/pull/3348)) @Kinglykrab 2023-05-17
* Fix #spawn command NPCs having 0 health ([#3371](https://github.com/EQEmu/Server/pull/3371)) @Kinglykrab 2023-05-21
* Remove CalcPetHp from spdat.h ([#3364](https://github.com/EQEmu/Server/pull/3364)) @Kinglykrab 2023-05-25
* Remove CountNPC() and QueueManaged() from entity.cpp/entity.h ([#3346](https://github.com/EQEmu/Server/pull/3346)) @Kinglykrab 2023-05-17
* Remove DumpMerchantList() from zone.cpp/zone.h ([#3343](https://github.com/EQEmu/Server/pull/3343)) @Kinglykrab 2023-05-17
* Remove GetDamageReceived() and GetHealReceived() from combat_record.cpp/combat_record.h ([#3358](https://github.com/EQEmu/Server/pull/3358)) @Kinglykrab 2023-05-17
* Remove GetEscapingEntOnHateList() from hate_list.cpp/hate_list.h ([#3353](https://github.com/EQEmu/Server/pull/3353)) @Kinglykrab 2023-05-17
* Remove GetMaxRank() from aa_ability.cpp/aa_ability.h ([#3347](https://github.com/EQEmu/Server/pull/3347)) @Kinglykrab 2023-05-25
* Remove IsEntityInFrenzyMode() from hate_list.cpp/hate_list.h ([#3352](https://github.com/EQEmu/Server/pull/3352)) @Kinglykrab 2023-05-17
* Remove IsRaid() from raids.h ([#3361](https://github.com/EQEmu/Server/pull/3361)) @Kinglykrab 2023-05-25
* Remove LoadSpawn2() and PopulateZoneSpawnListClose() from spawn2.cpp/zonedb.h ([#3344](https://github.com/EQEmu/Server/pull/3344)) @Kinglykrab 2023-05-25
* Remove SetGraveyard() from zone.cpp/zone.h ([#3354](https://github.com/EQEmu/Server/pull/3354)) @Kinglykrab 2023-05-17
* Remove SetTradeCash() from trading.cpp/common.h ([#3356](https://github.com/EQEmu/Server/pull/3356)) @Kinglykrab 2023-05-17
* Remove TraderUpdate() from trading.cpp/client.h ([#3357](https://github.com/EQEmu/Server/pull/3357)) @Kinglykrab 2023-05-17
* Remove TypeToSkill() from tradeskills.cpp/object.h ([#3355](https://github.com/EQEmu/Server/pull/3355)) @Kinglykrab 2023-05-17
* Remove Z_AGGRO from spdat.h ([#3365](https://github.com/EQEmu/Server/pull/3365)) @Kinglykrab 2023-05-25
* Remove numMembers from raids.h ([#3362](https://github.com/EQEmu/Server/pull/3362)) @Kinglykrab 2023-05-25
* Set GetAugmentType() to int again ([#3335](https://github.com/EQEmu/Server/pull/3335)) @Kinglykrab 2023-05-08

### Commands

* Add #findcurrency Command ([#3368](https://github.com/EQEmu/Server/pull/3368)) @Kinglykrab 2023-05-25
* Add entity variable command ([#3345](https://github.com/EQEmu/Server/pull/3345)) @Kinglykrab 2023-05-25
* Cleanup #setanim ([#3350](https://github.com/EQEmu/Server/pull/3350)) @Kinglykrab 2023-05-25

### Feature

* Intoxication setter/getter for source, getter for Perl/Lua ([#3330](https://github.com/EQEmu/Server/pull/3330)) @JasXSL 2023-05-03

### Fixes

* #augmentitem bypasses augment restrictions ([#3332](https://github.com/EQEmu/Server/pull/3332)) @Kinglykrab 2023-05-07
* Fix Heroic INT/WIS Bonuses ([#3341](https://github.com/EQEmu/Server/pull/3341)) @RekkasGit 2023-05-15
* Fix duplicate messages in #npcedit ([#3372](https://github.com/EQEmu/Server/pull/3372)) @Kinglykrab 2023-05-21
* Fix issue with Group Pointers/Member roles ([#3374](https://github.com/EQEmu/Server/pull/3374)) @Aeadoin 2023-05-25
* Fix mob item bonus calc ([#3334](https://github.com/EQEmu/Server/pull/3334)) @Akkadius 2023-05-07
* Fix typos in #zheader ([#3370](https://github.com/EQEmu/Server/pull/3370)) @Kinglykrab 2023-05-21
* Mob scaling issue with min dmg set to zero while max dmg is not ([#3351](https://github.com/EQEmu/Server/pull/3351)) @RekkasGit 2023-05-20
* NPC Armor Upgrade to a slot not handled correctly ([#3366](https://github.com/EQEmu/Server/pull/3366)) @noudess 2023-05-20
* ReloadQuests() on Zone::Init() to avoid cached global quests/plugins ([#3333](https://github.com/EQEmu/Server/pull/3333)) @Kinglykrab 2023-05-07
* Revert " ReloadQuests() on Zone::Init() to avoid cached global quests/plugins " ([#3333](https://github.com/EQEmu/Server/pull/3333)) @Akkadius 2023-05-08

### Memory Leak

* Fix large memory leak introduced in CalcItemBonuses ([#3331](https://github.com/EQEmu/Server/pull/3331)) @Akkadius 2023-05-07

### Messages

* Remove duplicate heal message for healing yourself ([#3329](https://github.com/EQEmu/Server/pull/3329)) @noudess 2023-05-03

### Performance

* Character bind is now bulk saved ([#3338](https://github.com/EQEmu/Server/pull/3338)) @Akkadius 2023-05-09
* Character buffs now save in bulk ([#3336](https://github.com/EQEmu/Server/pull/3336)) @Akkadius 2023-05-09
* Character pet bulk saving ([#3337](https://github.com/EQEmu/Server/pull/3337)) @Akkadius 2023-05-09
* Character tribute is now bulk saved ([#3340](https://github.com/EQEmu/Server/pull/3340)) @Kinglykrab 2023-05-25
* Mail key is now cached during player load ([#3339](https://github.com/EQEmu/Server/pull/3339)) @Akkadius 2023-05-09

### Pets

* Fix saving inconsistencies with pets ([#3375](https://github.com/EQEmu/Server/pull/3375)) @Akkadius 2023-05-25

### Quest API

* Add GetHateListClosest(), GetHateListClosestBot(), GetHateListClosestClient(), and GetHateListClosestNPC() methods/overloads to Perl/Lua ([#3359](https://github.com/EQEmu/Server/pull/3359)) @RekkasGit 2023-05-16
* Add GetPet() to Perl ([#3309](https://github.com/EQEmu/Server/pull/3309)) @Kinglykrab 2023-05-08
* Add Memorize and Scribe Spell Events to Perl/Lua ([#3363](https://github.com/EQEmu/Server/pull/3363)) @Kinglykrab 2023-05-25
* Add zone data methods to Perl/Lua ([#3342](https://github.com/EQEmu/Server/pull/3342)) @Kinglykrab 2023-05-24
* Cleanup The Darkened Sea Quest Methods Names ([#3369](https://github.com/EQEmu/Server/pull/3369)) @Kinglykrab 2023-05-21

### Rules

* Add World:MaximumQuestErrors Rule ([#3349](https://github.com/EQEmu/Server/pull/3349)) @Kinglykrab 2023-05-21
* ResurrectionEffectBlock to prevent/allow/move buffs. ([#3288](https://github.com/EQEmu/Server/pull/3288)) @nytmyr 2023-05-08

## [22.11.0] - 04/29/2023

### Code

* Add check for owner in quest::pausetimer() ([#3304](https://github.com/EQEmu/Server/pull/3304)) @Kinglykrab 2023-04-23
* Add check for owner in quest::resumetimer() ([#3305](https://github.com/EQEmu/Server/pull/3305)) @Kinglykrab 2023-04-23
* Add initiator/owner checks to various methods in questmgr.cpp ([#3306](https://github.com/EQEmu/Server/pull/3306)) @Kinglykrab 2023-04-23
* Fix possible nullptr in quest::addloot() ([#3303](https://github.com/EQEmu/Server/pull/3303)) @Kinglykrab 2023-04-23
* Remove GetClassHPFactor() from zone/client_mods.cpp and zone/client.h ([#3313](https://github.com/EQEmu/Server/pull/3313)) @Kinglykrab 2023-04-30
* Remove GetClassHPFactor() from zone/merc.h ([#3314](https://github.com/EQEmu/Server/pull/3314)) @Kinglykrab 2023-04-30
* Remove pDontCastBefore_casting_spell from zone/npc.h ([#3311](https://github.com/EQEmu/Server/pull/3311)) @Kinglykrab 2023-04-30
* Remove unused code in zone/pets.cpp ([#3310](https://github.com/EQEmu/Server/pull/3310)) @Kinglykrab 2023-04-30
* Remove unused methods in zone/bot.cpp and zone/bot.h ([#3315](https://github.com/EQEmu/Server/pull/3315)) @Kinglykrab 2023-04-30
* Remove unused methods in zone/client.cpp and zone/client.h ([#3312](https://github.com/EQEmu/Server/pull/3312)) @Kinglykrab 2023-04-30
* Remove unused variable in common/crash.cpp ([#3308](https://github.com/EQEmu/Server/pull/3308)) @Kinglykrab 2023-04-30
* Use default ctor/dtor in oriented_bounding_box.h ([#3307](https://github.com/EQEmu/Server/pull/3307)) @Kinglykrab 2023-04-30
* quest::createBot() unnecessary check against nullptr ([#3302](https://github.com/EQEmu/Server/pull/3302)) @Kinglykrab 2023-04-23
* quest::setallskill() had always true condition. ([#3301](https://github.com/EQEmu/Server/pull/3301)) @Kinglykrab 2023-04-30

### Crash

* Fix UCS crash that occurs during log reloading ([#3324](https://github.com/EQEmu/Server/pull/3324)) @Akkadius 2023-04-30
* Fix possible dereference of nullptr in Client::CalcHPRegen ([#3316](https://github.com/EQEmu/Server/pull/3316)) @Aeadoin 2023-04-23
* Fix possible nullptr in Client::GetCharMaxLevelFromQGlobal() ([#3317](https://github.com/EQEmu/Server/pull/3317)) @Kinglykrab 2023-04-23

### Discord

* Add Discord webhook callback processing to world ([#3322](https://github.com/EQEmu/Server/pull/3322)) @Akkadius 2023-04-30

### Fixes

* Fix issue with NPCs no longer using some armor. ([#3318](https://github.com/EQEmu/Server/pull/3318)) @noudess 2023-04-24
* Fix issue with spawning Mercs ([#3327](https://github.com/EQEmu/Server/pull/3327)) @Aeadoin 2023-04-29
* Possible issues with SummonItem in Client::QuestReward() methods ([#3325](https://github.com/EQEmu/Server/pull/3325)) @Kinglykrab 2023-04-27

### Maps

* Update download with faster releases link ([#3321](https://github.com/EQEmu/Server/pull/3321)) @Akkadius 2023-04-30

### Messages

* Remove duplicate you have lost a level message ([#3323](https://github.com/EQEmu/Server/pull/3323)) @noudess 2023-04-25

### Quest API

* Add GetDefaultRaceSize() overloads to Perl/Lua ([#3320](https://github.com/EQEmu/Server/pull/3320)) @Kinglykrab 2023-04-30
* Add HasSpellEffect() to Perl/Lua ([#3319](https://github.com/EQEmu/Server/pull/3319)) @Kinglykrab 2023-04-30

## [22.10.0] - 04/22/2023

### Backups

* Fix database dump error reporting ([#3175](https://github.com/EQEmu/Server/pull/3175)) @Akkadius 2023-04-04

### Bots

* Cleanup GetBotTables() ([#3270](https://github.com/EQEmu/Server/pull/3270)) @Aeadoin 2023-04-06

### Cleanuo

* Only define row if we have results in Database::GetCharacterID() ([#3199](https://github.com/EQEmu/Server/pull/3199)) @Kinglykrab 2023-04-05

### Code

* Add missing breaks and returns in bonuses.cpp ([#3231](https://github.com/EQEmu/Server/pull/3231)) @Kinglykrab 2023-04-05
* Breaks in wrong spot in cases in spell_effects.cpp ([#3297](https://github.com/EQEmu/Server/pull/3297)) @Kinglykrab 2023-04-22
* Cleanup always true/false statements in shareddb.cpp ([#3189](https://github.com/EQEmu/Server/pull/3189)) @Kinglykrab 2023-04-05
* Cleanup cheap-to-copy reference to use value instead in eq_stream_ident.cpp/eq_stream_ident.h ([#3209](https://github.com/EQEmu/Server/pull/3209)) @Kinglykrab 2023-04-05
* Cleanup discord.cpp and discord_manager.cpp ([#3205](https://github.com/EQEmu/Server/pull/3205)) @Kinglykrab 2023-04-05
* Cleanup duplicate conditions in negate bonuses in bonuses.cpp ([#3226](https://github.com/EQEmu/Server/pull/3226)) @Kinglykrab 2023-04-05
* Cleanup item_instance.cpp always true statements and reassigning of same values ([#3187](https://github.com/EQEmu/Server/pull/3187)) @Kinglykrab 2023-04-05
* Cleanup macros in features.h ([#3185](https://github.com/EQEmu/Server/pull/3185)) @Kinglykrab 2023-04-05
* Cleanup string -> char* -> string conversions in bot_command.cpp ([#3252](https://github.com/EQEmu/Server/pull/3252)) @Kinglykrab 2023-04-05
* Cleanup unnecessary condition in Client::SendAlternateCurrencyValue() ([#3266](https://github.com/EQEmu/Server/pull/3266)) @Kinglykrab 2023-04-05
* Cleanup zone/zoning.cpp ([#3289](https://github.com/EQEmu/Server/pull/3289)) @Kinglykrab 2023-04-14
* Combine similar cases in Client::InitInnates() ([#3260](https://github.com/EQEmu/Server/pull/3260)) @Kinglykrab 2023-04-05
* Convert equipable_slot_list to std::vector from std::list in bot_command.cpp ([#3253](https://github.com/EQEmu/Server/pull/3253)) @Kinglykrab 2023-04-05
* Delete embxs.cpp/embxs.h ([#3284](https://github.com/EQEmu/Server/pull/3284)) @Kinglykrab 2023-04-14
* Delete unused strings in bot_command.cpp ([#3251](https://github.com/EQEmu/Server/pull/3251)) @Kinglykrab 2023-04-05
* Fix GetLastName() length check in Client::SendWindow() ([#3263](https://github.com/EQEmu/Server/pull/3263)) @Kinglykrab 2023-04-05
* Fix always false conditions in Client::IncStats() ([#3256](https://github.com/EQEmu/Server/pull/3256)) @Kinglykrab 2023-04-05
* Fix check for !this in Client::SendHPUpdateMarquee() ([#3257](https://github.com/EQEmu/Server/pull/3257)) @Kinglykrab 2023-04-05
* Fix filter condition in attack.cpp ([#3218](https://github.com/EQEmu/Server/pull/3218)) @Kinglykrab 2023-04-05
* Fix ornamentation augment icons in inspect requests ([#3264](https://github.com/EQEmu/Server/pull/3264)) @Kinglykrab 2023-04-05
* Fix possible nullptr inst in GetSharedBank() ([#3190](https://github.com/EQEmu/Server/pull/3190)) @Kinglykrab 2023-04-08
* Fix possible overflows in Client::AddPlatinum() and Client::TakePlatinum() ([#3255](https://github.com/EQEmu/Server/pull/3255)) @Kinglykrab 2023-04-05
* Fix shared_tasks.cpp/shared_tasks.cpp variable named same as class member ([#3192](https://github.com/EQEmu/Server/pull/3192)) @Kinglykrab 2023-04-05
* Fix skill_used being used as boolean in Mob::CommonDamage() ([#3220](https://github.com/EQEmu/Server/pull/3220)) @Kinglykrab 2023-04-05
* Identical conditions right beside each other in aa.cpp ([#3213](https://github.com/EQEmu/Server/pull/3213)) @Kinglykrab 2023-04-05
* Move unreachable code in ApplySpellsBonuses() ([#3229](https://github.com/EQEmu/Server/pull/3229)) @Kinglykrab 2023-04-05
* Move variable definition to more relevant scope in DatabaseDumpService::Dump() ([#3200](https://github.com/EQEmu/Server/pull/3200)) @Kinglykrab 2023-04-05
* Multiple cases with same outcome in GetDiscordPayloadFromEvent() ([#3184](https://github.com/EQEmu/Server/pull/3184)) @Kinglykrab 2023-04-05
* Remove ExportVarComplex() from embparser.cpp/embparser.h ([#3282](https://github.com/EQEmu/Server/pull/3282)) @Kinglykrab 2023-04-14
* Remove GetQGlobal() from qglobals.cpp/qglobals.h ([#3285](https://github.com/EQEmu/Server/pull/3285)) @Kinglykrab 2023-04-14
* Remove IsFullHP from mob.cpp/mob.h ([#3277](https://github.com/EQEmu/Server/pull/3277)) @Kinglykrab 2023-04-14
* Remove IsMeleeDmg() from skills.cpp/skills.h ([#3279](https://github.com/EQEmu/Server/pull/3279)) @Kinglykrab 2023-04-14
* Remove _GetMovementSpeed() from mob.h ([#3276](https://github.com/EQEmu/Server/pull/3276)) @Kinglykrab 2023-04-14
* Remove always true condition in Strings::Commify() ([#3193](https://github.com/EQEmu/Server/pull/3193)) @Kinglykrab 2023-04-05
* Remove always true conditions and unreachable code in Client::SendMercPersonalInfo() ([#3258](https://github.com/EQEmu/Server/pull/3258)) @Kinglykrab 2023-04-05
* Remove always true statement in say_link.cpp ([#3188](https://github.com/EQEmu/Server/pull/3188)) @Kinglykrab 2023-04-05
* Remove always true statements in task_client_state.cpp ([#3292](https://github.com/EQEmu/Server/pull/3292)) @Kinglykrab 2023-04-14
* Remove always true/false conditions from bot.cpp ([#3237](https://github.com/EQEmu/Server/pull/3237)) @Kinglykrab 2023-04-05
* Remove bool return from GetSharedPlatinum() ([#3191](https://github.com/EQEmu/Server/pull/3191)) @Kinglykrab 2023-04-05
* Remove extra assignment of current_endurance in Client ctor ([#3261](https://github.com/EQEmu/Server/pull/3261)) @Kinglykrab 2023-04-05
* Remove extraneous check for NegateAttacks in SE_NegateAttacks ([#3228](https://github.com/EQEmu/Server/pull/3228)) @Kinglykrab 2023-04-05
* Remove extraneous parentheses around math in Mob::ApplySpellsBonuses() ([#3227](https://github.com/EQEmu/Server/pull/3227)) @Kinglykrab 2023-04-05
* Remove getd(), geti(), InUse(), lasterr(), my_get_sv(), and VarExists() in embperl.cpp/embperl.h ([#3283](https://github.com/EQEmu/Server/pull/3283)) @Kinglykrab 2023-04-15
* Remove item_timers from questmgr.cpp/questmgr.h ([#3286](https://github.com/EQEmu/Server/pull/3286)) @Kinglykrab 2023-04-14
* Remove pendinggroup from mob.h ([#3278](https://github.com/EQEmu/Server/pull/3278)) @Kinglykrab 2023-04-14
* Remove position_same_update_count from client.cpp/client.h ([#3280](https://github.com/EQEmu/Server/pull/3280)) @Kinglykrab 2023-04-14
* Remove unnecessary break in while loop in Mob::AddToHateList() ([#3219](https://github.com/EQEmu/Server/pull/3219)) @Kinglykrab 2023-04-05
* Remove unnecessary check for IsStackable() in DeleteItem() ([#3186](https://github.com/EQEmu/Server/pull/3186)) @Kinglykrab 2023-04-05
* Remove unnecessary condition and cleanup variable name in tasks.cpp ([#3293](https://github.com/EQEmu/Server/pull/3293)) @Kinglykrab 2023-04-14
* Remove unnecessary conditions in Client::Consume() ([#3265](https://github.com/EQEmu/Server/pull/3265)) @Kinglykrab 2023-04-05
* Remove unnecessary conditions in Client::FilteredMessageCheck() ([#3262](https://github.com/EQEmu/Server/pull/3262)) @Kinglykrab 2023-04-05
* Remove unnecessary conditions in Client::SendFactionMessage() ([#3267](https://github.com/EQEmu/Server/pull/3267)) @Kinglykrab 2023-04-05
* Remove unnecessary setting of reuse variable in Bot::DoClassAttacks() ([#3233](https://github.com/EQEmu/Server/pull/3233)) @Kinglykrab 2023-04-05
* Remove unused SetConfigFile in common/eqemu_config.h ([#3208](https://github.com/EQEmu/Server/pull/3208)) @Kinglykrab 2023-04-05
* Remove unused code in eq_packet.cpp/eq_packet.h ([#3183](https://github.com/EQEmu/Server/pull/3183)) @Kinglykrab 2023-04-05
* Remove unused ctor and use default dtor in xtargetautohaters.h ([#3290](https://github.com/EQEmu/Server/pull/3290)) @Kinglykrab 2023-04-14
* Remove unused macros in common/types.h ([#3194](https://github.com/EQEmu/Server/pull/3194)) @Kinglykrab 2023-04-05
* Remove unused variable in Database::CopyCharacter() ([#3197](https://github.com/EQEmu/Server/pull/3197)) @Kinglykrab 2023-04-05
* Remove unused variables and use reference in task_manager.cpp ([#3291](https://github.com/EQEmu/Server/pull/3291)) @Kinglykrab 2023-04-14
* SE_AttackSpeed3 effect_value is always less than 0 ([#3222](https://github.com/EQEmu/Server/pull/3222)) @Kinglykrab 2023-04-05
* SE_StrikeThrough and SE_StrikeThrough2 are the same in bonuses.cpp ([#3223](https://github.com/EQEmu/Server/pull/3223)) @Kinglykrab 2023-04-05
* Set bonuses to use spell ID instead of boolean ([#3230](https://github.com/EQEmu/Server/pull/3230)) @Kinglykrab 2023-04-05
* Use .clear() and .empty() instead of comparing to empty string or setting to empty string in CheckDatabaseConvertPPBlob() ([#3201](https://github.com/EQEmu/Server/pull/3201)) @Kinglykrab 2023-04-05
* Use .clear() instead of setting string to empty in eqemu_command_handler.cpp ([#3195](https://github.com/EQEmu/Server/pull/3195)) @Kinglykrab 2023-04-05
* Use .empty() in Client::ScribeSpells() and Client::LearnDisciplines() ([#3259](https://github.com/EQEmu/Server/pull/3259)) @Kinglykrab 2023-04-05
* Use constant reference and check for empty string properly in dbcore.cpp ([#3203](https://github.com/EQEmu/Server/pull/3203)) @Kinglykrab 2023-04-05
* Use default ctor instead of an empty ctor. ([#3206](https://github.com/EQEmu/Server/pull/3206)) @Kinglykrab 2023-04-05
* Use default dtor instead of empty dtor for EQTime in eqtime.cpp/eqtime.h ([#3210](https://github.com/EQEmu/Server/pull/3210)) @Kinglykrab 2023-04-05
* Use variable for c->GetTarget() instead of calling multiple times in bot_command.cpp ([#3254](https://github.com/EQEmu/Server/pull/3254)) @Kinglykrab 2023-04-05
* Use variable for character instead of a loop ([#3268](https://github.com/EQEmu/Server/pull/3268)) @Kinglykrab 2023-04-05
* Utilize .empty() instead of checking for an empty string in Database::ReserveName() ([#3198](https://github.com/EQEmu/Server/pull/3198)) @Kinglykrab 2023-04-05
* Utilize IsTaunting(), SetPetPower(), SetPetType(), and SetTaunting() ([#3275](https://github.com/EQEmu/Server/pull/3275)) @Kinglykrab 2023-04-15
* Validate for nullptrs in bot.cpp ([#3232](https://github.com/EQEmu/Server/pull/3232)) @Kinglykrab 2023-04-05
* other is always defined in these cases in attack.cpp ([#3217](https://github.com/EQEmu/Server/pull/3217)) @Kinglykrab 2023-04-05
* results variable is assigned but never used in SaveCharacterCreate() ([#3180](https://github.com/EQEmu/Server/pull/3180)) @Kinglykrab 2023-04-05

### Crash

* Add additional raid integrity checks on Bot Spawn. ([#3295](https://github.com/EQEmu/Server/pull/3295)) @Aeadoin 2023-04-16
* Fix crash with uninitialized item instance, and Bot timeout ([#3296](https://github.com/EQEmu/Server/pull/3296)) @Aeadoin 2023-04-15
* Resolve crash due to uninitialized pointer. ([#3271](https://github.com/EQEmu/Server/pull/3271)) @Aeadoin 2023-04-08

### Feature

* Make ornamentations work with any augment type ([#3281](https://github.com/EQEmu/Server/pull/3281)) @Kinglykrab 2023-04-16

### Fixes

* Camping was causing player to leave raid, causing unexpected behavior ([#3299](https://github.com/EQEmu/Server/pull/3299)) @Aeadoin 2023-04-22
* Resolve loading of inventory ([#3272](https://github.com/EQEmu/Server/pull/3272)) @Aeadoin 2023-04-08

### Quest API

* Add ApplySpellRaid() and SetSpellDurationRaid() to Bots in Perl/Lua ([#3274](https://github.com/EQEmu/Server/pull/3274)) @Kinglykrab 2023-04-09
* Add GetBuffSpellIDs() to Perl/Lua ([#3273](https://github.com/EQEmu/Server/pull/3273)) @Kinglykrab 2023-04-09
* Fix LDoN Methods in Perl/Lua ([#3287](https://github.com/EQEmu/Server/pull/3287)) @Kinglykrab 2023-04-10

### Rules

* Optional summoning when already in melee range ([#3204](https://github.com/EQEmu/Server/pull/3204)) @trentdm 2023-04-08

### Telnet

* Telnet encoding fix ([#3269](https://github.com/EQEmu/Server/pull/3269)) @Akkadius 2023-04-05

## [22.9.1] - 04/03/2023

### Code

* Add client pointer validation to Zone::GetClosestZonePoint() ([#3173](https://github.com/EQEmu/Server/pull/3173)) @Kinglykrab 2023-04-01
* Change level to bot_level in Bot::DoClassAttacks() to not overlap member variable ([#3239](https://github.com/EQEmu/Server/pull/3239)) @Kinglykrab 2023-04-03
* Cleanup unnecessary string -> char* -> string conversions in eqemu_config.cpp ([#3207](https://github.com/EQEmu/Server/pull/3207)) @Kinglykrab 2023-04-03
* Cleanup uses of insert/push_back when a temp object is used. ([#3170](https://github.com/EQEmu/Server/pull/3170)) @Aeadoin 2023-04-03
* Cleanup variable names in Bot::AddSpellToBotList() ([#3248](https://github.com/EQEmu/Server/pull/3248)) @Kinglykrab 2023-04-03
* Explicitly cast to float for more precision in Bot::GenerateBastHitPoints() ([#3238](https://github.com/EQEmu/Server/pull/3238)) @Kinglykrab 2023-04-03
* Fix SEResist array settings duplicate code ([#3225](https://github.com/EQEmu/Server/pull/3225)) @Kinglykrab 2023-04-03
* Fix loop and code duplication for SE_ProcOnKillShot ([#3224](https://github.com/EQEmu/Server/pull/3224)) @Kinglykrab 2023-04-03
* Fix possible dereferencing of invalid iterator in constants ([#3181](https://github.com/EQEmu/Server/pull/3181)) @Kinglykrab 2023-04-03
* Fix typo where itembonuses should have been used instead of spellbonuses ([#3221](https://github.com/EQEmu/Server/pull/3221)) @Kinglykrab 2023-04-03
* Further bot.cpp nullptr checks ([#3240](https://github.com/EQEmu/Server/pull/3240)) @Kinglykrab 2023-04-03
* Move cases in Bot::AICastSpell() ([#3247](https://github.com/EQEmu/Server/pull/3247)) @Kinglykrab 2023-04-03
* Multiple cases same outcome and set skip variable to same value ([#3216](https://github.com/EQEmu/Server/pull/3216)) @Kinglykrab 2023-04-03
* Multiple cases with same outcome in GetGMSayColorFromCategory() ([#3182](https://github.com/EQEmu/Server/pull/3182)) @Kinglykrab 2023-04-03
* Remove extraneous loottable_id setting in WakeTheDead in aa.cpp ([#3215](https://github.com/EQEmu/Server/pull/3215)) @Kinglykrab 2023-04-03
* Remove possible dereferenced nullptrs in bot.cpp ([#3241](https://github.com/EQEmu/Server/pull/3241)) @Kinglykrab 2023-04-03
* Remove unnecessary >= 0 checks for procs in botspellsai.cpp ([#3242](https://github.com/EQEmu/Server/pull/3242)) @Kinglykrab 2023-04-03
* Remove unnecessary botCaster check in Bot::GetDebuffBotSpell() ([#3246](https://github.com/EQEmu/Server/pull/3246)) @Kinglykrab 2023-04-03
* Remove unnecessary group validation in Bot::Death() ([#3235](https://github.com/EQEmu/Server/pull/3235)) @Kinglykrab 2023-04-03
* Remove unnecessary hpr checks in Bot::BotCastHeal() ([#3245](https://github.com/EQEmu/Server/pull/3245)) @Kinglykrab 2023-04-03
* Remove unnecessary setting of spell_type_index in Bot::GetChanceToCastBySpellType() ([#3243](https://github.com/EQEmu/Server/pull/3243)) @Kinglykrab 2023-04-03
* Remove unnecessary skill_to_use check in Bot::DoClassAttacks() ([#3236](https://github.com/EQEmu/Server/pull/3236)) @Kinglykrab 2023-04-03
* Remove unnecessary spell_list validation check in botspellsai.cpp ([#3244](https://github.com/EQEmu/Server/pull/3244)) @Kinglykrab 2023-04-03
* Remove unnecessary validation check in Zone::ClearBlockedSpells() ([#3172](https://github.com/EQEmu/Server/pull/3172)) @Kinglykrab 2023-04-01
* Remove unused Includes under zone files ([#3162](https://github.com/EQEmu/Server/pull/3162)) @Aeadoin 2023-04-02
* Remove unused query variable in Database::DeleteInstance() ([#3202](https://github.com/EQEmu/Server/pull/3202)) @Kinglykrab 2023-04-03
* Unconditional return in for loop in GetRaidByCharID() ([#3179](https://github.com/EQEmu/Server/pull/3179)) @Kinglykrab 2023-04-03
* Use a constant reference for content_flags in SetContentFlags() ([#3196](https://github.com/EQEmu/Server/pull/3196)) @Kinglykrab 2023-04-03
* Wake The Dead argument was named the same as a member variable in Mob ([#3214](https://github.com/EQEmu/Server/pull/3214)) @Kinglykrab 2023-04-03
* gid is assigned 2 values simultaneously in bot.cpp ([#3234](https://github.com/EQEmu/Server/pull/3234)) @Kinglykrab 2023-04-03
* summon_count > MAX_SWARM_PETS  is always false in aa.cpp ([#3212](https://github.com/EQEmu/Server/pull/3212)) @Kinglykrab 2023-04-03

### Fixes

* Correct Forward Declaration compilation warning ([#3176](https://github.com/EQEmu/Server/pull/3176)) @Aeadoin 2023-04-02
* Fix issue with Bot Raid invites not working. ([#3249](https://github.com/EQEmu/Server/pull/3249)) @Aeadoin 2023-04-03

### Performance

* Change to use Pass by reference where valid. ([#3163](https://github.com/EQEmu/Server/pull/3163)) @Aeadoin 2023-04-02

## [22.9.0] - 04/01/2023

### Bots

* Remove Bot Groups Functionality ([#3165](https://github.com/EQEmu/Server/pull/3165)) @Aeadoin 2023-04-01

### Code

* Cleanup excessive type casting: string -> char * -> string ([#3169](https://github.com/EQEmu/Server/pull/3169)) @Aeadoin 2023-04-01

### Crash

* Add Checks for valid pointers or fix existing. ([#3164](https://github.com/EQEmu/Server/pull/3164)) @Aeadoin 2023-04-01
* Fix out of bound arrays, other potential crashes ([#3166](https://github.com/EQEmu/Server/pull/3166)) @Aeadoin 2023-04-01

### Fixes

* Correct SE_SlayUndead & SE_HeadShotLevel limit Value when applied. ([#3171](https://github.com/EQEmu/Server/pull/3171)) @Aeadoin 2023-04-01
* Prevent VerifyGroup from setting OOZ membername to Null character. ([#3168](https://github.com/EQEmu/Server/pull/3168)) @Aeadoin 2023-04-01

### Quest API

* Add missing Luabind definitions to lua_general.cpp ([#3167](https://github.com/EQEmu/Server/pull/3167)) @Kinglykrab 2023-04-01

## [22.8.2] - 03/30/2023

### Code

* "equipped" not "equiped", "dual" not "duel". ([#3149](https://github.com/EQEmu/Server/pull/3149)) @Kinglykrab 2023-03-27

### Crash

* Add Checks for out of bounds & dereferencing nullptrs ([#3151](https://github.com/EQEmu/Server/pull/3151)) @Aeadoin 2023-03-28

### Fixes

* Check Rule "Bots Enabled" to prevent bot database calls on connect ([#3154](https://github.com/EQEmu/Server/pull/3154)) @Aeadoin 2023-03-29
* Correct logic checks for Bot rule AllowOwnerOptionAltCombat ([#3158](https://github.com/EQEmu/Server/pull/3158)) @Aeadoin 2023-03-30
* Fix an issue with EVENT_DISCONNECT not firing on regular /camp ([#3153](https://github.com/EQEmu/Server/pull/3153)) @Kinglykrab 2023-03-28
* Fix bot_raid_members.sql for MYSQL. ([#3155](https://github.com/EQEmu/Server/pull/3155)) @Aeadoin 2023-03-28
* Fix for OOZ Group updates when removing/inviting Bots ([#3159](https://github.com/EQEmu/Server/pull/3159)) @Aeadoin 2023-03-30
* Fix issues with Lua tables not starting at index 1 ([#3160](https://github.com/EQEmu/Server/pull/3160)) @Kinglykrab 2023-03-30
* Fix strcpy-param-overlap ([#3157](https://github.com/EQEmu/Server/pull/3157)) @Aeadoin 2023-03-29

### Rules

* Remove Guild Bank Zone ID Rule ([#3156](https://github.com/EQEmu/Server/pull/3156)) @Kinglykrab 2023-03-29

## [22.8.1] - 03/27/2023

### Fixes

* Fix for NPCs having spells interrupted. ([#3150](https://github.com/EQEmu/Server/pull/3150)) @Aeadoin 2023-03-27

## [22.8.0] - 03/25/2023

### Code

* Cleanup Strings::ToInt uses. ([#3142](https://github.com/EQEmu/Server/pull/3142)) @Aeadoin 2023-03-26
* Remove extern bool Critical ([#3146](https://github.com/EQEmu/Server/pull/3146)) @Kinglykrab 2023-03-25

### Crash

* Fix for crash in Raid::QueuePacket ([#3145](https://github.com/EQEmu/Server/pull/3145)) @Aeadoin 2023-03-25

### Feature

* Add support for -1 extradmgskill to allow all skills to be scaled. ([#3136](https://github.com/EQEmu/Server/pull/3136)) @Kinglykrab 2023-03-26

### Fixes

* Fix for Items looted from corpses. ([#3147](https://github.com/EQEmu/Server/pull/3147)) @Aeadoin 2023-03-26
* Fix for SQL Query in npc_scale_global_base ([#3144](https://github.com/EQEmu/Server/pull/3144)) @Aeadoin 2023-03-26

## [22.7.0] - 03/24/2023

### Bots

* Place BOT_COMMAND_CHAR inside messages ([#3027](https://github.com/EQEmu/Server/pull/3027)) @trentdm 2023-03-05
* Prevent interrupt spam when OOM ([#3011](https://github.com/EQEmu/Server/pull/3011)) @nytmyr 2023-03-07

### Code

* Cleaning up Raid.cpp ([#3125](https://github.com/EQEmu/Server/pull/3125)) @Aeadoin 2023-03-20
* Cleanup unused methods and variables in world/main.cpp and world/main.h ([#3105](https://github.com/EQEmu/Server/pull/3105)) @Kinglykrab 2023-03-17
* Cleanup uses of Strings::ToInt to match correct type. ([#3054](https://github.com/EQEmu/Server/pull/3054)) @Aeadoin 2023-03-22
* Delete deprecated/perlxs folder ([#3110](https://github.com/EQEmu/Server/pull/3110)) @Kinglykrab 2023-03-17
* Delete queues.h ([#3089](https://github.com/EQEmu/Server/pull/3089)) @Kinglykrab 2023-03-17
* Delete world/console.old.cpp ([#3099](https://github.com/EQEmu/Server/pull/3099)) @Kinglykrab 2023-03-17
* Delete zone_numbers.h ([#3129](https://github.com/EQEmu/Server/pull/3129)) @Kinglykrab 2023-03-20
* Remove AllConnected(), CanUpdate(), and SendInfo() from login_server_list.cpp and login_server_list.h ([#3104](https://github.com/EQEmu/Server/pull/3104)) @Kinglykrab 2023-03-17
* Remove CLIENT_TIMEOUT from world/client.h and zone/client.h ([#3071](https://github.com/EQEmu/Server/pull/3071)) @Kinglykrab 2023-03-17
* Remove ChangeHP() from mob.h ([#3128](https://github.com/EQEmu/Server/pull/3128)) @Kinglykrab 2023-03-19
* Remove CheckAuth(), SetOnline(), and pMD5Pass from cliententry.h ([#3095](https://github.com/EQEmu/Server/pull/3095)) @Kinglykrab 2023-03-17
* Remove CommandRequirement() from zonedb.h ([#3094](https://github.com/EQEmu/Server/pull/3094)) @Kinglykrab 2023-03-17
* Remove CountZones() from launcher_link.h ([#3100](https://github.com/EQEmu/Server/pull/3100)) @Kinglykrab 2023-03-17
* Remove DBInitVars() and HandleMysqlError() from queryserv/database.h ([#3114](https://github.com/EQEmu/Server/pull/3114)) @Kinglykrab 2023-03-17
* Remove DBInitVars(), HandleMysqlError(), and IsChatChannelInDB() in ucs/database.h ([#3113](https://github.com/EQEmu/Server/pull/3113)) @Kinglykrab 2023-03-17
* Remove DisableStats(), EnableStats(), DisableLoginserver(), and EnableLoginserver() from world_config.h ([#3107](https://github.com/EQEmu/Server/pull/3107)) @Kinglykrab 2023-03-17
* Remove DoBuffWearOffEffect() from mob.h ([#3062](https://github.com/EQEmu/Server/pull/3062)) @Kinglykrab 2023-03-17
* Remove FindByName(charname) from clientlist.h ([#3096](https://github.com/EQEmu/Server/pull/3096)) @Kinglykrab 2023-03-17
* Remove FindCLEByLSID(), GetCLE(), GetCLEIPCount(), and RemoveCLEByLSID() from clientlist.h ([#3098](https://github.com/EQEmu/Server/pull/3098)) @Kinglykrab 2023-03-17
* Remove FindPatch() from struct_category.cpp and struct_category.h ([#3130](https://github.com/EQEmu/Server/pull/3130)) @Kinglykrab 2023-03-20
* Remove FlushLootStats() from npc.h ([#3079](https://github.com/EQEmu/Server/pull/3079)) @Kinglykrab 2023-03-17
* Remove GetAILevel() from npc.h ([#3080](https://github.com/EQEmu/Server/pull/3080)) @Kinglykrab 2023-03-17
* Remove GetDestination() from doors.h ([#3078](https://github.com/EQEmu/Server/pull/3078)) @Kinglykrab 2023-03-17
* Remove GetServerByAddress() from server_manager.h ([#3119](https://github.com/EQEmu/Server/pull/3119)) @Kinglykrab 2023-03-17
* Remove GetStartCount() and InitStartTimer() from zone_launch.cpp and zone_launch.h ([#3121](https://github.com/EQEmu/Server/pull/3121)) @Kinglykrab 2023-03-17
* Remove GetTransformation() and GetInvertedTransformation() from oriented_bounding_box.h ([#3084](https://github.com/EQEmu/Server/pull/3084)) @Kinglykrab 2023-03-17
* Remove IsAffectedByBuff() ([#3068](https://github.com/EQEmu/Server/pull/3068)) @Kinglykrab 2023-03-17
* Remove IsConnected() from loginserver/database.h ([#3117](https://github.com/EQEmu/Server/pull/3117)) @Kinglykrab 2023-03-17
* Remove IsOrigin(glm::vec2) from position.h ([#3088](https://github.com/EQEmu/Server/pull/3088)) @Kinglykrab 2023-03-17
* Remove MakeGuildMembers() from wguild_mgr.h ([#3106](https://github.com/EQEmu/Server/pull/3106)) @Kinglykrab 2023-03-17
* Remove PlayerLogin_Struct from login_types.h ([#3118](https://github.com/EQEmu/Server/pull/3118)) @Kinglykrab 2023-03-17
* Remove RemoveSpawnGroup() from spawngroup.h ([#3090](https://github.com/EQEmu/Server/pull/3090)) @Kinglykrab 2023-03-17
* Remove SendGuildPacket() from clientlist.cpp, clientlist.h, and wguild_mgr.cpp ([#3097](https://github.com/EQEmu/Server/pull/3097)) @Kinglykrab 2023-03-17
* Remove SetConnection() from loginserver/world_server.h ([#3120](https://github.com/EQEmu/Server/pull/3120)) @Kinglykrab 2023-03-17
* Remove SetDBID() from object.h ([#3082](https://github.com/EQEmu/Server/pull/3082)) @Kinglykrab 2023-03-17
* Remove SetSentTime2 in petitions.h ([#3086](https://github.com/EQEmu/Server/pull/3086)) @Kinglykrab 2023-03-17
* Remove StoreCharacter() from worlddb.h ([#3108](https://github.com/EQEmu/Server/pull/3108)) @Kinglykrab 2023-03-17
* Remove UpdateLoginserverWorldAdminAccountPasswordById() from account_management.cpp ([#3115](https://github.com/EQEmu/Server/pull/3115)) @Kinglykrab 2023-03-17
* Remove _baseBotStance from bot.h ([#3076](https://github.com/EQEmu/Server/pull/3076)) @Kinglykrab 2023-03-17
* Remove _botRole from bot.h ([#3075](https://github.com/EQEmu/Server/pull/3075)) @Kinglykrab 2023-03-17
* Remove _previousTarget from bot.h ([#3074](https://github.com/EQEmu/Server/pull/3074)) @Kinglykrab 2023-03-17
* Remove authenticated from launcher_link.cpp and launcher_link.h ([#3101](https://github.com/EQEmu/Server/pull/3101)) @Kinglykrab 2023-03-17
* Remove can_corpse_be_rezzed from corpse.h ([#3077](https://github.com/EQEmu/Server/pull/3077)) @Kinglykrab 2023-03-17
* Remove casting_spell_type from mob.h ([#3064](https://github.com/EQEmu/Server/pull/3064)) @Kinglykrab 2023-03-17
* Remove class EQStream from client.h ([#3070](https://github.com/EQEmu/Server/pull/3070)) @Kinglykrab 2023-03-17
* Remove current_buff_count ([#3067](https://github.com/EQEmu/Server/pull/3067)) @Kinglykrab 2023-03-17
* Remove firstlogin and realfirstlogin from world/client.h ([#3072](https://github.com/EQEmu/Server/pull/3072)) @Kinglykrab 2023-03-17
* Remove fixedZ from mob.h ([#3065](https://github.com/EQEmu/Server/pull/3065)) @Kinglykrab 2023-03-17
* Remove inWater from mob.h ([#3069](https://github.com/EQEmu/Server/pull/3069)) @Kinglykrab 2023-03-17
* Remove is_authenticatd, LSShutDownUpdate(), and SetInstanceID() from zoneserver.h ([#3109](https://github.com/EQEmu/Server/pull/3109)) @Kinglykrab 2023-03-17
* Remove last_insert_id from petitions.h ([#3087](https://github.com/EQEmu/Server/pull/3087)) @Kinglykrab 2023-03-17
* Remove last_max_hp from mob.h ([#3063](https://github.com/EQEmu/Server/pull/3063)) @Kinglykrab 2023-03-17
* Remove m_inuse, m_z, and m_heading from object.h ([#3083](https://github.com/EQEmu/Server/pull/3083)) @Kinglykrab 2023-03-17
* Remove npc_ai.cpp/npc_ai.cpp ([#3081](https://github.com/EQEmu/Server/pull/3081)) @Kinglykrab 2023-03-17
* Remove ownHiddenTrigger from trap.cpp and trap.h ([#3092](https://github.com/EQEmu/Server/pull/3092)) @Kinglykrab 2023-03-17
* Remove perlparser.h ([#3085](https://github.com/EQEmu/Server/pull/3085)) @Kinglykrab 2023-03-17
* Remove unused AbilityTimer variable in client.h ([#3035](https://github.com/EQEmu/Server/pull/3035)) @Kinglykrab 2023-03-05
* Remove unused BotAA struct in bot_structs.h ([#3038](https://github.com/EQEmu/Server/pull/3038)) @Kinglykrab 2023-03-05
* Remove unused HandleUpdateTasksOnKill in client.h ([#3032](https://github.com/EQEmu/Server/pull/3032)) @Kinglykrab 2023-03-05
* Remove unused SaveBackup in client.h ([#3030](https://github.com/EQEmu/Server/pull/3030)) @Kinglykrab 2023-03-05
* Remove unused ^evacuate and ^succor subcommands from bot_command.h ([#3039](https://github.com/EQEmu/Server/pull/3039)) @Kinglykrab 2023-03-05
* Remove unused bot structs in bot_structs.h ([#3037](https://github.com/EQEmu/Server/pull/3037)) @Kinglykrab 2023-03-05
* Remove unused client queued work variable in client.cpp/client.h ([#3034](https://github.com/EQEmu/Server/pull/3034)) @Kinglykrab 2023-03-05
* Remove unused command variables in client.cpp ([#3031](https://github.com/EQEmu/Server/pull/3031)) @Kinglykrab 2023-03-05
* Remove unused lua_hate_entry.cpp ([#3057](https://github.com/EQEmu/Server/pull/3057)) @Kinglykrab 2023-03-12
* Remove unused methods in eql_config.cpp, eql_config.h, launcher_list.cpp, and launcher_list.h ([#3103](https://github.com/EQEmu/Server/pull/3103)) @Kinglykrab 2023-03-17
* Remove unused methods in loginserver/client.h ([#3116](https://github.com/EQEmu/Server/pull/3116)) @Kinglykrab 2023-03-17
* Remove unused player update variables in client.cpp/client.h ([#3033](https://github.com/EQEmu/Server/pull/3033)) @Kinglykrab 2023-03-05
* Utilize GetPlayerState() in mob methods ([#3066](https://github.com/EQEmu/Server/pull/3066)) @Kinglykrab 2023-03-17
* Utilize GetScheduler() in zone/worldserver.cpp ([#3093](https://github.com/EQEmu/Server/pull/3093)) @Kinglykrab 2023-03-17
* Utilize SetFilter in client.cpp ([#3036](https://github.com/EQEmu/Server/pull/3036)) @Kinglykrab 2023-03-05
* Utilize SetHiddenTrigger in trap.cpp ([#3091](https://github.com/EQEmu/Server/pull/3091)) @Kinglykrab 2023-03-17
* remove _botOrderAttack from bot.h ([#3073](https://github.com/EQEmu/Server/pull/3073)) @Kinglykrab 2023-03-17

### Commands

* Cleanup #haste Command ([#3042](https://github.com/EQEmu/Server/pull/3042)) @Kinglykrab 2023-03-06
* Cleanup #hideme Command ([#3043](https://github.com/EQEmu/Server/pull/3043)) @Kinglykrab 2023-03-06
* Cleanup #interrupt Command ([#3044](https://github.com/EQEmu/Server/pull/3044)) @Kinglykrab 2023-03-06
* Cleanup #level Command ([#3045](https://github.com/EQEmu/Server/pull/3045)) @Kinglykrab 2023-03-06
* Cleanup #picklock Command ([#3046](https://github.com/EQEmu/Server/pull/3046)) @Kinglykrab 2023-03-06
* Cleanup #resetaa and #resetaa_timer ([#3047](https://github.com/EQEmu/Server/pull/3047)) @Kinglykrab 2023-03-06
* Cleanup #wc Command ([#3049](https://github.com/EQEmu/Server/pull/3049)) @Kinglykrab 2023-03-06
* Remove #equipitem Command ([#3040](https://github.com/EQEmu/Server/pull/3040)) @Kinglykrab 2023-03-06

### Console

* Add IS_TTY to force terminal coloring output ([#3021](https://github.com/EQEmu/Server/pull/3021)) @Akkadius 2023-03-04

### Crash

* Fix dangling Group member pointers for Bots. ([#3134](https://github.com/EQEmu/Server/pull/3134)) @Aeadoin 2023-03-21
* Fixes Crash when Zoning with XTarget when Bots are in group. ([#3126](https://github.com/EQEmu/Server/pull/3126)) @Aeadoin 2023-03-19

### Feature

* Add Basic Bot Raiding Functionality ([#2782](https://github.com/EQEmu/Server/pull/2782)) @neckkola 2023-03-17
* Add Data Bucket support for scaling of Heroic Stats. ([#3058](https://github.com/EQEmu/Server/pull/3058)) @Aeadoin 2023-03-24
* Add Item Extra Skill Damage Percent Modifier ([#3127](https://github.com/EQEmu/Server/pull/3127)) @Kinglykrab 2023-03-19

### Fixes

* Add Avoidance and HP Regen Per Second too NPC Scaling. ([#3050](https://github.com/EQEmu/Server/pull/3050)) @Aeadoin 2023-03-09
* Add Heroic Strikethrough & HP Regen Per Second to GM Entity Info ([#3055](https://github.com/EQEmu/Server/pull/3055)) @Aeadoin 2023-03-12
* Add Heroic Strikethrough to NPC Scaling ([#3028](https://github.com/EQEmu/Server/pull/3028)) @Kinglykrab 2023-03-06
* Change SPA 193 Weapon Damage to allow values over 65,535 ([#3138](https://github.com/EQEmu/Server/pull/3138)) @Aeadoin 2023-03-23
* Checkmarks and X characters in popup messages ([#3041](https://github.com/EQEmu/Server/pull/3041)) @Kinglykrab 2023-03-06
* Cursor Coin Upon Death ([#3020](https://github.com/EQEmu/Server/pull/3020)) @cybernine186 2023-03-04
* Ensure synchronization of pet taunt state with UI ([#3025](https://github.com/EQEmu/Server/pull/3025)) @catapultam-habeo 2023-03-04
* Fix Bard Bot Casting ([#3122](https://github.com/EQEmu/Server/pull/3122)) @Aeadoin 2023-03-17
* Fix Discovered Items with Alternate Currency and LDoN Adventure Merchants ([#3026](https://github.com/EQEmu/Server/pull/3026)) @Kinglykrab 2023-03-04
* Fix Heal Scale and Spell Scale in NPC Scaling ([#3051](https://github.com/EQEmu/Server/pull/3051)) @Kinglykrab 2023-03-10
* Fix Raid Invites causing client desync issues ([#3053](https://github.com/EQEmu/Server/pull/3053)) @Aeadoin 2023-03-11
* Fix Raid methods that could cause crashes with Bots in raid ([#3111](https://github.com/EQEmu/Server/pull/3111)) @Aeadoin 2023-03-17
* Fix edge cases where camped bots would be left in a raid ([#3139](https://github.com/EQEmu/Server/pull/3139)) @Aeadoin 2023-03-23
* Fix for Raid Disband if leader not in same zone. ([#3135](https://github.com/EQEmu/Server/pull/3135)) @Aeadoin 2023-03-21
* Fix for incorrect bindpoint x,y,z,headings ([#3141](https://github.com/EQEmu/Server/pull/3141)) @Aeadoin 2023-03-23
* Fix for transferring Raid Leader ([#3140](https://github.com/EQEmu/Server/pull/3140)) @Aeadoin 2023-03-23
* Fix issue with overflow on min/max hit dmg in npc scaling calculations ([#3052](https://github.com/EQEmu/Server/pull/3052)) @Aeadoin 2023-03-10
* Fix typo for bot_id raid_members column in db_update_manifest.txt ([#3132](https://github.com/EQEmu/Server/pull/3132)) @Kinglykrab 2023-03-20
* Fixes for corpses not properly saving some item instance data correctly. ([#3123](https://github.com/EQEmu/Server/pull/3123)) @KimLS 2023-03-23

### Illusions

* Fix bug where spells like Ignite Bones left NPC size incorrect. ([#3061](https://github.com/EQEmu/Server/pull/3061)) @noudess 2023-03-16

### Quest API

* Add SendIllusion overloads/parameters to Perl/Lua ([#3059](https://github.com/EQEmu/Server/pull/3059)) @Kinglykrab 2023-03-16
* Add Spell GetActX methods to Perl/Lua ([#3056](https://github.com/EQEmu/Server/pull/3056)) @Kinglykrab 2023-03-12
* Add Timer related methods to Mobs in Perl/Lua ([#3133](https://github.com/EQEmu/Server/pull/3133)) @Kinglykrab 2023-03-20

### Rules

* Add Multiplier for Heroic Stats. ([#3014](https://github.com/EQEmu/Server/pull/3014)) @Aeadoin 2023-03-04
* Add ResurrectionEffectsBlock ([#2990](https://github.com/EQEmu/Server/pull/2990)) @nytmyr 2023-03-04
* Add Rule to allow ExtraDmgSkill/SPA 220 to effect Spell Skills ([#3124](https://github.com/EQEmu/Server/pull/3124)) @Aeadoin 2023-03-19
* Add Task System Rule ExpRewardsIgnoreLevelBasedEXPMods ([#3112](https://github.com/EQEmu/Server/pull/3112)) @Aeadoin 2023-03-17

### Scaling

* Add support for pipe-separated zone IDs and versions ([#3015](https://github.com/EQEmu/Server/pull/3015)) @Kinglykrab 2023-03-04

### Strings

* Add exception handling to converters themselves ([#3029](https://github.com/EQEmu/Server/pull/3029)) @Akkadius 2023-03-05
* Add more number formatters ([#2873](https://github.com/EQEmu/Server/pull/2873)) @Kinglykrab 2023-03-04

## [22.4.5] - 03/03/2023

### Bots

* Add additional Heroic Sta/Wis/Int bonuses for Bots. ([#3013](https://github.com/EQEmu/Server/pull/3013)) @Aeadoin 2023-03-01
* Cleanup AI_IdleCastCheck Logic ([#3004](https://github.com/EQEmu/Server/pull/3004)) @Aeadoin 2023-02-26

### Code

* Delete unused zone/skills.h ([#3007](https://github.com/EQEmu/Server/pull/3007)) @Kinglykrab 2023-02-27
* Remove DumpPacketProfile() from client.h ([#3000](https://github.com/EQEmu/Server/pull/3000)) @Kinglykrab 2023-02-26
* Remove GetCombinedAC_TEST() from client.h ([#2999](https://github.com/EQEmu/Server/pull/2999)) @Kinglykrab 2023-02-26
* Remove GetDamageMultiplier() from client.h ([#3001](https://github.com/EQEmu/Server/pull/3001)) @Kinglykrab 2023-02-26
* Remove NumberOfAvailableTitles() from titles.h ([#3006](https://github.com/EQEmu/Server/pull/3006)) @Kinglykrab 2023-02-27
* Remove ReturnItemPacket from client.h/inventory.cpp ([#3002](https://github.com/EQEmu/Server/pull/3002)) @Kinglykrab 2023-02-26
* Remove class EGNode from mob.h ([#3003](https://github.com/EQEmu/Server/pull/3003)) @Kinglykrab 2023-02-26
* Remove unused ClientFactory in client.h ([#2998](https://github.com/EQEmu/Server/pull/2998)) @Kinglykrab 2023-02-26
* Remove unused iterator from LoadCharacterDisciplines ([#3012](https://github.com/EQEmu/Server/pull/3012)) @Aeadoin 2023-03-02

### Crash

* Fix crash in CheckTradeskillLoreConflict ([#3009](https://github.com/EQEmu/Server/pull/3009)) @Aeadoin 2023-02-28

### Fixes

* Account for bad data in Tradeskill Recipe Entries ([#2991](https://github.com/EQEmu/Server/pull/2991)) @Aeadoin 2023-02-25
* Fix DoAnim quest method default speed ([#3016](https://github.com/EQEmu/Server/pull/3016)) @Kinglykrab 2023-03-01
* Fix an issue where EVENT_TIMER timers would not be cleaned up after zone ([#3018](https://github.com/EQEmu/Server/pull/3018)) @noudess 2023-03-03
* Fix for Discipline Loading from Database causing issues with slot_ids ([#3008](https://github.com/EQEmu/Server/pull/3008)) @Aeadoin 2023-02-28
* Fix for Lore Components where component is returned. ([#3005](https://github.com/EQEmu/Server/pull/3005)) @Aeadoin 2023-02-27
* Fix issue where quest saylink responses would occur before the NPC's response ([#3010](https://github.com/EQEmu/Server/pull/3010)) @Akkadius 2023-03-01
* Fix log messages when players join channel ([#2992](https://github.com/EQEmu/Server/pull/2992)) @Valorith 2023-03-03
* Fix npcfeature and playerfeature ([#3017](https://github.com/EQEmu/Server/pull/3017)) @Kinglykrab 2023-03-02

### Quest API

* Add GetDefaultRaceSize() to Perl/Lua ([#2993](https://github.com/EQEmu/Server/pull/2993)) @Kinglykrab 2023-02-27
* Add HasSpecialAbilities() to Perl/Lua ([#2994](https://github.com/EQEmu/Server/pull/2994)) @Kinglykrab 2023-02-27
* Add IsBerserk() to Perl/Lua ([#2997](https://github.com/EQEmu/Server/pull/2997)) @Kinglykrab 2023-03-01
* Add IsFindable() and IsTrackable() to Perl/Lua ([#2996](https://github.com/EQEmu/Server/pull/2996)) @Kinglykrab 2023-03-01
* Add IsUnderwaterOnly() to Perl/Lua ([#2995](https://github.com/EQEmu/Server/pull/2995)) @Kinglykrab 2023-03-01

## [22.4.4] - 02/24/2023

### Bots

* Add Caster Range Command, and IsValidSpellRange Checks ([#2942](https://github.com/EQEmu/Server/pull/2942)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-23
* Cleanup BotDatabase::LoadBuffs ([#2981](https://github.com/EQEmu/Server/pull/2981)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-24
* Verify Bots Group Integrity on join ([#2980](https://github.com/EQEmu/Server/pull/2980)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-23

### Commands

* Cleanup #peekinv Command ([#2969](https://github.com/EQEmu/Server/pull/2969)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-23

### Doors

* Fix doors triggering invalid zone fetches of dest_zone of "none" ([#2985](https://github.com/EQEmu/Server/pull/2985)) ([Akkadius](https://github.com/Akkadius)) 2023-02-24

### Fixes

* Adjust database manifest to include .sql extension  ([Akkadius](https://github.com/Akkadius)) 2023-02-25
* Correct Mend reuse time and add reduction support. ([#2972](https://github.com/EQEmu/Server/pull/2972)) ([nytmyr](https://github.com/nytmyr)) 2023-02-23
* Fix Beneficial Target of Target procs ([#2987](https://github.com/EQEmu/Server/pull/2987)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-24
* Fix for undefined MySQL library behavior. ([#2834](https://github.com/EQEmu/Server/pull/2834)) ([KimLS](https://github.com/KimLS)) 2023-02-25

### Pathing

* Improve roambox logic ([#2983](https://github.com/EQEmu/Server/pull/2983)) ([Akkadius](https://github.com/Akkadius)) 2023-02-24
* More z-clip improvements, Wurm and Spectral Iksar race adjustments ([#2988](https://github.com/EQEmu/Server/pull/2988)) ([Akkadius](https://github.com/Akkadius)) 2023-02-25
* Smoother pathing z-correction ([#2982](https://github.com/EQEmu/Server/pull/2982)) ([Akkadius](https://github.com/Akkadius)) 2023-02-24

### Player Events

* Add QS processing, mutex tweaks ([#2984](https://github.com/EQEmu/Server/pull/2984)) ([Akkadius](https://github.com/Akkadius)) 2023-02-25

### Quest API

* Add IsAutoAttackEnabled() to Perl/Lua ([#2979](https://github.com/EQEmu/Server/pull/2979)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-23
* Add IsAutoFireEnabled() to Perl/Lua ([#2978](https://github.com/EQEmu/Server/pull/2978)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-23
* Fix EVENT_TIMER crash when entity is no longer available ([#2986](https://github.com/EQEmu/Server/pull/2986)) ([Akkadius](https://github.com/Akkadius)) 2023-02-24

### Scaling

* Add support for zone ID and instance version to NPC Scaling ([#2968](https://github.com/EQEmu/Server/pull/2968)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-25

### Tradeskills

* Fix for Lore Conflict ([#2977](https://github.com/EQEmu/Server/pull/2977)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-24

## [22.4.3] - 02/21/2023

### Bots

* Change HasBotItem(item_id) to return slot_id instead of bool. ([#2966](https://github.com/EQEmu/Server/pull/2966)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-19
* Change SaveTimers to Replace instead of Insert. ([#2951](https://github.com/EQEmu/Server/pull/2951)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-18
* Fix output of ^spells while ^Enforcespellsettings is enabled ([#2959](https://github.com/EQEmu/Server/pull/2959)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-18

### Crash

* Fix crash with EVENT_UNEQUIP_ITEM_BOT ([#2973](https://github.com/EQEmu/Server/pull/2973)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-20
* Fix world crash in player event processing ([#2960](https://github.com/EQEmu/Server/pull/2960)) ([Akkadius](https://github.com/Akkadius)) 2023-02-18

### Database

* Address deadlock in player events ([#2974](https://github.com/EQEmu/Server/pull/2974)) ([Akkadius](https://github.com/Akkadius)) 2023-02-21

### Fixes

* Fix MIR LDoN Theme Items on LDoN Merchants ([#2971](https://github.com/EQEmu/Server/pull/2971)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-20
* Fix OOCMute not functioning ([#2970](https://github.com/EQEmu/Server/pull/2970)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-20

### Pathing

* Improvements to z-clipping, z-recovery and z-calculations ([#2975](https://github.com/EQEmu/Server/pull/2975)) ([Akkadius](https://github.com/Akkadius)) 2023-02-21

### Pets

* Client Pet summoned by NPC should not change guard location. ([#2967](https://github.com/EQEmu/Server/pull/2967)) ([noudess](https://github.com/noudess)) 2023-02-19

### Player Events

* Create new event ITEM_CREATION ([#2944](https://github.com/EQEmu/Server/pull/2944)) ([Akkadius](https://github.com/Akkadius)) 2023-02-18

### Quest API

* Add client->SignalClient() overload to Perl ([#2963](https://github.com/EQEmu/Server/pull/2963)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-19
* Fix Perl SetSimpleRoamBox Overloads ([#2961](https://github.com/EQEmu/Server/pull/2961)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-19

### Reload API

* Add world handlers for certain opcodes ([#2958](https://github.com/EQEmu/Server/pull/2958)) ([Akkadius](https://github.com/Akkadius)) 2023-02-18

### SQL

* Add date to optional Drakkin Guktan Faction Update ([#2965](https://github.com/EQEmu/Server/pull/2965)) ([joligario](https://github.com/joligario)) 2023-02-19

## [22.4.2] - 02/18/2023

### Content

* Added optional SQL 2023_02_17_fix_sseru_mischief_doors.sql to fix sseru/mischief doors ([#2955](https://github.com/EQEmu/Server/pull/2955)) ([Akkadius](https://github.com/Akkadius)) 2023-02-18

### Logging

* Remove noisy raid/group/forage errors ([#2952](https://github.com/EQEmu/Server/pull/2952)) ([Akkadius](https://github.com/Akkadius)) 2023-02-18

### MySQL

* Add keepalives to UCS and Loginserver ([#2953](https://github.com/EQEmu/Server/pull/2953)) ([Akkadius](https://github.com/Akkadius)) 2023-02-18

### Player Events

* Add logging category to hold processing batch logs ([#2954](https://github.com/EQEmu/Server/pull/2954)) ([Akkadius](https://github.com/Akkadius)) 2023-02-18

### Tradeskills

* Fix regression caused by #2932 ([#2956](https://github.com/EQEmu/Server/pull/2956)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-18

## [22.4.1] - 02/17/2023

### Bots

* Set Taunt to enabled for SK/Paladin Bots by Default. ([#2941](https://github.com/EQEmu/Server/pull/2941)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-17

### DevTools

* Fix NPC targetting dev tools display window ([#2943](https://github.com/EQEmu/Server/pull/2943)) ([Akkadius](https://github.com/Akkadius)) 2023-02-17

### Fixes

* Issue with AssignRaidToInstance that was using the groups repository instead of raid ([#2947](https://github.com/EQEmu/Server/pull/2947)) ([Akkadius](https://github.com/Akkadius)) 2023-02-17
* Missing comma in schema list breaking dumps  ([Akkadius](https://github.com/Akkadius)) 2023-02-17

### Player Events

* Fix issue with item instances not being validated properly before accessing causing crashes on handin ([#2945](https://github.com/EQEmu/Server/pull/2945)) ([Akkadius](https://github.com/Akkadius)) 2023-02-17
* Fix rare out of bound issue when loading event types ([#2946](https://github.com/EQEmu/Server/pull/2946)) ([Akkadius](https://github.com/Akkadius)) 2023-02-17
* Turn off KILLED_NPC (trash) off by default ([#2948](https://github.com/EQEmu/Server/pull/2948)) ([Akkadius](https://github.com/Akkadius)) 2023-02-17

## [22.4.0] - 02/17/2023

### Bots

* Add Additional HeroicAgi/Dex Modifiers. ([#2838](https://github.com/EQEmu/Server/pull/2838)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-07
* Add Additional HeroicStr modifiers. ([#2837](https://github.com/EQEmu/Server/pull/2837)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-07
* Add IsBot() to methods in attack.cpp where applicable. ([#2840](https://github.com/EQEmu/Server/pull/2840)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-09
* Add Lore Check for Augments. ([#2874](https://github.com/EQEmu/Server/pull/2874)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-12
* Add Pet Power Support for Temp Pets. ([#2853](https://github.com/EQEmu/Server/pull/2853)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-11
* Add Support for TryTriggerOnCastFocusEffect ([#2864](https://github.com/EQEmu/Server/pull/2864)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-13
* Add TotalDominationBonus modifiers. ([#2852](https://github.com/EQEmu/Server/pull/2852)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-09
* ST_AreaClientOnly spells to land on Bots ([#2849](https://github.com/EQEmu/Server/pull/2849)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-09
* Update ResistSpell to use temp_level_diff client formula ([#2851](https://github.com/EQEmu/Server/pull/2851)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-09

### Bots & Mercenaries

* Add 100% Hit chance if sitting while attacked. ([#2839](https://github.com/EQEmu/Server/pull/2839)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-07
* Add Support for TrySympatheticProc ([#2866](https://github.com/EQEmu/Server/pull/2866)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-13

### CI

* Fix Windows stderr not bubbling properly ([#2925](https://github.com/EQEmu/Server/pull/2925)) ([Akkadius](https://github.com/Akkadius)) 2023-02-14

### Code

* Add IsOfClientBot() virtual method. ([#2845](https://github.com/EQEmu/Server/pull/2845)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-07
* Doors EVENT_CLICK_DOOR syntax adjustment  ([Akkadius](https://github.com/Akkadius)) 2023-02-14
* Remove Unused Mod Hooks ([#2856](https://github.com/EQEmu/Server/pull/2856)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13

### Crash

* Crash fix where invalid input to #heromodel would crash zone ([#2937](https://github.com/EQEmu/Server/pull/2937)) ([Akkadius](https://github.com/Akkadius)) 2023-02-15
* Fix Bot Crash in Bot::Bot Constructor. ([#2868](https://github.com/EQEmu/Server/pull/2868)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-12
* Fix Crash in FindType ([#2867](https://github.com/EQEmu/Server/pull/2867)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-13
* Fix crash in Mob::CommonDamage when attacker was null ([#2872](https://github.com/EQEmu/Server/pull/2872)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-13
* Fix crash issue with dropping items and order of operations ([#2939](https://github.com/EQEmu/Server/pull/2939)) ([joligario](https://github.com/joligario)) 2023-02-16
* Fix issue where long short names overflow file_name  ([Akkadius](https://github.com/Akkadius)) 2023-02-09
* Fix potential crash in Mob::CommonDamage ([#2848](https://github.com/EQEmu/Server/pull/2848)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-09

### Doors

* Fix issue where NPC's wouldn't open doors because door param overflow ([#2934](https://github.com/EQEmu/Server/pull/2934)) ([Akkadius](https://github.com/Akkadius)) 2023-02-15

### Feature

* Add IsOfClientBotMerc() virtual method. ([#2843](https://github.com/EQEmu/Server/pull/2843)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-07

### Fixes

* Another doors fix  ([Akkadius](https://github.com/Akkadius)) 2023-02-14
* Fix CheckNumHitsRemaining() with 1H Blunt ([#2846](https://github.com/EQEmu/Server/pull/2846)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-07
* Fix Door opening regression caused by #2880  ([Akkadius](https://github.com/Akkadius)) 2023-02-14
* Fix EVENT_SPELL_EFFECT_TRANSLOCATE_COMPLETE regression caused by  #2897 ([#2928](https://github.com/EQEmu/Server/pull/2928)) ([Akkadius](https://github.com/Akkadius)) 2023-02-14
* Fix HP_EVENT regression ([#2927](https://github.com/EQEmu/Server/pull/2927)) ([Akkadius](https://github.com/Akkadius)) 2023-02-14
* Fix crash in EVENT_DISCOVER_ITEM ([#2933](https://github.com/EQEmu/Server/pull/2933)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-15
* Fix crash where dropped items crash Lua logic ([#2936](https://github.com/EQEmu/Server/pull/2936)) ([Akkadius](https://github.com/Akkadius)) 2023-02-15
* Fix for interrupting item casts to no longer lock the client if cast time of item greater than 0 ([#2921](https://github.com/EQEmu/Server/pull/2921)) ([Natedog2012](https://github.com/Natedog2012)) 2023-02-13
* Fix issue where Lore groundspawn pickups will desync ROF2+ ([#2929](https://github.com/EQEmu/Server/pull/2929)) ([Akkadius](https://github.com/Akkadius)) 2023-02-14
* Fix issue with EVENT_HP firing regression from #2904 ([#2924](https://github.com/EQEmu/Server/pull/2924)) ([Akkadius](https://github.com/Akkadius)) 2023-02-14
* Replace uses of SPELL_UNKNOWN with IsValidSpell() ([#2938](https://github.com/EQEmu/Server/pull/2938)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-16
* Self Only Spells will no longer check target level or buff restrictions ([#2931](https://github.com/EQEmu/Server/pull/2931)) ([noudess](https://github.com/noudess)) 2023-02-15

### Groundspawns

* Fix issue where groundspawns appear floating high off the ground ([#2930](https://github.com/EQEmu/Server/pull/2930)) ([Akkadius](https://github.com/Akkadius)) 2023-02-15

### Logging

* Add raw opcode when emu translated opcode is not found (OP_Unknown) via (C->S) ([#2847](https://github.com/EQEmu/Server/pull/2847)) ([Akkadius](https://github.com/Akkadius)) 2023-02-08
* Implement Player Event Logging system ([#2833](https://github.com/EQEmu/Server/pull/2833)) ([Akkadius](https://github.com/Akkadius)) 2023-02-13

### Quest API

* (Performance) Check equip or scale item events exist before export and execute ([#2898](https://github.com/EQEmu/Server/pull/2898)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event EVENT_AA_BUY or EVENT_AA_GAIN exist before export and execute ([#2892](https://github.com/EQEmu/Server/pull/2892)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event EVENT_AGGRO, EVENT_ATTACK, or EVENT_COMBAT exist before export and execute ([#2901](https://github.com/EQEmu/Server/pull/2901)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event EVENT_COMBINE, EVENT_COMBINE_SUCCESS, EVENT_COMBINE_FAILURE, or EVENT_COMBINE_VALIDATE exist before export and execute ([#2896](https://github.com/EQEmu/Server/pull/2896)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event EVENT_DEATH, EVENT_DEATH_COMPLETE, or EVENT_DEATH_ZONE exist before export and execute ([#2909](https://github.com/EQEmu/Server/pull/2909)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event EVENT_ENVIRONMENTAL_DAMAGE exists before export and execute ([#2899](https://github.com/EQEmu/Server/pull/2899)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event EVENT_FEIGN_DEATH exists before export and execute ([#2916](https://github.com/EQEmu/Server/pull/2916)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event EVENT_ITEM_TICK or EVENT_WEAPON_PROC exist before export and execute ([#2914](https://github.com/EQEmu/Server/pull/2914)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event EVENT_LANGUAGE_SKILL_UP, EVENT_SKILL_UP, or EVENT_USE_SKILL exist before export and execute ([#2894](https://github.com/EQEmu/Server/pull/2894)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event EVENT_PAYLOAD or EVENT_SIGNAL exist before export and execute ([#2902](https://github.com/EQEmu/Server/pull/2902)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event EVENT_SLAY exists before export and execute ([#2910](https://github.com/EQEmu/Server/pull/2910)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event EVENT_WAYPOINT_ARRIVE or EVENT_WAYPOINT_DEPART exist before export and execute ([#2905](https://github.com/EQEmu/Server/pull/2905)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event exists before export and execute EVENT_AGGRO_SAY, EVENT_SAY, and EVENT_PROXIMITY_SAY ([#2882](https://github.com/EQEmu/Server/pull/2882)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event exists before export and execute EVENT_BOT_CREATE ([#2886](https://github.com/EQEmu/Server/pull/2886)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event exists before export and execute EVENT_CLICK_DOOR and EVENT_CLICK_OBJECT ([#2880](https://github.com/EQEmu/Server/pull/2880)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event exists before export and execute EVENT_DESPAWN and EVENT_DESPAWN_ZONE ([#2887](https://github.com/EQEmu/Server/pull/2887)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event exists before export and execute EVENT_DISCOVER_ITEM ([#2912](https://github.com/EQEmu/Server/pull/2912)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event exists before export and execute EVENT_DUEL_LOSE and EVENT_DUEL_WIN ([#2915](https://github.com/EQEmu/Server/pull/2915)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event exists before export and execute EVENT_ENTER_ZONE and EVENT_ZONE ([#2900](https://github.com/EQEmu/Server/pull/2900)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event exists before export and execute EVENT_GM_COMMAND ([#2890](https://github.com/EQEmu/Server/pull/2890)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event exists before export and execute EVENT_GROUP_CHANGE ([#2884](https://github.com/EQEmu/Server/pull/2884)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event exists before export and execute EVENT_HP ([#2904](https://github.com/EQEmu/Server/pull/2904)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event exists before export and execute EVENT_KILLED_MERIT ([#2911](https://github.com/EQEmu/Server/pull/2911)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event exists before export and execute EVENT_LEVEL_UP and EVENT_LEVEL_DOWN ([#2889](https://github.com/EQEmu/Server/pull/2889)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event exists before export and execute EVENT_POPUP_RESPONSE ([#2881](https://github.com/EQEmu/Server/pull/2881)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event exists before export and execute EVENT_RESPAWN ([#2917](https://github.com/EQEmu/Server/pull/2917)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event exists before export and execute EVENT_TICK ([#2919](https://github.com/EQEmu/Server/pull/2919)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event exists before export and execute EVENT_TIMER ([#2903](https://github.com/EQEmu/Server/pull/2903)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event exists before export and execute EVENT_TRADE ([#2906](https://github.com/EQEmu/Server/pull/2906)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event exists before export and execute EVENT_UNHANDLED_OPCODE ([#2918](https://github.com/EQEmu/Server/pull/2918)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event exists before export and execute EVENT_WARP ([#2907](https://github.com/EQEmu/Server/pull/2907)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event exists before export and execute area events ([#2888](https://github.com/EQEmu/Server/pull/2888)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check merchant events exist before export and execute ([#2893](https://github.com/EQEmu/Server/pull/2893)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check spell or cast events exist before export and execute ([#2897](https://github.com/EQEmu/Server/pull/2897)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check task events exist before export and execute ([#2883](https://github.com/EQEmu/Server/pull/2883)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event exists before export and execute EVENT_CONNECT and EVENT_DISCONNECT ([#2913](https://github.com/EQEmu/Server/pull/2913)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* (Performance) Check event exists before export and execute EVENT_TEST_BUFF ([#2920](https://github.com/EQEmu/Server/pull/2920)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* Add $target export to EVENT_INSPECT in Perl ([#2891](https://github.com/EQEmu/Server/pull/2891)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* Add Additional XP Events EVENT_AA_EXP_GAIN, EVENT_EXP_GAIN ([#2865](https://github.com/EQEmu/Server/pull/2865)) ([Valorith](https://github.com/Valorith)) 2023-02-13
* Add EVENT_DESTROY_ITEM_CLIENT to Perl/Lua. ([#2871](https://github.com/EQEmu/Server/pull/2871)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* Add EVENT_DROP_ITEM_CLIENT to Perl/Lua ([#2869](https://github.com/EQEmu/Server/pull/2869)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* Add Recipe-based methods to Perl/Lua. ([#2844](https://github.com/EQEmu/Server/pull/2844)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-08
* Export $door to EVENT_CLICKDOOR in Perl ([#2861](https://github.com/EQEmu/Server/pull/2861)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-11
* Export $hate_entity to EVENT_HATE_LIST in Perl ([#2885](https://github.com/EQEmu/Server/pull/2885)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* Export $item and $augment to augment events in Perl ([#2895](https://github.com/EQEmu/Server/pull/2895)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* Export $item and $corpse to EVENT_LOOT and EVENT_LOOT_ZONE in Perl ([#2878](https://github.com/EQEmu/Server/pull/2878)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* Export $item to Client/Bot Equip Events in Perl ([#2860](https://github.com/EQEmu/Server/pull/2860)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-11
* Export $item to EVENT_DISCOVER_ITEM in Perl ([#2863](https://github.com/EQEmu/Server/pull/2863)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-11
* Export $item to EVENT_PLAYER_PICKUP in Perl. ([#2875](https://github.com/EQEmu/Server/pull/2875)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* Export $item to Fishing and Forage Events in Perl ([#2876](https://github.com/EQEmu/Server/pull/2876)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* Export $killed_npc to EVENT_NPC_SLAY to Perl ([#2879](https://github.com/EQEmu/Server/pull/2879)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* Export $object to EVENT_CLICK_OBJECT in Perl ([#2862](https://github.com/EQEmu/Server/pull/2862)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-11
* Export $spawned to EVENT_SPAWN_ZONE in Perl ([#2877](https://github.com/EQEmu/Server/pull/2877)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* Export target to EVENT_TARGET_CHANGE in Perl/Lua. ([#2870](https://github.com/EQEmu/Server/pull/2870)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* Export targets to EVENT_CONSIDER and EVENT_CONSIDER_CORPSE ([#2908](https://github.com/EQEmu/Server/pull/2908)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13
* Fix SetSimpleRoamBox in Perl to have optional params again ([#2935](https://github.com/EQEmu/Server/pull/2935)) ([Akkadius](https://github.com/Akkadius)) 2023-02-15

### Rules

* Add Group/Raid Experience Rules ([#2850](https://github.com/EQEmu/Server/pull/2850)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-13

### Tradeskills

* Check if combine would result in lore conflict ([#2932](https://github.com/EQEmu/Server/pull/2932)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-16

### Windows

* Fix MSVC compilation bug via workaround ([#2926](https://github.com/EQEmu/Server/pull/2926)) ([Akkadius](https://github.com/Akkadius)) 2023-02-14

## [22.3.0] - 02/06/2023

### Bots

* Add GetAugmentIDsBySlotID & AddItem with table ref Methods. ([#2805](https://github.com/EQEmu/Server/pull/2805)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-29

### Commands

* #list now searches without case sensitivity ([#2825](https://github.com/EQEmu/Server/pull/2825)) ([Akkadius](https://github.com/Akkadius)) 2023-02-06
* Remove extraneous else from #weather ([#2819](https://github.com/EQEmu/Server/pull/2819)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-01

### Crash

* Fix IsUnderwaterOnly crash where npc data references can be stale ([#2830](https://github.com/EQEmu/Server/pull/2830)) ([Akkadius](https://github.com/Akkadius)) 2023-02-06
* Fix command crash with #npcedit weapon when second weapon not passed ni ([#2829](https://github.com/EQEmu/Server/pull/2829)) ([Akkadius](https://github.com/Akkadius)) 2023-02-06
* Fix crash in bot command botdyearmor ([#2832](https://github.com/EQEmu/Server/pull/2832)) ([Akkadius](https://github.com/Akkadius)) 2023-02-06

### DB Updates

* Add Windows MySQL path auto detection for users where the path is not found ([#2836](https://github.com/EQEmu/Server/pull/2836)) ([Akkadius](https://github.com/Akkadius)) 2023-02-06

### Doors

* Have NPCs trigger double doors ([#2821](https://github.com/EQEmu/Server/pull/2821)) ([Akkadius](https://github.com/Akkadius)) 2023-02-06
* Remove door dev tools spam on client controlled doors ([#2824](https://github.com/EQEmu/Server/pull/2824)) ([Akkadius](https://github.com/Akkadius)) 2023-02-06

### Feature

* Add Min/Max Status to Merchants ([#2806](https://github.com/EQEmu/Server/pull/2806)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-29

### Fixes

* #reload aa will now refresh the AA table properly for every client when changes are made ([#2814](https://github.com/EQEmu/Server/pull/2814)) ([Natedog2012](https://github.com/Natedog2012)) 2023-01-31
* #reload static should now properly fill the entity_lists for… ([#2815](https://github.com/EQEmu/Server/pull/2815)) ([Natedog2012](https://github.com/Natedog2012)) 2023-01-31
* BuffLevelRestrictions were restricting group buffs if mob targeted ([#2809](https://github.com/EQEmu/Server/pull/2809)) ([noudess](https://github.com/noudess)) 2023-01-29
* Fix does_augment_fit_slot method. ([#2817](https://github.com/EQEmu/Server/pull/2817)) ([Aeadoin](https://github.com/Aeadoin)) 2023-02-01
* Fix NPC ghosting at safe coordinates ([#2823](https://github.com/EQEmu/Server/pull/2823)) ([Akkadius](https://github.com/Akkadius)) 2023-02-06
* Fixing % based mob see invis ([#2802](https://github.com/EQEmu/Server/pull/2802)) ([fryguy503](https://github.com/fryguy503)) 2023-01-29
* Resolve issue with max buff count being 25 in ROF2. ([#2800](https://github.com/EQEmu/Server/pull/2800)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-28

### Hotfix

* Post revert build fix for https://github.com/EQEmu/Server/commit/54050924d81d1f83268fe01f9c2b36fe10626601  ([Akkadius](https://github.com/Akkadius)) 2023-02-01

### Lua

* Resolve stoi Exception ([#2736](https://github.com/EQEmu/Server/pull/2736)) ([Akkadius](https://github.com/Akkadius)) 2023-02-06

### Pathing

* Improvements to handling tight corridors pathing, clipping detection and recovery ([#2826](https://github.com/EQEmu/Server/pull/2826)) ([Akkadius](https://github.com/Akkadius)) 2023-02-06

### Quest API

* Add Augment Slot support to does_augment_fit ([#2813](https://github.com/EQEmu/Server/pull/2813)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-31
* Add EVENT_DAMAGE_GIVEN and EVENT_DAMAGE_TAKEN to Perl/Lua. ([#2804](https://github.com/EQEmu/Server/pull/2804)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-29
* Add EVENT_ITEM_CLICK_CLIENT and EVENT_ITEM_CLICK_CAST_CLIENT to Perl/Lua. ([#2810](https://github.com/EQEmu/Server/pull/2810)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-30
* Add EVENT_TASKACCEPTED to Player scope ([#2822](https://github.com/EQEmu/Server/pull/2822)) ([Valorith](https://github.com/Valorith)) 2023-02-06
* Add GetItemCooldown to return the time remaining on items… ([#2811](https://github.com/EQEmu/Server/pull/2811)) ([Natedog2012](https://github.com/Natedog2012)) 2023-01-30
* Add LDoN Methods to Perl/Lua ([#2799](https://github.com/EQEmu/Server/pull/2799)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-29
* Add Override Parameters to ScaleNPC() in Perl/Lua. ([#2816](https://github.com/EQEmu/Server/pull/2816)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-01
* Add rule AlternateAugmentationSealer for using a different bagtype ([#2831](https://github.com/EQEmu/Server/pull/2831)) ([Natedog2012](https://github.com/Natedog2012)) 2023-02-06
* Default ScaleNPC to always scale. ([#2818](https://github.com/EQEmu/Server/pull/2818)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-02-06

### Readme

* Update build badges with Drone  ([Akkadius](https://github.com/Akkadius)) 2023-01-29

### Rules

* Add rule to ignore name filter on chat channel creation. ([#2820](https://github.com/EQEmu/Server/pull/2820)) ([Valorith](https://github.com/Valorith)) 2023-02-06
* Added rule to bypass level based haste caps ([#2835](https://github.com/EQEmu/Server/pull/2835)) ([jcr4990](https://github.com/jcr4990)) 2023-02-06
* Fix rule updates that affected bot booting checks ([#2841](https://github.com/EQEmu/Server/pull/2841)) ([Akkadius](https://github.com/Akkadius)) 2023-02-06

### Tasks

* Implement alternate currency rewards ([#2827](https://github.com/EQEmu/Server/pull/2827)) ([Akkadius](https://github.com/Akkadius)) 2023-02-06

## [22.2.0] - 01/27/2023

### Bots

* Add EVENT_UNEQUIP_ITEM_BOT & EVENT_EQUIP_ITEM_BOT ([#2796](https://github.com/EQEmu/Server/pull/2796)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-27
* ^create and ^viewcombos popup messages fix. ([#2797](https://github.com/EQEmu/Server/pull/2797)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-26

### Code Cleanup

* Cleanup #door Command. ([#2783](https://github.com/EQEmu/Server/pull/2783)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-24

### Crash

* Fix crash issue with log formatting during character creation ([#2798](https://github.com/EQEmu/Server/pull/2798)) ([Akkadius](https://github.com/Akkadius)) 2023-01-27

### Feature

* ResetItemCooldown added to lua/perl and fix item re-cast times to show properly ([#2793](https://github.com/EQEmu/Server/pull/2793)) ([Natedog2012](https://github.com/Natedog2012)) 2023-01-26

### Git

* Add CMake Files to .gitignore ([#2792](https://github.com/EQEmu/Server/pull/2792)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-25

## [22.1.2] - 01/24/2023

### CI/CD

* Build / Release Pipeline Changes ([#2788](https://github.com/EQEmu/Server/pull/2788)) ([Akkadius](https://github.com/Akkadius)) 2023-01-24

### Code Cleanup

* Cleanup #door Command. ([#2783](https://github.com/EQEmu/Server/pull/2783)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-24

### Crash

* Fix rarer world crash issue where scheduler database was not available ([#2789](https://github.com/EQEmu/Server/pull/2789)) ([Akkadius](https://github.com/Akkadius)) 2023-01-24

### Fixes

* Fix nullptr spell in BCSpells::Load() ([#2790](https://github.com/EQEmu/Server/pull/2790)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-24
* Remove duplicate logic in GetActSpellHealing reducing HOT criticals ([#2786](https://github.com/EQEmu/Server/pull/2786)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-24

## [22.1.1] - 01/23/2023

### Fixes

* Fix botgrouplist to display unique entries. ([#2785](https://github.com/EQEmu/Server/pull/2785)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-23
* Fix scenario where dereferenced object could be null. ([#2784](https://github.com/EQEmu/Server/pull/2784)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-23

## [22.1.0] - 01/22/2023

This is a first release using the new build system. Changelog entry representative of last year. Subsequent releases will consist of incremental changes since the last release.

### AA

* Fix AA tables dump ([#2769](https://github.com/EQEmu/Server/pull/2769)) ([Akkadius](https://github.com/Akkadius)) 2023-01-22

### Appveyor

* Remove bots preprocessor  ([Akkadius](https://github.com/Akkadius)) 2023-01-20

### Bot/Merc

* Cleanup methods, and virtual overrides. ([#2734](https://github.com/EQEmu/Server/pull/2734)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-15

### Bots

* Add Bot Command Reloading ([#2773](https://github.com/EQEmu/Server/pull/2773)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-22
* Add Bot-specific Spell Settings. ([#2553](https://github.com/EQEmu/Server/pull/2553)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-27
* Add Buff support for Bards under AI_IdleCastChecks ([#2590](https://github.com/EQEmu/Server/pull/2590)) ([Aeadoin](https://github.com/Aeadoin)) 2022-11-28
* Add Data Bucket support to Bot Spell Entries. ([#2505](https://github.com/EQEmu/Server/pull/2505)) ([Aeadoin](https://github.com/Aeadoin)) 2022-11-06
* Add EVENT_TRADE Support to Bots. ([#2560](https://github.com/EQEmu/Server/pull/2560)) ([Aeadoin](https://github.com/Aeadoin)) 2022-11-25
* Add Event_Trade Support for ^inventorygive Command ([#2628](https://github.com/EQEmu/Server/pull/2628)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-11
* Add Expansion Bitmask Quest APIs. ([#2523](https://github.com/EQEmu/Server/pull/2523)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-16
* Add GetBotOwnerByBotID Method ([#2715](https://github.com/EQEmu/Server/pull/2715)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-11
* Add Melee Support for Casting, Cleanup Bot Casting Logic ([#2571](https://github.com/EQEmu/Server/pull/2571)) ([Aeadoin](https://github.com/Aeadoin)) 2022-11-25
* Add Quest API Methods ([#2631](https://github.com/EQEmu/Server/pull/2631)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-12
* Add Quest API Support for Limits. ([#2522](https://github.com/EQEmu/Server/pull/2522)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-17
* Add Rule Allowing Bots to Equip Any Race Items ([#2578](https://github.com/EQEmu/Server/pull/2578)) ([Aeadoin](https://github.com/Aeadoin)) 2022-11-26
* Add Support for AA bonuses that were missing. ([#2764](https://github.com/EQEmu/Server/pull/2764)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-20
* Add Support for Bots to receive Auras, and other AoE Buffs. ([#2586](https://github.com/EQEmu/Server/pull/2586)) ([Aeadoin](https://github.com/Aeadoin)) 2022-11-27
* Add Virtual Override for Bot::Attack ([#2771](https://github.com/EQEmu/Server/pull/2771)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-20
* Add give/remove saylinks to ^itemuse. ([#2503](https://github.com/EQEmu/Server/pull/2503)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-10-30
* Add support for Bot scripting. ([#2515](https://github.com/EQEmu/Server/pull/2515)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-17
* Apply Spells:IgnoreSpellDmgLvlRestriction to bots ([#2024](https://github.com/EQEmu/Server/pull/2024)) ([catapultam-habeo](https://github.com/catapultam-habeo)) 2022-03-07
* Bot::PerformTradeWithClient Cleanup. ([#2084](https://github.com/EQEmu/Server/pull/2084)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-01
* Cleanup Bot Spell Functions, reduce reliance on NPC Functions/Attributes ([#2495](https://github.com/EQEmu/Server/pull/2495)) ([Aeadoin](https://github.com/Aeadoin)) 2022-10-29
* Cleanup Fast Rest Regen ([#2626](https://github.com/EQEmu/Server/pull/2626)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-07
* Cleanup Say Event Parse. ([#2557](https://github.com/EQEmu/Server/pull/2557)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-20
* Cleanup Spell Settings Commands ([#2607](https://github.com/EQEmu/Server/pull/2607)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-04
* Cleanup ^inventoryremove, ^inventorylist, and ^list Commands and bot groups. ([#2273](https://github.com/EQEmu/Server/pull/2273)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-07-03
* Cleanup and remove preprocessors. ([#2757](https://github.com/EQEmu/Server/pull/2757)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-20
* Cleanup various Bot Spell Focus methods ([#2649](https://github.com/EQEmu/Server/pull/2649)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-16
* Convert Load, Save, SaveNew, and Delete to Repositories. ([#2614](https://github.com/EQEmu/Server/pull/2614)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-04
* Expanded Bot Spell Settings List. ([#2606](https://github.com/EQEmu/Server/pull/2606)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-03
* Fix Bot Spell Type "In Combat Buffs" ([#2711](https://github.com/EQEmu/Server/pull/2711)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-08
* Fix Gender not saving as GetBaseGender on BotSave ([#2639](https://github.com/EQEmu/Server/pull/2639)) ([nytmyr](https://github.com/nytmyr)) 2022-12-13
* Fix Slow Query in QueryNameAvailablity ([#2781](https://github.com/EQEmu/Server/pull/2781)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-22
* Fix ^dyearmor command math. ([#2081](https://github.com/EQEmu/Server/pull/2081)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-04-30
* Fix bot spawn when bot id = char_id ([#1984](https://github.com/EQEmu/Server/pull/1984)) ([neckkola](https://github.com/neckkola)) 2022-03-07
* Hotfix for possible crash. ([#2539](https://github.com/EQEmu/Server/pull/2539)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-14
* Melee Bot Support for Spell Settings Commands ([#2599](https://github.com/EQEmu/Server/pull/2599)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-01
* Move Bot Spell Loading process to constructor from calcbotstats() ([#2583](https://github.com/EQEmu/Server/pull/2583)) ([Aeadoin](https://github.com/Aeadoin)) 2022-11-27
* Optimize inventory loading. ([#2588](https://github.com/EQEmu/Server/pull/2588)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-28
* Post pre-processor fixes ([#2770](https://github.com/EQEmu/Server/pull/2770)) ([Akkadius](https://github.com/Akkadius)) 2023-01-20
* Resolve incorrect values on Bot Creation ([#2644](https://github.com/EQEmu/Server/pull/2644)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-14
* Restrict Bot Groups from spawning while Feigned. ([#2761](https://github.com/EQEmu/Server/pull/2761)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-19
* Save Bot Toggle Archer Setting between Loads. ([#2612](https://github.com/EQEmu/Server/pull/2612)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-04
* Update Bot Heal & Damage methods to more closely match Clients + Bugfixes ([#2045](https://github.com/EQEmu/Server/pull/2045)) ([catapultam-habeo](https://github.com/catapultam-habeo)) 2022-03-11
* Update Bot Logic to ignore ST_TargetsTarget when buffing ([#2584](https://github.com/EQEmu/Server/pull/2584)) ([Aeadoin](https://github.com/Aeadoin)) 2022-11-27

### C++20

* Arithmetic on different enums is deprecated ([#2752](https://github.com/EQEmu/Server/pull/2752)) ([mackal](https://github.com/mackal)) 2023-01-17
* Enable C++20 + Fixes + FMT 9.1 ([#2664](https://github.com/EQEmu/Server/pull/2664)) ([Akkadius](https://github.com/Akkadius)) 2022-12-21

### Code

* Removed vscode setting ([#2753](https://github.com/EQEmu/Server/pull/2753)) ([xackery](https://github.com/xackery)) 2023-01-17

### Code Cleanup

* Add Validation to varchar number item fields. ([#2241](https://github.com/EQEmu/Server/pull/2241)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-06-04
* Cleanup #kick message. ([#2164](https://github.com/EQEmu/Server/pull/2164)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-10
* Cleanup Haste references and Lua API calls for unsigned to signed. ([#2240](https://github.com/EQEmu/Server/pull/2240)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-06-04
* Cleanup code smells and compiler warnings in common/shareddb ([#2270](https://github.com/EQEmu/Server/pull/2270)) ([Quintinon](https://github.com/Quintinon)) 2022-07-03
* Cleanup magic numbers ([#2662](https://github.com/EQEmu/Server/pull/2662)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-20
* Cleanup spell and max level bucket logic. ([#2181](https://github.com/EQEmu/Server/pull/2181)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-28
* Extra Space in NPC::AISpellsList(). ([#2555](https://github.com/EQEmu/Server/pull/2555)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-20
* Fix unintended copies in zone/zonedb.cpp by changing auto to auto& ([#2271](https://github.com/EQEmu/Server/pull/2271)) ([Quintinon](https://github.com/Quintinon)) 2022-07-03
* Make use of std::abs where possible. ([#2739](https://github.com/EQEmu/Server/pull/2739)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-15
* Merge Client::Attack and Bot::Attack into Mob::Attack ([#2756](https://github.com/EQEmu/Server/pull/2756)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-20
* Move Client::Undye() to client.cpp from #path Command. ([#2188](https://github.com/EQEmu/Server/pull/2188)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-21
* Possible issues with variable/parameter name equality. ([#2161](https://github.com/EQEmu/Server/pull/2161)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-10
* Quest API push methods using invalid types. ([#2172](https://github.com/EQEmu/Server/pull/2172)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-16
* Remove Unused EQEMU_DEPOP_INVALIDATES_CACHE ([#2292](https://github.com/EQEmu/Server/pull/2292)) ([Akkadius](https://github.com/Akkadius)) 2022-07-14
* Remove this-> in code where its implied ([#2088](https://github.com/EQEmu/Server/pull/2088)) ([Akkadius](https://github.com/Akkadius)) 2022-05-01
* Remove unused basic_functions.h ([#2729](https://github.com/EQEmu/Server/pull/2729)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-13
* Remove unused maxskill.h. ([#2728](https://github.com/EQEmu/Server/pull/2728)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-13
* Remove unused methods. ([#2171](https://github.com/EQEmu/Server/pull/2171)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-15
* Remove unusued Max Item ID Constant ([#2528](https://github.com/EQEmu/Server/pull/2528)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-08
* Remove use of bzero since it is deprecated for memset ([#2295](https://github.com/EQEmu/Server/pull/2295)) ([Akkadius](https://github.com/Akkadius)) 2022-07-14
* Resharper Warnings ([#2235](https://github.com/EQEmu/Server/pull/2235)) ([Quintinon](https://github.com/Quintinon)) 2022-06-01
* Resolve some warnings in loginserver/world_server.cpp ([#2347](https://github.com/EQEmu/Server/pull/2347)) ([mackal](https://github.com/mackal)) 2022-07-31
* Rework Lua QuestReward to not use try/catch blocks ([#2417](https://github.com/EQEmu/Server/pull/2417)) ([mackal](https://github.com/mackal)) 2022-09-03
* Send eqstr message in AddAAPoints ([#2507](https://github.com/EQEmu/Server/pull/2507)) ([hgtw](https://github.com/hgtw)) 2022-10-29
* Update to EQEmu #2253 to clean up message strings ([#2279](https://github.com/EQEmu/Server/pull/2279)) ([fryguy503](https://github.com/fryguy503)) 2022-07-03
* Zone Data Loading Refactor ([#2388](https://github.com/EQEmu/Server/pull/2388)) ([Akkadius](https://github.com/Akkadius)) 2022-09-01

### Commands

* #bind Typo. ([#2196](https://github.com/EQEmu/Server/pull/2196)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-23
* #ginfo Cleanup. ([#1955](https://github.com/EQEmu/Server/pull/1955)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-03
* #reload Command Overhaul. ([#2162](https://github.com/EQEmu/Server/pull/2162)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-10
* #reload level_mods could cause Non-Booted zones to crash. ([#2670](https://github.com/EQEmu/Server/pull/2670)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-24
* Add #bugs Command. ([#2559](https://github.com/EQEmu/Server/pull/2559)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-22
* Add #feature Command. ([#2142](https://github.com/EQEmu/Server/pull/2142)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-06
* Add #findcharacter Command. ([#2692](https://github.com/EQEmu/Server/pull/2692)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-03
* Add #findrecipe and #viewrecipe Commands. ([#2401](https://github.com/EQEmu/Server/pull/2401)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-08-31
* Add #setanon Command ([#2690](https://github.com/EQEmu/Server/pull/2690)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-03
* Add #suspendmulti Command. ([#2619](https://github.com/EQEmu/Server/pull/2619)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-11
* Add BestZ and Region Data to #loc ([#2245](https://github.com/EQEmu/Server/pull/2245)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-06-05
* Add additional #peqzone functionality. ([#2085](https://github.com/EQEmu/Server/pull/2085)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-01
* Add max_hp back to #modifynpcstat command. ([#2638](https://github.com/EQEmu/Server/pull/2638)) ([nytmyr](https://github.com/nytmyr)) 2022-12-13
* Adding movespeed to #showstats output ([#2596](https://github.com/EQEmu/Server/pull/2596)) ([fryguy503](https://github.com/fryguy503)) 2022-11-30
* Bug fix for #logs command. ([#2008](https://github.com/EQEmu/Server/pull/2008)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-17
* Cleanup #ai Command. ([#1980](https://github.com/EQEmu/Server/pull/1980)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-11
* Cleanup #appearanceeffects Command. ([#2777](https://github.com/EQEmu/Server/pull/2777)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-22
* Cleanup #attack Command. ([#2103](https://github.com/EQEmu/Server/pull/2103)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-04
* Cleanup #ban, #ipban, #flag, #kick, #setlsinfo, and #setpass Commands. ([#2104](https://github.com/EQEmu/Server/pull/2104)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-08
* Cleanup #chat Command. ([#2581](https://github.com/EQEmu/Server/pull/2581)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-27
* Cleanup #corpsefix Command. ([#2197](https://github.com/EQEmu/Server/pull/2197)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-27
* Cleanup #cvs Command. ([#2153](https://github.com/EQEmu/Server/pull/2153)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-08
* Cleanup #date Command. ([#2228](https://github.com/EQEmu/Server/pull/2228)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-06-01
* Cleanup #dbspawn2 Command. ([#2493](https://github.com/EQEmu/Server/pull/2493)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-10-30
* Cleanup #delacct Command. ([#2567](https://github.com/EQEmu/Server/pull/2567)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-22
* Cleanup #depop Command. ([#2536](https://github.com/EQEmu/Server/pull/2536)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-14
* Cleanup #depopzone Command. ([#2537](https://github.com/EQEmu/Server/pull/2537)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-14
* Cleanup #devtools Command. ([#2538](https://github.com/EQEmu/Server/pull/2538)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-14
* Cleanup #doanim Command. ([#2540](https://github.com/EQEmu/Server/pull/2540)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-14
* Cleanup #emote Command. ([#2535](https://github.com/EQEmu/Server/pull/2535)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-14
* Cleanup #emotesearch and #emoteview Command. ([#2494](https://github.com/EQEmu/Server/pull/2494)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-10-30
* Cleanup #emptyinventory Command. ([#2219](https://github.com/EQEmu/Server/pull/2219)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-29
* Cleanup #findaliases and #help Commands. ([#2204](https://github.com/EQEmu/Server/pull/2204)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-27
* Cleanup #findclass and #findrace Commands. ([#2211](https://github.com/EQEmu/Server/pull/2211)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-27
* Cleanup #flagedit Command. ([#1968](https://github.com/EQEmu/Server/pull/1968)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-10
* Cleanup #freeze and #unfreeze Commands. ([#2102](https://github.com/EQEmu/Server/pull/2102)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-04
* Cleanup #gassign Command. ([#2101](https://github.com/EQEmu/Server/pull/2101)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-06
* Cleanup #gearup Command. ([#2589](https://github.com/EQEmu/Server/pull/2589)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-27
* Cleanup #getvariable Command. ([#2100](https://github.com/EQEmu/Server/pull/2100)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-04
* Cleanup #guild Command ([#2693](https://github.com/EQEmu/Server/pull/2693)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-04
* Cleanup #hatelist Command. ([#1976](https://github.com/EQEmu/Server/pull/1976)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-10
* Cleanup #heromodel Command. ([#2566](https://github.com/EQEmu/Server/pull/2566)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-22
* Cleanup #kill Command. ([#2195](https://github.com/EQEmu/Server/pull/2195)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-23
* Cleanup #level Command. ([#2203](https://github.com/EQEmu/Server/pull/2203)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-27
* Cleanup #logs Command. ([#1969](https://github.com/EQEmu/Server/pull/1969)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-10
* Cleanup #makepet Command. ([#2105](https://github.com/EQEmu/Server/pull/2105)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #modifynpcstat Command. ([#2499](https://github.com/EQEmu/Server/pull/2499)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-10-30
* Cleanup #motd Command. ([#2190](https://github.com/EQEmu/Server/pull/2190)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-23
* Cleanup #name Command. ([#1977](https://github.com/EQEmu/Server/pull/1977)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-10
* Cleanup #netstats Command. ([#1970](https://github.com/EQEmu/Server/pull/1970)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-10
* Cleanup #npcedit Command. ([#2582](https://github.com/EQEmu/Server/pull/2582)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-27
* Cleanup #npcedit, #lastname, #title, and #titlesuffix Commands. ([#2215](https://github.com/EQEmu/Server/pull/2215)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-28
* Cleanup #npceditmass command. ([#1957](https://github.com/EQEmu/Server/pull/1957)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-03
* Cleanup #npcemote Command. ([#2106](https://github.com/EQEmu/Server/pull/2106)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #npcloot Command. ([#1974](https://github.com/EQEmu/Server/pull/1974)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-11
* Cleanup #npcsay and #npcshout Commands. ([#2107](https://github.com/EQEmu/Server/pull/2107)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #npcspecialattk Command. ([#2108](https://github.com/EQEmu/Server/pull/2108)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #npctype_cache Command. ([#2109](https://github.com/EQEmu/Server/pull/2109)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #npctypespawn Command. ([#2110](https://github.com/EQEmu/Server/pull/2110)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #nudge Command. ([#2220](https://github.com/EQEmu/Server/pull/2220)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-29
* Cleanup #oocmute Command. ([#2191](https://github.com/EQEmu/Server/pull/2191)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-27
* Cleanup #opcode Command. ([#2547](https://github.com/EQEmu/Server/pull/2547)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-16
* Cleanup #profanity Command. ([#2113](https://github.com/EQEmu/Server/pull/2113)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #push Command. ([#2114](https://github.com/EQEmu/Server/pull/2114)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #qglobal Command. ([#2115](https://github.com/EQEmu/Server/pull/2115)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #randomizefeatures Command. ([#2118](https://github.com/EQEmu/Server/pull/2118)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #refreshgroup Command. ([#2119](https://github.com/EQEmu/Server/pull/2119)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #reloadaa Command. ([#2120](https://github.com/EQEmu/Server/pull/2120)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #reloadallrules Command. ([#2121](https://github.com/EQEmu/Server/pull/2121)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #reloadlevelmods Command. ([#2122](https://github.com/EQEmu/Server/pull/2122)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #reloadmerchants Command. ([#2123](https://github.com/EQEmu/Server/pull/2123)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #reloadperlexportsettings Command. ([#2124](https://github.com/EQEmu/Server/pull/2124)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #reloadrulesworld Command. ([#2128](https://github.com/EQEmu/Server/pull/2128)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #reloadstatic Command. ([#2130](https://github.com/EQEmu/Server/pull/2130)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #reloadtitles Command. ([#2125](https://github.com/EQEmu/Server/pull/2125)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #reloadtraps Command. ([#2126](https://github.com/EQEmu/Server/pull/2126)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #reloadworld and #repop Command. ([#2127](https://github.com/EQEmu/Server/pull/2127)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #reloadzps Command. ([#2129](https://github.com/EQEmu/Server/pull/2129)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #resetaa Command. ([#2132](https://github.com/EQEmu/Server/pull/2132)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #resetaa_timer Command. ([#2131](https://github.com/EQEmu/Server/pull/2131)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #resetdisc_timer Command. ([#2133](https://github.com/EQEmu/Server/pull/2133)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-06
* Cleanup #revoke Command. ([#2134](https://github.com/EQEmu/Server/pull/2134)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #roambox Command. ([#2135](https://github.com/EQEmu/Server/pull/2135)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-06
* Cleanup #rules Command. ([#2593](https://github.com/EQEmu/Server/pull/2593)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-10
* Cleanup #save Command. ([#2136](https://github.com/EQEmu/Server/pull/2136)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-06
* Cleanup #scale Command. ([#2591](https://github.com/EQEmu/Server/pull/2591)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-05
* Cleanup #scribespell and #scribespells Commands. ([#2534](https://github.com/EQEmu/Server/pull/2534)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-14
* Cleanup #sensetrap Command. ([#2137](https://github.com/EQEmu/Server/pull/2137)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #serverinfo Command. ([#2568](https://github.com/EQEmu/Server/pull/2568)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-22
* Cleanup #serverrules Command. ([#2139](https://github.com/EQEmu/Server/pull/2139)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-06
* Cleanup #setlanguage Command. ([#2464](https://github.com/EQEmu/Server/pull/2464)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-10-13
* Cleanup #setskillall Command. ([#1992](https://github.com/EQEmu/Server/pull/1992)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-15
* Cleanup #shownpcgloballoot and #showzonegloballoot Command. ([#2141](https://github.com/EQEmu/Server/pull/2141)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-06
* Cleanup #showskills Command. ([#1994](https://github.com/EQEmu/Server/pull/1994)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-15
* Cleanup #spawneditmass Command. ([#2229](https://github.com/EQEmu/Server/pull/2229)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-06-04
* Cleanup #spawnfix Command. ([#2143](https://github.com/EQEmu/Server/pull/2143)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-06
* Cleanup #spawnstatus Command. ([#2144](https://github.com/EQEmu/Server/pull/2144)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-06
* Cleanup #summon Command. ([#2145](https://github.com/EQEmu/Server/pull/2145)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-06
* Cleanup #summonburiedplayercorpse Command. ([#2146](https://github.com/EQEmu/Server/pull/2146)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #suspend Command. ([#2564](https://github.com/EQEmu/Server/pull/2564)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-22
* Cleanup #task Command. ([#2071](https://github.com/EQEmu/Server/pull/2071)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-04-14
* Cleanup #time and #timezone Command. ([#2147](https://github.com/EQEmu/Server/pull/2147)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #timers Command. ([#2562](https://github.com/EQEmu/Server/pull/2562)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-22
* Cleanup #trapinfo Command. ([#2148](https://github.com/EQEmu/Server/pull/2148)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #ucs Command. ([#2149](https://github.com/EQEmu/Server/pull/2149)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #undye and #undyeme Commands. ([#1966](https://github.com/EQEmu/Server/pull/1966)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-10
* Cleanup #unscribespell Command. ([#1998](https://github.com/EQEmu/Server/pull/1998)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-16
* Cleanup #untraindisc Command. ([#1996](https://github.com/EQEmu/Server/pull/1996)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-16
* Cleanup #version Command. ([#1967](https://github.com/EQEmu/Server/pull/1967)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-10
* Cleanup #worldwide command. ([#2021](https://github.com/EQEmu/Server/pull/2021)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-03-01
* Cleanup #xtargets Command. ([#2545](https://github.com/EQEmu/Server/pull/2545)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-17
* Cleanup #zone and #zoneinstance Commands. ([#2202](https://github.com/EQEmu/Server/pull/2202)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-27
* Command Status Reload and Helper Method ([#2377](https://github.com/EQEmu/Server/pull/2377)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-08-20
* Consolidate #lock and #unlock Commands into #serverlock. ([#2193](https://github.com/EQEmu/Server/pull/2193)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-23
* Fix #copycharacter command crash ([#2446](https://github.com/EQEmu/Server/pull/2446)) ([Akkadius](https://github.com/Akkadius)) 2022-09-25
* Fix #killallnpcs from crashing ([#2037](https://github.com/EQEmu/Server/pull/2037)) ([Akkadius](https://github.com/Akkadius)) 2022-03-07
* Fix Flymode Command Help Prompt ([#2669](https://github.com/EQEmu/Server/pull/2669)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-23
* Fix typos in #ban and #ipban Commands. ([#2209](https://github.com/EQEmu/Server/pull/2209)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-26
* Make #damage require a target ([#2426](https://github.com/EQEmu/Server/pull/2426)) ([hgtw](https://github.com/hgtw)) 2022-09-05
* Nested Command Aliases ([#2636](https://github.com/EQEmu/Server/pull/2636)) ([Akkadius](https://github.com/Akkadius)) 2022-12-15
* Remove #guildapprove, #guildcreate, and #guildlist Commands ([#2775](https://github.com/EQEmu/Server/pull/2775)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-22
* Remove #iteminfo Command. ([#2565](https://github.com/EQEmu/Server/pull/2565)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-22
* Remove #profiledump and #profilereset Commands. ([#2546](https://github.com/EQEmu/Server/pull/2546)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-16
* Remove #undyeme Command. ([#2776](https://github.com/EQEmu/Server/pull/2776)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-22
* Remove unused #bestz and #pf Commands. ([#2112](https://github.com/EQEmu/Server/pull/2112)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Remove unused/broken #deletegraveyard and #setgraveyard Commands. ([#2198](https://github.com/EQEmu/Server/pull/2198)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-23

### Diawind

* Plus sign markdown fix ([#2727](https://github.com/EQEmu/Server/pull/2727)) ([Akkadius](https://github.com/Akkadius)) 2023-01-12

### Feature

* AA Cap Limit ([#2423](https://github.com/EQEmu/Server/pull/2423)) ([fryguy503](https://github.com/fryguy503)) 2022-10-13
* Add "Keeps Sold Items" Flag to NPCs ([#2671](https://github.com/EQEmu/Server/pull/2671)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-25
* Add Experience Gain Toggle. ([#2676](https://github.com/EQEmu/Server/pull/2676)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-30
* Add Guild Chat to Console. ([#2387](https://github.com/EQEmu/Server/pull/2387)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-08-22
* Add Hate Override for Heals ([#2485](https://github.com/EQEmu/Server/pull/2485)) ([Aeadoin](https://github.com/Aeadoin)) 2022-10-14
* Add Rule to Disable Group EXP Modifier. ([#2741](https://github.com/EQEmu/Server/pull/2741)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-15
* Add Support for "Show Mine Only" Filters ([#2484](https://github.com/EQEmu/Server/pull/2484)) ([Aeadoin](https://github.com/Aeadoin)) 2022-10-13
* Add Type 49545 to Spell Resistrictions ([#2436](https://github.com/EQEmu/Server/pull/2436)) ([Aeadoin](https://github.com/Aeadoin)) 2022-09-20
* Add humanoid and non-wielded restrictions to pick pocket ([#2276](https://github.com/EQEmu/Server/pull/2276)) ([noudess](https://github.com/noudess)) 2022-07-03
* Add player /inspect quest event ([#2508](https://github.com/EQEmu/Server/pull/2508)) ([hgtw](https://github.com/hgtw)) 2022-10-29
* Add special ability to block /open ([#2506](https://github.com/EQEmu/Server/pull/2506)) ([hgtw](https://github.com/hgtw)) 2022-10-29
* Allow Focus Effects to be Filtered out. ([#2447](https://github.com/EQEmu/Server/pull/2447)) ([Aeadoin](https://github.com/Aeadoin)) 2022-09-25
* Allow pets to zone with permanent (buffdurationformula 50) buffs to maintain them through zone transitions ([#2035](https://github.com/EQEmu/Server/pull/2035)) ([catapultam-habeo](https://github.com/catapultam-habeo)) 2022-03-07
* Bind Wound and Forage while mounted. ([#2257](https://github.com/EQEmu/Server/pull/2257)) ([fryguy503](https://github.com/fryguy503)) 2022-07-03
* Change #scribespells to be aware of spellgroups & ranks ([#2501](https://github.com/EQEmu/Server/pull/2501)) ([Aeadoin](https://github.com/Aeadoin)) 2022-11-06
* Change GetSkillDmgAmt to int32 ([#2364](https://github.com/EQEmu/Server/pull/2364)) ([Aeadoin](https://github.com/Aeadoin)) 2022-08-10
* Change Lifetap Emotes to be filterable. ([#2454](https://github.com/EQEmu/Server/pull/2454)) ([Aeadoin](https://github.com/Aeadoin)) 2022-09-29
* Change Mana Costs to use Signed Int ([#2384](https://github.com/EQEmu/Server/pull/2384)) ([Aeadoin](https://github.com/Aeadoin)) 2022-08-21
* Change mana_used to int32 ([#2321](https://github.com/EQEmu/Server/pull/2321)) ([Aeadoin](https://github.com/Aeadoin)) 2022-07-30
* Client Checksum Verification (Resubmit old 1678) ([#1922](https://github.com/EQEmu/Server/pull/1922)) ([noudess](https://github.com/noudess)) 2022-03-07
* EQ2-style implied targeting for spells. ([#2032](https://github.com/EQEmu/Server/pull/2032)) ([catapultam-habeo](https://github.com/catapultam-habeo)) 2022-03-07
* Faction Association ([#2408](https://github.com/EQEmu/Server/pull/2408)) ([mackal](https://github.com/mackal)) 2022-09-03
* GM State Change Persistance ([#2328](https://github.com/EQEmu/Server/pull/2328)) ([fryguy503](https://github.com/fryguy503)) 2022-07-31
* Implement Heroic Strikethrough to NPCs ([#2395](https://github.com/EQEmu/Server/pull/2395)) ([Aeadoin](https://github.com/Aeadoin)) 2022-08-31
* Implement OP_CashReward ([#2307](https://github.com/EQEmu/Server/pull/2307)) ([mackal](https://github.com/mackal)) 2022-07-15
* Instance Version Specific Experience Modifiers ([#2376](https://github.com/EQEmu/Server/pull/2376)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-08-20
* NPCs with bows and arrows do ranged attacks ([#2322](https://github.com/EQEmu/Server/pull/2322)) ([mackal](https://github.com/mackal)) 2022-07-30
* Soft Delete Bots on Character Soft Delete ([#2467](https://github.com/EQEmu/Server/pull/2467)) ([Aeadoin](https://github.com/Aeadoin)) 2022-10-13
* Spell Ranks will now work with AllowSpellMemorizeFromItem Rule ([#2475](https://github.com/EQEmu/Server/pull/2475)) ([Aeadoin](https://github.com/Aeadoin)) 2022-10-13
* Update HateMod used by SPA 114 to Int32. ([#2428](https://github.com/EQEmu/Server/pull/2428)) ([Aeadoin](https://github.com/Aeadoin)) 2022-09-08

### Fixes

* #npcstats command displaying incorrect faction ([#2710](https://github.com/EQEmu/Server/pull/2710)) ([noudess](https://github.com/noudess)) 2023-01-08
* #peqzone no longer bypass Handle_OP_ZoneChange ([#2063](https://github.com/EQEmu/Server/pull/2063)) ([Natedog2012](https://github.com/Natedog2012)) 2022-03-19
* #scribespells triggered error on mysql keyword rank ([#2779](https://github.com/EQEmu/Server/pull/2779)) ([noudess](https://github.com/noudess)) 2023-01-21
* #tune command various fixes ([#2046](https://github.com/EQEmu/Server/pull/2046)) ([KayenEQ](https://github.com/KayenEQ)) 2022-03-11
* Add Complete Heal Spell back to IsCompleteHealSpell Method ([#2722](https://github.com/EQEmu/Server/pull/2722)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-11
* Add SE_MakeDrunk to avoid error message. ([#2601](https://github.com/EQEmu/Server/pull/2601)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-01
* Add omitted function call in UCS ([#2768](https://github.com/EQEmu/Server/pull/2768)) ([Valorith](https://github.com/Valorith)) 2023-01-20
* Add required distance to CoTH before aggro wipe ([#2253](https://github.com/EQEmu/Server/pull/2253)) ([fryguy503](https://github.com/fryguy503)) 2022-07-03
* Adjustment for nullptr crash ([#2232](https://github.com/EQEmu/Server/pull/2232)) ([Akkadius](https://github.com/Akkadius)) 2022-06-01
* Alleviate some lag with crosszone/worldwide spell casting. ([#2016](https://github.com/EQEmu/Server/pull/2016)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-21
* Allow High Level Spells to be Unmemorized. ([#2641](https://github.com/EQEmu/Server/pull/2641)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-14
* Allow Songs to be scribed from scrolls ([#2460](https://github.com/EQEmu/Server/pull/2460)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-10-12
* AltCurrencySelectItemReply_Struct was not handled correctly. ([#2702](https://github.com/EQEmu/Server/pull/2702)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-04
* Any use of TempName left old clean_name. ([#1946](https://github.com/EQEmu/Server/pull/1946)) ([noudess](https://github.com/noudess)) 2022-01-26
* Avoid erase in discord queue range loop ([#2411](https://github.com/EQEmu/Server/pull/2411)) ([hgtw](https://github.com/hgtw)) 2022-09-03
* Bandolier didn't recognize source weapon on cursor ([#2026](https://github.com/EQEmu/Server/pull/2026)) ([noudess](https://github.com/noudess)) 2022-03-07
* Bard Invisible causing display issues. ([#2067](https://github.com/EQEmu/Server/pull/2067)) ([KayenEQ](https://github.com/KayenEQ)) 2022-04-01
* Bard update fixes 1 ([#1982](https://github.com/EQEmu/Server/pull/1982)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-09
* Bazaar Search MYSQL Error ([#2252](https://github.com/EQEmu/Server/pull/2252)) ([fryguy503](https://github.com/fryguy503)) 2022-06-08
* Blocked spells max spell id increased ([#2207](https://github.com/EQEmu/Server/pull/2207)) ([Isaaru](https://github.com/Isaaru)) 2022-05-25
* Boats should never get FixZ'd ([#2246](https://github.com/EQEmu/Server/pull/2246)) ([noudess](https://github.com/noudess)) 2022-07-02
* Clamp Item Ldon Sell Back Rates. ([#2592](https://github.com/EQEmu/Server/pull/2592)) ([Aeadoin](https://github.com/Aeadoin)) 2022-11-30
* Clear title/suffix bug fix. ([#2068](https://github.com/EQEmu/Server/pull/2068)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-04-02
* Correct (probably) unintended bitwise AND instead of logical AND ([#2239](https://github.com/EQEmu/Server/pull/2239)) ([Quintinon](https://github.com/Quintinon)) 2022-06-02
* Correct type signed/unsigned int when reading item from database in shareddb ([#2269](https://github.com/EQEmu/Server/pull/2269)) ([Quintinon](https://github.com/Quintinon)) 2022-06-15
* Data Bucket Permanent Duration String ([#2624](https://github.com/EQEmu/Server/pull/2624)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-06
* Delete NpcType Struct returned by Bot::CreateDefaultNPCTypeStructForBot() when unused ([#2267](https://github.com/EQEmu/Server/pull/2267)) ([Quintinon](https://github.com/Quintinon)) 2022-06-18
* Do not allow /open to be used on traps or auras, causes crash ([#1951](https://github.com/EQEmu/Server/pull/1951)) ([KayenEQ](https://github.com/KayenEQ)) 2022-01-30
* Doors::GetSize() Perl Croak Typo. ([#2027](https://github.com/EQEmu/Server/pull/2027)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-03-04
* EVENT_ENTER_AREA/EVENT_LEAVE_AREA. ([#2698](https://github.com/EQEmu/Server/pull/2698)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-02
* Edge case AA reset timer issue fixes ([#1995](https://github.com/EQEmu/Server/pull/1995)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-14
* Fix #door Save ([#2699](https://github.com/EQEmu/Server/pull/2699)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-02
* Fix #findaa and GetAAName(). ([#2774](https://github.com/EQEmu/Server/pull/2774)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-22
* Fix #zone 0. ([#2691](https://github.com/EQEmu/Server/pull/2691)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-01
* Fix Aug Clicks where item has no click effect. ([#2725](https://github.com/EQEmu/Server/pull/2725)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-12
* Fix Bot "Failed to Load" Messages. ([#2719](https://github.com/EQEmu/Server/pull/2719)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-11
* Fix Bot Group Loading ([#2780](https://github.com/EQEmu/Server/pull/2780)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-22
* Fix Bot ^spellsettingsadd command ([#2603](https://github.com/EQEmu/Server/pull/2603)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-01
* Fix Duplicate Silent Saylink Messages ([#2386](https://github.com/EQEmu/Server/pull/2386)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-08-22
* Fix EntityList::GetBotListByCharacterID() ([#2569](https://github.com/EQEmu/Server/pull/2569)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-22
* Fix Flag Updating with SetGMStatus() in Lua. ([#2554](https://github.com/EQEmu/Server/pull/2554)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-20
* Fix Group XP not working. ([#2748](https://github.com/EQEmu/Server/pull/2748)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-17
* Fix HP Regen Per Second. ([#2206](https://github.com/EQEmu/Server/pull/2206)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-25
* Fix IDFile Crash with spaces or invalid data. ([#2597](https://github.com/EQEmu/Server/pull/2597)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-01
* Fix IP Exemptions. ([#2189](https://github.com/EQEmu/Server/pull/2189)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-28
* Fix Instance Repository ([#2598](https://github.com/EQEmu/Server/pull/2598)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-01
* Fix Legacy Combat Lua Script ([#2226](https://github.com/EQEmu/Server/pull/2226)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-31
* Fix MovePC in #zone and #zoneinstance Commands. ([#2236](https://github.com/EQEmu/Server/pull/2236)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-06-01
* Fix NPC Reference in EVENT_SPAWN ([#2712](https://github.com/EQEmu/Server/pull/2712)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-08
* Fix ST_TargetsTarget Spells with Restrictions ([#2746](https://github.com/EQEmu/Server/pull/2746)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-15
* Fix Silent Saylinks Sending Message to Others. ([#2389](https://github.com/EQEmu/Server/pull/2389)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-08-22
* Fix Spell Bucket and Spell Global Logic Checks. ([#2285](https://github.com/EQEmu/Server/pull/2285)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-07-05
* Fix Spellinfo Command to work with SpellIDs above int16 ([#2437](https://github.com/EQEmu/Server/pull/2437)) ([Aeadoin](https://github.com/Aeadoin)) 2022-09-20
* Fix Strings::Money Missing Conditions. ([#2383](https://github.com/EQEmu/Server/pull/2383)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-08-21
* Fix Swarm Pet Flurry/Rampages Messages ([#2444](https://github.com/EQEmu/Server/pull/2444)) ([Aeadoin](https://github.com/Aeadoin)) 2022-09-25
* Fix bot compile locking client on server enter. ([#2210](https://github.com/EQEmu/Server/pull/2210)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-26
* Fix bot guild removal. ([#2194](https://github.com/EQEmu/Server/pull/2194)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-23
* Fix botgrouplist to display unique entries. ([#2785](https://github.com/EQEmu/Server/pull/2785)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-23
* Fix bots equipping augments. ([#2772](https://github.com/EQEmu/Server/pull/2772)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-22
* Fix case-sensitivity in #suspend Command. ([#2613](https://github.com/EQEmu/Server/pull/2613)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-05
* Fix duplicate and missing messages due to innate in spells ([#2170](https://github.com/EQEmu/Server/pull/2170)) ([noudess](https://github.com/noudess)) 2022-05-20
* Fix empty spawned merchants ([#2275](https://github.com/EQEmu/Server/pull/2275)) ([hgtw](https://github.com/hgtw)) 2022-06-28
* Fix for Bot command casting ([#1990](https://github.com/EQEmu/Server/pull/1990)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-12
* Fix for PR1954 target restriction with npcpc_only_flag from groupbuffs ([#1986](https://github.com/EQEmu/Server/pull/1986)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-10
* Fix for being able to skill up on corspe. ([#2058](https://github.com/EQEmu/Server/pull/2058)) ([noudess](https://github.com/noudess)) 2022-03-19
* Fix for castspell command ([#2010](https://github.com/EQEmu/Server/pull/2010)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-18
* Fix issue where #advnpcspawn addspawn does not add spawn sometimes. ([#2247](https://github.com/EQEmu/Server/pull/2247)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-06-07
* Fix issue where you can set your title to titles you don't have. ([#1917](https://github.com/EQEmu/Server/pull/1917)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-01-30
* Fix issue with Bot::LoadAndSpawnAllZonedBots. ([#2733](https://github.com/EQEmu/Server/pull/2733)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-15
* Fix issue with mobs summoning PCs into ceilings ([#1921](https://github.com/EQEmu/Server/pull/1921)) ([noudess](https://github.com/noudess)) 2022-01-30
* Fix loading world shared task state ([#2398](https://github.com/EQEmu/Server/pull/2398)) ([hgtw](https://github.com/hgtw)) 2022-08-28
* Fix luamod GetExperienceForKill return value  ([Cole-SoD](https://github.com/Cole-SoD)) 2023-01-12
* Fix memory leak in ucs ([#2409](https://github.com/EQEmu/Server/pull/2409)) ([hgtw](https://github.com/hgtw)) 2022-09-03
* Fix miscellaneous memory leaks related to EQApplicationPacket and it's pBuffer ([#2262](https://github.com/EQEmu/Server/pull/2262)) ([Quintinon](https://github.com/Quintinon)) 2022-07-03
* Fix null pointer crash on zones that have not booted a zone yet with #reload commands or anything that calls GetZoneDescription ([#2231](https://github.com/EQEmu/Server/pull/2231)) ([Akkadius](https://github.com/Akkadius)) 2022-06-01
* Fix possible crash in ProcessSpecialAbilities. ([#2630](https://github.com/EQEmu/Server/pull/2630)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-11
* Fix possible crash with zone name methods. ([#2055](https://github.com/EQEmu/Server/pull/2055)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-03-13
* Fix possible issue where variables have the same name. ([#2156](https://github.com/EQEmu/Server/pull/2156)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-08
* Fix quest::updatespawntimer() Perl croak. ([#1947](https://github.com/EQEmu/Server/pull/1947)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-01-26
* Fix recipient sound (vtell) on non-player races ([#2066](https://github.com/EQEmu/Server/pull/2066)) ([noudess](https://github.com/noudess)) 2022-04-02
* Fix scenario where dereferenced object could be null. ([#2784](https://github.com/EQEmu/Server/pull/2784)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-23
* Fix stack leaks in Lua events #2254  ([hgtw](https://github.com/hgtw)) 2022-06-09
* Fix trading with bots when in an illusion. ([#2645](https://github.com/EQEmu/Server/pull/2645)) ([nytmyr](https://github.com/nytmyr)) 2022-12-15
* Fix two invalid data accesses in zone/client.cpp ([#2238](https://github.com/EQEmu/Server/pull/2238)) ([Quintinon](https://github.com/Quintinon)) 2022-06-07
* Fixed Spell Logic for Bot Nukes ([#2481](https://github.com/EQEmu/Server/pull/2481)) ([Aeadoin](https://github.com/Aeadoin)) 2022-10-13
* Fixed message on promote/demote permissions check. ([#2700](https://github.com/EQEmu/Server/pull/2700)) ([Valorith](https://github.com/Valorith)) 2023-01-02
* Fixed several instances of incorrect comparision - & executes after == ([#2025](https://github.com/EQEmu/Server/pull/2025)) ([noudess](https://github.com/noudess)) 2022-03-07
* Force NPCs to respect special ability 24 and 50 when set on player pets ([#2059](https://github.com/EQEmu/Server/pull/2059)) ([Natedog2012](https://github.com/Natedog2012)) 2022-03-16
* Free return value of ZoneDatabase::LoadTraderItemWithCharges() ([#2264](https://github.com/EQEmu/Server/pull/2264)) ([Quintinon](https://github.com/Quintinon)) 2022-06-18
* Hacker_Str was causing sql errors - Non Escaped ([#2251](https://github.com/EQEmu/Server/pull/2251)) ([fryguy503](https://github.com/fryguy503)) 2022-06-08
* Handle memory leaks from return value of Client::GetTraderItems() ([#2266](https://github.com/EQEmu/Server/pull/2266)) ([Quintinon](https://github.com/Quintinon)) 2022-07-03
* Handle_OP_AugmentItem could cause Zone crash ([#2750](https://github.com/EQEmu/Server/pull/2750)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-17
* HasPet() Zone Crashes ([#2744](https://github.com/EQEmu/Server/pull/2744)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-15
* Illusions will now properly display armor to other clients when they zone in. ([#1958](https://github.com/EQEmu/Server/pull/1958)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-04
* Instrument Mods should not affect spells that change model size. ([#2072](https://github.com/EQEmu/Server/pull/2072)) ([KayenEQ](https://github.com/KayenEQ)) 2022-04-13
* Invisible will display as dropped now on air pets when they attack. ([#2042](https://github.com/EQEmu/Server/pull/2042)) ([KayenEQ](https://github.com/KayenEQ)) 2022-03-07
* IsDamage test for lifetap was not complete. ([#2213](https://github.com/EQEmu/Server/pull/2213)) ([noudess](https://github.com/noudess)) 2022-05-27
* Limit merchant temp item list to zone and instance ([#2346](https://github.com/EQEmu/Server/pull/2346)) ([mackal](https://github.com/mackal)) 2022-07-31
* Lua GetBlockNextSpell() no return. ([#2151](https://github.com/EQEmu/Server/pull/2151)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Make Perl TakeMoneyFromPP int64 ([#2158](https://github.com/EQEmu/Server/pull/2158)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-08
* Missing break ([#2031](https://github.com/EQEmu/Server/pull/2031)) ([KayenEQ](https://github.com/KayenEQ)) 2022-03-04
* Move EVENT_SPAWN for adding NPCs back to original spot, also add NPCs… ([#2749](https://github.com/EQEmu/Server/pull/2749)) ([Natedog2012](https://github.com/Natedog2012)) 2023-01-17
* NPC Constructor was passing hp_regen_per_second out of order to Mob(). ([#2681](https://github.com/EQEmu/Server/pull/2681)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-29
* NPC::CountItem and Corpse::CountItem 0 Charge Item Fix. ([#1959](https://github.com/EQEmu/Server/pull/1959)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-04
* NPC::GetNPCStat has no default return. ([#2150](https://github.com/EQEmu/Server/pull/2150)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* NPCs were getting weapon proc added twice ([#2277](https://github.com/EQEmu/Server/pull/2277)) ([noudess](https://github.com/noudess)) 2022-07-07
* Objects::GetTiltX() and Objects::GetTiltY() Perl Croak Typos. ([#2028](https://github.com/EQEmu/Server/pull/2028)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-03-04
* PR 1982 ([#1985](https://github.com/EQEmu/Server/pull/1985)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-10
* PR 2032 would lock client on casting fail as written ([#2038](https://github.com/EQEmu/Server/pull/2038)) ([KayenEQ](https://github.com/KayenEQ)) 2022-03-07
* Remove StringUtilTest::EscapeStringMemoryTest ([#2310](https://github.com/EQEmu/Server/pull/2310)) ([mackal](https://github.com/mackal)) 2022-07-15
* Remove Unnecessary Attack Log ([#2643](https://github.com/EQEmu/Server/pull/2643)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-14
* Remove unnecessary log messages. ([#2642](https://github.com/EQEmu/Server/pull/2642)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-14
* Removed Lua Event Argument Dispatch.  ([Kinglykrab](https://github.com/Kinglykrab)) 2022-03-01
* Resolve Warning due to Virtual Mob Method GetInv() ([#2650](https://github.com/EQEmu/Server/pull/2650)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-19
* Resolve XP Calculation Bug introduced w/ recent Rule addition ([#2703](https://github.com/EQEmu/Server/pull/2703)) ([Valorith](https://github.com/Valorith)) 2023-01-07
* Resolve logic error in Raid::QueueClients ([#2404](https://github.com/EQEmu/Server/pull/2404)) ([mackal](https://github.com/mackal)) 2022-09-01
* Resolve subroutine redefinition due to bot methods. ([#2117](https://github.com/EQEmu/Server/pull/2117)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Restore missing messages for lifetap and dmg spells. ([#2057](https://github.com/EQEmu/Server/pull/2057)) ([noudess](https://github.com/noudess)) 2022-04-14
* Shared Memory Faction Association Typo ([#2419](https://github.com/EQEmu/Server/pull/2419)) ([mackal](https://github.com/mackal)) 2022-09-03
* Spell Buckets/Globals SQL Escape. ([#2019](https://github.com/EQEmu/Server/pull/2019)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-26
* Spell Buckets/Globals did not allow string-based values. ([#2043](https://github.com/EQEmu/Server/pull/2043)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-03-09
* Stop skill ups on Charmed NPCs. ([#2249](https://github.com/EQEmu/Server/pull/2249)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-06-10
* Summon Companion causing pets to warps away. ([#1972](https://github.com/EQEmu/Server/pull/1972)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-08
* Touch Of Vinitras was ignoring pet DT rule ([#2469](https://github.com/EQEmu/Server/pull/2469)) ([Aeadoin](https://github.com/Aeadoin)) 2022-10-03
* Tradeskill Autocombine MinSkill ([#2260](https://github.com/EQEmu/Server/pull/2260)) ([fryguy503](https://github.com/fryguy503)) 2022-06-10
* Tradeskill Item 0 Error ([#2256](https://github.com/EQEmu/Server/pull/2256)) ([fryguy503](https://github.com/fryguy503)) 2022-06-10
* Zone Flags Regression ([#2760](https://github.com/EQEmu/Server/pull/2760)) ([Akkadius](https://github.com/Akkadius)) 2023-01-19
* checking casting_spell_slot before its defined is bad ([#2013](https://github.com/EQEmu/Server/pull/2013)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-20
* manifest for db version 9176 had incorrect field name ([#2062](https://github.com/EQEmu/Server/pull/2062)) ([noudess](https://github.com/noudess)) 2022-03-19
* quest::MovePCInstance() Arguments Fix. ([#2020](https://github.com/EQEmu/Server/pull/2020)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-27

### Hotfix

* Add Bazaar portal discs to SQL  ([Akkadius](https://github.com/Akkadius)) 2022-09-05
* Add discord_webhooks to server tables  ([Akkadius](https://github.com/Akkadius)) 2022-07-03
* Blocks are nested too deeply. ([#2689](https://github.com/EQEmu/Server/pull/2689)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-01
* Cleanup #questerrors Command. ([#2116](https://github.com/EQEmu/Server/pull/2116)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Compiling fails on FMT 9.1 with Bots ([#2665](https://github.com/EQEmu/Server/pull/2665)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-21
* Correct database call to point to the content_db connection  ([Akkadius](https://github.com/Akkadius)) 2022-06-12
* Corrected misnamed Database Query file for Experience Toggle ([#2683](https://github.com/EQEmu/Server/pull/2683)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-31
* Faction associations file naming / lock consistency  ([Akkadius](https://github.com/Akkadius)) 2022-09-05
* Fix DB version merge  ([Akkadius](https://github.com/Akkadius)) 2022-05-08
* Fix door click crash issue if destination zone doesn't exist  ([Akkadius](https://github.com/Akkadius)) 2023-01-20
* Fix issue with Bot Loading with 0 Health causing buffs to be lost. ([#2552](https://github.com/EQEmu/Server/pull/2552)) ([Aeadoin](https://github.com/Aeadoin)) 2022-11-18
* Fix lua mod load path  ([Akkadius](https://github.com/Akkadius)) 2022-09-29
* Fix merge issue  ([Akkadius](https://github.com/Akkadius)) 2022-07-14
* Fix path load ordering for CLI commands  ([Akkadius](https://github.com/Akkadius)) 2022-10-16
* Fix potential race for crash dumps (Linux)  ([Akkadius](https://github.com/Akkadius)) 2022-07-31
* Fix regression caused by #2129  ([Akkadius](https://github.com/Akkadius)) 2022-05-09
* Flipped positive / negative values for legacy_combat.lua  ([Akkadius](https://github.com/Akkadius)) 2022-06-09
* Force collation on conversion script  ([Akkadius](https://github.com/Akkadius)) 2022-09-28
* Instances Repository Fix ([#2576](https://github.com/EQEmu/Server/pull/2576)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-26
* Login Server failing to compile on Windows. ([#2758](https://github.com/EQEmu/Server/pull/2758)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-19
* Lua Parser Needs Lua_ItemInst ([#2696](https://github.com/EQEmu/Server/pull/2696)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-01
* Make sure we have a proper split size before assuming we can split it  ([Akkadius](https://github.com/Akkadius)) 2023-01-20
* Move discord_webhooks to state tables because we don't want webhooks being exported  ([Akkadius](https://github.com/Akkadius)) 2022-07-03
* Possible windows compile fix  ([Akkadius](https://github.com/Akkadius)) 2022-07-07
* Possible windows compile fix take 2  ([Akkadius](https://github.com/Akkadius)) 2022-07-07
* Remove appveyor fetch bots  ([Akkadius](https://github.com/Akkadius)) 2023-01-21
* Remove expansion field from account for those who have it ([#2357](https://github.com/EQEmu/Server/pull/2357)) ([Akkadius](https://github.com/Akkadius)) 2022-08-01
* Resolve Zone Crashing when grouped with Bots. ([#2747](https://github.com/EQEmu/Server/pull/2747)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-16
* Resolve issue with Bot Casting after zoning. ([#2617](https://github.com/EQEmu/Server/pull/2617)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-04
* Return weather_type_map  ([Akkadius](https://github.com/Akkadius)) 2022-10-14
* SQL Update  ([Akkadius](https://github.com/Akkadius)) 2022-07-31
* Shared Memory Protection Fixes  ([Akkadius](https://github.com/Akkadius)) 2022-07-27
* Windows compile fix take 3 (final)  ([Akkadius](https://github.com/Akkadius)) 2022-07-07
* fix manifest  ([Akkadius](https://github.com/Akkadius)) 2022-07-16

### Logging

* Add stack trace in code paths that shouldn't occur ([#2453](https://github.com/EQEmu/Server/pull/2453)) ([Akkadius](https://github.com/Akkadius)) 2022-09-28
* Cleanup AI Logging Events ([#2615](https://github.com/EQEmu/Server/pull/2615)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-04
* Fix log messages to final damage values ([#2056](https://github.com/EQEmu/Server/pull/2056)) ([noudess](https://github.com/noudess)) 2022-03-14
* Fix zoning log typo ([#2478](https://github.com/EQEmu/Server/pull/2478)) ([Akkadius](https://github.com/Akkadius)) 2022-10-11
* Force crash logs to always be on regardless of setting ([#2762](https://github.com/EQEmu/Server/pull/2762)) ([Akkadius](https://github.com/Akkadius)) 2023-01-20
* Improvements to GM Say Logging ([#2765](https://github.com/EQEmu/Server/pull/2765)) ([Akkadius](https://github.com/Akkadius)) 2023-01-20
* Logging Improvements ([#2755](https://github.com/EQEmu/Server/pull/2755)) ([Akkadius](https://github.com/Akkadius)) 2023-01-18
* More AI Logging Cleanup ([#2616](https://github.com/EQEmu/Server/pull/2616)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-10
* Netcode Logging Unify ([#2443](https://github.com/EQEmu/Server/pull/2443)) ([Akkadius](https://github.com/Akkadius)) 2022-09-28
* Remove function prefixes ([#2766](https://github.com/EQEmu/Server/pull/2766)) ([Akkadius](https://github.com/Akkadius)) 2023-01-20
* Remove loginserver unhandled error ([#2458](https://github.com/EQEmu/Server/pull/2458)) ([Akkadius](https://github.com/Akkadius)) 2022-09-29
* Reset stream so we don't bold the whole line  ([Akkadius](https://github.com/Akkadius)) 2023-01-18
* Table Injection - Member Variable Cleanup ([#2281](https://github.com/EQEmu/Server/pull/2281)) ([Akkadius](https://github.com/Akkadius)) 2022-07-07
* Update BUILD_LOGGING=false Blank Aliases ([#2083](https://github.com/EQEmu/Server/pull/2083)) ([Akkadius](https://github.com/Akkadius)) 2022-05-01

### Luamod

* Add CalcSpellEffectValue_formula to luamods ([#2721](https://github.com/EQEmu/Server/pull/2721)) ([Natedog2012](https://github.com/Natedog2012)) 2023-01-11

### Mercs

* Add Mercenary Support ([#2745](https://github.com/EQEmu/Server/pull/2745)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-17

### Optimization

* Handle channel name filter checks in memory ([#2767](https://github.com/EQEmu/Server/pull/2767)) ([Valorith](https://github.com/Valorith)) 2023-01-20

### QS

* Database class name change ([#2743](https://github.com/EQEmu/Server/pull/2743)) ([Akkadius](https://github.com/Akkadius)) 2023-01-15

### Quest API

* Add AddAISpellEffect(spell_effect_id, base_value, limit_value, max_value) and RemoveAISpellEffect(spell_effect_id) to Lua. ([#1981](https://github.com/EQEmu/Server/pull/1981)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-09
* Add AddItem() to Perl/Lua. ([#2054](https://github.com/EQEmu/Server/pull/2054)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-03-13
* Add AddPlatinum(), GetCarriedPlatinum() and TakePlatinum() to Perl/Lua. ([#2079](https://github.com/EQEmu/Server/pull/2079)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-04-30
* Add Area Damage Methods to Perl/Lua. ([#2549](https://github.com/EQEmu/Server/pull/2549)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-17
* Add Augment Slot Type/Visible to GetItemStat ([#2686](https://github.com/EQEmu/Server/pull/2686)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-01
* Add Bot Methods to Lua. ([#2731](https://github.com/EQEmu/Server/pull/2731)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-14
* Add Bot::Camp() to Perl/Lua. ([#2718](https://github.com/EQEmu/Server/pull/2718)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-11
* Add BuffCount() Overloads to Perl/Lua. ([#2679](https://github.com/EQEmu/Server/pull/2679)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-29
* Add CampAllBots() to Perl/Lua. ([#2732](https://github.com/EQEmu/Server/pull/2732)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-14
* Add Charges/Augment/Attuned Support to Varlink. ([#2685](https://github.com/EQEmu/Server/pull/2685)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-01
* Add CheckNameFilter to Perl/Lua. ([#2175](https://github.com/EQEmu/Server/pull/2175)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-20
* Add Client Augment Events to Perl/Lua. ([#2735](https://github.com/EQEmu/Server/pull/2735)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-15
* Add Client Spell Methods to Perl/Lua. ([#2550](https://github.com/EQEmu/Server/pull/2550)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-22
* Add CloneAppearance() to Perl/Lua. ([#2531](https://github.com/EQEmu/Server/pull/2531)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-14
* Add CopyHateList() to Perl/Lua. ([#2623](https://github.com/EQEmu/Server/pull/2623)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-06
* Add Corpse::AddItem overloads for Lua ([#2509](https://github.com/EQEmu/Server/pull/2509)) ([hgtw](https://github.com/hgtw)) 2022-10-29
* Add Despawn Events to Perl/Lua. ([#2707](https://github.com/EQEmu/Server/pull/2707)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-07
* Add DoAnim Overloads to Perl/Lua. ([#2627](https://github.com/EQEmu/Server/pull/2627)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-10
* Add DoAugmentSlotsMatch() to Perl/Lua. ([#2687](https://github.com/EQEmu/Server/pull/2687)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-01
* Add DoesAugmentFit() to Perl/Lua. ([#2688](https://github.com/EQEmu/Server/pull/2688)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-01
* Add Door Methods to Perl/Lua. ([#2724](https://github.com/EQEmu/Server/pull/2724)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-12
* Add EVENT_AA_BUY and EVENT_AA_GAIN to Perl/Lua. ([#2504](https://github.com/EQEmu/Server/pull/2504)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-05
* Add EVENT_BOT_CREATE to Perl/Lua ([#2713](https://github.com/EQEmu/Server/pull/2713)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-09
* Add EVENT_CAST_ON exports to EVENT_CAST and EVENT_CAST_BEGIN. ([#2051](https://github.com/EQEmu/Server/pull/2051)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-03-12
* Add EVENT_EQUIP_ITEM_CLIENT and EVENT_UNEQUIP_ITEM_CLIENT to Perl/Lua. ([#2015](https://github.com/EQEmu/Server/pull/2015)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-23
* Add EVENT_GM_COMMAND to Perl/Lua. ([#2634](https://github.com/EQEmu/Server/pull/2634)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-11
* Add EVENT_LEVEL_DOWN to Perl/Lua. ([#2620](https://github.com/EQEmu/Server/pull/2620)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-11
* Add EVENT_PAYLOAD to Perl/Lua. ([#2611](https://github.com/EQEmu/Server/pull/2611)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-04
* Add EVENT_SKILL_UP & EVENT_LANGUAGE_SKILL_UP to Perl/Lua ([#2076](https://github.com/EQEmu/Server/pull/2076)) ([nytmyr](https://github.com/nytmyr)) 2022-04-25
* Add Entity Variable Methods to Perl/Lua. ([#2609](https://github.com/EQEmu/Server/pull/2609)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-04
* Add Fling Overloads to Perl/Lua. ([#2622](https://github.com/EQEmu/Server/pull/2622)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-06
* Add GetAugmentIDsBySlotID() to Perl/Lua. ([#2673](https://github.com/EQEmu/Server/pull/2673)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-25
* Add GetAverageLevel() to Perl/Lua. ([#2524](https://github.com/EQEmu/Server/pull/2524)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-14
* Add GetBotItem() and GetBotItemIDBySlot() to Perl/Lua. ([#2350](https://github.com/EQEmu/Server/pull/2350)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-07-31
* Add GetBotListByCharacterID() to Perl/Lua. ([#2069](https://github.com/EQEmu/Server/pull/2069)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-04-02
* Add GetBotListByClientName() Class Overload to Perl/Lua. ([#2577](https://github.com/EQEmu/Server/pull/2577)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-26
* Add GetBotListByClientName(client_name) to Perl/Lua. ([#2064](https://github.com/EQEmu/Server/pull/2064)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-03-23
* Add GetEnvironmentalDamageName() to Perl/Lua. ([#1964](https://github.com/EQEmu/Server/pull/1964)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-10
* Add GetGMStatus() to Perl/Lua. ([#2448](https://github.com/EQEmu/Server/pull/2448)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-09-28
* Add GetGuildPublicNote() to Perl/Lua. ([#2608](https://github.com/EQEmu/Server/pull/2608)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-04
* Add GetHealAmount() and GetSpellDamage() to Perl/Lua. ([#2165](https://github.com/EQEmu/Server/pull/2165)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-11
* Add GetLeader() and GetLeaderName() to Perl/Lua. ([#2701](https://github.com/EQEmu/Server/pull/2701)) ([Valorith](https://github.com/Valorith)) 2023-01-04
* Add GetLowestLevel() to Perl. ([#2517](https://github.com/EQEmu/Server/pull/2517)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-05
* Add GetRandomBot() to Perl/Lua ([#2543](https://github.com/EQEmu/Server/pull/2543)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-16
* Add GetRandomClient(), GetRandomMob() and GetRandomNPC() overloads to Perl/Lua. ([#2541](https://github.com/EQEmu/Server/pull/2541)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-14
* Add GetRandomMob() and GetRandomNPC() to Perl/Lua. ([#2006](https://github.com/EQEmu/Server/pull/2006)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-17
* Add GetSkillDmgAmt() to Perl. ([#2365](https://github.com/EQEmu/Server/pull/2365)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-08-10
* Add GetUltimateOwner() to Perl/Lua. ([#2516](https://github.com/EQEmu/Server/pull/2516)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-05
* Add Goto Player Teleport Methods. ([#2379](https://github.com/EQEmu/Server/pull/2379)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-08-20
* Add Group/Raid Overloads to Perl/Lua. ([#2587](https://github.com/EQEmu/Server/pull/2587)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-27
* Add Group/Raid overloads to Perl/Lua. ([#2526](https://github.com/EQEmu/Server/pull/2526)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-14
* Add HasBotSpellEntry() to Perl/Lua. ([#2563](https://github.com/EQEmu/Server/pull/2563)) ([Aeadoin](https://github.com/Aeadoin)) 2022-11-25
* Add Hotzone Methods to Perl/Lua. ([#2558](https://github.com/EQEmu/Server/pull/2558)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-22
* Add Instance ID/Version exports to EVENT_ZONE. ([#2502](https://github.com/EQEmu/Server/pull/2502)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-05
* Add Instance Methods to Perl/Lua. ([#2573](https://github.com/EQEmu/Server/pull/2573)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-26
* Add IsAttackAllowed() to Perl/Lua. ([#2672](https://github.com/EQEmu/Server/pull/2672)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-25
* Add IsRaining() and IsSnowing() to Perl/Lua. ([#2477](https://github.com/EQEmu/Server/pull/2477)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-10-14
* Add IsRareSpawn() to Perl/Lua. ([#2338](https://github.com/EQEmu/Server/pull/2338)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-07-30
* Add Lua handlers for zone controller events ([#2514](https://github.com/EQEmu/Server/pull/2514)) ([hgtw](https://github.com/hgtw)) 2022-11-05
* Add Marquee methods to Perl/Lua. ([#2544](https://github.com/EQEmu/Server/pull/2544)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-16
* Add MaxSkills() to Perl/Lua. ([#2621](https://github.com/EQEmu/Server/pull/2621)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-06
* Add Merchant Events to Perl/Lua. ([#2452](https://github.com/EQEmu/Server/pull/2452)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-09-28
* Add Mob Hate Methods to Perl/Lua. ([#2548](https://github.com/EQEmu/Server/pull/2548)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-16
* Add Overloads to MoveZone Methods in Perl/Lua. ([#2551](https://github.com/EQEmu/Server/pull/2551)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-22
* Add Owner methods to Perl/Lua. ([#2542](https://github.com/EQEmu/Server/pull/2542)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-14
* Add Popup methods to Perl/Lua. ([#2533](https://github.com/EQEmu/Server/pull/2533)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-14
* Add Proximity Range Methods to Perl/Lua. ([#2572](https://github.com/EQEmu/Server/pull/2572)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-26
* Add RandomizeFeature() overloads to Perl/Lua. ([#2532](https://github.com/EQEmu/Server/pull/2532)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-14
* Add Recipe Methods ([#2393](https://github.com/EQEmu/Server/pull/2393)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-08-23
* Add ResetAlternateAdvancementRank() to Perl/Lua. ([#2510](https://github.com/EQEmu/Server/pull/2510)) ([hgtw](https://github.com/hgtw)) 2022-10-29
* Add ResetDecayTimer() to Perl/Lua. ([#2520](https://github.com/EQEmu/Server/pull/2520)) ([hgtw](https://github.com/hgtw)) 2022-11-06
* Add SendGMCommand() to Perl/Lua. ([#2527](https://github.com/EQEmu/Server/pull/2527)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-14
* Add SendPath() to Perl/Lua. ([#2740](https://github.com/EQEmu/Server/pull/2740)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-15
* Add SignalAllBotsByOwnerName() to Perl/Lua. ([#2730](https://github.com/EQEmu/Server/pull/2730)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-13
* Add SplitMoney() with Client splitter to Perl. ([#2525](https://github.com/EQEmu/Server/pull/2525)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-14
* Add TaskSelector to Perl/Lua. ([#2177](https://github.com/EQEmu/Server/pull/2177)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-15
* Add Time String to Seconds Method to Perl/Lua. ([#2580](https://github.com/EQEmu/Server/pull/2580)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-27
* Add TrackNPC to Perl/Lua. ([#2272](https://github.com/EQEmu/Server/pull/2272)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-06-29
* Add WearChange Overloads to Perl/Lua. ([#2600](https://github.com/EQEmu/Server/pull/2600)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-01
* Add Zone Flag Methods to Perl/Lua. ([#2574](https://github.com/EQEmu/Server/pull/2574)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-26
* Add apis to end shared tasks ([#2521](https://github.com/EQEmu/Server/pull/2521)) ([hgtw](https://github.com/hgtw)) 2022-11-06
* Add caster_id and caster_level export to EVENT_CAST_ON in Perl/Lua. ([#2049](https://github.com/EQEmu/Server/pull/2049)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-03-11
* Add commify to Perl/Lua. ([#2099](https://github.com/EQEmu/Server/pull/2099)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-03
* Add inventory->CountItemEquippedByID(item_id) and inventory->HasItemEquippedByID(item_id) to Perl/Lua. ([#1963](https://github.com/EQEmu/Server/pull/1963)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-06
* Add missing methods/package.adds to Perl API. ([#2287](https://github.com/EQEmu/Server/pull/2287)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-07-05
* Add multiple inventory method short hands to client. ([#2078](https://github.com/EQEmu/Server/pull/2078)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-04-30
* Add option to Ignore Mods to CalcEXP ([#2704](https://github.com/EQEmu/Server/pull/2704)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-06
* Adjustment to depop_all function. ([#2595](https://github.com/EQEmu/Server/pull/2595)) ([fryguy503](https://github.com/fryguy503)) 2022-11-30
* Allow CreateInstance to be used without a Client initiator. ([#2399](https://github.com/EQEmu/Server/pull/2399)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-08-28
* Allow EVENT_ZONE to be parsed as non-zero to prevent zoning. ([#2052](https://github.com/EQEmu/Server/pull/2052)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-03-12
* Allow scripts to prevent door click ([#2327](https://github.com/EQEmu/Server/pull/2327)) ([hgtw](https://github.com/hgtw)) 2022-07-27
* Cleanup Proximity Events ([#2697](https://github.com/EQEmu/Server/pull/2697)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-02
* Cleanup Signal Methods in Perl/Lua. ([#2604](https://github.com/EQEmu/Server/pull/2604)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-04
* Expand Bot quest API functionality. ([#2096](https://github.com/EQEmu/Server/pull/2096)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-04
* Expand SaveGuardSpot ([#2258](https://github.com/EQEmu/Server/pull/2258)) ([fryguy503](https://github.com/fryguy503)) 2022-06-10
* Export corpse in EVENT_DEATH_COMPLETE ([#2519](https://github.com/EQEmu/Server/pull/2519)) ([hgtw](https://github.com/hgtw)) 2022-11-06
* Export killed XYZH to EVENT_DEATH_ZONE in Perl. ([#2050](https://github.com/EQEmu/Server/pull/2050)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-03-12
* Fix Lua Door/Object Create Methods. ([#2633](https://github.com/EQEmu/Server/pull/2633)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-11
* Fix Perl EVENT_HP double parsing in Spire. ([#2585](https://github.com/EQEmu/Server/pull/2585)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-27
* Fix lua task selector count when over max ([#2353](https://github.com/EQEmu/Server/pull/2353)) ([hgtw](https://github.com/hgtw)) 2022-07-31
* Fix missing arg in perl set_proximity ([#2291](https://github.com/EQEmu/Server/pull/2291)) ([hgtw](https://github.com/hgtw)) 2022-07-09
* Fix parameters in some Perl worldwide methods. ([#2224](https://github.com/EQEmu/Server/pull/2224)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-31
* Let HasQuestSub check encounters ([#2435](https://github.com/EQEmu/Server/pull/2435)) ([hgtw](https://github.com/hgtw)) 2022-09-20
* Perl Doors Fix. ([#2288](https://github.com/EQEmu/Server/pull/2288)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-07-05
* Perl Money Fixes. ([#2098](https://github.com/EQEmu/Server/pull/2098)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-04
* Send delivered task items in trade events ([#2518](https://github.com/EQEmu/Server/pull/2518)) ([hgtw](https://github.com/hgtw)) 2022-11-06
* Use Floating Point for CameraEffect Intensity ([#2337](https://github.com/EQEmu/Server/pull/2337)) ([hgtw](https://github.com/hgtw)) 2022-07-31
* Use binding library for perl apis ([#2216](https://github.com/EQEmu/Server/pull/2216)) ([hgtw](https://github.com/hgtw)) 2022-07-04

### Rules

* Add Backstab Rules ([#2666](https://github.com/EQEmu/Server/pull/2666)) ([Valorith](https://github.com/Valorith)) 2022-12-21
* Add Frontal Stun Immunity Rules. ([#2217](https://github.com/EQEmu/Server/pull/2217)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-06-07
* Add Keep Level on Death ([#2319](https://github.com/EQEmu/Server/pull/2319)) ([trentdm](https://github.com/trentdm)) 2022-07-30
* Add LDoN Loot Count Modifier Rule ([#2694](https://github.com/EQEmu/Server/pull/2694)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-03
* Add ManaOnDeath and EndurOnDeath ([#2661](https://github.com/EQEmu/Server/pull/2661)) ([fryguy503](https://github.com/fryguy503)) 2022-12-20
* Add Rule to Disable NPC Last Names. ([#2227](https://github.com/EQEmu/Server/pull/2227)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-06-04
* Add Rule to Enable Tells with #hideme ([#2358](https://github.com/EQEmu/Server/pull/2358)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-08-04
* Add Rule to allow Assassinate on non-Humanoid body types. ([#2331](https://github.com/EQEmu/Server/pull/2331)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-07-29
* Add Rule to allow Headshots on non-Humanoid body types. ([#2329](https://github.com/EQEmu/Server/pull/2329)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-07-29
* Add Rules to disable various item functionalities and cleanup data types. ([#2225](https://github.com/EQEmu/Server/pull/2225)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-06-01
* Add Spells:BuffsFadeOnDeath. ([#2200](https://github.com/EQEmu/Server/pull/2200)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-27
* Add Spells:IllusionsAlwaysPersist. ([#2199](https://github.com/EQEmu/Server/pull/2199)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-27
* Add Toggle for Warrior Shielding ([#2496](https://github.com/EQEmu/Server/pull/2496)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-10-22
* Add adjustment for zone forage. ([#2330](https://github.com/EQEmu/Server/pull/2330)) ([fryguy503](https://github.com/fryguy503)) 2022-07-30
* Add rule for NPC Level Based Buff Restrictions. ([#2708](https://github.com/EQEmu/Server/pull/2708)) ([noudess](https://github.com/noudess)) 2023-01-15
* Add rule to allow players to permanently save chat channels to database, up to a limit. ([#2706](https://github.com/EQEmu/Server/pull/2706)) ([Valorith](https://github.com/Valorith)) 2023-01-19
* Change TradeskillUp Rules to be Floats ([#2674](https://github.com/EQEmu/Server/pull/2674)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-25
* Cleanup all unused rules. ([#2184](https://github.com/EQEmu/Server/pull/2184)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-23
* Rule Gate Pet Zoning ([#2625](https://github.com/EQEmu/Server/pull/2625)) ([fryguy503](https://github.com/fryguy503)) 2022-12-07
* Rule to allow cap on % XP gain per kill ([#2667](https://github.com/EQEmu/Server/pull/2667)) ([Valorith](https://github.com/Valorith)) 2022-12-25
* Update logic checks everywhere for FVNoDropFlag. ([#2179](https://github.com/EQEmu/Server/pull/2179)) ([Quintinon](https://github.com/Quintinon)) 2022-07-30

### SQL

* Bugs Table Migration  (#2602) ([#2559](https://github.com/EQEmu/Server/pull/2559)) ([joligario](https://github.com/joligario)) 2022-12-01
* Update 2023_01_15_merc_data.sql ([#2763](https://github.com/EQEmu/Server/pull/2763)) ([joligario](https://github.com/joligario)) 2023-01-20

### UCS

* Auto Client Reconnection ([#2154](https://github.com/EQEmu/Server/pull/2154)) ([Akkadius](https://github.com/Akkadius)) 2022-05-08

### Websocket

* Fix cpp20/gcc11 compile failure ([#2737](https://github.com/EQEmu/Server/pull/2737)) ([Akkadius](https://github.com/Akkadius)) 2023-01-15

### Zone Flags

* Use database connection, not content connection ([#2759](https://github.com/EQEmu/Server/pull/2759)) ([Akkadius](https://github.com/Akkadius)) 2023-01-19
