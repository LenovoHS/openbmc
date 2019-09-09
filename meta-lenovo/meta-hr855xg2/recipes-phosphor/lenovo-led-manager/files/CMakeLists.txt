cmake_minimum_required (VERSION 3.6)
project (led-manager CXX)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable (led-manager src/led-manager.cpp)


target_include_directories (led-manager PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries (led-manager systemd sdbusplus pthread phosphor_logging ipmid -lstdc++fs)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/include)

install (TARGETS led-manager
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib/static)

find_package (Boost 1.66 REQUIRED)
include_directories (${BOOST_SRC_DIR})

add_definitions (-DBOOST_ERROR_CODE_HEADER_ONLY)
add_definitions (-DBOOST_SYSTEM_NO_DEPRECATED)
add_definitions (-DBOOST_ALL_NO_LIB)
add_definitions (-DBOOST_NO_RTTI)
add_definitions (-DBOOST_NO_TYPEID)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-rtti")

set (
    SERVICE_FILES
    ${PROJECT_SOURCE_DIR}/service_files/led-manager.service
)
install (FILES ${SERVICE_FILES} DESTINATION /lib/systemd/system/)
