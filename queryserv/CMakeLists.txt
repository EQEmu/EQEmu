cmake_minimum_required(VERSION 3.20)

set(qserv_sources
    database.cpp
    lfguild.cpp
    queryserv.cpp
    queryservconfig.cpp
    worldserver.cpp
    zonelist.cpp
    zoneserver.cpp
)

set(qserv_headers
    database.h
    lfguild.h
    queryservconfig.h
    worldserver.h
    zonelist.h
    zoneserver.h
)

add_executable(queryserv ${qserv_sources} ${qserv_headers})

install(TARGETS queryserv RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

add_definitions(-DQSERV)

target_link_libraries(queryserv common)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
