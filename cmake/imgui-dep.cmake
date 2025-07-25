find_package(imgui CONFIG REQUIRED)

target_compile_definitions(${PROJECT_NAME} PUBLIC IMGUI_DEFINE_MATH_OPERATORS)
target_compile_definitions(${PROJECT_NAME} PRIVATE ARCDPS_EXTENSION_IMGUI)

target_sources(${PROJECT_NAME} PUBLIC
		FILE_SET HEADERS
		FILES
		ImGui_Math.h
		imgui_stdlib.h
		Widgets.h
		Windows/ComponentBase.h
		Windows/Demo/DemoPositioningComponent.h
		Windows/Demo/DemoTable.h
		Windows/Demo/DemoTableWindow.h
		Windows/Demo/DemoWindow.h
		Windows/MainTable.h
		Windows/MainWindow.h
		Windows/PositioningComponent.h
)

target_sources(${PROJECT_NAME}
		PRIVATE
		imgui_stdlib.cpp
		Widgets.cpp
		Windows/Demo/DemoPositioningComponent.cpp
		Windows/Demo/DemoTable.cpp
		Windows/Demo/DemoTableWindow.cpp
		Windows/Demo/DemoWindow.cpp
		Windows/MainWindow.cpp
		Windows/PositioningComponent.cpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC imgui::imgui)
