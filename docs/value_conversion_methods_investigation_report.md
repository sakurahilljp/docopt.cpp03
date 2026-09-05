# Value クラスの Accessor および型変換メソッド不具合 調査・対応報告書

## 1. 概要 (Overview)

| 項目 | 内容 |
| :--- | :--- |
| **対象コンポーネント** | `docoptcpp03::Value` (`as_long`, `as_bool`, `as_string`, `as_string_list`, `as<T>`, `as_list<T>`, `as_..._or`) |
| **不具合分類** | 型アクセサ・型変換の不整合による論理値取得失敗 (Logic / Type Conversion Defect) |
| **深刻度** | **中 (Medium)** - コマンドライン引数の数値・真偽値パース結果が静かに無効化される |
| **作業ブランチ** | `bugfix/fix-value-as-long-conversion` |
| **修正コミット** | `daf9ee9 fix: implement safe type conversions in Value accessors (as_long, as_bool, as_string, as_string_list)`<br>`a2684d7 fix: refine Value conversion methods (as_long_or, as_string_or, as_list, and as_bool trimming)` |

---

## 2. 不具合の詳細と根本原因 (Root Cause & Mechanics)

### 2.1 不具合の事象
1. **`as_long()` / `asLong()`**:
   コマンドライン引数（例: `--port=8080`, `<count> 5`）は内部で文字列（`KIND_STRING`）として格納されますが、`as_long()` が文字列のパースを行わず初期値の `0L` をそのまま返却していました。
2. **`as_bool()` / `asBool()`**:
   `"true"` や `"false"` などの文字列、あるいは `1L` や `0L` などの整数値に対して型解釈を行わず、未初期化の `false` を返却していました。また、前後の空白文字（例: `"  true  "`）が含まれる場合にトリムされず変換に失敗していました。
3. **`as_string()` / `asString()`**:
   `42L` や `true` のような他型の `Value` に対する文字列表現が返却されず、空文字列 `""` が返却されていました。
4. **`as_string_list()` / `asStringList()`**:
   単一文字列 `Value("hello")` に対して 1 要素リストが返却されず、空の vector が返却されていました。
5. **`as<bool>()` / `as_or<bool>()`**:
   `boost::lexical_cast<bool>` の内部ストリームが `"0"` / `"1"` の数字しか受け付けず、`"true"` / `"false"` に対して例外（`bad_lexical_cast`）を送出、または `as_or` で意図せず `false` にフォールバックしていました。
6. **`as_long_or()` / `as_string_or()`**:
   `as_long_or()` は `KIND_BOOL` を判定せずデフォルト値にフォールバックし、`as_string_or()` は `KIND_LONG` や `KIND_BOOL` を判定せずデフォルト値にフォールバックしていました。
7. **`as_list<bool>()`**:
   リスト要素の変換に直接 `boost::lexical_cast<T>` を呼び出していたため、`{"true", "false"}` からの boolean リスト変換で `bad_lexical_cast` がスローされていました。

---

## 3. 解決策 (Fix Implementation)

### 3.1 [`docopt.h`](../docopt.h) の改善
- **フィールドの mutable 化と自動同期**:
  `str_val_` と `str_list_val_` を `mutable` とし、コンストラクタ（`long`, `int`, `bool`）およびアクセサ呼び出し時に同期することで、参照寿命（`const std::string&`）を安全に維持。
- **`as_long()` の型変換実装**:
  `KIND_STRING` の場合は `boost::lexical_cast<long>` で変換。失敗時は `std::runtime_error` を送出。
- **`as_bool()` の安全判定とトリム処理**:
  `detail::trim_str` で前後の空白を除去し、`"true"`, `"1"`, `"yes"`, `"on"` を `true`、`"false"`, `"0"`, `"no"`, `"off"`, `""` を `false`（大文字小文字不問）としてパース。
- **フォールバックアクセサ（`as_long_or`, `as_string_or`）の改善**:
  `as_long_or` は `as_long()` を呼び出し `bool`（`1L`/`0L`）や数値文字列の変換に一貫して対応。`as_string_or` は `long` や `bool` の文字列表現を安全に返却。
- **`as_list<T>()` の要素変換委譲**:
  各要素の変換を `Value(list[i]).as<T>()` に委譲し、`as<bool>()` 特殊化を含む `Value::as<T>` の変換ルールがリスト要素にも適用されるよう修正。
- **`as<bool>()` および `as<std::string>()` の特殊化**:
  `as<bool>()` は `as_bool()`、`as<std::string>()` は `as_string()` を呼び出すテンプレート特殊化を実装。

### 3.2 [`docopt.cpp`](../docopt.cpp) の改善
- **`as_double()` の例外統一**:
  不正文字列パース時の `boost::bad_lexical_cast` を捕捉し、`as_long()` と統一された `std::runtime_error` として送出。

---

## 4. テスト・検証結果 (Verification)

### 4.1 実行結果サマリー
```text
[AddressSanitizer & UBSan (Debug) ビルド]
1/2 Test #1: docopt_testcases ................. Passed (0.47 sec)
2/2 Test #2: docopt_gtest_unit_tests .......... Passed (0.08 sec)
100% tests passed, 0 tests failed out of 2

[Release (C++03 最適化) ビルド]
1/2 Test #1: docopt_testcases ................. Passed (0.36 sec)
2/2 Test #2: docopt_gtest_unit_tests .......... Passed (0.34 sec)
100% tests passed, 0 tests failed out of 2
```

- GoogleTest 全 53 ユニットテスト: **100% 合格**（初期の 7 件および追加エッジケース 4 件の計 11 箇所の変換不具合が解消）
- Python 公式 175 テストケース: **100% 合格**
- AddressSanitizer / UBSan: **エラー 0 件**
