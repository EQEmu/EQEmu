if(EQEMU_BUILD_LUA)
    add_subdirectory(luabind)
endif()

if(EQEMU_BUILD_PERL)
    add_subdirectory(perlbind)
endif()
