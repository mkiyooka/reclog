# reclog

C++17 の出力システムライブラリ。診断ログ（`reclog::logging`）と解析データ記録
（`reclog::recording`）を DI で分離しつつ統一的に管理する。
pixi による再現性の高い開発環境と、カバレッジ計測・コード品質ツールを提供する。

設計の詳細は [docs/output-system.md](docs/output-system.md)、
利用方法は [docs/output-system-guide.md](docs/output-system-guide.md) を参照。

## 必要条件

- [pixi](https://prefix.dev/docs/pixi/overview)

pixi をインストール後、`pixi install` でコンパイラ・ツール類が自動セットアップされる。

## クイックスタート

```bash
pixi run config   # CMake 設定（Release）
pixi run build    # ビルド
pixi run test     # テスト実行
```

## 対応プラットフォーム

| プラットフォーム              | コンパイラ        | リンカ             |
| ----------------------------- | ----------------- | ------------------ |
| Linux x86-64                  | GCC 15 / Clang 21 | mold（高速リンク） |
| macOS (Apple Silicon / Intel) | AppleClang        | system             |

## サードパーティライブラリ

| ライブラリ | バージョン | ライセンス | 用途                                |
| ---------- | ---------- | ---------- | ----------------------------------- |
| fmt        | 12.2.0     | MIT        | 文字列フォーマット（公開API）       |
| spdlog     | 1.17.0     | MIT        | ログ出力（実装に閉じる、非公開）    |
| doctest    | 2.5.3      | MIT        | テストフレームワーク                |
| nanobench  | 4.3.11     | MIT        | マイクロベンチマーク                |

バージョン・取得先は [cmake/dependencies-lib.cmake](cmake/dependencies-lib.cmake)、
[cmake/dependencies-test.cmake](cmake/dependencies-test.cmake) を参照。

## 主要タスク一覧

```bash
# ---- 通常ワークフロー ----
pixi run config           # CMake 設定（Release）
pixi run config-debug     # CMake 設定（Debug）
pixi run build            # ビルド
pixi run test             # テスト実行
pixi run clean            # ビルド成果物をクリーン

# ---- コード品質 ----
pixi run format           # clang-format によるコード整形
pixi run lint             # clang-tidy による静的解析
pixi run run-cppcheck     # cppcheck による静的解析
pixi run fullcheck        # typos + lint + cppcheck をまとめて実行

# ---- サニタイザ（ASan + UBSan） ----
pixi run asan             # 設定 → ビルド → テストをまとめて実行（build-asan/）

# ---- カバレッジ ----
pixi run coverage         # 設定 → 計測 → HTML レポート生成（build-coverage/）

# ---- メモリチェック（Linux のみ） ----
pixi run config-debug && pixi run build
pixi run valgrind         # ctest memcheck 経由で valgrind を実行
```

詳細なビルドシステムの説明は [docs/build-system.md](docs/build-system.md) を参照。

## ビルド成果物

```text
build/
├── src/
│   ├── libreclog_logging.a      # 診断ログライブラリ（静的、デフォルト）
│   └── libreclog_recording.a    # 解析データ記録ライブラリ（静的、デフォルト）
├── tests/
│   └── test_*                 # テストバイナリ
├── benches/
│   └── bench_*                # ベンチマークバイナリ
└── examples/
    └── example_output          # output-system の使用例
```

## ライブラリのビルド形式

`BUILD_SHARED_LIBS` CMake 変数で静的・共有ライブラリを切り替えられる。

```bash
# 静的ライブラリ（デフォルト）
cmake --preset=release
cmake --build build

# 共有ライブラリ
cmake --preset=release -DBUILD_SHARED_LIBS=ON
cmake --build build
```

## ターゲット構成

reclog は 3 つの CMake ターゲットで構成される。

| ターゲット        | 種別       | 内容                                                            |
| ----------------- | ---------- | --------------------------------------------------------------- |
| `reclog::logging`   | ライブラリ | `Logger` インターフェース + spdlog 実装                         |
| `reclog::recording` | ライブラリ | `DataRecorder` インターフェース + spdlog 実装                   |
| `reclog::output`    | INTERFACE  | `OutputContext<Key>`（header-only、logging/recording を束ねる） |

spdlog / fmt は公開ヘッダーから隠蔽されており、利用側は `reclog::output` をリンクするだけで
`Logger` / `DataRecorder` / `OutputContext` を利用できる。詳細は
[docs/output-system.md](docs/output-system.md) を参照。

## ライブラリの利用方法

### 1. FetchContent で取得する（推奨）

利用側の CMakeLists.txt に追加するだけでビルド・リンクまで自動化される。

```cmake
include(FetchContent)
FetchContent_Declare(
    reclog
    GIT_REPOSITORY https://github.com/<your-org>/reclog.git
    GIT_TAG        main  # またはタグ・コミットハッシュ
)
set(RECLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(RECLOG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(reclog)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE reclog::output)
```

### 2. add_subdirectory で取得する

ローカルにクローンしてサブディレクトリとして追加する場合：

```cmake
set(RECLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(RECLOG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/reclog)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE reclog::output)
```

### 3. インストール済みライブラリを find_package で取得する

`-DRECLOG_INSTALL=ON` を指定してビルド・インストールした場合、`find_package(reclog CONFIG)`
で利用できる。

```bash
cmake --preset=release -DRECLOG_INSTALL=ON
cmake --build build
cmake --install build --prefix /path/to/install
```

```cmake
find_package(reclog CONFIG REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE reclog::output)
```

## ディレクトリ構成

- `src/` — ライブラリの実装（spdlog 依存はこの層に閉じる）
    - `logging/` — `reclog::logging::LoggerFactory` の spdlog 実装
    - `recording/` — `reclog::recording::RecorderFactory` の spdlog 実装
- `include/` — 公開ヘッダファイル
    - `reclog/`
        - `logging/` — 診断ログの抽象インターフェース（spdlog 非依存）
        - `recording/` — 解析データ記録の抽象インターフェース（spdlog 非依存）
        - `output/` — `OutputContext<Key>`（DI コンテナ、header-only）
- `tests/` — テストコード（doctest）
    - `support/` — テスト用ヘルパー（`SpyLogger` 等）
- `examples/` — 使用例（`example_output` — Logger + DataRecorder のサンプル）
- `benches/` — ベンチマーク（nanobench）
- `cmake/` — CMake モジュール群
- `docs/` — ドキュメント

## ドキュメント

- [docs/build-system.md](docs/build-system.md) — ビルドシステム・開発ツールの詳細
- [docs/output-system.md](docs/output-system.md) — 出力システムの設計ドキュメント
- [docs/output-system-guide.md](docs/output-system-guide.md) — 出力システムの利用ガイド
