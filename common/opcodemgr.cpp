/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

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

#include "opcodemgr.h"

#include "common/emu_opcodes.h"
#include "common/eqemu_logsys.h"

#include <cstring>
#include <map>
#include <stdio.h>
#include <string>

OpcodeManager::OpcodeManager() {
	loaded = false;
}

bool OpcodeManager::LoadOpcodesFile(const char *filename, OpcodeSetStrategy *s, bool report_errors) {
	FILE *opf = fopen(filename, "r");
	if(opf == nullptr) {
		LogError("Unable to open opcodes file [{}]", filename);
		return(false);
	}

	std::map<std::string, uint16> eq;

	//load the opcode file into eq, could swap in a nice XML parser here
	char line[2048];
	int lineno = 0;
	uint16 curop;
	while(!feof(opf)) {
		lineno++;
		line[0] = '\0';	//for blank line at end of file
		if(fgets(line, sizeof(line), opf) == nullptr)
			break;

		//ignore any line that dosent start with OP_
		if(line[0] != 'O' || line[1] != 'P' || line[2] != '_')
			continue;

		char *num = line+3;	//skip OP_
		//look for the = sign
		while(*num != '=' && *num != '\0') {
			num++;
		}
		//make sure we found =
		if(*num != '=') {
			if(report_errors) fprintf(stderr, "Malformed opcode line at %s:%d\n", filename, lineno);
			continue;
		}
		*num = '\0';	//null terminate the name
		num++;			//num should point to the opcode

		//read the opcode
		if(sscanf(num, "0x%hx", &curop) != 1) {
			if(report_errors) fprintf(stderr, "Malformed opcode at %s:%d\n", filename, lineno);
			continue;
		}

		//we have a name and our opcode... stick it in the map
		eq[line] = curop;
	}
	fclose(opf);


	//do the mapping and store them in the shared memory array
	bool ret = true;
	EmuOpcode emu_op;
	std::map<std::string, uint16>::iterator res;
	//stupid enum wont let me ++ on it...
	for(emu_op = OP_Unknown; emu_op < _maxEmuOpcode; emu_op=(EmuOpcode)(emu_op+1)) {
		//get the name of this emu opcode
		const char *op_name = OpcodeNames[emu_op];
		if(op_name[0] == '\0') {
			if(report_errors) printf("Over-ran the bounds of the opcode name array. You prolly need to recompile the opcode stuff.\n");
			if(report_errors) printf("I will let you continue, since you may have gotten all the opcodes you need.\n");
			break;
		}

		//find the opcode in the file
		res = eq.find(op_name);
		if(res == eq.end()) {
			if(report_errors) fprintf(stderr, "Opcode %s is missing from %s\n", op_name, filename);
			//ret = false;
			continue;	//continue to give them a list of all missing opcodes
		}

		//ship the mapping off to shared mem.
		s->Set(emu_op, res->second);
	}

	return(ret);
}

//convenience routines
const char *OpcodeManager::EmuToName(const EmuOpcode emu_op) {
	return(OpcodeNames[emu_op]);
}

const char *OpcodeManager::EQToName(const uint16 eq_op) {
	//first must resolve the eq op to an emu op
	EmuOpcode emu_op = EQToEmu(eq_op);
	return(OpcodeNames[emu_op]);
}

EmuOpcode OpcodeManager::NameSearch(const char *name) {
	EmuOpcode emu_op;
	//stupid enum wont let me ++ on it...
	for(emu_op = OP_Unknown; emu_op < _maxEmuOpcode; emu_op=(EmuOpcode)(emu_op+1)) {
		//get the name of this emu opcode
		const char *op_name = OpcodeNames[emu_op];
		if(!strcasecmp(op_name, name)) {
			return(emu_op);
		}
	}
	return(OP_Unknown);
}

RegularOpcodeManager::RegularOpcodeManager()
: MutableOpcodeManager()
{
	emu_to_eq = nullptr;
	eq_to_emu = nullptr;
}

RegularOpcodeManager::~RegularOpcodeManager() {
	safe_delete_array(emu_to_eq);
	safe_delete_array(eq_to_emu);
}

bool RegularOpcodeManager::LoadOpcodes(const char *filename, bool report_errors) {
	NormalMemStrategy s;
	s.it = this;
	MOpcodes.lock();

	loaded = true;
	eq_to_emu = new EmuOpcode[MAX_EQ_OPCODE];
	emu_to_eq = new uint16[_maxEmuOpcode];
	EQOpcodeCount = MAX_EQ_OPCODE;
	EmuOpcodeCount = _maxEmuOpcode;

	//dont need to set eq_to_emu cause every element should get a value
	memset(eq_to_emu, 0, sizeof(EmuOpcode)*MAX_EQ_OPCODE);
	memset(emu_to_eq, 0, sizeof(uint16)*_maxEmuOpcode);

	bool ret = LoadOpcodesFile(filename, &s, report_errors);
	MOpcodes.unlock();
	return ret;
}

bool RegularOpcodeManager::ReloadOpcodes(const char *filename, bool report_errors) {
	if(!loaded)
		return(LoadOpcodes(filename));

	NormalMemStrategy s;
	s.it = this;
	MOpcodes.lock();

	memset(eq_to_emu, 0, sizeof(uint16)*MAX_EQ_OPCODE);

	bool ret = LoadOpcodesFile(filename, &s, report_errors);

	MOpcodes.unlock();
	return(ret);
}

uint16 RegularOpcodeManager::EmuToEQ(const EmuOpcode emu_op) {
	//opcode is checked for validity in GetEQOpcode
	uint16 res;
	MOpcodes.lock();
	res = emu_to_eq[emu_op];
	MOpcodes.unlock();

	LogNetcodeDetail("[Opcode Manager] Translate emu [{}] ({:#06x}) eq [{:#06x}]", OpcodeNames[emu_op], emu_op, res);

#ifdef DEBUG_TRANSLATE
	fprintf(stderr, "M Translate Emu %s (%d) to EQ 0x%.4x\n", OpcodeNames[emu_op], emu_op, res);
#endif
	return(res);
}

EmuOpcode RegularOpcodeManager::EQToEmu(const uint16 eq_op) {
	//opcode is checked for validity in GetEmuOpcode
//Disabled since current live EQ uses the entire uint16 bitspace for opcodes
//	if(eq_op > MAX_EQ_OPCODE)
//		return(OP_Unknown);
	EmuOpcode res;
	MOpcodes.lock();
	res = eq_to_emu[eq_op];
	MOpcodes.unlock();
#ifdef DEBUG_TRANSLATE
	fprintf(stderr, "M Translate EQ 0x%.4x to Emu %s (%d)\n", eq_op, OpcodeNames[res], res);
#endif
	return(res);
}

void RegularOpcodeManager::SetOpcode(EmuOpcode emu_op, uint16 eq_op) {

	//clear out old mapping
	uint16 oldop = emu_to_eq[emu_op];
	if(oldop != 0)
		eq_to_emu[oldop] = OP_Unknown;

	//use our strategy, since we have it
	NormalMemStrategy s;
	s.it = this;
	s.Set(emu_op, eq_op);
}

void RegularOpcodeManager::NormalMemStrategy::Set(EmuOpcode emu_op, uint16 eq_op) {
	if(uint32(emu_op) >= it->EmuOpcodeCount || eq_op >= it->EQOpcodeCount)
		return;
	it->emu_to_eq[emu_op] = eq_op;
	it->eq_to_emu[eq_op] = emu_op;
}

NullOpcodeManager::NullOpcodeManager()
: MutableOpcodeManager() {
}

bool NullOpcodeManager::LoadOpcodes(const char *filename, bool report_errors) {
	return(true);
}

bool NullOpcodeManager::ReloadOpcodes(const char *filename, bool report_errors) {
	return(true);
}

uint16 NullOpcodeManager::EmuToEQ(const EmuOpcode emu_op) {
	return(0);
}

EmuOpcode NullOpcodeManager::EQToEmu(const uint16 eq_op) {
	return(OP_Unknown);
}

EmptyOpcodeManager::EmptyOpcodeManager()
: MutableOpcodeManager() {
}

bool EmptyOpcodeManager::LoadOpcodes(const char *filename, bool report_errors) {
	return(true);
}

bool EmptyOpcodeManager::ReloadOpcodes(const char *filename, bool report_errors) {
	return(true);
}

uint16 EmptyOpcodeManager::EmuToEQ(const EmuOpcode emu_op) {
	std::map<EmuOpcode, uint16>::iterator f;
	f = emu_to_eq.find(emu_op);
	return(f == emu_to_eq.end()? 0 : f->second);
}

EmuOpcode EmptyOpcodeManager::EQToEmu(const uint16 eq_op) {
	std::map<uint16, EmuOpcode>::iterator f;
	f = eq_to_emu.find(eq_op);
	return(f == eq_to_emu.end()?OP_Unknown:f->second);
}

void EmptyOpcodeManager::SetOpcode(EmuOpcode emu_op, uint16 eq_op) {
	emu_to_eq[emu_op] = eq_op;
	eq_to_emu[eq_op] = emu_op;
}

