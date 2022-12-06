# Generate ProjectInfo.h with appropriate information.

set(REPO_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

# Get the current working branch
execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${REPO_ROOT_DIR}
    OUTPUT_VARIABLE HLAM_GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE)

# CI builds don't fetch tags so this should have a sensible value.
if ("$ENV{CI}" STREQUAL "true")
	set(HLAM_GIT_TAG "CI build")
else()
	# Get the latest tag name
	execute_process(
		COMMAND ${GIT_EXECUTABLE} describe --abbrev=0 --tags
		WORKING_DIRECTORY ${REPO_ROOT_DIR}
		OUTPUT_VARIABLE HLAM_GIT_TAG
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_QUIET)
		
	# This can happen if there are no tags in the repository, so provide a suitable fallback.
	if (HLAM_GIT_TAG STREQUAL "")
		set(HLAM_GIT_TAG "None (No tags found)")
	endif()
endif()

# Get the latest commit hash
execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
    WORKING_DIRECTORY ${REPO_ROOT_DIR}
    OUTPUT_VARIABLE HLAM_GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(
    COMMAND ${GIT_EXECUTABLE} status --porcelain=v1
    WORKING_DIRECTORY ${REPO_ROOT_DIR}
    OUTPUT_VARIABLE HLAM_GIT_STATUS_RESULT
    OUTPUT_STRIP_TRAILING_WHITESPACE)

# If not performing a CI build, mark commit as dirty if there are uncommitted changes.
if (NOT "$ENV{CI}" STREQUAL "true" AND NOT HLAM_GIT_STATUS_RESULT STREQUAL "")
	set(HLAM_GIT_COMMIT_HASH "${HLAM_GIT_COMMIT_HASH}-dirty")
endif()

configure_file(${REPO_ROOT_DIR}/ProjectInfo.hpp.in ${CMAKE_BINARY_DIR}/ProjectInfo.hpp @ONLY)
