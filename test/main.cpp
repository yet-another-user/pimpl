#include "./test.hpp"

static
void
test_basics()
{
    Book book("Dune", "Frank Herbert");

    BOOST_TEST(book.title() == "Dune");
    BOOST_TEST(book.author() == "Frank Herbert");
}

static
void
test_is_pimpl()
{
    BOOST_TEST(false == boost::is_impl_ptr<Foo>::value);
    BOOST_TEST(false == boost::is_impl_ptr<int>::value);
    BOOST_TEST(false == boost::is_impl_ptr<int*>::value);
    BOOST_TEST(true  == boost::is_impl_ptr<Shared>::value);
    BOOST_TEST(true  == boost::is_impl_ptr<Shared const>::value);
    BOOST_TEST(false == boost::is_impl_ptr<Shared*>::value);
    BOOST_TEST(true  == boost::is_impl_ptr<Copied>::value);
    BOOST_TEST(true  == boost::is_impl_ptr<Base>::value);
    BOOST_TEST(true  == boost::is_impl_ptr<Derived1>::value);
    BOOST_TEST(true  == boost::is_impl_ptr<Derived1 const>::value);
    BOOST_TEST(true  == boost::is_impl_ptr<Derived2>::value);
}

static
void
test_swap()
{
    Shared s01 (1);
    Shared s02 (2);
    Copied c03 (5);
    Copied c04 (6);
    void const* p01 = &*s01;
    void const* p02 = &*s02;
    void const* p03 = &*c03;
    void const* p04 = &*c04;

    BOOST_TEST(s01.trace() == "Shared(int)");
    BOOST_TEST(s02.trace() == "Shared(int)");
    BOOST_TEST(s01 != s02);
    BOOST_TEST(&*s01 != &*s02);
    BOOST_TEST(c03.trace() == "Copied(int)");
    BOOST_TEST(c04.trace() == "Copied(int)");
    BOOST_TEST(&*c03 != &*c04);

    BOOST_TEST(s01.value() == 1);
    BOOST_TEST(s02.value() == 2);
    BOOST_TEST(&*s01 == p01);
    BOOST_TEST(&*s02 == p02);
    BOOST_TEST(c03.value() == 5);
    BOOST_TEST(c04.value() == 6);
    BOOST_TEST(&*c03 == p03);
    BOOST_TEST(&*c04 == p04);

    s01.swap(s02);
    c03.swap(c04);

    BOOST_TEST(s01.value() == 2);
    BOOST_TEST(s02.value() == 1);
    BOOST_TEST(&*s01 == p02);
    BOOST_TEST(&*s02 == p01);
    BOOST_TEST(c03.value() == 6);
    BOOST_TEST(c04.value() == 5);
    BOOST_TEST(&*c03 == p04);
    BOOST_TEST(&*c04 == p03);
}

