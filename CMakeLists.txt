cmake_minimum_required(VERSION 3.21)

project(
    StarForge
    VERSION 0.1.0
    DESCRIPTION "Offline-first 2D space sandbox RPG"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(
    STARFORGE_WARNINGS_AS_ERRORS
    "Treat compiler warnings as errors"
    OFF
)

option(
    STARFORGE_FETCH_SDL3
    "Fetch SDL3 when it is not provided by the toolchain"
    ON
)

option(
    STARFORGE_BUILD_GAME
    "Build the SDL3 game executable"
    ON
)

option(
    STARFORGE_BUILD_TESTS
    "Build simulation tests"
    ON
)

add_library(
    star_forge_sim
    STATIC
    src/game.c
    src/world.c
)

target_include_directories(
    star_forge_sim
    PUBLIC
    src
)

if(NOT MSVC)
    target_link_libraries(star_forge_sim PUBLIC m)
endif()

if(STARFORGE_BUILD_GAME)
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

    add_executable(
        star_forge
        src/main.c
        src/render.c
    )

    target_link_libraries(
        star_forge
        PRIVATE
        star_forge_sim
        SDL3::SDL3
    )
endif()

if(MSVC)
    target_compile_options(star_forge_sim PRIVATE /W4)

    if(TARGET star_forge)
        target_compile_options(star_forge PRIVATE /W4)
    endif()

    if(STARFORGE_WARNINGS_AS_ERRORS)
        target_compile_options(star_forge_sim PRIVATE /WX)

        if(TARGET star_forge)
            target_compile_options(star_forge PRIVATE /WX)
        endif()
    endif()
else()
    set(STARFORGE_WARNING_FLAGS
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
        -Wstrict-prototypes
    )

    target_compile_options(
        star_forge_sim
        PRIVATE
        ${STARFORGE_WARNING_FLAGS}
    )

    if(TARGET star_forge)
        target_compile_options(
            star_forge
            PRIVATE
            ${STARFORGE_WARNING_FLAGS}
        )
    endif()

    if(STARFORGE_WARNINGS_AS_ERRORS)
        target_compile_options(star_forge_sim PRIVATE -Werror)

        if(TARGET star_forge)
            target_compile_options(star_forge PRIVATE -Werror)
        endif()
    endif()
endif()

if(STARFORGE_BUILD_TESTS)
    enable_testing()

    add_executable(
        star_forge_tests
        tests/test_game.c
    )

    target_link_libraries(
        star_forge_tests
        PRIVATE
        star_forge_sim
    )

    add_test(
        NAME star_forge_simulation
        COMMAND star_forge_tests
    )
endif()
