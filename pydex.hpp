/**
    Copyright 2023 OGNYAN MIREV

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
    documentation files (the “Software”), to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
    permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
    COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
    ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once
#include <array>
#include <ostream>
#include <sstream>

namespace pydex_ {
namespace detail {

template<auto S>
consteval int size() {
    int s = 0;
    for (int i = 0; i < S.size(); i++, s++) {
        if (S[i] == '\0') break;
    }
    return s;
}

template<auto S, char C, int N = 0>
consteval int count() {
    if constexpr (size<S>() == N) {
        return 0;
    } else {
        return (S[N] == C) + count<S, C, N + 1>();
    }
}

template<auto S>
consteval bool is_number() {
    constexpr int n = S.size();
    if (n == 0) {
        return false;
    }

    constexpr bool neg = S[0] == '-';
    if (neg && n == 1) {
        return false;
    }

    for (int i = neg; i < n; ++i) {
        if (S[i] == '\0') {
            return !(neg && i == 1) && i > 0;
        }
        if (S[i] < '0' || S[i] > '9') {
            return false;
        }
    }

    return true;
}

template<auto S>
consteval int stoi() {
    static_assert(is_number<S>(), "Not a number");
    int res = 0;
    constexpr bool neg = S[0] == '-';

    for (int i = neg; i < S.size(); ++i) {
        if (S[i] == '\0') break;
        res *= 10;
        res += S[i] - '0';
    }

    return neg ? -res : res;
}

template<auto S>
consteval auto pop_front() {
    std::array<std::decay_t<decltype(S[0])>, S.size() - 1> arr;
    for (int i = 1; i < S.size(); i++) {
        arr[i - 1] = S[i];
    }
    return arr;
}

template<auto S, char delim>
consteval auto split() {
    constexpr int n = size<S>();
    constexpr int c = count<S, delim>();
    std::array<std::array<char, n>, c + 1> res{'\0'};
    int i = 0;
    int j = 0;
    int k = 0;
    for (; i < n; ++i) {
        if (S[i] == delim) {
            ++j;
            k = 0;
            continue;
        }
        res[j][k++] = S[i];
    }
    return res;
}

template<auto S>
consteval auto sanitize() {
    constexpr int n = size<S>();
    std::array<char, n - count<S, ' '>() - count<S, '\0'>()> res;
    int i = 0;
    int j = 0;
    for (; i < n; ++i) {
        if (S[i] == ' ' || S[i] == '\0') {
            continue;
        }
        res[j] = S[i];
        ++j;
    }
    return res;
}

template<typename T> concept Pydexable = requires(T x) { x[x.size()]; };

template<typename T> concept SizedIterable = requires(T x) { x.size(); x.begin(); x.end(); };

template<typename T> concept Resizable = requires(T x) { x.resize(0); };

template<typename T> concept Decayable = requires(T x) { x.decay(); };

template<typename T>
requires (!Pydexable<T> && !SizedIterable<T>)
consteval int dimensionality() {
    return 0;
}

template<typename T>
requires (Pydexable<T>)
consteval int dimensionality() {
    return 1 + dimensionality<decltype(std::declval<T>()[0])>();
}

template<typename T>
requires (!Pydexable<T> && SizedIterable<T>)
consteval int dimensionality() {
    return 1 + dimensionality<decltype(*std::declval<T>().begin())>();
}

template<auto S, int K, auto Default>
consteval auto get_if_number() {
    if constexpr (K < S.size()) {
        if constexpr (S[K][0] == '\0' || S[K][0] == '.') {
            return Default;
        } else return stoi<S[K]>();
    } else {
        return Default;
    }
}

template<auto S>
consteval int ellipsis_count() {
    int count = 0;
    for (int i = 0; i < S.size(); i++) {
        count += S[i][0] == '.' && S[i][1] == '.' && S[i][2] == '.';
    }
    return count;
}

constexpr auto &deconst(const auto &x) {
    return const_cast<std::decay_t<decltype(x)> &>(x);
}

template<auto S, Pydexable Vt, bool bounds_checks = false> requires(S.size() > 0)
struct View : protected Vt {
    static constexpr int colon_count = count<S[0], ':'>();
    static constexpr bool is_ellipsis = S[0][0] == '.' && S[0][1] == '.' && S[0][2] == '.';
    static constexpr bool is_slice = colon_count > 0 || is_ellipsis;
    static constexpr int rank = dimensionality<Vt>();
    static constexpr int dims = dimensionality<View>();
    static constexpr auto tokenized = split<S[0], ':'>();
    static constexpr int step = get_if_number<tokenized, 2, 1>();

    static_assert(colon_count <= 2, "Too many colons");
    static_assert(step != 0, "Step cannot be 0");
    static_assert(S.size() <= rank, "Too many indices");
    static_assert(ellipsis_count<S>() <= 1, "Cannot have more than one ellipsis.");

    [[nodiscard]] constexpr int first() const {
        constexpr int first = get_if_number<tokenized, 0, step < 0 ? -1 : 0>();

        if constexpr (first < 0) {
            return int(Vt::size()) + first;
        } else {
            return first;
        }
    }

    [[nodiscard]] constexpr int last() const {
        // Use the bool type as a sentinel value to indicate that the last index is not specified.
        constexpr auto last = get_if_number<tokenized, 1, false>();

        if constexpr (std::is_same_v<std::decay_t<decltype(last)>, bool>) {
            if constexpr (step < 0) {
                return -1;
            } else {
                return Vt::size();
            }
        } else {
            if constexpr (last < 0) {
                return int(Vt::size()) + last;
            } else {
                return last;
            }
        }
    }

    View(const View &) = delete;

    constexpr View &operator=(const auto &other) {
        return assignment_impl(other);
    }

    template<typename T>
    constexpr View &operator=(const std::initializer_list<T> &init) {
        return assignment_impl(init);
    }

    constexpr bool operator==(const Pydexable auto &other) const {
        if constexpr (rank != dimensionality<decltype(other)>()) {
            return false;
        }
        if (size() != other.size()) return false;
        for (int i = 0; i < size(); i++) {
            if ((*this)[i] != other[i]) return false;
        }
        return true;
    }

    [[nodiscard]] constexpr int size() const {
        if constexpr (is_slice) {
            auto s = (last() - first());
            return std::max(s / step + bool(s % step), 0);
        } else if constexpr (Pydexable<decltype(next())>) {
            return next().size();
        }
    }

    constexpr auto &operator[](auto i) const { return index_impl(i); }
    constexpr auto &operator[](auto i) { return deconst(index_impl(i)); }

    constexpr auto &decay() const { return decay_impl(); }
    constexpr auto &decay() { return deconst(decay_impl()); }

    constexpr auto &next() const requires(!is_slice) { return next_impl(); }
    constexpr auto &next() requires(!is_slice) { return deconst(next_impl()); }

    template<typename T>
    struct Iterator {
        T &indexer;
        size_t i = 0;

        Iterator(T &indexer, size_t start = 0) : indexer(indexer), i(start) {}

        auto &operator*() {
            return indexer[i];
        }

        Iterator &operator++() {
            ++i;
            return *this;
        }

        bool operator!=(const Iterator &other) {
            return i != other.i;
        }
    };
    auto begin() { return Iterator<View &>(*this); }
    auto end() { return Iterator<View &>(*this, View::size()); }
    auto begin() const { return Iterator<const View &>(*this); }
    auto end() const { return Iterator<const View &>(*this, View::size()); }

    /// @return a deep copy of the given object with the same type
    auto copy() const;
private:
    constexpr auto &next(auto index) const { return next_impl(index); }
    constexpr auto &next(auto index) { return deconst(next_impl(index)); }

    constexpr View &assignment_impl(const auto &other) {
        constexpr int dims_this = dims;
        using otherT = std::decay_t<decltype(other)>;
        constexpr int dims_other = dimensionality<otherT>();
        static_assert(dims_other <= dims_this,
                      "Cannot assign a higher dimensional object to a lower dimensional one");

        if constexpr (!is_slice) {
            next() = other;
            return *this;
        } else if constexpr (dims_other == dims_this) {
            if (size() < other.size()) {
                if (Vt::size() == size()) {
                    if constexpr (Resizable<Vt>) {
                        Vt::resize(other.size());
                    } else {
                        throw std::runtime_error("Cannot assign to a smaller non-resizable container");
                    }
                } else {
                    throw std::runtime_error("Cannot assign data of size " + std::to_string(other.size()) +
                                             " to a view of size " + std::to_string(size()));
                }
            }
            if constexpr (Pydexable<decltype(other)>) {
                for (int i = 0; i < other.size(); ++i) {
                    (*this)[i] = other[i];
                }

            } else {
                int i = 0;
                for (const auto &j: other) {
                    (*this)[i++] = j;
                }
            }

        } else if constexpr (dims_other <= dims_this) {
            if (this->size() == 0) {
                Vt::resize(1);
            }
            for (int i = 0; i < size(); ++i) {
                (*this)[i] = other;
            }
        }
        return *this;
    }

    constexpr auto &decay_once() const {
        return reinterpret_cast<const Vt &>(*this);
    }

    template<typename T = Vt>
    constexpr auto &decay_impl() const {
        auto &v = decay_once();
        if constexpr (Decayable<T>) {
            return v.decay();
        } else return v;
    }

    constexpr auto &next_impl() const requires(!is_slice) {
        constexpr auto index = stoi<S[0]>();
        if constexpr (index < 0) {
            return next_impl(Vt::size() + index);
        } else return next_impl(index);
    }

    constexpr auto &next_impl(int index) const {
        if (index < 0) {
            index = Vt::size() + index;
        }
        if constexpr (bounds_checks) {
            if (index >= Vt::size() || index < 0) {
                throw std::out_of_range("Index out of range");
            }
        }
        auto &k = decay_once();

        if constexpr (S.size() == 1) {
            if constexpr (is_ellipsis && rank > 1) {
                return reinterpret_cast<const View<S, std::decay_t<decltype(k[index])>, bounds_checks> &>(k[index]);
            } else {
                return k[index];
            }
        } else {
            if constexpr (is_ellipsis && rank > S.size()) {
                return reinterpret_cast<const View<S, std::decay_t<decltype(k[index])>, bounds_checks> &>(k[index]);
            } else {
                auto &m = reinterpret_cast<const View<pop_front<S>(), std::decay_t<decltype(k[index])>, bounds_checks> &>(k[index]);
                if constexpr (is_number<S[1]>()) {
                    return m.next();
                } else {
                    return m;
                }
            }
        }
    }

    constexpr auto &index_impl(int i) const {
        if constexpr (is_slice) {
            return next_impl(first() + i * step);
        } else {
            return next()[i];
        }
    }
};

template<auto N>
struct Expression : std::array<char, N - 1> {
    consteval Expression(const char (&cstr)[N]) {
        for (std::size_t i = 0; i < N - 1; ++i)
            (*this)[i] = cstr[i];
    }
};
}; // namespace detail
};

template<pydex_::detail::Expression S, bool bounds_checks = false>
constexpr auto &pydex(pydex_::detail::Pydexable auto &v) {
    using namespace pydex_::detail;
    auto &t = reinterpret_cast<View<split<sanitize<S>(), ','>(), std::decay_t<decltype(v)>, bounds_checks> &>(v);
    if constexpr (!std::decay_t<decltype(t)>::is_slice) {
        return t.next();
    } else {
        return t;
    }
}

template<pydex_::detail::Expression S, bool bounds_checks = false>
constexpr auto &pydex(const pydex_::detail::Pydexable auto &v) {
    using namespace pydex_::detail;
    auto &t = reinterpret_cast<const View<split<sanitize<S>(), ','>(), const std::decay_t<decltype(v)>, bounds_checks> &>(v);
    if constexpr (!std::decay_t<decltype(t)>::is_slice) {
        return t.next();
    } else {
        return t;
    }
}

namespace pydex_ {
namespace detail {
template<auto S, Pydexable Vt, bool bounds_checks> requires (S.size() > 0)
auto View<S, Vt, bounds_checks>::copy() const {
    std::decay_t<decltype(decay())> b;
    pydex<"...">(b) = *this;
    return b;
}
};
};

template<auto E, pydex_::detail::Pydexable Vt>
std::ostream &operator<<(std::ostream &os, const pydex_::detail::View<E, Vt> &v) {
    os << "{";
    for (int j = 0; j < v.size(); j++) {
        auto &i = v[j];
        os << i;
        if (j == v.size() - 1) {
            break;
        }
        if constexpr (pydex_::detail::View<E, Vt>::rank == 1) {
            os << ", ";
        } else {
            os << std::endl;
        }
    }
    os << "},";
    if (v.rank > 1) {
        os << std::endl;
    }
    return os;
}
