cmake_minimum_required(VERSION 3.25)

include(FetchContent)
FetchContent_Declare(my-vcpkg-triplets
		GIT_REPOSITORY "https://github.com/Zinn-o-Matics/my-vcpkg-triplets.git"
		GIT_TAG "master"
)
FetchContent_MakeAvailable(my-vcpkg-triplets)

list(APPEND VCPKG_OVERLAY_TRIPLETS "${my-vcpkg-triplets_SOURCE_DIR}")

project(ArcdpsExtension CXX)

option(BUILD_TESTS "Build the GTest executable" OFF)
option(ARCDPS_EXTENSION_CURL "make tools available, that depend on curl" ON)
option(ARCDPS_EXTENSION_IMGUI "make imgui tools available" ON)
option(ARCDPS_EXTENSION_UNOFFICIAL_EXTRAS "make tools available, that depend on arcdps-unofficial-extras" ON)

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)

find_package(magic_enum CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(${PROJECT_NAME} STATIC)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# Use -MT / -MTd runtime library
set_property(TARGET ${PROJECT_NAME} PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_include_directories(
		${PROJECT_NAME}
		INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		$<INSTALL_INTERFACE:include>
)

target_compile_definitions(
		${PROJECT_NAME}
		PUBLIC
		NOMINMAX
		MAGIC_ENUM_RANGE_MAX=256
)

# add general sources
target_sources(${PROJECT_NAME}
		PUBLIC
		FILE_SET HEADERS
		FILES
		ArcdpsExtension.h
		arcdps_structs.h
		arcdps_structs_slim.h
		CombatEventHandler.h
		EventSequencer.h
		ExtensionTranslations.h
		IconLoader.h
		Localization.h
		map.h
		MobIDs.h
		MumbleLink.h
		nlohmannJsonExtension.h
		rapidfuzz_amalgamated.hpp
		SimpleRingBuffer.h
		Singleton.h
		UpdateCheckerBase.h
)

target_sources(${PROJECT_NAME}
		PRIVATE
		ArcdpsExtension.cpp
		arcdps_structs.cpp
		CombatEventHandler.cpp
		EventSequencer.cpp
		IconLoader.cpp
		Localization.cpp
		Singleton.cpp
		UpdateCheckerBase.cpp
)

# add sources that depend on imgui
if (ARCDPS_EXTENSION_IMGUI)
	include(cmake/imgui-dep.cmake)
endif ()

# add sources that depend on unofficial extras
if (ARCDPS_EXTENSION_UNOFFICIAL_EXTRAS)
	include(cmake/unofficial-extras-dep.cmake)
endif ()

# add sources that depend on imgui AND unofficial extras
if (ARCDPS_EXTENSION_IMGUI AND ARCDPS_EXTENSION_UNOFFICIAL_EXTRAS)
	target_sources(${PROJECT_NAME}
			PUBLIC
			FILE_SET HEADERS
			FILES
			KeyInput.h
			Windows/Demo/DemoKeyBindComponent.h
			Windows/KeyBindComponent.h
	)

	target_sources(${PROJECT_NAME}
			PRIVATE
			KeyInput.cpp
			Windows/Demo/DemoKeyBindComponent.cpp
			Windows/KeyBindComponent.cpp
	)
endif ()

# add sources that depend on curl, also links curl
if (ARCDPS_EXTENSION_CURL)
	include(cmake/curl-dep.cmake)
endif ()

# add sources that depend on imgui AND curl
if (ARCDPS_EXTENSION_CURL AND ARCDPS_EXTENSION_IMGUI)
	target_sources(${PROJECT_NAME}
			PUBLIC
			FILE_SET HEADERS
			FILES
			UpdateChecker.h
	)

	target_sources(${PROJECT_NAME}
			PRIVATE
			UpdateChecker.cpp
	)
endif ()

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

target_link_libraries(${PROJECT_NAME} PUBLIC magic_enum::magic_enum)
target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PUBLIC Version.lib d3d11.lib)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
		EXPORT ArcdpsExtensionTargets
		FILE_SET HEADERS
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
		${CMAKE_CURRENT_BINARY_DIR}/ArcdpsExtensionConfig.cmake
		INSTALL_DESTINATION share/${PROJECT_NAME}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ArcdpsExtensionConfig.cmake
		DESTINATION share/${PROJECT_NAME}
)

install(EXPORT ArcdpsExtensionTargets
		NAMESPACE ${PROJECT_NAME}::
		FILE ArcdpsExtensionTargets.cmake
		DESTINATION share/${PROJECT_NAME}
)

install(FILES cmake/vs-version.cmake cmake/VersionInfo.rc.in DESTINATION share/${PROJECT_NAME})

# install natvis file
#INSTALL(FILES SimpleRingBuffer.natvis DESTINATION .) # maybe share/${PROJECT_NAME} as destination (nlohman_json uses . though)

# when tests are build, curl should always be defined
if (BUILD_TESTS AND ARCDPS_EXTENSION_CURL)
	find_package(GTest CONFIG REQUIRED)
	include(GoogleTest)
	add_executable(
			${PROJECT_NAME}Tests
			UpdateCheckerTest.cpp
			SimpleRingBufferTests.cpp
			SimpleNetworkStackTests.cpp
			IconLoaderTests.cpp
			EventSequencerTests.cpp
			test/tests.rc
			test/resource.h
	)

	# Use -MT / -MTd runtime library
	set_property(TARGET ${PROJECT_NAME}Tests PROPERTY
			MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

	target_link_libraries(${PROJECT_NAME}Tests PRIVATE ArcdpsExtension::ArcdpsExtension GTest::gtest GTest::gtest_main)
	if (WIN32)
		target_link_libraries(${PROJECT_NAME}Tests PRIVATE Version.lib d3d11.lib)
	else ()
		# Link libraries on non-windows, is this even possible?
	endif ()

	gtest_discover_tests(${PROJECT_NAME}Tests)

	target_compile_definitions(${PROJECT_NAME}Tests PUBLIC TEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/test/\")
endif ()
