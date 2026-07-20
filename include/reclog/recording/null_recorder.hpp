#pragma once

#include "reclog/recording/data_recorder.hpp"

namespace reclog::recording {

/**
 * @brief 何も出力しないレコーダー
 *
 * 出力を完全に無効化したい場合や、DI先のデフォルト実装として使用する。
 */
class NullRecorder : public DataRecorder {
public:
    void Enable() override {}
    void Disable() override {}
    bool IsEnabled() const override { return false; }
    void Output(std::string_view /*message*/) override {}
    void Flush() override {}
};

} // namespace reclog::recording
