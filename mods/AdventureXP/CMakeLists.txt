cmake_minimum_required(VERSION 3.25)

project(
    AdventureXP
    VERSION 4.0.0
    DESCRIPTION "Clean-room SKSE plugin: quest / explore / dungeon-clear player leveling"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ADVENTUREXP_BUILD_PLUGIN "Build the SKSE plugin DLL (Windows / CommonLibSSE-NG)" ${WIN32})
option(ADVENTUREXP_HOST_TESTS "Build host-side unit tests that do not need Skyrim" ON)

include(CTest)

# Shared, engine-free math used by the plugin and by host tests.
add_library(adventurexp_math INTERFACE)
target_include_directories(adventurexp_math INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(ADVENTUREXP_HOST_TESTS)
    add_executable(adventurexp_math_tests tests/test_xp_math.cpp)
    target_link_libraries(adventurexp_math_tests PRIVATE adventurexp_math)
    add_test(NAME adventurexp_math_tests COMMAND adventurexp_math_tests)

    add_executable(adventurexp_preset_tests tests/test_presets.cpp)
    target_link_libraries(adventurexp_preset_tests PRIVATE adventurexp_math)
    add_test(NAME adventurexp_preset_tests COMMAND adventurexp_preset_tests)

    add_executable(adventurexp_type_tests tests/test_types.cpp)
    target_link_libraries(adventurexp_type_tests PRIVATE adventurexp_math)
    add_test(NAME adventurexp_type_tests COMMAND adventurexp_type_tests)
endif()

if(NOT ADVENTUREXP_BUILD_PLUGIN)
    message(STATUS "ADVENTUREXP_BUILD_PLUGIN=OFF — skipping AdventureXP.dll (use Windows MSVC + vcpkg).")
    return()
endif()

if(NOT WIN32)
    message(FATAL_ERROR "AdventureXP.dll requires a Windows MSVC (or clang-cl) toolchain. See README.md.")
endif()

set(ENABLE_SKYRIM_SE ON CACHE BOOL "CommonLibSSE-NG: SE runtime" FORCE)
set(ENABLE_SKYRIM_AE ON CACHE BOOL "CommonLibSSE-NG: AE runtime (1.6 / 1.7.x)" FORCE)
set(ENABLE_SKYRIM_VR OFF CACHE BOOL "CommonLibSSE-NG: VR runtime" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "CommonLibSSE-NG tests" FORCE)

# Factory pin: alandtse/CommonLibVR ng @ d13d10a0 == CommonLibSSE-NG 8.0.1
# and is the SHA Get-CommonLib refuses to build without Format::SSEv5.
# Do not take a vcpkg/config CommonLib (those pins are often pre-format-5).
# Do not use a shallow v8.0.1 tag clone — pin the exact SSEv5 commit.
set(ADVENTUREXP_COMMONLIB_GIT https://github.com/alandtse/CommonLibVR.git)
set(ADVENTUREXP_COMMONLIB_SHA d13d10a0ccb4945870eb841bf1ad8a6cf5ed84dd)

include(FetchContent)
FetchContent_Declare(
    CommonLibSSE
    GIT_REPOSITORY ${ADVENTUREXP_COMMONLIB_GIT}
    GIT_TAG ${ADVENTUREXP_COMMONLIB_SHA}
    GIT_SHALLOW FALSE
)
FetchContent_MakeAvailable(CommonLibSSE)

if(NOT EXISTS "${commonlibsse_SOURCE_DIR}/include/REL/IDDB.h")
    message(FATAL_ERROR "CommonLib FetchContent did not produce include/REL/IDDB.h")
endif()
file(READ "${commonlibsse_SOURCE_DIR}/include/REL/IDDB.h" ADVENTUREXP_IDDB_H)
if(NOT ADVENTUREXP_IDDB_H MATCHES "SSEv5")
    message(FATAL_ERROR
        "CommonLib at ${ADVENTUREXP_COMMONLIB_SHA} has no Format::SSEv5 — "
        "refusing to build a DLL that cannot read Address Library format 5")
endif()
message(STATUS "CommonLibSSE-NG at ${ADVENTUREXP_COMMONLIB_SHA} (Format::SSEv5 present)")

list(APPEND CMAKE_MODULE_PATH "${commonlibsse_SOURCE_DIR}/cmake")
include(CommonLibSSE)

set(ADVENTUREXP_SOURCES
    src/Plugin.cpp
    src/Config.cpp
    src/PlayerXp.cpp
    src/Events.cpp
    src/SettingsHook.cpp
    src/Serialization.cpp
    src/Messages.cpp
    src/Globals.cpp
    src/Papyrus.cpp
)

add_commonlibsse_plugin(
    ${PROJECT_NAME}
    NAME "AdventureXP"
    AUTHOR "Jo"
    EMAIL ""
    VERSION ${PROJECT_VERSION}
    MINIMUM_SKSE_VERSION 2.3.1
    USE_ADDRESS_LIBRARY
    SOURCES ${ADVENTUREXP_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(${PROJECT_NAME} PRIVATE adventurexp_math)
target_precompile_headers(${PROJECT_NAME} PRIVATE include/AdventureXP/PCH.h)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

# __AdventureXPPlugin.cpp (from add_commonlibsse_plugin) uses "sv" literals
# and is not guaranteed to pick up the PCH on MSVC.
if(MSVC)
    target_compile_options(
        ${PROJECT_NAME}
        PRIVATE
        "/FI${CMAKE_CURRENT_SOURCE_DIR}/include/AdventureXP/PluginLiterals.h"
    )
endif()

find_path(SIMPLEINI_INCLUDE_DIR NAMES SimpleIni.h)
if(SIMPLEINI_INCLUDE_DIR)
    target_include_directories(${PROJECT_NAME} PRIVATE "${SIMPLEINI_INCLUDE_DIR}")
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /permissive- /Zc:preprocessor /W4 /WX-)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Drop the DLL next to the packaged Data layout when SKYRIM_MODS_FOLDER is set.
if(DEFINED ENV{SKYRIM_MODS_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_MODS_FOLDER}")
    set(ADVENTUREXP_OUT "$ENV{SKYRIM_MODS_FOLDER}/AdventureXP/SKSE/Plugins")
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${ADVENTUREXP_OUT}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${PROJECT_NAME}>"
            "${ADVENTUREXP_OUT}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
        VERBATIM
    )
endif()
