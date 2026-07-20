#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <cstdint>

#include "reclog/output/output_context.hpp"
#include "reclog/recording/null_recorder.hpp"
#include "reclog/recording/recorder_manager.hpp"
#include "support/spy_logger.hpp"

namespace {
enum class Module : std::uint8_t { kX };
} // namespace

TEST_CASE("OutputContext exposes the injected logger and recorders") {
    SpyLogger logger;
    logger.SetLevel(reclog::logging::LogLevel::kTrace);

    reclog::recording::RecorderManager<Module> manager;
    manager.RegisterRecorder(Module::kX, std::make_unique<reclog::recording::NullRecorder>());

    reclog::output::OutputContext<Module> context(logger, manager);

    context.GetLogger().Log(reclog::logging::LogLevel::kInfo, "initialize start");
    context.GetRecorders()[Module::kX].Enable();

    REQUIRE(logger.Entries().size() == 1);
    CHECK(logger.Entries()[0] == "initialize start");
    CHECK_FALSE(context.GetRecorders()[Module::kX].IsEnabled());
}
