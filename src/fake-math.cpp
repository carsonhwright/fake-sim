#include <stdlib.h>
#include <stdio.h>
#include "include/fake-math.hpp"

Base::Base(int param1, int param2) {
    parameter1 = param1;
    parameter2 = param2;
}

Base::Base(int param1) : Base(0, param1) {}

void Base::show_params() {
    printf("param1: %d\nparam2: %d\n", this->parameter1, this->parameter2);
    fflush(stdout);
}

int Base::get_param(char *param_name) {
    switch(*param_name) {
        case "param1" : // this doesn't work
            return parameter1;
        case "param2" : // this doesn't work
            return parameter2;
        default:
            return NULL;
    }
    // TODO but something like the below might
    /*
    enum class Color { Red, Green, Unknown };

Color getColor(const std::string& str) {
    static const std::unordered_map<std::string, Color> colorMap = {
        {"red", Color::Red},
        {"green", Color::Green}
    };
    
    auto it = colorMap.find(str);
    if (it != colorMap.end()) {
        return it->second;
    }
    return Color::Unknown;
}

int main() {
    std::string input = "green";

    switch (getColor(input)) {
        case Color::Red:
            std::cout << "Stop!" << std::endl;
            break;
        case Color::Green:
            std::cout << "Go!" << std::endl;
            break;
        default:
            std::cout << "Unknown color." << std::endl;
            break;
    }
} */
    return 0;
}