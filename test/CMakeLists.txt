option(BUILD_TEST "To enable test" OFF)

if (BUILD_TEST)
    set(sources test_pipe_state_machine.cpp)
    set(test test_${PROJECT_NAME})

    add_executable(${test} ${sources})

    find_package(GTest)

    target_include_directories(${test}
        PRIVATE ${GTEST_INCLUDE_DIR})

    target_link_libraries(${test}
        PRIVATE GTest::gtest GTest::gtest_main)
endif()
