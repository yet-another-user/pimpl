#include <boost/detail/lightweight_test.hpp>
#include <boost/pimpl/pimpl.hpp>
#include <string>

struct Book 
    : public pimpl<Book>::pointer_semantics
{
    Book(const std::string& title, const std::string& author);
    const std::string& title() const;
    const std::string& author() const;
};

template<>
struct pimpl<Book>::implementation
{
    implementation(const std::string& the_title,
        const std::string& the_author)
        : title(the_title),
          author(the_author)
    {
    }
  
    std::string title;
    std::string author;
    int price;
};
  
Book::Book(const std::string& title,
    const std::string& author)
    : pimpl_base(title, author)
{
}

const std::string& Book::title() const
{
    const implementation& impl = **this;
    return impl.title;
}

const std::string& Book::author() const
{
    const implementation& impl = **this;
    return impl.author;
}

int main() 
{
    Book book("Dune", "Frank Herbert");
    BOOST_TEST(book.title() == "Dune");
    BOOST_TEST(book.author() == "Frank Herbert");
    return boost::report_errors();
}

