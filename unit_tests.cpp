#include "docopt.h"
#include <gtest/gtest.h>

#include <vector>
#include <string>
#include <map>
#include <climits>

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

TEST(ValueTest, KindAndConstructors) {
    Value v_empty;
    EXPECT_EQ(Value::KIND_EMPTY, v_empty.kind());
    EXPECT_TRUE(v_empty.is_empty());

    Value v_bool(true);
    EXPECT_EQ(Value::KIND_BOOL, v_bool.kind());
    EXPECT_TRUE(v_bool.is<bool>());
    EXPECT_TRUE(v_bool.isBool());

    Value v_long(42L);
    EXPECT_EQ(Value::KIND_LONG, v_long.kind());
    EXPECT_TRUE(v_long.is<long>());
    EXPECT_TRUE(v_long.isLong());

    Value v_int(100);
    EXPECT_EQ(Value::KIND_LONG, v_int.kind());
    EXPECT_TRUE(v_int.is<long>());
    EXPECT_TRUE(v_int.is<int>());
    EXPECT_TRUE(v_int.isLong());
    EXPECT_EQ(100L, v_int.as<long>());

    std::string s = "hello";
    Value v_str(s);
    EXPECT_EQ(Value::KIND_STRING, v_str.kind());
    EXPECT_TRUE(v_str.is<std::string>());
    EXPECT_TRUE(v_str.isString());

    Value v_cstr("world");
    EXPECT_EQ(Value::KIND_STRING, v_cstr.kind());
    EXPECT_TRUE(v_cstr.is<std::string>());
    EXPECT_TRUE(v_cstr.isString());

    std::vector<std::string> list;
    list.push_back("a");
    list.push_back("b");
    Value v_list(list);
    EXPECT_EQ(Value::KIND_STRING_LIST, v_list.kind());
    EXPECT_TRUE(v_list.is_string_list());
    EXPECT_TRUE(v_list.isStringList());
}

TEST(ValueTest, EmptyValue) {
    Value v;
    EXPECT_TRUE(v.is_empty());
    EXPECT_FALSE(v.is<bool>());
    EXPECT_FALSE(v.is<long>());
    EXPECT_FALSE(v.is<std::string>());
    EXPECT_FALSE(v.is_string_list());
    EXPECT_EQ("null", v.to_json());
}

TEST(ValueTest, BoolValue) {
    Value v_true(true);
    Value v_false(false);

    EXPECT_TRUE(v_true.is<bool>());
    EXPECT_TRUE(v_true.as<bool>());
    EXPECT_TRUE(v_true.asBool());
    EXPECT_EQ("true", v_true.to_json());

    EXPECT_TRUE(v_false.is<bool>());
    EXPECT_FALSE(v_false.as<bool>());
    EXPECT_FALSE(v_false.asBool());
    EXPECT_EQ("false", v_false.to_json());
}

TEST(ValueTest, LongValue) {
    Value v(42L);
    EXPECT_TRUE(v.is<long>());
    EXPECT_EQ(42L, v.as<long>());
    EXPECT_EQ(42L, v.asLong());
    EXPECT_EQ("42", v.to_json());
}

TEST(ValueTest, StringValue) {
    Value v("hello world");
    EXPECT_TRUE(v.is<std::string>());
    EXPECT_EQ("hello world", v.as<std::string>());
    EXPECT_EQ("hello world", v.asString());
    EXPECT_EQ("\"hello world\"", v.to_json());
}

TEST(ValueTest, StringListValue) {
    std::vector<std::string> list;
    list.push_back("foo");
    list.push_back("bar");
    Value v(list);

    EXPECT_TRUE(v.is_string_list());
    EXPECT_EQ(2u, v.as_string_list().size());
    EXPECT_EQ(2u, v.asStringList().size());
    EXPECT_EQ("foo", v.as_string_list()[0]);
    EXPECT_EQ("bar", v.as_string_list()[1]);
    EXPECT_EQ("[\"foo\", \"bar\"]", v.to_json());
}

TEST(ValueTest, DoubleValue) {
    Value v_long(42L);
    EXPECT_DOUBLE_EQ(42.0, v_long.as<double>());
    EXPECT_DOUBLE_EQ(42.0, v_long.asDouble());

    Value v_str("3.14159");
    EXPECT_DOUBLE_EQ(3.14159, v_str.as<double>());
    EXPECT_DOUBLE_EQ(3.14159, v_str.asDouble());

    Value v_empty;
    EXPECT_DOUBLE_EQ(1.5, v_empty.as_or(1.5));
}

