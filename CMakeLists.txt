cmake_minimum_required(VERSION 3.13)
project(lab6)
list(APPEND CMAKE_MODULE_PATH "$ENV{WORK}/cmake")
include(CsoiWin)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
add_definitions(-D_UNICODE -DUNICODE)

aux_source_directory(. SRCS)
add_executable(${PROJECT_NAME} WIN32
        ${SRCS} ${PROJECT_NAME}.rc)
