#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "reclog/logging/logger_factory.hpp"
#include "reclog/recording/recorder_factory.hpp"
#include "reclog/sink/shared_sink.hpp"

using reclog::logging::LogLevel;

namespace {

std::vector<std::string> ReadLines(const std::filesystem::path &path) {
    std::ifstream file(path);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    return lines;
}

// テスト間でファイルパスが衝突しないよう、テストケースごとに一意なパスを払い出す。
std::filesystem::path MakeTempPath(const std::string &name) {
    auto path = std::filesystem::temp_directory_path() / ("reclog_test_" + name + ".log");
    std::filesystem::remove(path);
    return path;
}

} // namespace

TEST_CASE("SharedSink: Logger writes with a [LOG] marker") {
    auto path = MakeTempPath("logger_marker");
    {
        const reclog::sink::SharedSink sink(path.string());
        auto logger = reclog::logging::LoggerFactory::MakeShared(sink, LogLevel::kInfo);
        logger->Log(LogLevel::kInfo, "hello={}", 1);
    }

    auto lines = ReadLines(path);
    REQUIRE(lines.size() == 1);
    CHECK(lines[0] == "[LOG] hello=1");
}

TEST_CASE("SharedSink: Logger below the configured level is suppressed") {
    auto path = MakeTempPath("logger_level");
    {
        const reclog::sink::SharedSink sink(path.string());
        auto logger = reclog::logging::LoggerFactory::MakeShared(sink, LogLevel::kWarn);
        logger->Log(LogLevel::kInfo, "hidden");
        logger->Log(LogLevel::kWarn, "visible");
    }

    auto lines = ReadLines(path);
    REQUIRE(lines.size() == 1);
    CHECK(lines[0] == "[LOG] visible");
}

TEST_CASE("SharedSink: DataRecorder writes with a [REC:marker] prefix") {
    auto path = MakeTempPath("recorder_marker");
    {
        const reclog::sink::SharedSink sink(path.string());
        auto rec = reclog::recording::RecorderFactory::MakeShared(sink, "solver");
        rec->Enable();
        rec->Write("{},{:.6f}", 0, 1.0);
        rec->Flush();
    }

    auto lines = ReadLines(path);
    REQUIRE(lines.size() == 1);
    CHECK(lines[0] == "[REC:solver] 0,1.000000");
}

TEST_CASE("SharedSink: DataRecorder discards Write() while disabled") {
    auto path = MakeTempPath("recorder_disabled");
    {
        const reclog::sink::SharedSink sink(path.string());
        auto rec = reclog::recording::RecorderFactory::MakeShared(sink, "solver");
        rec->Write("{},{:.6f}", 0, 1.0); // 初期状態は disabled
        rec->Flush();
    }

    CHECK(ReadLines(path).empty());
}

TEST_CASE("SharedSink: Logger and DataRecorder sharing a sink do not clobber each other's state") {
    auto path = MakeTempPath("shared_independent_state");
    {
        const reclog::sink::SharedSink sink(path.string());
        auto logger = reclog::logging::LoggerFactory::MakeShared(sink, LogLevel::kError);
        auto rec = reclog::recording::RecorderFactory::MakeShared(sink, "mesh");
        rec->Enable();

        // Logger のレベルを厳しく絞っても、同じ sink を使う Recorder の Enable 状態は影響を受けない。
        logger->Log(LogLevel::kInfo, "suppressed");
        rec->Write("{}", "kept");
        logger->Log(LogLevel::kError, "kept-log");
    }

    auto lines = ReadLines(path);
    REQUIRE(lines.size() == 2);
    CHECK(lines[0] == "[REC:mesh] kept");
    CHECK(lines[1] == "[LOG] kept-log");
}

TEST_CASE("SharedSink: multiple recorders with different markers append to the same file in call order") {
    auto path = MakeTempPath("recorder_interleave");
    {
        const reclog::sink::SharedSink sink(path.string());
        auto solver = reclog::recording::RecorderFactory::MakeShared(sink, "solver");
        auto mesh = reclog::recording::RecorderFactory::MakeShared(sink, "mesh");
        solver->Enable();
        mesh->Enable();

        solver->Write("{}", "s0");
        mesh->Write("{}", "m0");
        solver->Write("{}", "s1");
    }

    auto lines = ReadLines(path);
    REQUIRE(lines.size() == 3);
    CHECK(lines[0] == "[REC:solver] s0");
    CHECK(lines[1] == "[REC:mesh] m0");
    CHECK(lines[2] == "[REC:solver] s1");
}
