#
#Keyvalues library
#

set(TARGET_NAME Keyvalues)

add_library(${TARGET_NAME} STATIC)

check_winxp_support(${TARGET_NAME})

target_include_directories(${TARGET_NAME}
	PRIVATE
		${SHARED_INCLUDEPATHS})

target_compile_definitions(${TARGET_NAME}
	PRIVATE	
		${SHARED_DEFS})

target_sources(${TARGET_NAME}
	PRIVATE
		CKeyvalue.cpp
		CKeyvalue.h
		CKeyvalueBlock.cpp
		CKeyvalueBlock.h
		CKeyvalueNode.cpp
		CKeyvalueNode.h
		CKeyvaluesLexer.cpp
		CKeyvaluesLexer.h
		CKeyvaluesParser.cpp
		CKeyvaluesParser.h
		CKeyvaluesWriter.cpp
		CKeyvaluesWriter.h
		Keyvalues.h
		KeyvaluesConstants.h
		KVForward.h)

set_target_properties(${TARGET_NAME} 
	PROPERTIES COMPILE_FLAGS "${SHARED_COMPILE_FLAGS}" 
	LINK_FLAGS "${SHARED_LINK_FLAGS}")

target_link_libraries(${TARGET_NAME}
	PUBLIC
		HLStdLib)

#Create filters
get_target_property(SOURCE_FILES ${TARGET_NAME} SOURCES)
source_group(TREE ${SRC_DIR} FILES ${SOURCE_FILES})
