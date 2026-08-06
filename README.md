# docopt.cpp03 : A C++03 Port

This repository is a complete, header-friendly C++03 port of the official Python `docopt` library (v0.6.2).

`docopt.cpp03` helps you create beautiful command-line interfaces by parsing arguments automatically based on your help message.

## Features

- **Strict C++03 Compliance**: Compatible with legacy systems, embedded compilers, and modern C++ standard options (`-std=c++03` / `-std=c++98`).
- **100% Test Pass Rate**: Verified against all 175 official `docopt` test cases in `testcases.docopt`.
- **Memory Safe**: Custom intrusive smart pointers handle AST graph node lifecycles without memory leaks or double frees.
- **Clean Public Interface**: All public class declarations are placed at the top of `docopt.h`, keeping internal implementation details encapsulated in `docopt.cpp`.

## Usage Example

```cpp
#include "docopt.h"
#include <iostream>
#include <vector>

static const char USAGE[] =
"Naval Fate.\n"
"\n"
"Usage:\n"
"  naval_fate ship new <name>...\n"
"  naval_fate ship [<name>] move <x> <y> [--speed=<kn>]\n"
"  naval_fate ship shoot <x> <y>\n"
"  naval_fate mine (set|remove) <x> <y> [--moored|--drifting]\n"
"  naval_fate -h | --help\n"
"  naval_fate --version\n"
"\n"
"Options:\n"
"  -h --help     Show this screen.\n"
"  --version     Show version.\n"
"  --speed=<kn>  Speed in knots [default: 10].\n"
"  --moored      Moored (anchored) mine.\n"
"  --drifting    Drifting mine.\n";

int main(int argc, char* argv[]) {
    // 1. Convenience method: docopt() handles --help, --version, and argument errors automatically via std::exit(status)
    const docoptcpp03::Options result = docoptcpp03::docopt(USAGE, argc - 1, argv + 1, true, "Naval Fate 2.0");

    for (docoptcpp03::Options::const_iterator it = result.begin(); it != result.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }

    return 0;
}
```

## Parsing Functions: `docopt()` vs `docopt_parse()`

`docoptcpp03` provides two functions depending on your error handling needs:

- **`docoptcpp03::docopt(...)`**: Convenient auto-exiting parser. Automatically catches exceptions, prints help/error messages to `std::cout`/`std::cerr`, and calls `std::exit(status)`. Ideal for standard CLI applications.
- **`docoptcpp03::docopt_parse(...)`**: Pure parser that throws exceptions on help, version, or syntax errors. Ideal when you want to handle exceptions manually or embed the parser in larger applications.

## Exception Handling (`DocoptExit` & `DocoptLanguageError`)

`docoptcpp03` defines a structured exception hierarchy under the `docoptcpp03` namespace for use with `docopt_parse()`:

### `DocoptExit` Hierarchy

`DocoptExit` (inheriting from `std::runtime_error`) is the base class thrown when command-line argument parsing requires program exit. It provides three specialized derived exception classes:

1. **`DocoptExitHelp`**: Thrown when the user requests help via `-h` or `--help`. (`e.status == 0`, `e.usage` contains help text)
2. **`DocoptExitVersion`**: Thrown when the user requests version information via `--version`. (`e.status == 0`, `e.usage` contains version string)
3. **`DocoptArgumentError`**: Thrown when command-line arguments are invalid or malformed (e.g. missing arguments, unrecognized options). (`e.status == 1`, `e.usage` contains usage pattern)

#### Class Properties

- `e.status`: Exit status integer (`0` for `--help`/`--version`, `1` for argument errors).
- `e.usage`: Formatted usage text or version string.
- `e.what()`: Detailed descriptive error message.

#### Recommended Handling Pattern for `docopt_parse()`

You can catch all exit conditions uniformly via `DocoptExit`, or catch specific conditions individually:

```cpp
try {
    const docoptcpp03::Options opts = docoptcpp03::docopt_parse(USAGE, argc - 1, argv + 1, true, "1.0.0");
    // Normal application execution logic
} catch (const docoptcpp03::DocoptExitHelp& e) {
    std::cout << e.usage << std::endl;
    return e.status; // 0
} catch (const docoptcpp03::DocoptExitVersion& e) {
    std::cout << e.usage << std::endl;
    return e.status; // 0
} catch (const docoptcpp03::DocoptArgumentError& e) {
    std::cerr << "Invalid arguments!\n" << e.usage << std::endl;
    return e.status; // 1
} catch (const docoptcpp03::DocoptExit& e) {
    // Catch-all fallback for any DocoptExit
    std::cout << e.usage << std::endl;
    return e.status;
} catch (const docoptcpp03::DocoptLanguageError& e) {
    // Log developer docstring syntax error
    std::cerr << "Docopt docstring error: " << e.what() << std::endl;
    return 1;
}
```

