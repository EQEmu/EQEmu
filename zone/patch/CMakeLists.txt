set(patch_sources
    client_version.cpp
)

set(patch_headers
    client_version.h
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES ${patch_sources})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Header Files" FILES ${patch_headers})

target_sources(zone PRIVATE ${patch_sources} ${patch_headers})

add_subdirectory(components)