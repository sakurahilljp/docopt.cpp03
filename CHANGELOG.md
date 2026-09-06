# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- **Unit Test Coverage for `options_first` parameter**: Added `OptionsFirstTest` test suite in `unit_tests.cpp` covering:
  - Disabling option parsing after the first positional argument.
  - Parsing options specified before the first positional argument.
  - Behavior comparison between `options_first = true` and `options_first = false` (default).
  - Git-style subcommand dispatching with subcommand-specific options/flags passed as positional arguments.
  - Parsing options when no positional argument is provided.
  - Both `std::vector<std::string>` and C-style `argc, argv` overloads.
- **Unit Test Coverage for `Options` Container API**: Added `OptionsContainerTest` test suite in `unit_tests.cpp` covering:
  - Container capacity and state manipulation (`size()`, `empty()`, `clear()`).
  - Equality and inequality operators (`operator==`, `operator!=`).
  - Iteration and element access via `const_iterator` and non-const `iterator`.
  - Lookup and querying methods (`find()`, `count()`, `has_key()`, `contains()`).
  - Underlying `std::map` accessor (`map()`) and `std::map` constructor.
- **Death Test Coverage for `docopt()` Convenience Wrapper**: Added `DocoptDeathTest` test suite in `unit_tests.cpp` using GoogleTest `EXPECT_EXIT`:
  - Process termination with code `0` on `--help`.
  - Process termination with code `0` on `--version`.
  - Process termination with code `1` on invalid/unrecognized arguments.
  - Process termination with code `1` on usage string syntax errors (`DocoptLanguageError`) with error message output to `std::cerr`.
  - Verification of C-style array `(int argc, char const* const argv[])` overload.




## [1.4.0] - 2026-09-06

### Added
- **Unit Test Coverage for Unified Value API**: Added exhaustive unit test suites in `unit_tests.cpp`:
  - `IsTypeQueryExhaustive`: Complete orthogonal type-query testing across all `Value` variants and `is*()` compatibility wrappers.
  - `AsUnsupportedConversionsThrow`: Complete exception verification (`boost::bad_lexical_cast`) for unconvertible variants (empty, list, invalid strings).
  - `BoolStringVariantsExhaustive`: Edge cases and case-insensitivity verification for boolean conversions (`yes`, `no`, `on`, `off`, whitespace, uppercase).
  - `AsListElementConversionError`: Exception verification on invalid list element conversion.
  - `StringListComparisonAndCache`: Verification of list comparisons and lazy string-list caching.
  - `GetTemplateVariants`: Multi-type retrieval test for `Options::get<T>()`.
  - `ValueAsIntOptimization`: Dedicated testing of optimized `as<int>()` conversion, boundary values (`INT_MAX`, `INT_MIN`), and overflow/underflow handling.

### Removed
- **Redundant Non-Template Scalar Methods**: Removed non-template methods replaced by unified template methods:
  - Type query methods: `is_bool()`, `is_long()`, and `is_string()`.
  - Scalar accessors and fallback methods: `as_bool()`, `as_long()`, `as_double()`, `as_string()`, `as_bool_or()`, `as_long_or()`, `as_double_or()`, and `as_string_or()`.

### Changed
- **Value Type Query Method Unification**: Unified scalar type-checking methods into generic template method `is<T>()`:
  - Retained `is_string_list()` and `is_empty()` as non-template member methods.
  - Added template explicit specializations for `is<bool>()`, `is<long>()`, `is<int>()`, and `is<std::string>()` with dedicated implementations in `docopt.cpp`.
  - Enforced a C++03-compatible compile-time error (static assertion failure via undefined template specialization) for unspecialized / unsupported types (e.g. `double`, custom types) instead of silently returning `false`.
  - Updated C++11 compatibility wrappers (`isBool()`, `isLong()`, `isString()`, `isStringList()`) to delegate to `is<T>()` and `is_string_list()`.
  - Updated all unit tests (`unit_tests.cpp`), test runner (`test_docopt.cpp`), and internal callers across `docopt.cpp` to use the unified `is<T>()` API.
