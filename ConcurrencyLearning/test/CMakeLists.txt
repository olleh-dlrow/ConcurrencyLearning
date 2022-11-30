# Now simply link against gtest or gtest_main as needed. Eg
# Add source to this project's executable.

set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/ConcurrencyLearning/src")

include_directories(${INCLUDE_DIR})

add_executable (SpinLockMutexTest "SpinLockMutexTest.cpp")
target_link_libraries(SpinLockMutexTest gtest_main)
add_test(NAME SpinLockMutexTest COMMAND SpinLockMutexTest)

add_executable (ThreadSafeQueueTest "ThreadSafeQueueTest.cpp")
target_link_libraries(ThreadSafeQueueTest gtest_main)
add_test(NAME ThreadSafeQueueTest COMMAND ThreadSafeQueueTest)

add_executable (FineGrainedLockQueueTest "FineGrainedLockQueueTest.cpp")
target_link_libraries(FineGrainedLockQueueTest gtest_main)
add_test(NAME FineGrainedLockQueueTest COMMAND FineGrainedLockQueueTest)

add_executable (ThreadSafeHashTableTest "ThreadSafeHashTableTest.cpp")
target_link_libraries(ThreadSafeHashTableTest gtest_main)
add_test(NAME ThreadSafeHashTableTest COMMAND ThreadSafeHashTableTest)

add_executable (ThreadSafeListTest "ThreadSafeListTest.cpp")
target_link_libraries(ThreadSafeListTest gtest_main)
add_test(NAME ThreadSafeListTest COMMAND ThreadSafeListTest)

add_executable (ParallelAlgorithmTest "ParallelAlgorithmTest.cpp")
target_link_libraries(ParallelAlgorithmTest gtest_main)
add_test(NAME ParallelAlgorithmTest COMMAND ParallelAlgorithmTest)

add_executable (ThreadPoolTest "ThreadPoolTest.cpp")
target_link_libraries(ThreadPoolTest gtest_main)
add_test(NAME ThreadPoolTest COMMAND ThreadPoolTest)


if(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    add_executable (InputSystemTest "InputSystemTest.cpp")
endif()