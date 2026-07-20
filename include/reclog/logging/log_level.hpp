#pragma once

#include <cstdint>

namespace reclog::logging {

/**
 * @brief ログレベル定義
 */
enum class LogLevel : std::uint8_t { kTrace = 0, kDebug, kInfo, kWarn, kError, kCritical, kOff };

} // namespace reclog::logging
