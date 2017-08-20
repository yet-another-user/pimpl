#ifndef IMPL_PTR_DETAIL_FIRST_HPP
#define IMPL_PTR_DETAIL_FIRST_HPP

namespace detail
{
    template <typename first_type =void, typename...> struct first { using type = first_type; };

//    using alloc = typename std::conditional<
//                  sizeof...(arg_types) == 0,
//                  std::allocator<impl_type>,
//                  typename first<arg_types...>::type>::type;
};

#endif // IMPL_PTR_DETAIL_FIRST_HPP
