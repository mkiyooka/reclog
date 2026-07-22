# reclog::sink — 複数の Logger/DataRecorder が書き込み先を共有するための SharedSink
# （実装は spdlog に依存、公開ヘッダーには現れない）。
# logging/recording の両方から参照されるため、両者より依存グラフの下流に置く。
add_library(reclog_sink
    sink/shared_sink.cpp
)
add_library(reclog::sink ALIAS reclog_sink)
target_include_directories(reclog_sink
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_link_libraries(reclog_sink
    PRIVATE spdlog::spdlog
)
set_target_properties(reclog_sink PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME sink
)

# reclog::logging — 診断ログ（実装は spdlog に依存、公開ヘッダーには現れない）
add_library(reclog_logging
    logging/logger_factory.cpp
)
add_library(reclog::logging ALIAS reclog_logging)
target_include_directories(reclog_logging
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_link_libraries(reclog_logging
    PRIVATE spdlog::spdlog
    PUBLIC reclog::sink
    PUBLIC fmt::fmt
)
set_target_properties(reclog_logging PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME logging
)

# reclog::recording — 解析データ記録（実装は spdlog に依存、公開ヘッダーには現れない）
add_library(reclog_recording
    recording/recorder_factory.cpp
)
add_library(reclog::recording ALIAS reclog_recording)
target_include_directories(reclog_recording
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_link_libraries(reclog_recording
    PRIVATE spdlog::spdlog
    PUBLIC reclog::sink
    PUBLIC fmt::fmt
)
set_target_properties(reclog_recording PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME recording
)

# reclog::output — logging + recording を束ねる DI コンテナ（header-only）
add_library(reclog_output INTERFACE)
add_library(reclog::output ALIAS reclog_output)
target_include_directories(reclog_output
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_link_libraries(reclog_output INTERFACE reclog::logging reclog::recording)
set_target_properties(reclog_output PROPERTIES EXPORT_NAME output)
