CMAKE_MINIMUM_REQUIRED(VERSION 3.25)

PROJECT(ArcdpsExtension CXX)

OPTION(BUILD_TESTS "Build the GTest executable" OFF)

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

FIND_PACKAGE(imgui CONFIG REQUIRED)
FIND_PACKAGE(magic_enum CONFIG REQUIRED)
FIND_PACKAGE(nlohmann_json CONFIG REQUIRED)
FIND_PACKAGE(ArcdpsUnofficialExtras CONFIG REQUIRED)
FIND_PACKAGE(CURL REQUIRED)

ADD_LIBRARY(${PROJECT_NAME} STATIC)
ADD_LIBRARY(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# Use -MT / -MTd runtime library
SET_PROPERTY(TARGET ${PROJECT_NAME} PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

TARGET_INCLUDE_DIRECTORIES(
		${PROJECT_NAME}
		PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		$<INSTALL_INTERFACE:include>
)

ADD_COMPILE_DEFINITIONS(
		NOMINMAX
		MAGIC_ENUM_RANGE_MAX=256
		IMGUI_DEFINE_MATH_OPERATORS
)

TARGET_SOURCES(${PROJECT_NAME} PRIVATE
		FILE_SET HEADERS
		FILES
		arcdps_structs.h
		arcdps_structs_slim.h
		ArcdpsExtension.h
		CombatEventHandler.h
		EventSequencer.h
		ExtensionTranslations.h
		IconLoader.h
		ImGui_Math.h
		imgui_stdlib.h
		KeyBindHandler.h
		KeyInput.h
		Localization.h
		map.h
		MobIDs.h
		MumbleLink.h
		nlohmannJsonExtension.h
		rapidfuzz_amalgamated.hpp
		SimpleNetworkStack.h
		SimpleRingBuffer.h
		Singleton.h
		UpdateChecker.h
		UpdateCheckerBase.h
		Widgets.h
		Windows/ComponentBase.h
		Windows/Demo/DemoKeyBindComponent.h
		Windows/Demo/DemoPositioningComponent.h
		Windows/Demo/DemoTable.h
		Windows/Demo/DemoTableWindow.h
		Windows/Demo/DemoWindow.h
		Windows/KeyBindComponent.h
		Windows/MainTable.h
		Windows/MainWindow.h
		Windows/PositioningComponent.h
)

TARGET_SOURCES(${PROJECT_NAME}
		PRIVATE
		arcdps_structs.cpp
		CombatEventHandler.cpp
		EventSequencer.cpp
		IconLoader.cpp
		imgui_stdlib.cpp
		KeyBindHandler.cpp
		KeyInput.cpp
		Localization.cpp
		SimpleNetworkStack.cpp
		Singleton.cpp
		UpdateChecker.cpp
		UpdateCheckerBase.cpp
		Widgets.cpp
		Windows/Demo/DemoKeyBindComponent.cpp
		Windows/Demo/DemoPositioningComponent.cpp
		Windows/Demo/DemoTable.cpp
		Windows/Demo/DemoTableWindow.cpp
		Windows/Demo/DemoWindow.cpp
		Windows/KeyBindComponent.cpp
		Windows/MainWindow.cpp
		Windows/PositioningComponent.cpp
)

TARGET_COMPILE_FEATURES(${PROJECT_NAME} PUBLIC cxx_std_23)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE imgui::imgui)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE magic_enum::magic_enum)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE ArcdpsUnofficialExtras)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE CURL::libcurl)

INSTALL(TARGETS ${PROJECT_NAME}
		EXPORT ArcdpsExtensionTargets
		FILE_SET HEADERS
		DESTINATION include/${PROJECT_NAME}
)

INCLUDE(CMakePackageConfigHelpers)
CONFIGURE_PACKAGE_CONFIG_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
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
