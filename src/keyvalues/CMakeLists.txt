#
#Keyvalues library
#

#Add in the shared sources
add_sources( "${SHARED_SRCS}" )

#Add sources

add_sources(
	CKeyvalue.h
	CKeyvalue.cpp
	CKeyvalueBlock.h
	CKeyvalueBlock.cpp
	CKeyvalueNode.h
	CKeyvalueNode.cpp
	CKeyvaluesLexer.h
	CKeyvaluesLexer.cpp
	CKeyvaluesParser.h
	CKeyvaluesParser.cpp
	CKeyvaluesWriter.h
	CKeyvaluesWriter.cpp
	Keyvalues.h
	KeyvaluesConstants.h
	KVForward.h
)

add_includes(
	CKeyvalue.h
	CKeyvalueBlock.h
	CKeyvalueNode.h
	CKeyvaluesLexer.h
	CKeyvaluesParser.h
	CKeyvaluesWriter.h
	Keyvalues.h
	KeyvaluesConstants.h
	KVForward.h
)

preprocess_sources()

include_directories(
	"${SHARED_INCLUDEPATHS}"
)

add_library( Keyvalues STATIC ${PREP_SRCS} )

target_compile_definitions( Keyvalues PRIVATE	
	"${SHARED_DEFS}"
)

set_target_properties( Keyvalues 
	PROPERTIES COMPILE_FLAGS "${SHARED_COMPILE_FLAGS}" 
	LINK_FLAGS "${SHARED_LINK_FLAGS}"
)

target_link_libraries( Keyvalues
	HLStdLib
)

#Create filters
create_source_groups( "${CMAKE_SOURCE_DIR}" )

clear_sources()
