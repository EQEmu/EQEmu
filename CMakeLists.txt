cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/" ${CMAKE_MODULE_PATH})

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	message(STATUS "Using vcpkg env variable")
	set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
elseif(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	message(STATUS "Using vcpkg submodule")
	set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/submodules/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(EQEmu)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(EQEMU_BUILD_PCH "Build with precompiled headers (Windows)" ON)

# Requires libgoogle-perftools-dev google-perftools packages for linux (debian)
if(EQEMU_ADD_PROFILER)
	set(CMAKE_EXE_LINKER_FLAGS "-Wl,--no-as-needed,-lprofiler,--as-needed")
endif()

if(USE_MAP_MMFS)
	add_definitions(-DUSE_MAP_MMFS)
endif()

if(MSVC)
	add_compile_options(/bigobj)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-DNOMINMAX)
	add_definitions(-DCRASH_LOGGING)
	add_definitions(-D_HAS_AUTO_PTR_ETC) # for Luabind on C++17

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

	option(EQEMU_DISABLE_MSVC_WARNINGS "Disable MSVC compile warnings." ON)
	if(EQEMU_DISABLE_MSVC_WARNINGS)
		add_definitions( "/W0 /D_CRT_SECURE_NO_WARNINGS /wd4005 /wd4996 /nologo /Os")
	endif()
else(MSVC)
	add_definitions(-DHAS_UNION_SEMUN)
endif()

#FreeBSD support
if(UNIX)
	if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
		add_definitions(-DFREEBSD)
		add_definitions(-D_GLIBCXX_USE_C99)
		set(FREEBSD TRUE)
	endif()
	if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
		add_definitions(-DDARWIN)
		set(DARWIN TRUE)
	endif()
endif()

add_definitions(-DGLM_FORCE_RADIANS)
add_definitions(-DGLM_FORCE_CTOR_INIT)
add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

