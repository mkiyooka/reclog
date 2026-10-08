#pragma once

#include <memory>
#include <string>

#include "reclog/recording/data_recorder.hpp"

namespace reclog::sink {
class SharedSink;
} // namespace reclog::sink

namespace reclog::recording {

/**
 * @brief DataRecorder インスタンスを生成するファクトリ
 *
 * 出力先・フォーマットの選択を一箇所に集約し、呼び出し側が
 * spdlog 等の実装詳細を知らなくてもよくする。実装（.cpp）側でのみ
 * spdlog に依存するため、公開ヘッダーに spdlog の型は一切現れない。
 *
 * @code
 * // CSV: ヘッダ行を自動出力し、以降は Write() で行を追記する。
 * auto csv = reclog::recording::RecorderFactory::MakeCsvFile("results", "results.csv", "name,value,remainder");
 * csv->Enable();
 * csv->Write("{},{:.6f},{}", "doubled", 7.0, 1);
 *
 * // JSON Lines (NDJSON): 1行1文字列を追記する。JSON化は呼び出し側の責務。
 * auto jl = reclog::recording::RecorderFactory::MakeJsonLinesFile("results", "results.jsonl");
 * jl->Enable();
 * jl->Write("{}", json_string);
 * @endcode
 */
class RecorderFactory {
public:
    /**
     * @brief ファイルに書き込む汎用同期レコーダーを生成する
     *
     * 出力パターンをメッセージのみに設定する。初期状態は disabled。
     *
     * @param name      内部名（重複不可）
     * @param file_path 出力ファイルパス
     */
    static std::unique_ptr<DataRecorder> MakeFile(const std::string &name, const std::string &file_path);

    /**
     * @brief CSV ファイルに書き込む同期レコーダーを生成する
     *
     * ファクトリ生成時にヘッダ行を即時書き込む。
     * 以降は Write() で 1 行ずつデータ行を追記する。
     * 初期状態は disabled（Enable() 後に Write() すること）。
     *
     * @param name      内部名（重複不可）
     * @param file_path 出力ファイルパス（例: "results.csv"）
     * @param header    CSVヘッダ行（例: "step,value,label"）
     */
    static std::unique_ptr<DataRecorder>
    MakeCsvFile(const std::string &name, const std::string &file_path, const std::string &header);

    /**
     * @brief JSON Lines (NDJSON) ファイルに書き込む同期レコーダーを生成する
     *
     * 1行1文字列を追記する形式。Write() に JSON 文字列を渡す（JSON構築は呼び出し側の責務）。
     * MakeFile() と同じ動作だが、用途を名前で明示する。
     * 初期状態は disabled。
     *
     * @param name      内部名（重複不可）
     * @param file_path 出力ファイルパス（例: "results.jsonl"）
     */
    static std::unique_ptr<DataRecorder> MakeJsonLinesFile(const std::string &name, const std::string &file_path);

    /**
     * @brief 標準出力（カラー付き）に書き込む同期レコーダーを生成する
     * @param name 内部名（重複不可）
     */
    static std::unique_ptr<DataRecorder> MakeConsole(const std::string &name);

    /**
     * @brief 何も出力しないレコーダーを生成する（テスト・無効化用）
     */
    static std::unique_ptr<DataRecorder> MakeNull();

    /**
     * @brief SharedSink に "[REC:marker] " マーカー付きで書き込むレコーダーを生成する
     *
     * Logger::Log() の出力と時系列を混同せず突き合わせられるよう、
     * どのモジュールの解析データかを示すマーカーを各行の先頭に付与する。
     * 出力パターンは "%v"（メッセージのみ）に固定される。初期状態は disabled。
     *
     * 同じ SharedSink を使う複数の Logger / DataRecorder は、書き込み先が
     * 単一の spdlog::logger（単一のファイルハンドル）に集約されるため、
     * 別々のファイルハンドルに書き込む場合と異なり書き込み順序が保証される。
     *
     * @param sink   書き込み先を共有する SharedSink
     * @param marker 行頭に付与するマーカー文字列（例: "solver" → "[REC:solver] "）
     */
    static std::unique_ptr<DataRecorder> MakeShared(const reclog::sink::SharedSink &sink, const std::string &marker);
};

} // namespace reclog::recording
