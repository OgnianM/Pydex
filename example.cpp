#include "pydex.hpp"
#include <vector>
#include <iostream>


int main(int argc, char *argv[]) {
    std::vector<std::vector<std::vector<int>>> data {
            {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}},
            {{10, 11, 12}, {13, 14, 15}, {16, 33, 17, 18}},
            {{19, 20, 21}, {22, 23, 24}, {25, 26, 27}},
    };


    std::vector<int> v1 {1, 2, 3, 4, 5, 6, 7, 8, 9};

    std::cout << pydex::pydex<pydex::expr(":, -1, 0:-2")>(data) << std::endl;
    std::cout << pydex::pydex<pydex::expr(":-4")>(v1) << std::endl;

    return 0;
}
