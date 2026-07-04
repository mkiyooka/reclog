#pragma once

#include "acta/logging/logger.hpp"

namespace acta::logging {

/**
 * @brief 何も出力しないロガー
 *
 * ロギングを無効化したい場合や、DI先のデフォルト実装として使用する。
 */
class NullLogger : public Logger {
public:
    using Logger::Log;

    void Log(LogLevel /*level*/, std::string_view /*msg*/) override {}
    void SetLevel(LogLevel /*level*/) override {}
    LogLevel Level() const override { return LogLevel::kOff; }
};

} // namespace acta::logging
