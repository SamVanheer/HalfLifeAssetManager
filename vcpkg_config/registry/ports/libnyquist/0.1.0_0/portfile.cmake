vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO ddiakopoulos/libnyquist
    REF e95e517748bf9724a08473e78d8c1497bba2e4bd
    SHA512 e5e6c7ff7b5406a1b351cd4bb487914b25efe68f1d1082edbe44dd20a1fc7907b9c06def5be835e79e5b477fda42fea18b1a6e4439fcb042198011d484c47a40
    HEAD_REF main
	PATCHES
        install-config.patch
)

file(COPY
    ${CMAKE_CURRENT_LIST_DIR}/libnyquistConfig.cmake.in
    DESTINATION ${SOURCE_PATH}
)

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DLIBNYQUIST_ENABLE_AVX=OFF
        -DLIBNYQUIST_BUILD_EXAMPLE=OFF
	MAYBE_UNUSED_VARIABLES
		LIBNYQUIST_ENABLE_AVX
)

vcpkg_install_cmake()

set(PKG_NAME "libnyquist")
vcpkg_fixup_cmake_targets(CONFIG_PATH lib/cmake/${PKG_NAME} TARGET_PATH share/${PKG_NAME})

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)

file(INSTALL ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
file(INSTALL ${CMAKE_CURRENT_LIST_DIR}/usage DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT})

