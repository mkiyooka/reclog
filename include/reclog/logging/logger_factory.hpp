#pragma once

#include <memory>
#include <string>

#include "reclog/logging/logger.hpp"

namespace reclog::logging {

/**
 * @brief Logger インスタンスを生成するファクトリ
 *
 * 出力先・同期/非同期の選択を一箇所に集約し、呼び出し側が
 * spdlog 等の実装詳細を知らなくてもよくする。実装（.cpp）側でのみ
 * spdlog に依存するため、公開ヘッダーに spdlog の型は一切現れない。
 *
 * @code
 * auto logger = reclog::logging::LoggerFactory::MakeConsole("app", reclog::logging::LogLevel::kInfo);
 * @endcode
 */
class LoggerFactory {
public:
    /**
     * @brief 標準出力（カラー付き）に書き込む同期ロガーを生成する
     * @param name    内部名（重複不可）
     * @param level   初期ログレベル
     * @param pattern ログ書式パターン文字列（空文字列の場合は実装デフォルト）
     */
    static std::unique_ptr<Logger>
    MakeConsole(const std::string &name, LogLevel level = LogLevel::kInfo, const std::string &pattern = "");

    /**
     * @brief ファイルに書き込む同期ロガーを生成する
     * @param name      内部名（重複不可）
     * @param file_path 出力ファイルパス
     * @param level     初期ログレベル
     * @param pattern   ログ書式パターン文字列（空文字列の場合は実装デフォルト）
     */
    static std::unique_ptr<Logger> MakeFile(
        const std::string &name, const std::string &file_path, LogLevel level = LogLevel::kInfo,
        const std::string &pattern = ""
    );

    /**
     * @brief 何も出力しないロガーを生成する（テスト・無効化用）
     */
    static std::unique_ptr<Logger> MakeNull();
};

} // namespace reclog::logging
