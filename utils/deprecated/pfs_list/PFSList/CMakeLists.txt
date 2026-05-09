SET(pfslist_sources
	Source/main.cpp
)

SET(pfslist_headers
)

INCLUDE_DIRECTORIES(Include
	../Common/Include
)

ADD_EXECUTABLE(PFSList ${pfslist_sources} ${pfslist_headers})

TARGET_LINK_LIBRARIES(PFSList Common ${ZLIB_LIBRARY} "Ws2_32.lib")

IF(MSVC)
	ADD_DEFINITIONS(/D _CONSOLE)
	SET_TARGET_PROPERTIES(PFSList PROPERTIES LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF")
ENDIF(MSVC)

SET(EXECUTABLE_OUTPUT_PATH ../Build/PFSList)
