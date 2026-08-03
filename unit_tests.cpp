#include "docopt.h"
#include <gtest/gtest.h>

#include <vector>
#include <string>
#include <map>

using namespace docoptcpp03;

// Helper to construct vector of strings from C-style array
static std::vector<std::string> make_args(char const* const argv[], size_t count) {
    std::vector<std::string> res;
    for (size_t i = 0; i < count; ++i) {
        res.push_back(argv[i]);
    }
    return res;
}

//------------------------------------------------------------------------------
// 1. Value Variant Type Unit Tests
//------------------------------------------------------------------------------

TEST(ValueTest, EmptyValue) {
    Value v;
    EXPECT_TRUE(v.is_empty());
    EXPECT_FALSE(v.is_bool());
    EXPECT_FALSE(v.is_long());
    EXPECT_FALSE(v.is_string());
    EXPECT_FALSE(v.is_string_list());
    EXPECT_EQ("null", v.to_json());
}

TEST(ValueTest, BoolValue) {
    Value v_true(true);
    Value v_false(false);

    EXPECT_TRUE(v_true.is_bool());
    EXPECT_TRUE(v_true.as_bool());
    EXPECT_EQ("true", v_true.to_json());

    EXPECT_TRUE(v_false.is_bool());
    EXPECT_FALSE(v_false.as_bool());
    EXPECT_EQ("false", v_false.to_json());
}

TEST(ValueTest, LongValue) {
    Value v(42L);
    EXPECT_TRUE(v.is_long());
    EXPECT_EQ(42L, v.as_long());
    EXPECT_EQ("42", v.to_json());
}

TEST(ValueTest, StringValue) {
    Value v("hello world");
    EXPECT_TRUE(v.is_string());
    EXPECT_EQ("hello world", v.as_string());
    EXPECT_EQ("\"hello world\"", v.to_json());
}

TEST(ValueTest, StringListValue) {
    std::vector<std::string> list;
    list.push_back("foo");
    list.push_back("bar");
    Value v(list);

    EXPECT_TRUE(v.is_string_list());
    EXPECT_EQ(2u, v.as_string_list().size());
    EXPECT_EQ("foo", v.as_string_list()[0]);
    EXPECT_EQ("bar", v.as_string_list()[1]);
    EXPECT_EQ("[\"foo\", \"bar\"]", v.to_json());
}

TEST(ValueTest, AsListConversion) {
    std::vector<std::string> list;
    list.push_back("10");
    list.push_back("20");
    list.push_back("30");
    Value v(list);

    std::vector<int> int_list = v.as_list<int>();
    ASSERT_EQ(3u, int_list.size());
    EXPECT_EQ(10, int_list[0]);
    EXPECT_EQ(20, int_list[1]);
    EXPECT_EQ(30, int_list[2]);

    std::vector<double> double_list = v.as_list<double>();
    ASSERT_EQ(3u, double_list.size());
    EXPECT_DOUBLE_EQ(10.0, double_list[0]);
    EXPECT_DOUBLE_EQ(20.0, double_list[1]);
    EXPECT_DOUBLE_EQ(30.0, double_list[2]);
}

TEST(ValueTest, SafeBoolConversion) {
    Value empty_val;
    Value bool_true(true);
    Value bool_false(false);
    Value long_val(42L);
    Value str_val("hello");
    Value empty_str("");

    std::vector<std::string> vec;
    vec.push_back("item");
    Value vec_val(vec);

    std::vector<std::string> empty_vec;
    Value empty_vec_val(empty_vec);

    EXPECT_FALSE(empty_val);
    EXPECT_TRUE(bool_true);
    EXPECT_FALSE(bool_false);
    EXPECT_TRUE(long_val);
    EXPECT_TRUE(str_val);
    EXPECT_FALSE(empty_str);
    EXPECT_TRUE(vec_val);
    EXPECT_FALSE(empty_vec_val);

    if (str_val) {
        SUCCEED();
    } else {
        ADD_FAILURE() << "str_val should evaluate to true in if condition";
    }

    if (!empty_val) {
        SUCCEED();
    } else {
        ADD_FAILURE() << "!empty_val should be true";
    }
}

TEST(ValueTest, EqualityAndComparison) {
    Value v1(10L);
    Value v2(10L);
    Value v3(20L);

    EXPECT_EQ(v1, v2);
    EXPECT_NE(v1, v3);
    EXPECT_LT(v1, v3);
}

//------------------------------------------------------------------------------
// 2. Exception Handling Tests
//------------------------------------------------------------------------------

