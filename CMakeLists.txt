cmake_minimum_required(VERSION 3.5)

project(PutToSleep LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR})

set(HEADERS )

set(SOURCES
	main.cpp
)

add_executable(PutToSleep ${HEADERS} ${SOURCES})

install(TARGETS PutToSleep LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
