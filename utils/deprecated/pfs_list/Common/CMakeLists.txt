SET(common_sources
	Source/PFSArchive.cpp
	Source/Compression.cpp
)

SET(common_headers
	Include/Archive.h
	Include/PFSArchive.h
	Include/PFSDataStructs.h
	Include/Compression.h
)

INCLUDE_DIRECTORIES(Include)

ADD_LIBRARY(Common ${common_sources} ${common_headers})
