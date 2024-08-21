set(UNIT_TESTS_CMAKE_TARGET "main")


# ******************************************************************************
# Option that dynamically fetches (downloads) gtest version 1.13.0 into the build
# directory every time. To get later release, update the commit hash to download
# ******************************************************************************
#include(FetchContent)
#FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/b796f7d.zip)
#FetchContent_MakeAvailable(googletest)

# ******************************************************************************
# Alternativelly to fetching it every time, it's possible to install it into 
# the system, and just add the following line, to that it is a required packege.
# ******************************************************************************
find_package(GTest 1.13.0 REQUIRED)

# Enables testing in CMake
enable_testing()

# Define cmake binary taget (in this case, an executable)
add_executable(${UNIT_TESTS_CMAKE_TARGET}
    testBoostDeadlineTimer.cpp
    testThreadSafeQueue.cpp
)

# Make the directory known
target_include_directories(${UNIT_TESTS_CMAKE_TARGET} PUBLIC
    ${CMAKE_SOURCE_DIR}/lib/Infrastructure/BoostDeadlineTimer
)

# Link library to the binary target. GTest::gtest_main offers me a default main() function
target_link_libraries(${UNIT_TESTS_CMAKE_TARGET}
    GTest::gtest_main
    BoostDeadlineTimer
)

# Enable CMake’s test runner to discover the tests included in the binary
include(GoogleTest)
gtest_discover_tests(${UNIT_TESTS_CMAKE_TARGET})