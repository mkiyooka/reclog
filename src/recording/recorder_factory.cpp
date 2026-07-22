#include "reclog/recording/recorder_factory.hpp"

#include <memory>
#include <string>
#include <utility>

#include <fmt/format.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "reclog/recording/data_recorder.hpp"
#include "reclog/recording/null_recorder.hpp"
#include "reclog/sink/shared_sink.hpp"

namespace reclog::recording {

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

/**
 * @brief SharedSink 経由で "[REC:marker] " マーカー付きに書き込むレコーダー実装
 *
 * SharedSink が保持する spdlog::logger は複数の Logger / DataRecorder で共有される
 * ため、そのレベルを直接操作すると他の共有先の出力を意図せず変えてしまう。
 * そのため Enable/Disable の状態はこのクラス自身が保持する enabled_ で管理し、
 * 共有 spdlog::logger 自体は常に trace レベル固定で使う。
 */
class SharedRecorderImpl final : public DataRecorder {
public:
    SharedRecorderImpl(std::shared_ptr<spdlog::logger> logger, std::string prefix)
        : logger_(std::move(logger)),
          prefix_(std::move(prefix)) {}

    void Enable() override { enabled_ = true; }
    void Disable() override { enabled_ = false; }
    bool IsEnabled() const override { return enabled_; }

    void Output(std::string_view msg) override {
        if (enabled_) {
            logger_->info("{}{}", prefix_, msg);
        }
    }

    void Flush() override { logger_->flush(); }

private:
    std::shared_ptr<spdlog::logger> logger_;
    std::string prefix_;
    bool enabled_ = false;
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

std::unique_ptr<DataRecorder> RecorderFactory::MakeShared(reclog::sink::SharedSink &sink, const std::string &marker) {
    auto inner = std::static_pointer_cast<spdlog::logger>(sink.Impl());
    return std::make_unique<SharedRecorderImpl>(std::move(inner), fmt::format("[REC:{}] ", marker));
}

} // namespace reclog::recording
