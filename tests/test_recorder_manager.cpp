#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <cstdint>
#include <stdexcept>

#include "acta/recording/null_recorder.hpp"
#include "acta/recording/recorder_manager.hpp"

namespace {
enum class Module : std::uint8_t { kX, kY };
} // namespace

TEST_CASE("RecorderManager registers and accesses recorders by key") {
    acta::recording::RecorderManager<Module> manager;
    manager.RegisterRecorder(Module::kX, std::make_unique<acta::recording::NullRecorder>());

    CHECK_NOTHROW(manager[Module::kX]);
}

TEST_CASE("RecorderManager throws on unregistered key access") {
    acta::recording::RecorderManager<Module> manager;
    manager.RegisterRecorder(Module::kX, std::make_unique<acta::recording::NullRecorder>());

    CHECK_THROWS_AS(manager[Module::kY], std::out_of_range);
}

TEST_CASE("RecorderManager::FlushAll flushes every registered recorder") {
    class FlushCountingRecorder : public acta::recording::DataRecorder {
    public:
        void Enable() override {}
        void Disable() override {}
        bool IsEnabled() const override { return false; }
        void Output(std::string_view /*message*/) override {}
        void Flush() override { ++flush_count; }

        int flush_count = 0;
    };

    acta::recording::RecorderManager<Module> manager;
    auto recorder_x = std::make_unique<FlushCountingRecorder>();
    auto recorder_y = std::make_unique<FlushCountingRecorder>();
    auto *raw_x = recorder_x.get();
    auto *raw_y = recorder_y.get();

    manager.RegisterRecorder(Module::kX, std::move(recorder_x));
    manager.RegisterRecorder(Module::kY, std::move(recorder_y));

    manager.FlushAll();

    CHECK(raw_x->flush_count == 1);
    CHECK(raw_y->flush_count == 1);
}
