#pragma once

#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cctype>

#if defined(DOCOPT_USE_CUSTOM_LEXICAL_CAST)
#include <typeinfo>
#else
#include <boost/lexical_cast.hpp>
#endif

namespace docoptcpp03 {

//==============================================================================
// PART 1: PUBLIC INTERFACE & CLASS DECLARATIONS
// (利用者が最初に見る公開 API の宣言一覧)
//==============================================================================

//------------------------------------------------------------------------------
// Exceptions
//------------------------------------------------------------------------------

class DocoptLanguageError : public std::runtime_error {
public:
    explicit DocoptLanguageError(const std::string& message);
};

class DocoptExit : public std::runtime_error {
public:
    std::string usage;
    int status;

    explicit DocoptExit(int exit_status, const std::string& message = "", const std::string& usage_str = "");
    explicit DocoptExit(const std::string& message = "", const std::string& usage_str = "");
    ~DocoptExit() throw();
};

class DocoptExitHelp : public DocoptExit {
public:
    explicit DocoptExitHelp(const std::string& usage_str);
    ~DocoptExitHelp() throw();
};

class DocoptExitVersion : public DocoptExit {
public:
    explicit DocoptExitVersion(const std::string& version_str);
    ~DocoptExitVersion() throw();
};

class DocoptArgumentError : public DocoptExit {
public:
    explicit DocoptArgumentError(const std::string& message = "", const std::string& usage_str = "");
    ~DocoptArgumentError() throw();
};

//------------------------------------------------------------------------------
// Value Class (Dynamic variant type holding bool, long, string, list, or empty)
//------------------------------------------------------------------------------

class Value {
public:
    enum Kind {
        KIND_EMPTY,
        KIND_BOOL,
        KIND_LONG,
        KIND_STRING,
        KIND_STRING_LIST
    };

    typedef void (Value::*unspecified_bool_type)() const;

    Value();
    Value(bool b);
    Value(long l);
    Value(int i);
    Value(const std::string& s);
    Value(const char* s);
    Value(const std::vector<std::string>& v);

    Kind kind() const;

    bool is_empty() const;
    bool is_bool() const;
    bool is_long() const;
    bool is_string() const;
    bool is_string_list() const;

    bool as_bool() const;
    long as_long() const;
    double as_double() const;
    const std::string& as_string() const;
    const std::vector<std::string>& as_string_list() const;

    // docopt for C++11 value compatibility methods (CamelCase)
    bool isBool() const { return is_bool(); }
    bool isLong() const { return is_long(); }
    bool isString() const { return is_string(); }
    bool isStringList() const { return is_string_list(); }

    bool asBool() const { return as_bool(); }
    long asLong() const { return as_long(); }
    double asDouble() const { return as_double(); }
    const std::string& asString() const { return as_string(); }
    const std::vector<std::string>& asStringList() const { return as_string_list(); }

    bool as_bool_or(bool default_val) const;
    long as_long_or(long default_val) const;
    double as_double_or(double default_val) const;
    std::string as_string_or(const std::string& default_val) const;
    std::string as_string_or(const char* default_val) const;

    template <typename T>
    T as() const;

    template <typename T>
    T as_or(const T& default_val) const;

    template <typename T>
    std::vector<T> as_list() const;

    bool is_truthy() const;

    operator unspecified_bool_type() const;
    bool operator!() const;

    bool operator==(const Value& other) const;
    bool operator!=(const Value& other) const;
    bool operator<(const Value& other) const;

    std::string to_json() const;

private:
    void dummy_for_bool() const {}

    Kind kind_;
    bool bool_val_;
    long long_val_;
    mutable std::string str_val_;
    mutable std::vector<std::string> str_list_val_;
};

std::ostream& operator<<(std::ostream& os, const Value& val);

//------------------------------------------------------------------------------
// Options Class (Composition container over parsed arguments)
//------------------------------------------------------------------------------

class Options {
public:
    typedef std::map<std::string, Value>::const_iterator const_iterator;
    typedef std::map<std::string, Value>::iterator iterator;
    typedef std::map<std::string, Value>::size_type size_type;
    typedef std::map<std::string, Value>::key_type key_type;
    typedef std::map<std::string, Value>::mapped_type mapped_type;
    typedef std::map<std::string, Value>::value_type value_type;

    Options();
    Options(const std::map<std::string, Value>& other);

    Value& operator[](const std::string& key);
    const Value& operator[](const std::string& key) const;
    const Value& at(const std::string& key) const;

    const_iterator begin() const;
    const_iterator end() const;
    iterator begin();
    iterator end();

