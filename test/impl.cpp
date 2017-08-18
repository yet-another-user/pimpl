#include "./test.hpp"

template<> struct boost::impl_ptr<Book>::implementation
{
    implementation(string const& the_title, string const& the_author)
    :
        title(the_title), author(the_author)
    {}

    string  title;
    string author;
};

Book::Book(string const& title, string const& author)
:
//  impl_ptr_type(in_place, title, author)
    impl_ptr_type(in_place, std::allocator<implementation>(), title, author)
{
    // Various ways of initializing the impl_ptr base:
    // 1) Internally calls std::make_shared
    // 2) Internally calls std::allocate_shared(allocator, ...)
    // 3) Calling std::make_shared explicitly.
    // 4) Calling std::allocate_shared explicitly.
}

string const& Book:: title() const { return (*this)->title; }
string const& Book::author() const { return (*this)->author; }

