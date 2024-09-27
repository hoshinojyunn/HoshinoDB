#pragma once

#include <type_traits>

namespace HoshinoDB {
template <template <typename...> class C, typename... Ts>
std::true_type is_base_of_template_impl(const C<Ts...>*);

template <template <typename...> class C>
std::false_type is_base_of_template_impl(...);

// 判断T是否继承于带任意模板参数的C，构造T*类型，看是否能够转换为基类C指针
// 只能用于public继承
template <template <typename...> class C, typename T>
using is_base_of_template =
    decltype(is_base_of_template_impl<C>(std::declval<T*>()));

template <typename Comp, typename Elem, typename = void>
struct can_compare_with : std::false_type {};

template <typename Comp, typename Elem>
struct can_compare_with<Comp, Elem,
                        std::void_t<decltype(std::declval<Comp>().Compare(
                            std::declval<Elem>(), std::declval<Elem>()))>> {
  constexpr static bool value = true;
};

}  // namespace HoshinoDB
