#
#CVar library
#

set(TARGET_NAME CVar)

add_library(${TARGET_NAME} SHARED)

check_winxp_support(${TARGET_NAME})

target_include_directories(${TARGET_NAME}
	PRIVATE
		${SHARED_INCLUDEPATHS})

target_compile_definitions(${TARGET_NAME}
	PRIVATE	
		${SHARED_DEFS})

target_sources(${TARGET_NAME}
	PRIVATE
		CCVarSystem.cpp
		CCVarSystem.h)

add_subdirectory(../lib ${CMAKE_CURRENT_BINARY_DIR}/lib)

target_link_libraries( ${TARGET_NAME}
	PUBLIC
		HLStdLib
		HLCore
		${SHARED_DEPENDENCIES})

set_target_properties(${TARGET_NAME} 
	PROPERTIES COMPILE_FLAGS "${SHARED_COMPILE_FLAGS}" 
	LINK_FLAGS "${SHARED_LINK_FLAGS}")

#Create filters
get_target_property(SOURCE_FILES ${TARGET_NAME} SOURCES)
source_group(TREE ${SRC_DIR} FILES ${SOURCE_FILES})
