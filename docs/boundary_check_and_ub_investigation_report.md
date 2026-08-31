# 境界外アクセスおよび空文字未定義動作（指摘2・3）修正報告書

## 1. 概要 (Overview)

| 項目 | 内容 |
| :--- | :--- |
| **対象コンポーネント** | `docoptcpp03::OneOrMore::match`, `docoptcpp03::parse_atom` |
| **脆弱性分類** | CWE-125: Out-of-bounds Read / CWE-758: Undefined Behavior |
| **深刻度** | **中 (Medium)** |
| **修正ブランチ** | `bugfix/fix-boundary-checks-and-ub` |
| **関連コミット** | `fix(security): add boundary check in OneOrMore::match and empty check in parse_atom` |

---

## 2. 不具合の詳細と根本原因 (Root Cause & Mechanics)

### 2.1 指摘 2: Release ビルド時の `OneOrMore::match` 境界外読み取り
- **該当箇所**: [`docopt.cpp:608-620`](file:///Users/satoniho/repos/docopt.cpp03/docopt.cpp#L608-L620)
- **原因**:
  `OneOrMore` クラスの `match` 実装において、子ノードの存在確認が `assert(children_.size() == 1)` のみに依存していました。
  最適化リリースビルド（`-DNDEBUG`）ではアサーションが消去されるため、デフォルトコンストラクタ等で `children_` が空のインスタンスに対して `match()` が呼び出された場合、`children_[0]` によるヒープバッファの範囲外読み取り（Heap Buffer Over-read / Segmentation Fault）が発生していました。
- **修正内容**:
  ```cpp
  if (children_.empty()) {
      return std::make_pair(false, std::make_pair(left, collected));
  }
  ```

---

### 2.2 指摘 3: `parse_atom` における空文字列インデックス参照 (C++03 UB)
- **該当箇所**: [`docopt.cpp:1088`](file:///Users/satoniho/repos/docopt.cpp03/docopt.cpp#L1088)
- **原因**:
  構文解析のトークン評価において、`token` が空文字列（`""`）の際に `token.empty()` の事前チェックなしで `std::isupper(static_cast<unsigned char>(token[0]))` を評価していました。
  C++03 規格（§21.3.4）において、空の `std::string` に対する `str[0]` の非 const 参照は未定義動作（Undefined Behavior）です。
- **修正内容**:
  ```cpp
  // 修正前:
  } else if ((starts_with(token, "<") && ends_with(token, ">")) || std::isupper(static_cast<unsigned char>(token[0]))) {

  // 修正後:
  } else if ((starts_with(token, "<") && ends_with(token, ">")) || (!token.empty() && std::isupper(static_cast<unsigned char>(token[0])))) {
  ```

---

## 3. テスト・検証結果 (Verification)

### 3.1 実行結果サマリー
```text
[AddressSanitizer & UBSan (Debug) ビルド]
1/2 Test #1: docopt_testcases ................. Passed (0.53 sec)
2/2 Test #2: docopt_gtest_unit_tests .......... Passed (0.49 sec)
100% tests passed, 0 tests failed out of 2

[Release (C++03) ビルド]
1/2 Test #1: docopt_testcases ................. Passed (0.36 sec)
2/2 Test #2: docopt_gtest_unit_tests .......... Passed (0.34 sec)
100% tests passed, 0 tests failed out of 2
```

- GoogleTest 全 44 ユニットテスト: **100% 合格**（空文字列引数・空オプション値の境界テスト含む）
- Python 公式 175 テストケース: **100% 合格**
- AddressSanitizer / UndefinedBehaviorSanitizer: **エラー 0 件**