TEST(ValueTest, DoubleValue) {
    Value v_long(42L);
    EXPECT_DOUBLE_EQ(42.0, v_long.as_double());

    Value v_str("3.14159");
    EXPECT_DOUBLE_EQ(3.14159, v_str.as_double());

    Value v_empty;
    EXPECT_DOUBLE_EQ(1.5, v_empty.as_double_or(1.5));
}

TEST(ExceptionTest, DocoptExitException) {
    DocoptExit ex("User exit", "Usage: prog");
    EXPECT_STREQ("User exit\nUsage: prog", ex.what());
    EXPECT_EQ("Usage: prog", ex.usage);
    EXPECT_EQ(1, ex.status);

    DocoptExit ex_success(0, "Help", "Usage: prog");
    EXPECT_EQ(0, ex_success.status);
}

TEST(ExceptionTest, DerivedExitExceptions) {
    DocoptExitHelp help_ex("Usage: myapp --help");
    EXPECT_EQ(0, help_ex.status);
    EXPECT_EQ("Usage: myapp --help", help_ex.usage);

    DocoptExitVersion ver_ex("1.2.3");
    EXPECT_EQ(0, ver_ex.status);
    EXPECT_EQ("1.2.3", ver_ex.usage);

    DocoptArgumentError arg_ex("Invalid arg", "Usage: myapp");
    EXPECT_EQ(1, arg_ex.status);
    EXPECT_EQ("Usage: myapp", arg_ex.usage);
}

TEST(ExceptionTest, DocoptLanguageErrorException) {
    DocoptLanguageError ex("Invalid doc grammar");
    EXPECT_STREQ("Invalid doc grammar", ex.what());
}

//------------------------------------------------------------------------------
// 3. Basic Usage Parsing Tests
//------------------------------------------------------------------------------

TEST(BasicUsageTest, PositionalArgument) {
    const std::string doc = "Usage: prog <name>";
    char const* argv_raw[] = { "John" };
    std::vector<std::string> argv = make_args(argv_raw, 1);

    Options result = docoptcpp03::docopt_parse(doc, argv);
    EXPECT_TRUE(result["<name>"].is_string());
    EXPECT_EQ("John", result["<name>"].as_string());
}

TEST(BasicUsageTest, MultiplePositionalArguments) {
    const std::string doc = "Usage: prog <src> <dst>";
    char const* argv_raw[] = { "/path/src", "/path/dst" };
    std::vector<std::string> argv = make_args(argv_raw, 2);

    Options result = docoptcpp03::docopt_parse(doc, argv);
    EXPECT_EQ("/path/src", result["<src>"].as_string());
    EXPECT_EQ("/path/dst", result["<dst>"].as_string());
}

TEST(BasicUsageTest, ArgcArgvOverload) {
    const std::string doc = "Usage: my_prog <name> [--verbose]";
    char arg0[] = "Alice";
    char arg1[] = "--verbose";
    char* argv[] = { arg0, arg1 };
    int argc = 2;

    Options result = docoptcpp03::docopt_parse(doc, argc, argv);
    EXPECT_EQ("Alice", result["<name>"].as_string());
    EXPECT_TRUE(result["--verbose"].as_bool());
}

//------------------------------------------------------------------------------
// 4. Options Parsing Tests
//------------------------------------------------------------------------------

TEST(OptionParsingTest, ShortOptionFlag) {
    const std::string doc =
        "Usage: prog [-a]\n"
        "Options: -a  All flag.";
    
    char const* argv_with_a[] = { "-a" };
    Options res1 = docoptcpp03::docopt_parse(doc, make_args(argv_with_a, 1));
    EXPECT_TRUE(res1["-a"].as_bool());

    char const* argv_empty[] = {};
    Options res2 = docoptcpp03::docopt_parse(doc, make_args(argv_empty, 0));
    EXPECT_FALSE(res2["-a"].as_bool());
}

TEST(OptionParsingTest, LongOptionWithEquals) {
    const std::string doc =
        "Usage: prog [--output=<file>]\n"
        "Options: --output=<file>  Output file.";

    char const* argv[] = { "--output=out.txt" };
    Options res = docoptcpp03::docopt_parse(doc, make_args(argv, 1));
    EXPECT_EQ("out.txt", res["--output"].as_string());
}

TEST(OptionParsingTest, ShortOptionStacking) {
    const std::string doc =
        "Usage: prog [-abc]\n"
        "Options:\n"
        "  -a  Option A\n"
        "  -b  Option B\n"
        "  -c  Option C";

    char const* argv[] = { "-ac" };
    Options res = docoptcpp03::docopt_parse(doc, make_args(argv, 1));
    EXPECT_TRUE(res["-a"].as_bool());
    EXPECT_FALSE(res["-b"].as_bool());
    EXPECT_TRUE(res["-c"].as_bool());
}

//------------------------------------------------------------------------------
// 5. Flag Counting Tests
//------------------------------------------------------------------------------

