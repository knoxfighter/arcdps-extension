CMAKE_MINIMUM_REQUIRED(VERSION 3.25)

PROJECT(ArcdpsExtension CXX)

OPTION(BUILD_TESTS "Build the GTest executable" OFF)
OPTION(ARCDPS_EXTENSION_IMGUI "make imgui tools available" ON)
OPTION(ARCDPS_EXTENSION_UNOFFICIAL_EXTRAS "make tools available, that depend on arcdps-unofficial-extras" ON)

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

FIND_PACKAGE(magic_enum CONFIG REQUIRED)
FIND_PACKAGE(nlohmann_json CONFIG REQUIRED)
FIND_PACKAGE(CURL REQUIRED)

ADD_LIBRARY(${PROJECT_NAME} STATIC)
ADD_LIBRARY(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# Use -MT / -MTd runtime library
SET_PROPERTY(TARGET ${PROJECT_NAME} PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

TARGET_INCLUDE_DIRECTORIES(
		${PROJECT_NAME}
		INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		$<INSTALL_INTERFACE:include>
)

TARGET_COMPILE_DEFINITIONS(
		${PROJECT_NAME}
		PUBLIC
		NOMINMAX
		MAGIC_ENUM_RANGE_MAX=256
)

# add general sources
TARGET_SOURCES(${PROJECT_NAME} PUBLIC
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
		SimpleNetworkStack.h
		SimpleRingBuffer.h
		Singleton.h
		UpdateCheckerBase.h
)

TARGET_SOURCES(${PROJECT_NAME}
		PRIVATE
		ArcdpsExtension.cpp
		arcdps_structs.cpp
		CombatEventHandler.cpp
		EventSequencer.cpp
		IconLoader.cpp
		Localization.cpp
		SimpleNetworkStack.cpp
		Singleton.cpp
		UpdateCheckerBase.cpp
)

# add sources that depend on imgui
IF (ARCDPS_EXTENSION_IMGUI)
	INCLUDE(cmake/imgui-dep.cmake)
ENDIF ()

# add sources that depend on unofficial extras
IF (ARCDPS_EXTENSION_UNOFFICIAL_EXTRAS)
	INCLUDE(cmake/unofficial-extras-dep.cmake)
ENDIF ()

# add sources that depend on imgui AND unofficial extras
IF (ARCDPS_EXTENSION_IMGUI AND ARCDPS_EXTENSION_UNOFFICIAL_EXTRAS)
	TARGET_SOURCES(${PROJECT_NAME} PUBLIC
			FILE_SET HEADERS
			FILES
			KeyInput.h
	)

	TARGET_SOURCES(${PROJECT_NAME}
			PRIVATE
			KeyInput.cpp
	)
ENDIF ()

TARGET_COMPILE_FEATURES(${PROJECT_NAME} PUBLIC cxx_std_23)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC magic_enum::magic_enum)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC nlohmann_json::nlohmann_json)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC CURL::libcurl)

INCLUDE(GNUInstallDirs)
INSTALL(TARGETS ${PROJECT_NAME}
		EXPORT ArcdpsExtensionTargets
		FILE_SET HEADERS
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)

INCLUDE(CMakePackageConfigHelpers)
CONFIGURE_PACKAGE_CONFIG_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
		${CMAKE_CURRENT_BINARY_DIR}/ArcdpsExtensionConfig.cmake
		INSTALL_DESTINATION share/${PROJECT_NAME}
)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/ArcdpsExtensionConfig.cmake
		DESTINATION share/${PROJECT_NAME}
)

INSTALL(EXPORT ArcdpsExtensionTargets
		NAMESPACE ${PROJECT_NAME}::
		FILE ArcdpsExtensionTargets.cmake
		DESTINATION share/${PROJECT_NAME}
)

INSTALL(FILES cmake/vs-version.cmake cmake/VersionInfo.rc.in DESTINATION share/${PROJECT_NAME})

# install natvis file
#INSTALL(FILES SimpleRingBuffer.natvis DESTINATION .) # maybe share/${PROJECT_NAME} as destination (nlohman_json uses . though)

IF (BUILD_TESTS)
	FIND_PACKAGE(GTest CONFIG REQUIRED)
	INCLUDE(GoogleTest)
	ADD_EXECUTABLE(
			${PROJECT_NAME}Tests
			UpdateCheckerTest.cpp
			SimpleRingBufferTests.cpp
			SimpleNetworkStackTests.cpp
			IconLoaderTests.cpp
			test/tests.rc
			test/resource.h
	)

	# Use -MT / -MTd runtime library
	SET_PROPERTY(TARGET ${PROJECT_NAME}Tests PROPERTY
			MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

	TARGET_LINK_LIBRARIES(${PROJECT_NAME}Tests PRIVATE ArcdpsExtension::ArcdpsExtension GTest::gtest GTest::gtest_main)
	IF (WIN32)
		TARGET_LINK_LIBRARIES(${PROJECT_NAME}Tests PRIVATE Version.lib d3d11.lib)
	ELSE ()
		# Link libraries on non-windows, is this even possible?
	ENDIF ()

	GTEST_DISCOVER_TESTS(${PROJECT_NAME}Tests)

	TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME}Tests PUBLIC TEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/test/\")
ENDIF ()
