/*	EQEmu: EQEmulator

	Copyright (C) 2001-2026 EQEmu Development Team

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

/*

missing:
OP_TGB
OP_GuildMemberList

*/


//opcode_dispatch.h

/*

There are currently eight (times two) dispatch modes possible, the different
modes for a given state differ only in the arguments which
are passed to the dispatch function.

The variable length versions ensure that the packet is
AT LEAST as long as the supplied structure.

The variable length versions should only be used when the packet
is truely variable length, and usage of the raw versions should
be avoided as much as reasonable. There is almost no valid reason
to use the raw version. Use the zero length version when the packet
should be zero length.

the 'struct' passed can also be a raw type like 'float' or 'uint32'
in the case that the packet only contains that single variable.

Connecting state:
IN_C(opcode, struct) -> Typed dispatch in connecting mode
	Dispatch Prototype:
	void Client::Handle_Connect_opcode(struct *packet);
IN_Cv(opcode, struct) -> Typed dispatch in connecting mode, variable length
	Dispatch Prototype:
	void Client::Handle_Connect_opcode(struct *packet, uint32 length);
IN_Cz(opcode) -> Zero length dispatch in connecting mode
	Dispatch Prototype:
	void Client::Handle_Connect_opcode();
IN_Cr(opcode) -> Raw dispatch in connecting mode
	Dispatch Prototype:
	void Client::Handle_Connect_opcode(const EQApplicationPacket *app);

Connected state:
IN(opcode, struct) -> Typed dispatch in connected mode
	Dispatch Prototype:
	void Client::Handle_opcode(struct *packet);
INv(opcode, struct) -> Typed dispatch in connected mode, variable length
	Dispatch Prototype:
	void Client::Handle_opcode(struct *packet, uint32 length);
INz(opcode) -> Zero length dispatch in connected mode
	Dispatch Prototype:
	void Client::Handle_opcode();
INr(opcode) -> Raw dispatch in connected mode
	Dispatch Prototype:
	void Client::Handle_opcode(const EQApplicationPacket *app);

All of the above functions apply to OUT mode as well.

lines prefixed with //alt: provide information about alternate
structures for an opcode, mainly used by opcode finder

*/


//IN_C(OP_SetDataRate, float);
IN_C(OP_ZoneEntry, ClientZoneEntry_Struct);
IN_C(OP_SetServerFilter, SetServerFilter_Struct);
IN_Cz(OP_SendAATable);
IN_Cz(OP_SendTributes);
IN_Cz(OP_SendGuildTributes);
IN_Cz(OP_SendAAStats);
IN_Cz(OP_ReqClientSpawn);
IN_Cz(OP_ReqNewZone);
IN_Cz(OP_SendExpZonein);
//IN_Cr(OP_ZoneComplete);

//these three should stay raw, since we ignore their contents and
//they overlap with the connected opcodes
IN_Cr(OP_SpawnAppearance);
IN_Cr(OP_WearChange);
IN_Cr(OP_ClientUpdate);

IN_Cz(OP_ClientReady);
IN_C(OP_ClientError, ClientError_Struct);
IN_C(OP_ApproveZone, ApproveZone_Struct);
IN_Cr(OP_TGB);

