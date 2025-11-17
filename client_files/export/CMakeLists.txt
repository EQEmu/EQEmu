CMAKE_MINIMUM_REQUIRED(VERSION 3.17.0)

SET(export_sources
	main.cpp
)

SET(export_headers
)

ADD_EXECUTABLE(export_client_files ${export_sources} ${export_headers})

INSTALL(TARGETS export_client_files RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

TARGET_LINK_LIBRARIES(export_client_files ${SERVER_LIBS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
