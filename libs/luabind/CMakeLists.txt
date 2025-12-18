cmake_minimum_required(VERSION 3.20.0)

set(lb_sources
    src/class.cpp
	src/class_info.cpp
	src/class_registry.cpp
	src/class_rep.cpp
	src/create_class.cpp
	src/error.cpp
	src/exception_handler.cpp
	src/function.cpp
	src/inheritance.cpp
	src/link_compatibility.cpp
	src/object_rep.cpp
	src/open.cpp
	src/pcall.cpp
	src/scope.cpp
	src/stack_content_by_name.cpp
	src/weak_ref.cpp
	src/wrapper_base.cpp
)

add_library(luabind ${lb_sources})

target_include_directories(luabind PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${LUAJIT_INCLUDE_DIR})
target_link_libraries(luabind PUBLIC Boost::dynamic_bitset Boost::tuple Boost::foreach ${LUAJIT_LIBRARY})
target_include_directories(luabind PRIVATE ..)

if(UNIX)
    set_source_files_properties(${lb_sources} PROPERTY COMPILE_FLAGS -Wno-deprecated-declarations)
endif()

if(MSVC)
	set_source_files_properties(${lb_sources} PROPERTY COMPILE_FLAGS " /W0 " )
endif()

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set_property(TARGET luabind PROPERTY FOLDER contrib)