IN(OP_AckPacket, uint32);
IN(OP_ClientUpdate, PlayerPositionUpdateClient_Struct);
IN(OP_AutoAttack, uint32);
IN(OP_AutoAttack2, uint32);
INv(OP_Consent, Consent_Struct);
INv(OP_ConsentDeny, Consent_Struct);
IN(OP_TargetMouse, ClientTarget_Struct);
IN(OP_TargetCommand, ClientTarget_Struct);
IN(OP_Shielding, Shielding_Struct);
INr(OP_Jump);	//?
INr(OP_AdventureInfoRequest);
INr(OP_AdventureRequest);
IN(OP_LDoNButton, bool);	//not sure on length
INr(OP_LeaveAdventure);
IN(OP_Consume, Consume_Struct);
IN(OP_AdventureMerchantRequest, AdventureMerchant_Struct);
IN(OP_AdventureMerchantPurchase, Adventure_Purchase_Struct);
IN(OP_ConsiderCorpse, Consider_Struct);
IN(OP_Consider, Consider_Struct);
INr(OP_Begging);	//?
INr(OP_TestBuff);
IN(OP_Surname, Surname_Struct);
INr(OP_YellForHelp);
IN(OP_Assist, EntityId_Struct);		//generic
IN(OP_GMTraining, GMTrainee_Struct);
IN(OP_GMEndTraining, GMTrainEnd_Struct);
IN(OP_GMTrainSkill, GMSkillChange_Struct);
IN(OP_RequestDuel, Duel_Struct);
IN(OP_DuelDecline, DuelResponse_Struct);
IN(OP_DuelAccept, Duel_Struct);
IN(OP_SpawnAppearance, SpawnAppearance_Struct);
IN(OP_BazaarInspect, BazaarInspect_Struct);
IN(OP_Death, Death_Struct);
IN(OP_MoveCoin, MoveCoin_Struct);
IN(OP_ItemLinkClick, ItemViewRequest_Struct);
IN(OP_MoveItem, MoveItem_Struct);
INz(OP_Camp);	//?
INz(OP_Logout);
INz(OP_SenseHeading);	//?
INz(OP_FeignDeath);	//?
INz(OP_Sneak);		//?
INz(OP_Hide);		//?
INv(OP_ChannelMessage, ChannelMessage_Struct);
IN(OP_WearChange, WearChange_Struct);
IN(OP_DeleteSpawn, EntityId_Struct);	//client->server follows OP_SaveOnZoneReq
IN(OP_SaveOnZoneReq, Save_Struct);		//follows OP_ZoneChange
IN(OP_Save, Save_Struct);
IN(OP_WhoAllRequest, Who_All_Struct);
IN(OP_GMZoneRequest, GMZoneRequest_Struct);
IN(OP_GMZoneRequest2, uint32);
IN(OP_EndLootRequest, EntityId_Struct);	//follows OP_LootRequest
IN(OP_LootRequest, EntityId_Struct);	//entity must be a corpse
IN(OP_Dye, DyeStruct);
INr(OP_ConfirmDelete);	//?
IN(OP_LootItem, LootingItem_Struct);
INr(OP_GuildDelete);	//?
IN(OP_GuildPublicNote, GuildUpdate_PublicNote);
INz(OP_GetGuildsList);	//?
IN(OP_SetGuildMOTD, GuildMOTD_Struct);
IN(OP_SetRunMode, SetRunMode_Struct);
INz(OP_GuildPeace);		//?
INz(OP_GuildWar);		//?
IN(OP_GuildLeader, GuildMakeLeader);
IN(OP_GuildDemote, GuildDemoteStruct);
IN(OP_GuildInvite, GuildCommand_Struct);
IN(OP_GuildRemove, GuildCommand_Struct);
IN(OP_GuildInviteAccept, GuildInviteAccept_Struct);
IN(OP_ManaChange, ManaChange_Struct);	//possibly zero len too
//alt:INz(OP_ManaChange);
IN(OP_MemorizeSpell, MemorizeSpell_Struct);
IN(OP_SwapSpell, SwapSpell_Struct);
IN(OP_CastSpell, CastSpell_Struct);
IN(OP_DeleteItem, DeleteItem_Struct);
IN(OP_CombatAbility, CombatAbility_Struct);
IN(OP_Taunt, ClientTarget_Struct);
INz(OP_InstillDoubt);
IN(OP_RezzAnswer, Resurrect_Struct);
IN(OP_GMSummon, GMSummon_Struct);
IN(OP_TradeBusy, TradeBusy_Struct);
IN(OP_TradeRequest, TradeRequest_Struct);
IN(OP_TradeRequestAck, TradeRequest_Struct);	//follows OP_TradeRequest
IN(OP_CancelTrade, CancelTrade_Struct);
IN(OP_TradeAcceptClick, TradeAccept_Struct);
IN(OP_BoardBoat, EntityId_Struct);		//not really the struct, just 4 bytes
INz(OP_LeaveBoat);		//?
IN(OP_RandomReq, RandomReq_Struct);
IN(OP_Buff, SpellBuffPacket_Struct);
IN(OP_GMHideMe, SpawnAppearance_Struct);
IN(OP_GMNameChange, GMName_Struct);
IN(OP_GMKill, GMKill_Struct);
IN(OP_GMLastName, GMLastName_Struct);
IN(OP_GMToggle, GMToggle_Struct);
IN(OP_LFGCommand, LFG_Struct);
IN(OP_GMGoto, GMSummon_Struct);
INv(OP_TraderShop, TraderClick_Struct);
IN(OP_ShopRequest, Merchant_Click_Struct);
IN(OP_Bazaar, BazaarSearch_Struct);
//alt:IN(OP_Bazaar, BazaarWelcome_Struct);		//alternate structure for OP_Bazaar
IN(OP_ShopPlayerBuy, Merchant_Sell_Struct);
IN(OP_ShopPlayerSell, Merchant_Purchase_Struct);
INr(OP_ShopEnd);		//?
IN(OP_CloseContainer, ClickObjectAction_Struct);
IN(OP_ClickObjectAction, ClickObjectAction_Struct);
IN(OP_ClickObject, ClickObject_Struct);
IN(OP_RecipesFavorite, TradeskillFavorites_Struct);
IN(OP_RecipesSearch, RecipesSearch_Struct);
IN(OP_RecipeDetails, uint32);
//there is also a complicated OP_RecipeDetails reply struct OUT
IN(OP_RecipeAutoCombine, RecipeAutoCombine_Struct);
IN(OP_TradeSkillCombine, NewCombine_Struct);
IN(OP_TradeSkillRecipeInspect, TradeSkillRecipeInspect_Struct);
IN(OP_ItemName, ItemNamePacket_Struct);
IN(OP_AugmentItem, AugmentItem_Struct);
IN(OP_ClickDoor, ClickDoor_Struct);
INr(OP_CreateObject);	//?
IN(OP_FaceChange, FaceChange_Struct);
IN(OP_GroupInvite, GroupInvite_Struct);
IN(OP_GroupInvite2, GroupInvite_Struct);	//will generally follow OP_GroupInvite for next invite
IN(OP_GroupFollow, GroupGeneric_Struct);	//will follow invite
IN(OP_GroupFollow2, GroupGeneric_Struct);	//will follow invite2
INr(OP_GroupAcknowledge);	//?
IN(OP_GroupCancelInvite, GroupGeneric_Struct);	//follows invite or invite2
IN(OP_GroupDisband, GroupGeneric_Struct);
INr(OP_GroupDelete);	//?
IN(OP_GMEmoteZone, GMEmoteZone_Struct);
IN(OP_InspectRequest, Inspect_Struct);
IN(OP_InspectAnswer, Inspect_Struct);	//follows request
IN(OP_DeleteSpell, DeleteSpell_Struct);
IN(OP_PetitionBug, PetitionBug_Struct);
IN(OP_Bug, BugStruct);

