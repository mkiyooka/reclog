# ビルドシステム

このドキュメントでは、プロジェクトのビルドシステム・開発ツール・ディレクトリ構成を説明する。

## 概要

| 項目                 | 内容              |
| -------------------- | ----------------- |
| ビルドシステム       | CMake 4.1 + Ninja |
| 環境管理             | pixi              |
| C++ 標準             | C++17             |
| コンパイラキャッシュ | ccache            |

## pixi タスク一覧

日常的な開発作業はすべて `pixi run <タスク名>` で実行できる。

### ビルド関連

| タスク         | コマンド                             | 説明                        |
| -------------- | ------------------------------------ | --------------------------- |
| `config`       | `cmake --preset=release`             | リリースビルドの CMake 設定 |
| `config-debug` | `cmake --preset=debug`               | デバッグビルドの CMake 設定 |
| `build`        | `cmake --build build -j 8`           | ビルド（8並列）             |
| `test`         | `ctest --test-dir build`             | テスト実行                  |
| `clean`        | `cmake --build build --target clean` | ビルド成果物の削除          |

### コード品質

| タスク         | 説明                                       |
| -------------- | ------------------------------------------ |
| `format`       | clang-format によるコード整形              |
| `lint`         | clang-tidy による静的解析                  |
| `run-cppcheck` | cppcheck による静的解析                    |
| `fullcheck`    | typos + lint + run-cppcheck をまとめて実行 |

### カバレッジ

| タスク            | 説明                                             |
| ----------------- | ------------------------------------------------ |
| `config-coverage` | カバレッジビルドの CMake 設定                    |
| `coverage`        | テスト実行・プロファイルデータ統合・レポート生成 |
| `coverage-report` | HTML レポートをブラウザで開く（macOS のみ）      |

### Valgrind（Linux のみ）

| タスク     | 説明                                           |
| ---------- | ---------------------------------------------- |
| `valgrind` | ctest memcheck 経由で valgrind を実行（Linux） |

事前に `pixi run build`（デバッグビルド推奨）を済ませてから実行する。

```bash
pixi run config-debug && pixi run build
pixi run valgrind
```

## CMake プリセット

`CMakePresets.json` で以下のプリセットを定義している。

| プリセット       | ビルドタイプ   | ビルドディレクトリ | 説明                       |
| ---------------- | -------------- | ------------------ | -------------------------- |
| `release`        | Release        | `build/`           | 最適化ビルド               |
| `debug`          | Debug          | `build/`           | デバッグシンボル付きビルド |
| `relwithdebinfo` | RelWithDebInfo | `build/`           | 最適化 + デバッグシンボル  |
| `coverage`       | Debug          | `build-coverage/`  | カバレッジ計測用ビルド     |

```bash
# プリセットを直接使う場合
cmake --preset=release
cmake --build build --preset=release
ctest --preset=release
```

## CMake モジュール構成

`cmake/` 以下のモジュールを目的別に分けて管理している。

- `cmake/`
    - `local-or-fetch.cmake` — `add_external_package()` ヘルパー。`third_party/<名前>` があればローカルを使い、なければ FetchContent でダウンロードする
    - `dependencies-lib.cmake` — メインライブラリ用サードパーティライブラリの定義
    - `dependencies-test.cmake` — テスト・ベンチマーク用ライブラリの定義
    - `coverage-flags.cmake` — カバレッジ計測用コンパイルフラグ（`add_subdirectory` より前に include する必要がある）
    - `coverage.cmake` — カバレッジ計測・レポート生成ターゲットの定義
    - `quality-setup.cmake` — コード品質ツールのセットアップエントリポイント
    - `quality-tools.cmake` — clang-format / clang-tidy / cppcheck ターゲットの定義
    - `custom-targets.cmake` — `run-tests`・`show-help` などのカスタムターゲット
    - `collect-fetchcontent-licenses.cmake` — サードパーティライブラリのライセンスファイル収集

## ライブラリのビルド形式

`BUILD_SHARED_LIBS` CMake 変数で共有ライブラリ・静的ライブラリを切り替えられる。

```bash
# 静的ライブラリ（デフォルト）
cmake --preset=release

# 共有ライブラリ
cmake --preset=release -DBUILD_SHARED_LIBS=ON
```

## ビルド成果物

ビルド後の主要な成果物は以下の場所に生成される。

