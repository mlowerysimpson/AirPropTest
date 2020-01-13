cmake_minimum_required(VERSION 2.8)
project(AirPropTest)
# Locate libraries and headers
find_library(wiringPi_LIB wiringPi)
find_package(Threads REQUIRED)

# Include headers
include_directories(${WIRINGPI_INCLUDE_DIRS}) 
include_directories("../RemoteControlTest")

#the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "./*.cpp")
add_executable(AirPropTest ${SOURCES} "../RemoteControlTest/AToD.cpp" "../RemoteControlTest/ShipLog.cpp" "../RemoteControlTest/CommandList.cpp" "../RemoteControlTest/AirProp.cpp" "../RemoteControlTest/BatteryCharge.cpp" "../RemoteControlTest/CommandList.cpp" "../RemoteControlTest/DiagnosticsSensor.cpp" "../RemoteControlTest/Util.cpp" "../RemoteControlTest/filedata.cpp" "../RemoteControlTest/Sensor.cpp" "../RemoteControlTest/HumiditySensor.cpp")

# Link against libraries
target_link_libraries(AirPropTest ${wiringPi_LIB} )
target_link_libraries(AirPropTest ${CMAKE_THREAD_LIBS_INIT})