static
void
test_runtime_polymorphic_behavior()
{
    Base        base1 (1);
    Derived1 derived1 (1, 2);
    Derived2 derived2 (1, 2, 3);
    Base        base2 (derived1);
    Base        base3 (derived2);
    Base        base4 (Derived2(2,3,4)/*const ref to temporary*/);
    Base*         bs1 = &base1;
    Base*         bs2 = &base2;
    Base*         bp3 = &base3;
    Base*         bp4 = &derived1;
    Base*         bp5 = &derived2;
    Base         bad1 = boost::impl_ptr<Base>::null();
    Base         bad2 = boost::impl_ptr<Base>::null();
    Base         bad3 = boost::impl_ptr<Derived1>::null();
    Base         bad4 = boost::impl_ptr<Derived2>::null();
    Derived1     bad5 (boost::impl_ptr<Derived1>::null());
    Derived2     bad6 (boost::impl_ptr<Derived2>::null());

    BOOST_TEST(derived1.trace() == "Derived1::implementation(int, int)");
    BOOST_TEST(derived2.trace() == "Derived2::implementation(int, int, int)");

    BOOST_TEST(base1 != derived1);
    BOOST_TEST(base1 != derived2);
    BOOST_TEST(base2 == derived1);
    BOOST_TEST(base3 == derived2);
    BOOST_TEST(derived1 != base1);
    BOOST_TEST(derived2 != base1);
    BOOST_TEST(derived1 == base2);
    BOOST_TEST(derived2 == base3);

    bool bad1_bool1 =   bad5 ? true : false; // Shared conversion to bool
    bool bad1_bool2 = !!bad5; // Shared operator!()
    bool bad2_bool1 =   bad6 ? true : false; // Shared conversion to bool
    bool bad2_bool2 = !!bad6; // Shared operator!()

    BOOST_TEST(!bad1_bool1);
    BOOST_TEST(!bad1_bool2);
    BOOST_TEST(!bad2_bool1);
    BOOST_TEST(!bad2_bool2);

    BOOST_TEST(base2.call_virtual() == "Derived1::call_virtual()");
    BOOST_TEST(base3.call_virtual() == "Derived2::call_virtual()");
    BOOST_TEST(bs1 ->call_virtual() == "Base::call_virtual()");
    BOOST_TEST(bs2 ->call_virtual() == "Derived1::call_virtual()");
    BOOST_TEST(bp3 ->call_virtual() == "Derived2::call_virtual()");
    BOOST_TEST(bp4 ->call_virtual() == "Derived1::call_virtual()");
    BOOST_TEST(bp5 ->call_virtual() == "Derived2::call_virtual()");
}

static
void
test_null()
{
    Shared s01 = boost::impl_ptr<Shared>::null(); BOOST_TEST(s01.trace() == "null");
    Shared s02 (boost::impl_ptr<Shared>::null()); BOOST_TEST(s02.trace() == "null");
    Copied c01 = boost::impl_ptr<Copied>::null(); BOOST_TEST(c01.trace() == "null");
    Copied c02 (boost::impl_ptr<Copied>::null()); BOOST_TEST(c02.trace() == "null");

    Base     p03 (boost::impl_ptr<    Base>::null()); BOOST_TEST(p03.trace() == "null");
    Derived1 p04 (boost::impl_ptr<Derived1>::null()); BOOST_TEST(p04.trace() == "null");
    Derived2 p05 (boost::impl_ptr<Derived2>::null()); BOOST_TEST(p05.trace() == "null");
    Base     p06 (p03); BOOST_TEST(p06.trace() == "null");
    Base     p07 (p04); BOOST_TEST(p07.trace() == "null");
    Base     p08 (p05); BOOST_TEST(p08.trace() == "null");
}

static
void
test_shared()
{
    Foo              foo;
    Foo const  const_foo = foo;
    Foo&             ref = foo;
    Foo const& const_ref = const_foo;
    Foo*             ptr = &foo;
    Foo const* const_ptr = &const_foo;

    Shared s11;            BOOST_TEST(s11.trace() == "Shared()");
    Shared s12 (5);        BOOST_TEST(s12.trace() == "Shared(int)");
    Shared s15 (5, 6);     BOOST_TEST(s15.trace() == "Shared(int, int)");
    Shared s13 = s12;      BOOST_TEST(&*s13 == &*s12); // Implementation shared.
    Shared s14 (s12);      BOOST_TEST(&*s14 == &*s12); // Implementation shared.
                           BOOST_TEST(s13.trace() == "Shared(int)"); // trace state is the same
                           BOOST_TEST(s14.trace() == "Shared(int)"); // trace state is the same
    Shared s21(foo);       BOOST_TEST(s21.trace() == "Shared(Foo&)");
    Shared s22(const_foo); BOOST_TEST(s22.trace() == "Shared(Foo const&)");
    Shared s23(ref);       BOOST_TEST(s23.trace() == "Shared(Foo&)");
    Shared s24(const_ref); BOOST_TEST(s24.trace() == "Shared(Foo const&)");
    Shared s25(ptr);       BOOST_TEST(s25.trace() == "Shared(Foo*)");
    Shared s26(const_ptr); BOOST_TEST(s26.trace() == "Shared(Foo const*)");

    Shared s16 (test::singleton); BOOST_TEST(s16.trace() == "Shared()");
    Shared s17 (test::singleton); BOOST_TEST(s17.trace() == "Shared()");
                                  BOOST_TEST(&*s17 == &*s16); // Implementation shared.
    Shared s31;
    Shared s32 (5);
    Shared s33 = s32;

    BOOST_TEST(s32 != s31); // calls impl_ptr::op!=()
    BOOST_TEST(s32 == s33); // calls impl_ptr::op==()
}

