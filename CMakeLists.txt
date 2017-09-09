cmake_minimum_required(VERSION 3.8)
cmake_policy(VERSION 3.8)

#
# I have used the hunter package manager here so that boost is automatically downloaded and used if not already
# available.
# If you don't want to use Hunter (I strongly recommend that you do) then simply define the variable HUNTER_DISABLED
option(USE_HUNTER_FOR_DEPENDENCIES "Use the hunter package manager to find boost" ON)
option(IMPL_PTR_BUILD_TESTS "build the tests" ON)

if (USE_HUNTER_FOR_DEPENDENCIES)
    include(cmake/HunterGate.cmake)
    HunterGate(
            URL "https://github.com/ruslo/hunter/archive/v0.19.96.tar.gz"
            SHA1 "8316d0491ee03a918d2e0973aaf5878d9bcfe472"
    )
endif ()

project(ImplPtr)
set(CMAKE_CXX_STANDARD 14)

if (USE_HUNTER_FOR_DEPENDENCIES)
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
    if (USE_HUNTER_FOR_DEPENDENCIES)
        sugar_include(test)
    else ()
        find_package(Boost REQUIRED COMPONENTS test)
        add_subdirectory(test)
    endif ()

    add_executable(impl_ptr_tests ${TEST_SOURCES})
    target_link_libraries(impl_ptr_tests PRIVATE impl_ptr)
endif ()