TEST(ValueTest, FallbackAccessors) {
    Value empty_val;
    Value str_val("100");
    Value str_invalid("not_number");
    Value bool_val(true);
    Value long_val(42L);

    // as_or with strings
    EXPECT_EQ("default", empty_val.as_or("default"));
    EXPECT_EQ("default", empty_val.as_or<std::string>("default"));
    EXPECT_EQ("100", str_val.as_or("default"));
    EXPECT_EQ("100", str_val.as_or<std::string>("default"));
    EXPECT_EQ("42", long_val.as_or<std::string>("default"));
    EXPECT_EQ("true", bool_val.as_or<std::string>("default"));
    EXPECT_EQ("false", Value(false).as_or<std::string>("default"));

    // as_or with longs
    EXPECT_EQ(100L, str_val.as_or(0L));
    EXPECT_EQ(50L, empty_val.as_or(50L));
    EXPECT_EQ(99L, str_invalid.as_or(99L));
    EXPECT_EQ(42L, long_val.as_or(0L));
    EXPECT_EQ(1L, bool_val.as_or(0L));
    EXPECT_EQ(0L, Value(false).as_or(99L));

    // as_or with doubles
    EXPECT_DOUBLE_EQ(100.0, str_val.as_or(0.0));
    EXPECT_DOUBLE_EQ(5.5, empty_val.as_or(5.5));
    EXPECT_DOUBLE_EQ(9.9, str_invalid.as_or(9.9));
    EXPECT_DOUBLE_EQ(42.0, long_val.as_or(0.0));

    // as_or with bools
    EXPECT_TRUE(bool_val.as_or(false));
    EXPECT_TRUE(empty_val.as_or(true));
    EXPECT_FALSE(empty_val.as_or(false));
    EXPECT_TRUE(Value("true").as_or(false));
    EXPECT_FALSE(str_val.as_or(false));
    EXPECT_TRUE(str_val.as_or(true));
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

    std::vector<std::string> str_list = v.as_list<std::string>();
    ASSERT_EQ(3u, str_list.size());
    EXPECT_EQ("10", str_list[0]);
    EXPECT_EQ("20", str_list[1]);
    EXPECT_EQ("30", str_list[2]);

    // Single string to list conversion
    Value v_single("42");
    std::vector<int> single_int_list = v_single.as_list<int>();
    ASSERT_EQ(1u, single_int_list.size());
    EXPECT_EQ(42, single_int_list[0]);

    // Boolean list conversion
    std::vector<std::string> bool_str_list;
    bool_str_list.push_back("true");
    bool_str_list.push_back("false");
    Value v_bool_list(bool_str_list);
    std::vector<bool> bool_list = v_bool_list.as_list<bool>();
    ASSERT_EQ(2u, bool_list.size());
    EXPECT_TRUE(bool_list[0]);
    EXPECT_FALSE(bool_list[1]);

    // Empty to list conversion
    Value v_empty;
    std::vector<int> empty_list = v_empty.as_list<int>();
    EXPECT_TRUE(empty_list.empty());
}

TEST(ValueTest, SafeBoolConversion) {
    Value empty_val;
    Value bool_true(true);
    Value bool_false(false);
    Value long_val(42L);
    Value long_zero(0L);
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
    EXPECT_FALSE(long_zero);
    EXPECT_TRUE(str_val);
    EXPECT_FALSE(empty_str);
    EXPECT_TRUE(vec_val);
    EXPECT_FALSE(empty_vec_val);

    EXPECT_TRUE(empty_val.is_empty());
    EXPECT_FALSE(empty_val.is_truthy());
    EXPECT_TRUE(bool_true.is_truthy());
    EXPECT_FALSE(bool_false.is_truthy());
    EXPECT_TRUE(long_val.is_truthy());
    EXPECT_FALSE(long_zero.is_truthy());
    EXPECT_TRUE(str_val.is_truthy());
    EXPECT_FALSE(empty_str.is_truthy());
    EXPECT_TRUE(vec_val.is_truthy());
    EXPECT_FALSE(empty_vec_val.is_truthy());

    EXPECT_TRUE(!empty_val);
    EXPECT_FALSE(!str_val);
}

TEST(ValueTest, StreamOutputOperator) {
    std::stringstream ss;
    ss << Value();
    EXPECT_EQ("null", ss.str());

    ss.str("");
    ss << Value(true);
    EXPECT_EQ("true", ss.str());

    ss.str("");
    ss << Value(false);
    EXPECT_EQ("false", ss.str());

    ss.str("");
    ss << Value(42L);
    EXPECT_EQ("42", ss.str());

    ss.str("");
    ss << Value("hello");
    EXPECT_EQ("\"hello\"", ss.str());

    std::vector<std::string> list;
    list.push_back("foo");
    list.push_back("bar");
    ss.str("");
    ss << Value(list);
    EXPECT_EQ("[\"foo\", \"bar\"]", ss.str());
}

TEST(ValueTest, EqualityAndComparison) {
    Value v1(10L);
    Value v2(10L);
    Value v3(20L);

    EXPECT_EQ(v1, v2);
    EXPECT_NE(v1, v3);
    EXPECT_LT(v1, v3);

    Value s1("abc");
    Value s2("abc");
    Value s3("def");
    EXPECT_EQ(s1, s2);
    EXPECT_NE(s1, s3);
    EXPECT_LT(s1, s3);

    Value b1(false);
    Value b2(true);
    EXPECT_NE(b1, b2);
    EXPECT_LT(b1, b2);

    Value empty1;
    Value empty2;
    EXPECT_EQ(empty1, empty2);
    EXPECT_LT(empty1, v1);
}

// Reproduction test cases demonstrating accessor & conversion defects
TEST(ValueTest, StringToLongConversion) {
    Value v("12345");
    EXPECT_TRUE(v.is<std::string>());
    EXPECT_EQ(12345L, v.as<long>());
    EXPECT_EQ(12345L, v.asLong());

    Value v_neg("-99");
    EXPECT_EQ(-99L, v_neg.as<long>());

    Value v_invalid("not_a_number");
    EXPECT_THROW(v_invalid.as<long>(), boost::bad_lexical_cast);

    Value v_bool(true);
    EXPECT_EQ(1L, v_bool.as<long>());
}

TEST(ValueTest, ParsedArgumentAsLong) {
    const std::string doc =
        "Usage:\n"
        "  prog --port=<port> <count>\n";

    char const* argv[] = { "--port=8080", "5" };
    Options opts = docoptcpp03::docopt_parse(doc, make_args(argv, 2));

    EXPECT_EQ(8080L, opts["--port"].as<long>());
    EXPECT_EQ(5L, opts["<count>"].as<long>());
}

