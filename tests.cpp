#include "pydex.hpp"
#include <vector>
#include <iostream>
#include <numeric>
#include <chrono>


void matmul(const auto& A, const auto& B, auto& C) {
    auto n = A.size();
    auto m = B.size();
    auto k = B[0].size();

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < k; ++j)
            for (int l = 0; l < m; ++l)
                C[i][j] += A[i][l] * B[l][j];
}

void matmul_test(int n, int m, int k) {
    std::vector<std::vector<float>> A(n, std::vector<float>(m, 0));
    std::vector<std::vector<int>> B(m, std::vector<int>(k, 0));
    std::vector<std::vector<double>> C(n, std::vector<double>(k, 0));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; ++j) {
            A[i][j] = rand();
        }
    }

    for (int i = 0; i < m; i++) {
        for (int j = 0; j < k; ++j) {
            B[i][j] = rand();
        }
    }

    auto D = A;
    auto E = B;
    auto F = C;

    auto G = A;
    auto H = B;
    auto I = C;


    auto t0 = std::chrono::high_resolution_clock::now();
    matmul(pydex<"...">(A), pydex<"...">(B), pydex<"...">(C));
    auto t1 = std::chrono::high_resolution_clock::now();
    std::cout << "Pydexed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() << std::endl;


    t0 = std::chrono::high_resolution_clock::now();
    matmul(D, E, F);
    t1 = std::chrono::high_resolution_clock::now();
    std::cout << "Normal time: " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() << std::endl;

    t0 = std::chrono::high_resolution_clock::now();
    matmul(pydex<"...", true>(G), pydex<"...", true>(H), pydex<"...", true>(I));
    t1 = std::chrono::high_resolution_clock::now();
    std::cout << "Pydexed bounds checked time: " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() << std::endl;


    for (int i = 0; i < n; ++i)
        for (int j = 0; j < k; ++j)
            if (C[i][j] != F[i][j])
                std::cerr << "Assertion failed: " << C[i][j] << " != " << m << std::endl;
}


void check_equal(const auto &a, const auto &b) {
    auto &a_ = pydex<"...", true>(a);
    auto &b_ = pydex<"...", true>(b);

    if (a_ != b_) {
        std::cerr << "Assertion failed: " << a_ << " != " << b_ << std::endl;

        std::cerr << "First: " << a.first() << " Last: " << a.last() << " Step: " << a.step << " Size: " << a.size()
                  << '\n';
    }
}

template<typename T>
void check_equal(const auto &a, const std::initializer_list<T> &b) {
    std::vector<T> v{b};
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
    check_equal(pydex<"2:5:2">(arr), {2, 4});
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
    std::vector<std::vector<std::vector<int>>> data{{{1,  2,  3},  {4,  5,  6},  {7,  8,  9}},
                                                    {{10, 11, 12}, {13, 14, 15}, {16, 17, 18}},
                                                    {{19, 20, 21}, {22, 23, 24}, {25, 26, 27}}};


    check_equal(pydex<":,1::-1,:">(data),
                std::vector{std::vector{std::vector{4, 5, 6,}, {1, 2, 3,},}, {{13, 14, 15,}, {10, 11, 12,},},
                            {{22, 23, 24,}, {19, 20, 21,},},});

    check_equal(pydex<":,1::-1,1">(data), std::vector{std::vector{5, 2,}, {14, 11,}, {23, 20,}});

    check_equal(pydex<"::-1,1::-1,1">(data), std::vector{std::vector{23, 20,}, {14, 11,}, {5, 2,},});

    auto s = pydex<"1,:,0">(data).copy();

    std::cout << pydex<"...">(s) << '\n';


    std::vector<std::vector<int>> data2{{1, 2,  3},
                                        {1, 2,  3,  4,  5,  6,  7,  8},
                                        {9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20}};

    std::vector<std::vector<int>> arr3 = pydex<":, 1">(data2).copy();
    std::cout << pydex<"...">(arr3)[-1] << '\n';

    matmul_test(5033, 1024, 1024);

    std::cout << "All tests passed!\n";
    return 0;
}
