// Copyright (c) 2006 Vladimir Batov.
// Use, modification and distribution are subject to the Boost Software License,
// Version 1.0. See http://www.boost.org/LICENSE_1_0.txt.

#ifndef BOOST_PIMPL_CONSTRUCTORS_DETAIL_HEADER_VB
#define BOOST_PIMPL_CONSTRUCTORS_DETAIL_HEADER_VB

// Series of constructors forwarding parameters down to the implementation
// class. That is done to encapsulate object construction inside that implementation
// class and, consequently, *fully* automate memory management (not just deletion).

#if defined(_MSC_VER) && 1500 <= _MSC_VER // Tested with Visual Studio 2008 v9.0.

#define BOOST_PIMPL_MANY_MORE_CONSTRUCTORS                                              \
                                                                                        \
    template<class A>                                                                   \
    explicit pimpl_base(A& a, BOOST_PIMPL_DEPLOY_IF_NOT_PIMPL_DERIVED(A))               \
    : impl_(new implementation(a)) {}                                                   \
    template<class A, class B>                                                          \
    pimpl_base(A& a, B& b)                                                              \
    : impl_(new implementation(a,b)) {}                                                 \
    template<class A, class B, class C>                                                 \
    pimpl_base(A& a, B& b, C& c)                                                        \
    : impl_(new implementation(a,b,c)) {}                                               \
    template<class A, class B, class C, class D>                                        \
    pimpl_base(A a, B b, C& c, D& d)                                                    \
    : impl_(new implementation(a,b,c,d)) {}                                             \
    template<class A, class B, class C, class D, class E>                               \
    pimpl_base(A& a, B& b, C& c, D& d, E& e)                                            \
    : impl_(new implementation(a,b,c,d,e)) {}                                           \
    template<class A, class B, class C, class D, class E, class F>                      \
    pimpl_base(A& a, B& b, C& c, D& d, E& e, F& f)                                      \
    : impl_(new implementation(a,b,c,d,e,f)) {}                                         \
    template<class A, class B, class C, class D, class E, class F, class G>             \
    pimpl_base(A& a, B& b, C& c, D& d, E& e, F& f, G& g)                                \
    : impl_(new implementation(a,b,c,d,e,f,g)) {}                                       \
    template<class A, class B, class C, class D, class E, class F, class G, class H>    \
    pimpl_base(A& a, B& b, C& c, D& d, E& e, F& f, G& g, H& h)                          \
    : impl_(new implementation(a,b,c,d,e,f,g,h)) {}

#else

// gcc 4.2.4 gets confused by the following:
// Foo create_foo() { ... return foo; }
//
// struct Moo : public pimpl<Moo>::pointer_semantics
// {
//      Moo() : base_type(create_foo()) {}
// };                ^^^^^^^^^^^^
// The above passes a temporary parameter (returned by create_foo()) and
// requires template<A1> pimpl_base(A1 const&) to kick in.
// However, gcc 4.2.4 gets confused as it tries to find a constructor for
// "base(Foo)" when in fact it should have been trying for "base(Foo const)".
// Consequently, it fails to pick the matching constructor
//      template<class A> pimpl_base::pimpl_base(A&)
// with A = "Foo const".
// Interestingly, gcc handles the following correctly:
// struct Moo : public pimpl<Moo>::pointer_semantics
// {
//      Moo() : base_type(Foo()) {}
// };                ^^^^^
// So, I have to resort to the juggling with consts below.

#undef  PIMPL_CONSTRUCTOR_2
#undef  PIMPL_CONSTRUCTOR_3
#undef  PIMPL_CONSTRUCTOR_4
#undef  PIMPL_CONSTRUCTOR_5

#define PIMPL_CONSTRUCTOR_2(c1, c2)     \
    template<class A1, class A2>        \
    pimpl_base(A1 c1& a1, A2 c2& a2)    \
    : impl_(new implementation(a1,a2)) {}

#define PIMPL_CONSTRUCTOR_3(c1, c2, c3)         \
    template<class A1, class A2, class A3>      \
    pimpl_base(A1 c1& a1, A2 c2& a2, A3 c3& a3) \
    : impl_(new implementation(a1,a2,a3)) {}

#define PIMPL_CONSTRUCTOR_4(c1, c2, c3, c4)                 \
    template<class A1, class A2, class A3, class A4>        \
    pimpl_base(A1 c1& a1, A2 c2& a2, A3 c3& a3, A4 c4& a4)  \
    : impl_(new implementation(a1,a2,a3,a4)) {}

#define PIMPL_CONSTRUCTOR_5(c1, c2, c3, c4, c5)                         \
    template<class A1, class A2, class A3, class A4, class A5>          \
    pimpl_base(A1 c1& a1, A2 c2& a2, A3 c3& a3, A4 c4& a4, A5 c5& a5)   \
    : impl_(new implementation(a1,a2,a3,a4,a5)) {}

#define BOOST_PIMPL_MANY_MORE_CONSTRUCTORS                                  \
                                                                            \
template<class A>                                                           \
explicit pimpl_base(A& a, BOOST_PIMPL_DEPLOY_IF_NOT_PIMPL_DERIVED(A))       \
: impl_(new implementation(a)) {}                                           \
template<class A>                                                           \
explicit pimpl_base(A const& a, BOOST_PIMPL_DEPLOY_IF_NOT_PIMPL_DERIVED(A)) \
: impl_(new implementation(a)) {}                                           \
                                                                            \