TEST(ValueTest, BoolConversionFromOtherTypes) {
    Value v_true_str("true");
    EXPECT_TRUE(v_true_str.as<bool>());

    Value v_false_str("false");
    EXPECT_FALSE(v_false_str.as<bool>());

    Value v_long_one(1L);
    EXPECT_TRUE(v_long_one.as<bool>());

    Value v_long_zero(0L);
    EXPECT_FALSE(v_long_zero.as<bool>());

    Value v_true_ws("  true  ");
    EXPECT_TRUE(v_true_ws.as<bool>());

    Value v_false_ws(" \tfalse\n ");
    EXPECT_FALSE(v_false_ws.as<bool>());

    Value v_one_ws(" 1 ");
    EXPECT_TRUE(v_one_ws.as<bool>());
}

TEST(ValueTest, StringConversionFromOtherTypes) {
    Value v_long(42L);
    EXPECT_EQ("42", v_long.as<std::string>());

    Value v_bool(true);
    EXPECT_EQ("true", v_bool.as<std::string>());
}

TEST(ValueTest, StringListConversionFromSingleString) {
    Value v_single("hello");
    std::vector<std::string> list = v_single.as_string_list();
    ASSERT_EQ(1u, list.size());
    EXPECT_EQ("hello", list[0]);
}

TEST(ValueTest, DoubleConversionVerification) {
    Value v_str("3.14");
    EXPECT_DOUBLE_EQ(3.14, v_str.as<double>());

    Value v_long(42L);
    EXPECT_DOUBLE_EQ(42.0, v_long.as<double>());

    Value v_bool(true);
    EXPECT_DOUBLE_EQ(1.0, v_bool.as<double>());

    Value v_invalid("abc");
    EXPECT_THROW(v_invalid.as<double>(), boost::bad_lexical_cast);
}

// 1. is<T>() および C++11 is*() の全型網羅・相互排他テスト
TEST(ValueTest, IsTypeQueryExhaustive) {
    Value v_empty;
    Value v_bool(true);
    Value v_long(42L);
    Value v_int(100);
    Value v_str("hello");
    std::vector<std::string> list;
    list.push_back("a");
    Value v_list(list);

    // v_bool
    EXPECT_TRUE(v_bool.is<bool>());
    EXPECT_FALSE(v_bool.is<long>());
    EXPECT_FALSE(v_bool.is<int>());
    EXPECT_FALSE(v_bool.is<std::string>());
    EXPECT_FALSE(v_bool.is_string_list());
    EXPECT_FALSE(v_bool.is_empty());
    EXPECT_TRUE(v_bool.isBool());
    EXPECT_FALSE(v_bool.isLong());
    EXPECT_FALSE(v_bool.isString());
    EXPECT_FALSE(v_bool.isStringList());

    // v_long
    EXPECT_FALSE(v_long.is<bool>());
    EXPECT_TRUE(v_long.is<long>());
    EXPECT_TRUE(v_long.is<int>());
    EXPECT_FALSE(v_long.is<std::string>());
    EXPECT_FALSE(v_long.is_string_list());
    EXPECT_FALSE(v_long.is_empty());
    EXPECT_FALSE(v_long.isBool());
    EXPECT_TRUE(v_long.isLong());
    EXPECT_FALSE(v_long.isString());
    EXPECT_FALSE(v_long.isStringList());

    // v_int
    EXPECT_FALSE(v_int.is<bool>());
    EXPECT_TRUE(v_int.is<long>());
    EXPECT_TRUE(v_int.is<int>());
    EXPECT_FALSE(v_int.is<std::string>());
    EXPECT_FALSE(v_int.is_string_list());
    EXPECT_FALSE(v_int.is_empty());

    // v_str
    EXPECT_FALSE(v_str.is<bool>());
    EXPECT_FALSE(v_str.is<long>());
    EXPECT_FALSE(v_str.is<int>());
    EXPECT_TRUE(v_str.is<std::string>());
    EXPECT_FALSE(v_str.is_string_list());
    EXPECT_FALSE(v_str.is_empty());
    EXPECT_FALSE(v_str.isBool());
    EXPECT_FALSE(v_str.isLong());
    EXPECT_TRUE(v_str.isString());
    EXPECT_FALSE(v_str.isStringList());

    // v_list
    EXPECT_FALSE(v_list.is<bool>());
    EXPECT_FALSE(v_list.is<long>());
    EXPECT_FALSE(v_list.is<int>());
    EXPECT_FALSE(v_list.is<std::string>());
    EXPECT_TRUE(v_list.is_string_list());
    EXPECT_FALSE(v_list.is_empty());
    EXPECT_FALSE(v_list.isBool());
    EXPECT_FALSE(v_list.isLong());
    EXPECT_FALSE(v_list.isString());
    EXPECT_TRUE(v_list.isStringList());

    // v_empty
    EXPECT_FALSE(v_empty.is<bool>());
    EXPECT_FALSE(v_empty.is<long>());
    EXPECT_FALSE(v_empty.is<int>());
    EXPECT_FALSE(v_empty.is<std::string>());
    EXPECT_FALSE(v_empty.is_string_list());
    EXPECT_TRUE(v_empty.is_empty());
    EXPECT_FALSE(v_empty.isBool());
    EXPECT_FALSE(v_empty.isLong());
    EXPECT_FALSE(v_empty.isString());
    EXPECT_FALSE(v_empty.isStringList());
}

// 2. as<T>() の例外パス網羅テスト
TEST(ValueTest, AsUnsupportedConversionsThrow) {
    Value empty_val;
    std::vector<std::string> l;
    l.push_back("x");
    Value list_val(l);

    EXPECT_THROW(empty_val.as<bool>(), boost::bad_lexical_cast);
    EXPECT_THROW(empty_val.as<long>(), boost::bad_lexical_cast);
    EXPECT_THROW(empty_val.as<int>(), boost::bad_lexical_cast);
    EXPECT_THROW(empty_val.as<double>(), boost::bad_lexical_cast);
    EXPECT_THROW(empty_val.as<std::string>(), boost::bad_lexical_cast);

    EXPECT_THROW(list_val.as<bool>(), boost::bad_lexical_cast);
    EXPECT_THROW(list_val.as<long>(), boost::bad_lexical_cast);
    EXPECT_THROW(list_val.as<int>(), boost::bad_lexical_cast);
    EXPECT_THROW(list_val.as<double>(), boost::bad_lexical_cast);
    EXPECT_THROW(list_val.as<std::string>(), boost::bad_lexical_cast);
}

