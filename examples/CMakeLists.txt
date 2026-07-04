# サンプルバイナリは build/examples/ 以下
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples)

# Output system example (Logger + DataRecorder)
add_executable(example_output
    example_output.cpp
)
target_link_libraries(example_output PRIVATE
    acta::output
)
