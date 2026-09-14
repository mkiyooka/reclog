# ベンチマークバイナリは build/benches/ 以下
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/benches)

# ベンチマーク記述パターンのサンプル
add_executable(bench_template_library_cpp
    bench_template_library_cpp.cpp
)
target_link_libraries(bench_template_library_cpp PRIVATE
    nanobench::nanobench
    PRIVATE sanitizer_flags
)
