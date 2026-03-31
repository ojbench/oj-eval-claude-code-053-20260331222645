#include "regex.h"
#include <iostream>
#include <string>

int main() {
    std::string regex, str;
    std::cin >> regex >> str;

    Grammar::RegexChecker checker(regex);

    if (checker.Check(str)) {
        std::cout << "yes" << std::endl;
    } else {
        std::cout << "no" << std::endl;
    }

    return 0;
}
