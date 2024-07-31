cmake_minimum_required(VERSION 3.0.0)
project(TauIR VERSION 0.1.1 LANGUAGES CXX C)

add_subdirectory(TauIRLib)
add_subdirectory(TauIRDebug)
add_subdirectory(TauIRTest)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