TEST(FlagCountingTest, RepeatableFlags) {
    const std::string doc = "Usage: prog [-v...]";

    char const* argv_v1[] = { "-v" };
    Options res1 = docoptcpp03::docopt_parse(doc, make_args(argv_v1, 1));
    EXPECT_EQ(1L, res1["-v"].as_long());

    char const* argv_v3[] = { "-vvv" };
    Options res3 = docoptcpp03::docopt_parse(doc, make_args(argv_v3, 1));
    EXPECT_EQ(3L, res3["-v"].as_long());
}

//------------------------------------------------------------------------------
// 6. Command & Subcommand Tests
//------------------------------------------------------------------------------

TEST(CommandTest, Subcommands) {
    const std::string doc =
        "Usage:\n"
        "  prog ship new <name>\n"
        "  prog ship move <x> <y>\n";

    char const* argv_new[] = { "ship", "new", "Enterprise" };
    Options res1 = docoptcpp03::docopt_parse(doc, make_args(argv_new, 3));
    EXPECT_TRUE(res1["ship"].as_bool());
    EXPECT_TRUE(res1["new"].as_bool());
    EXPECT_EQ("Enterprise", res1["<name>"].as_string());

    char const* argv_move[] = { "ship", "move", "100", "200" };
    Options res2 = docoptcpp03::docopt_parse(doc, make_args(argv_move, 4));
    EXPECT_TRUE(res2["ship"].as_bool());
    EXPECT_TRUE(res2["move"].as_bool());
    EXPECT_FALSE(res2["new"].as_bool());
    EXPECT_EQ("100", res2["<x>"].as_string());
    EXPECT_EQ("200", res2["<y>"].as_string());
}

//------------------------------------------------------------------------------
// 7. Default Values Tests
//------------------------------------------------------------------------------

TEST(DefaultsTest, DefaultOptionValues) {
    const std::string doc =
        "Usage: prog [--speed=<kn>]\n"
        "Options:\n"
        "  --speed=<kn>  Speed in knots [default: 10].";

    char const* argv_default[] = {};
    Options res1 = docoptcpp03::docopt_parse(doc, make_args(argv_default, 0));
    EXPECT_EQ("10", res1["--speed"].as_string());

    char const* argv_custom[] = { "--speed=25" };
    Options res2 = docoptcpp03::docopt_parse(doc, make_args(argv_custom, 1));
    EXPECT_EQ("25", res2["--speed"].as_string());
}

//------------------------------------------------------------------------------
// 8. Options Shortcut ([options])
//------------------------------------------------------------------------------

TEST(OptionsShortcutTest, AutomaticDocOptions) {
    const std::string doc =
        "Usage: prog [options] <file>\n"
        "Options:\n"
        "  -v, --verbose  Verbose output.\n"
        "  -o, --output=<out>  Output file [default: stdout].";

    char const* argv[] = { "-v", "input.txt" };
    Options res = docoptcpp03::docopt_parse(doc, make_args(argv, 2));
    EXPECT_TRUE(res["--verbose"].as_bool());
    EXPECT_EQ("stdout", res["--output"].as_string());
    EXPECT_EQ("input.txt", res["<file>"].as_string());
}

TEST(OptionsTest, ConstOptionsAccess) {
    const std::string doc = "Usage: prog [--foo=<bar>]";
    char const* argv[] = { "--foo=baz" };

    const Options opts = docoptcpp03::docopt_parse(doc, make_args(argv, 1));

    // const Options operator[] access
    EXPECT_EQ("baz", opts["--foo"].as_string());
    EXPECT_TRUE(opts["--nonexistent"].is_empty());

    // const Options at() access
    EXPECT_EQ("baz", opts.at("--foo").as_string());
    EXPECT_THROW(opts.at("--nonexistent"), std::out_of_range);
}

TEST(ValueTest, ValueDefaultAccessors) {
    Value empty_val;
    Value str_val("100");
    Value bool_val(true);

    EXPECT_EQ("default", empty_val.as_string_or("default"));
    EXPECT_EQ("100", str_val.as_string_or("default"));

    EXPECT_EQ(100L, str_val.as_long_or(0L));
    EXPECT_EQ(50L, empty_val.as_long_or(50L));

    EXPECT_TRUE(bool_val.as_bool_or(false));
    EXPECT_TRUE(empty_val.as_bool_or(true));
    EXPECT_FALSE(empty_val.as_bool_or(false));
}

TEST(ValueTest, ValueAsCast) {
    Value str_int("42");
    Value str_double("3.14");
    Value empty_val;

    EXPECT_EQ(42, str_int.as<int>());
    EXPECT_DOUBLE_EQ(3.14, str_double.as<double>());

    EXPECT_EQ(42, str_int.as_or<int>(0));
    EXPECT_EQ(99, empty_val.as_or<int>(99));
}

