find_package(CURL REQUIRED)

target_compile_definitions(${PROJECT_NAME} PRIVATE ARCDPS_EXTENSION_CURL)

target_sources(${PROJECT_NAME}
		PUBLIC
		FILE_SET HEADERS
		FILES
		SimpleNetworkStack.h
)

target_sources(${PROJECT_NAME}
		PRIVATE
		SimpleNetworkStack.cpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC CURL::libcurl)
