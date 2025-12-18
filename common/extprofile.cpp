/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2004 EQEMu Development Team (http://eqemulator.net)

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

#include "extprofile.h"

//Set defaults in the extended profile...
void InitExtendedProfile(ExtendedProfile_Struct *p) {
	memset(p, 0, sizeof(ExtendedProfile_Struct));
	//set any special values here...
}

bool SetExtendedProfile(ExtendedProfile_Struct *to, char *old, unsigned int len) {
	if(len == 0 || old == nullptr) {
		//handle old chars without an extended profile...
		InitExtendedProfile(to);
		return(true);
	}
	if(len == sizeof(ExtendedProfile_Struct)) {
		memcpy(to, old, sizeof(ExtendedProfile_Struct));
		return(true);
	}

	//convert an old block of memory of size old to the new struct
	//and store the new results in 'to'

	//generic converter that will work as long as the structre
	//only grows, and nothign gets re-arranged
	if(len < sizeof(ExtendedProfile_Struct)) {
		InitExtendedProfile(to);
		memcpy(to, old, len);
		return(true);
	}

	return(false);
}
