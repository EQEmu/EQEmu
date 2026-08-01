--copy or symbolic link this file to /server/lua_modules/, it will not work in /server/quests/lua_modules/

local commands_path = "commands/";
local commands      = { };

commands["don"]       = { 80, require(commands_path .. "don") };
commands["endurance"] = { 50, require(commands_path .. "endurance") };
commands["lockouts"]  = {  0, require(commands_path .. "lockouts") };
commands["timeleft"]  = {  0, require(commands_path .. "time_left") };
commands["hotzone"]   = {  0, require(commands_path .. "hotzone") };
commands["hotzones"]  = {  0, require(commands_path .. "hotzone") };

-- AoTv4 GM commands. ⚠️ THIS TABLE IS THE ONLY REGISTRATION. A file in commands/ that is not listed
-- here is simply never reachable -- and note that `command_settings` in the DB is for C++ commands
-- ONLY: the zone deletes any row there whose command does not exist in C++, logging
-- "Command [x] no longer exists. Deleting orphaned entry". #worldboss sat unregistered for exactly
-- this reason until 2026-07-27.
commands["worldboss"] = { 200, require(commands_path .. "worldboss") };
commands["worldbuff"] = { 200, require(commands_path .. "worldbuff") };

function eq.DispatchCommands(e)
	local command = commands[e.command];

	for k,v in pairs(e.args) do
		eq.debug("[command.lua] key " .. k .. ' value ' .. v, 3);
	end

	if(command) then
		local access = command[1];
		if(access > e.self:Admin()) then
			e.self:Message(MT.Red, "Access level not high enough.");
			return 1;
		end

		local func = command[2];
		func(e);
		return 1;
	end
	return 0;
end
