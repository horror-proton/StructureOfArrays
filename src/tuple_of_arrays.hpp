#include <tuple>
#include <type_traits>
#include <vector>

namespace detail {

#ifdef __cpp_lib_type_identity
using std::type_identity;
#else
template<typename T>
struct type_identity {
    using type = T;
};
#endif

template<std::size_t N, typename ...Ts>
struct parameter_pack_Nth {
    static_assert(sizeof...(Ts) /* always false */, "parameter pack index out of range");
};

template<typename First, typename ...Rest>
struct parameter_pack_Nth<0, First, Rest...> : type_identity<First> {
};

template<std::size_t N, typename First, typename ...Rest>
struct parameter_pack_Nth<N, First, Rest...> : type_identity<typename parameter_pack_Nth<N - 1, Rest...>::type> {
};

}

namespace detail {

template<class TupleT, typename Func, typename ...Args, std::size_t ...I>
constexpr auto tuple_for_each_impl(std::index_sequence<I...>, TupleT &t, Func f, Args &&...args) {
    using std::get;
    (f(get<I>(t), std::forward<Args>(args)...), ...); // TODO: what to return?
}

template<class TupleT, typename Func, typename ...Args>
constexpr auto tuple_for_each(TupleT &t, Func f/* [](auto &e, Args...) { ... } */, Args &&...args) {
    tuple_for_each_impl(std::make_index_sequence<std::tuple_size_v<TupleT>>{}, t, f, std::forward<Args>(args)...);
}

}

namespace detail {

template<std::size_t CommArgNum = 0>
struct tuple_transform_helper;

template<>
struct tuple_transform_helper<0> {
    template<class TupleT, typename Func, typename ...Elems, std::size_t ...I>
    static constexpr auto apply(std::index_sequence<I...>, TupleT &t, Func f, Elems &&...elems) {
        using std::get;
        (f(get<I>(t), std::forward<Elems>(elems)), ...);
    }
};

template<>
struct tuple_transform_helper<1> {
    template<class TupleT, typename Func, typename ...Elems, std::size_t ...I>
    static constexpr auto apply(std::size_t it, std::index_sequence<I...>, TupleT &t, Func f, Elems &&...elems) {
        using std::get;
        (f(get<I>(t), it, std::forward<Elems>(elems)), ...);
    }
};

}

namespace detail::force_adl {

template<std::size_t N, typename Arg>
constexpr auto &get(Arg &arg) {
    return get<N>(arg);
}

}

template<typename ...Elems>
class tuple_of_arrays {
public:

    template<typename ...Ts>
    using tuple_template = std::tuple<Ts...>;

    template<typename T>
    using array_template = std::vector<T>;

    using tuple_type = tuple_template<array_template<Elems>...>;

    template<std::size_t I>
    using Nth_value_type = typename detail::parameter_pack_Nth<I, Elems...>::type;

    template<std::size_t I>
    using Nth_array_type = array_template<typename detail::parameter_pack_Nth<I, Elems...>::type>;

    using size_type = typename Nth_array_type<0>::size_type;

    static constexpr std::size_t tuple_size = sizeof...(Elems);

public:

    tuple_of_arrays() = default;

    explicit tuple_of_arrays(size_type num) : m_tuple({array_template<Elems>(num)...}) {
    }

    // TODO: other constructors, assign operators and swap

    [[nodiscard]] constexpr size_type size() const noexcept { return get_array<0>().size(); }

    constexpr void resize(size_type new_size) {
        detail::tuple_for_each(m_tuple, [](auto &arr, size_type n) { arr.resize(n); }, new_size);
    }

    constexpr void shrink_to_fit() {
        detail::tuple_for_each(m_tuple, [](auto &arr) { arr.shrink_to_fit(); });
    }

    [[nodiscard]] constexpr bool empty() const noexcept { return get_array<0>().empty(); }

    constexpr void reserve(size_type n) {
        detail::tuple_for_each(m_tuple, [](auto &arr, size_type n) { arr.reserve(n); }, n);
    }

    template<std::size_t Array>
    const Nth_array_type<Array> &get_array() const { return detail::force_adl::get<Array>(m_tuple); }

    template<std::size_t Array>
    [[nodiscard]] constexpr const Nth_value_type<Array> &get(size_type n) const { return get<Array>(m_tuple)[n]; }

    template<std::size_t Array>
    [[nodiscard]] constexpr Nth_value_type<Array> &get(size_type n) { return get<Array>(m_tuple)[n]; }

    template<typename ...Args>
    constexpr void push_back(Args &&...elems) {
        static_assert(sizeof...(Args) == sizeof...(Elems));
        detail::tuple_transform_helper<0>::apply(
                std::make_index_sequence<tuple_size>{},
                m_tuple,
                [](auto &arr, auto &&e) { arr.push_back(std::forward<decltype(e)>(e)); },
                std::forward<Args>(elems)...);
    }

    constexpr void pop_back() noexcept {
        detail::tuple_for_each(m_tuple, [](auto &arr) { arr.pop_back(); });
    }

    template<typename ...Args>
    constexpr void insert(size_type position, Args &&...elems) {
        detail::tuple_transform_helper<1>::apply(
                position,
                std::make_index_sequence<tuple_size>{},
                m_tuple,
                [](auto &arr, size_type pos, auto &&e) { arr.insert(arr.begin() + pos, std::forward<decltype(e)>(e)); },
                std::forward<Args>(elems)...);
    }

    constexpr void clear() noexcept {
        detail::tuple_for_each(m_tuple, [](auto &arr) { arr.clear(); });
    }

private:
    tuple_type m_tuple;
};
