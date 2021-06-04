# CMakeList.txt : CMake project for new_elvis, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
#find_package(CryptoPP REQUIRED)

set(CMAKE_CXX_FLAGS "-std=c++17")
#set(LDFLAGS "-lpthread -lpqxx -lpq -lcryptopp")
# Add source to this project's executable.
add_library (Elvis "app_context.cc" "app_context.h" "cache.h" "common_headers.h" "controllers.cc"
					"controllers.h" "db_connector.h" "event_queue.h" "io_context.cc" "io_context.h"
					"json_utils.cc" "json_utils.h" "logger.h" "models.h" "request_context.cc" "request_context.h"
					"response_context.cc" "response_context.h" "route_manager.h" "utils.cc" "utils.h")
target_link_libraries(Elvis PUBLIC "-lpthread" "-lpqxx" "-lpq" "-lcryptopp")
# TODO: Add tests and install targets if needed.
