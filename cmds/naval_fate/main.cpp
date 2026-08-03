#include "docopt.h"
#include <iostream>
#include <string>
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
    try {
        const docoptcpp03::Options opts = docoptcpp03::Docopt(USAGE, argc - 1, argv + 1, true, "Naval Fate 2.0");

        std::cout << "--- Parsed Command Line Options ---" << std::endl;
        for (docoptcpp03::Options::const_iterator it = opts.begin(); it != opts.end(); ++it) {
            std::cout << it->first << ": " << it->second << std::endl;
        }

        std::cout << "\n--- Logic Execution ---" << std::endl;
        if (opts["ship"] && opts["new"]) {
            std::vector<std::string> names = opts["<name>"].as_string_list();
            std::cout << "Creating new ship(s): ";
            for (size_t i = 0; i < names.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << names[i];
            }
            std::cout << std::endl;
        } else if (opts["ship"] && opts["move"]) {
            int x = opts["<x>"].as_or<int>(0);
            int y = opts["<y>"].as_or<int>(0);
            int speed = opts["--speed"].as_or<int>(10);
            std::string name = opts["<name>"].as_string_or("Unassigned");
            std::cout << "Moving ship '" << name << "' to (" << x << ", " << y << ") at speed " << speed << " kn" << std::endl;
        } else if (opts["mine"]) {
            int x = opts["<x>"].as_or<int>(0);
            int y = opts["<y>"].as_or<int>(0);
            std::string mine_type = opts["--drifting"] ? "drifting" : "moored";
            std::string action = opts["set"] ? "Setting" : "Removing";
            std::cout << action << " " << mine_type << " mine at (" << x << ", " << y << ")" << std::endl;
        }
    } catch (const docoptcpp03::DocoptExit& e) {
        std::cout << e.usage << std::endl;
        return e.status;
    } catch (const docoptcpp03::DocoptLanguageError& e) {
        std::cerr << "Docopt language error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
