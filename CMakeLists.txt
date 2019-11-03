cmake_minimum_required(VERSION 3.8)

project(KHTcpC)

set(lib_name "khtcpc")
set(lib_name_preload ${lib_name}-preload)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)

file(GLOB proj_headers "include/*.h")
file(GLOB proj_sources "src/*.cc")
file(GLOB preload_sources "preload/*.cc")

find_package(Boost 1.69.0 REQUIRED COMPONENTS log)
include_directories("include/" ${Boost_INCLUDE_DIRS} ${PCAP_INCLUDE_DIRS})
link_libraries(${Boost_LIBRARIES} ${PCAP_LIBRARIES})
add_definitions(-DBOOST_LOG_DYN_LINK)

add_library(${lib_name} SHARED ${proj_headers} ${proj_sources})
add_library(${lib_name_preload} SHARED ${preload_sources})
target_link_libraries(${lib_name_preload} ${lib_name})

add_subdirectory(tests)