//all these petition opcodes need checking
//converted most of them to raw so they wouldent cause problems
INr(OP_Petition);	//used to be just a string
IN(OP_PetitionCheckIn, Petition_Struct);
IN(OP_PetitionResolve, PetitionUpdate_Struct);
IN(OP_PetitionDelete, PetitionUpdate_Struct);
INr(OP_PetitionUnCheckout);	//prolly 4 bytes in length
INr(OP_PetitionQue);
INr(OP_PDeletePetition);	//used to be just a string
INr(OP_PetitionCheckout);	//prolly 4 bytes in length
INr(OP_PetitionRefresh);

IN(OP_PetCommands, PetCommand_Struct);
IN(OP_ReadBook, BookRequest_Struct);
IN(OP_Emote, Emote_Struct);
#ifdef DISJOINT_STATES
IN(OP_SetServerFilter, SetServerFilter_Struct);
#endif
IN(OP_GMDelCorpse, GMDelCorpse_Struct);
IN(OP_GMKick, GMKick_Struct);
INr(OP_GMServers);	//?
IN(OP_Illusion, Illusion_Struct);
IN(OP_GMBecomeNPC, BecomeNPC_Struct);
INz(OP_Fishing);	//?
INz(OP_Forage);		//?
INz(OP_Mend);		//?
IN(OP_EnvDamage, EnvDamage2_Struct);
IN(OP_Damage, CombatDamage_Struct);
IN(OP_AAAction, AA_Action);
IN(OP_TraderBuy, TraderBuy_Struct);
IN(OP_Trader, Trader_ShowItems_Struct);
IN(OP_GMFind, GMSummon_Struct);
IN(OP_PickPocket, PickPocket_Struct);
IN(OP_PickZone, PickZone_Struct);
IN(OP_Bind_Wound, BindWound_Struct);
INr(OP_TrackTarget);
INr(OP_Track);
INz(OP_TrackUnknown);		//follows OP_Track
#ifdef DISJOINT_STATES
IN(OP_ClientError, ClientError_Struct);
#endif
INr(OP_ReloadUI);	//?
INr(OP_TGB);		//4 bytes, value is 0, 1, or 2
IN(OP_Split, Split_Struct);
INz(OP_SenseTraps);		//?
INz(OP_DisarmTraps);	//?
IN(OP_OpenTributeMaster, StartTribute_Struct);
IN(OP_OpenGuildTributeMaster, StartTribute_Struct);
IN(OP_TributeItem, TributeItem_Struct);
IN(OP_TributeMoney, TributeMoney_Struct);
IN(OP_SelectTribute, SelectTributeReq_Struct);
IN(OP_TributeUpdate, TributeInfo_Struct);
IN(OP_TributeToggle, uint32);	//value is 0 or 1
IN(OP_TributeNPC, uint32);		//contains tribute master entity ID
INr(OP_CrashDump);
INr(OP_ControlBoat);
INr(OP_DumpName);
INr(OP_SetRunMode);
INr(OP_SafeFallSuccess);
INr(OP_Heartbeat);
INr(OP_SafePoint);
INr(OP_Ignore);
IN(OP_FindPersonRequest, FindPersonRequest_Struct);
IN(OP_LeadershipExpToggle, uint8);
IN(OP_PurchaseLeadershipAA, uint32);	//value is < _maxLeaderAA
INr(OP_ClearTitle);		//follows OP_SendTitleList
INr(OP_BankerChange);
IN(OP_SetTitle, SetTitle_Struct);
INz(OP_RequestTitles);
IN(OP_ItemVerifyRequest, ItemVerifyRequest_Struct);