static
void
test_copied()
{
    Copied c11;       BOOST_TEST(c11.trace() == "Copied()");
    Copied c12 (5);   BOOST_TEST(c12.trace() == "Copied(int)");
    Copied c13 = c12; BOOST_TEST(c13.trace() == "Copied(Copied const&)");
    Copied c14 (c12); BOOST_TEST(c14.trace() == "Copied(Copied const&)");
                      BOOST_TEST(&*c13 != &*c12); // Copied.
                      BOOST_TEST(&*c14 != &*c12); // Copied.

    Copied c21 (3);
    Copied c22 (5);

    BOOST_TEST(&*c21 != &*c22);
    BOOST_TEST(c21 != c22);

    c21 = c22;

    BOOST_TEST(c21.trace() == "Copied::operator=(Copied const&)");
    BOOST_TEST(&*c21 != &*c22);
    BOOST_TEST(c21 == c22);

    Copied c31;
    Copied c32 (5);
    Copied c33 = c32;

    BOOST_TEST(c32 == c33);
    BOOST_TEST(c32.trace() == "Copied::operator==(Copied const&)");
    BOOST_TEST(c32 != c31);
    BOOST_TEST(c32.trace() == "Copied::operator==(Copied const&)");
}

static
void
test_unique()
{
    Unique u11 (3); BOOST_TEST(u11.value() == 3);
    Unique u12 (5); BOOST_TEST(u12.value() == 5);

//  u11 = u12; // Properly does not compile
    u11 = Unique(6); BOOST_TEST(u11.value() == 6);
}

static
void
test_onstack()
{
    OnStack s11 (3); BOOST_TEST(s11.value() == 3);
    OnStack s12 (5); BOOST_TEST(s12.value() == 5);
    OnStack s13 = boost::impl_ptr<OnStack>::null();

    // Check that implementation is allocated on the stack.
    BOOST_TEST((void*) &s11 == (void*) &*s11);
    BOOST_TEST(!bool(s13)); // Test op bool()
    BOOST_TEST(!s13);       // Test op!()

    s11 = s12;          BOOST_TEST(s11.value() == 5);
    s11 = OnStack(6);   BOOST_TEST(s11.value() == 6);
}

static
void
test_bool_conversions()
{
    Shared s1;
    Copied c1;
    Shared s2 = boost::impl_ptr<Shared>::null();
    Copied c2 = boost::impl_ptr<Copied>::null();

    BOOST_TEST(s1.trace() == "Shared()");
    BOOST_TEST(c1.trace() == "Copied()");
    BOOST_TEST(s2.trace() == "null");
    BOOST_TEST(c2.trace() == "null");

    BOOST_TEST(bool(s1));   // Test op bool()
    BOOST_TEST(!!s1);       // Test op!()
    BOOST_TEST(bool(c1));
    BOOST_TEST(!!c1);
    BOOST_TEST(!bool(s2));
    BOOST_TEST(!s2);
    BOOST_TEST(!bool(c2));
    BOOST_TEST(!c2);
}

int
main(int argc, char const* argv[])
{
    test_basics();
    test_null();
    test_is_pimpl();
    test_shared();
    test_copied();
    test_unique();
    test_onstack();
    test_bool_conversions();
    test_runtime_polymorphic_behavior();
    test_swap();

    return boost::report_errors();
}