### `DocoptLanguageError`

`DocoptLanguageError` is thrown when the developer-supplied `USAGE` docstring itself contains syntax errors, invalid pattern definitions, or malformed options. This signals a bug in the docstring grammar definition rather than an issue with user input.

## Building & Testing

```bash
mkdir build
cd build
cmake ..
make
ctest --output-on-failure
```

### Integrating with CMake

You can use `find_package` or `FetchContent` to easily incorporate `docopt.cpp03` into your CMake project:

```cmake
# Using find_package after installing
find_package(docoptcpp03 REQUIRED)
target_link_libraries(my_target PRIVATE docoptcpp03::docopt)
```

### Running GoogleTest Unit Tests Directly

```bash
./build/unit_tests
```

### Running Official docopt Testcases Directly

```bash
./build/test_docopt docopt.python/testcases.docopt
```

### Running Sample Programs (`cmds/`)

```bash
# Naval Fate sample
./build/naval_fate ship new Enterprise Voyager
./build/naval_fate ship Enterprise move 10 20 --speed=15
./build/naval_fate mine set 5 5 --drifting

# Calculator sample
./build/calculator add 10 20 30
./build/calculator multiply 2 3 4
```

## Value Types & Accessors

The result of `docoptcpp03::Docopt` is `docoptcpp03::Options`. `docoptcpp03::Value` represents parsed command-line option values.

### `Value` Accessor & Conversion Method Table

`docoptcpp03::Value` provides a unified, intuitive API for value retrieval, type conversion, and fallback accessors:

| Method | Return / Target Type | Description | Example |
| :--- | :--- | :--- | :--- |
| `val.as_bool()` | `bool` | Gets boolean value (`true` / `false`) | `opts["--verbose"].as_bool()` |
| `val.as_long()` | `long` | Gets long integer value | `opts["--count"].as_long()` |
| `val.as_double()` | `double` | Gets floating-point value (converted from number or string) | `opts["--speed"].as_double()` |
| `val.as_string()` | `const std::string&` | Gets string value | `opts["<file>"].as_string()` |
| `val.as_string_list()` | `const std::vector<std::string>&` | Gets vector of string arguments | `opts["<file>"].as_string_list()` |
| `val.as<T>()` | `T` | Converts single value to type `T` via `boost::lexical_cast<T>` | `opts["--port"].as<int>()` |
| `val.as_list<T>()` | `std::vector<T>` | Converts string list to `std::vector<T>` via `boost::lexical_cast<T>` | `opts["<nums>"].as_list<int>()` |
| `val.as_bool_or(default_val)` | `bool` | Gets boolean value or returns fallback if empty | `opts["--flag"].as_bool_or(false)` |
| `val.as_long_or(default_val)` | `long` | Gets long value or returns fallback if empty | `opts["--count"].as_long_or(1L)` |
| `val.as_double_or(default_val)` | `double` | Gets double value or returns fallback if empty | `opts["--speed"].as_double_or(1.0)` |
| `val.as_string_or(default_val)` | `std::string` | Gets string value or returns fallback if empty | `opts["--output"].as_string_or("stdout")` |
| `val.as_or<T>(default_val)` | `T` | Converts single value to type `T` or returns fallback if empty | `opts["--speed"].as_or<double>(0.05)` |
| `val.is_truthy()` | `bool` | Returns `true` if flag is `true` or non-empty value is set | `opts["--verbose"].is_truthy()` |
| `if (val)` / Safe Bool Cast | `bool` | Contextual Safe Bool conversion to test if value is set / truthy | `if (opts["--verbose"])` |
| `if (!val)` / Logical NOT | `bool` | Logical negation operator to test if value is empty / falsy | `if (!opts["--output"])` |

### `docopt for C++11` `value` Compatibility Methods (CamelCase)

