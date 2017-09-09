#
# this file is used if the developer has opted not to use Hunter cmake extensions
#

set(TEST_FILES
        allocator.hpp
        impl.cpp
        impl_always_inplace.cpp
        impl_copied.cpp
        impl_inplace.cpp
        impl_poly.cpp
        impl_shared.cpp
        impl_unique.cpp
        main.cpp
        test.hpp
        )

foreach (file ${TEST_FILES})
    list(APPEND TEST_SOURCES "test/${file}")
endforeach ()

set(TEST_SOURCES ${TEST_SOURCES} PARENT_SCOPE)
