#pragma once

#include <string>
#include <vector>

#include "acta/logging/logger.hpp"

/**
 * @brief テスト・ベンチマーク用スパイロガー
 *
 * ログエントリをメモリに蓄積し、テストコードから検証できる。
 *
 * @code
 * SpyLogger logger;
 * process(logger);
 * assert(logger.Entries().size() == 3);
 * @endcode
 */
class SpyLogger : public acta::logging::Logger {
public:
    using Logger::Log;

    void Log(acta::logging::LogLevel level, std::string_view message) override {
        if (level >= level_) {
            entries_.emplace_back(message);
        }
    }

    void SetLevel(acta::logging::LogLevel level) override { level_ = level; }

    acta::logging::LogLevel Level() const override { return level_; }

    /**
     * @brief 蓄積されたログエントリを返す
     */
    const std::vector<std::string> &Entries() const { return entries_; }

    /**
     * @brief 蓄積されたログエントリをクリアする
     */
    void clear() { entries_.clear(); }

private:
    acta::logging::LogLevel level_ = acta::logging::LogLevel::kTrace;
    std::vector<std::string> entries_;
};
