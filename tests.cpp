#include "pydex.hpp"
#include <vector>
#include <iostream>
#include <numeric>


void check_equal(const auto& a, const auto& b) {
    auto& a_ = pydex::index<"...">(a);
    auto& b_ = pydex::index<"...">(b);

    if (a_ != b_) {
        std::cerr << "Assertion failed: " << a_ << " != " << b_ << std::endl;


    }

    std::cerr << "First: " << a.first() << " Last: " << a.last() <<
              " Step: " << a.step << " Size: " << a.size() << " Underlying size: " << a.underlying_size() << '\n';
}
template<typename T>
void check_equal(const auto& a, const std::initializer_list<T>& b) {
    std::vector<T> v {b};
    return check_equal(a, v);
}

template <auto N> consteval auto expr(char const (&cstr)[N]) {
    std::array<char, N-1> arr;
    for (std::size_t i = 0; i < N-1; ++i)
        arr[i] = cstr[i];
    return arr;
}

int main(int argc, char *argv[]) {


    std::array<int, 10> arr;
    std::iota(arr.begin(), arr.end(), 0);

    check_equal(pydex::index<"...">(arr), arr); // arr[...] == arr

    check_equal(pydex::index<":">(arr), {0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
    check_equal(pydex::index<"2:">(arr), {2, 3, 4, 5, 6, 7, 8, 9});
    check_equal(pydex::index<"-4:">(arr), {6, 7, 8, 9});
    check_equal(pydex::index<":5">(arr), {0, 1, 2, 3, 4});
    check_equal(pydex::index<":-3">(arr), {0, 1, 2, 3, 4, 5, 6});
    check_equal(pydex::index<"2:5">(arr), {2, 3, 4});
    check_equal(pydex::index<"-4:-1">(arr), {6, 7, 8});
    check_equal(pydex::index<"2:5:2">(arr), {2,4});
    check_equal(pydex::index<"-4:-1:2">(arr), {6, 8});


    check_equal(pydex::index<"::-3">(arr), {9, 6, 3, 0});
    check_equal(pydex::index<"2::-1">(arr), {2, 1, 0});
    check_equal(pydex::index<"-4::-4">(arr), {6, 2});
    check_equal(pydex::index<":5:2">(arr), {0, 2, 4});
    check_equal(pydex::index<":-3:-3">(arr), {9});
    check_equal(pydex::index<"2:5:5">(arr), {2});
    check_equal(pydex::index<"-4:-1:-1">(arr), std::vector<int>{});
    check_equal(pydex::index<"2:5:2">(arr), {2, 4});
    check_equal(pydex::index<"-4:-1:2">(arr), {6, 8});
    std::vector<std::vector<std::vector<int>>> data {
            {{1, 2, 3}, {4,5, 6}, {7, 8, 9}},
            {{10, 11, 12}, {13, 14, 15}, {16, 17, 18}},
            {{19, 20, 21}, {22, 23, 24}, {25, 26, 27}}
    };


    std::cout<< pydex::index<"...">(data);

    using namespace pydex::detail;

    return 0;
}