// 3. as<bool>() 文字列トークン解釈の網羅テスト
TEST(ValueTest, BoolStringVariantsExhaustive) {
    EXPECT_TRUE(Value("yes").as<bool>());
    EXPECT_TRUE(Value("on").as<bool>());
    EXPECT_TRUE(Value("YES").as<bool>());
    EXPECT_TRUE(Value("ON").as<bool>());
    EXPECT_TRUE(Value("True").as<bool>());

    EXPECT_FALSE(Value("no").as<bool>());
    EXPECT_FALSE(Value("off").as<bool>());
    EXPECT_FALSE(Value("NO").as<bool>());
    EXPECT_FALSE(Value("OFF").as<bool>());
    EXPECT_FALSE(Value("False").as<bool>());
    EXPECT_FALSE(Value("").as<bool>());

    EXPECT_THROW(Value("maybe").as<bool>(), boost::bad_lexical_cast);
}

// 4. as_list<T>() 要素変換失敗例外テスト
TEST(ValueTest, AsListElementConversionError) {
    std::vector<std::string> invalid_list;
    invalid_list.push_back("10");
    invalid_list.push_back("not_int");
    Value v(invalid_list);
    EXPECT_THROW(v.as_list<int>(), boost::bad_lexical_cast);
}

// 5. 文字列リストの比較およびキャッシュテスト
TEST(ValueTest, StringListComparisonAndCache) {
    std::vector<std::string> l1;
    l1.push_back("a");
    std::vector<std::string> l2;
    l2.push_back("b");
    Value vl1(l1);
    Value vl2(l2);

    EXPECT_EQ(vl1, vl1);
    EXPECT_NE(vl1, vl2);
    EXPECT_LT(vl1, vl2);
    EXPECT_FALSE(Value() < Value());

    Value v_str("cached");
    const std::vector<std::string>& ref1 = v_str.as_string_list();
    const std::vector<std::string>& ref2 = v_str.as_string_list();
    EXPECT_EQ(&ref1, &ref2);
    EXPECT_EQ(1u, ref1.size());
    EXPECT_EQ("cached", ref1[0]);
}

TEST(ValueTest, ValueAsIntOptimization) {
    // 1. KIND_LONG to int conversion
    Value v_zero(0L);
    EXPECT_EQ(0, v_zero.as<int>());

    Value v_pos(12345L);
    EXPECT_EQ(12345, v_pos.as<int>());

    Value v_neg(-6789L);
    EXPECT_EQ(-6789, v_neg.as<int>());

    // Boundary values INT_MAX, INT_MIN
    Value v_max(static_cast<long>(INT_MAX));
    EXPECT_EQ(INT_MAX, v_max.as<int>());

    Value v_min(static_cast<long>(INT_MIN));
    EXPECT_EQ(INT_MIN, v_min.as<int>());

    // Overflow & underflow detection if sizeof(long) > sizeof(int)
    if (sizeof(long) > sizeof(int)) {
        Value v_overflow(static_cast<long>(INT_MAX) + 1L);
        EXPECT_THROW(v_overflow.as<int>(), boost::bad_lexical_cast);

        Value v_underflow(static_cast<long>(INT_MIN) - 1L);
        EXPECT_THROW(v_underflow.as<int>(), boost::bad_lexical_cast);

        // as_or<int> with overflow fallback
        EXPECT_EQ(999, v_overflow.as_or<int>(999));
        EXPECT_EQ(-999, v_underflow.as_or<int>(-999));
    }

    // 2. KIND_STRING to int conversion
    Value v_str("42");
    EXPECT_EQ(42, v_str.as<int>());

    Value v_str_invalid("not_an_int");
    EXPECT_THROW(v_str_invalid.as<int>(), boost::bad_lexical_cast);

    // 3. KIND_BOOL to int conversion
    Value v_true(true);
    EXPECT_EQ(1, v_true.as<int>());

    Value v_false(false);
    EXPECT_EQ(0, v_false.as<int>());

    // 4. Unsupported kinds throw boost::bad_lexical_cast
    Value v_empty;
    EXPECT_THROW(v_empty.as<int>(), boost::bad_lexical_cast);

    std::vector<std::string> list;
    list.push_back("1");
    Value v_list(list);
    EXPECT_THROW(v_list.as<int>(), boost::bad_lexical_cast);
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
    EXPECT_TRUE(result["<name>"].is<std::string>());
    EXPECT_EQ("John", result["<name>"].as<std::string>());
}

TEST(BasicUsageTest, MultiplePositionalArguments) {
    const std::string doc = "Usage: prog <src> <dst>";
    char const* argv_raw[] = { "/path/src", "/path/dst" };
    std::vector<std::string> argv = make_args(argv_raw, 2);

    Options result = docoptcpp03::docopt_parse(doc, argv);
    EXPECT_EQ("/path/src", result["<src>"].as<std::string>());
    EXPECT_EQ("/path/dst", result["<dst>"].as<std::string>());
}

TEST(BasicUsageTest, ArgcArgvOverload) {
    const std::string doc = "Usage: my_prog <name> [--verbose]";
    char arg0[] = "Alice";
    char arg1[] = "--verbose";
    char* argv[] = { arg0, arg1 };
    int argc = 2;

    Options result = docoptcpp03::docopt_parse(doc, argc, argv);
    EXPECT_EQ("Alice", result["<name>"].as<std::string>());
    EXPECT_TRUE(result["--verbose"].as<bool>());
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
    EXPECT_TRUE(res1["-a"].as<bool>());

    char const* argv_empty[] = {};
    Options res2 = docoptcpp03::docopt_parse(doc, make_args(argv_empty, 0));
    EXPECT_FALSE(res2["-a"].as<bool>());
}

