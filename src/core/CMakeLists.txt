#
#HLCore library
#

set(TARGET_NAME HLCore)

add_library(${TARGET_NAME} SHARED)

check_winxp_support(${TARGET_NAME})

target_include_directories(${TARGET_NAME} PRIVATE
	${SHARED_INCLUDEPATHS})

#TODO: CMake adds exports macros automatically - Solokiller
target_compile_definitions(${TARGET_NAME}
	PRIVATE	
		${SHARED_DEFS}
		LIBHLCORE_EXPORTS)

target_sources(${TARGET_NAME}
	PRIVATE
		LibHLCore.cpp
		LibHLCore.h)

add_subdirectory(shared)

target_link_libraries(${TARGET_NAME}
	PUBLIC
		Keyvalues
		HLStdLib
		${SHARED_DEPENDENCIES})

set_target_properties(${TARGET_NAME} 
	PROPERTIES COMPILE_FLAGS "${SHARED_COMPILE_FLAGS}" 
	LINK_FLAGS "${SHARED_LINK_FLAGS}")

#Create filters
get_target_property(SOURCE_FILES ${TARGET_NAME} SOURCES)
source_group(TREE ${SRC_DIR} FILES ${SOURCE_FILES})
