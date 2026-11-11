cmake_minimum_required(VERSION 3.21)

project(StarForge
    VERSION 0.1.0
    DESCRIPTION "Offline-first 2D space sandbox RPG"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(STARFORGE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(STARFORGE_FETCH_SDL3 "Fetch SDL3 when it is not provided by the toolchain" ON)

if(STARFORGE_FETCH_SDL3)
    include(FetchContent)
    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-3.2.10
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(SDL3)
else()
    find_package(SDL3 CONFIG REQUIRED)
endif()

add_executable(star_forge
    src/main.c
)

target_link_libraries(star_forge PRIVATE SDL3::SDL3)

if(MSVC)
    target_compile_options(star_forge PRIVATE /W4)
    if(STARFORGE_WARNINGS_AS_ERRORS)
        target_compile_options(star_forge PRIVATE /WX)
    endif()
else()
    target_compile_options(star_forge PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
        -Wstrict-prototypes
    )
    if(STARFORGE_WARNINGS_AS_ERRORS)
        target_compile_options(star_forge PRIVATE -Werror)
    endif()
endif()

# SDL's CMake package handles platform-specific linkage and deployment.
