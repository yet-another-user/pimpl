cmake_minimum_required(VERSION 3.8)
cmake_policy(VERSION 3.8)

###############################################
# NOTE FOR THOSE UNFORTUNATES WHO DON'T KNOW OF HUNTER
#
# Hunter is a dependency manager for CMake. It automatically downloads, configures and compiles
# libraries that your program depends on.
# Hunter is clever enough to understand the toolchain file you have specified on the command line, and
# build dependencies (including quite demanding ones like boost, openssl etc) with the correct command line arguments.
# Tt then caches the build by target, toolchain and configuration so that you never have to build that
# configuration again.
# see https://github.com/ruslo/hunter
#
# If you don't want to use Hunter then simply define the variable HUNTER_ENABLED=OFF or USE_HUNTER_FOR_DEPENDENCIES=OFF
# on the CMAKE command line
if (DEFINED HUNTER_ENABLED)
    option(USE_HUNTER_FOR_DEPENDENCIES "Use the hunter package manager to find boost" ${HUNTER_ENABLED})
else ()
    option(USE_HUNTER_FOR_DEPENDENCIES "Use the hunter package manager to find boost" ON)
endif ()

option(IMPL_PTR_BUILD_TESTS "build the tests" ON)

if (USE_HUNTER_FOR_DEPENDENCIES)
    include(cmake/HunterGate.cmake)
    HunterGate(
            URL "https://github.com/ruslo/hunter/archive/v0.19.96.tar.gz"
            SHA1 "8316d0491ee03a918d2e0973aaf5878d9bcfe472"
    )
endif ()
#
# at this point, HUNTER_ENABLED is guaranteed to be set to ON or OFF
#

project(ImplPtr)
set(CMAKE_CXX_STANDARD 14)

if (HUNTER_ENABLED)
    hunter_add_package(Sugar)
    include(${SUGAR_ROOT}/cmake/Sugar)
    include(sugar_files)
    include(sugar_include)
    hunter_add_package(Boost)
    find_package(Boost CONFIG REQUIRED)
else ()
    find_package(Boost REQUIRED)
endif ()


add_library(impl_ptr INTERFACE)
target_include_directories(impl_ptr SYSTEM INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_link_libraries(impl_ptr INTERFACE Boost::boost)


if (IMPL_PTR_BUILD_TESTS)
    if (HUNTER_ENABLED)
        sugar_include(test)
    else ()
        find_package(Boost REQUIRED)
        add_subdirectory(test)
    endif ()

    add_executable(impl_ptr_tests ${TEST_SOURCES})
    target_link_libraries(impl_ptr_tests PRIVATE impl_ptr)
endif ()
