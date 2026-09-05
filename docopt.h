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
// PART 2: INLINE IMPLEMENTATIONS & DETAILS
// (ファイル後半に集約された内部の実装詳細)
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
// Exceptions Implementation
//------------------------------------------------------------------------------

inline DocoptLanguageError::DocoptLanguageError(const std::string& message)
    : std::runtime_error(message) {}

inline DocoptExit::DocoptExit(int exit_status, const std::string& message, const std::string& usage_str)
    : std::runtime_error(message.empty() ? (usage_str.empty() ? "DocoptExit" : usage_str)
                                         : (usage_str.empty() ? message : message + "\n" + usage_str)),
      usage(usage_str),
      status(exit_status) {}

inline DocoptExit::DocoptExit(const std::string& message, const std::string& usage_str)
    : std::runtime_error(message.empty() ? (usage_str.empty() ? "DocoptExit" : usage_str)
                                         : (usage_str.empty() ? message : message + "\n" + usage_str)),
      usage(usage_str),
      status(1) {}

inline DocoptExit::~DocoptExit() throw() {}

inline DocoptExitHelp::DocoptExitHelp(const std::string& usage_str)
    : DocoptExit(0, "Help requested", usage_str) {}

inline DocoptExitHelp::~DocoptExitHelp() throw() {}

inline DocoptExitVersion::DocoptExitVersion(const std::string& version_str)
    : DocoptExit(0, version_str, version_str) {}

inline DocoptExitVersion::~DocoptExitVersion() throw() {}

inline DocoptArgumentError::DocoptArgumentError(const std::string& message, const std::string& usage_str)
    : DocoptExit(1, message, usage_str) {}

inline DocoptArgumentError::~DocoptArgumentError() throw() {}

//------------------------------------------------------------------------------
// Value Class Implementation
//------------------------------------------------------------------------------

namespace detail {
inline std::string to_lower_str(const std::string& s) {
    std::string res = s;
    for (size_t i = 0; i < res.size(); ++i) {
        res[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(res[i])));
    }
    return res;
}
} // namespace detail

inline Value::Value() : kind_(KIND_EMPTY), bool_val_(false), long_val_(0) {}
inline Value::Value(bool b) : kind_(KIND_BOOL), bool_val_(b), long_val_(0), str_val_(b ? "true" : "false") {}
inline Value::Value(long l) : kind_(KIND_LONG), bool_val_(false), long_val_(l) {
    std::ostringstream oss;
    oss << l;
    str_val_ = oss.str();
}
inline Value::Value(int i) : kind_(KIND_LONG), bool_val_(false), long_val_(i) {
    std::ostringstream oss;
    oss << i;
    str_val_ = oss.str();
}
inline Value::Value(const std::string& s) : kind_(KIND_STRING), bool_val_(false), long_val_(0), str_val_(s) {}
inline Value::Value(const char* s) : kind_(KIND_STRING), bool_val_(false), long_val_(0), str_val_(s ? s : "") {}
inline Value::Value(const std::vector<std::string>& v) : kind_(KIND_STRING_LIST), bool_val_(false), long_val_(0), str_list_val_(v) {}

inline Value::Kind Value::kind() const { return kind_; }

inline bool Value::is_empty() const { return kind_ == KIND_EMPTY; }
inline bool Value::is_bool() const { return kind_ == KIND_BOOL; }
inline bool Value::is_long() const { return kind_ == KIND_LONG; }
inline bool Value::is_string() const { return kind_ == KIND_STRING; }
inline bool Value::is_string_list() const { return kind_ == KIND_STRING_LIST; }

inline bool Value::as_bool() const {
    if (kind_ == KIND_BOOL) {
        return bool_val_;
    }
    if (kind_ == KIND_LONG) {
        return long_val_ != 0;
    }
    if (kind_ == KIND_STRING) {
        std::string s = detail::to_lower_str(str_val_);
        if (s == "true" || s == "1" || s == "yes" || s == "on") {
            return true;
        }
        if (s == "false" || s == "0" || s == "no" || s == "off" || s.empty()) {
            return false;
        }
        throw std::runtime_error("Value cannot be converted to bool: '" + str_val_ + "'");
    }
    throw std::runtime_error("Value cannot be converted to bool.");
}

inline long Value::as_long() const {
    if (kind_ == KIND_LONG) {
        return long_val_;
    }
    if (kind_ == KIND_STRING) {
        try {
            return boost::lexical_cast<long>(str_val_);
        } catch (const boost::bad_lexical_cast&) {
            throw std::runtime_error("Value cannot be converted to long: '" + str_val_ + "'");
        }
    }
    if (kind_ == KIND_BOOL) {
        return bool_val_ ? 1L : 0L;
    }
    throw std::runtime_error("Value cannot be converted to long.");
}