TEST(OptionParsingTest, LongOptionWithEquals) {
    const std::string doc =
        "Usage: prog [--output=<file>]\n"
        "Options: --output=<file>  Output file.";

    char const* argv[] = { "--output=out.txt" };
    Options res = docoptcpp03::docopt_parse(doc, make_args(argv, 1));
    EXPECT_EQ("out.txt", res["--output"].as<std::string>());
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
    EXPECT_TRUE(res["-a"].as<bool>());
    EXPECT_FALSE(res["-b"].as<bool>());
    EXPECT_TRUE(res["-c"].as<bool>());
}

//------------------------------------------------------------------------------
// 5. Flag Counting Tests
//------------------------------------------------------------------------------

TEST(FlagCountingTest, RepeatableFlags) {
    const std::string doc = "Usage: prog [-v...]";

    char const* argv_v1[] = { "-v" };
    Options res1 = docoptcpp03::docopt_parse(doc, make_args(argv_v1, 1));
    EXPECT_EQ(1L, res1["-v"].as<long>());

    char const* argv_v3[] = { "-vvv" };
    Options res3 = docoptcpp03::docopt_parse(doc, make_args(argv_v3, 1));
    EXPECT_EQ(3L, res3["-v"].as<long>());
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
    EXPECT_TRUE(res1["ship"].as<bool>());
    EXPECT_TRUE(res1["new"].as<bool>());
    EXPECT_EQ("Enterprise", res1["<name>"].as<std::string>());

    char const* argv_move[] = { "ship", "move", "100", "200" };
    Options res2 = docoptcpp03::docopt_parse(doc, make_args(argv_move, 4));
    EXPECT_TRUE(res2["ship"].as<bool>());
    EXPECT_TRUE(res2["move"].as<bool>());
    EXPECT_FALSE(res2["new"].as<bool>());
    EXPECT_EQ("100", res2["<x>"].as<std::string>());
    EXPECT_EQ("200", res2["<y>"].as<std::string>());
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
    EXPECT_EQ("10", res1["--speed"].as<std::string>());

    char const* argv_custom[] = { "--speed=25" };
    Options res2 = docoptcpp03::docopt_parse(doc, make_args(argv_custom, 1));
    EXPECT_EQ("25", res2["--speed"].as<std::string>());
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
    EXPECT_TRUE(res["--verbose"].as<bool>());
    EXPECT_EQ("stdout", res["--output"].as<std::string>());
    EXPECT_EQ("input.txt", res["<file>"].as<std::string>());
}

TEST(OptionsTest, ConstOptionsAccess) {
    const std::string doc = "Usage: prog [--foo=<bar>]";
    char const* argv[] = { "--foo=baz" };

    const Options opts = docoptcpp03::docopt_parse(doc, make_args(argv, 1));

    // const Options operator[] access
    EXPECT_EQ("baz", opts["--foo"].as<std::string>());
    EXPECT_TRUE(opts["--nonexistent"].is_empty());

    // const Options at() access
    EXPECT_EQ("baz", opts.at("--foo").as<std::string>());
    EXPECT_THROW(opts.at("--nonexistent"), std::out_of_range);
}

