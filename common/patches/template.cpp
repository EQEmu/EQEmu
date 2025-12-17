
#include "TEMPLATE.h"
#include "TEMPLATE_structs.h"

#include "common/eq_packet_structs.h"
#include "common/eq_stream_ident.h"
#include "common/logsys.h"
#include "common/opcodemgr.h"

namespace TEMPLATE {

static const char *name = "CHANGEME";
static OpcodeManager *opcodes = NULL;
static Strategy struct_strategy;

void Register(EQStreamIdentifier &into) {
	//create our opcode manager if we havent already
	if(opcodes == NULL) {
		string opfile = Config->PatchDir;
		opfile += "patch_";
		opfile += name;
		opfile += ".conf";
		//load up the opcode manager.
		//TODO: figure out how to support shared memory with multiple patches...
		opcodes = new RegularOpcodeManager();
		if(!opcodes->LoadOpcodes(opfile.c_str())) {
			logger.LogDebugType(EQEmuLogSys::General, EQEmuLogSys::Netcode, "[OPCODES] Error loading opcodes file %s. Not registering patch %s.", opfile.c_str(), name);
			return;
		}
	}

	//ok, now we have what we need to register.

	EQStream::Signature signature;

	//register our world signature.
	signature.first_length = sizeof(structs::LoginInfo_Struct);
	signature.first_eq_opcode = opcodes->EmuToEQ(OP_SendLoginInfo);
	into.RegisterPatch(signature, name, &opcodes, &struct_strategy);

	//register our zone signature.
	signature.first_length = sizeof(structs::ClientZoneEntry_Struct);
	signature.first_eq_opcode = opcodes->EmuToEQ(OP_ZoneEntry);
	into.RegisterPatch(signature, name, &opcodes, &struct_strategy);
}

void Reload() {

	//we have a big problem to solve here when we switch back to shared memory
	//opcode managers because we need to change the manager pointer, which means
	//we need to go to every stream and replace it's manager.

	if(opcodes != NULL) {
		//TODO: get this file name from the config file
		string opfile = Config->PatchDir;
		opfile += "patch_";
		opfile += name;
		opfile += ".conf";
		if(!opcodes->ReloadOpcodes(opfile.c_str())) {
			logger.LogDebugType(EQEmuLogSys::General, EQEmuLogSys::Netcode, "[OPCODES] Error reloading opcodes file %s for patch %s.", opfile.c_str(), name);
			return;
		}
		logger.LogDebugType(EQEmuLogSys::General, EQEmuLogSys::Netcode, "[OPCODES] Reloaded opcodes for patch %s", name);
	}
}



Strategy::Strategy()
: StructStrategy()
{
	//all opcodes default to passthrough.
	#include "ss_register.h"
	#include "TEMPLATE_ops.h"
}

std::string Strategy::Describe() const {
	std::string r;
	r += "Patch ";
	r += name;
	return(r);
}


#include "ss_define.h"


/*ENCODE(OP_PlayerProfile) {
	SETUP_DIRECT(PlayerProfile_Struct, structs::PlayerProfile_Struct);



	FINISH_DIRECT();
}

ENCODE(OP_NewZone) {
	SETUP_DIRECT(PlayerProfile_Struct, structs::PlayerProfile_Struct);



	FINISH_DIRECT();
}*/

ENCODE(OP_SendAATable) {
	SETUP_DIRECT_ENCODE(SendAA_Struct, structs::SendAA_Struct);
	OUT(id);
	OUT(hotkey_sid);
	OUT(hotkey_sid2);
	OUT(title_sid);
	OUT(desc_sid);
	OUT(cost);
	OUT(seq);
	OUT(current_level);
	OUT(prereq_skill);
	OUT(prereq_minpoints);
	OUT(type);
	OUT(spellid);
	OUT(spell_type);
	OUT(spell_refresh);
	OUT(classes);
	OUT(berserker);
	OUT(max_level);
	OUT(last_id);
	OUT(next_id);
	OUT(cost2);
	OUT(unknown80[0]);
	OUT(unknown80[1]);
	OUT(total_abilities);
	unsigned int r;
	for(r = 0; r < emu->total_abilities; r++) {
		OUT(abilities[r].skill_id);
		OUT(abilities[r].increase_amt);
		OUT(abilities[r].unknown08);
		OUT(abilities[r].last_level);
	}
	FINISH_DIRECT_ENCODE();
}

DECODE(OP_SetServerFilter) {
	SETUP_DIRECT_DECODE(SetServerFilter_Struct, structs::SetServerFilter_Struct);
	int r;
	for(r = 0; r < 25; r++) {
		IN(filters[r]);
	}
	emu->filters[25] = 1;
	FINISH_DIRECT_DECODE();
}

} // namespace TEMPLATE
