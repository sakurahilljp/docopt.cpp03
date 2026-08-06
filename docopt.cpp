#include "docopt.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cassert>

namespace docoptcpp03 {

namespace {

template <typename T>
class shared_ptr {
private:
    T* px;

public:
    shared_ptr() : px(0) {}

    explicit shared_ptr(T* p) : px(p) {
        if (px) px->add_ref();
    }

    ~shared_ptr() {
        if (px) px->release_ref();
    }

    shared_ptr(const shared_ptr& r) : px(r.px) {
        if (px) px->add_ref();
    }

    shared_ptr& operator=(const shared_ptr& r) {
        if (px != r.px) {
            if (px) px->release_ref();
            px = r.px;
            if (px) px->add_ref();
        }
        return *this;
    }

    T& operator*() const { return *px; }
    T* operator->() const { return px; }
    T* get() const { return px; }

    typedef T* shared_ptr::*unspecified_bool_type;
    operator unspecified_bool_type() const { return px ? &shared_ptr::px : 0; }
    bool operator!() const { return px == 0; }

    bool operator==(const shared_ptr& r) const { return px == r.px; }
    bool operator!=(const shared_ptr& r) const { return px != r.px; }
    bool operator<(const shared_ptr& r) const { return px < r.px; }
};

} // anonymous namespace

//------------------------------------------------------------------------------
// String Helper Utilities
//------------------------------------------------------------------------------

static std::string to_lower(const std::string& str) {
    std::string result = str;
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
    }
    return result;
}

