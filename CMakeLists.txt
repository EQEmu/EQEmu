cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/" ${CMAKE_MODULE_PATH})

if(NOT CMAKE_TOOLCHAIN_FILE)
	if(DEFINED ENV{VCPKG_ROOT})
		message(STATUS "Using vcpkg from VCPKG_ROOT")
        set(CMAKE_TOOLCHAIN_FILE
            "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
            CACHE FILEPATH "Vcpkg toolchain file"
        )
	else()
        message(STATUS "Using vcpkg submodule")
        set(CMAKE_TOOLCHAIN_FILE
            "${CMAKE_CURRENT_SOURCE_DIR}/submodules/vcpkg/scripts/buildsystems/vcpkg.cmake"
            CACHE FILEPATH "Vcpkg toolchain file"
        )
    endif()
endif()

project(EQEmu
	VERSION 24.10.3
	LANGUAGES CXX 
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#explicitly set CMP0167 for Find Boost
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(EQEMU_BUILD_PCH "Build with precompiled headers (Windows)" ON)

if(MSVC)
	add_compile_options(/bigobj)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS NOMINMAX CRASH_LOGGING _HAS_AUTO_PTR_ETC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

	option(EQEMU_DISABLE_MSVC_WARNINGS "Disable MSVC compile warnings." ON)
	if(EQEMU_DISABLE_MSVC_WARNINGS)
		add_compile_options(/W0 /wd4005 /wd4996 /nologo /Os)
	endif()
else()
	add_compile_definitions(HAS_UNION_SEMUN)
endif()

#FreeBSD support
if(UNIX)
	if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
		add_compile_definitions(FREEBSD)
		add_compile_definitions(_GLIBCXX_USE_C99)
		set(FREEBSD TRUE)
	endif()
	if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
		add_compile_definitions(DARWIN)
		set(DARWIN TRUE)
	endif()
endif()

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
find_package(unofficial-sodium CONFIG REQUIRED)
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

#options
option(EQEMU_BUILD_SERVER "Build the game server." ON)
option(EQEMU_BUILD_LOGIN "Build the login server." ON)
option(EQEMU_BUILD_TESTS "Build utility tests." OFF)
option(EQEMU_BUILD_CLIENT_FILES "Build Client Import/Export Data Programs." ON)

if(PerlLibs_FOUND)
	option(EQEMU_BUILD_PERL "Build Perl parser." ON)
	
	if(EQEMU_BUILD_PERL)
		set(PERL_LIBRARY_TYPE "    Perl")
	else()
		set(PERL_LIBRARY_TYPE " Missing")
	endif()
else()
	set(PERL_LIBRARY_TYPE "Disabled")
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

if(EQEMU_BUILD_SERVER OR EQEMU_BUILD_LOGIN OR EQEMU_BUILD_TESTS OR EQEMU_BUILD_CLIENT_FILES)
	add_subdirectory(common)
	add_subdirectory(libs)
else()
	message(FATAL_ERROR "No targets were selected to build, we must build at least one target.")
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
