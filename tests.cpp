#include "pydex.hpp"
#include <vector>
#include <iostream>
#include <numeric>

void check_equal(const auto& a, const auto& b) {
    auto& a_ = pydex<"...">(a);
    auto& b_ = pydex<"...">(b);

    if (a_ != b_) {
        std::cerr << "Assertion failed: " << a_ << " != " << b_ << std::endl;

        std::cerr << "First: " << a.first() << " Last: " << a.last() <<
                  " Step: " << a.step << " Size: " << a.size() << '\n';
    }
}
template<typename T>
void check_equal(const auto& a, const std::initializer_list<T>& b) {
    std::vector<T> v {b};
    return check_equal(a, v);
}

int main(int argc, char *argv[]) {
    std::array<int, 10> arr;
    std::iota(arr.begin(), arr.end(), 0);

    check_equal(pydex<"...">(arr), arr); // arr[...] == arr

    check_equal(pydex<":">(arr), {0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
    check_equal(pydex<"2:">(arr), {2, 3, 4, 5, 6, 7, 8, 9});
    check_equal(pydex<"-4:">(arr), {6, 7, 8, 9});
    check_equal(pydex<":5">(arr), {0, 1, 2, 3, 4});
    check_equal(pydex<":-3">(arr), {0, 1, 2, 3, 4, 5, 6});
    check_equal(pydex<"2:5">(arr), {2, 3, 4});
    check_equal(pydex<"-4:-1">(arr), {6, 7, 8});
    check_equal(pydex<"2:5:2">(arr), {2,4});
    check_equal(pydex<"-4:-1:2">(arr), {6, 8});


    check_equal(pydex<"::-3">(arr), {9, 6, 3, 0});
    check_equal(pydex<"2::-1">(arr), {2, 1, 0});
    check_equal(pydex<"-4::-4">(arr), {6, 2});
    check_equal(pydex<":5:2">(arr), {0, 2, 4});
    check_equal(pydex<":-3:-3">(arr), {9});
    check_equal(pydex<"2:5:5">(arr), {2});
    check_equal(pydex<"-4:-1:-1">(arr), std::vector<int>{});
    check_equal(pydex<"2:5:2">(arr), {2, 4});
    check_equal(pydex<"-4:-1:2">(arr), {6, 8});
    std::vector<std::vector<std::vector<int>>> data {
            {{1, 2, 3}, {4,5, 6}, {7, 8, 9}},
            {{10, 11, 12}, {13, 14, 15}, {16, 17, 18}},
            {{19, 20, 21}, {22, 23, 24}, {25, 26, 27}}
    };



    check_equal(pydex<":,1::-1,:">(data),
                std::vector{std::vector{std::vector
                  {4, 5, 6, },
                  {1, 2, 3, },
                 },
                 {{13, 14, 15, },
                  {10, 11, 12, },
                 },
                 {{22, 23, 24, },
                  {19, 20, 21, },
                 },
                }
    );

    check_equal(pydex<":,1::-1,1">(data),
                std::vector{std::vector
                            {5, 2, },
                            {14, 11, },
                            {23, 20, }}
                        );

    check_equal(pydex<"::-1,1::-1,1">(data),
                std::vector{std::vector{23, 20, },
                                        {14, 11, },
                                        {5, 2, },
                }
    );

    auto c = pydex_::copy(pydex<"::-1,1::-1,1">(data));
    std::cout << pydex<"...">(c) << '\n';

    int &scalar = pydex<"1,2,2">(data);
    std::cout << scalar << '\n';

    try {
        pydex<"1,:,1">(data) = {1, 2, 3, 4, 5, 6};
    } catch(...) {
        std::cout << pydex<"...">(data) << '\n';
    }


   std::vector<std::vector<int>> data2 {
           {1,2,3},
           {1,2,3,4,5,6,7,8},
           {9,10,11,12,13,14,15,16,17,18,19,20}
   };

   std::cout << pydex<"2, :3">(data2)[-3] << '\n';

    std::cout << "All tests passed!\n";
    return 0;
}
