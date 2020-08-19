#pragma once

namespace initregex {

template <typename... args> struct list;

template <typename list> struct size_of;

template <template <typename...> typename list> struct size_of<list<>> {
  constexpr static const std::size_t value = 0;
};

template <template <typename...> typename list, typename arg, typename... args>
struct size_of<list<arg, args...>> {
  constexpr static const std::size_t value = 1 + size_of<list<args...>>::value;
};

template <typename list, typename arg> struct prepend;

template <template <typename...> typename list, typename arg, typename... args>
struct prepend<list<args...>, arg> {
  using type = list<arg, args...>;
};

template <typename list, typename arg> struct append;

template <template <typename...> typename list, typename arg>
struct append<list<>, arg> {
  using type = list<arg>;
};

template <template <typename...> typename list, typename arg0, typename arg1,
          typename... args>
struct append<list<arg0, args...>, arg1> {
  using type = typename prepend<arg0, append<list<args...>, arg1>>::type;
};

template <typename list> struct last;

template <template <typename...> typename list, typename arg>
struct last<list<arg>> {
  using type = arg;
};

template <template <typename...> typename list, typename arg, typename... args>
struct last<list<arg, args...>> {
  using type = typename last<args...>::type;
};

template <typename list, std::size_t n> struct take;

template <template <typename...> typename list> struct take<list<>, 0> {
  using type = list<>;
};

template <template <typename...> typename list, typename arg, typename... args>
struct take<list<arg, args...>, 0> {
  using type = list<>;
};

template <template <typename...> typename list, std::size_t n, typename arg,
          typename... args>
struct take<list<arg, args...>, n> {
  using type =
      typename prepend<typename take<list<args...>, n - 1>::type, arg>::type;
};

template <typename list, std::size_t n> struct drop_right;

template <template <typename...> typename list, std::size_t n, typename arg,
          typename... args>
struct drop_right<list<arg, args...>, n> {
  using type = typename take<list<arg, args...>,
                             size_of<list<arg, args...>>::value - n>::type;
};

} // namespace initregex