To make migration from the `docopt for C++11` library seamless, `docoptcpp03::Value` provides CamelCase compatibility wrappers matching [`docopt_value.h`](https://github.com/docopt/docopt.cpp/blob/master/docopt_value.h):

| CamelCase Method | Equivalent `snake_case` Method | Description |
| :--- | :--- | :--- |
| `val.isBool()` | `val.is_bool()` | Check if value is boolean |
| `val.isLong()` | `val.is_long()` | Check if value is long integer |
| `val.isString()` | `val.is_string()` | Check if value is string |
| `val.isStringList()` | `val.is_string_list()` | Check if value is string list |
| `val.asBool()` | `val.as_bool()` | Get boolean value |
| `val.asLong()` | `val.as_long()` | Get long integer value |
| `val.asDouble()` | `val.as_double()` | Get double floating-point value |
| `val.asString()` | `val.as_string()` | Get string reference |
| `val.asStringList()` | `val.as_string_list()` | Get vector of strings reference |

### Options Helper Methods

`docoptcpp03::Options` provides convenient query, getter, and developer debug dumping methods:

- `opts.has_key("--speed")` / `opts.contains("--speed")` -> check if key exists
- `opts.get("--speed", "10")` -> get string value or default if not set
- `opts.get<int>("--port", 8080)` -> get type-casted value or default if not set
- `opts.dump()` / `std::cout << opts;` -> print developer debug dump of all parsed options
- `opts.dump_string()` -> returns debug dump as `std::string`

`docoptcpp03::Value` also provides `.to_json()` for JSON serialization and supports stream output (`std::cout << val`).

By default, `docopt.h` includes official Boost headers (`<boost/lexical_cast.hpp>`).
If compiling with `-std=c++03` under newer Boost versions (Boost 1.81+) that require C++11, define `DOCOPT_USE_CUSTOM_LEXICAL_CAST` to use the C++03-compatible `boost::lexical_cast` fallback:

```cpp
#define DOCOPT_USE_CUSTOM_LEXICAL_CAST
#include "docopt.h"
```

## Help Message Section Rules

`docopt` automatically parses the help message string (docstring) to extract configurations. The parser looks for specific keywords to define sections.

### Supported Section Names
- **`usage:`** (Required)
- **`options:`** (Optional)

### Syntax and Formatting Rules
- **Case-Insensitive**: Section headers are case-insensitive. `Usage:`, `USAGE:`, `Options:`, and `options:` are all treated identically.
- **Trailing Colon Required**: Section headers must end with a colon (`:`). For example, `Usage` without a colon will not start a section.
- **Partial Matching**: The parser searches for headers using substring matching (`std::string::find`). This allows custom prefixes, such as:
  - `Standard Options:`
  - `Advanced Options:`
  - `PROGRAM USAGE:`
  These are parsed successfully as `options:` or `usage:` sections respectively.
- **Indentation**: Sub-items and patterns within a section must be indented relative to the section header. The section terminates at the first empty line or unindented block.
- **Unrecognized Sections**: Any sections that do not contain `"usage:"` or `"options:"` in their headers (e.g., `Description:`, `Examples:`, `Notes:`) are treated as plain text and ignored during argument parsing. However, they are still displayed when the help message is printed.

  **Example of Unrecognized Sections:**
  ```text
  Usage:
    program --help

  Description:
    This is a sample program. This text is ignored by the parser.

  Examples:
    program --help
  ```
  In this example, both the `Description:` and `Examples:` sections are skipped by the parser during argument validation, but will be printed to standard output when the user runs the program with `--help`.

### Example of Custom Section Names

```text
PROGRAM USAGE:
  program [options] <argument>

Standard Options:
  --verbose       Enable verbose logging.

Developer Options:
  --debug         Enable debug mode.
```

In this example:
1. `PROGRAM USAGE:` is parsed as the `usage:` section due to substring matching.
2. Both `Standard Options:` and `Developer Options:` contain the substring `"options:"` and are therefore parsed and merged together into the unified options configuration.

## Docstring Syntax Quick Reference

`docopt` automatically generates command-line parsers by parsing your help message string (`USAGE`). Here is a quick reference for supported `USAGE` patterns:

| Pattern | Description | Example |
| :--- | :--- | :--- |
| `<name>` or `NAME` | Required positional argument | `my_app <file>` |
| `-f`, `--flag` | Option flag (boolean `true`/`false`) | `my_app --verbose` |
| `-s <val>`, `--speed=<val>` | Option requiring an explicit argument | `my_app --speed=10` |
| `[--option]` | Optional flag or argument | `my_app [--speed=<kn>]` |
| `(set \| remove)` | Required mutually exclusive choice | `my_app (set \| remove)` |
| `<item>...` | Repeatable argument or option (parses into string list) | `my_app <file>...` |
| `[default: 10]` | Default value specification in option description | `--speed=<kn> Speed [default: 10]` |

## License

This project is open-source software licensed under the [MIT License](LICENSE).
