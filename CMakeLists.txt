cmake_minimum_required(VERSION 3.1.0)
project(CLTuneDemosProject)

set(CLTUNE_DIR ${CMAKE_SOURCE_DIR}/cltune)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/Modules/)

find_Package(OpenCL REQUIRED)
set(FRAMEWORK_INCLUDE_DIRS ${OPENCL_INCLUDE_DIRS} ${CLTUNE_DIR})
set(FRAMEWORK_LIBRARY_DIRS ${CLTUNE_DIR})
set(FRAMEWORK_LIBRARIES ${OPENCL_LIBRARIES} cltune)

include_directories(${FRAMEWORK_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/util/include/)
link_directories(${FRAMEWORK_LIBRARY_DIRS})

set(CLTuneDemosUtilitiesSrc
	util/src/CLIArgumentsBase.cpp
	util/include/CLIArgumentsBase.h)

add_library(CLTuneDemosUtilities SHARED ${CLTuneDemosUtilitiesSrc})

set_property(TARGET CLTuneDemosUtilities PROPERTY CXX_STANDARD 11)
set_property(TARGET CLTuneDemosUtilities PROPERTY CXX_STANDARD_REQUIRED ON)

add_executable(MedianFilter
    median_filter/src/main.cpp
    median_filter/src/medfilt.clh
    median_filter/src/medfilt.cl
    median_filter/src/medfilt_baseline.cl)

set_property(TARGET MedianFilter PROPERTY CXX_STANDARD 11)
set_property(TARGET MedianFilter PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(MedianFilter ${FRAMEWORK_LIBRARIES} CLTuneDemosUtilities)

add_custom_target(MedianFilterResources ALL DEPENDS
	"${CMAKE_BINARY_DIR}/median_filter/src/medfilt.clh"
	"${CMAKE_BINARY_DIR}/median_filter/src/medfilt.cl"
	"${CMAKE_BINARY_DIR}/median_filter/src/medfilt_baseline.cl"
	COMMENT "Copy MedianFilter OpenCL kernels.")
add_dependencies(MedianFilter MedianFilterResources)

add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/median_filter/src"
					COMMAND ${CMAKE_COMMAND} -E
                         make_directory ${CMAKE_BINARY_DIR}/median_filter/src/)
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/median_filter/src/medfilt.clh"
					COMMAND ${CMAKE_COMMAND} -E
                         copy_if_different "${CMAKE_SOURCE_DIR}/median_filter/src/medfilt.clh" "${CMAKE_BINARY_DIR}/median_filter/src/"
                   DEPENDS "${CMAKE_SOURCE_DIR}/median_filter/src/medfilt.clh"
                   DEPENDS ${CMAKE_BINARY_DIR}/median_filter/src/)
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/median_filter/src/medfilt.cl"
					COMMAND ${CMAKE_COMMAND} -E
                         copy ${CMAKE_SOURCE_DIR}/median_filter/src/medfilt.cl ${CMAKE_BINARY_DIR}/median_filter/src/
                   DEPENDS "${CMAKE_SOURCE_DIR}/median_filter/src/medfilt.cl"
                   DEPENDS ${CMAKE_BINARY_DIR}/median_filter/src/)
add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/median_filter/src/medfilt_baseline.cl"
					COMMAND ${CMAKE_COMMAND} -E
                         copy ${CMAKE_SOURCE_DIR}/median_filter/src/medfilt_baseline.cl ${CMAKE_BINARY_DIR}/median_filter/src/
                   DEPENDS "${CMAKE_SOURCE_DIR}/median_filter/src/medfilt_baseline.cl"
                   DEPENDS ${CMAKE_BINARY_DIR}/median_filter/src/)

add_executable(ParallelReduction
	ParallelReduction/src/main.cpp)
set_property(TARGET ParallelReduction PROPERTY CXX_STANDARD 11)
set_property(TARGET ParallelReduction PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(ParallelReduction ${FRAMEWORK_LIBRARIES} CLTuneDemosUtilities)