/*
	for now we are listing outgoing packets which are NOT
	also incoming packets
*/
OUTz(OP_ClearObject);
OUTz(OP_FinishTrade);		//follows OP_TradeAcceptClick
OUTz(OP_GMEndTrainingResponse);	//follows OP_GMTraining
OUTz(OP_LootComplete);		//follows OP_LootItem
OUTz(OP_WorldObjectsSent);
OUTz(OP_FinishWindow);
OUTz(OP_FinishWindow2);
//OUTz(OP_TradeSkillCombine);

//OUTv(OP_AdventureDetails, strlen(AF.text)+1);
//OUTv(OP_AdventureInfo, strlen(buffer1)+1);
//OUTv(OP_AdventureInfo, strlen(p)+1);
//OUTv(OP_AdventureMerchantResponse, strlen(msg)+2);
OUTv(OP_ItemPacket, ItemPacket_Struct);
OUTv(OP_ColoredText, ColoredText_Struct);
OUTv(OP_ItemRecastDelay, ItemRecastDelay_Struct);
OUTv(OP_FormattedMessage, FormattedMessage_Struct);
OUTv(OP_GuildMemberList, uint32);		//variable length, but nasty
OUTv(OP_InterruptCast, InterruptCast_Struct);
OUTv(OP_ItemLinkResponse, ItemPacket_Struct);
OUTv(OP_ZoneSpawns, Spawn_Struct);
OUTv(OP_CompletedTasks, TaskHistory_Struct);
OUTv(OP_CharInventory, ItemPacket_Struct);
OUTv(OP_CustomTitles, Titles_Struct);
OUTv(OP_SpawnDoor, Door_Struct);
OUTv(OP_SendZonepoints, ZonePoints);
OUTv(OP_TributeInfo, TributeAbility_Struct);
OUTv(OP_GuildTributeInfo, GuildTributeAbility_Struct);
OUTv(OP_SendTitleList, TitleList_Struct);
//these arnt used anymore
//OUTv(OP_ItemLinkText, strlen(itemlink)+14+strlen(charname));
//OUTv(OP_ItemLinkText, strlen(name2)+68);

