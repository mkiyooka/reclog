#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <cstdint>
#include <stdexcept>

#include "reclog/recording/null_recorder.hpp"
#include "reclog/recording/recorder_manager.hpp"
#include "support/spy_recorder.hpp"

namespace {
enum class Module : std::uint8_t { kX, kY };
} // namespace

TEST_CASE("RecorderManager registers and accesses recorders by key") {
    reclog::recording::RecorderManager<Module> manager;
    manager.RegisterRecorder(Module::kX, std::make_unique<reclog::recording::NullRecorder>());

    CHECK_NOTHROW(manager[Module::kX]);
}

TEST_CASE("RecorderManager throws on unregistered key access") {
    reclog::recording::RecorderManager<Module> manager;
    manager.RegisterRecorder(Module::kX, std::make_unique<reclog::recording::NullRecorder>());

    CHECK_THROWS_AS(manager[Module::kY], std::out_of_range);
}

TEST_CASE("SpyRecorder discards Write() while disabled") {
    SpyRecorder recorder;

    recorder.Write("{},{:.6f}", 0, 0.5);
    CHECK(recorder.Entries().empty());
}

TEST_CASE("SpyRecorder accumulates Write() output while enabled") {
    SpyRecorder recorder;
    recorder.Enable();

    recorder.Write("{},{:.6f}", 0, 0.5);
    recorder.Write("{},{:.6f}", 1, 1.5);

    REQUIRE(recorder.Entries().size() == 2);
    CHECK(recorder.Entries()[0] == "0,0.500000");
    CHECK(recorder.Entries()[1] == "1,1.500000");
}

TEST_CASE("SpyRecorder clear() resets entries and flush count") {
    SpyRecorder recorder;
    recorder.Enable();
    recorder.Write("value={}", 1);
    recorder.Flush();

    recorder.clear();

    CHECK(recorder.Entries().empty());
    CHECK(recorder.FlushCount() == 0);
}

TEST_CASE("RecorderManager::FlushAll flushes every registered recorder") {
    auto recorder_x = std::make_unique<SpyRecorder>();
    auto recorder_y = std::make_unique<SpyRecorder>();
    auto *raw_x = recorder_x.get();
    auto *raw_y = recorder_y.get();

    reclog::recording::RecorderManager<Module> manager;
    manager.RegisterRecorder(Module::kX, std::move(recorder_x));
    manager.RegisterRecorder(Module::kY, std::move(recorder_y));

    manager.FlushAll();

    CHECK(raw_x->FlushCount() == 1);
    CHECK(raw_y->FlushCount() == 1);
}
