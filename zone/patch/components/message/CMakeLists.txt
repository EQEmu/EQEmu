set(message_component_sources
    titanium.cpp
    rof2.cpp
    tob.cpp
)

set(message_component_headers
    IMessage.h
    titanium.h
    sof.h
    sod.h
    uf.h
    rof.h
    rof2.h
    tob.h
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES ${message_component_sources})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Header Files" FILES ${message_component_headers})

target_sources(zone PRIVATE ${message_component_sources} ${message_component_headers})