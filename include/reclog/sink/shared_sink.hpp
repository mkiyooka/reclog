#pragma once

#include <memory>
#include <string>

namespace reclog::sink {

/**
 * @brief 複数の Logger / DataRecorder が書き込み先を共有するためのハンドル
 *
 * 診断ログと解析データを別々のファイルに分けず、1つのファイルに
 * マーカー付きで集約したい場合に使う。実体（spdlog::logger）は
 * pimpl で隠蔽され、公開ヘッダーに spdlog の型は一切現れない。
 *
 * SharedSink 自体は出力先（ファイル）を表すだけで、ログレベルや
 * Enable/Disable の状態は持たない。マーカー付与や出力制御は
 * LoggerFactory::MakeShared() / RecorderFactory::MakeShared() が
 * 返す Logger / DataRecorder 側の責務になる。
 *
 * @code
 * reclog::sink::SharedSink sink("run.log");
 *
 * auto logger = reclog::logging::LoggerFactory::MakeShared(sink, reclog::logging::LogLevel::kInfo);
 * auto rec    = reclog::recording::RecorderFactory::MakeShared(sink, "solver");
 *
 * logger->Log(reclog::logging::LogLevel::kInfo, "start");   // [LOG] start
 * rec->Enable();
 * rec->Write("{},{:.6f}", step, value);                     // [REC:solver] 0,1.000000
 * @endcode
 */
class SharedSink {
public:
    explicit SharedSink(const std::string &file_path);
    ~SharedSink();

    SharedSink(const SharedSink &) = delete;
    SharedSink &operator=(const SharedSink &) = delete;
    SharedSink(SharedSink &&) = delete;
    SharedSink &operator=(SharedSink &&) = delete;

    /**
     * @brief 内部実装（spdlog::logger）への型消去されたハンドルを返す
     *
     * LoggerFactory::MakeShared() / RecorderFactory::MakeShared() が
     * 内部で使うためのものであり、利用側が直接使うことは想定していない。
     */
    const std::shared_ptr<void> &Impl() const { return impl_; }

private:
    std::shared_ptr<void> impl_;
};

} // namespace reclog::sink