PIMPL_CONSTRUCTOR_2 (     ,     )                                           \
PIMPL_CONSTRUCTOR_2 (     ,const)                                           \
PIMPL_CONSTRUCTOR_2 (const,     )                                           \
PIMPL_CONSTRUCTOR_2 (const,const)                                           \
PIMPL_CONSTRUCTOR_3 (     ,     ,     )                                     \
PIMPL_CONSTRUCTOR_3 (     ,     ,const)                                     \
PIMPL_CONSTRUCTOR_3 (     ,const,     )                                     \
PIMPL_CONSTRUCTOR_3 (     ,const,const)                                     \
PIMPL_CONSTRUCTOR_3 (const,     ,     )                                     \
PIMPL_CONSTRUCTOR_3 (const,     ,const)                                     \
PIMPL_CONSTRUCTOR_3 (const,const,     )                                     \
PIMPL_CONSTRUCTOR_3 (const,const,const)                                     \
PIMPL_CONSTRUCTOR_4 (     ,     ,     ,     )                               \
PIMPL_CONSTRUCTOR_4 (     ,     ,     ,const)                               \
PIMPL_CONSTRUCTOR_4 (     ,     ,const,     )                               \
PIMPL_CONSTRUCTOR_4 (     ,     ,const,const)                               \
PIMPL_CONSTRUCTOR_4 (     ,const,     ,     )                               \
PIMPL_CONSTRUCTOR_4 (     ,const,     ,const)                               \
PIMPL_CONSTRUCTOR_4 (     ,const,const,     )                               \
PIMPL_CONSTRUCTOR_4 (     ,const,const,const)                               \
PIMPL_CONSTRUCTOR_4 (const,     ,     ,     )                               \
PIMPL_CONSTRUCTOR_4 (const,     ,     ,const)                               \
PIMPL_CONSTRUCTOR_4 (const,     ,const,     )                               \
PIMPL_CONSTRUCTOR_4 (const,     ,const,const)                               \
PIMPL_CONSTRUCTOR_4 (const,const,     ,     )                               \
PIMPL_CONSTRUCTOR_4 (const,const,     ,const)                               \
PIMPL_CONSTRUCTOR_4 (const,const,const,     )                               \
PIMPL_CONSTRUCTOR_4 (const,const,const,const)                               \
PIMPL_CONSTRUCTOR_5 (     ,     ,     ,     ,     )                         \
PIMPL_CONSTRUCTOR_5 (     ,     ,     ,     ,const)                         \
PIMPL_CONSTRUCTOR_5 (     ,     ,     ,const,     )                         \
PIMPL_CONSTRUCTOR_5 (     ,     ,     ,const,const)                         \
PIMPL_CONSTRUCTOR_5 (     ,     ,const,     ,     )                         \
PIMPL_CONSTRUCTOR_5 (     ,     ,const,     ,const)                         \
PIMPL_CONSTRUCTOR_5 (     ,     ,const,const,     )                         \
PIMPL_CONSTRUCTOR_5 (     ,     ,const,const,const)                         \
PIMPL_CONSTRUCTOR_5 (     ,const,     ,     ,     )                         \
PIMPL_CONSTRUCTOR_5 (     ,const,     ,     ,const)                         \
PIMPL_CONSTRUCTOR_5 (     ,const,     ,const,     )                         \
PIMPL_CONSTRUCTOR_5 (     ,const,     ,const,const)                         \
PIMPL_CONSTRUCTOR_5 (     ,const,const,     ,     )                         \
PIMPL_CONSTRUCTOR_5 (     ,const,const,     ,const)                         \
PIMPL_CONSTRUCTOR_5 (     ,const,const,const,     )                         \
PIMPL_CONSTRUCTOR_5 (     ,const,const,const,const)                         \
PIMPL_CONSTRUCTOR_5 (const,     ,     ,     ,     )                         \
PIMPL_CONSTRUCTOR_5 (const,     ,     ,     ,const)                         \
PIMPL_CONSTRUCTOR_5 (const,     ,     ,const,     )                         \
PIMPL_CONSTRUCTOR_5 (const,     ,     ,const,const)                         \
PIMPL_CONSTRUCTOR_5 (const,     ,const,     ,     )                         \
PIMPL_CONSTRUCTOR_5 (const,     ,const,     ,const)                         \
PIMPL_CONSTRUCTOR_5 (const,     ,const,const,     )                         \
PIMPL_CONSTRUCTOR_5 (const,     ,const,const,const)                         \
PIMPL_CONSTRUCTOR_5 (const,const,     ,     ,     )                         \
PIMPL_CONSTRUCTOR_5 (const,const,     ,     ,const)                         \
PIMPL_CONSTRUCTOR_5 (const,const,     ,const,     )                         \
PIMPL_CONSTRUCTOR_5 (const,const,     ,const,const)                         \
PIMPL_CONSTRUCTOR_5 (const,const,const,     ,     )                         \
PIMPL_CONSTRUCTOR_5 (const,const,const,     ,const)                         \
PIMPL_CONSTRUCTOR_5 (const,const,const,const,     )                         \
PIMPL_CONSTRUCTOR_5 (const,const,const,const,const)

// Implement more when needed.
// Hopefully more compilers will be as intelligent as VS C++ 9.0

#endif // _MSC_VER
#endif // BOOST_PIMPL_CONSTRUCTORS_DETAIL_HEADER_VB
