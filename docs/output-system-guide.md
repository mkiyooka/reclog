# 出力システム利用ガイド

設計・実装の詳細は [output-system.md](output-system.md) を参照。

---

## 基本的な使い方

### 1. モジュールキーを定義する

出力先をモジュール単位で切り替えるために `enum class` を定義する。

```cpp
enum class Module : std::uint8_t { kSolver, kPostproc };
```

### 2. Logger と RecorderManager を初期化する

```cpp
#include "reclog/logging/logger_factory.hpp"
#include "reclog/output/output_context.hpp"
#include "reclog/recording/recorder_factory.hpp"
#include "reclog/recording/recorder_manager.hpp"

// 診断ログ: コンソール出力、Info レベル以上
auto logger = reclog::logging::LoggerFactory::MakeConsole("app", reclog::logging::LogLevel::kInfo);

// 解析データ: モジュールごとにファイル出力
reclog::recording::RecorderManager<Module> manager;
manager.RegisterRecorder(Module::kSolver,   reclog::recording::RecorderFactory::MakeFile("solver",   "solver.csv"));
manager.RegisterRecorder(Module::kPostproc, reclog::recording::RecorderFactory::MakeFile("postproc", "postproc.csv"));
```

### 3. OutputContext にまとめて注入する

```cpp
reclog::output::OutputContext<Module> out(*logger, manager);
run(out);
```

---

## Logger の使い方

### ログレベルと出力制御

```cpp
reclog::logging::Logger& log = out.GetLogger();

log.Log(reclog::logging::LogLevel::kTrace,    "詳細トレース");
log.Log(reclog::logging::LogLevel::kDebug,    "デバッグ情報");
log.Log(reclog::logging::LogLevel::kInfo,     "進行状況");
log.Log(reclog::logging::LogLevel::kWarn,     "警告");
log.Log(reclog::logging::LogLevel::kError,    "エラー");
log.Log(reclog::logging::LogLevel::kCritical, "致命的エラー");
```

`fmt::format_string` を使ったフォーマットヘルパーもある。`ShouldLog()` が false の場合は
フォーマット処理自体をスキップするため、無効時のコストはゼロ。

```cpp
log.Log(reclog::logging::LogLevel::kDebug, "step={}, value={:.6f}", step, value);
```

### ファクトリ一覧

| メソッド                                             | 出力先                 |
| ---------------------------------------------------- | ---------------------- |
| `reclog::logging::LoggerFactory::MakeConsole(name)`    | 標準出力（カラー付き） |
| `reclog::logging::LoggerFactory::MakeFile(name, path)` | ファイル（同期）       |
| `reclog::logging::LoggerFactory::MakeNull()`           | 何もしない             |

`MakeConsole` / `MakeFile` は第3引数でログレベルを指定できる（省略時は `kInfo`）。

---

## DataRecorder の使い方

### Enable / Disable と Write

```cpp
reclog::recording::DataRecorder& rec = out.GetRecorders()[Module::kSolver];

rec.Enable();                              // 記録開始
rec.Write("{},{:.6f}", step, value);       // fmt 形式で書き込み
rec.Write("{},{},{}", t, x, y);
rec.Disable();                             // 記録停止（以降の Write は無視される）
rec.Flush();                               // バッファをフラッシュ
```

`Write()` はフォーマット文字列をコンパイル時にチェックする（`fmt::format_string`）。
`IsEnabled()` が `false` のときはフォーマット処理自体をスキップするため、無効時のコストはゼロ。

### 全レコーダーを一括フラッシュする

```cpp
out.GetRecorders().FlushAll();
```

### ファクトリ一覧

| メソッド                                                            | 出力先                           | 初期状態 |
| ------------------------------------------------------------------- | -------------------------------- | -------- |
| `reclog::recording::RecorderFactory::MakeFile(name, path)`            | ファイル（同期）                 | disabled |
| `reclog::recording::RecorderFactory::MakeCsvFile(name, path, header)` | CSV ファイル（ヘッダ行自動出力） | disabled |
| `reclog::recording::RecorderFactory::MakeJsonLinesFile(name, path)`   | JSON Lines (NDJSON)              | disabled |
| `reclog::recording::RecorderFactory::MakeConsole(name)`               | 標準出力                         | disabled |
| `reclog::recording::RecorderFactory::MakeNull()`                      | 何もしない                       | disabled |

いずれのファクトリで生成したレコーダーも初期状態は `disabled`。
記録を開始するには明示的に `Enable()` を呼ぶ。

JSON Lines 出力では JSON 文字列の構築は呼び出し側の責務。`Write()` には
組み立て済みの JSON 文字列を渡す。

---

## テストでの使い方

テストでは `reclog::logging::NullLogger` と `SpyLogger` を使い、外部 I/O なしに動作を検証できる。

```cpp
#include "reclog/logging/null_logger.hpp"
#include "reclog/recording/null_recorder.hpp"
#include "support/spy_logger.hpp"

// ログ検証が不要な場合
reclog::logging::NullLogger null_log;

// ログ内容を検証したい場合
SpyLogger test_log;
test_log.SetLevel(reclog::logging::LogLevel::kDebug);

reclog::recording::RecorderManager<Module> manager;
manager.RegisterRecorder(Module::kSolver, std::make_unique<reclog::recording::NullRecorder>());

reclog::output::OutputContext<Module> out(test_log, manager);
run(out);

assert(test_log.Entries().size() == 3);
assert(test_log.Entries()[0] == "initialize start");

test_log.clear(); // エントリをリセット
```

---

## よくあるパターン

### モジュールごとに出力を切り替える

```cpp
void run(reclog::output::OutputContext<Module>& out) {
    out.GetRecorders()[Module::kSolver].Enable();
    out.GetRecorders()[Module::kPostproc].Disable(); // この回は不要

    for (int step = 0; step < steps; ++step) {
        double x = compute(step);
        out.GetRecorders()[Module::kSolver].Write("{},{:.9f}", step, x);
        out.GetLogger().Log(reclog::logging::LogLevel::kDebug, "step done");
    }

    out.GetRecorders().FlushAll();
}
```

### 出力を完全に無効化する（本番ランの高速化）

```cpp
auto logger = reclog::logging::LoggerFactory::MakeNull();

reclog::recording::RecorderManager<Module> manager;
manager.RegisterRecorder(Module::kSolver,   reclog::recording::RecorderFactory::MakeNull());
manager.RegisterRecorder(Module::kPostproc, reclog::recording::RecorderFactory::MakeNull());

reclog::output::OutputContext<Module> out(*logger, manager);
run(out); // I/O コストゼロ
```

### ログレベルを動的に変更する

```cpp
out.GetLogger().SetLevel(reclog::logging::LogLevel::kWarn); // Debug・Info を抑制
```