TEST(ValueTest, ValueDefaultAccessors) {
    Value empty_val;
    Value str_val("100");
    Value bool_val(true);

    EXPECT_EQ("default", empty_val.as_or("default"));
    EXPECT_EQ("100", str_val.as_or("default"));

    EXPECT_EQ(100L, str_val.as_or(0L));
    EXPECT_EQ(50L, empty_val.as_or(50L));

    EXPECT_TRUE(bool_val.as_or(false));
    EXPECT_TRUE(empty_val.as_or(true));
    EXPECT_FALSE(empty_val.as_or(false));
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

TEST(ValueTest, ValueAsTemplateComprehensive) {
    // 1. as<int> and as<long>
    Value v_str_int("42");
    EXPECT_EQ(42, v_str_int.as<int>());
    EXPECT_EQ(42L, v_str_int.as<long>());

    Value v_long(100L);
    EXPECT_EQ(100, v_long.as<int>());
    EXPECT_EQ(100L, v_long.as<long>());

    Value v_bool_true(true);
    EXPECT_EQ(1, v_bool_true.as<int>());
    EXPECT_EQ(1L, v_bool_true.as<long>());

    Value v_invalid("abc");
    EXPECT_THROW(v_invalid.as<int>(), boost::bad_lexical_cast);

    Value v_empty;
    EXPECT_THROW(v_empty.as<int>(), boost::bad_lexical_cast);

    // 2. as<double>
    Value v_str_double("3.14");
    EXPECT_DOUBLE_EQ(3.14, v_str_double.as<double>());
    EXPECT_DOUBLE_EQ(100.0, v_long.as<double>());
    EXPECT_DOUBLE_EQ(1.0, v_bool_true.as<double>());

    // 3. as<std::string>
    EXPECT_EQ("42", v_str_int.as<std::string>());
    EXPECT_EQ("100", v_long.as<std::string>());
    EXPECT_EQ("true", v_bool_true.as<std::string>());

    // 4. as<bool>
    Value v_str_1("1");
    EXPECT_TRUE(v_str_1.as<bool>());

    Value v_str_0("0");
    EXPECT_FALSE(v_str_0.as<bool>());

    Value v_str_true("true");
    EXPECT_TRUE(v_str_true.as<bool>());

    Value v_str_false("false");
    EXPECT_FALSE(v_str_false.as<bool>());

    Value v_long_non_one(42L);
    EXPECT_TRUE(v_long_non_one.as<bool>());

    Value v_long_zero(0L);
    EXPECT_FALSE(v_long_zero.as<bool>());
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

TEST(OptionsTest, GetTemplateVariants) {
    const std::string doc = "Usage: prog [--speed=<kn>] [--count=<n>] [--verbose]";
    char const* argv[] = { "--speed=2.5", "--count=42", "--verbose" };
    const Options opts = docoptcpp03::docopt_parse(doc, make_args(argv, 3));

    EXPECT_EQ(42L, opts.get<long>("--count", 0L));
    EXPECT_DOUBLE_EQ(2.5, opts.get<double>("--speed", 0.0));
    EXPECT_TRUE(opts.get<bool>("--verbose", false));
    EXPECT_FALSE(opts.get<bool>("--missing", false));
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

TEST(ValueTest, EmptyStringArgument) {
    const std::string doc =
        "Usage: prog --option <FILE>\n"
        "\n"
        "Options:\n"
        "  --option=<FILE>   The file option.\n";

    char const* argv[] = { "--option", "" };
    Options res = docoptcpp03::docopt_parse(doc, make_args(argv, 2));
    EXPECT_EQ("", res["--option"].as<std::string>());
    EXPECT_FALSE(res["--option"].is_empty());
    EXPECT_TRUE(res["--option"].is<std::string>());
}

TEST(OptionsTest, CustomSectionNamesPartialMatch) {
    const std::string doc =
        "Usage:\n"
        "  program [options]\n"
        "\n"
        "Standard Options:\n"
        "  --verbose      Enable verbose output.\n"
        "\n"
        "Advanced Options:\n"
        "  --speed=<kn>   Set speed [default: 10].\n";

    // Test case 1: Defaults
    {
        char const* argv[] = {};
        Options res = docoptcpp03::docopt_parse(doc, make_args(argv, 0));
        EXPECT_FALSE(res["--verbose"].as<bool>());
        EXPECT_EQ("10", res["--speed"].as<std::string>());
    }

    // Test case 2: Provided values
    {
        char const* argv[] = { "--speed=20", "--verbose" };
        Options res = docoptcpp03::docopt_parse(doc, make_args(argv, 2));
        EXPECT_TRUE(res["--verbose"].as<bool>());
        EXPECT_EQ("20", res["--speed"].as<std::string>());
    }
}

TEST(OptionsTest, UnrecognizedSectionsIgnored) {
    const std::string doc =
        "Usage:\n"
        "  program --foo\n"
        "\n"
        "Description:\n"
        "  This is a description. It should be ignored by the parser.\n"
        "  Even if we write --bar here, it should not be parsed as an option.\n"
        "\n"
        "Examples:\n"
        "  program --foo\n";

    // Test case: Parsing should succeed with --foo and ignore other sections
    {
        char const* argv[] = { "--foo" };
        Options res = docoptcpp03::docopt_parse(doc, make_args(argv, 1));
        EXPECT_TRUE(res["--foo"].as<bool>());
        EXPECT_TRUE(res["--bar"].is_empty()); // --bar is ignored and should be empty
    }
}

TEST(OptionsTest, MultilineUsageAndOptions) {
    const std::string doc =
        "Usage:\n"
        "  program ship move <x> <y>\n"
        "                 [--speed=<kn>]\n"
        "\n"
        "Options:\n"
        "  -s, --speed=<kn>  Set the speed of the ship.\n"
        "                    This description is on a new line.\n"
        "                    [default: 10]\n";

    // Test case 1: Defaults
    {
        char const* argv[] = { "ship", "move", "1", "2" };
        Options res = docoptcpp03::docopt_parse(doc, make_args(argv, 4));
        EXPECT_EQ("10", res["--speed"].as<std::string>());
        EXPECT_EQ("1", res["<x>"].as<std::string>());
        EXPECT_EQ("2", res["<y>"].as<std::string>());
    }

    // Test case 2: Provided values
    {
        char const* argv[] = { "ship", "move", "1", "2", "--speed=25" };
        Options res = docoptcpp03::docopt_parse(doc, make_args(argv, 5));
        EXPECT_EQ("25", res["--speed"].as<std::string>());
    }
}

//------------------------------------------------------------------------------
// 8. SharedPtr Memory Safety & UAF Tests
//------------------------------------------------------------------------------

#include "docopt_private.h"

struct TestNode {
    mutable unsigned int ref_count;
    docoptcpp03::detail::shared_ptr<TestNode> child;
    int id;

    TestNode(int i = 0) : ref_count(0), child(), id(i) {}
    ~TestNode() {
        id = -999;
    }

    void add_ref() const { ++ref_count; }
    void release_ref() const {
        if (--ref_count == 0) {
            delete this;
        }
    }
};

TEST(SharedPtrTest, SelfAssignment) {
    using docoptcpp03::detail::shared_ptr;
    shared_ptr<TestNode> p(new TestNode(42));
    p = p;
    EXPECT_TRUE(p);
    EXPECT_EQ(42, p->id);
}

TEST(SharedPtrTest, NullAssignment) {
    using docoptcpp03::detail::shared_ptr;
    shared_ptr<TestNode> p1;
    shared_ptr<TestNode> p2;
    p1 = p2;
    EXPECT_FALSE(p1);

    shared_ptr<TestNode> p3(new TestNode(10));
    p3 = p1;
    EXPECT_FALSE(p3);
}

TEST(SharedPtrTest, CascadingDestructionUseAfterFree) {
    using docoptcpp03::detail::shared_ptr;

    // Node A (id 1) holds the only reference to Node B (id 2).
    // Pointer p holds the only reference to Node A.
    shared_ptr<TestNode> p(new TestNode(1));
    p->child = shared_ptr<TestNode>(new TestNode(2));

    // When assigning p = p->child:
    // In unpatched operator=:
    // 1. px->release_ref() deletes Node A.
    // 2. Node A's destructor deletes Node B.
    // 3. px = r.px points to deleted Node B.
    // 4. px->add_ref() accesses freed heap memory -> Heap Use-After-Free!
    p = p->child;

    ASSERT_TRUE(p);
    EXPECT_EQ(2, p->id);
}

//------------------------------------------------------------------------------
// 9. Boundary Checks & Empty Token Handling Tests
//------------------------------------------------------------------------------

TEST(BoundaryCheckTest, EmptyStringPositionalArgument) {
    const std::string doc =
        "Usage:\n"
        "  program [<arg>]\n";

    char const* argv[] = { "" };
    Options res = docoptcpp03::docopt_parse(doc, make_args(argv, 1));
    EXPECT_TRUE(res["<arg>"].is<std::string>());
    EXPECT_EQ("", res["<arg>"].as<std::string>());
}

TEST(BoundaryCheckTest, EmptyStringWithMultipleArgs) {
    const std::string doc =
        "Usage:\n"
        "  program <first> <second>\n";

    char const* argv[] = { "", "val" };
    Options res = docoptcpp03::docopt_parse(doc, make_args(argv, 2));
    EXPECT_EQ("", res["<first>"].as<std::string>());
    EXPECT_EQ("val", res["<second>"].as<std::string>());
}

TEST(BoundaryCheckTest, EmptyStringOptionValue) {
    const std::string doc =
        "Usage:\n"
        "  program [--name=<val>]\n"
        "\n"
        "Options:\n"
        "  --name=<val>  Set name.\n";

    char const* argv[] = { "--name=" };
    Options res = docoptcpp03::docopt_parse(doc, make_args(argv, 1));
    EXPECT_TRUE(res["--name"].is<std::string>());
    EXPECT_EQ("", res["--name"].as<std::string>());
}

//------------------------------------------------------------------------------
// 10. OptionsFirst Parameter Tests
//------------------------------------------------------------------------------

TEST(OptionsFirstTest, StopsOptionParsingAfterFirstPositional) {
    const std::string doc =
        "Usage:\n"
        "  prog [options] <cmd> [<args>...]\n"
        "\n"
        "Options:\n"
        "  -v             Verbose output.\n"
        "  -a, --all      Process all.\n";

    char const* argv[] = { "cmd", "-v", "--all", "file.txt" };
    Options opts = docoptcpp03::docopt_parse(doc, make_args(argv, 4), true, "", true);

    EXPECT_FALSE(opts["-v"].as<bool>());
    EXPECT_FALSE(opts["--all"].as<bool>());
    EXPECT_EQ("cmd", opts["<cmd>"].as<std::string>());
    std::vector<std::string> args = opts["<args>"].as_string_list();
    ASSERT_EQ(3u, args.size());
    EXPECT_EQ("-v", args[0]);
    EXPECT_EQ("--all", args[1]);
    EXPECT_EQ("file.txt", args[2]);
}

TEST(OptionsFirstTest, ParsesOptionsBeforeFirstPositional) {
    const std::string doc =
        "Usage:\n"
        "  prog [options] <cmd> [<args>...]\n"
        "\n"
        "Options:\n"
        "  -v             Verbose output.\n"
        "  -a, --all      Process all.\n";

    char const* argv[] = { "-v", "cmd", "--all", "file.txt" };
    Options opts = docoptcpp03::docopt_parse(doc, make_args(argv, 4), true, "", true);

    EXPECT_TRUE(opts["-v"].as<bool>());
    EXPECT_FALSE(opts["--all"].as<bool>());
    EXPECT_EQ("cmd", opts["<cmd>"].as<std::string>());
    std::vector<std::string> args = opts["<args>"].as_string_list();
    ASSERT_EQ(2u, args.size());
    EXPECT_EQ("--all", args[0]);
    EXPECT_EQ("file.txt", args[1]);
}

TEST(OptionsFirstTest, ComparisonWithDefaultOptionsFirstFalse) {
    const std::string doc =
        "Usage:\n"
        "  prog [options] <cmd> [<args>...]\n"
        "\n"
        "Options:\n"
        "  -v  Verbose output.\n";

    char const* argv[] = { "cmd", "-v" };

    // options_first = false (default)
    Options opts_default = docoptcpp03::docopt_parse(doc, make_args(argv, 2));
    EXPECT_TRUE(opts_default["-v"].as<bool>());
    EXPECT_EQ("cmd", opts_default["<cmd>"].as<std::string>());
    EXPECT_TRUE(opts_default["<args>"].as_string_list().empty());

    // options_first = true
    Options opts_first = docoptcpp03::docopt_parse(doc, make_args(argv, 2), true, "", true);
    EXPECT_FALSE(opts_first["-v"].as<bool>());
    EXPECT_EQ("cmd", opts_first["<cmd>"].as<std::string>());
    std::vector<std::string> args = opts_first["<args>"].as_string_list();
    ASSERT_EQ(1u, args.size());
    EXPECT_EQ("-v", args[0]);
}

TEST(OptionsFirstTest, GitStyleSubcommandDispatch) {
    const std::string doc =
        "Usage:\n"
        "  git [--version] [--help] [-C <path>] <command> [<args>...]\n"
        "\n"
        "Options:\n"
        "  -C <path>  Run as if git was started in <path>.\n";

    char const* argv[] = { "-C", "/repo", "commit", "-m", "initial commit", "--amend" };
    Options opts = docoptcpp03::docopt_parse(doc, make_args(argv, 6), true, "", true);

    EXPECT_EQ("/repo", opts["-C"].as<std::string>());
    EXPECT_EQ("commit", opts["<command>"].as<std::string>());
    std::vector<std::string> args = opts["<args>"].as_string_list();
    ASSERT_EQ(3u, args.size());
    EXPECT_EQ("-m", args[0]);
    EXPECT_EQ("initial commit", args[1]);
    EXPECT_EQ("--amend", args[2]);
}

TEST(OptionsFirstTest, OptionsOnlyWithoutPositional) {
    const std::string doc =
        "Usage:\n"
        "  prog [options] [<cmd>]\n"
        "\n"
        "Options:\n"
        "  -v  Verbose mode.\n";

    char const* argv[] = { "-v" };
    Options opts = docoptcpp03::docopt_parse(doc, make_args(argv, 1), true, "", true);

    EXPECT_TRUE(opts["-v"].as<bool>());
    EXPECT_TRUE(opts["<cmd>"].is_empty());
}

TEST(OptionsFirstTest, CStyleArgvOverload) {
    const std::string doc =
        "Usage:\n"
        "  prog [options] <cmd> [<args>...]\n"
        "\n"
        "Options:\n"
        "  -v  Verbose output.\n";

    char const* argv[] = { "cmd", "-v" };
    Options opts = docoptcpp03::docopt_parse(doc, 2, argv, true, "", true);

    EXPECT_FALSE(opts["-v"].as<bool>());
    EXPECT_EQ("cmd", opts["<cmd>"].as<std::string>());
    ASSERT_EQ(1u, opts["<args>"].as_string_list().size());
    EXPECT_EQ("-v", opts["<args>"].as_string_list()[0]);
}

//------------------------------------------------------------------------------
// 11. Options Container API Tests
//------------------------------------------------------------------------------

TEST(OptionsContainerTest, SizeAndEmptyAndClear) {
    Options opts;
    EXPECT_TRUE(opts.empty());
    EXPECT_EQ(0u, opts.size());

    char const* argv[] = { "hello" };
    opts = docoptcpp03::docopt_parse("Usage: prog <msg>\n", make_args(argv, 1));
    EXPECT_FALSE(opts.empty());
    EXPECT_EQ(1u, opts.size());

    opts.clear();
    EXPECT_TRUE(opts.empty());
    EXPECT_EQ(0u, opts.size());
}

TEST(OptionsContainerTest, EqualityAndInequalityOperators) {
    char const* argv1[] = { "-v" };
    Options a = docoptcpp03::docopt_parse("Usage: prog [-v]\nOptions: -v", make_args(argv1, 1));
    Options b = docoptcpp03::docopt_parse("Usage: prog [-v]\nOptions: -v", make_args(argv1, 1));
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);

    char const* argv2[] = {};
    Options c = docoptcpp03::docopt_parse("Usage: prog [-v]\nOptions: -v", make_args(argv2, 0));
    EXPECT_FALSE(a == c);
    EXPECT_TRUE(a != c);

    Options empty_opts;
    EXPECT_FALSE(a == empty_opts);
    EXPECT_TRUE(a != empty_opts);
}

TEST(OptionsContainerTest, IteratorsConstAndNonConst) {
    char const* argv[] = { "foo", "--bar" };
    const std::string doc =
        "Usage: prog <name> [--bar]\n"
        "Options:\n"
        "  --bar  Bar flag.\n";
    Options opts = docoptcpp03::docopt_parse(doc, make_args(argv, 2));

    // const_iterator
    const Options& const_opts = opts;
    size_t count = 0;
    for (Options::const_iterator it = const_opts.begin(); it != const_opts.end(); ++it) {
        EXPECT_FALSE(it->first.empty());
        ++count;
    }
    EXPECT_EQ(opts.size(), count);

    // non-const iterator: modify value
    for (Options::iterator it = opts.begin(); it != opts.end(); ++it) {
        if (it->first == "<name>") {
            it->second = Value("modified");
        }
    }
    EXPECT_EQ("modified", opts["<name>"].as<std::string>());
}

TEST(OptionsContainerTest, FindAndCountAndKeyQueries) {
    char const* argv[] = { "-a" };
    const std::string doc =
        "Usage: prog [-a]\n"
        "Options:\n"
        "  -a  Alpha.\n";
    Options opts = docoptcpp03::docopt_parse(doc, make_args(argv, 1));

    // find (const)
    const Options& const_opts = opts;
    Options::const_iterator cit = const_opts.find("-a");
    ASSERT_NE(const_opts.end(), cit);
    EXPECT_TRUE(cit->second.as<bool>());
    EXPECT_EQ(const_opts.end(), const_opts.find("-nonexistent"));

    // find (non-const) & modify
    Options::iterator it = opts.find("-a");
    ASSERT_NE(opts.end(), it);
    it->second = Value(false);
    EXPECT_FALSE(opts["-a"].as<bool>());

    // count
    EXPECT_EQ(1u, opts.count("-a"));
    EXPECT_EQ(0u, opts.count("-nonexistent"));

    // has_key & contains
    EXPECT_TRUE(opts.has_key("-a"));
    EXPECT_TRUE(opts.contains("-a"));
    EXPECT_FALSE(opts.has_key("-nonexistent"));
    EXPECT_FALSE(opts.contains("-nonexistent"));
}

TEST(OptionsContainerTest, MapAccessorAndConstructor) {
    std::map<std::string, Value> raw_map;
    raw_map["key1"] = Value("value1");
    raw_map["key2"] = Value(42L);

    Options opts(raw_map);
    EXPECT_EQ(2u, opts.size());
    EXPECT_EQ("value1", opts["key1"].as<std::string>());
    EXPECT_EQ(42L, opts["key2"].as<long>());

    const std::map<std::string, Value>& retrieved_map = opts.map();
    EXPECT_EQ(2u, retrieved_map.size());
    EXPECT_EQ("value1", retrieved_map.find("key1")->second.as<std::string>());
}


