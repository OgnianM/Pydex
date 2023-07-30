#pragma once
#include <array>
#include <ostream>

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
            return !(neg && i == 1) && i > 0;
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

template<auto S, char delim>
consteval auto split() {
    constexpr int n = S.size();
    constexpr int c = count(S, delim);
    std::array<std::array<char, n>, c+1> res{'\0'};
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

template<typename T>
concept SizedIterable = requires(T x) { x.size(); x.begin(); x.end(); };

template<typename T>
concept Resizable = requires(T x) { x.resize(0); };

template<typename A, typename B>
concept Assignable = requires(A a, B b) { a = b; };

template<typename T>
concept Decayable = requires(T x) { x.decay(); };

template<typename T> requires (!Pydexable<T> && !SizedIterable<T>)
consteval int dimensionality() {
    return 0;
}
template<typename T> requires (Pydexable<T>)
consteval int dimensionality() {
    return 1 + dimensionality<decltype(std::declval<T>()[0])>();
}

template<typename T> requires (!Pydexable<T> && SizedIterable<T>)
consteval int dimensionality() {
    return 1 + dimensionality<decltype(*std::declval<T>().begin())>();
}

template<auto S, int K, int Default>
consteval int get_if_number() {
    if constexpr (K < S.size()) {
        if constexpr (S[K][0] == '\0' || S[K][0] == '.') {
            return Default;
        } else return stoi<S[K]>();
    } else {
        return Default;
    }
}

constexpr auto& deconst(const auto& x) {
    return const_cast<std::decay_t<decltype(x)>&>(x);
}

template<auto S, Pydexable Vt> requires(S.size() > 0)
struct Indexer :  Vt {
    static constexpr int colon_count = count(S[0], ':');
    static constexpr bool is_ellipsis = S[0][0] == '.' && S[0][1] == '.' && S[0][2] == '.';
    static constexpr bool is_slice = colon_count > 0 || is_ellipsis;
    static constexpr int dim = dimensionality<Vt>();
    static constexpr auto tokenized = split<S[0], ':'>();
    static constexpr int step = get_if_number<tokenized, 2, 1>();
    static constexpr int first = get_if_number<tokenized, 0, step < 0 ? -1 : 0>();
    static constexpr int last = get_if_number<tokenized, 1, step < 0 ? 0 : -1>();

    static_assert(colon_count <= 2, "Too many colons");
    static_assert(step != 0, "Step cannot be 0");
    static_assert(S.size() <= dim, "Too many indices");

    constexpr int get_last() const {
        if constexpr (last < 0) {
            return Vt::size() + last;
        } else return last;
    }

    constexpr int get_first() const {
        if constexpr (first < 0) {
            return Vt::size() + first;
        } else return first;
    }

    Indexer(const Indexer& other) = delete;
    constexpr Indexer& operator=(const auto& other) {
        return assignment_impl(other);
    }
    template<typename T> constexpr Indexer& operator=(const std::initializer_list<T>& init) {
        return assignment_impl(init);
    }

    constexpr size_t size() const {
        if constexpr (is_slice) {
            return std::max((get_last() - get_first() + step) / step, 0);
        } else return next().size();
    }

    constexpr auto &operator[](auto i) const {
        return index_impl(i);
    }

    constexpr auto& operator[](auto i) {
        return deconst(index_impl(i));
    }

    template<typename T> struct Iterator {
        T& indexer;
        size_t i = 0;
        Iterator(T& indexer, size_t start = 0) : indexer(indexer), i(start) { }

        auto& operator*() {
            return indexer[i];
        }
        Iterator& operator++() {
            ++i;
            return *this;
        }
        bool operator!=(const Iterator& other) {
            return i != other.i;
        }
    };
    auto begin() { return Iterator<Indexer&>(*this); }
    auto end() { return Iterator<Indexer&>(*this, Indexer::size()); }
    auto begin() const { return Iterator<const Indexer&>(*this); }
    auto end() const { return Iterator<const Indexer&>(*this, Indexer::size()); }

    constexpr const auto& decay() const {
        return decay_impl();
    }

    constexpr auto& decay() {
        return deconst(decay_impl());
    }

    constexpr auto& next() const requires(!is_slice) {
        constexpr auto index = stoi<S[0]>();
        if constexpr (index < 0) {
            return next_impl(Vt::size() + index);
        } else return next_impl(index);
    }

private:
    constexpr auto& next(auto index) const {
        return next_impl(index);
    }

    constexpr auto& next(auto index) {
        return deconst(next_impl(index));
    }

    constexpr Indexer& assignment_impl(const auto& other) {
        constexpr int dims_this = dim;
        constexpr int dims_other = dimensionality<decltype(other)>();
        static_assert(dims_other <= dims_this, "Cannot assign a higher dimensional object to a lower dimensional one");

        if constexpr (!is_slice) {
            deconst(next()) = other;
            return *this;
        } else if constexpr (dims_other == dims_this) {
            if (size() < other.size()) {
                if constexpr (Resizable<Vt>) {
                    Vt::resize(other.size());
                } else {
                    throw std::runtime_error("Cannot assign to a smaller non-resizable container");
                }
            }
            if constexpr (SizedIterable<decltype(other)>) {
                int i = 0;
                for (const auto& j : other) {
                    (*this)[i++] = j;
                }
            } else {
                for (int i = 0; i < other.size(); ++i) {
                    (*this)[i] = other[i];
                }
            }

        } else if constexpr (dims_other <= dims_this) {
            for (int i = 0; i < size(); ++i) {
                (*this)[i] = other;
            }
        }
        return *this;
    }

    constexpr auto& decay_once() const {
        return reinterpret_cast<const Vt&>(*this);
    }

    template<typename T = Vt> constexpr auto& decay_impl() const {
        auto& v = decay_once();
        if constexpr (Decayable<T>) {
            return v.decay();
        } else return v;
    }

    constexpr auto& next_impl(int index) const {
        if (index < 0) {
            index = Vt::size() + index;
        }

        if (index > Vt::size()) {
            throw std::out_of_range("Index out of range");
        }
        auto& k = decay_once();

        if constexpr (S.size() == 1) {
            if constexpr (is_ellipsis && dim > 1) {
                return reinterpret_cast<const Indexer<S, std::decay_t<decltype(k[index])>>&>(k[index]);
            } else {
                return k[index];
            }
        } else {
            if constexpr (is_ellipsis && dim > S.size()) {
                return reinterpret_cast<const Indexer<S, std::decay_t<decltype(k[index])>>&>(k[index]);
            } else {
                auto &m = reinterpret_cast<const Indexer<pop_front<S>(), std::decay_t<decltype(k[index])>> &>(k[index]);
                if constexpr (is_number<S[1]>()) {
                    return m.next();
                } else {
                    return m;
                }
            }
        }
    }

    constexpr auto& index_impl(int i) const {
        if constexpr (is_slice) {
            if (i < 0) {
                i = int(size()) + i;
                if (i < 0) {
                    throw std::out_of_range("Index out of range");
                }
            }
            return next_impl(first + i * step);
        } else {
            return next()[i];
        }
    }
};
}; // namespace detail