OUT(OP_SendMaxCharacters, MaxCharacters_Struct);
OUT(OP_AAExpUpdate, AAExpUpdate_Struct);
OUT(OP_Action, Action_Struct);
OUT(OP_AdventureData, AdventureRequestResponse_Struct);
OUT(OP_AdventureFinish, AdventureFinish_Struct);
OUT(OP_AdventurePointsUpdate, AdventurePoints_Update_Struct);
OUT(OP_Animation, Animation_Struct);
OUT(OP_AnnoyingZoneUnknown, AnnoyingZoneUnknown_Struct);
OUT(OP_BankerChange, BankerChange_Struct);
OUT(OP_BecomeTrader, BecomeTrader_Struct);
OUT(OP_BeginCast, BeginCast_Struct);
OUT(OP_Charm, Charm_Struct);
OUT(OP_CameraEffect, Camera_Struct);
OUT(OP_ClickObjectAction, ClickObjectAction_Struct);
OUT(OP_ConsentResponse, ConsentResponse_Struct);
//OUT(OP_ConsumeAmmo, MoveItem_Struct);
OUT(OP_EnduranceUpdate, EnduranceUpdate_Struct);
OUT(OP_ExpUpdate, ExpUpdate_Struct);
OUT(OP_GroundSpawn, Object_Struct);
OUT(OP_GroupUpdate, GroupJoin_Struct);	//takes on 3 sizes...
//alt:OUT(OP_GroupUpdate, GroupUpdate2_Struct);
//alt:OUT(OP_GroupUpdate, GroupUpdate_Struct);
OUT(OP_GuildMOTD, GuildMOTD_Struct);
OUT(OP_GuildManageAdd, GuildJoin_Struct);
OUT(OP_GuildManageRemove, GuildManageRemove_Struct);
OUT(OP_GuildManageStatus, GuildManageStatus_Struct);
OUT(OP_GuildMemberUpdate, GuildMemberUpdate_Struct);
OUT(OP_HPUpdate, SpawnHPUpdate_Struct);
OUT(OP_IncreaseStats, IncreaseStat_Struct);
OUT(OP_ItemVerifyReply, ItemVerifyReply_Struct);
OUT(OP_LFGAppearance, LFG_Appearance_Struct);
OUT(OP_LeadershipExpUpdate, LeadershipExpUpdate_Struct);
OUT(OP_LevelAppearance, LevelAppearance_Struct);
OUT(OP_LevelUpdate, LevelUpdate_Struct);
OUT(OP_ManaUpdate, ManaUpdate_Struct);
OUT(OP_MobEnduranceUpdate, MobEnduranceUpdate_Struct);
OUT(OP_MobHealth, MobHealth_Struct);
OUT(OP_MobManaUpdate, MobManaUpdate_Struct);
OUT(OP_MobRename, MobRename_Struct);
OUT(OP_MoneyOnCorpse, moneyOnCorpseStruct);	//follows OP_LootRequest
OUT(OP_MoneyUpdate, MoneyUpdate_Struct);
OUT(OP_MoveDoor, MoveDoor_Struct);
OUT(OP_NewSpawn, NewSpawn_Struct);
OUT(OP_NewZone, NewZone_Struct);
OUT(OP_PetitionCheckout, Petition_Struct);
OUT(OP_PetitionUpdate, PetitionUpdate_Struct);
OUT(OP_PlayerProfile, PlayerProfile_Struct);
OUT(OP_RaidUpdate, ZoneInSendName_Struct);
//alt:OUTv(OP_RaidUpdate, RaidMembers_Struct);
OUT(OP_RandomReply, RandomReply_Struct);
OUT(OP_RecipeReply, RecipeReply_Struct);
OUT(OP_RequestClientZoneChange, RequestClientZoneChange_Struct);
OUT(OP_RespondAA, AATable_Struct);
OUT(OP_RezzRequest, Resurrect_Struct);
OUT(OP_SetTitleReply, SetTitleReply_Struct);
OUT(OP_ShopDelItem, Merchant_DelItem_Struct);
OUT(OP_SimpleMessage, SimpleMessage_Struct);
OUT(OP_SkillUpdate, SkillUpdate_Struct);
OUT(OP_SomeItemPacketMaybe, Arrow_Struct);
OUT(OP_SpellEffect, SpellEffect_Struct);
OUT(OP_Stamina, Stamina_Struct);
OUT(OP_Stun, Stun_Struct);
OUT(OP_TargetReject, TargetReject_Struct);
OUT(OP_TimeOfDay, TimeOfDay_Struct);
OUT(OP_Track, Track_Struct);
OUT(OP_TradeCoins, TradeCoin_Struct);
OUT(OP_TradeMoneyUpdate, TradeMoneyUpdate_Struct);
OUT(OP_TraderDelItem, TraderDelItem_Struct);
OUT(OP_TraderItemUpdate, TraderItemUpdate_Struct);
OUT(OP_TributeTimer, uint32);
OUT(OP_UpdateLeadershipAA, UpdateLeadershipAA_Struct);
OUT(OP_Weather, Weather_Struct);
OUT(OP_ZoneChange, ZoneChange_Struct);
OUT(OP_ZoneInUnknown, ZoneInUnknown_Struct);