- `build/src/libtemplate_library_cpp.a` — 静的ライブラリ（デフォルト）
- `build/src/libtemplate_library_cpp.dylib` / `.so` — 共有ライブラリ（`BUILD_SHARED_LIBS=ON` 時）
- `build/tests/test_*` — テストバイナリ
- `build/benches/bench_*` — ベンチマークバイナリ
- `compile_commands.json` — LSP / clangd 向けの補完データベース（プロジェクトルートに自動コピーされる）

## コンパイラ設定

### リンカの自動選択

CMakeLists.txt でリンカを自動選択する。GCC / Clang 系コンパイラの場合、以下の順で試みる。

1. mold
2. lld
3. システムデフォルト

`LINKER` キャッシュ変数で明示的に指定することもできる。

```bash
cmake --preset=release -DLINKER=lld
```

AppleClang は `-fuse-ld=` オプション非対応のためリンカ切り替えは行われない。

### ccache

`ccache` が PATH 上にある場合、自動的にコンパイラランチャーとして設定される。
再ビルド時間を大幅に短縮できる。

## カバレッジ

LLVM ソースベースカバレッジ（`-fprofile-instr-generate -fcoverage-mapping`）を使用する。

### 仕組み

1. `cmake/coverage-flags.cmake` でコンパイルフラグを設定（`add_subdirectory` より前）
2. テストバイナリを `LLVM_PROFILE_FILE` 環境変数付きで実行し `.profraw` を生成
3. `llvm-profdata merge` で `.profraw` を統合して `.profdata` を生成
4. `llvm-cov report` でターミナルにテキストレポートを出力
5. `llvm-cov show --format=html` で `build-coverage/coverage-html/` に HTML レポートを生成

### 実行手順

```bash
pixi run config-coverage   # build-coverage/ に CMake 設定
pixi run -e dev coverage          # テスト実行 → レポート生成
pixi run -e dev coverage-report   # ブラウザで HTML レポートを開く（macOS のみ）
```

### 注意事項

- カバレッジビルドは `build-coverage/` を使用する（`build/` とは別）
- macOS では AppleClang、Linux では pixi の clang（`CC=clang CXX=clang++`）を使用する
- `llvm-tools` パッケージ（`llvm-cov`・`llvm-profdata`）が pixi 環境に含まれている

## コード品質ツール

### clang-format

`.clang-format` の設定に従いコードを整形する。

```bash
pixi run -e dev format      # 整形を適用
cmake --build build --target format-dry  # 変更なしで確認のみ
```

### clang-tidy

`.clang-tidy` の設定に従い静的解析を行う。
`run-clang-tidy` が利用可能な場合は並列実行される（コア数の半分）。

```bash
pixi run -e dev lint
```

### cppcheck

ソースコードとヘッダファイルに対して静的解析を実行する。

```bash
pixi run -e dev run-cppcheck          # 通常実行
cmake --build build --target run-cppcheck-verbose  # 詳細出力
```

### typos

ソースコード中のスペルミスを検出する。

```bash
pixi run typos
```

## サードパーティライブラリの管理

`add_external_package()` ヘルパー（`cmake/local-or-fetch.cmake`）を使って依存関係を管理する。

- `third_party/<ライブラリ名>-<バージョン>/` ディレクトリが存在する場合: ローカルのソースを使用
- ディレクトリが存在しない場合: FetchContent で自動ダウンロード

新しいライブラリを追加する場合は `cmake/dependencies-lib.cmake`（または `dependencies-test.cmake`）に以下の形式で追記する。

```cmake
add_external_package(mylib third_party/mylib-1.0.0
    URL https://github.com/example/mylib/archive/refs/tags/v1.0.0.tar.gz
    URL_HASH SHA256=...
)
FetchContent_MakeAvailable(mylib)
```

ライセンスファイルは以下のコマンドで収集できる。

```bash
cmake --build build --target collect-licenses
# → build/third_party_licenses/ に収集される
```

## 対応プラットフォーム詳細

### macOS

- コンパイラ: AppleClang（システムデフォルト）
- リンカ: system
- pixi の `clang++` は macOS SDK のリンカと非互換のため使用しない
- カバレッジは `config-coverage` タスクで自動的に AppleClang を使用

### Linux x86-64

- コンパイラ: GCC 15（デフォルト）または Clang 21
- リンカ: mold（高速リンク）
- カバレッジビルドは `[target.linux-64.tasks.config-coverage]` で `CC=clang CXX=clang++` を明示指定
