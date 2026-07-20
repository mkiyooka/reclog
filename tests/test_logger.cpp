#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "reclog/logging/logger_factory.hpp"
#include "reclog/logging/null_logger.hpp"
#include "support/spy_logger.hpp"

using reclog::logging::LogLevel;

TEST_CASE("NullLogger discards all messages") {
    reclog::logging::NullLogger logger;

    CHECK(logger.Level() == LogLevel::kOff);

    logger.SetLevel(LogLevel::kTrace);
    logger.Log(LogLevel::kInfo, "message");

    CHECK(logger.Level() == LogLevel::kOff);
}

TEST_CASE("SpyLogger records entries at or above the configured level") {
    SpyLogger logger;
    logger.SetLevel(LogLevel::kInfo);

    logger.Log(LogLevel::kDebug, "hidden");
    logger.Log(LogLevel::kInfo, "visible-1");
    logger.Log(LogLevel::kError, "visible-2");

    REQUIRE(logger.Entries().size() == 2);
    CHECK(logger.Entries()[0] == "visible-1");
    CHECK(logger.Entries()[1] == "visible-2");
}

TEST_CASE("SpyLogger clear() resets accumulated entries") {
    SpyLogger logger;
    logger.SetLevel(LogLevel::kTrace);
    logger.Log(LogLevel::kInfo, "message");
    REQUIRE(logger.Entries().size() == 1);

    logger.clear();
    CHECK(logger.Entries().empty());
}

TEST_CASE("ShouldLog compares against the current level") {
    SpyLogger logger;
    logger.SetLevel(LogLevel::kWarn);

    CHECK_FALSE(logger.ShouldLog(LogLevel::kInfo));
    CHECK(logger.ShouldLog(LogLevel::kWarn));
    CHECK(logger.ShouldLog(LogLevel::kError));
}

TEST_CASE("Logger::Log format helper skips formatting cost when suppressed") {
    SpyLogger logger;
    logger.SetLevel(LogLevel::kError);

    logger.Log(LogLevel::kInfo, "value={}", 42);
    CHECK(logger.Entries().empty());

    logger.Log(LogLevel::kError, "value={}", 42);
    REQUIRE(logger.Entries().size() == 1);
    CHECK(logger.Entries()[0] == "value=42");
}

TEST_CASE("LoggerFactory::MakeNull returns a logger that discards messages") {
    auto logger = reclog::logging::LoggerFactory::MakeNull();
    logger->SetLevel(LogLevel::kTrace);
    logger->Log(LogLevel::kInfo, "message");

    CHECK(logger->Level() == LogLevel::kOff);
}
