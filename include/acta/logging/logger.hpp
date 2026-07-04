#pragma once

#include <string_view>

#include <fmt/format.h>

#include "acta/logging/log_level.hpp"

namespace acta::logging {

/**
 * @brief ロガーの共通インターフェース
 *
 * DI（依存性注入）により呼び出し側をロガー実装から分離する。
 * spdlog 等の実装詳細は公開ヘッダーに一切露出しない。
 * 実運用では LoggerFactory が返す実装を、テストでは NullLogger を注入する。
 */
class Logger {
public:
    Logger() = default;
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger(Logger &&) = delete;
    Logger &operator=(Logger &&) = delete;

    virtual ~Logger() = default;

    /**
     * @brief メッセージをログに出力する
     * @param level ログレベル
     * @param msg   出力メッセージ
     */
    virtual void Log(LogLevel level, std::string_view msg) = 0;

    /**
     * @brief 最小ログレベルを設定する
     */
    virtual void SetLevel(LogLevel level) = 0;

    /**
     * @brief 現在のログレベルを返す
     */
    virtual LogLevel Level() const = 0;

    /**
     * @brief 指定レベルが出力対象かを返す
     *
     * コストの高い文字列生成を出力が確定した場合だけ行うために使う。
     * @code
     * if (logger.ShouldLog(acta::logging::LogLevel::kDebug)) {
     *     logger.Log(acta::logging::LogLevel::kDebug, expensive_to_string());
     * }
     * @endcode
     */
    bool ShouldLog(LogLevel lvl) const { return lvl >= Level(); }

    /**
     * @brief fmt::format でフォーマットしてから Log() に渡す非仮想ヘルパー
     *
     * フォーマット文字列はコンパイル時にチェックされる。
     * ShouldLog() が false の場合はフォーマット処理自体をスキップする。
     *
     * @code
     * logger.Log(acta::logging::LogLevel::kInfo, "step={}, value={:.6f}", step, value);
     * @endcode
     */
    template <typename... Args>
    void Log(LogLevel level, fmt::format_string<Args...> fmt_str, Args &&...args) {
        if (ShouldLog(level)) {
            Log(level, fmt::format(fmt_str, std::forward<Args>(args)...));
        }
    }
};

} // namespace acta::logging
