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
#include "web_interface_eqw.h"

#include "world/clientlist.h"
#include "world/launcher_list.h"
#include "world/login_server_list.h"
#include "world/web_interface.h"
#include "world/world_config.h"
#include "world/zonelist.h"

extern LauncherList launcher_list;

void EQW__GetConfig(WebInterface *i, const std::string& method, const std::string& id, const Json::Value& params) {
	if (params.isArray() && params.size() == 1) {
		auto config_name = params[0];
		if (config_name.isString()) {
			auto config = config_name.asString();

			Json::Value ret = WorldConfig::get()->GetByName(config);
			i->SendResponse(id, ret);
			return;
		}
	}

	i->SendError("EBADPARAMS", id);
}

void EQW__IsLocked(WebInterface *i, const std::string& method, const std::string& id, const Json::Value& params) {
	Json::Value ret = WorldConfig::get()->Locked;
	i->SendResponse(id, ret);
}

void EQW__Lock(WebInterface *i, const std::string& method, const std::string& id, const Json::Value& params) {
	WorldConfig::LockWorld();
	if (LoginServerList::Instance()->Connected()) {
		LoginServerList::Instance()->SendStatus();
	}

	Json::Value ret;
	ret["status"] = "complete";
	i->SendResponse(id, ret);
}

void EQW__Unlock(WebInterface *i, const std::string& method, const std::string& id, const Json::Value& params) {
	WorldConfig::UnlockWorld();
	if (LoginServerList::Instance()->Connected()) {
		LoginServerList::Instance()->SendStatus();
	}

	Json::Value ret;
	ret["status"] = "complete";
	i->SendResponse(id, ret);
}

void EQW__GetPlayerCount(WebInterface *i, const std::string& method, const std::string& id, const Json::Value& params) {
	Json::Value ret = ClientList::Instance()->GetClientCount();
	i->SendResponse(id, ret);
}

void EQW__GetZoneCount(WebInterface *i, const std::string& method, const std::string& id, const Json::Value& params) {
	Json::Value ret = ZSList::Instance()->GetZoneCount();
	i->SendResponse(id, ret);
}

void EQW__GetLauncherCount(WebInterface *i, const std::string& method, const std::string& id, const Json::Value& params) {
	Json::Value ret = launcher_list.GetLauncherCount();
	i->SendResponse(id, ret);
}

void EQW__GetLoginServerCount(WebInterface *i, const std::string& method, const std::string& id, const Json::Value& params) {
	Json::Value ret = LoginServerList::Instance()->GetServerCount();
	i->SendResponse(id, ret);
}

void RegisterEQW(WebInterface *i)
{
	i->AddCall("EQW::GetConfig", std::bind(EQW__GetConfig, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	i->AddCall("EQW::IsLocked", std::bind(EQW__IsLocked, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	i->AddCall("EQW::Lock", std::bind(EQW__Lock, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	i->AddCall("EQW::Unlock", std::bind(EQW__Unlock, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	i->AddCall("EQW::GetPlayerCount", std::bind(EQW__GetPlayerCount, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	i->AddCall("EQW::GetZoneCount", std::bind(EQW__GetZoneCount, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	i->AddCall("EQW::GetLauncherCount", std::bind(EQW__GetLauncherCount, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	i->AddCall("EQW::GetLoginServerCount", std::bind(EQW__GetLoginServerCount, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
}
