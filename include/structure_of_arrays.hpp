#pragma once
#include <cstddef>
#include <functional>
#include <iterator>
#include <tuple>
#include <utility>
#include <vector>

namespace detail {

template <std::size_t Tag, typename Value> struct tag_and_value {
  static constexpr auto tag = Tag;
  using value_type = Value;

  Value value;
};

template <typename IdxSeq, typename... Ts> struct type_enumerate;

template <std::size_t... Is, typename... Ts>
struct type_enumerate<std::index_sequence<Is...>, Ts...> {
  template <template <typename...> typename Sequence,
            template <std::size_t, typename> typename Packer>
  using sequence = Sequence<Packer<Is, Ts>...>;
};

template <typename...> struct dummy {};

template <typename MetaStruct, typename F, typename... Args, std::size_t... Is>
inline constexpr void soa_for_each_impl(std::index_sequence<Is...>,
                                        MetaStruct &&ms_self, F &&func,
                                        Args &&...args) {
  (std::invoke(std::forward<F>(func),
               std::forward<MetaStruct>(ms_self).template get_array<Is>(),
               std::forward<Args>(args)...),
   ...);
}

// TODO: use Member parameter pack instead
template <std::size_t N> struct soa_transform_helper;

template <> struct soa_transform_helper<0> {
  template <std::size_t... Is, typename S, typename F, typename... Args>
  static constexpr auto apply(std::index_sequence<Is...>, S &&s_self, F &&f,
                              Args &&...args) {
    (std::invoke(std::forward<F>(f),
                 std::forward<S>(s_self).template get_array<Is>(),
                 std::forward<Args>(args)),
     ...);
  }
};

} // namespace detail

template <typename MemberInfo> class structure_of_arrays_impl;
template <typename... Members>
class structure_of_arrays_impl<detail::dummy<Members...>> : Members... {
public:
  static constexpr auto narray = sizeof...(Members);

  structure_of_arrays_impl() = default;

  template <std::size_t N>
  using Nth_member_type = std::tuple_element_t<N, std::tuple<Members...>>;

  // TODO: find better solution in C++23
  template <std::size_t N> decltype(auto) get_array() & {
    return (static_cast<Nth_member_type<N> &>(*this).value);
  }

  template <std::size_t N> constexpr decltype(auto) get_array() const & {
    return (static_cast<const Nth_member_type<N> &>(*this).value);
  }

  template <std::size_t N> constexpr decltype(auto) get_array() && {
    return std::move(static_cast<Nth_member_type<N> &&>(*this).value);
  }

  constexpr auto size() const { return get_array<0>().size(); }

  constexpr auto empty() const { return size() == 0; }

  constexpr void resize(std::size_t new_size) {
    detail::soa_for_each_impl(
        std::make_index_sequence<narray>{}, *this,
        [](auto &&array, std::size_t s) {
          std::forward<decltype(array)>(array).resize(s);
        },
        new_size);
  }

  constexpr void reserve(std::size_t new_size) {
    detail::soa_for_each_impl(
        std::make_index_sequence<narray>{}, *this,
        [](auto &&array, std::size_t s) {
          std::forward<decltype(array)>(array).reserve(s);
        },
        new_size);
  }

  constexpr void shrink_to_fit() {
    detail::soa_for_each_impl(
        std::make_index_sequence<narray>{}, *this, [](auto &&array) {
          std::forward<decltype(array)>(array).shrink_to_fit();
        });
  }

  constexpr void clear() {
    detail::soa_for_each_impl(
        std::make_index_sequence<narray>{}, *this,
        [](auto &&array) { std::forward<decltype(array)>(array).clear(); });
  }

  constexpr void
  swap(structure_of_arrays_impl<detail::dummy<Members...>> &other) {
    // FIXME
  }

  template <typename... Ts> constexpr void push_back(Ts &&...args) {
    detail::soa_transform_helper<0>::apply(
        std::make_index_sequence<narray>{}, *this,
        [](auto &&arr, auto &&val) {
          std::forward<decltype(arr)>(arr).push_back(
              std::forward<decltype(val)>(val));
        },
        std::forward<Ts>(args)...);
  }

  template <typename... Ts> constexpr void pop_back() {
    detail::soa_for_each_impl(
        std::make_index_sequence<narray>{}, *this,
        [](auto &&array) { std::forward<decltype(array)>(array).pop_back(); });
  }
};

template <template <typename> typename Array = std::vector, typename... T>
using structure_of_arrays =
    structure_of_arrays_impl<typename detail::type_enumerate<
        std::index_sequence_for<T...>,
        Array<T>...>::template sequence<detail::dummy, detail::tag_and_value>>;
