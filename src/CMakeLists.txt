# acta::logging — 診断ログ（実装は spdlog に依存、公開ヘッダーには現れない）
add_library(acta_logging
    logging/logger_factory.cpp
)
add_library(acta::logging ALIAS acta_logging)
target_include_directories(acta_logging
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_link_libraries(acta_logging
    PRIVATE spdlog::spdlog
    PUBLIC fmt::fmt
)
set_target_properties(acta_logging PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME logging
)

# acta::recording — 解析データ記録（実装は spdlog に依存、公開ヘッダーには現れない）
add_library(acta_recording
    recording/recorder_factory.cpp
)
add_library(acta::recording ALIAS acta_recording)
target_include_directories(acta_recording
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_link_libraries(acta_recording
    PRIVATE spdlog::spdlog
    PUBLIC fmt::fmt
)
set_target_properties(acta_recording PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME recording
)

# acta::output — logging + recording を束ねる DI コンテナ（header-only）
add_library(acta_output INTERFACE)
add_library(acta::output ALIAS acta_output)
target_include_directories(acta_output
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_link_libraries(acta_output INTERFACE acta::logging acta::recording)
set_target_properties(acta_output PROPERTIES EXPORT_NAME output)
