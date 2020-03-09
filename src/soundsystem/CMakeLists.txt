#
#SoundSystem library
#

set(TARGET_NAME SoundSystem)

# Disable module based lookup (OpenAL Soft uses CONFIG mode and MODULE mode only works with the Creative Labs version)
find_package(OpenAL REQUIRED NO_MODULE)

add_library(${TARGET_NAME} SHARED)

check_winxp_support(${TARGET_NAME})

target_include_directories(${TARGET_NAME}
	PRIVATE
		${EXTERNAL_DIR}/AudioFile/include
		${SHARED_INCLUDEPATHS})

target_compile_definitions(${TARGET_NAME}
	PRIVATE	
		${SHARED_DEFS})

target_sources(${TARGET_NAME}
	PRIVATE
		CSoundSystem.cpp
		CSoundSystem.h)

add_subdirectory(shared)

add_subdirectory(../lib ${CMAKE_CURRENT_BINARY_DIR}/lib)

target_link_libraries(${TARGET_NAME}
	PRIVATE
		HLCore
		${SHARED_DEPENDENCIES}
		OpenAL::OpenAL)

set_target_properties(${TARGET_NAME} 
	PROPERTIES COMPILE_FLAGS "${SHARED_COMPILE_FLAGS}" 
	LINK_FLAGS "${SHARED_LINK_FLAGS}")

#Create filters
get_target_property(SOURCE_FILES ${TARGET_NAME} SOURCES)
source_group(TREE ${SRC_DIR} FILES ${SOURCE_FILES})