inline const std::string& Value::as_string() const {
    if (kind_ == KIND_LONG && str_val_.empty()) {
        std::ostringstream oss;
        oss << long_val_;
        str_val_ = oss.str();
    } else if (kind_ == KIND_BOOL && str_val_.empty()) {
        str_val_ = bool_val_ ? "true" : "false";
    }
    return str_val_;
}

inline const std::vector<std::string>& Value::as_string_list() const {
    if (kind_ == KIND_STRING && str_list_val_.empty()) {
        str_list_val_.push_back(str_val_);
    }
    return str_list_val_;
}

inline bool Value::as_bool_or(bool default_val) const {
    if (is_bool()) return bool_val_;
    if (is_empty()) return default_val;
    try {
        return as_bool();
    } catch (...) {
        return is_truthy();
    }
}

inline long Value::as_long_or(long default_val) const {
    if (is_long()) return as_long();
    if (is_string()) {
        try {
            return boost::lexical_cast<long>(as_string());
        } catch (const boost::bad_lexical_cast&) {}
    }
    return default_val;
}

inline std::string Value::as_string_or(const std::string& default_val) const {
    if (is_string()) return as_string();
    return default_val;
}

inline std::string Value::as_string_or(const char* default_val) const {
    if (is_string()) return as_string();
    return default_val ? std::string(default_val) : std::string();
}

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
            res.push_back(boost::lexical_cast<T>(list[i]));
        }
    } else if (is_string()) {
        res.push_back(boost::lexical_cast<T>(as_string()));
    }
    return res;
}

inline bool Value::is_truthy() const {
    switch (kind_) {
        case KIND_EMPTY: return false;
        case KIND_BOOL: return bool_val_;
        case KIND_LONG: return long_val_ != 0;
        case KIND_STRING: return !str_val_.empty();
        case KIND_STRING_LIST: return !str_list_val_.empty();
    }
    return false;
}

inline Value::operator unspecified_bool_type() const {
    return is_truthy() ? &Value::dummy_for_bool : 0;
}

inline bool Value::operator!() const {
    return !is_truthy();
}

inline bool Value::operator!=(const Value& other) const {
    return !(*this == other);
}

//------------------------------------------------------------------------------
// Options Class Implementation
//------------------------------------------------------------------------------

inline Options::Options() : map_() {}
inline Options::Options(const std::map<std::string, Value>& other) : map_(other) {}

inline Value& Options::operator[](const std::string& key) {
    return map_[key];
}

inline const Value& Options::operator[](const std::string& key) const {
    const_iterator it = map_.find(key);
    if (it == map_.end()) {
        static const Value empty_val;
        return empty_val;
    }
    return it->second;
}

inline const Value& Options::at(const std::string& key) const {
    const_iterator it = map_.find(key);
    if (it == map_.end()) {
        throw std::out_of_range("Option not found: " + key);
    }
    return it->second;
}

inline Options::const_iterator Options::begin() const { return map_.begin(); }
inline Options::const_iterator Options::end() const { return map_.end(); }
inline Options::iterator Options::begin() { return map_.begin(); }
inline Options::iterator Options::end() { return map_.end(); }

inline Options::const_iterator Options::find(const std::string& key) const { return map_.find(key); }
inline Options::iterator Options::find(const std::string& key) { return map_.find(key); }
inline Options::size_type Options::count(const std::string& key) const { return map_.count(key); }
inline bool Options::has_key(const std::string& key) const { return map_.find(key) != map_.end(); }
inline bool Options::contains(const std::string& key) const { return has_key(key); }

inline std::string Options::get(const std::string& key, const std::string& default_val) const {
    const_iterator it = map_.find(key);
    if (it != map_.end()) {
        return it->second.as_string_or(default_val);
    }
    return default_val;
}

inline std::string Options::get(const std::string& key, const char* default_val) const {
    return get(key, default_val ? std::string(default_val) : std::string());
}

template <typename T>
inline T Options::get(const std::string& key, const T& default_val) const {
    const_iterator it = map_.find(key);
    if (it != map_.end()) {
        return it->second.as_or<T>(default_val);
    }
    return default_val;
}

inline Options::size_type Options::size() const { return map_.size(); }
inline bool Options::empty() const { return map_.empty(); }
inline void Options::clear() { map_.clear(); }

inline bool Options::operator==(const Options& other) const { return map_ == other.map_; }
inline bool Options::operator!=(const Options& other) const { return map_ != other.map_; }

inline const std::map<std::string, Value>& Options::map() const { return map_; }

inline void Options::dump(std::ostream& os) const {
    os << "Options (" << size() << " items): {\n";
    for (const_iterator it = begin(); it != end(); ++it) {
        os << "  \"" << it->first << "\": " << it->second << "\n";
    }
    os << "}";
}

inline std::string Options::dump_string() const {
    std::ostringstream oss;
    dump(oss);
    return oss.str();
}

inline std::ostream& operator<<(std::ostream& os, const Options& opts) {
    opts.dump(os);
    return os;
}

} // namespace docoptcpp03
