#ifndef IMPL_PTR_DETAIL_IS_ALLOCATOR_HPP
#define IMPL_PTR_DETAIL_IS_ALLOCATOR_HPP

#include <boost/convert/detail/has_member.hpp>
#include <type_traits>

namespace detail
{
    template <typename first_type =void, typename...> struct first { using type = first_type; };

//    using alloc = typename std::conditional<
//                  sizeof...(arg_types) == 0,
//                  std::allocator<impl_type>,
//                  typename first<arg_types...>::type>::type;

    template<typename, typename =void>
    struct is_allocator { BOOST_STATIC_CONSTANT(bool, value = false); };

    template<typename class_type>
    struct is_allocator<class_type, typename std::enable_if<std::is_class<class_type>::value, void>::type>
    {
        BOOST_DECLARE_HAS_MEMBER(has_allocate, allocate);
        BOOST_DECLARE_HAS_MEMBER(has_deallocate, deallocate);

        BOOST_STATIC_CONSTANT(bool, value = has_allocate<class_type>::value && has_deallocate<class_type>::value);
    };
}

#endif // IMPL_PTR_DETAIL_IS_ALLOCATOR_HPP
