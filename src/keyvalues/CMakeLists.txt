#
#Keyvalues library
#

set( TARGET_NAME Keyvalues )

#Add in the shared sources
add_sources( ${SHARED_SRCS} )

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

add_library( ${TARGET_NAME} STATIC ${PREP_SRCS} )

target_include_directories( ${TARGET_NAME} PRIVATE
	${SHARED_INCLUDEPATHS}
)

target_compile_definitions( ${TARGET_NAME} PRIVATE	
	${SHARED_DEFS}
)

set_target_properties( ${TARGET_NAME} 
	PROPERTIES COMPILE_FLAGS "${SHARED_COMPILE_FLAGS}" 
	LINK_FLAGS "${SHARED_LINK_FLAGS}"
)

target_link_libraries( ${TARGET_NAME}
	HLStdLib
)

#Create filters
create_source_groups( "${CMAKE_SOURCE_DIR}" )

clear_sources()