//this is the set of opcodes which are already listed
//in the IN section above, but are also sent OUT
#ifdef DISJOINT_DIRECTIONS
OUTz(OP_ClientReady);		//follows OP_SetServerFilter
OUTz(OP_Dye);
OUTz(OP_GMKick);
OUTz(OP_SendAAStats);		//follows OP_ReqNewZone
OUTz(OP_SendExpZonein);		//follows OP_SendZonepoints

OUTv(OP_ReadBook, BookText_Struct);
OUTv(OP_SendAATable, SendAA_Struct);

OUT(OP_AAAction, UseAA_Struct);
OUT(OP_Bazaar, BazaarReturnDone_Struct);
//alt:OUT(OP_Bazaar, BazaarWelcome_Struct);
OUT(OP_Buff, SpellBuffPacket_Struct);
OUT(OP_ClickObject, ClickObject_Struct);
OUT(OP_ClientUpdate, PlayerPositionUpdateServer_Struct);
OUT(OP_SpawnPositionUpdate, SpawnPositionUpdate_Struct);
OUT(OP_Consider, Consider_Struct);
OUT(OP_Damage, CombatDamage_Struct);
OUT(OP_Death, Death_Struct);
OUT(OP_DeleteSpawn, EntityId_Struct);
OUT(OP_DeleteSpell, DeleteSpell_Struct);
OUT(OP_EmoteAnim, EmoteAnim_Struct);
OUT(OP_GMFind, GMSummon_Struct);
OUT(OP_GMKick, GMKick_Struct);
OUT(OP_GMKill, GMKill_Struct);
OUT(OP_GMLastName, GMLastName_Struct);
OUT(OP_GMNameChange, GMName_Struct);
OUT(OP_GMSummon, GMSummon_Struct);
OUT(OP_GMZoneRequest, GMZoneRequest_Struct);
OUT(OP_Illusion, Illusion_Struct);
OUT(OP_ItemName, ItemNamePacket_Struct);
OUT(OP_ManaChange, ManaChange_Struct);
//alt:OUTz(OP_ManaChange);		//takes on at least two lengths
OUT(OP_MemorizeSpell, MemorizeSpell_Struct);
OUT(OP_MoveItem, MoveItem_Struct);
OUT(OP_PickPocket, sPickPocket_Struct);
OUT(OP_RecipeAutoCombine, RecipeAutoCombine_Struct);
OUT(OP_RequestDuel, Duel_Struct);
OUT(OP_ShopPlayerBuy, Merchant_Sell_Struct);
OUT(OP_ShopPlayerSell, Merchant_Purchase_Struct);
OUT(OP_ShopRequest, Merchant_Click_Struct);
OUT(OP_SpawnAppearance, SpawnAppearance_Struct);
OUT(OP_TradeRequestAck, TradeRequest_Struct);
OUT(OP_Trader, TraderBuy_Struct);	//3 possible lengths
//alt:OUT(OP_Trader, Trader_ShowItems_Struct);
//alt:OUT(OP_Trader, Trader_Struct);
OUT(OP_TraderBuy, TraderBuy_Struct);
OUTv(OP_TraderShop, TraderClick_Struct);
OUT(OP_WearChange, WearChange_Struct);
OUT(OP_ZoneEntry, ServerZoneEntry_Struct);
#endif