static bool starts_with(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

static bool ends_with(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

static std::string trim(const std::string& str, const std::string& drop = " \t\n\r") {
    size_t first = str.find_first_not_of(drop);
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(drop);
    return str.substr(first, last - first + 1);
}

static std::string replace_all(std::string str, const std::string& from, const std::string& to) {
    if (from.empty()) return str;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

static std::vector<std::string> split_words(const std::string& str) {
    std::vector<std::string> result;
    size_t len = str.length();
    size_t i = 0;
    while (i < len) {
        while (i < len && std::isspace(static_cast<unsigned char>(str[i]))) {
            ++i;
        }
        if (i >= len) break;
        size_t start = i;
        while (i < len && !std::isspace(static_cast<unsigned char>(str[i]))) {
            ++i;
        }
        result.push_back(str.substr(start, i - start));
    }
    return result;
}

//------------------------------------------------------------------------------
// Value Implementation
//------------------------------------------------------------------------------

double Value::as_double() const {
    if (kind_ == KIND_LONG) {
        return static_cast<double>(long_val_);
    }
    if (kind_ == KIND_STRING) {
#if defined(DOCOPT_USE_CUSTOM_LEXICAL_CAST)
        return boost::lexical_cast<double>(str_val_);
#else
        return boost::lexical_cast<double>(str_val_);
#endif
    }
    if (kind_ == KIND_BOOL) {
        return bool_val_ ? 1.0 : 0.0;
    }
    throw std::runtime_error("Value cannot be converted to double.");
}

double Value::as_double_or(double default_val) const {
    if (is_empty()) return default_val;
    try {
        return as_double();
    } catch (...) {
        return default_val;
    }
}
//------------------------------------------------------------------------------

bool Value::operator==(const Value& other) const {
    if (kind_ != other.kind_) return false;
    switch (kind_) {
        case KIND_EMPTY: return true;
        case KIND_BOOL: return bool_val_ == other.bool_val_;
        case KIND_LONG: return long_val_ == other.long_val_;
        case KIND_STRING: return str_val_ == other.str_val_;
        case KIND_STRING_LIST: return str_list_val_ == other.str_list_val_;
    }
    return false;
}

bool Value::operator<(const Value& other) const {
    if (kind_ != other.kind_) return kind_ < other.kind_;
    switch (kind_) {
        case KIND_EMPTY: return false;
        case KIND_BOOL: return bool_val_ < other.bool_val_;
        case KIND_LONG: return long_val_ < other.long_val_;
        case KIND_STRING: return str_val_ < other.str_val_;
        case KIND_STRING_LIST: return str_list_val_ < other.str_list_val_;
    }
    return false;
}

std::string Value::to_json() const {
    switch (kind_) {
        case KIND_EMPTY: return "null";
        case KIND_BOOL: return bool_val_ ? "true" : "false";
        case KIND_LONG: {
            std::ostringstream oss;
            oss << long_val_;
            return oss.str();
        }
        case KIND_STRING: {
            std::string s = "\"";
            for (size_t i = 0; i < str_val_.size(); ++i) {
                char c = str_val_[i];
                if (c == '"') s += "\\\"";
                else if (c == '\\') s += "\\\\";
                else if (c == '\b') s += "\\b";
                else if (c == '\f') s += "\\f";
                else if (c == '\n') s += "\\n";
                else if (c == '\r') s += "\\r";
                else if (c == '\t') s += "\\t";
                else s += c;
            }
            s += "\"";
            return s;
        }
        case KIND_STRING_LIST: {
            std::string s = "[";
            for (size_t i = 0; i < str_list_val_.size(); ++i) {
                if (i > 0) s += ", ";
                Value v(str_list_val_[i]);
                s += v.to_json();
            }
            s += "]";
            return s;
        }
    }
    return "null";
}

std::ostream& operator<<(std::ostream& os, const Value& val) {
    os << val.to_json();
    return os;
}

//------------------------------------------------------------------------------
// Pattern Abstract Syntax Tree (AST)
//------------------------------------------------------------------------------

enum PatternKind {
    KIND_ARGUMENT,
    KIND_COMMAND,
    KIND_OPTION,
    KIND_REQUIRED,
    KIND_OPTIONAL,
    KIND_OPTIONS_SHORTCUT,
    KIND_ONE_OR_MORE,
    KIND_EITHER
};

class Pattern;

typedef shared_ptr<Pattern> PatternPtr;

class Pattern {
private:
    mutable unsigned int ref_count_;

public:
    Pattern() : ref_count_(0) {}
    virtual ~Pattern() {}

    void add_ref() const { ++ref_count_; }
    void release_ref() const {
        if (--ref_count_ == 0) {
            delete this;
        }
    }

    virtual PatternKind kind() const = 0;
    virtual std::string name() const { return ""; }
    virtual const Value& get_value() const { static Value empty_v; return empty_v; }
    virtual void set_value(const Value& /*v*/) {}
    virtual std::string repr() const = 0;

    virtual bool is_leaf() const = 0;

    virtual std::vector<PatternPtr>& children() {
        static std::vector<PatternPtr> empty_children;
        return empty_children;
    }

    virtual std::vector<PatternPtr> flat(const std::vector<PatternKind>& kinds = std::vector<PatternKind>()) = 0;

    // Match outcome: pair<matched, pair<left, collected> >
    typedef std::pair<bool, std::pair<std::vector<PatternPtr>, std::vector<PatternPtr> > > MatchResult;

    virtual MatchResult match(
        const std::vector<PatternPtr>& left,
        const std::vector<PatternPtr>& collected = std::vector<PatternPtr>()
    ) = 0;

    PatternPtr fix();
    void fix_identities(const std::vector<PatternPtr>* uniq = NULL);
    PatternPtr fix_repeating_arguments();
};

// Forward declaration of transform
static PatternPtr transform(PatternPtr pattern);

//------------------------------------------------------------------------------
// Leaf Pattern Base & Implementations
//------------------------------------------------------------------------------

class LeafPattern : public Pattern {
protected:
    std::string name_;
    Value value_;

public:
    LeafPattern(const std::string& name, const Value& val = Value())
        : name_(name), value_(val) {}

    virtual bool is_leaf() const { return true; }
    virtual std::string name() const { return name_; }
    virtual const Value& get_value() const { return value_; }
    virtual void set_value(const Value& v) { value_ = v; }

    virtual std::vector<PatternPtr> flat(const std::vector<PatternKind>& kinds = std::vector<PatternKind>()) {
        std::vector<PatternPtr> res;
        if (kinds.empty() || std::find(kinds.begin(), kinds.end(), kind()) != kinds.end()) {
            res.push_back(PatternPtr(this_ptr()));
        }
        return res;
    }

    // Returns index pos and matched pattern pointer (or NULL if no match)
    virtual std::pair<int, PatternPtr> single_match(const std::vector<PatternPtr>& left) = 0;

    virtual MatchResult match(
        const std::vector<PatternPtr>& left,
        const std::vector<PatternPtr>& collected = std::vector<PatternPtr>()
    ) {
        std::pair<int, PatternPtr> sm = single_match(left);
        int pos = sm.first;
        PatternPtr match_ptr = sm.second;

        if (!match_ptr) {
            return std::make_pair(false, std::make_pair(left, collected));
        }

        std::vector<PatternPtr> left_remaining;
        for (size_t i = 0; i < left.size(); ++i) {
            if (static_cast<int>(i) != pos) {
                left_remaining.push_back(left[i]);
            }
        }

        std::vector<PatternPtr> same_name;
        for (size_t i = 0; i < collected.size(); ++i) {
            if (collected[i]->name() == name_) {
                same_name.push_back(collected[i]);
            }
        }

        std::vector<PatternPtr> new_collected = collected;

        if (value_.is_long() || value_.is_string_list()) {
            Value increment;
            if (value_.is_long()) {
                increment = Value(1L);
            } else {
                if (match_ptr->get_value().is_string()) {
                    std::vector<std::string> v;
                    v.push_back(match_ptr->get_value().as_string());
                    increment = Value(v);
                } else if (match_ptr->get_value().is_string_list()) {
                    increment = match_ptr->get_value();
                } else {
                    increment = Value(std::vector<std::string>());
                }
            }

            if (same_name.empty()) {
                match_ptr->set_value(increment);
                new_collected.push_back(match_ptr);
                return std::make_pair(true, std::make_pair(left_remaining, new_collected));
            } else {
                PatternPtr target = same_name[0];
                Value cur = target->get_value();
                if (cur.is_long() && increment.is_long()) {
                    target->set_value(Value(cur.as_long() + increment.as_long()));
                } else if (cur.is_string_list() && increment.is_string_list()) {
                    std::vector<std::string> v = cur.as_string_list();
                    const std::vector<std::string>& inc_v = increment.as_string_list();
                    v.insert(v.end(), inc_v.begin(), inc_v.end());
                    target->set_value(Value(v));
                }
                return std::make_pair(true, std::make_pair(left_remaining, new_collected));
            }
        }

        new_collected.push_back(match_ptr);
        return std::make_pair(true, std::make_pair(left_remaining, new_collected));
    }

private:
    Pattern* this_ptr() { return this; }
};

//------------------------------------------------------------------------------
// Argument, Command, Option
//------------------------------------------------------------------------------

class Argument : public LeafPattern {
public:
    Argument(const std::string& name, const Value& val = Value())
        : LeafPattern(name, val) {}

    virtual PatternKind kind() const { return KIND_ARGUMENT; }

    virtual std::string repr() const {
        return "Argument(" + name_ + ", " + value_.to_json() + ")";
    }

    virtual std::pair<int, PatternPtr> single_match(const std::vector<PatternPtr>& left) {
        for (size_t i = 0; i < left.size(); ++i) {
            if (left[i]->kind() == KIND_ARGUMENT) {
                return std::make_pair(static_cast<int>(i), PatternPtr(new Argument(name_, left[i]->get_value())));
            }
        }
        return std::make_pair(-1, PatternPtr());
    }

    static Argument* parse(const std::string& source) {
        std::string name;
        size_t start = source.find('<');
        size_t end = source.find('>', start);
        if (start != std::string::npos && end != std::string::npos) {
            name = source.substr(start, end - start + 1);
        } else {
            name = source;
        }

        Value val;
        std::string lower_source = to_lower(source);
        size_t def_pos = lower_source.find("[default: ");
        if (def_pos != std::string::npos) {
            size_t val_start = def_pos + 10;
            size_t val_end = source.find(']', val_start);
            if (val_end != std::string::npos) {
                val = Value(source.substr(val_start, val_end - val_start));
            }
        }
        return new Argument(name, val);
    }
};

class Command : public Argument {
public:
    Command(const std::string& name, const Value& val = Value(false))
        : Argument(name, val) {}

    virtual PatternKind kind() const { return KIND_COMMAND; }

    virtual std::string repr() const {
        return "Command(" + name_ + ", " + value_.to_json() + ")";
    }

    virtual std::pair<int, PatternPtr> single_match(const std::vector<PatternPtr>& left) {
        for (size_t i = 0; i < left.size(); ++i) {
            if (left[i]->kind() == KIND_ARGUMENT) {
                if (left[i]->get_value().is_string() && left[i]->get_value().as_string() == name_) {
                    return std::make_pair(static_cast<int>(i), PatternPtr(new Command(name_, Value(true))));
                } else {
                    break;
                }
            }
        }
        return std::make_pair(-1, PatternPtr());
    }
};

class Option : public LeafPattern {
public:
    std::string short_name;
    std::string long_name;
    int argcount;

    Option(const std::string& s = "", const std::string& l = "", int ac = 0, const Value& val = Value(false))
        : LeafPattern(l.empty() ? s : l, (val == Value(false) && ac > 0) ? Value() : val),
          short_name(s), long_name(l), argcount(ac) {}

    virtual PatternKind kind() const { return KIND_OPTION; }

    virtual std::string repr() const {
        return "Option(" + short_name + ", " + long_name + ", " + (argcount ? "1" : "0") + ", " + value_.to_json() + ")";
    }

    virtual std::pair<int, PatternPtr> single_match(const std::vector<PatternPtr>& left) {
        for (size_t i = 0; i < left.size(); ++i) {
            if (name() == left[i]->name()) {
                return std::make_pair(static_cast<int>(i), left[i]);
            }
        }
        return std::make_pair(-1, PatternPtr());
    }

    static Option* parse(const std::string& option_description) {
        std::string s_name, l_name;
        int ac = 0;
        Value val = Value(false);

        std::string trimmed = trim(option_description);
        std::string options_part = trimmed;
        std::string description_part;

        size_t double_space = trimmed.find("  ");
        if (double_space != std::string::npos) {
            options_part = trimmed.substr(0, double_space);
            description_part = trimmed.substr(double_space);
        }

        options_part = replace_all(options_part, ",", " ");
        options_part = replace_all(options_part, "=", " ");
        std::vector<std::string> words = split_words(options_part);

        for (size_t i = 0; i < words.size(); ++i) {
            const std::string& w = words[i];
            if (starts_with(w, "--")) {
                l_name = w;
            } else if (starts_with(w, "-")) {
                s_name = w;
            } else {
                ac = 1;
            }
        }

        if (ac == 1) {
            val = Value();
            std::string lower_desc = to_lower(description_part);
            size_t def_pos = lower_desc.find("[default: ");
            if (def_pos != std::string::npos) {
                size_t val_start = def_pos + 10;
                size_t val_end = description_part.find(']', val_start);
                if (val_end != std::string::npos) {
                    val = Value(description_part.substr(val_start, val_end - val_start));
                }
            }
        }

        return new Option(s_name, l_name, ac, val);
    }
};

//------------------------------------------------------------------------------
// Branch Patterns
//------------------------------------------------------------------------------

class BranchPattern : public Pattern {
protected:
    std::vector<PatternPtr> children_;

public:
    BranchPattern() {}
    explicit BranchPattern(const std::vector<PatternPtr>& children)
        : children_(children) {}

    virtual bool is_leaf() const { return false; }
    virtual std::vector<PatternPtr>& children() { return children_; }

    virtual std::vector<PatternPtr> flat(const std::vector<PatternKind>& kinds = std::vector<PatternKind>()) {
        std::vector<PatternPtr> res;
        if (!kinds.empty() && std::find(kinds.begin(), kinds.end(), kind()) != kinds.end()) {
            res.push_back(PatternPtr(this));
        }
        for (size_t i = 0; i < children_.size(); ++i) {
            std::vector<PatternPtr> child_flat = children_[i]->flat(kinds);
            res.insert(res.end(), child_flat.begin(), child_flat.end());
        }
        return res;
    }
};

class Required : public BranchPattern {
public:
    Required() {}
    explicit Required(const std::vector<PatternPtr>& children)
        : BranchPattern(children) {}

    virtual PatternKind kind() const { return KIND_REQUIRED; }

    virtual std::string repr() const {
        std::string s = "Required(";
        for (size_t i = 0; i < children_.size(); ++i) {
            if (i > 0) s += ", ";
            s += children_[i]->repr();
        }
        s += ")";
        return s;
    }

    virtual MatchResult match(
        const std::vector<PatternPtr>& left,
        const std::vector<PatternPtr>& collected = std::vector<PatternPtr>()
    ) {
        std::vector<PatternPtr> l = left;
        std::vector<PatternPtr> c = collected;
        for (size_t i = 0; i < children_.size(); ++i) {
            MatchResult res = children_[i]->match(l, c);
            if (!res.first) {
                return std::make_pair(false, std::make_pair(left, collected));
            }
            l = res.second.first;
            c = res.second.second;
        }
        return std::make_pair(true, std::make_pair(l, c));
    }
};

class Optional : public BranchPattern {
public:
    Optional() {}
    explicit Optional(const std::vector<PatternPtr>& children)
        : BranchPattern(children) {}

    virtual PatternKind kind() const { return KIND_OPTIONAL; }

    virtual std::string repr() const {
        std::string s = "Optional(";
        for (size_t i = 0; i < children_.size(); ++i) {
            if (i > 0) s += ", ";
            s += children_[i]->repr();
        }
        s += ")";
        return s;
    }

    virtual MatchResult match(
        const std::vector<PatternPtr>& left,
        const std::vector<PatternPtr>& collected = std::vector<PatternPtr>()
    ) {
        std::vector<PatternPtr> l = left;
        std::vector<PatternPtr> c = collected;
        for (size_t i = 0; i < children_.size(); ++i) {
            MatchResult res = children_[i]->match(l, c);
            l = res.second.first;
            c = res.second.second;
        }
        return std::make_pair(true, std::make_pair(l, c));
    }
};

class OptionsShortcut : public Optional {
public:
    OptionsShortcut() {}
    explicit OptionsShortcut(const std::vector<PatternPtr>& children)
        : Optional(children) {}

    virtual PatternKind kind() const { return KIND_OPTIONS_SHORTCUT; }

    virtual std::string repr() const {
        return "OptionsShortcut()";
    }
};

class OneOrMore : public BranchPattern {
public:
    OneOrMore() {}
    explicit OneOrMore(const std::vector<PatternPtr>& children)
        : BranchPattern(children) {}

    virtual PatternKind kind() const { return KIND_ONE_OR_MORE; }

    virtual std::string repr() const {
        std::string s = "OneOrMore(";
        for (size_t i = 0; i < children_.size(); ++i) {
            if (i > 0) s += ", ";
            s += children_[i]->repr();
        }
        s += ")";
        return s;
    }

    virtual MatchResult match(
        const std::vector<PatternPtr>& left,
        const std::vector<PatternPtr>& collected = std::vector<PatternPtr>()
    ) {
        assert(children_.size() == 1);
        std::vector<PatternPtr> l = left;
        std::vector<PatternPtr> c = collected;
        std::vector<PatternPtr> l_prev;
        bool matched = true;
        int times = 0;

        while (matched) {
            MatchResult res = children_[0]->match(l, c);
            matched = res.first;
            if (matched) ++times;
            l = res.second.first;
            c = res.second.second;
            if (l_prev == l) break;
            l_prev = l;
        }

        if (times >= 1) {
            return std::make_pair(true, std::make_pair(l, c));
        }
        return std::make_pair(false, std::make_pair(left, collected));
    }
};

class Either : public BranchPattern {
public:
    Either() {}
    explicit Either(const std::vector<PatternPtr>& children)
        : BranchPattern(children) {}

    virtual PatternKind kind() const { return KIND_EITHER; }

    virtual std::string repr() const {
        std::string s = "Either(";
        for (size_t i = 0; i < children_.size(); ++i) {
            if (i > 0) s += ", ";
            s += children_[i]->repr();
        }
        s += ")";
        return s;
    }

    virtual MatchResult match(
        const std::vector<PatternPtr>& left,
        const std::vector<PatternPtr>& collected = std::vector<PatternPtr>()
    ) {
        std::vector<MatchResult> outcomes;
        for (size_t i = 0; i < children_.size(); ++i) {
            MatchResult res = children_[i]->match(left, collected);
            if (res.first) {
                outcomes.push_back(res);
            }
        }

        if (!outcomes.empty()) {
            size_t min_idx = 0;
            size_t min_left_size = outcomes[0].second.first.size();
            for (size_t i = 1; i < outcomes.size(); ++i) {
                size_t left_size = outcomes[i].second.first.size();
                if (left_size < min_left_size) {
                    min_left_size = left_size;
                    min_idx = i;
                }
            }
            return outcomes[min_idx];
        }

        return std::make_pair(false, std::make_pair(left, collected));
    }
};

//------------------------------------------------------------------------------
// Pattern Fix Methods
//------------------------------------------------------------------------------

PatternPtr Pattern::fix() {
    fix_identities();
    fix_repeating_arguments();
    return PatternPtr(this);
}

void Pattern::fix_identities(const std::vector<PatternPtr>* uniq) {
    if (is_leaf()) return;

    std::vector<PatternPtr> local_uniq;
    if (!uniq) {
        std::vector<PatternPtr> fl = flat();
        for (size_t i = 0; i < fl.size(); ++i) {
            bool exists = false;
            for (size_t j = 0; j < local_uniq.size(); ++j) {
                if (fl[i]->repr() == local_uniq[j]->repr()) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                local_uniq.push_back(fl[i]);
            }
        }
        uniq = &local_uniq;
    }

    std::vector<PatternPtr>& child_list = children();
    for (size_t i = 0; i < child_list.size(); ++i) {
        if (child_list[i]->is_leaf()) {
            for (size_t j = 0; j < uniq->size(); ++j) {
                if (child_list[i]->repr() == (*uniq)[j]->repr()) {
                    child_list[i] = (*uniq)[j];
                    break;
                }
            }
        } else {
            child_list[i]->fix_identities(uniq);
        }
    }
}

PatternPtr Pattern::fix_repeating_arguments() {
    PatternPtr transformed = transform(PatternPtr(this));
    std::vector<PatternPtr>& either_children = transformed->children();

    for (size_t i = 0; i < either_children.size(); ++i) {
        std::vector<PatternPtr>& case_children = either_children[i]->children();
        for (size_t j = 0; j < case_children.size(); ++j) {
            PatternPtr e = case_children[j];
            int count = 0;
            for (size_t k = 0; k < case_children.size(); ++k) {
                if (case_children[k]->repr() == e->repr()) {
                    count++;
                }
            }
            if (count > 1) {
                if (e->kind() == KIND_ARGUMENT || (e->kind() == KIND_OPTION && static_cast<Option*>(e.get())->argcount > 0)) {
                    if (e->get_value().is_empty()) {
                        e->set_value(Value(std::vector<std::string>()));
                    } else if (!e->get_value().is_string_list()) {
                        if (e->get_value().is_string()) {
                            e->set_value(Value(split_words(e->get_value().as_string())));
                        }
                    }
                }
                if (e->kind() == KIND_COMMAND || (e->kind() == KIND_OPTION && static_cast<Option*>(e.get())->argcount == 0)) {
                    e->set_value(Value(0L));
                }
            }
        }
    }
    return PatternPtr(this);
}

//------------------------------------------------------------------------------
// transform Implementation
//------------------------------------------------------------------------------

static PatternPtr transform(PatternPtr pattern) {
    std::vector<std::vector<PatternPtr> > result;
    std::vector<std::vector<PatternPtr> > groups;

    std::vector<PatternPtr> init_group;
    init_group.push_back(pattern);
    groups.push_back(init_group);

    while (!groups.empty()) {
        std::vector<PatternPtr> children = groups.front();
        groups.erase(groups.begin());

        int parent_idx = -1;
        for (size_t i = 0; i < children.size(); ++i) {
            if (!children[i]->is_leaf()) {
                parent_idx = static_cast<int>(i);
                break;
            }
        }

        if (parent_idx != -1) {
            PatternPtr child = children[parent_idx];
            children.erase(children.begin() + parent_idx);

            if (child->kind() == KIND_EITHER) {
                for (size_t c = 0; c < child->children().size(); ++c) {
                    std::vector<PatternPtr> new_g;
                    new_g.push_back(child->children()[c]);
                    new_g.insert(new_g.end(), children.begin(), children.end());
                    groups.push_back(new_g);
                }
            } else if (child->kind() == KIND_ONE_OR_MORE) {
                std::vector<PatternPtr> new_g;
                new_g.insert(new_g.end(), child->children().begin(), child->children().end());
                new_g.insert(new_g.end(), child->children().begin(), child->children().end());
                new_g.insert(new_g.end(), children.begin(), children.end());
                groups.push_back(new_g);
            } else {
                std::vector<PatternPtr> new_g;
                new_g.insert(new_g.end(), child->children().begin(), child->children().end());
                new_g.insert(new_g.end(), children.begin(), children.end());
                groups.push_back(new_g);
            }
        } else {
            result.push_back(children);
        }
    }

    std::vector<PatternPtr> either_children;
    for (size_t i = 0; i < result.size(); ++i) {
        either_children.push_back(PatternPtr(new Required(result[i])));
    }
    return PatternPtr(new Either(either_children));
}

//------------------------------------------------------------------------------
// Tokens Helper Class
//------------------------------------------------------------------------------

enum ErrorType {
    ERROR_EXIT,
    ERROR_LANGUAGE
};

class Tokens {
private:
    std::vector<std::string> tokens_;
    ErrorType error_type_;

public:
    Tokens(const std::vector<std::string>& source, ErrorType err_type = ERROR_EXIT)
        : tokens_(source), error_type_(err_type) {}

    ErrorType error_type() const { return error_type_; }

    static Tokens from_pattern(const std::string& source) {
        std::string s;
        size_t i = 0;
        while (i < source.size()) {
            if (starts_with(source.substr(i), "...")) {
                s += " ... ";
                i += 3;
            } else if (source[i] == '[' || source[i] == ']' || source[i] == '(' || source[i] == ')' || source[i] == '|') {
                s += ' ';
                s += source[i];
                s += ' ';
                i++;
            } else {
                s += source[i];
                i++;
            }
        }

        std::vector<std::string> toks;
        std::string current;
        i = 0;
        while (i < s.size()) {
            if (std::isspace(static_cast<unsigned char>(s[i]))) {
                if (!current.empty()) {
                    toks.push_back(current);
                    current.clear();
                }
                i++;
            } else if (s[i] == '<') {
                size_t end = s.find('>', i);
                if (end != std::string::npos) {
                    current += s.substr(i, end - i + 1);
                    toks.push_back(current);
                    current.clear();
                    i = end + 1;
                } else {
                    current += s[i];
                    i++;
                }
            } else {
                current += s[i];
                i++;
            }
        }
        if (!current.empty()) {
            toks.push_back(current);
        }

        return Tokens(toks, ERROR_LANGUAGE);
    }

    void raise_error(const std::string& msg) const {
        if (error_type_ == ERROR_LANGUAGE) {
            throw DocoptLanguageError(msg);
        } else {
            throw DocoptArgumentError(msg);
        }
    }

    bool empty() const { return tokens_.empty(); }

    std::string move() {
        if (tokens_.empty()) return "";
        std::string tok = tokens_.front();
        tokens_.erase(tokens_.begin());
        return tok;
    }

    std::string current() const {
        if (tokens_.empty()) return "";
        return tokens_.front();
    }

    const std::vector<std::string>& get_all() const { return tokens_; }
};

//------------------------------------------------------------------------------
// Parsers
//------------------------------------------------------------------------------

static std::vector<PatternPtr> parse_expr(Tokens& tokens, std::vector<PatternPtr>& options);

static std::vector<PatternPtr> parse_long(Tokens& tokens, std::vector<PatternPtr>& options) {
    std::string token = tokens.move();
    size_t eq_pos = token.find('=');
    std::string long_opt = (eq_pos != std::string::npos) ? token.substr(0, eq_pos) : token;
    std::string val_str = (eq_pos != std::string::npos) ? token.substr(eq_pos + 1) : "";
    bool has_eq = (eq_pos != std::string::npos);

    Value val = has_eq ? Value(val_str) : Value();

    std::vector<PatternPtr> similar;
    for (size_t i = 0; i < options.size(); ++i) {
        if (options[i]->kind() == KIND_OPTION && static_cast<Option*>(options[i].get())->long_name == long_opt) {
            similar.push_back(options[i]);
        }
    }

    if (tokens.error_type() == ERROR_EXIT && similar.empty()) {
        for (size_t i = 0; i < options.size(); ++i) {
            if (options[i]->kind() == KIND_OPTION) {
                Option* opt_ptr = static_cast<Option*>(options[i].get());
                if (!opt_ptr->long_name.empty() && starts_with(opt_ptr->long_name, long_opt)) {
                    similar.push_back(options[i]);
                }
            }
        }
    }

    PatternPtr o;

    if (similar.size() > 1) {
        std::string msg = long_opt + " is not a unique prefix: ";
        for (size_t i = 0; i < similar.size(); ++i) {
            if (i > 0) msg += ", ";
            msg += static_cast<Option*>(similar[i].get())->long_name;
        }
        msg += "?";
        tokens.raise_error(msg);
    } else if (similar.empty()) {
        int argcount = has_eq ? 1 : 0;
        PatternPtr opt(new Option("", long_opt, argcount));
        options.push_back(opt);
        if (tokens.error_type() == ERROR_EXIT) {
            o = PatternPtr(new Option("", long_opt, argcount, argcount ? Value(val_str) : Value(true)));
        } else {
            o = opt;
        }
    } else {
        Option* match_opt = static_cast<Option*>(similar[0].get());
        PatternPtr opt(new Option(match_opt->short_name, match_opt->long_name, match_opt->argcount, match_opt->get_value()));
        Option* opt_ptr = static_cast<Option*>(opt.get());
        if (opt_ptr->argcount == 0) {
            if (has_eq) {
                tokens.raise_error(opt_ptr->long_name + " must not have an argument");
            }
        } else {
            if (!has_eq) {
                if (tokens.empty() || tokens.current() == "--") {
                    tokens.raise_error(opt_ptr->long_name + " requires argument");
                }
                val_str = tokens.move();
                has_eq = true;
            }
        }
        if (tokens.error_type() == ERROR_EXIT) {
            opt_ptr->set_value(has_eq ? Value(val_str) : Value(true));
        }
        o = opt;
    }

    std::vector<PatternPtr> result;
    result.push_back(o);
    return result;
}

static std::vector<PatternPtr> parse_shorts(Tokens& tokens, std::vector<PatternPtr>& options) {
    std::string token = tokens.move();
    std::string left = token.substr(1); // skip leading '-'

    std::vector<PatternPtr> parsed;

    while (!left.empty()) {
        std::string short_opt = std::string("-") + left[0];
        left = left.substr(1);

        std::vector<PatternPtr> similar;
        for (size_t i = 0; i < options.size(); ++i) {
            if (options[i]->kind() == KIND_OPTION && static_cast<Option*>(options[i].get())->short_name == short_opt) {
                similar.push_back(options[i]);
            }
        }

        PatternPtr o;

        if (similar.size() > 1) {
            std::ostringstream oss;
            oss << short_opt << " is specified ambiguously " << similar.size() << " times";
            tokens.raise_error(oss.str());
        } else if (similar.empty()) {
            PatternPtr opt(new Option(short_opt, "", 0));
            options.push_back(opt);
            if (tokens.error_type() == ERROR_EXIT) {
                o = PatternPtr(new Option(short_opt, "", 0, Value(true)));
            } else {
                o = opt;
            }
        } else {
            Option* match_opt = static_cast<Option*>(similar[0].get());
            PatternPtr opt(new Option(short_opt, match_opt->long_name, match_opt->argcount, match_opt->get_value()));
            Option* opt_ptr = static_cast<Option*>(opt.get());
            std::string val_str;
            bool has_val = false;

            if (opt_ptr->argcount != 0) {
                if (left.empty()) {
                    if (tokens.empty() || tokens.current() == "--") {
                        tokens.raise_error(short_opt + " requires argument");
                    }
                    val_str = tokens.move();
                    has_val = true;
                } else {
                    val_str = left;
                    left.clear();
                    has_val = true;
                }
            }

            if (tokens.error_type() == ERROR_EXIT) {
                opt_ptr->set_value(has_val ? Value(val_str) : Value(true));
            }
            o = opt;
        }
        parsed.push_back(o);
    }
    return parsed;
}

static std::vector<PatternPtr> parse_atom(Tokens& tokens, std::vector<PatternPtr>& options) {
    std::string token = tokens.current();
    std::vector<PatternPtr> result;

    if (token == "(" || token == "[") {
        tokens.move();
        std::string matching = (token == "(") ? ")" : "]";
        std::vector<PatternPtr> expr = parse_expr(tokens, options);

        if (tokens.move() != matching) {
            tokens.raise_error("unmatched '" + token + "'");
        }

        PatternPtr pattern;
        if (token == "(") {
            pattern = PatternPtr(new Required(expr));
        } else {
            pattern = PatternPtr(new Optional(expr));
        }
        result.push_back(pattern);
        return result;
    } else if (token == "options") {
        tokens.move();
        result.push_back(PatternPtr(new OptionsShortcut()));
        return result;
    } else if (starts_with(token, "--") && token != "--") {
        return parse_long(tokens, options);
    } else if (starts_with(token, "-") && token != "-" && token != "--") {
        return parse_shorts(tokens, options);
    } else if ((starts_with(token, "<") && ends_with(token, ">")) || std::isupper(static_cast<unsigned char>(token[0]))) {
        result.push_back(PatternPtr(new Argument(tokens.move())));
        return result;
    } else {
        result.push_back(PatternPtr(new Command(tokens.move())));
        return result;
    }
}

static std::vector<PatternPtr> parse_seq(Tokens& tokens, std::vector<PatternPtr>& options) {
    std::vector<PatternPtr> result;
    while (!tokens.empty() && tokens.current() != "]" && tokens.current() != ")" && tokens.current() != "|") {
        std::vector<PatternPtr> atom = parse_atom(tokens, options);
        if (tokens.current() == "...") {
            std::vector<PatternPtr> one_or_more_children;
            one_or_more_children.push_back(PatternPtr(new Required(atom)));
            atom.clear();
            atom.push_back(PatternPtr(new OneOrMore(one_or_more_children)));
            tokens.move();
        }
        result.insert(result.end(), atom.begin(), atom.end());
    }
    return result;
}

static std::vector<PatternPtr> parse_expr(Tokens& tokens, std::vector<PatternPtr>& options) {
    std::vector<PatternPtr> seq = parse_seq(tokens, options);

    if (tokens.current() != "|") {
        return seq;
    }

    std::vector<PatternPtr> result;
    if (seq.size() > 1) {
        result.push_back(PatternPtr(new Required(seq)));
    } else {
        result.insert(result.end(), seq.begin(), seq.end());
    }

    while (tokens.current() == "|") {
        tokens.move();
        seq = parse_seq(tokens, options);
        if (seq.size() > 1) {
            result.push_back(PatternPtr(new Required(seq)));
        } else {
            result.insert(result.end(), seq.begin(), seq.end());
        }
    }

    std::vector<PatternPtr> either_result;
    if (result.size() > 1) {
        either_result.push_back(PatternPtr(new Either(result)));
        return either_result;
    }
    return result;
}

static PatternPtr parse_pattern(const std::string& source, std::vector<PatternPtr>& options) {
    Tokens tokens = Tokens::from_pattern(source);
    std::vector<PatternPtr> result = parse_expr(tokens, options);
    if (!tokens.empty()) {
        std::string unparsed;
        while (!tokens.empty()) {
            if (!unparsed.empty()) unparsed += " ";
            unparsed += tokens.move();
        }
        tokens.raise_error("unexpected ending: '" + unparsed + "'");
    }
    return PatternPtr(new Required(result));
}

static std::vector<PatternPtr> parse_argv(Tokens& tokens, std::vector<PatternPtr>& options, bool options_first = false) {
    std::vector<PatternPtr> parsed;
    while (!tokens.empty()) {
        if (tokens.current() == "--") {
            tokens.move();
            while (!tokens.empty()) {
                parsed.push_back(PatternPtr(new Argument("", Value(tokens.move()))));
            }
            return parsed;
        } else if (starts_with(tokens.current(), "--")) {
            std::vector<PatternPtr> long_opts = parse_long(tokens, options);
            parsed.insert(parsed.end(), long_opts.begin(), long_opts.end());
        } else if (starts_with(tokens.current(), "-") && tokens.current() != "-") {
            std::vector<PatternPtr> short_opts = parse_shorts(tokens, options);
            parsed.insert(parsed.end(), short_opts.begin(), short_opts.end());
        } else if (options_first) {
            while (!tokens.empty()) {
                parsed.push_back(PatternPtr(new Argument("", Value(tokens.move()))));
            }
            return parsed;
        } else {
            parsed.push_back(PatternPtr(new Argument("", Value(tokens.move()))));
        }
    }
    return parsed;
}

static std::vector<std::string> parse_section(const std::string& name, const std::string& source) {
    std::vector<std::string> sections;
    std::string lower_name = to_lower(name);
    std::istringstream iss(source);
    std::string line;

    std::string current_section;
    bool in_section = false;

    while (std::getline(iss, line)) {
        std::string lower_line = to_lower(line);
        size_t pos = lower_line.find(lower_name);
        if (pos != std::string::npos) {
            if (in_section && !trim(current_section).empty()) {
                sections.push_back(trim(current_section));
            }
            in_section = true;
            current_section = line + "\n";
        } else if (in_section) {
            if (line.empty() || line[0] == ' ' || line[0] == '\t') {
                current_section += line + "\n";
            } else {
                if (!trim(current_section).empty()) {
                    sections.push_back(trim(current_section));
                }
                in_section = false;
                current_section.clear();
            }
        }
    }

    if (in_section && !trim(current_section).empty()) {
        sections.push_back(trim(current_section));
    }

    return sections;
}

static std::vector<PatternPtr> parse_defaults(const std::string& doc) {
    std::vector<PatternPtr> defaults;
    std::vector<std::string> section_list = parse_section("options:", doc);

    for (size_t i = 0; i < section_list.size(); ++i) {
        std::string s = section_list[i];
        size_t colon_pos = s.find(':');
        if (colon_pos != std::string::npos) {
            s = s.substr(colon_pos + 1);
        }

        std::istringstream iss(s);
        std::string line;
        std::vector<std::string> option_blocks;
        std::string current_block;

        while (std::getline(iss, line)) {
            std::string t = trim(line);
            if (starts_with(t, "-")) {
                if (!current_block.empty()) {
                    option_blocks.push_back(current_block);
                }
                current_block = line;
            } else if (!current_block.empty()) {
                current_block += "\n" + line;
            }
        }
        if (!current_block.empty()) {
            option_blocks.push_back(current_block);
        }

        for (size_t b = 0; b < option_blocks.size(); ++b) {
            std::string block_trim = trim(option_blocks[b]);
            if (starts_with(block_trim, "-")) {
                PatternPtr opt(Option::parse(option_blocks[b]));
                defaults.push_back(opt);
            }
        }
    }
    return defaults;
}

static std::string formal_usage(const std::string& section) {
    std::string sec = section;
    size_t colon = sec.find(':');
    if (colon != std::string::npos) {
        sec = sec.substr(colon + 1);
    }
    std::vector<std::string> pu = split_words(sec);
    if (pu.empty()) return "( )";

    std::string prog = pu[0];
    std::string result = "( ";
    for (size_t i = 1; i < pu.size(); ++i) {
        if (pu[i] == prog) {
            result += ") | ( ";
        } else {
            result += pu[i] + " ";
        }
    }
    result += ")";
    return result;
}

static bool is_known_option(const PatternPtr& opt, const std::vector<PatternPtr>& doc_options) {
    if (opt->kind() != KIND_OPTION) return true;
    Option* o = static_cast<Option*>(opt.get());
    for (size_t i = 0; i < doc_options.size(); ++i) {
        if (doc_options[i]->kind() == KIND_OPTION) {
            Option* d = static_cast<Option*>(doc_options[i].get());
            if ((!o->short_name.empty() && o->short_name == d->short_name) ||
                (!o->long_name.empty() && o->long_name == d->long_name)) {
                return true;
            }
        }
    }
    return false;
}

static void extras(bool help, const std::string& version, const std::vector<PatternPtr>& argv_options, const std::vector<PatternPtr>& doc_options, const std::string& doc) {
    for (size_t i = 0; i < argv_options.size(); ++i) {
        if (!is_known_option(argv_options[i], doc_options)) {
            return;
        }
    }

    if (help) {
        for (size_t i = 0; i < argv_options.size(); ++i) {
            std::string n = argv_options[i]->name();
            if ((n == "-h" || n == "--help") && argv_options[i]->get_value().is_bool() && argv_options[i]->get_value().as_bool()) {
                std::string help_doc = trim(doc, "\n");
                throw DocoptExitHelp(help_doc);
            }
        }
    }
    if (!version.empty()) {
        for (size_t i = 0; i < argv_options.size(); ++i) {
            std::string n = argv_options[i]->name();
            if (n == "--version" && argv_options[i]->get_value().is_bool() && argv_options[i]->get_value().as_bool()) {
                throw DocoptExitVersion(version);
            }
        }
    }
}

//------------------------------------------------------------------------------
// Main docopt Entry Points
//------------------------------------------------------------------------------

Options docopt_parse(
    const std::string& doc,
    const std::vector<std::string>& argv,
    bool help,
    const std::string& version,
    bool options_first
) {
    std::vector<std::string> usage_sections = parse_section("usage:", doc);
    if (usage_sections.empty()) {
        throw DocoptLanguageError("\"usage:\" (case-insensitive) not found.");
    }
    if (usage_sections.size() > 1) {
        throw DocoptLanguageError("More than one \"usage:\" (case-insensitive).");
    }

    std::string usage_str = usage_sections[0];

    std::vector<PatternPtr> doc_options = parse_defaults(doc);
    std::vector<PatternPtr> options = doc_options;

    PatternPtr pattern = parse_pattern(formal_usage(usage_str), options);

    Tokens tokens(argv, ERROR_EXIT);
    std::vector<PatternPtr> argv_parsed = parse_argv(tokens, options, options_first);

    std::vector<PatternKind> option_kinds;
    option_kinds.push_back(KIND_OPTION);
    std::vector<PatternPtr> pattern_options = pattern->flat(option_kinds);

    std::vector<PatternKind> shortcut_kinds;
    shortcut_kinds.push_back(KIND_OPTIONS_SHORTCUT);
    std::vector<PatternPtr> options_shortcuts = pattern->flat(shortcut_kinds);

    for (size_t i = 0; i < options_shortcuts.size(); ++i) {
        std::vector<PatternPtr> parse_doc_options = parse_defaults(doc);

        std::vector<PatternPtr> children;
        for (size_t d = 0; d < parse_doc_options.size(); ++d) {
            bool in_pattern = false;
            for (size_t p = 0; p < pattern_options.size(); ++p) {
                if (parse_doc_options[d]->name() == pattern_options[p]->name()) {
                    in_pattern = true;
                    break;
                }
            }
            if (!in_pattern) {
                Option* opt_ptr = static_cast<Option*>(parse_doc_options[d].get());
                children.push_back(PatternPtr(new Option(opt_ptr->short_name, opt_ptr->long_name, opt_ptr->argcount, opt_ptr->get_value())));
            }
        }
        options_shortcuts[i]->children() = children;
    }

    extras(help, version, argv_parsed, doc_options, doc);

    PatternPtr fixed_pattern = pattern->fix();
    Pattern::MatchResult match_res = fixed_pattern->match(argv_parsed);

    bool matched = match_res.first;
    std::vector<PatternPtr> left = match_res.second.first;
    std::vector<PatternPtr> collected = match_res.second.second;

    if (matched && left.empty()) {
        docoptcpp03::Options result;
        std::vector<PatternPtr> flat_all = pattern->flat();
        flat_all.insert(flat_all.end(), collected.begin(), collected.end());

        for (size_t i = 0; i < flat_all.size(); ++i) {
            if (flat_all[i]->is_leaf()) {
                result[flat_all[i]->name()] = flat_all[i]->get_value();
            }
        }
        return result;
    }

    throw DocoptArgumentError("", usage_str);
}

Options docopt_parse(
    const std::string& doc,
    int argc,
    char const* const argv[],
    bool help,
    const std::string& version,
    bool options_first
) {
    std::vector<std::string> args;
    if (argc > 0 && argv) {
        for (int i = 0; i < argc; ++i) {
            if (argv[i]) {
                args.push_back(argv[i]);
            }
        }
    }
    return docopt_parse(doc, args, help, version, options_first);
}

Options docopt(
    const std::string& doc,
    const std::vector<std::string>& argv,
    bool help,
    const std::string& version,
    bool options_first
) {
    try {
        return docopt_parse(doc, argv, help, version, options_first);
    } catch (const DocoptExit& e) {
        if (!e.usage.empty()) {
            std::cout << e.usage << std::endl;
        }
        std::exit(e.status);
    } catch (const DocoptLanguageError& e) {
        std::cerr << "Docopt language error: " << e.what() << std::endl;
        std::exit(1);
    }
}

Options docopt(
    const std::string& doc,
    int argc,
    char const* const argv[],
    bool help,
    const std::string& version,
    bool options_first
) {
    std::vector<std::string> args;
    if (argc > 0 && argv) {
        for (int i = 0; i < argc; ++i) {
            if (argv[i]) {
                args.push_back(argv[i]);
            }
        }
    }
    return docopt(doc, args, help, version, options_first);
}

} // namespace docoptcpp03
