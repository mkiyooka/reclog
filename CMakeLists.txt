# CMake version requirements:
# - cmake --preset: >= 3.19
# - project requirement: >= 3.19
cmake_minimum_required(VERSION 3.19)

# Set default build type（project()より前に設定する必要がある）
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting default build type to Release")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

project(reclog
    VERSION 0.1.0
    LANGUAGES CXX C
)

# PROJECT_IS_TOP_LEVEL は CMake 3.21 以降のみ。3.19対応のため CMAKE_PROJECT_NAME で代用する。
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(RECLOG_IS_TOP_LEVEL ON)
else()
    set(RECLOG_IS_TOP_LEVEL OFF)
endif()
option(RECLOG_BUILD_TESTS "Build reclog tests" ${RECLOG_IS_TOP_LEVEL})
option(RECLOG_BUILD_EXAMPLES "Build reclog examples" ${RECLOG_IS_TOP_LEVEL})
# FetchContent/add_subdirectory 経由の利用では install(EXPORT) を評価しない。
# cmake --install で配布したい場合のみ明示的に ON にする。
option(RECLOG_INSTALL "Enable reclog install()/export (find_package(reclog CONFIG) support)" OFF)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON) # Enable position-independent code. add "-fPIC"

# --- コンパイラの確認 ---
# コンパイラは project() 呼び出し時に確定する。
# pixi run 経由で実行する場合は pixi がPATHをセットしてからCMakeを起動するため、
# pixi環境のコンパイラが自動的に検出される。
message(STATUS "C   compiler : ${CMAKE_C_COMPILER} (${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION})")
message(STATUS "C++ compiler : ${CMAKE_CXX_COMPILER} (${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION})")

# --- ccache ---
# pixi run 経由ではpixi環境のccacheがPATHに含まれるため NO_DEFAULT_PATH は使わない
find_program(CCACHE_EXE NAMES ccache)
if(CCACHE_EXE)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXE}")
    set(CMAKE_C_COMPILER_LAUNCHER   "${CCACHE_EXE}")
    message(STATUS "ccache        : ${CCACHE_EXE}")
else()
    message(STATUS "ccache        : not found")
endif()
option(USE_PCH "Use precompiled headers" OFF)

# --- リンカー選択 ---
set(LINKER "auto" CACHE STRING "Linker to use (auto/mold/lld/bfd/default)")
set_property(CACHE LINKER PROPERTY STRINGS auto mold lld bfd default)

# AppleClang は -fuse-ld= オプション非対応のためリンカ切り替え不可
if (CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang)$")
    if (LINKER STREQUAL "auto")
        set(_candidates mold lld)
    elseif(LINKER STREQUAL "default")
        set(_candidates "")
    else()
        set(_candidates ${LINKER})
    endif()

    set(_linker_found FALSE)
    foreach(linker IN LISTS _candidates)
        if (linker STREQUAL "bfd")
            message(STATUS "linker        : bfd (system default)")
            set(_linker_found TRUE)
            break()
        endif()

        # pixi run 経由ではPATHにpixi環境のbinが含まれているため通常のfind_programで発見できる
        # mold: "mold", lld: "lld"（macOSではld.moldやld.lldは存在しない場合がある）
        find_program(_linker_path NAMES ${linker} ld.${linker})
        if (_linker_path)
            message(STATUS "linker        : ${linker} (${_linker_path})")
            add_link_options(-fuse-ld=${linker})
            set(_linker_found TRUE)
            unset(_linker_path CACHE)
            break()
        else()
            message(STATUS "linker        : ${linker} not found, trying next")
            unset(_linker_path CACHE)
        endif()
    endforeach()

    if(NOT _linker_found)
        message(STATUS "linker        : system default (no alternative found)")
    endif()
endif()

# compile_commands.json 生成（エディタは build/compile_commands.json を直接参照する）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)

# カバレッジフラグ（add_subdirectory より前に設定する必要がある）
include(cmake/coverage-flags.cmake)

# サードパーティライブラリの導入
# FETCHCONTENT_BASE_DIR をプロジェクトルート直下の _deps に固定することで
# build / build-asan / build-coverage 間でダウンロード・展開済みソースを共有し
# 各プリセットの初回 configure 時の再ダウンロードを防ぐ
# （サブプロジェクトとして取り込まれた場合は取り込み側の設定を尊重する）
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/_deps" CACHE PATH
        "FetchContent shared source cache")
endif()
include(FetchContent)
include(cmake/local-or-fetch.cmake)
include(cmake/dependencies-lib.cmake)
include(cmake/dependencies-test.cmake)

