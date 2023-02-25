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

} // namespace detail

template <typename MemberInfo> class structure_of_arrays_impl;
template <typename... Members>
class structure_of_arrays_impl<detail::dummy<Members...>> : Members... {
public:
  static constexpr auto narray = sizeof...(Members);

  structure_of_arrays_impl() = default;

  template <std::size_t N>
  using Nth_member_type = std::tuple_element_t<N, std::tuple<Members...>>;

private:
  template <typename M> constexpr decltype(auto) get_member() & {
    return (static_cast<M &>(*this));
  }

  template <typename M> constexpr decltype(auto) get_member() const & {
    return (static_cast<const M &>(*this));
  }

  template <typename M> constexpr decltype(auto) get_member() && {
    return (static_cast<M &&>(*this));
  }

  template <typename M> constexpr decltype(auto) get_array() & {
    return (static_cast<M &>(*this).value);
  }

  template <typename M> constexpr decltype(auto) get_array() const & {
    return (static_cast<const M &>(*this).value);
  }

  template <typename M> constexpr decltype(auto) get_array() && {
    return std::move(static_cast<M &&>(*this).value);
  }

#define SOA_ARRAY_PARAM_PACK                                                   \
  std::forward<decltype(get_array<Members>())>(get_array<Members>())

public:
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
    (SOA_ARRAY_PARAM_PACK.resize(new_size), ...);
  }

  constexpr void reserve(std::size_t new_size) {
    (SOA_ARRAY_PARAM_PACK.reserve(new_size), ...);
  }

  constexpr void shrink_to_fit() {
    (SOA_ARRAY_PARAM_PACK.shrink_to_fit(), ...);
  }

  constexpr void clear() { (SOA_ARRAY_PARAM_PACK.clear(), ...); }

  constexpr void
  swap(structure_of_arrays_impl<detail::dummy<Members...>> &other) {
    (SOA_ARRAY_PARAM_PACK.swap(other.get_array<Members>()), ...);
  }

  template <typename... Ts> constexpr void push_back(Ts &&...args) {
    (SOA_ARRAY_PARAM_PACK.push_back(std::forward<Ts>(args)), ...);
  }

  template <typename... Ts> constexpr void pop_back() {
    (SOA_ARRAY_PARAM_PACK.pop_back(), ...);
  }

#undef SOA_ARRAY_PARAM_PACK
};

template <template <typename> typename Array = std::vector, typename... T>
using structure_of_arrays =
    structure_of_arrays_impl<typename detail::type_enumerate<
        std::index_sequence_for<T...>,
        Array<T>...>::template sequence<detail::dummy, detail::tag_and_value>>;