    const_iterator find(const std::string& key) const;
    iterator find(const std::string& key);
    size_type count(const std::string& key) const;
    bool has_key(const std::string& key) const;
    bool contains(const std::string& key) const;

    std::string get(const std::string& key, const std::string& default_val) const;
    std::string get(const std::string& key, const char* default_val) const;

    template <typename T>
    T get(const std::string& key, const T& default_val) const;

    size_type size() const;
    bool empty() const;
    void clear();

    bool operator==(const Options& other) const;
    bool operator!=(const Options& other) const;

    const std::map<std::string, Value>& map() const;

    void dump(std::ostream& os = std::cout) const;
    std::string dump_string() const;

private:
    std::map<std::string, Value> map_;
};

std::ostream& operator<<(std::ostream& os, const Options& opts);

//------------------------------------------------------------------------------
// Main Docopt Functions
//------------------------------------------------------------------------------

// docopt_parse: Parses arguments and throws DocoptExit (DocoptExitHelp, DocoptExitVersion, DocoptArgumentError) or DocoptLanguageError.
Options docopt_parse(
    const std::string& doc,
    const std::vector<std::string>& argv,
    bool help = true,
    const std::string& version = "",
    bool options_first = false
);

Options docopt_parse(
    const std::string& doc,
    int argc,
    char const* const argv[],
    bool help = true,
    const std::string& version = "",
    bool options_first = false
);

// docopt: Convenience function that catches DocoptExit / DocoptLanguageError, prints usage/error messages, and exits via std::exit(status).
Options docopt(
    const std::string& doc,
    const std::vector<std::string>& argv,
    bool help = true,
    const std::string& version = "",
    bool options_first = false
);

Options docopt(
    const std::string& doc,
    int argc,
    char const* const argv[],
    bool help = true,
    const std::string& version = "",
    bool options_first = false
);



} // namespace docoptcpp03


//==============================================================================
// PART 2: TEMPLATE IMPLEMENTATIONS
// (ヘッダー定義が必須なテンプレート実装)
//==============================================================================

#if defined(DOCOPT_USE_CUSTOM_LEXICAL_CAST)

namespace boost {

class bad_lexical_cast : public std::bad_cast {
public:
    virtual const char* what() const throw();
};

inline const char* bad_lexical_cast::what() const throw() {
    return "bad lexical cast: source type value could not be interpreted as target";
}

template <typename Target, typename Source>
inline Target lexical_cast(const Source& arg) {
    std::stringstream ss;
    Target result;
    if (!(ss << arg) || !(ss >> result) || !(ss >> std::ws).eof()) {
        throw bad_lexical_cast();
    }
    return result;
}

template <typename Target>
inline Target lexical_cast(const std::string& arg) {
    std::stringstream ss(arg);
    Target result;
    if (!(ss >> result) || !(ss >> std::ws).eof()) {
        throw bad_lexical_cast();
    }
    return result;
}

} // namespace boost

#endif

namespace docoptcpp03 {

//------------------------------------------------------------------------------
// Value Template Implementations
//------------------------------------------------------------------------------

template <typename T>
inline T Value::as() const {
    if (is_string()) {
        return boost::lexical_cast<T>(as_string());
    } else if (is_long()) {
        return boost::lexical_cast<T>(as_long());
    } else if (is_bool()) {
        return boost::lexical_cast<T>(as_bool());
    }
    throw boost::bad_lexical_cast();
}

template <>
inline bool Value::as<bool>() const {
    try {
        return as_bool();
    } catch (const std::exception&) {
        throw boost::bad_lexical_cast();
    }
}

template <>
inline std::string Value::as<std::string>() const {
    if (is_empty()) throw boost::bad_lexical_cast();
    return as_string();
}

template <typename T>
inline T Value::as_or(const T& default_val) const {
    try {
        if (is_empty()) return default_val;
        return as<T>();
    } catch (const boost::bad_lexical_cast&) {
        return default_val;
    }
}

template <typename T>
inline std::vector<T> Value::as_list() const {
    std::vector<T> res;
    if (is_string_list()) {
        const std::vector<std::string>& list = as_string_list();
        res.reserve(list.size());
        for (size_t i = 0; i < list.size(); ++i) {
            res.push_back(Value(list[i]).as<T>());
        }
    } else if (is_string()) {
        res.push_back(Value(as_string()).as<T>());
    }
    return res;
}

//------------------------------------------------------------------------------
// Options Template Implementations
//------------------------------------------------------------------------------

template <typename T>
inline T Options::get(const std::string& key, const T& default_val) const {
    const_iterator it = map_.find(key);
    if (it != map_.end()) {
        return it->second.as_or<T>(default_val);
    }
    return default_val;
}

} // namespace docoptcpp03
