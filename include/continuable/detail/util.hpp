
/**

                        /~` _  _ _|_. _     _ |_ | _
                        \_,(_)| | | || ||_|(_||_)|(/_

                    https://github.com/Naios/continuable
                                   v2.0.0

  Copyright(c) 2015 - 2017 Denis Blank <denis.blank at outlook dot com>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files(the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions :

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
**/

#ifndef CONTINUABLE_DETAIL_UTIL_HPP_INCLUDED__
#define CONTINUABLE_DETAIL_UTIL_HPP_INCLUDED__

#include <tuple>
#include <type_traits>
#include <utility>

#include "continuable/detail/api.hpp"

namespace cti {
namespace detail {
/// Utility namespace which provides useful meta-programming support
namespace util {

/// \cond false
#define CTI__FOR_EACH_BOOLEAN_BIN_OP(CTI__OP__)                                \
  CTI__OP__(==)                                                                \
  CTI__OP__(!=) CTI__OP__(<=) CTI__OP__(>=) CTI__OP__(<) CTI__OP__(>)
#define CTI__FOR_EACH_BOOLEAN_UNA_OP(CTI__OP__) CTI__OP__(!)
#define CTI__FOR_EACH_INTEGRAL_BIN_OP(CTI__OP__)                               \
  CTI__OP__(*)                                                                 \
  CTI__OP__(/) CTI__OP__(+) CTI__OP__(-) CTI__FOR_EACH_BOOLEAN_BIN_OP(CTI__OP__)
#define CTI__FOR_EACH_INTEGRAL_UNA_OP(CTI__OP__)                               \
  CTI__OP__(~) CTI__FOR_EACH_BOOLEAN_UNA_OP(CTI__OP__)
/// \endcond

template <typename T, T Value>
struct constant : std::integral_constant<T, Value> {
/// \cond false
#define CTI__INST(CTI__OP)                                                     \
  template <typename OT, OT OValue>                                            \
  /*constexpr*/ auto operator CTI__OP(std::integral_constant<OT, OValue>)      \
      const noexcept {                                                         \
    return constant<decltype((Value CTI__OP OValue)),                          \
                    (Value CTI__OP OValue)>{};                                 \
  }
  CTI__FOR_EACH_INTEGRAL_BIN_OP(CTI__INST)
#undef CTI__INST
#define CTI__INST(CTI__OP)                                                     \
  /*constexpr*/ auto operator CTI__OP() const noexcept {                       \
    return constant<decltype((CTI__OP Value)), (CTI__OP Value)>{};             \
  }
  CTI__FOR_EACH_INTEGRAL_UNA_OP(CTI__INST)
#undef CTI__INST
  /// \endcond
};

template <bool Value>
struct constant<bool, Value> : std::integral_constant<bool, Value> {
/// \cond false
#define CTI__INST(CTI__OP)                                                     \
  template <typename OT, OT OValue>                                            \
  /*constexpr*/ auto operator CTI__OP(std::integral_constant<bool, OValue>)    \
      const noexcept {                                                         \
    return constant<bool, (Value CTI__OP OValue)>{};                           \
  }
  CTI__FOR_EACH_BOOLEAN_BIN_OP(CTI__INST)
#undef CTI__INST
#define CTI__INST(CTI__OP)                                                     \
  /*constexpr*/ auto operator CTI__OP() const noexcept {                       \
    return constant<bool, CTI__OP Value>{};                                    \
  }
  CTI__FOR_EACH_BOOLEAN_UNA_OP(CTI__INST)
#undef CTI__INST
  /// \endcond
};

template <bool Value>
using bool_constant = constant<bool, Value>;
template <std::size_t Value>
using size_constant = constant<std::size_t, Value>;

template <typename T, bool Value>
constexpr auto constant_of(std::integral_constant<T, Value> /*value*/ = {}) {
  return constant<T, Value>{};
}
template <std::size_t Value>
constexpr auto
size_constant_of(std::integral_constant<std::size_t, Value> /*value*/ = {}) {
  return size_constant<Value>{};
}
template <bool Value>
constexpr auto
bool_constant_of(std::integral_constant<bool, Value> /*value*/ = {}) {
  return bool_constant<Value>{};
}

#undef CTI__FOR_EACH_BOOLEAN_BIN_OP
#undef CTI__FOR_EACH_BOOLEAN_UNA_OP
#undef CTI__FOR_EACH_INTEGRAL_BIN_OP
#undef CTI__FOR_EACH_INTEGRAL_UNA_OP

/// Evaluates to the element at position I.
template <std::size_t I, typename... Args>
using at_t = decltype(std::get<I>(std::declval<std::tuple<Args...>>()));

/// Evaluates to an integral constant which represents the size
/// of the given pack.
template <typename... Args>
using size_of_t = size_constant<sizeof...(Args)>;

/// A tagging type for wrapping other types
template <typename... T>
struct identity {};
template <typename T>
struct identity<T> : std::common_type<T> {};

template <typename>
struct is_identity : std::false_type {};
template <typename... Args>
struct is_identity<identity<Args...>> : std::true_type {};

template <typename T>
identity<std::decay_t<T>> constexpr identity_of(T const& /*type*/) noexcept {
  return {};
}
template <typename... Args>
constexpr identity<Args...> identity_of(identity<Args...> /*type*/) noexcept {
  return {};
}
template <typename T>
constexpr auto identity_of() noexcept {
  return std::conditional_t<is_identity<std::decay_t<T>>::value, T,
                            identity<std::decay_t<T>>>{};
}

template <std::size_t I, typename... T>
constexpr auto get(identity<T...>) noexcept {
  return identity_of<at_t<I, T...>>();
}

/// Helper to trick compilers about that a parameter pack is used
template <typename... T>
void unused(T&&... args) {
  auto use = [](auto&& type) mutable {
    (void)type;
    return 0;
  };
  auto deduce = {0, use(std::forward<decltype(args)>(args))...};
  (void)deduce;
  (void)use;
}

namespace detail {
// Equivalent to C++17's std::void_t which targets a bug in GCC,
// that prevents correct SFINAE behavior.
// See http://stackoverflow.com/questions/35753920 for details.
template <typename...>
struct deduce_to_void : std::common_type<void> {};
} // end namespace detail

/// C++17 like void_t type
template <typename... T>
using void_t = typename detail::deduce_to_void<T...>::type;

namespace detail {
template <typename T, typename Check, typename = void_t<>>
struct is_valid_impl : std::common_type<std::false_type> {};

template <typename T, typename Check>
struct is_valid_impl<T, Check,
                     void_t<decltype(std::declval<Check>()(std::declval<T>()))>>
    : std::common_type<std::true_type> {};

template <typename Type, typename TrueCallback>
constexpr void static_if_impl(std::true_type, Type&& type,
                              TrueCallback&& trueCallback) {
  std::forward<TrueCallback>(trueCallback)(std::forward<Type>(type));
}

template <typename Type, typename TrueCallback>
constexpr void static_if_impl(std::false_type, Type&& /*type*/,
                              TrueCallback&& /*trueCallback*/) {
}

template <typename Type, typename TrueCallback, typename FalseCallback>
constexpr auto static_if_impl(std::true_type, Type&& type,
                              TrueCallback&& trueCallback,
                              FalseCallback&& /*falseCallback*/) {
  return std::forward<TrueCallback>(trueCallback)(std::forward<Type>(type));
}

template <typename Type, typename TrueCallback, typename FalseCallback>
constexpr auto static_if_impl(std::false_type, Type&& type,
                              TrueCallback&& /*trueCallback*/,
                              FalseCallback&& falseCallback) {
  return std::forward<FalseCallback>(falseCallback)(std::forward<Type>(type));
}
} // end namespace detail

/// Returns the pack size of the given type
template <typename... Args>
constexpr auto pack_size_of(identity<std::tuple<Args...>>) noexcept {
  return size_of_t<Args...>{};
}
/// Returns the pack size of the given type
template <typename First, typename Second>
constexpr auto pack_size_of(identity<std::pair<First, Second>>) noexcept {
  return size_of_t<First, Second>{};
}
/// Returns the pack size of the given type
template <typename... Args>
constexpr auto pack_size_of(identity<Args...>) noexcept {
  return size_of_t<Args...>{};
}

/// Returns an index sequence of the given type
template <typename T>
constexpr auto sequenceOf(T&& /*sequenceable*/) noexcept {
  return std::make_index_sequence<decltype(
      pack_size_of(std::declval<T>()))::value>();
}

/// Returns a check which returns a true type if the current value
/// is below the
template <std::size_t End>
constexpr auto isLessThen(size_constant<End> end) noexcept {
  return [=](auto current) { return end > current; };
}

/// Compile-time check for validating a certain expression
template <typename T, typename Check>
constexpr auto is_valid(T&& /*type*/, Check&& /*check*/) noexcept {
  return typename detail::is_valid_impl<T, Check>::type{};
}

/// Creates a static functional validator object.
template <typename Check>
constexpr auto validatorOf(Check&& check) noexcept(
    std::is_nothrow_move_constructible<std::decay_t<Check>>::value) {
  return [check = std::forward<Check>(check)](auto&& matchable) {
    return is_valid(std::forward<decltype(matchable)>(matchable), check);
  };
}

/// Invokes the callback only if the given type matches the check
template <typename Type, typename Check, typename TrueCallback>
constexpr void static_if(Type&& type, Check&& check,
                         TrueCallback&& trueCallback) {
  detail::static_if_impl(std::forward<Check>(check)(type),
                         std::forward<Type>(type),
                         std::forward<TrueCallback>(trueCallback));
}

/// Invokes the callback only if the given type matches the check
template <typename Type, typename Check, typename TrueCallback,
          typename FalseCallback>
constexpr auto static_if(Type&& type, Check&& check,
                         TrueCallback&& trueCallback,
                         FalseCallback&& falseCallback) {
  return detail::static_if_impl(std::forward<Check>(check)(type),
                                std::forward<Type>(type),
                                std::forward<TrueCallback>(trueCallback),
                                std::forward<FalseCallback>(falseCallback));
}

/// A compile-time while loop, which loops as long the value matches
/// the predicate. The handler shall return the next value.
template <typename Value, typename Predicate, typename Handler>
constexpr auto static_while(Value&& value, Predicate&& predicate,
                            Handler&& handler) {
  return static_if(std::forward<Value>(value), predicate,
                   [&](auto&& result) mutable {
                     return static_while(
                         handler(std::forward<decltype(result)>(result)),
                         std::forward<Predicate>(predicate),
                         std::forward<Handler>(handler));
                   },
                   [&](auto&& result) mutable {
                     return std::forward<decltype(result)>(result);
                   });
}

/// Returns a validator which checks whether the given sequenceable is empty
inline auto is_empty() noexcept {
  return [](auto const& checkable) {
    return pack_size_of(checkable) == size_constant_of<0>();
  };
}

/// Calls the given unpacker with the content of the given sequence
template <typename U, std::size_t... I>
constexpr auto unpack(std::integer_sequence<std::size_t, I...>, U&& unpacker) {
  return std::forward<U>(unpacker)(size_constant_of<I>()...);
}

/// Calls the given unpacker with the content of the given sequenceable
template <typename F, typename U, std::size_t... I>
constexpr auto unpack(F&& firstSequenceable, U&& unpacker,
                      std::integer_sequence<std::size_t, I...>) {
  using std::get;
  (void)firstSequenceable;
  return std::forward<U>(unpacker)(
      get<I>(std::forward<F>(firstSequenceable))...);
}
/// Calls the given unpacker with the content of the given sequenceable
template <typename F, typename S, typename U, std::size_t... IF,
          std::size_t... IS>
constexpr auto unpack(F&& firstSequenceable, S&& secondSequenceable,
                      U&& unpacker, std::integer_sequence<std::size_t, IF...>,
                      std::integer_sequence<std::size_t, IS...>) {
  using std::get;
  (void)firstSequenceable;
  (void)secondSequenceable;
  return std::forward<U>(unpacker)(
      get<IF>(std::forward<F>(firstSequenceable))...,
      get<IS>(std::forward<S>(secondSequenceable))...);
}
/// Calls the given unpacker with the content of the given sequenceable
template <typename F, typename U>
auto unpack(F&& firstSequenceable, U&& unpacker) {
  return unpack(std::forward<F>(firstSequenceable), std::forward<U>(unpacker),
                sequenceOf(identity_of(firstSequenceable)));
}
/// Calls the given unpacker with the content of the given sequenceables
template <typename F, typename S, typename U>
constexpr auto unpack(F&& firstSequenceable, S&& secondSequenceable,
                      U&& unpacker) {
  return unpack(std::forward<F>(firstSequenceable),
                std::forward<S>(secondSequenceable), std::forward<U>(unpacker),
                sequenceOf(identity_of(firstSequenceable)),
                sequenceOf(identity_of(secondSequenceable)));
}

/// Applies the handler function to each element contained in the sequenceable
template <typename Sequenceable, typename Handler>
constexpr void static_for_each_in(Sequenceable&& sequenceable,
                                  Handler&& handler) {
  unpack(
      std::forward<Sequenceable>(sequenceable), [&](auto&&... entries) mutable {
        auto consume = [&](auto&& entry) mutable {
          handler(std::forward<decltype(entry)>(entry));
          return 0;
        };
        // Apply the consume function to every entry inside the pack
        auto deduce = {0, consume(std::forward<decltype(entries)>(entries))...};
        (void)deduce;
        (void)consume;
      });
}

/// Adds the given type at the back of the left sequenceable
template <typename Left, typename Element>
constexpr auto push(Left&& left, Element&& element) {
  return unpack(std::forward<Left>(left), [&](auto&&... leftArgs) {
    return std::make_tuple(std::forward<decltype(leftArgs)>(leftArgs)...,
                           std::forward<Element>(element));
  });
}

/// Adds the element to the back of the identity
template <typename... Args, typename Element>
constexpr auto push(identity<Args...>, identity<Element>) noexcept {
  return identity<Args..., Element>{};
}

/// Removes the first element from the identity
template <typename First, typename... Rest>
constexpr auto pop_first(identity<First, Rest...>) noexcept {
  return identity<Rest...>{};
}

/// Returns the merged sequence
template <typename Left>
constexpr auto merge(Left&& left) {
  return std::forward<Left>(left);
}
/// Merges the left sequenceable with the right ones
template <typename Left, typename Right, typename... Rest>
constexpr auto merge(Left&& left, Right&& right, Rest&&... rest) {
  // Merge the left with the right sequenceable and
  // merge the result with the rest.
  return merge(unpack(std::forward<Left>(left), std::forward<Right>(right),
                      [&](auto&&... args) {
                        // Maybe use: template <template<typename...> class T,
                        // typename... Args>
                        return std::make_tuple(
                            std::forward<decltype(args)>(args)...);
                      }),
               std::forward<Rest>(rest)...);
}
/// Merges the left identity with the right ones
template <typename... LeftArgs, typename... RightArgs, typename... Rest>
constexpr auto merge(identity<LeftArgs...> /*left*/,
                     identity<RightArgs...> /*right*/, Rest&&... rest) {
  return merge(identity<LeftArgs..., RightArgs...>{},
               std::forward<Rest>(rest)...);
}

/// Combines the given arguments with the given folding function
template <typename F, typename First>
constexpr auto fold(F&& /*folder*/, First&& first) {
  return std::forward<First>(first);
}
/// Combines the given arguments with the given folding function
template <typename F, typename First, typename Second, typename... Rest>
auto fold(F&& folder, First&& first, Second&& second, Rest&&... rest) {
  auto res = folder(std::forward<First>(first), std::forward<Second>(second));
  return fold(std::forward<F>(folder), std::move(res),
              std::forward<Rest>(rest)...);
}

/// Returns a folding function using operator `&&`.
inline auto and_folding() noexcept {
  return [](auto&& left, auto&& right) {
    return std::forward<decltype(left)>(left) &&
           std::forward<decltype(right)>(right);
  };
}
/// Returns a folding function using operator `||`.
inline auto or_folding() noexcept {
  return [](auto&& left, auto&& right) {
    return std::forward<decltype(left)>(left) ||
           std::forward<decltype(right)>(right);
  };
}
/// Returns a folding function using operator `>>`.
inline auto seq_folding() noexcept {
  return [](auto&& left, auto&& right) {
    return std::forward<decltype(left)>(left) >>
           std::forward<decltype(right)>(right);
  };
}

/// Deduces to a std::false_type
template <typename T>
using fail = std::integral_constant<bool, !std::is_same<T, T>::value>;

namespace detail {
template <typename T, typename Args, typename = void_t<>>
struct is_invokable_impl : std::common_type<std::false_type> {};

template <typename T, typename... Args>
struct is_invokable_impl<
    T, std::tuple<Args...>,
    void_t<decltype(std::declval<T>()(std::declval<Args>()...))>>
    : std::common_type<std::true_type> {};
} // end namespace detail

/// Deduces to a std::true_type if the given type is callable with the arguments
/// inside the given tuple.
/// The main reason for implementing it with the detection idiom instead of
/// hana like detection is that MSVC has issues with capturing raw template
/// arguments inside lambda closures.
///
/// ```cpp
/// util::is_invokable_t<object, std::tuple<Args...>>
/// ```
template <typename T, typename Args>
using is_invokable_t = typename detail::is_invokable_impl<T, Args>::type;

namespace detail {
/// Forwards every element in the tuple except the last one
template <typename T>
auto forward_except_last(T&& sequenceable) {
  auto size = pack_size_of(identity_of(sequenceable)) - size_constant_of<1>();
  auto sequence = std::make_index_sequence<size.value>();

  return unpack(std::forward<T>(sequenceable),
                [](auto&&... args) {
                  return std::forward_as_tuple(
                      std::forward<decltype(args)>(args)...);
                },
                sequence);
}

/// We are able to call the callable with the arguments given in the tuple
template <typename T, typename... Args>
auto partial_invoke_impl(std::true_type, T&& callable,
                         std::tuple<Args...> args) {
  return unpack(std::move(args), [&](auto&&... arg) {
    return std::forward<T>(callable)(std::forward<decltype(arg)>(arg)...);
  });
}

/// We were unable to call the callable with the arguments in the tuple.
/// Remove the last argument from the tuple and try it again.
template <typename T, typename... Args>
auto partial_invoke_impl(std::false_type, T&& callable,
                         std::tuple<Args...> args) {

  // If you are encountering this assertion you tried to attach a callback
  // which can't accept the arguments of the continuation.
  //
  // ```cpp
  // continuable<int, int> c;
  // std::move(c).then([](std::vector<int> v) { /*...*/ })
  // ```
  static_assert(
      sizeof...(Args) > 0,
      "There is no way to call the given object with these arguments!");

  // Remove the last argument from the tuple
  auto next = forward_except_last(std::move(args));

  // Test whether we are able to call the function with the given tuple
  is_invokable_t<decltype(callable), decltype(next)> is_invokable;

  return partial_invoke_impl(is_invokable, std::forward<T>(callable),
                             std::move(next));
}

/// Shortcut - we can call the callable directly
template <typename T, typename... Args>
auto partial_invoke_impl_shortcut(std::true_type, T&& callable,
                                  Args&&... args) {
  return std::forward<T>(callable)(std::forward<Args>(args)...);
}

/// Failed shortcut - we were unable to invoke the callable with the
/// original arguments.
template <typename T, typename... Args>
auto partial_invoke_impl_shortcut(std::false_type failed, T&& callable,
                                  Args&&... args) {

  // Our shortcut failed, convert the arguments into a forwarding tuple
  return partial_invoke_impl(
      failed, std::forward<T>(callable),
      std::forward_as_tuple(std::forward<Args>(args)...));
}
} // end namespace detail

/// Partially invokes the given callable with the given arguments.
///
/// \note This function will assert statically if there is no way to call the
///       given object with less arguments.
template <typename T, typename... Args>
auto partial_invoke(T&& callable, Args&&... args) {
  // Test whether we are able to call the function with the given arguments.
  is_invokable_t<decltype(callable), std::tuple<Args...>> is_invokable;

  // The implementation is done in a shortcut way so there are less
  // type instantiations needed to call the callable with its full signature.
  return detail::partial_invoke_impl_shortcut(
      is_invokable, std::forward<T>(callable), std::forward<Args>(args)...);
}

// Class for making child classes non copyable
struct non_copyable {
  constexpr non_copyable() = default;
  non_copyable(non_copyable const&) = delete;
  constexpr non_copyable(non_copyable&&) = default;
  non_copyable& operator=(non_copyable const&) = delete;
  non_copyable& operator=(non_copyable&&) = default;
};

// Class for making child classes non copyable and movable
struct non_movable {
  constexpr non_movable() = default;
  non_movable(non_movable const&) = delete;
  constexpr non_movable(non_movable&&) = delete;
  non_movable& operator=(non_movable const&) = delete;
  non_movable& operator=(non_movable&&) = delete;
};

/// This class is responsible for holding an abstract copy- and
/// move-able ownership that is invalidated when the object
/// is moved to another instance.
class ownership {
  explicit constexpr ownership(bool acquired, bool frozen)
      : acquired_(acquired), frozen_(frozen) {
  }

public:
  constexpr ownership() : acquired_(true), frozen_(false) {
  }
  constexpr ownership(ownership const&) = default;
  ownership(ownership&& right) noexcept
      : acquired_(right.consume()), frozen_(right.is_frozen()) {
  }
  ownership& operator=(ownership const&) = default;
  ownership& operator=(ownership&& right) noexcept {
    acquired_ = right.consume();
    frozen_ = right.is_frozen();
    return *this;
  }

  // Merges both ownerships together
  ownership operator|(ownership const& right) const noexcept {
    return ownership(is_acquired() && right.is_acquired(),
                     is_frozen() || right.is_frozen());
  }

  constexpr bool is_acquired() const noexcept {
    return acquired_;
  }
  constexpr bool is_frozen() const noexcept {
    return frozen_;
  }

  void release() noexcept {
    assert(is_acquired() && "Tried to release the ownership twice!");
    acquired_ = false;
  }
  void freeze(bool enabled = true) noexcept {
    assert(is_acquired() && "Tried to freeze a released object!");
    frozen_ = enabled;
  }

private:
  bool consume() noexcept {
    if (is_acquired()) {
      release();
      return true;
    }
    return false;
  }

  /// Is true when the object is in a valid state
  bool acquired_ : 1;
  /// Is true when the automatic invocation on destruction is disabled
  bool frozen_ : 1;
};
} // namespace util
} // namespace detail
} // namespace cti

#endif // CONTINUABLE_DETAIL_UTIL_HPP_INCLUDED__
