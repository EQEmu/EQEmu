/* luacheck -- syntax-check Lua files without running them.
 *
 * The container ships no lua/luac binary, so a broken quest module could only be discovered by
 * restarting zones and reading a player's chat log. That is a terrible feedback loop: a mangled
 * string literal in spell_choice.lua took the WHOLE picker down (global_player.lua requires it) and
 * surfaced only as "error loading module" in game.
 *
 * Uses luaL_loadfile, which parses and compiles but never executes -- so quest files that call eq.*
 * are safe to check. Exit 0 = all files parse.
 *
 *   cc -o luacheck luacheck.c -I/usr/include/lua5.1 -llua5.1
 */
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>

int main(int argc, char **argv)
{
    int bad = 0;
    for (int i = 1; i < argc; ++i) {
        lua_State *L = luaL_newstate();
        if (luaL_loadfile(L, argv[i]) != 0) {
            fprintf(stderr, "FAIL %s\n  %s\n", argv[i], lua_tostring(L, -1));
            bad = 1;
        } else {
            printf("ok   %s\n", argv[i]);
        }
        lua_close(L);
    }
    return bad;
}
