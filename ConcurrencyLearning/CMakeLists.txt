# CMakeList.txt : CMake project for ConcurrencyLearning, include source and define
# project specific logic here.
#

add_subdirectory ("thirdparty")
add_subdirectory ("src")

option(BUILD_TEST "Whether or not to build the tests" ON)
if (${BUILD_TEST})
    message(STATUS "Building tests")
    enable_testing()  # Enable testing only works in root scope
    add_subdirectory ("test")
endif ()