cmake_minimum_required(VERSION 3.23)

project(ArcdpsExtension CXX)
set(CMAKE_CXX_STANDARD 20)

option(BUILD_TESTS "Build the GTest executable" OFF)

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)

find_package(cpr CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(ArcdpsUnofficialExtras REQUIRED)

add_library(${PROJECT_NAME} STATIC)
add_library(${PROJECT_NAME}::extension ALIAS ${PROJECT_NAME})

target_include_directories(
		${PROJECT_NAME}
		PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		$<INSTALL_INTERFACE:include/${PROJECT_NAME}>
)

add_compile_definitions(
	NOMINMAX
	MAGIC_ENUM_RANGE_MAX=256
	IMGUI_DEFINE_MATH_OPERATORS
)

target_sources(${PROJECT_NAME} INTERFACE
	FILE_SET HEADERS
	FILES
		arcdps_structs.h
		arcdps_structs_slim.h
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

target_sources(${PROJECT_NAME}
	PRIVATE
		arcdps_structs.cpp
		CombatEventHandler.cpp
		EventSequencer.cpp
		IconLoader.cpp
		imgui_stdlib.cpp
		KeyBindHandler.cpp
		KeyInput.cpp
		Localization.cpp
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

target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE cpr::cpr)
target_link_libraries(${PROJECT_NAME} PRIVATE magic_enum::magic_enum)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE ArcdpsUnofficialExtras)

install(TARGETS ${PROJECT_NAME}
	EXPORT ArcdpsExtensionTargets
	FILE_SET HEADERS
	DESTINATION include/${PROJECT_NAME}
)

include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/ArcdpsExtensionConfig.cmake
	INSTALL_DESTINATION share/${PROJECT_NAME}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ArcdpsExtensionConfig.cmake
	DESTINATION share/${PROJECT_NAME}
)

install(EXPORT ArcdpsExtensionTargets
	FILE ArcdpsExtensionTargets.cmake
	DESTINATION share/${PROJECT_NAME}
)

IF(BUILD_TESTS)
	find_package(GTest CONFIG REQUIRED)
	include(GoogleTest)
	add_executable(
			${PROJECT_NAME}Tests
			UpdateCheckerTest.cpp
			SimpleRingBufferTests.cpp
	)

	target_link_libraries(${PROJECT_NAME}Tests PRIVATE ArcdpsExtension::extension GTest::gtest GTest::gtest_main)
	IF (WIN32)
		target_link_libraries(${PROJECT_NAME}Tests PRIVATE Version.lib)
	ELSE()
	ENDIF()

	gtest_discover_tests(${PROJECT_NAME}Tests)
ENDIF()
