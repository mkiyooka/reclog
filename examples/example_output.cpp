#include <cstdint>
#include <vector>

#include "reclog/logging/logger_factory.hpp"
#include "reclog/output/output_context.hpp"
#include "reclog/recording/recorder_factory.hpp"
#include "reclog/recording/recorder_manager.hpp"

namespace {

enum class OutputModule : std::uint8_t { kResultsCsv, kResultsJson };

// Logger と DataRecorder を使った出力サンプル
//
// Logger のフォーマット:
//   MakeConsole() の pattern 引数でログ書式パターン文字列を指定する。
//   空文字列（デフォルト）の場合は実装デフォルトの書式が使われる。
//
// DataRecorder のフォーマット:
//   - MakeCsvFile()      : ヘッダ行を自動出力。Write() で行を追記する。
//   - MakeJsonLinesFile(): 1行1文字列 (NDJSON)。JSON構築は呼び出し側の責務。
void RunOutputSample(reclog::output::OutputContext<OutputModule> &output_context) {
    reclog::logging::Logger &logger = output_context.GetLogger();
    reclog::recording::DataRecorder &csv_recorder = output_context.GetRecorders()[OutputModule::kResultsCsv];
    reclog::recording::DataRecorder &json_recorder = output_context.GetRecorders()[OutputModule::kResultsJson];

    constexpr double kInput = 3.5;
    constexpr int kStart = 1;
    constexpr int kCount = 5;
    constexpr int kDivisor = 7;
    constexpr int kTargetValue = 15;

    logger.Log(reclog::logging::LogLevel::kInfo, "=== output sample start ===");
    logger.Log(
        reclog::logging::LogLevel::kInfo, "input={}, start={}, count={}, divisor={}, target_value={}", kInput, kStart,
        kCount, kDivisor, kTargetValue
    );

    constexpr double kDoubled = kInput * 2.0;
    std::vector<int> sequence;
    sequence.reserve(kCount);
    for (int i = 0; i < kCount; ++i) {
        sequence.push_back(kStart + i);
    }
    const int remainder = kTargetValue % kDivisor;

    logger.Log(reclog::logging::LogLevel::kDebug, "doubled({}) = {}", kInput, kDoubled);
    logger.Log(reclog::logging::LogLevel::kDebug, "sequence({}, {}) size={}", kStart, kCount, sequence.size());
    logger.Log(reclog::logging::LogLevel::kDebug, "remainder({}) = {}", kTargetValue, remainder);

    // CSV 出力
    csv_recorder.Enable();
    csv_recorder.Write("{},{:.6f},{}", "doubled", kDoubled, remainder);
    csv_recorder.Write("{},{:.6f},{}", "single", kInput, 2);
    csv_recorder.Flush();
    csv_recorder.Disable();

    // JSON Lines 出力（1行1文字列 / NDJSON。JSON構築は呼び出し側の責務）
    json_recorder.Enable();
    json_recorder.Write(
        R"({{"input":{},"start":{},"count":{},"divisor":{},"target_value":{},"doubled":{},"remainder":{}}})", kInput,
        kStart, kCount, kDivisor, kTargetValue, kDoubled, remainder
    );
    json_recorder.Flush();
    json_recorder.Disable();

    logger.Log(reclog::logging::LogLevel::kInfo, "=== output sample end ===");
}

} // namespace

int main() {
    // Logger: pattern 引数でタイムスタンプ・ロガー名・レベルを含む書式を指定する。
    auto logger = reclog::logging::LoggerFactory::MakeConsole(
        "app", reclog::logging::LogLevel::kDebug, "[%Y-%m-%d %H:%M:%S.%e][%n][%^%l%$]%v"
    );

    // DataRecorder: CSV と JSON Lines (NDJSON) の 2 形式を使い分ける例。
    reclog::recording::RecorderManager<OutputModule> recorder_manager;
    recorder_manager.RegisterRecorder(
        OutputModule::kResultsCsv,
        reclog::recording::RecorderFactory::MakeCsvFile("results_csv", "output/results.csv", "name,value,remainder")
    );
    recorder_manager.RegisterRecorder(
        OutputModule::kResultsJson, reclog::recording::RecorderFactory::MakeJsonLinesFile("results_json", "output/results.jsonl")
    );
    reclog::output::OutputContext<OutputModule> output_context(*logger, recorder_manager);

    RunOutputSample(output_context);

    return 0;
}
