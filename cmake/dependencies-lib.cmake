# reclog ライブラリのサードパーティ依存定義
#
# 呼び出し側が既に fmt/spdlog を導入している場合はそれを優先し、
# 無ければ find_package → FetchContent の順にフォールバックする。

# ---------- fmt ----------
# reclog_logging / reclog_recording の公開ヘッダーが fmt::format_string を使うため PUBLIC 依存。
#
# RECLOG_FMT_FETCHED: このプロジェクトの FetchContent で fmt ターゲットを新規に
# 作成したかどうか。RECLOG_INSTALL=ON 時、FetchContent 由来の fmt は PUBLIC 依存として
# 利用側のリンクに必要になるため reclog 自身の export セットに含める（install(TARGETS) 側で参照）。
set(RECLOG_FMT_FETCHED OFF)
if(NOT TARGET fmt::fmt)
    find_package(fmt CONFIG QUIET)
    if(NOT fmt_FOUND)
        add_external_package(fmt third_party/fmt-12.0.0
            URL https://github.com/fmtlib/fmt/archive/refs/tags/12.0.0.tar.gz
            URL_HASH SHA256=aa3e8fbb6a0066c03454434add1f1fc23299e85758ceec0d7d2d974431481e40
        )
        FetchContent_MakeAvailable(fmt)
        set(RECLOG_FMT_FETCHED ON)
    endif()
endif()

# ---------- spdlog ----------
# spdlog は実装（.cpp）にのみ現れる PRIVATE 依存。
# SPDLOG_FMT_EXTERNAL=ON により、上記 fmt::fmt を共有し ODR 違反を防ぐ。
#
# RECLOG_SPDLOG_FETCHED: このプロジェクトの FetchContent で spdlog ターゲットを
# 新規に作成したかどうか。RECLOG_INSTALL=ON 時、FetchContent 由来の spdlog は
# reclog 自身の export セットに含める必要があるため、install(TARGETS) 側で参照する。
set(RECLOG_SPDLOG_FETCHED OFF)
if(TARGET spdlog::spdlog)
    # 呼び出し側が既に導入済み
elseif(TARGET spdlog::spdlog_header_only)
    # 同上（header-only 版）
else()
    find_package(spdlog CONFIG QUIET)
    if(NOT spdlog_FOUND)
        set(SPDLOG_FMT_EXTERNAL ON CACHE BOOL "" FORCE)
        add_external_package(spdlog third_party/spdlog-1.17.0
            URL https://github.com/gabime/spdlog/archive/refs/tags/v1.17.0.tar.gz
            URL_HASH SHA256=d8862955c6d74e5846b3f580b1605d2428b11d97a410d86e2fb13e857cd3a744
        )
        FetchContent_MakeAvailable(spdlog)
        set(RECLOG_SPDLOG_FETCHED ON)
    endif()
endif()
