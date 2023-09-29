cmake_minimum_required(VERSION 3.10)
project(lai C)

# this CMakeLists.txt allows one to either build LAI as a library
# or, by setting BUILD_LAI_LIBRARY to OFF, to include the sources
# and include directory in another project

# Set custom flags if needed
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffreestanding -mcmodel=kernel -mno-red-zone")

set(LAI_SOURCES
    core/error.c
    core/eval.c
    core/exec.c
    core/exec-operand.c
    core/libc.c
    core/ns.c
    core/object.c
    core/opregion.c
    core/os_methods.c
    core/variable.c
    core/vsnprintf.c
    helpers/pc-bios.c
    helpers/pci.c
    helpers/resource.c
    helpers/sci.c
    helpers/pm.c
    drivers/ec.c
    drivers/timer.c
    CACHE INTERNAL "LAI sources"
)

set(LAI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "LAI include directory")
set(LAI_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "LAI source directory")

option(BUILD_LAI_LIBRARY "Build LAI as a library" ON)

if (BUILD_LAI_LIBRARY)
    target_include_directories(lai PUBLIC include)
    add_library(lai STATIC ${LAI_SOURCES})
    set_target_properties(lai PROPERTIES POSITION_INDEPENDENT_CODE OFF)
endif()