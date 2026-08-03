#include "docopt.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <algorithm>

static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

static bool starts_with(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

static bool ends_with(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

// Simple JSON Value for Test Verification
struct JsonVal {
    enum Type { J_NULL, J_BOOL, J_NUMBER, J_STRING, J_ARRAY, J_OBJECT } type;
    bool bool_val;
    long num_val;
    std::string str_val;
    std::vector<JsonVal> arr_val;
    std::map<std::string, JsonVal> obj_val;

    JsonVal() : type(J_NULL), bool_val(false), num_val(0) {}

    static JsonVal parse(const std::string& str) {
        std::string s = trim(str);
        JsonVal v;
        if (s == "null") {
            v.type = J_NULL;
            return v;
        }
        if (s == "true") {
            v.type = J_BOOL;
            v.bool_val = true;
            return v;
        }
        if (s == "false") {
            v.type = J_BOOL;
            v.bool_val = false;
            return v;
        }
        if (s.size() >= 2 && s[0] == '"' && s[s.size() - 1] == '"') {
            v.type = J_STRING;
            v.str_val = s.substr(1, s.size() - 2);
            return v;
        }
        if (!s.empty() && (s[0] == '-' || std::isdigit(static_cast<unsigned char>(s[0])))) {
            v.type = J_NUMBER;
            v.num_val = std::atol(s.c_str());
            return v;
        }
        if (!s.empty() && s[0] == '[') {
            v.type = J_ARRAY;
            std::string content = s.substr(1, s.size() - (s[s.size() - 1] == ']' ? 2 : 1));
            size_t idx = 0;
            while (idx < content.size()) {
                while (idx < content.size() && (content[idx] == ' ' || content[idx] == ',' || content[idx] == '\n' || content[idx] == '\r' || content[idx] == '\t')) idx++;
                if (idx >= content.size()) break;
                size_t start = idx;
                if (content[idx] == '"') {
                    idx++;
                    while (idx < content.size() && content[idx] != '"') {
                        if (content[idx] == '\\') idx++;
                        idx++;
                    }
                    if (idx < content.size()) idx++;
                } else {
                    while (idx < content.size() && content[idx] != ',' && content[idx] != ']' && !std::isspace(static_cast<unsigned char>(content[idx]))) {
                        idx++;
                    }
                }
                v.arr_val.push_back(JsonVal::parse(content.substr(start, idx - start)));
            }
            return v;
        }
        if (!s.empty() && s[0] == '{') {
            v.type = J_OBJECT;
            std::string content = trim(s.substr(1, s.size() - (s[s.size() - 1] == '}' ? 2 : 1)));
            size_t i = 0;
            while (i < content.size()) {
                while (i < content.size() && (std::isspace(static_cast<unsigned char>(content[i])) || content[i] == ',')) i++;
                if (i >= content.size()) break;
                if (content[i] != '"') break;
                size_t k_start = i + 1;
                size_t k_end = content.find('"', k_start);
                if (k_end == std::string::npos) break;
                std::string key = content.substr(k_start, k_end - k_start);
                i = k_end + 1;
                while (i < content.size() && (std::isspace(static_cast<unsigned char>(content[i])) || content[i] == ':')) i++;
                if (i >= content.size()) break;

                size_t v_start = i;
                if (content[i] == '"') {
                    i++;
                    while (i < content.size() && content[i] != '"') {
                        if (content[i] == '\\') i++;
                        i++;
                    }
                    if (i < content.size()) i++;
                } else if (content[i] == '[') {
                    int depth = 1;
                    i++;
                    while (i < content.size() && depth > 0) {
                        if (content[i] == '[') depth++;
                        else if (content[i] == ']') depth--;
                        i++;
                    }
                } else {
                    while (i < content.size() && content[i] != ',' && content[i] != '}' && !std::isspace(static_cast<unsigned char>(content[i]))) {
                        i++;
                    }
                }
                std::string val_str = content.substr(v_start, i - v_start);
                v.obj_val[key] = JsonVal::parse(val_str);
            }
            return v;
        }
        return v;
    }

    bool equals_docopt_value(const docoptcpp03::Value& dv) const {
        if (type == J_NULL) return dv.is_empty();
        if (type == J_BOOL) return dv.is_bool() && dv.as_bool() == bool_val;
        if (type == J_NUMBER) return dv.is_long() && dv.as_long() == num_val;
        if (type == J_STRING) return dv.is_string() && dv.as_string() == str_val;
        if (type == J_ARRAY) {
            if (!dv.is_string_list()) return false;
            const std::vector<std::string>& dv_list = dv.as_string_list();
            if (dv_list.size() != arr_val.size()) return false;
            for (size_t i = 0; i < dv_list.size(); ++i) {
                if (arr_val[i].type != J_STRING || arr_val[i].str_val != dv_list[i]) return false;
            }
            return true;
        }
        return false;
    }
};

struct TestCase {
    std::string doc;
    std::string cmd;
    std::string expected_json;
};

static std::vector<TestCase> parse_testcases(const std::string& filepath) {
    std::ifstream ifs(filepath.c_str());
    if (!ifs.is_open()) {
        std::cerr << "Failed to open testcases file: " << filepath << std::endl;
        std::exit(1);
    }

    std::vector<TestCase> cases;
    std::string line;
    std::string current_doc;
    bool in_doc = false;

    while (std::getline(ifs, line)) {
        std::string stripped = trim(line);

        if (starts_with(stripped, "#")) continue;

        if (starts_with(stripped, "r\"\"\"") || starts_with(stripped, "\"\"\"")) {
            if (in_doc) {
                in_doc = false;
            } else {
                in_doc = true;
                current_doc.clear();
                size_t q = line.find("\"\"\"");
                std::string rest = line.substr(q + 3);
                if (ends_with(trim(rest), "\"\"\"")) {
                    std::string inner = trim(rest);
                    inner = inner.substr(0, inner.size() - 3);
                    current_doc = inner + "\n";
                    in_doc = false;
                } else if (!rest.empty()) {
                    current_doc += rest + "\n";
                }
            }
            continue;
        }

        if (in_doc) {
            std::string t = trim(line);
            if (ends_with(t, "\"\"\"")) {
                current_doc += line.substr(0, line.rfind("\"\"\"")) + "\n";
                in_doc = false;
            } else {
                current_doc += line + "\n";
            }
        } else if (starts_with(stripped, "$ ")) {
            std::string cmd = stripped.substr(2);
            std::string exp;
            while (std::getline(ifs, line)) {
                std::string l = trim(line);
                if (starts_with(l, "#")) continue;
                if (l.empty()) {
                    if (!exp.empty()) break;
                    continue;
                }
                if (starts_with(l, "$") || starts_with(l, "r\"\"\"") || starts_with(l, "\"\"\"")) {
                    break;
                }
                if (l.find('#') != std::string::npos) {
                    l = trim(l.substr(0, l.find('#')));
                }
                if (!exp.empty()) exp += "\n";
                exp += l;
                if (l == "\"user-error\"" || (ends_with(l, "}") && std::count(exp.begin(), exp.end(), '{') == std::count(exp.begin(), exp.end(), '}'))) {
                    break;
                }
            }
            TestCase tc;
            tc.doc = current_doc;
            tc.cmd = cmd;
            tc.expected_json = exp;
            cases.push_back(tc);
        }
    }
    return cases;
}

static std::vector<std::string> parse_cmdline(const std::string& cmdline) {
    std::vector<std::string> args;
    std::istringstream iss(cmdline);
    std::string arg;
    iss >> arg; // Skip program name (e.g. "prog")
    while (iss >> arg) {
        args.push_back(arg);
    }
    return args;
}

int main(int argc, char* argv[]) {
    std::string test_file = "docopt.python/testcases.docopt";
    if (argc > 1) {
        test_file = argv[1];
    }

    std::cout << "Loading testcases from " << test_file << "..." << std::endl;
    std::vector<TestCase> cases = parse_testcases(test_file);
    std::cout << "Found " << cases.size() << " test cases." << std::endl;

    int passed = 0;
    int failed = 0;

    for (size_t i = 0; i < cases.size(); ++i) {
        std::cout << "Running test " << (i + 1) << "/" << cases.size() << "..." << std::endl;
        const TestCase& tc = cases[i];
        std::vector<std::string> args = parse_cmdline(tc.cmd);

        bool is_user_error = (tc.expected_json == "\"user-error\"");
        bool test_ok = false;

        try {
            docoptcpp03::Options result = docoptcpp03::Docopt(tc.doc, args);

            if (is_user_error) {
                std::cout << "[FAIL] Case " << (i + 1) << ": Expected user-error, but docopt succeeded." << std::endl;
                std::cout << "  Cmd: " << tc.cmd << std::endl;
            } else {
                JsonVal expected = JsonVal::parse(tc.expected_json);
                bool match = true;

                if (expected.type == JsonVal::J_OBJECT) {
                    if (result.size() != expected.obj_val.size()) {
                        match = false;
                    } else {
                        for (std::map<std::string, JsonVal>::const_iterator it = expected.obj_val.begin(); it != expected.obj_val.end(); ++it) {
                            docoptcpp03::Options::const_iterator res_it = result.find(it->first);
                            if (res_it == result.end()) {
                                match = false;
                                break;
                            }
                            if (!it->second.equals_docopt_value(res_it->second)) {
                                match = false;
                                break;
                            }
                        }
                    }
                } else {
                    match = false;
                }

                if (match) {
                    test_ok = true;
                } else {
                    std::cout << "[FAIL] Case " << (i + 1) << " mismatch." << std::endl;
                    std::cout << "  Cmd: " << tc.cmd << std::endl;
                    std::cout << "  Expected: " << tc.expected_json << std::endl;
                    std::cout << "  Got     : {";
                    for (docoptcpp03::Options::const_iterator it = result.begin(); it != result.end(); ++it) {
                        if (it != result.begin()) std::cout << ", ";
                        std::cout << "\"" << it->first << "\": " << it->second;
                    }
                    std::cout << "}" << std::endl;
                }
            }
        } catch (const docoptcpp03::DocoptExit&) {
            if (is_user_error) {
                test_ok = true;
            } else {
                std::cout << "[FAIL] Case " << (i + 1) << ": Unexpected DocoptExit." << std::endl;
                std::cout << "  Cmd: " << tc.cmd << std::endl;
                std::cout << "  Expected: " << tc.expected_json << std::endl;
            }
        } catch (const docoptcpp03::DocoptLanguageError& e) {
            if (is_user_error) {
                test_ok = true;
            } else {
                std::cout << "[FAIL] Case " << (i + 1) << ": Unexpected DocoptLanguageError (" << e.what() << ")" << std::endl;
                std::cout << "  Cmd: " << tc.cmd << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "[FAIL] Case " << (i + 1) << ": Unexpected exception: " << e.what() << std::endl;
        }

        if (test_ok) {
            passed++;
        } else {
            failed++;
        }
    }

    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Results: " << passed << " passed, " << failed << " failed out of " << cases.size() << " tests." << std::endl;

    return (failed == 0) ? 0 : 1;
}
