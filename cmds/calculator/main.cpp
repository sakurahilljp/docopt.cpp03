#include "docopt.h"
#include <iostream>
#include <vector>

static const char USAGE[] =
"Calculator Example.\n"
"\n"
"Usage:\n"
"  calculator add <numbers>...\n"
"  calculator multiply <numbers>...\n"
"  calculator -h | --help\n"
"\n"
"Options:\n"
"  -h --help     Show this screen.\n";

int main(int argc, char* argv[]) {
    // docopt() handles --help and argument errors automatically via std::exit(status)
    const docoptcpp03::Options opts = docoptcpp03::docopt(USAGE, argc - 1, argv + 1);

    if (opts["<numbers>"]) {
        std::vector<double> nums = opts["<numbers>"].as_list<double>();

        if (opts["add"]) {
            double sum = 0;
            for (size_t i = 0; i < nums.size(); ++i) {
                sum += nums[i];
            }
            std::cout << "Sum: " << sum << std::endl;
        } else if (opts["multiply"]) {
            double prod = 1;
            for (size_t i = 0; i < nums.size(); ++i) {
                prod *= nums[i];
            }
            std::cout << "Product: " << prod << std::endl;
        }
    }

    return 0;
}
