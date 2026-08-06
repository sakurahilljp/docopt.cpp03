# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased] - 2026-08-07

### Added
- Added documentation in `README.md` explaining help message section parsing rules:
  - Details on case-insensitivity and the requirement of a trailing colon (`:`).
  - Explanation of substring/partial matching for custom section names (e.g., `Standard Options:`, `Advanced Options:`, `PROGRAM USAGE:`).
  - Explanation of how unrecognized sections (e.g., `Description:`, `Examples:`) are treated (ignored by parser, printed on help request).
  - Concrete examples for both custom section names and unrecognized sections.
- Added a GoogleTest unit test case `CustomSectionNamesPartialMatch` in `unit_tests.cpp` to verify partial match section parsing.
- Added a GoogleTest unit test case `UnrecognizedSectionsIgnored` in `unit_tests.cpp` to verify that sections not containing `"usage:"` or `"options:"` are ignored by the parser.
- Added a GoogleTest unit test case `MultilineUsageAndOptions` in `unit_tests.cpp` to verify parsing of options and usage patterns spread across multiple lines.

### Fixed
- Fixed a bug in `docopt.cpp` where argument parsing terminated prematurely when encountering an empty string (`""`) as a command-line argument.
  - Replaced incorrect `tokens.current().empty()` checks with `tokens.empty()` in `parse_long`, `parse_shorts`, and `parse_argv` to avoid falsely treating an empty string argument as the end of the argument stream.
- Added a GoogleTest unit test case `EmptyStringArgument` in `unit_tests.cpp` to verify parsing empty string option values and positional arguments.


## [1.0.0] - 2026-07-31

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