- **Value Accessor and Conversion Method Unification**: Unified non-template scalar accessors into generic template methods `as<T>()` and fallback methods `as_or<T>()` (and `as_or(const char*)`):
  - Retained `as_string_list()` as non-template to provide zero-copy reference access to string arguments (`const std::vector<std::string>&`).
  - Added template explicit specializations for `as<bool>()`, `as<long>()`, `as<int>()`, `as<double>()`, and `as<std::string>()` with dedicated implementations in `docopt.cpp`.
  - Optimized `as<int>()` with a dedicated explicit specialization avoiding string round-trips when converting from `KIND_LONG`, with safe `INT_MIN`/`INT_MAX` boundary checking.
  - Added automatic fallback method `as_or(default_val)` supporting literal strings (`const char*`) and type-deduced defaults.
  - Updated C++11 compatibility wrappers (`asBool()`, `asLong()`, `asDouble()`, `asString()`, `asStringList()`) to delegate to the unified template accessors.
  - Updated all unit tests (`unit_tests.cpp`), test runner (`test_docopt.cpp`), and sample programs (`cmds/`) to use the unified template API.


## [1.3.0] - 2026-09-05

### Changed
- **Refactoring (No functional changes)**: Separated interface declarations and implementations by moving all non-template method definitions from `docopt.h` to `docopt.cpp`:
  - Exception classes: `DocoptLanguageError`, `DocoptExit`, `DocoptExitHelp`, `DocoptExitVersion`, and `DocoptArgumentError`.
  - `Value` class: Constructors, kind checks, non-template accessors (`as_bool`, `as_long`, `as_double`, `as_string`, `as_string_list`), fallback methods (`as_..._or`), and operators (`is_truthy`, `operator!`, `operator!=`, `operator unspecified_bool_type`).
  - `Options` class: Constructors, element access (`operator[]`, `at`, `find`, `count`, `has_key`, `contains`), non-template `get` overloads, iterators, capacity, comparisons, and dump helpers.
  - Internal string helpers: Removed `detail::to_lower_str` and `detail::trim_str` from `docopt.h` and encapsulated string utilities directly in `docopt.cpp`.
- **Refactoring**: Slimmed down `docopt.h` from 651 lines to 363 lines (44% reduction), retaining only pure interface declarations and essential template implementations while completely preserving external behavior, ABI, and API compatibility.


## [1.2.4] - 2026-09-05

### Added
- Added comprehensive unit test coverage for all `Value` accessors, constructors, and conversion methods in `unit_tests.cpp`:
  - `KindAndConstructors`: Direct verification of all 5 `kind()` values and all constructors (`int`, `const char*`, etc.).
  - `FallbackAccessors`: Verification of `as_bool_or`, `as_long_or`, `as_double_or`, and `as_string_or` overloads.
  - `ValueAsTemplateComprehensive`: Full verification of template casts (`as<int>`, `as<long>`, `as<double>`, `as<bool>`, `as<std::string>`).
  - `StreamOutputOperator`: Verification of stream operator `operator<<(std::ostream&, const Value&)`.
- Added investigation report `docs/value_conversion_methods_investigation_report.md`.

### Fixed
- **Value Accessors & Conversion (Logic Defect)**: Resolved silent conversion failures where CLI arguments stored as strings returned uninitialized default values:
  - `as_long()` / `asLong()`: Properly converts string arguments (`KIND_STRING`) to numeric `long` values via lexical casting and handles boolean conversions.
  - `as_bool()` / `asBool()`: Parses boolean strings (`"true"`, `"false"`, `"1"`, `"0"`, `"yes"`, `"no"`, etc.) with whitespace trimming, and non-zero long values.
  - `as_string()` / `asString()`: Returns string representations for `long` and `bool` variants without dangling references.
  - `as_string_list()` / `asStringList()`: Wraps single strings in a 1-element list.
  - `as_long_or()` & `as_string_or()`: Refined fallback methods to properly convert compatible `Value` kinds (`bool` to `long`, `long`/`bool` to `string`) instead of falling back to default values.
  - `as_list<T>()`: Delegates element conversion to `Value::as<T>()`, allowing boolean lists (`as_list<bool>()`) to parse strings like `"true"` / `"false"` correctly.
  - `as<bool>()`: Added template specialization ensuring `"true"` / `"false"` strings are correctly parsed instead of throwing `bad_lexical_cast`.
  - `as_double()`: Standardized exception handling to catch `bad_lexical_cast` and throw `std::runtime_error`.


## [1.2.3] - 2026-08-31

### Added
- Added internal header `docopt_private.h` encapsulating `docoptcpp03::detail::shared_ptr` and internal AST utilities.
- Added CMake build option `ENABLE_ASAN` (`-DENABLE_ASAN=ON`) for AddressSanitizer and UndefinedBehaviorSanitizer dynamic analysis.
- Added GoogleTest unit test suite `SharedPtrTest` in `unit_tests.cpp` to verify smart pointer memory safety:
  - `SelfAssignment`: Verifies self-assignment (`p = p`).
  - `NullAssignment`: Verifies assigning NULL pointers and to NULL pointers.
  - `CascadingDestructionUseAfterFree`: Verifies prevention of Use-After-Free during cascading node destruction (`p = p->child`).
- Added GoogleTest unit test suite `BoundaryCheckTest` in `unit_tests.cpp` for edge cases:
  - `EmptyStringPositionalArgument`: Verifies parsing of empty string positional arguments (`""`).
  - `EmptyStringWithMultipleArgs`: Verifies empty string arguments with surrounding arguments.
  - `EmptyStringOptionValue`: Verifies empty string option values (`--name=`).
- Added comprehensive vulnerability investigation reports in `docs/`:
  - `docs/shared_ptr_uaf_investigation_report.md` (Analysis and resolution of CWE-416 Use-After-Free).
  - `docs/boundary_check_and_ub_investigation_report.md` (Analysis and resolution of CWE-125 out-of-bounds access and empty token UB).

### Security
- **Memory Safety (CWE-416)**: Resolved a critical Use-After-Free (UAF) vulnerability in `shared_ptr::operator=`.
  - Stored the target raw pointer locally and incremented the new object's reference count before decrementing the old object's reference count, preventing destruction of aliased child nodes during assignment.
- **Boundary Safety (CWE-125)**: Fixed an unchecked out-of-bounds vector read in `OneOrMore::match`.
  - Added an explicit `if (children_.empty())` guard to safely return match failure when `children_` is empty in Release builds (`-DNDEBUG`).
- **C++03 Undefined Behavior (CWE-758)**: Fixed an unchecked empty string index access in `parse_atom`.
  - Added `!token.empty()` check before evaluating `token[0]` in `std::isupper(token[0])`.


## [1.2.2] - 2026-08-07

### Added
- Added GoogleTest unit test case `MultilineUsageAndOptions` in `unit_tests.cpp` to verify parsing of options and usage patterns spread across multiple lines.


## [1.2.1] - 2026-08-07

### Added
- Added documentation in `README.md` explaining help message section parsing rules:
  - Details on case-insensitivity and the requirement of a trailing colon (`:`).
  - Explanation of substring/partial matching for custom section names (e.g., `Standard Options:`, `Advanced Options:`, `PROGRAM USAGE:`).
  - Explanation of how unrecognized sections (e.g., `Description:`, `Examples:`) are treated (ignored by parser, printed on help request).
  - Concrete examples for both custom section names and unrecognized sections.
- Added a GoogleTest unit test case `CustomSectionNamesPartialMatch` in `unit_tests.cpp` to verify partial match section parsing.
- Added a GoogleTest unit test case `UnrecognizedSectionsIgnored` in `unit_tests.cpp` to verify that sections not containing `"usage:"` or `"options:"` are ignored by the parser.


## [1.2.0] - 2026-08-07

### Fixed
- Fixed a bug in `docopt.cpp` where argument parsing terminated prematurely when encountering an empty string (`""`) as a command-line argument.
  - Replaced incorrect `tokens.current().empty()` checks with `tokens.empty()` in `parse_long`, `parse_shorts`, and `parse_argv` to avoid falsely treating an empty string argument as the end of the argument stream.

### Added
- Added a GoogleTest unit test case `EmptyStringArgument` in `unit_tests.cpp` to verify parsing empty string option values and positional arguments.
- Added initial `CHANGELOG.md` file documenting project release history.


## [1.1.0] - 2026-08-03

### Added
- Added C++11 `docopt.cpp` compatibility methods to `Value` (CamelCase aliases):
  - `isBool()`, `isLong()`, `isString()`, `isStringList()`
  - `asBool()`, `asLong()`, `asDouble()`, `asString()`, `asStringList()`
- Added `docopt` auto-exiting convenience function overload for CLI applications.
- Added derived exception classes: `DocoptExitHelp`, `DocoptExitVersion`, and `DocoptArgumentError`.

### Changed
- Renamed `Docopt` to `docopt_parse` for clearer separation between exception-throwing parser and auto-exiting convenience function.


## [1.0.0] - 2026-08-03

### Added
- Created complete, strict C++03 port of the official Python `docopt` library (v0.6.2).
- Added GoogleTest unit test suite with 28 comprehensive test cases and 100% pass rate.
- Added standard CLI helper overloads for `docopt` (takes `int argc, char const* const argv[]` excluding program name).
- Designed memory-safe `shared_ptr` intrusive ref-counting smart pointer for AST nodes (fully encapsulated in `docopt.cpp`).
- Implemented `docoptcpp03::Options` class using composition over `std::map<std::string, Value>` with support for `const operator[]` and `at()`.
- Added helper accessors: `Value::as_or<T>()`, `Value::as_string_or()`, `Value::as_long_or()`, `Value::as_bool_or()`, and `Value::as_list<T>()`.
- Integrated customizable compile-time fallback macro `DOCOPT_USE_CUSTOM_LEXICAL_CAST` to support legacy systems lacking modern Boost headers.
- Implemented Safe Bool cast / Truthy evaluation (`if (val)`) on the `Value` class via the C++03 Safe Bool Idiom.
- Added Options helper methods: `has_key()`, `contains()`, and `get()`.
- Added developer debug dump methods: `Options::dump()`, `Options::dump_string()`, and stream `operator<<`.
- Created sample programs `naval_fate` and `calculator` in `cmds/` subdirectories.

### Changed
- Reorganized `docopt.h` to declare all public classes at the top and inline implementations at the bottom for maximum interface readability.
- Aligned naming rules: using PascalCase for classes (`Value`, `Options`) and snake_case for parse functions (`docopt`, `docopt_parse`).
- Standardized const styling to WEST const (`const Type&`).


## [0.6.2] - 2026-08-03

### Added
- Initial ported codebase based on Python `docopt` v0.6.2.

[Unreleased]: https://github.com/sakurahilljp/docopt.cpp03/compare/v1.4.0...HEAD
[1.4.0]: https://github.com/sakurahilljp/docopt.cpp03/compare/v1.3.0...v1.4.0
[1.3.0]: https://github.com/sakurahilljp/docopt.cpp03/compare/v1.2.4...v1.3.0
[1.2.4]: https://github.com/sakurahilljp/docopt.cpp03/compare/v1.2.3...v1.2.4
[1.2.3]: https://github.com/sakurahilljp/docopt.cpp03/compare/v1.2.2...v1.2.3
[1.2.2]: https://github.com/sakurahilljp/docopt.cpp03/compare/v1.2.1...v1.2.2
[1.2.1]: https://github.com/sakurahilljp/docopt.cpp03/compare/v1.2.0...v1.2.1
[1.2.0]: https://github.com/sakurahilljp/docopt.cpp03/compare/v1.1.0...v1.2.0
[1.1.0]: https://github.com/sakurahilljp/docopt.cpp03/compare/v1.0.0...v1.1.0
[1.0.0]: https://github.com/sakurahilljp/docopt.cpp03/compare/v0.6.2...v1.0.0
[0.6.2]: https://github.com/sakurahilljp/docopt.cpp03/releases/tag/v0.6.2

