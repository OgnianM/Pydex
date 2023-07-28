#include "pydex.hpp"
#include <vector>
#include <iostream>


int main(int argc, char *argv[]) {
    std::vector<std::vector<std::vector<int>>> data {
            {{1, 2, 3}, {4, 3,5, 6}, {7, 8, 9}},
            {{10, 11, 12}, {13, 14, 15}, {16, 33, 17, 44}},
            {{19, 20, 21}, {22, 23, 24}, {25, 26, 27, 44}},
    };

    std::cout << pydex::pydex<pydex::expr("0,1,2")>(data) << std::endl;

    pydex::pydex<pydex::expr(":, 1, :")>(data) = pydex::pydex<pydex::expr(":, 2, :")>(data);
    std::cout << pydex::pydex<pydex::expr(":, :, :")>(data) << std::endl;

    pydex::pydex<pydex::expr(":,:,:")>(data) = {69, 420, 1337};
    std::cout << pydex::pydex<pydex::expr("-1, :, :")>(data) << std::endl;

    return 0;
}
