cmake_minimum_required (VERSION 3.6)
project (fan_switch CXX)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable (fan_switch src/fan_switch.cpp)

set (SERVICE_FILES ${PROJECT_SOURCE_DIR}/service_files/lenovo-fan.service)

install (FILES ${SERVICE_FILES} DESTINATION /lib/systemd/system/)
install (TARGETS fan_switch DESTINATION bin)
