#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <type_traits>
template < template <typename...> class U, template <typename...> class V>
struct is_same_temptemplate : std::false_type
{};

template < template <typename...> class U>
struct is_same_temptemplate<U, U> : std::true_type
{};

template < template <typename...> class U, template <typename...> class V>
constexpr bool is_same_temptemplate_v = is_same_temptemplate<U,V>::value;


#endif;