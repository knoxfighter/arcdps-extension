find_package(ArcdpsUnofficialExtras CONFIG REQUIRED)

target_compile_definitions(${PROJECT_NAME} PRIVATE ARCDPS_EXTENSION_UNOFFICIAL_EXTRAS)

target_sources(${PROJECT_NAME} PUBLIC
		FILE_SET HEADERS
		FILES
		KeyBindHandler.h
)

target_sources(${PROJECT_NAME}
		PRIVATE
		KeyBindHandler.cpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC ArcdpsUnofficialExtras)
