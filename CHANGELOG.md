# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

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
  - `as_bool()` / `asBool()`: Parses boolean strings (`"true"`, `"false"`, `"1"`, `"0"`, `"yes"`, `"no"`, etc.) and non-zero long values.
  - `as_string()` / `asString()`: Returns string representations for `long` and `bool` variants without dangling references.
  - `as_string_list()` / `asStringList()`: Wraps single strings in a 1-element list.
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

### Fixed
- **Memory Safety (CWE-416)**: Resolved a critical Use-After-Free (UAF) vulnerability in `shared_ptr::operator=`.
  - Stored the target raw pointer locally and incremented the new object's reference count before decrementing the old object's reference count, preventing destruction of aliased child nodes during assignment.
- **Boundary Safety (CWE-125)**: Fixed an unchecked out-of-bounds vector read in `OneOrMore::match`.
  - Added an explicit `if (children_.empty())` guard to safely return match failure when `children_` is empty in Release builds (`-DNDEBUG`).
- **C++03 Undefined Behavior**: Fixed an unchecked empty string index access in `parse_atom`.
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
