// Copyright 2016 Daniel Parker
// Distributed under Boost license

#include <cassert>
#include <string>
#include <vector>
#include <list>
#include <iomanip>
#include <jsoncons/json.hpp>

namespace json_type_traits_macro_examples_ns {

enum class BookCategory {fiction,biography};

std::ostream& operator<<(std::ostream& os, BookCategory c)
{
    switch(c)
    {
        case BookCategory::fiction: os << "fiction"; break;
        case BookCategory::biography: os << "biography"; break;
    }
    return os;
}

// #1 Class with public member data and default constructor   
struct Book1
{
    BookCategory category;
    std::string author;
    std::string title;
    double price;
};

// #2 Class with private member data and default constructor   
class Book2
{
    BookCategory category_;
    std::string author_;
    std::string title_;
    double price_;
    Book2() = default;

    JSONCONS_TYPE_TRAITS_FRIEND
public:
    BookCategory category() const {return category_;}

    const std::string& author() const {return author_;}

    const std::string& title() const{return title_;}

    double price() const{return price_;}
};

// #3 Class with getters and initializing constructor
class Book3
{
    BookCategory category_;
    std::string author_;
    std::string title_;
    double price_;
public:
    Book3(BookCategory category,
          const std::string& author,
          const std::string& title,
          double price)
        : category_(category), author_(author), title_(title), price_(price)
    {
    }

    Book3(const Book3&) = default;
    Book3(Book3&&) = default;
    Book3& operator=(const Book3&) = default;
    Book3& operator=(Book3&&) = default;

    BookCategory category() const {return category_;}

    const std::string& author() const{return author_;}

    const std::string& title() const{return title_;}

    double price() const{return price_;}
};

// #4 Class with getters, setters and default constructor
class Book4
{
    BookCategory category_;
    std::string author_;
    std::string title_;
    double price_;
public:
    Book4() = default;
    Book4(const Book4&) = default;
    Book4(Book4&&) = default;
    Book4& operator=(const Book4&) = default;
    Book4& operator=(Book4&&) = default;

    BookCategory getCategory() const {return category_;}
    void setCategory(const BookCategory& value) {category_ = value;}

    const std::string& getAuthor() const {return author_;}
    void setAuthor(const std::string& value) {author_ = value;}

    const std::string& getTitle() const {return title_;}
    void setTitle(const std::string& value) {title_ = value;}

    double getPrice() const {return price_;}
    void setPrice(double value) {price_ = value;}
};
}

namespace ns = json_type_traits_macro_examples_ns;

// Declare the traits at global scope
JSONCONS_ENUM_NAMED_TRAITS_DECL(ns::BookCategory,(fiction,"Fiction"),(biography,"Biography"))

JSONCONS_ALL_MEMBER_NAMED_TRAITS_DECL(ns::Book1,(category,"Category"),(author,"Author"),
                                                (title,"Title"),(price,"Price"))
JSONCONS_ALL_MEMBER_NAMED_TRAITS_DECL(ns::Book2,(category_,"Category"),(author_,"Author"),
                                                (title_,"Title"),(price_,"Price"))
JSONCONS_ALL_GETTER_CTOR_NAMED_TRAITS_DECL(ns::Book3,(category,"Category"),(author,"Author"),
                                                 (title,"Title"),(price,"Price"))
JSONCONS_ALL_GETTER_SETTER_NAMED_TRAITS_DECL(ns::Book4,(getCategory,setCategory,"Category"),
                                                       (getAuthor,setAuthor,"Author"),
                                                       (getTitle,setTitle,"Title"),
                                                       (getPrice,setPrice,"Price"))

using namespace jsoncons;

static void json_type_traits_book_examples()
{
    const std::string input = R"(
    [
        {
            "Category" : "Fiction",
            "Author" : "Haruki Murakami",
            "Title" : "Kafka on the Shore",
            "Price" : 25.17
        },
        {
            "Category" : "Biography",
            "Author" : "Robert A. Caro",
            "Title" : "The Path to Power: The Years of Lyndon Johnson I",
            "Price" : 16.99
        }
    ]
    )";

    std::cout << "(1)\n\n";
    auto books1 = decode_json<std::vector<ns::Book1>>(input);
    for (const auto& item : books1)
    {
        std::cout << item.category << ", " 
                  << item.author << ", " 
                  << item.title << ", " 
                  << item.price << "\n";
    }
    std::cout << "\n";
    encode_json(books1, std::cout, indenting::indent);
    std::cout << "\n\n";

    std::cout << "(2)\n\n";
    auto books2 = decode_json<std::vector<ns::Book2>>(input);
    for (const auto& item : books2)
    {
        std::cout << item.category() << ", " 
                  << item.author() << ", " 
                  << item.title() << ", " 
                  << item.price() << "\n";
    }
    std::cout << "\n";
    encode_json(books2, std::cout, indenting::indent);
    std::cout << "\n\n";

    std::cout << "(3)\n\n";
    auto books3 = decode_json<std::vector<ns::Book3>>(input);
    for (const auto& item : books3)
    {
        std::cout << item.category() << ", " 
                  << item.author() << ", " 
                  << item.title() << ", " 
                  << item.price() << "\n";
    }
    std::cout << "\n";
    encode_json(books3, std::cout, indenting::indent);
    std::cout << "\n\n";

    std::cout << "(4)\n\n";
    auto books4 = decode_json<std::vector<ns::Book4>>(input);
    for (const auto& item : books4)
    {
        std::cout << item.getCategory() << ", " 
                  << item.getAuthor() << ", " 
                  << item.getTitle() << ", " 
                  << item.getPrice() << "\n";
    }
    std::cout << "\n";
    encode_json(books4, std::cout, indenting::indent);
    std::cout << "\n\n";
}

void json_type_traits_macros_named_examples()
{
    std::cout << "\njson_type_traits macro named examples\n\n";

    std::cout << std::setprecision(6);

    json_type_traits_book_examples();

    std::cout << std::endl;
}