template <auto N> consteval auto expr(char const (&cstr)[N]) {
    std::array<char, N-1> arr;
    for (std::size_t i = 0; i < N-1; ++i)
        arr[i] = cstr[i];
    return arr;
}


template<auto S> constexpr auto& index(detail::Pydexable auto& v) {
    return reinterpret_cast<detail::Indexer<detail::split<detail::sanitize<S>(), ','>(), std::decay_t<decltype(v)>>&>(v);
}
template<auto S> constexpr auto& index(const detail::Pydexable auto& v) {
    return reinterpret_cast<const detail::Indexer<detail::split<detail::sanitize<S>(), ','>(), const std::decay_t<decltype(v)>>&>(v);
}

/// @return a deep copy of the given object
constexpr auto copy(const auto& v) {
    auto& a = pydex::index<pydex::expr("...")>(v);
    std::decay_t<decltype(a.decay())> b;
    pydex::index<pydex::expr("...")>(b) = a;
    return b;
}

};

template<auto E, pydex::detail::Pydexable Vt>
std::ostream& operator<<(std::ostream& os, const pydex::detail::Indexer<E, Vt>& v) {
    os << "[";
    for (int j = 0; j < v.size(); j++) {
        auto& i = v[j];
        os << i;
        if (j == v.size() - 1) {
            break;
        }
        if constexpr (pydex::detail::Indexer<E, Vt>::dim == 1) {
            os << ", ";
        } else {
            os << std::endl;
        }
    }

    os << "]";

    if (v.dim > 1) {
        os << std::endl;
    }

    return os;
}
