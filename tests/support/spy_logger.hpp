#pragma once

#include <string>
#include <vector>

#include "reclog/logging/logger.hpp"

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
class SpyLogger : public reclog::logging::Logger {
public:
    using Logger::Log;

    void Log(reclog::logging::LogLevel level, std::string_view message) override {
        if (level >= level_) {
            entries_.emplace_back(message);
        }
    }

    void SetLevel(reclog::logging::LogLevel level) override { level_ = level; }

    reclog::logging::LogLevel Level() const override { return level_; }

    /**
     * @brief 蓄積されたログエントリを返す
     */
    const std::vector<std::string> &Entries() const { return entries_; }

    /**
     * @brief 蓄積されたログエントリをクリアする
     */
    void clear() { entries_.clear(); }

private:
    reclog::logging::LogLevel level_ = reclog::logging::LogLevel::kTrace;
    std::vector<std::string> entries_;
};
