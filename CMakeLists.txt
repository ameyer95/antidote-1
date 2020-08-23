cmake_minimum_required(VERSION 3.10)
project(antidote LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

file(GLOB HEADERS
    include/*.hpp
    include/*.h
)

file(GLOB SOURCES
    src/*.cpp
)

list(FILTER SOURCES EXCLUDE REGEX "main.cpp$")

set(MAIN 
    src/main.cpp
)

set(CATCH2
    test/catch2/catch.hpp
)

file(GLOB TEST_SOURCES
    test/src/*.cpp
)

# used to set common build flags

add_library(common INTERFACE)

add_executable(antidote ${HEADERS} ${SOURCES} ${MAIN})
add_executable(test ${HEADERS} ${SOURCES} ${CATCH2} ${TEST_SOURCES})

target_include_directories(common INTERFACE include/)

target_include_directories(test PUBLIC test/catch2/)

target_link_libraries(antidote common)
target_link_libraries(test common)

# build flags

if(MSVC)
    # target_compile_options(common INTERFACE /O2) # equivalent to -O3 
    target_compile_options(common INTERFACE /GL) # equivalent to -flto
    target_compile_options(common INTERFACE /W4) 

    set_property(TARGET antidote PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
    set_property(TARGET test PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
else()
    target_compile_options(common INTERFACE -O3 -Wall -flto)
endif()

# copy data folders 

add_custom_command(TARGET antidote PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/data/ $<TARGET_FILE_DIR:antidote>/data)

add_custom_command(TARGET test PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/data/ $<TARGET_FILE_DIR:test>/data)

add_custom_command(TARGET test PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/test/data/ $<TARGET_FILE_DIR:test>/test/data)