TEST(OptionsTest, OptionsHelperMethods) {
    const std::string doc = "Usage: prog [--speed=<kn>] [--verbose]";
    char const* argv[] = { "--speed=25" };

    const Options opts = docoptcpp03::docopt_parse(doc, make_args(argv, 1));

    EXPECT_TRUE(opts.has_key("--speed"));
    EXPECT_TRUE(opts.contains("--speed"));
    EXPECT_TRUE(opts.has_key("--verbose"));
    EXPECT_FALSE(opts.has_key("--nonexistent"));

    EXPECT_EQ("25", opts.get("--speed", "10"));
    EXPECT_EQ("stdout", opts.get("--output", "stdout"));
    EXPECT_EQ(25, opts.get<int>("--speed", 10));
    EXPECT_EQ(8080, opts.get<int>("--port", 8080));
}

TEST(OptionsTest, OptionsDump) {
    const std::string doc = "Usage: prog [--speed=<kn>] <name>";
    char const* argv[] = { "--speed=20", "John" };

    const Options opts = docoptcpp03::docopt_parse(doc, make_args(argv, 2));

    std::string dumped = opts.dump_string();
    EXPECT_TRUE(dumped.find("Options (2 items):") != std::string::npos);
    EXPECT_TRUE(dumped.find("\"--speed\": \"20\"") != std::string::npos);
    EXPECT_TRUE(dumped.find("\"<name>\": \"John\"") != std::string::npos);

    std::ostringstream oss;
    oss << opts;
    EXPECT_EQ(dumped, oss.str());
}

//------------------------------------------------------------------------------
// 9. Error Handling & Validation Tests
//------------------------------------------------------------------------------

TEST(ErrorHandlingTest, MissingRequiredArgument) {
    const std::string doc = "Usage: prog <required>";
    char const* argv[] = {};
    EXPECT_THROW(docoptcpp03::docopt_parse(doc, make_args(argv, 0)), DocoptExit);
}

TEST(ErrorHandlingTest, UnexpectedOption) {
    const std::string doc = "Usage: prog";
    char const* argv[] = { "--unexpected" };
    try {
        docoptcpp03::docopt_parse(doc, make_args(argv, 1));
        FAIL() << "Expected DocoptExit exception";
    } catch (const docoptcpp03::DocoptExit& e) {
        EXPECT_EQ(1, e.status);
    }
}

TEST(ErrorHandlingTest, AmbiguousPrefixOption) {
    const std::string doc = "Usage: prog --aabb | --aa";
    char const* argv[] = { "--a" };
    EXPECT_THROW(docoptcpp03::docopt_parse(doc, make_args(argv, 1)), DocoptExit);
}

TEST(ErrorHandlingTest, InvalidDocSyntaxNoUsage) {
    const std::string doc = "Invalid doc without usage section";
    char const* argv[] = {};
    EXPECT_THROW(docoptcpp03::docopt_parse(doc, make_args(argv, 0)), DocoptLanguageError);
}

TEST(ErrorHandlingTest, HelpAndVersionExitStatus) {
    const std::string doc = "Usage: prog --help\n       prog --version\n\nOptions:\n  --help     Show help\n  --version  Show version";
    
    char const* help_argv[] = { "--help" };
    try {
        docoptcpp03::docopt_parse(doc, make_args(help_argv, 1), true, "2.0.0");
        FAIL() << "Expected DocoptExitHelp exception";
    } catch (const docoptcpp03::DocoptExitHelp& e) {
        EXPECT_EQ(0, e.status);
        EXPECT_TRUE(e.usage.find("Usage: prog") != std::string::npos);
    }

    char const* ver_argv[] = { "--version" };
    try {
        docoptcpp03::docopt_parse(doc, make_args(ver_argv, 1), true, "2.0.0");
        FAIL() << "Expected DocoptExitVersion exception";
    } catch (const docoptcpp03::DocoptExitVersion& e) {
        EXPECT_EQ(0, e.status);
        EXPECT_EQ("2.0.0", e.usage);
    }
}

TEST(ErrorHandlingTest, StackedShortOptionWithUnknown) {
    const std::string doc = "Usage: prog [-h | --help]\n\nOptions:\n  -h --help  Show help";
    char const* argv[] = { "-hoge" };
    try {
        docoptcpp03::docopt_parse(doc, make_args(argv, 1), true);
        FAIL() << "Expected DocoptArgumentError exception";
    } catch (const docoptcpp03::DocoptArgumentError& e) {
        EXPECT_EQ(1, e.status);
    }
}


