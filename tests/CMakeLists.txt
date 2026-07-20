# テストバイナリは build/tests/ 以下
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

# doctest 記述パターンのサンプルテスト
add_executable(test_doctest_usage
    test_doctest_usage.cpp
)
target_include_directories(test_doctest_usage PRIVATE
    ${PROJECT_SOURCE_DIR}/tests
)
target_link_libraries(test_doctest_usage PRIVATE doctest::doctest)
add_test(
    NAME test_doctest_usage
    COMMAND $<TARGET_FILE:test_doctest_usage>
)

# logging (Logger / NullLogger / LoggerFactory) のテスト
add_executable(test_logger
    test_logger.cpp
)
target_include_directories(test_logger PRIVATE
    ${PROJECT_SOURCE_DIR}/tests
)
target_link_libraries(test_logger PRIVATE
    reclog::logging
    doctest::doctest
)
add_test(
    NAME test_logger
    COMMAND $<TARGET_FILE:test_logger>
)

# recording::RecorderManager のテスト
add_executable(test_recorder_manager
    test_recorder_manager.cpp
)
target_include_directories(test_recorder_manager PRIVATE
    ${PROJECT_SOURCE_DIR}/tests
)
target_link_libraries(test_recorder_manager PRIVATE
    reclog::recording
    doctest::doctest
)
add_test(
    NAME test_recorder_manager
    COMMAND $<TARGET_FILE:test_recorder_manager>
)

# output::OutputContext のテスト
add_executable(test_output_context
    test_output_context.cpp
)
target_include_directories(test_output_context PRIVATE
    ${PROJECT_SOURCE_DIR}/tests
)
target_link_libraries(test_output_context PRIVATE
    reclog::output
    doctest::doctest
)
add_test(
    NAME test_output_context
    COMMAND $<TARGET_FILE:test_output_context>
)
