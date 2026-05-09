// Copyright (c) 2004 Daniel Wallin and Arvid Norberg

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef LUA_502_HPP
#define LUA_502_HPP

#if LUA_VERSION_NUM >= 502

inline LUA_API int lua_equal(lua_State *L, int idx1, int idx2)
{
  return lua_compare(L, idx1, idx2, LUA_OPEQ);
}

inline LUA_API int lua_lessthan(lua_State *L, int idx1, int idx2)
{
  return lua_compare(L, idx1, idx2, LUA_OPLT);
}

#undef lua_strlen
#define lua_strlen lua_rawlen

#undef lua_getfenv
#define lua_getfenv lua_getuservalue

#undef lua_setfenv
#define lua_setfenv lua_setuservalue

#undef lua_open
#define lua_open luaL_newstate

#else  // LUA_VERSION_NUM >= 502

#define lua_pushglobaltable(L) lua_pushvalue(L, LUA_GLOBALSINDEX)

#endif  // LUA_VERSION_NUM >= 502

#endif
