#include "reclog/sink/shared_sink.hpp"

#include <memory>
#include <string>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

namespace reclog::sink {

namespace {
// SharedSink 用ロガーの内部名は利用側から見えないため、
// ファイルパス衝突時の spdlog レジストリ重複登録を避けられるよう固定名にしない。
std::shared_ptr<spdlog::logger> MakeSharedLogger(const std::string &file_path) {
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(file_path);
    auto logger = std::make_shared<spdlog::logger>(file_path, sink);
    logger->set_pattern("%v");
    logger->set_level(spdlog::level::trace);
    return logger;
}
} // namespace

SharedSink::SharedSink(const std::string &file_path)
    : impl_(MakeSharedLogger(file_path)) {}

SharedSink::~SharedSink() = default;

} // namespace reclog::sink
