# -*- mode: cmake; cmake-tab-width: 4 -*-

# Just reduce this version if needed and send me a patch if it
# works. This is just the version I have.
cmake_minimum_required(VERSION 3.16)
project("Camera autofocus based on gcode" C)

# the `pkg_check_modules` function is created with this call
find_package(PkgConfig REQUIRED) 

# these calls create special `PkgConfig::<MODULE>` variables
pkg_check_modules(GLIB REQUIRED IMPORTED_TARGET glib-2.0)

add_compile_options(
    -std=gnu17 # C17 standard
    -O2 # optimize
    -Wall # enable warnings
)

file(GLOB SRC_FILES "src/*.c") # Load all files in src folder

add_executable(gcfocus ${SRC_FILES})

target_link_libraries(gcfocus PUBLIC
    PkgConfig::GLIB
    m # math
)
