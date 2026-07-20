#pragma once

#include <string>
#include <vector>

#include "reclog/recording/data_recorder.hpp"

/**
 * @brief テスト・ベンチマーク用スパイレコーダー
 *
 * Output() に渡された内容をメモリに蓄積し、テストコードから検証できる。
 * Flush() の呼び出し回数も記録する。
 *
 * @code
 * SpyRecorder recorder;
 * recorder.Enable();
 * process(recorder);
 * assert(recorder.Entries().size() == 3);
 * assert(recorder.FlushCount() == 1);
 * @endcode
 */
class SpyRecorder : public reclog::recording::DataRecorder {
public:
    void Enable() override { enabled_ = true; }
    void Disable() override { enabled_ = false; }
    bool IsEnabled() const override { return enabled_; }

    void Output(std::string_view message) override { entries_.emplace_back(message); }

    void Flush() override { ++flush_count_; }

    /**
     * @brief 蓄積された出力エントリを返す
     */
    const std::vector<std::string> &Entries() const { return entries_; }

    /**
     * @brief Flush() が呼ばれた回数を返す
     */
    int FlushCount() const { return flush_count_; }

    /**
     * @brief 蓄積されたエントリと Flush 回数をリセットする
     */
    void clear() {
        entries_.clear();
        flush_count_ = 0;
    }

private:
    bool enabled_ = false;
    std::vector<std::string> entries_;
    int flush_count_ = 0;
};
