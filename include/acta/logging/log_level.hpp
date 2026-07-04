#pragma once

#include <cstdint>

namespace acta::logging {

/**
 * @brief ログレベル定義
 */
enum class LogLevel : std::uint8_t { kTrace = 0, kDebug, kInfo, kWarn, kError, kCritical, kOff };

} // namespace acta::logging
