# CMakeList.txt : CMake project for new_elvis, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_FLAGS "-std=c++17")
option(USE_POSTGRES "Use Postgresql as database" OFF) #OFF by default
option(USE_CRYPTO "Use CRYPTOCPP" OFF) #OFF by default
option(DEBUG "DEBUG" OFF) #OFF by default
if (DEBUG)
	message("DEBUG is on")
	add_definitions(-DDEBUG)
endif()
if(USE_POSTGRES)
	add_definitions(-DUSE_POSTGRES)
	if (USE_CRYPTO)
		add_definitions(-DUSE_CRYPTO)
		add_library (Elvis "app_context.cpp" "app_context.h" "cache.h" "common_headers.h" "controllers.cpp"
							"controllers.h" "cryptocpp_crypto_manager" "crypto_manager.h" "db_connector.h" "queue.h" "io_context.cpp" "io_context.h"
							"json_utils.cpp" "json_utils.h" "logger.h" "models.h" "request_context.cpp" "request_context.h"
							"response_context.cpp" "response_context.h" "route_manager.h" "utils.cc" "utils.h" "pg_db_connector.h")
		target_link_libraries(Elvis PUBLIC "-lpthread" "-lpqxx" "-lpq" "-lcryptopp")
	else()
		add_library (Elvis "app_context.cpp" "app_context.h" "cache.h" "common_headers.h" "controllers.cpp"
							"controllers.h" "crypto_manager.h" "db_connector.h" "queue.h" "io_context.cpp" "io_context.h"
							"json_utils.cpp" "json_utils.h" "logger.h" "models.h" "request_context.cpp" "request_context.h"
							"response_context.cpp" "response_context.h" "route_manager.h" "utils.cc" "utils.h" "pg_db_connector.h")
	target_link_libraries(Elvis PUBLIC "-lpthread" "-lpqxx" "-lpq")
	endif()
else()
	if (USE_CRYPTO)
		add_definitions(-DUSE_CRYPTO)
		add_library (Elvis "app_context.cpp" "app_context.h" "cache.h" "common_headers.h" "controllers.cpp"
		"controllers.h" "cryptocpp_crypto_manager" "crypto_manager.h" "db_connector.h" "queue.h" "io_context.cpp" "io_context.h"
		"json_utils.cpp" "json_utils.h" "logger.h" "models.h" "request_context.cpp" "request_context.h"
		"response_context.cpp" "response_context.h" "route_manager.h" "utils.cc" "utils.h")
		target_link_libraries(Elvis PUBLIC "-lpthread" "-lcryptopp")
	else()
		add_library (Elvis "app_context.cpp" "app_context.h" "cache.h" "common_headers.h" "controllers.cpp"
		"controllers.h" "crypto_manager.h" "db_connector.h" "queue.h" "io_context.cpp" "io_context.h"
		"json_utils.cpp" "json_utils.h" "logger.h" "models.h" "request_context.cpp" "request_context.h"
		"response_context.cpp" "response_context.h" "route_manager.h" "utils.cc" "utils.h")
		target_link_libraries(Elvis PUBLIC "-lpthread")
	endif()
endif()
unset(USE_POSTGRES CACHE)
unset(USE_CRYPTO CACHE)
unset(DEBUG CACHE)
# TODO: Add tests and install targets if needed.
