#pragma once
#include <array>
#include <ostream>
#include <cassert>

namespace pydex {
namespace detail {
consteval int count(auto str, const char c, int n = 0) {
    return str.size() == n ? 0 : (str[n] == c) + count(str, c, n + 1);
}

template<auto S> consteval bool is_number() {
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
            return !(neg && i == 1);
        }
        if (S[i] < '0' || S[i] > '9') {
            return false;
        }
    }

    return true;
}

template<auto S> consteval int stoi() {
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

template<auto S> consteval auto pop_front() {
    std::array<std::decay_t<decltype(S[0])>, S.size() - 1> arr;
    for (int i = 1; i < S.size(); i++) {
        arr[i-1] = S[i];
    }
    return arr;
}


template<auto S> consteval auto pop_back() {
    std::array<std::decay_t<decltype(S[0])>, S.size() - 1> arr;
    for (int i = 0; i < (S.size()-1); i++) {
        arr[i] = S[i];
    }
    return arr;
}

template<auto S>
consteval auto pop_back_if_empty() {
    if constexpr (S[S.size()-1][0] == '\0') {
        return pop_back<S>();
    } else {
        return S;
    }
}

template<auto S, char delim>
consteval auto split_impl() {
    constexpr int n = S.size();
    constexpr int c = count(S, delim);
    std::array<std::array<char, n>, c+1> res;
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

template<auto S, char delim>
consteval auto split() {
    return pop_back_if_empty<pop_back_if_empty<split_impl<S, delim>()>()>();
}

template<auto S>
consteval auto sanitize() {
    constexpr int n = S.size();
    std::array<char, n-count(S, ' ') - count(S, '\0')> res;
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

template<typename T>
concept Pydexable = requires(T x) { x[x.size()]; };


template<auto S, Pydexable Vt> requires(S.size() > 0)
struct Indexer : Vt {
    static constexpr bool is_slice = detail::count(S[0], ':') == 1;
private:
    constexpr auto next_impl(auto index) const {
        if constexpr (S.size() == 1) {
            return Vt::operator[](index);
        } else {
            auto m = reinterpret_cast<const Indexer<detail::pop_front<S>(),
                    std::decay_t<decltype(Vt::operator[](index))>>&>(Vt::operator[](index));
            if constexpr (detail::is_number<S[1]>()) {
                return m.operator auto();
            } else {
                return m;
            }
        }
    }
    constexpr auto next_impl() const {
        if constexpr (!is_slice) {
            constexpr auto index = detail::stoi<S[0]>();
            if constexpr (index < 0) {
                return next_impl(Vt::size() + index);
            } else return next_impl(index);
        }
    }

    constexpr auto index_impl(auto i) const requires(is_slice) {
        constexpr auto e = detail::split<S[0], ':'>();

        static_assert(e.size() <= 2, "Error");

        if constexpr (e.size() == 0) { // [:]
            return next(i);
        } else if constexpr (e.size() == 1) { // Either [A:] or [:A]
            if constexpr (S[0][0] == ':') {
                auto end = detail::stoi<e[0]>();
                if (end < 0) {
                    end = Vt::size() + end;
                }
                assert(i < end);
                return next(i);
            } else {
                auto start = detail::stoi<e[0]>();
                if (start < 0) {
                    start = Vt::size() + start;
                }
                return next(i + start);
            }
        } else { // [A:B]
            auto start = detail::stoi<e[0]>();
            auto end = detail::stoi<e[1]>();
            if (start < 0) {
                start = Vt::size() + start;
            }

            if (end < 0) {
                end = Vt::size() + end;
            }
            auto index = i + start;

            assert(index < end);
            return next(index);
        }
    }
public:

    constexpr auto next(auto index) {
        auto res = next_impl(index);
        return std::is_const_v<decltype(res)> ? const_cast<std::remove_const_t<decltype(res)>>(res) : res;
    }
    constexpr auto next(auto index) const {
        return next_impl(index);
    }

    constexpr auto next() const {
        return next_impl();
    }
    constexpr auto next() {
        if constexpr (!std::is_same_v<decltype(next_impl()), void>) {
            return next_impl();
        }
    }

    constexpr auto operator[](auto i) const requires(is_slice) {
        return index_impl(i);
    }

    constexpr auto operator[](auto i) requires(is_slice) {
        if constexpr (std::is_const_v<decltype(index_impl(0))>) {
            return const_cast<std::remove_const_t<decltype(index_impl(0))>>(index_impl(i));
        } else {
            return index_impl(i);
        }
    }

    size_t size() const requires(is_slice) {
        constexpr auto e = detail::split<S[0], ':'>();
        static_assert(e.size() <= 2, "Error");

        if constexpr (e.size() == 0) { // [:]
            return Vt::size();
        } else if constexpr (e.size() == 1) { // Either [A:] or [:A]
            if constexpr (S[0][0] == ':') {
                auto end = detail::stoi<e[0]>();
                if (end < 0) {
                    end = Vt::size() + end;
                }
                return Vt::size() - (Vt::size() - end);
            } else {
                auto start = detail::stoi<e[0]>();
                if (start < 0) {
                    start = Vt::size() + start;
                }
                return Vt::size() - start;
            }
        } else { // [A:B]
            auto start = detail::stoi<e[0]>();
            auto end = detail::stoi<e[1]>();
            if (start < 0) {
                start = Vt::size() + start;
            }

            if (end < 0) {
                end = Vt::size() + end;
            }

            if (start >= end) return 0;
            return end-start;
        }
    }


    size_t size() const requires(!is_slice) {
        return next().size();
    }

    auto operator[](auto i) requires(!is_slice) {
        return next()[i];
    }

    template<typename T> struct Iterator {
        T indexer;
        size_t i = 0;
        Iterator(T indexer, size_t start = 0) : indexer(indexer), i(start) { }

        auto operator*() {
            return indexer[i];
        }
        auto operator++() {
            ++i;
            return *this;
        }
        auto operator!=(const Iterator& other) {
            return i != other.i;
        }
    };
    auto begin() { return Iterator<Indexer&>(*this); }
    auto end() { return Iterator<Indexer&>(*this, size()); }
    auto begin() const { return Iterator<const Indexer&>(*this); }
    auto end() const { return Iterator<const Indexer&>(*this, size()); }


    constexpr operator auto() {
        return next();
    }
    constexpr operator auto() const {
        return next();
    }
};
}; // namespace detail

template <auto N> consteval auto expr(char const (&cstr)[N]) {
    std::array<char, N-1> arr;
    for (std::size_t i = 0; i < N-1; ++i)
        arr[i] = cstr[i];
    return arr;
}


template<auto s> constexpr auto pydex(detail::Pydexable auto& v) {
    return reinterpret_cast<detail::Indexer<detail::split<detail::sanitize<s>(), ','>(), std::decay_t<decltype(v)>>&>(v);
}
};


template<auto E, pydex::detail::Pydexable Vt> requires (pydex::detail::Indexer<E, Vt>::is_slice)
std::ostream& operator<<(std::ostream& os, const pydex::detail::Indexer<E, Vt>& v) {
    os << "[";
    for (auto i : v) {
        os << i << ", ";
    }
    os << "]";

    return os;
}

template<auto E, pydex::detail::Pydexable Vt> requires (!pydex::detail::Indexer<E, Vt>::is_slice)
std::ostream& operator<<(std::ostream& os, const pydex::detail::Indexer<E, Vt>& v) {
    os << v.next();
    return os;
}