cmake_minimum_required(VERSION 3.21)
project(AutoPlantEverything VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CommonLibSSE CONFIG REQUIRED)

# src/main.cpp already exports SKSEPlugin_Version (UsesAddressLibrary / UsesNoStructs).
# add_commonlibsse_plugin() would emit a second SKSEPlugin_Version via SKSEPluginInfo
# and the link would fail. Link the CommonLib target directly.
add_library(${PROJECT_NAME} SHARED src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE CommonLibSSE::CommonLibSSE)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_precompile_headers(${PROJECT_NAME} PRIVATE src/PCH.h)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive- /utf-8)
    target_link_options(${PROJECT_NAME} PRIVATE /DEBUG:FULL)
endif()
