cmake_minimum_required(VERSION 3.1.0)
project(CLTuneDemosProject)

set(CLTUNE_DIR ${CMAKE_SOURCE_DIR}/cltune)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/Modules/)

find_Package(OpenCL REQUIRED)
set(FRAMEWORK_INCLUDE_DIRS ${OPENCL_INCLUDE_DIRS} ${CLTUNE_DIR})
set(FRAMEWORK_LIBRARY_DIRS ${CLTUNE_DIR})
set(FRAMEWORK_LIBRARIES ${OPENCL_LIBRARIES} cltune)

include_directories(${FRAMEWORK_INCLUDE_DIRS})
link_directories(${FRAMEWORK_LIBRARY_DIRS})


add_executable(MedianFilter
    median_filter/src/main.cpp)

set_property(TARGET MedianFilter PROPERTY CXX_STANDARD 11)
set_property(TARGET MedianFilter PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(MedianFilter ${FRAMEWORK_LIBRARIES})

add_custom_command(TARGET MedianFilter PRE_BUILD
					COMMAND ${CMAKE_COMMAND} -E
                         make_directory $<TARGET_FILE_DIR:MedianFilter>/median_filter/src/)
add_custom_command(TARGET MedianFilter PRE_BUILD
					COMMAND ${CMAKE_COMMAND} -E
                         copy ${CMAKE_SOURCE_DIR}/median_filter/src/medfilt.cl $<TARGET_FILE_DIR:MedianFilter>/median_filter/src/)
add_custom_command(TARGET MedianFilter PRE_BUILD
					COMMAND ${CMAKE_COMMAND} -E
                         copy ${CMAKE_SOURCE_DIR}/median_filter/src/medfilt_baseline.cl $<TARGET_FILE_DIR:MedianFilter>/median_filter/src/)