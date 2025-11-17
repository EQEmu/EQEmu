CMAKE_MINIMUM_REQUIRED(VERSION 3.17.0)

SET(import_sources
	main.cpp
)

SET(import_headers
)

ADD_EXECUTABLE(import_client_files ${import_sources} ${import_headers})

INSTALL(TARGETS import_client_files RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

TARGET_LINK_LIBRARIES(import_client_files ${SERVER_LIBS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
