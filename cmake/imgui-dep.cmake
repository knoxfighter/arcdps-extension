FIND_PACKAGE(imgui CONFIG REQUIRED)

TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PUBLIC IMGUI_DEFINE_MATH_OPERATORS)

TARGET_SOURCES(${PROJECT_NAME} PUBLIC
		FILE_SET HEADERS
		FILES
		ArcdpsExtension.h
		ImGui_Math.h
		imgui_stdlib.h
		UpdateChecker.h
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
		imgui_stdlib.cpp
		UpdateChecker.cpp
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

TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC imgui::imgui)