# サニタイザフラグ（FetchContent より後に定義することでサードパーティに伝播しない）
include(cmake/sanitizer-flags.cmake)

# ==============================================================================
# Library - メインライブラリ
# ==============================================================================

# メインのソースディレクトリ
add_subdirectory(src)

# Add debug flags for Debug and RelWithDebInfo builds
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")
    message(STATUS "Debug symbols enabled for CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
elseif (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -g -O2")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g -O2")
    message(STATUS "Release with debug symbols enabled for CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
endif()

# ==============================================================================
# Development utilities
# ==============================================================================

# valgrind (memcheck) の設定
# include(CTest) は enable_testing() を含み、DartConfiguration.tcl を生成する。
# ctest -T memcheck で登録済みテストを自動的に valgrind 実行できる。
find_program(MEMORYCHECK_COMMAND NAMES valgrind)
if(MEMORYCHECK_COMMAND)
    set(MEMORYCHECK_COMMAND_OPTIONS
        "--error-exitcode=1 --leak-check=full --show-leak-kinds=all --track-origins=yes"
        CACHE STRING "valgrind options for ctest memcheck")
    message(STATUS "valgrind      : ${MEMORYCHECK_COMMAND}")
endif()
include(CTest)

configure_file(
    ${PROJECT_SOURCE_DIR}/cmake/CTestCustom.cmake.in
    ${PROJECT_BINARY_DIR}/CTestCustom.cmake
    @ONLY
)

if(RECLOG_BUILD_TESTS)
    add_subdirectory(tests)
    add_subdirectory(benches)
endif()

if(RECLOG_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# カスタムターゲット
include(cmake/custom-targets.cmake)

# カバレッジ（COVERAGE_TEST_TARGETS に実行するテストバイナリを指定する）
if(RECLOG_BUILD_TESTS)
    set(COVERAGE_TEST_TARGETS test_doctest_usage test_logger test_recorder_manager test_output_context test_shared_sink)
    include(cmake/coverage.cmake)
endif()

# コード品質ツール
include(cmake/quality-setup.cmake)

# ==============================================================================
# Third-Party License Aggregation
# ==============================================================================

# サードパーティライブラリのライセンス集約
# FetchContent 経由で取り込まれた場合、取り込み側にも同名ターゲットがあると衝突するため
# トップレベルビルドのときだけ定義する。
# 取り込んだライブラリ側（このガードが入る前のバージョン）が既に同名ターゲットを
# 定義しているケースもあるため NOT TARGET も併せて確認する。
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR AND NOT TARGET collect-licenses)
    add_custom_target(collect-licenses
        # FetchContent で取得したライブラリのライセンス
        COMMAND ${CMAKE_COMMAND} -E echo "Collecting FetchContent package licenses..."
        COMMAND ${CMAKE_COMMAND}
            -DDEPS_DIR=${FETCHCONTENT_BASE_DIR}
            -DOUTPUT_DIR=${CMAKE_BINARY_DIR}/third_party_licenses
            -P ${PROJECT_SOURCE_DIR}/cmake/collect-fetchcontent-licenses.cmake
        COMMENT "Collecting third-party library licenses"
    )
endif()

# ==============================================================================
# Install / Export（find_package(reclog CONFIG) 対応）
# ==============================================================================
# FetchContent/add_subdirectory 経由で取り込まれる場合はこのブロックを評価しない
# （reclog_logging/reclog_recording の PRIVATE 依存である spdlog が export セットに
#   含まれていないと install(EXPORT) がエラーになるため）。
# cmake --install で配布したい場合は -DRECLOG_INSTALL=ON を指定する。

if(RECLOG_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    # FetchContent でビルドした依存は reclog 自身の export セットに含める必要がある
    # （install(EXPORT) は静的リンクの推移依存が export セットに存在することを要求するため）。
    # find_package で見つかった依存は呼び出し側の環境にインストール済みなので対象外
    # （reclogConfig.cmake.in の find_dependency() で解決される）。
    set(_reclog_export_targets reclog_sink reclog_logging reclog_recording reclog_output)
    if(RECLOG_FMT_FETCHED)
        list(APPEND _reclog_export_targets fmt)
    endif()
    if(RECLOG_SPDLOG_FETCHED)
        list(APPEND _reclog_export_targets spdlog)
    endif()

    install(TARGETS ${_reclog_export_targets}
        EXPORT reclogTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(EXPORT reclogTargets
        FILE reclogTargets.cmake
        NAMESPACE reclog::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/reclog
    )

    configure_package_config_file(
        cmake/reclogConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/reclogConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/reclog
    )
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/reclogConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/reclogConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/reclogConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/reclog
    )
endif()

