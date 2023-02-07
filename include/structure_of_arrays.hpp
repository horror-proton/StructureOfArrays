#pragma once
#include <cstddef>
#include <iterator>
#include <utility>
#include <vector>
#include <tuple>

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

} // namespace detail

template <typename MemberInfo> class structure_of_arrays_impl;
template <typename... Members>
class structure_of_arrays_impl<detail::dummy<Members...>> : Members... {
public:
  structure_of_arrays_impl() = default;

  template <std::size_t N>
  using Nth_member_type = std::tuple_element_t<N, std::tuple<Members...>>;

  template <std::size_t N> decltype(auto) get_array() {
    return static_cast<Nth_member_type<N>>(*this).value;
  }
};

template <template <typename> typename Array = std::vector, typename... T>
using structure_of_arrays =
    structure_of_arrays_impl<typename detail::type_enumerate<
        std::index_sequence_for<T...>,
        Array<T>...>::template sequence<detail::dummy, detail::tag_and_value>>;
