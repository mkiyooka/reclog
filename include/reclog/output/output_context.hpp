#pragma once

#include "reclog/logging/logger.hpp"
#include "reclog/recording/recorder_manager.hpp"

namespace reclog::output {

/**
 * @brief 出力処理に必要な依存をまとめた DI コンテキスト
 *
 * 診断ログ（Logger）と解析データ（RecorderManager）を一つにまとめ、
 * アプリケーションコアに DI で注入する。
 *
 * @tparam Key RecorderManager のキー型（enum class 等）
 *
 * @code
 * enum class Module { kX, kY };
 *
 * reclog::output::OutputContext<Module> output_context(logger, recorder_manager);
 *
 * void run(reclog::output::OutputContext<Module>& output_context) {
 *     output_context.GetLogger().Log(reclog::logging::LogLevel::kDebug, "initialize start");
 *     output_context.GetRecorders()[Module::kX].Enable();
 *     output_context.GetRecorders()[Module::kX].Write("{},{:.6f}", step, value);
 * }
 * @endcode
 */
template <typename Key>
class OutputContext {
public:
    OutputContext(logging::Logger &logger, recording::RecorderManager<Key> &recorders)
        : logger_(&logger),
          recorders_(&recorders) {}

    /**
     * @brief 診断ロガーへの参照を返す
     */
    logging::Logger &GetLogger() { return *logger_; }

    /**
     * @brief レコーダーマネージャーへの参照を返す
     */
    recording::RecorderManager<Key> &GetRecorders() { return *recorders_; }

private:
    logging::Logger *logger_;
    recording::RecorderManager<Key> *recorders_;
};

} // namespace reclog::output
