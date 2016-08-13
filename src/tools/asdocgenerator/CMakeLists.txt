#
#ASDocGenerator exe
#

#Add in the shared sources
add_sources( "${SHARED_SRCS}" )

#Add sources
add_sources(
	CDocGenerator.h
	CDocGenerator.cpp
	Main.cpp
)

preprocess_sources()

include_directories(
	"${CMAKE_SOURCE_DIR}/external/Angelscript/include"
	"${SHARED_INCLUDEPATHS}"
)

add_executable( ASDocGenerator ${PREP_SRCS} )

target_compile_definitions( ASDocGenerator PRIVATE	
	"${SHARED_DEFS}"
)

target_link_libraries( ASDocGenerator
	HLCore
	"${SHARED_DEPENDENCIES}"
)

set_target_properties( ASDocGenerator 
	PROPERTIES COMPILE_FLAGS "${SHARED_COMPILE_FLAGS}" 
	LINK_FLAGS "${SHARED_LINK_FLAGS}"
)

#Create filters
create_source_groups( "${CMAKE_CURRENT_SOURCE_DIR}" )

clear_sources()
