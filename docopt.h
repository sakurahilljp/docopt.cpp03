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
// (Public API declarations for library consumers)
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
    bool is_string_list() const;

    // Type query based on Value::Kind (unsupported types cause compile-time error)
    template <typename T>
    bool is() const;

    // Type accessors & conversions
    template <typename T>
    T as() const;

    template <typename T>
    T as_or(const T& default_val) const;

    std::string as_or(const char* default_val) const;

    const std::vector<std::string>& as_string_list() const;

    template <typename T>
    std::vector<T> as_list() const;

    // docopt for C++11 value compatibility methods (CamelCase)
    bool isBool() const;
    bool isLong() const;
    bool isString() const;
    bool isStringList() const { return is_string_list(); }

    bool asBool() const;
    long asLong() const;
    double asDouble() const;
    std::string asString() const;
    const std::vector<std::string>& asStringList() const { return as_string_list(); }

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
// (Header-only template definitions)
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
namespace detail {
template <bool> struct docopt_is_supported_type;
template <> struct docopt_is_supported_type<true> {};

template <typename T>
struct is_supported_value_type {
    enum { value = 0 };
};
template <> struct is_supported_value_type<bool> { enum { value = 1 }; };
template <> struct is_supported_value_type<long> { enum { value = 1 }; };
template <> struct is_supported_value_type<int> { enum { value = 1 }; };
template <> struct is_supported_value_type<std::string> { enum { value = 1 }; };
} // namespace detail

// Value::is<T>() primary template definition
// Checks the held type based on Value::Kind (unsupported types trigger compile-time error)
template <typename T>
inline bool Value::is() const {
    detail::docopt_is_supported_type<detail::is_supported_value_type<T>::value> ERROR_unsupported_type_for_Value_is;
    (void)ERROR_unsupported_type_for_Value_is;
    return false;
}

template <>
bool Value::is<bool>() const;

template <>
bool Value::is<long>() const;

template <>
bool Value::is<int>() const;

template <>
bool Value::is<std::string>() const;

inline bool Value::isBool() const { return is<bool>(); }
inline bool Value::isLong() const { return is<long>(); }
inline bool Value::isString() const { return is<std::string>(); }

template <typename T>
inline T Value::as() const {
    if (kind_ == KIND_STRING) {
        return boost::lexical_cast<T>(str_val_);
    } else if (kind_ == KIND_LONG) {
        return boost::lexical_cast<T>(long_val_);
    } else if (kind_ == KIND_BOOL) {
        return boost::lexical_cast<T>(bool_val_ ? 1 : 0);
    }
    throw boost::bad_lexical_cast();
}

template <>
bool Value::as<bool>() const;

template <>
long Value::as<long>() const;

template <>
double Value::as<double>() const;

template <>
std::string Value::as<std::string>() const;

inline bool Value::asBool() const { return as<bool>(); }
inline long Value::asLong() const { return as<long>(); }
inline double Value::asDouble() const { return as<double>(); }
inline std::string Value::asString() const { return as<std::string>(); }

template <typename T>
inline T Value::as_or(const T& default_val) const {
    try {
        if (is_empty()) return default_val;
        return as<T>();
    } catch (const boost::bad_lexical_cast&) {
        return default_val;
    }
}

inline std::string Value::as_or(const char* default_val) const {
    return as_or<std::string>(default_val ? std::string(default_val) : std::string());
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
    } else if (kind_ == KIND_STRING) {
        res.push_back(Value(str_val_).as<T>());
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
