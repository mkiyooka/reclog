#include "acta/recording/recorder_factory.hpp"

#include <memory>
#include <string>
#include <utility>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "acta/recording/data_recorder.hpp"
#include "acta/recording/null_recorder.hpp"

namespace acta::recording {

namespace {

/**
 * @brief spdlog を使った解析データレコーダー実装
 *
 * spdlog::logger を内包し、DataRecorder インターフェース経由でデータを出力する。
 * ログレベルではなく Enable/Disable で出力制御する。
 * パターンを "%v" にすることでメッセージのみを出力する（タイムスタンプ等なし）。
 * 公開ヘッダーには一切現れない、この翻訳単位に閉じた実装クラス。
 */
class SpdlogRecorderImpl final : public DataRecorder {
public:
    explicit SpdlogRecorderImpl(std::shared_ptr<spdlog::logger> logger)
        : logger_(std::move(logger)) {
        logger_->set_pattern("%v");
        logger_->set_level(spdlog::level::off);
    }

    void Enable() override { logger_->set_level(spdlog::level::info); }

    void Disable() override { logger_->set_level(spdlog::level::off); }

    bool IsEnabled() const override { return logger_->level() != spdlog::level::off; }

    void Output(std::string_view msg) override { logger_->info(msg); }

    void Flush() override { logger_->flush(); }

private:
    std::shared_ptr<spdlog::logger> logger_;
};

} // namespace

std::unique_ptr<DataRecorder> RecorderFactory::MakeFile(const std::string &name, const std::string &file_path) {
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(file_path);
    auto inner = std::make_shared<spdlog::logger>(name, sink);
    return std::make_unique<SpdlogRecorderImpl>(inner);
}

std::unique_ptr<DataRecorder>
RecorderFactory::MakeCsvFile(const std::string &name, const std::string &file_path, const std::string &header) {
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(file_path, /* truncate = */ true);
    auto inner = std::make_shared<spdlog::logger>(name, sink);
    // ヘッダ行をファクトリ内で書き込む（Enable 不要）
    inner->set_pattern("%v");
    inner->set_level(spdlog::level::info);
    inner->info(header);
    inner->flush();
    return std::make_unique<SpdlogRecorderImpl>(inner);
}

std::unique_ptr<DataRecorder>
RecorderFactory::MakeJsonLinesFile(const std::string &name, const std::string &file_path) {
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(file_path);
    auto inner = std::make_shared<spdlog::logger>(name, sink);
    return std::make_unique<SpdlogRecorderImpl>(inner);
}

std::unique_ptr<DataRecorder> RecorderFactory::MakeConsole(const std::string &name) {
    auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto inner = std::make_shared<spdlog::logger>(name, sink);
    return std::make_unique<SpdlogRecorderImpl>(inner);
}

std::unique_ptr<DataRecorder> RecorderFactory::MakeNull() { return std::make_unique<NullRecorder>(); }

} // namespace acta::recording
