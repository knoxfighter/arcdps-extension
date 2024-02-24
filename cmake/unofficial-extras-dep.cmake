FIND_PACKAGE(ArcdpsUnofficialExtras CONFIG REQUIRED)

TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PRIVATE ARCDPS_EXTENSION_UNOFFICIAL_EXTRAS)

TARGET_SOURCES(${PROJECT_NAME} PUBLIC
		FILE_SET HEADERS
		FILES
		KeyBindHandler.h
)

TARGET_SOURCES(${PROJECT_NAME}
		PRIVATE
		KeyBindHandler.cpp
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC ArcdpsUnofficialExtras)
