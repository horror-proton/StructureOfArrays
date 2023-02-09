#pragma once
#include <cstddef>
#include <iterator>
#include <tuple>
#include <utility>
#include <vector>

namespace detail {

template <std::size_t Tag, typename Value> struct tag_and_value {
  static constexpr auto tag = Tag;
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

template <typename M>
constexpr decltype(auto) meta_struct_get(std::remove_reference_t<M> &ms_self) {
  return (ms_self.value);
}

template <typename M>
constexpr decltype(auto)
meta_struct_get(const std::remove_reference_t<M> &ms_self) {
  return (ms_self.value);
}

template <typename M>
constexpr decltype(auto) meta_struct_get(std::remove_reference_t<M> &&ms_self) {
  return std::move(ms_self.value);
}

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
    return detail::meta_struct_get<Nth_member_type<N>>(*this);
  }

  template <std::size_t N> decltype(auto) get_array() const & {
    return detail::meta_struct_get<Nth_member_type<N>>(*this);
  }

  template <std::size_t N> decltype(auto) get_array() && {
    return detail::meta_struct_get<Nth_member_type<N>>(*this);
  }
};

template <template <typename> typename Array = std::vector, typename... T>
using structure_of_arrays =
    structure_of_arrays_impl<typename detail::type_enumerate<
        std::index_sequence_for<T...>,
        Array<T>...>::template sequence<detail::dummy, detail::tag_and_value>>;
