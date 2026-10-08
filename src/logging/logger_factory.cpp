#include "reclog/logging/logger_factory.hpp"

#include <memory>
#include <string>
#include <utility>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "reclog/logging/logger.hpp"
#include "reclog/logging/null_logger.hpp"
#include "reclog/sink/shared_sink.hpp"

namespace reclog::logging {

namespace {

spdlog::level::level_enum ToSpdlogLevel(LogLevel lvl) {
    switch (lvl) {
        case LogLevel::kTrace:
            return spdlog::level::trace;
        case LogLevel::kDebug:
            return spdlog::level::debug;
        case LogLevel::kInfo:
            return spdlog::level::info;
        case LogLevel::kWarn:
            return spdlog::level::warn;
        case LogLevel::kError:
            return spdlog::level::err;
        case LogLevel::kCritical:
            return spdlog::level::critical;
        default:
            return spdlog::level::off;
    }
}

/**
 * @brief spdlog を使ったロガー実装
 *
 * spdlog::logger を内包し、Logger インターフェース経由でメッセージを出力する。
 * 公開ヘッダーには一切現れない、この翻訳単位に閉じた実装クラス。
 */
class SpdlogLoggerImpl final : public Logger {
public:
    using Logger::Log;

    explicit SpdlogLoggerImpl(std::shared_ptr<spdlog::logger> logger)
        : logger_(std::move(logger)) {}

    void Log(LogLevel lvl, std::string_view msg) override { logger_->log(ToSpdlogLevel(lvl), msg); }

    void SetLevel(LogLevel lvl) override {
        level_ = lvl;
        logger_->set_level(ToSpdlogLevel(lvl));
    }

    LogLevel Level() const override { return level_; }

private:
    std::shared_ptr<spdlog::logger> logger_;
    LogLevel level_ = LogLevel::kTrace;
};

/**
 * @brief SharedSink 経由で "[LOG] " マーカー付きに書き込むロガー実装
 *
 * SharedSink が保持する spdlog::logger は複数の Logger / DataRecorder で共有される
 * ため、そのレベルを直接操作すると他の共有先の出力を意図せず変えてしまう。
 * そのためレベル判定はこのクラス自身が保持する level_ で行い、
 * 共有 spdlog::logger 自体は常に info レベル固定で使う（実際の抑制は Log() 側で行う）。
 */
class SharedLoggerImpl final : public Logger {
public:
    using Logger::Log;

    SharedLoggerImpl(std::shared_ptr<spdlog::logger> logger, LogLevel level)
        : logger_(std::move(logger)),
          level_(level) {}

    void Log(LogLevel lvl, std::string_view msg) override {
        if (lvl >= level_) {
            logger_->info("[LOG] {}", msg);
        }
    }

    void SetLevel(LogLevel lvl) override { level_ = lvl; }

    LogLevel Level() const override { return level_; }

private:
    std::shared_ptr<spdlog::logger> logger_;
    LogLevel level_;
};

} // namespace

std::unique_ptr<Logger>
LoggerFactory::MakeConsole(const std::string &name, LogLevel level, const std::string &pattern) {
    auto inner = spdlog::stdout_color_mt(name);
    if (!pattern.empty()) {
        inner->set_pattern(pattern);
    }
    auto logger = std::make_unique<SpdlogLoggerImpl>(inner);
    logger->SetLevel(level);
    return logger;
}

std::unique_ptr<Logger> LoggerFactory::MakeFile(
    const std::string &name, const std::string &file_path, LogLevel level, const std::string &pattern
) {
    auto inner = spdlog::basic_logger_mt(name, file_path);
    if (!pattern.empty()) {
        inner->set_pattern(pattern);
    }
    auto logger = std::make_unique<SpdlogLoggerImpl>(inner);
    logger->SetLevel(level);
    return logger;
}

std::unique_ptr<Logger> LoggerFactory::MakeNull() { return std::make_unique<NullLogger>(); }

std::unique_ptr<Logger> LoggerFactory::MakeShared(const reclog::sink::SharedSink &sink, LogLevel level) {
    auto inner = std::static_pointer_cast<spdlog::logger>(sink.Impl());
    return std::make_unique<SharedLoggerImpl>(std::move(inner), level);
}

} // namespace reclog::logging
