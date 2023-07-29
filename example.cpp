#include "pydex.hpp"
#include <vector>
#include <iostream>


int main(int argc, char *argv[]) {
    std::vector<std::vector<std::vector<int>>> data {
            {{1, 2, 3}, {4,5, 6}, {7, 8, 9}},
            {{10, 11, 12}, {13, 14, 15}, {16, 17, 18}},
            {{19, 20, 21}, {22, 23, 24}, {25, 26, 27}}
    };
    using namespace std;


    auto data2 = data;

    cout << pydex::pydex<pydex::expr("1, :, :")>(data) << endl << pydex::pydex<pydex::expr(":, :, 0")>(data) << endl;
    pydex::pydex<pydex::expr(":, :, :")>(data2) = pydex::pydex<pydex::expr(":, :, 0")>(data);
    std::cout << pydex::pydex<pydex::expr(":, :, :")>(data2) << std::endl;

    pydex::pydex<pydex::expr(":,:,:")>(data) = {69, 420, 1337};
    std::cout << pydex::pydex<pydex::expr("-1, :, :")>(data) << std::endl;

    std::array<std::array<int, 3>, 10> arr{
            std::array<int, 3>{1, 2, 3},
            std::array<int, 3>{4, 5, 6},
            std::array<int, 3>{7, 8, 9},
            std::array<int, 3>{10, 11, 12},
            std::array<int, 3>{13, 14, 15},
            std::array<int, 3>{16, 17, 18},
            std::array<int, 3>{19, 20, 21},
            std::array<int, 3>{22, 23, 24},
            std::array<int, 3>{25, 26, 27},
            std::array<int, 3>{28, 29, 30},
    };
    pydex::pydex<pydex::expr("4:,:1")>(arr) = std::array{100};
    std::cout << pydex::pydex<pydex::expr(":, :")>(arr) << std::endl;

    std::vector<std::string> vec{"Hello", "World", "!"};
    pydex::pydex<pydex::expr("0")>(vec) = 'h';
    std::cout << pydex::pydex<pydex::expr(":")>(vec) << std::endl;

    return 0;
}
