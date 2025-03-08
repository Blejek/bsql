cmake_minimum_required(VERSION 3.28.3)

set(PROJ_LIB_NAME BSQL_lib)
set(PROJ_EXE_NAME BSQL)

set(CMAKE_C_STANDARD 23)
project(${PROJ_LIB_NAME} VERSION 1.0 DESCRIPTION "BSQL lib" LANGUAGES C)
project(${PROJ_EXE_NAME} VERSION 1.0 DESCRIPTION "BSQL lib" LANGUAGES C)

set(LIB_SOURCES
    src/bsql.c
)

set(LIB_INCLUDES
    src/bsql.h
)

add_library(${PROJ_LIB_NAME} STATIC ${LIB_SOURCES} ${LIB_INCLUDES})
target_include_directories(${PROJ_LIB_NAME} PUBLIC ${LIB_INCLUDES})

add_executable(${PROJ_EXE_NAME} src/main.c)
target_link_libraries(${PROJ_EXE_NAME} PRIVATE ${PROJ_LIB_NAME})
