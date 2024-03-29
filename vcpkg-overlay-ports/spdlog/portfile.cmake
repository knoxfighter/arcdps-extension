vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO gabime/spdlog
    REF v1.11.0
    SHA512 210f3135c7af3ec774ef9a5c77254ce172a44e2fa720bf590e1c9214782bf5c8140ff683403a85b585868bc308286fbdeb1c988e4ed1eb3c75975254ffe75412
    HEAD_REF v1.x
)

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        benchmark   SPDLOG_BUILD_BENCH
        wchar       SPDLOG_WCHAR_SUPPORT
        stdformat   SPDLOG_USE_STD_FORMAT
        fmt         SPDLOG_FMT_EXTERNAL
)

if(NOT DEFINED SPDLOG_FMT_EXTERNAL)
    set(SPDLOG_FMT_EXTERNAL ON)
endif()

# SPDLOG_WCHAR_FILENAMES can only be configured in triplet file since it is an alternative (not additive)
if(NOT DEFINED SPDLOG_WCHAR_FILENAMES)
    set(SPDLOG_WCHAR_FILENAMES OFF)
endif()
if(NOT VCPKG_TARGET_IS_WINDOWS)
    if("wchar" IN_LIST FEATURES)
        message(WARNING "Feature 'wchar' is only supported for Windows and has no effect on other platforms.")
    elseif(SPDLOG_WCHAR_FILENAMES)
        message(FATAL_ERROR "Build option 'SPDLOG_WCHAR_FILENAMES' is for Windows.")
    endif()
endif()

string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "dynamic" SPDLOG_BUILD_SHARED)

# if using std::format, spdlog requires minimum c++20 
set(CXX_STANDARD_OPTION ${CMAKE_CXX_STANDARD})
if (SPDLOG_USE_STD_FORMAT AND (NOT DEFINED CXX_STANDARD_OPTION OR CXX_STANDARD_OPTION LESS 17))
    set(CXX_STANDARD_OPTION 20)
endif()

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        ${FEATURE_OPTIONS}
        -DSPDLOG_FMT_EXTERNAL=${SPDLOG_FMT_EXTERNAL}
        -DSPDLOG_INSTALL=ON
        -DSPDLOG_BUILD_SHARED=${SPDLOG_BUILD_SHARED}
        -DSPDLOG_WCHAR_FILENAMES=${SPDLOG_WCHAR_FILENAMES}
        -DSPDLOG_BUILD_EXAMPLE=OFF
        -DCMAKE_CXX_STANDARD=${CXX_STANDARD_OPTION}
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/spdlog)
vcpkg_fixup_pkgconfig()
vcpkg_copy_pdbs()

# use vcpkg-provided fmt library (see also option SPDLOG_FMT_EXTERNAL above)
if(SPDLOG_FMT_EXTERNAL)
    file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/include/spdlog/fmt/bundled")
    # add support for integration other than cmake
    vcpkg_replace_string(${CURRENT_PACKAGES_DIR}/include/spdlog/tweakme.h
        "// #define SPDLOG_FMT_EXTERNAL"
        "#ifndef SPDLOG_FMT_EXTERNAL\n#define SPDLOG_FMT_EXTERNAL\n#endif"
    )
endif()
if(SPDLOG_USE_STD_FORMAT)
    vcpkg_replace_string(${CURRENT_PACKAGES_DIR}/include/spdlog/tweakme.h
        "// #define SPDLOG_USE_STD_FORMAT"
        "#ifndef SPDLOG_USE_STD_FORMAT\n#define SPDLOG_USE_STD_FORMAT\n#endif"
    )
endif()

if(SPDLOG_WCHAR_SUPPORT)
    vcpkg_replace_string(${CURRENT_PACKAGES_DIR}/include/spdlog/tweakme.h
        "// #define SPDLOG_WCHAR_TO_UTF8_SUPPORT"
        "#ifndef SPDLOG_WCHAR_TO_UTF8_SUPPORT\n#define SPDLOG_WCHAR_TO_UTF8_SUPPORT\n#endif"
    )
endif()
if(SPDLOG_WCHAR_FILENAMES)
    vcpkg_replace_string(${CURRENT_PACKAGES_DIR}/include/spdlog/tweakme.h
        "// #define SPDLOG_WCHAR_FILENAMES"
        "#ifndef SPDLOG_WCHAR_FILENAMES\n#define SPDLOG_WCHAR_FILENAMES\n#endif"
    )
endif()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include"
                    "${CURRENT_PACKAGES_DIR}/debug/share")

# Handle copyright
file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)