find_package(Boost REQUIRED COMPONENTS dynamic_bitset foreach tuple)
find_package(cereal CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(unofficial-libmariadb CONFIG REQUIRED)
find_package(libuv CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(recastnavigation CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(LuaJit REQUIRED)
find_package(PerlLibs)

message(STATUS "**************************************************")
message(STATUS "* Library Detection                              *")
message(STATUS "**************************************************")

if(MARIADB_FOUND)
	message(STATUS "* MariaDB:                                 FOUND *")
else()
	message(STATUS "* MariaDB:                               MISSING *")
endif()

if(ZLIB_FOUND)
	message(STATUS "* ZLIB:                                    FOUND *")
else()
	message(STATUS "* ZLIB:                                  MISSING *")
endif()

if(LuaJit_FOUND)
	message(STATUS "* LuaJIT:                                  FOUND *")
else()
	message(STATUS "* LuaJIT:                                MISSING *")
endif()

if(PerlLibs_FOUND)
	message(STATUS "* Perl:                                    FOUND *")
else()
	message(STATUS "* Perl:                                  MISSING *")
endif()

if(OpenSSL_FOUND)
	message(STATUS "* OpenSSL:                                 FOUND *")
else()
	message(STATUS "* OpenSSL:                               MISSING *")
endif()

message(STATUS "PERL_INCLUDE_PATH: ${PERL_INCLUDE_PATH}")
message(STATUS "PERL_LIBRARY: ${PERL_LIBRARY}")
message(STATUS "PERL_INCLUDE_DIR: ${PERL_INCLUDE_DIR}")
message(STATUS "PERL_INCLUDE_DIRS: ${PERL_INCLUDE_DIRS}")
message(STATUS "PERL_LIBRARIES: ${PERL_LIBRARIES}")
message(STATUS "PERL_VERSION: ${PERL_VERSION}")
message(STATUS "**************************************************")

include_directories(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/submodules/websocketpp")

add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

#options
option(EQEMU_COMMANDS_LOGGING "Enable GM Command logs" ON)
option(EQEMU_BUILD_SERVER "Build the game server." ON)
option(EQEMU_BUILD_LOGIN "Build the login server." ON)
option(EQEMU_BUILD_TESTS "Build utility tests." OFF)
option(EQEMU_BUILD_CLIENT_FILES "Build Client Import/Export Data Programs." ON)
option(EQEMU_PREFER_LUA "Build with normal Lua even if LuaJIT is found." OFF)

#PRNG options
option(EQEMU_ADDITIVE_LFIB_PRNG "Use Additive LFib for PRNG." OFF)
mark_as_advanced(EQEMU_ADDITIVE_LFIB_PRNG)
option(EQEMU_BIASED_INT_DIST "Use biased int dist instead of uniform." OFF)
mark_as_advanced(EQEMU_BIASED_INT_DIST)
set(EQEMU_CUSTOM_PRNG_ENGINE "" CACHE STRING "Custom random engine. (ex. std::default_random_engine)")
mark_as_advanced(EQEMU_CUSTOM_PRNG_ENGINE)

if(CMAKE_COMPILER_IS_GNUCXX)
	option(EQEMU_SFMT19937 "Use GCC's extention for SIMD Fast MT19937." OFF)
	mark_as_advanced(EQEMU_SFMT19937)
endif()

if(EQEMU_ADDITIVE_LFIB_PRNG)
	add_definitions(-DUSE_ADDITIVE_LFIB_PRNG)
	if(EQEMU_SFMT19937)
		message(STATUS "SFMT19937 and ADDITITVE_LFIB_PRNG both set, SFMT19937 ignored.")
		set(EQEMU_SFMT19937 OFF)
	endif()
	if(NOT EQEMU_CUSTOM_PRNG_ENGINE STREQUAL "")
		message(STATUS "CUSTOM_PRNG_ENGINE and ADDITITVE_LFIB_PRNG both set, CUSTOM_PRNG_ENGINE ignored.")
		set(EQEMU_CUSTOM_PRNG_ENGINE "")
	endif()
endif()

if(EQEMU_SFMT19937)
	add_definitions(-DUSE_SFMT19937)
	if(NOT EQEMU_CUSTOM_PRNG_ENGINE STREQUAL "")
		message(STATUS "CUSTOM_PRNG_ENGINE and SFMT19937 both set, CUSTOM_PRNG_ENGINE ignored.")
		set(EQEMU_CUSTOM_PRNG_ENGINE "")
	endif()
endif()

if(NOT EQEMU_CUSTOM_PRNG_ENGINE STREQUAL "")
	add_definitions(-DUSE_CUSTOM_PRNG_ENGINE=${EQEMU_CUSTOM_PRNG_ENGINE})
endif()

if(EQEMU_BIASED_INT_DIST)
	add_definitions(-DBIASED_INT_DIST)
endif()

if(EQEMU_COMMANDS_LOGGING)
	add_definitions(-DCOMMANDS_LOGGING)
endif()

if(${OPENSSL_VERSION} VERSION_GREATER_EQUAL "1.1.1")
	add_definitions(-DCPPHTTPLIB_OPENSSL_SUPPORT)
endif()

if(PerlLibs_FOUND)
	set(PERL_LIBRARY_TYPE "    Perl")
	set(PERL_LIBRARY_ENABLED ON)
	set(PERL_LIBRARY_LIBS ${PERL_LIBRARY})
	set(PERL_LIBRARY_INCLUDE ${PERL_INCLUDE_PATH})
else()
	set(PERL_LIBRARY_TYPE "Disabled")
	set(PERL_LIBRARY_ENABLED OFF)
endif()

message(STATUS "")
message(STATUS "**************************************************")
message(STATUS "* Library Usage                                  *")
message(STATUS "**************************************************")
message(STATUS "* Database:                              MariaDB *")
message(STATUS "* TLS:                                   OpenSSL *")
message(STATUS "* Lua:                                    LuaJIT *")
message(STATUS "* Perl:                                 ${PERL_LIBRARY_TYPE} *")
message(STATUS "* zlib:                                     ZLIB *")
message(STATUS "**************************************************")

option(EQEMU_BUILD_LUA "Build Lua parser." ON)

if(EQEMU_BUILD_LUA)
	add_definitions(-DLUA_EQEMU)

	include_directories(SYSTEM "${LUAJIT_INCLUDE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/libs/luabind")

	option(EQEMU_SANITIZE_LUA_LIBS "Sanitize Lua Libraries (Remove OS and IO standard libraries from being able to run)." ON)
	if(EQEMU_SANITIZE_LUA_LIBS)
		add_definitions(-DSANITIZE_LUA_LIBS)
	endif()
endif()

if(PERL_LIBRARY_ENABLED)
	option(EQEMU_BUILD_PERL "Build Perl parser." ON)
	if(EQEMU_BUILD_PERL)
		set(SERVER_LIBS ${SERVER_LIBS} ${PERL_LIBRARY_LIBS} perlbind)
		INCLUDE_DIRECTORIES(SYSTEM "${PERL_LIBRARY_INCLUDE}" "${CMAKE_CURRENT_SOURCE_DIR}/libs/perlbind/include")
		add_definitions(-DEMBPERL)
		add_definitions(-DEMBPERL_PLUGIN)
		add_definitions(-DPERLBIND_NO_STRICT_SCALAR_TYPES)
	endif()
endif()

set(SERVER_LIBS common)
# silence obnoxious deprecation message
add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

if(WIN32)
	set(SERVER_LIBS ${SERVER_LIBS} "ws2_32" "psapi" "iphlpapi" "userenv")
endif()

if(UNIX)
	set(SERVER_LIBS ${SERVER_LIBS} ${CMAKE_DL_LIBS} "z" "m" "pthread")
	if(NOT DARWIN)
		set(SERVER_LIBS ${SERVER_LIBS} "rt")
	endif()
	# Freebsd provides uuids in the C library
	if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
		set(SERVER_LIBS ${SERVER_LIBS} "uuid")
	endif()
endif()

set(ZONE_LIBS ${ZONE_LIBS} ${SERVER_LIBS})

if(EQEMU_BUILD_SERVER OR EQEMU_BUILD_LOGIN OR EQEMU_BUILD_TESTS)
	add_subdirectory(common)
	add_subdirectory(libs)
endif()

if(EQEMU_BUILD_SERVER)
	add_subdirectory(shared_memory)
	add_subdirectory(world)
	add_subdirectory(zone)
	add_subdirectory(ucs)
	add_subdirectory(queryserv)
	add_subdirectory(eqlaunch)
endif()

if(EQEMU_BUILD_LOGIN)
	add_subdirectory(loginserver)
endif()

if(EQEMU_BUILD_TESTS)
	add_subdirectory(tests)
endif()

if(EQEMU_BUILD_CLIENT_FILES)
	add_subdirectory(client_files)
endif()