/*

//... client_packet.h

#define IN_C(op, s) \
	void Handle_Connect_##op (s *in);
#define IN_Cv(op, s) \
	void Handle_Connect_##op (s *in, uint32 length);
#define IN_Cz(op) \
	void Handle_Connect_##op ();
#define IN_Cr(op) \
	void Handle_Connect_##op (const EQApplicationPacket *app);
#define IN(op, s) \
	void Handle_##op (s *in);
#define INv(op, s) \
	void Handle_##op (s *in, uint32 length);
#define INz(op) \
	void Handle_##op ();
#define INr(op) \
	void Handle_##op (const EQApplicationPacket *app);
#define OUT_C(op, s)
#define OUT_Cv(op, s)
#define OUT_Cz(op)
#define OUT_Cr(op)
#define OUT(op, s)
#define OUTv(op, s)
#define OUTz(op)
#define OUTr(op)
#include "opcode_dispatch.h"
#undef IN_C
#undef IN_Cr
#undef IN_Cv
#undef IN_Cz
#undef IN
#undef INr
#undef INv
#undef INz
#undef OUT_C
#undef OUT_Cr
#undef OUT_Cv
#undef OUT_Cz
#undef OUT
#undef OUTr
#undef OUTv
#undef OUTz

class OpcodeDispatcher {
public:
	virtual void dispatch(Client *on, const EQApplicationPacket *app) = 0;
};

class RawOpcodeDispatcher : public OpcodeDispatcher {
public:
	typedef void (Client::*proc)(const EQApplicationPacket *app);
	RawOpcodeDispatcher(proc p) {
		d = p;
	}

	virtual void dispatch(Client *on, const EQApplicationPacket *app) {
		(on->*d)(app);
	}
protected:
	proc d;
};

class ZeroOpcodeDispatcher : public OpcodeDispatcher {
public:
	typedef void (Client::*proc)();
	ZeroOpcodeDispatcher(proc p) {
		d = p;
	}

	virtual void dispatch(Client *on, const EQApplicationPacket *app) {
		if(app->size != 0) {
			//error..
			return;
		}
		(on->*d)();
	}
protected:
	proc d;
};

template <class T>
class TypedOpcodeDispatcher : public OpcodeDispatcher {
public:
	typedef void (Client::*proc)(T *packet);
	TypedOpcodeDispatcher(proc p, const char *sn) {
		d = p;
		struct_name = sn;
	}

	virtual void dispatch(Client *on, const EQApplicationPacket *app) {
		if(app->size != sizeof(T)) {
			//error..
			return;
		}
		T * tmp = (T *) app->pBuffer;
		(on->*d)(tmp);
	}

protected:
	proc d;
	const char *struct_name;
};

template <class T>
class TypedVarOpcodeDispatcher : public OpcodeDispatcher {
public:
	typedef void (Client::*proc)(T *packet, uint32 length);
	TypedVarOpcodeDispatcher(proc p, const char *sn) {
		d = p;
		struct_name = sn;
	}

	virtual void dispatch(Client *on, const EQApplicationPacket *app) {
		if(app->size < sizeof(T)) {
			//error..
			return;
		}
		T * tmp = (T *) app->pBuffer;
		(on->*d)(tmp, app->size);
	}

protected:
	proc d;
	const char *struct_name;
};



//... client_packet.cpp
void MapOpcodes() {
	//..
#define IN_C(op, s) \
	ConnectingOpcodes[op] = new TypedOpcodeDispatcher<s>(&Client::Handle_Connect_##op , #s);
#define IN_Cv(op, s) \
	ConnectingOpcodes[op] = new TypedVarOpcodeDispatcher<s>(&Client::Handle_Connect_##op , #s);
#define IN_Cz(op) \
	ConnectingOpcodes[op] = new ZeroOpcodeDispatcher(&Client::Handle_Connect_##op);
#define IN_Cr(op) \
	ConnectingOpcodes[op] = new RawOpcodeDispatcher(&Client::Handle_Connect_##op);
#define IN(op, s) \
	ConnectedOpcodes[op] = new TypedOpcodeDispatcher<s>(&Client::Handle_##op , #s);
#define INv(op, s) \
	ConnectedOpcodes[op] = new TypedVarOpcodeDispatcher<s>(&Client::Handle_##op , #s);
#define INz(op) \
	ConnectedOpcodes[op] = new ZeroOpcodeDispatcher(&Client::Handle_##op);
#define INr(op) \
	ConnectedOpcodes[op] = new RawOpcodeDispatcher(&Client::Handle_##op);
#define OUT_C(op, s)
#define OUT_Cv(op, s)
#define OUT_Cz(op)
#define OUT_Cr(op)
#define OUT(op, s)
#define OUTv(op, s)
#define OUTz(op)
#define OUTr(op)
#include "opcode_dispatch.h"
#undef IN_C
#undef IN_Cr
#undef IN_Cv
#undef IN_Cz
#undef IN
#undef INr
#undef INv
#undef INz
#undef OUT_C
#undef OUT_Cr
#undef OUT_Cv
#undef OUT_Cz
#undef OUT
#undef OUTr
#undef OUTv
#undef OUTz

*/
