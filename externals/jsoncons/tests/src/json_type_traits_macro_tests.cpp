// Copyright 2013 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <cstdint>

using namespace jsoncons;

namespace json_type_traits_macro_tests
{

    template <typename T1, typename T2>
    struct TemplatedStruct
    {
          T1 aT1;
          T2 aT2;
    };

    template <typename T1>
    struct MyStruct
    {
          T1 typeContent;
          std::string someString;
    };

    template <typename T1>
    struct MyStruct2
    {
          T1 typeContent;
          std::string someString;
    };

    template <typename T1>
    struct MyStruct3
    {
        T1 typeContent_;
        std::string someString_;
    public:
        MyStruct3(T1 typeContent, const std::string& someString)
            : typeContent_(typeContent), someString_(someString)
        {
        }

        const T1& typeContent() const {return typeContent_;}
        const std::string& someString() const {return someString_;}
    };

    struct book1a
    {
        std::string author;
        std::string title;
        double price;

        friend std::ostream& operator<<(std::ostream& os, const book1a& b)
        {
            std::cout << "author: " << b.author << ", title: " << b.title << ", price: " << b.price << "\n";
            return os;
        }
    };
    struct book1b
    {
        std::string author;
        std::string title;
        double price;
        std::string isbn;
    };

    class book2a
    {
        std::string author_;
        std::string title_;
        double price_;
    public:
        book2a(const std::string& author,
              const std::string& title,
              double price)
            : author_(author), title_(title), price_(price)
        {
        }

        book2a(const book2a&) = default;
        book2a(book2a&&) = default;
        book2a& operator=(const book2a&) = default;
        book2a& operator=(book2a&&) = default;

        const std::string& author() const
        {
            return author_;
        }

        const std::string& title() const
        {
            return title_;
        }

        double price() const
        {
            return price_;
        }
    };

    class book2b
    {
        std::string author_;
        std::string title_;
        double price_;
        std::string isbn_;
    public:
        book2b(const std::string& author,
              const std::string& title,
              double price,
              const std::string& isbn)
            : author_(author), title_(title), price_(price), isbn_(isbn)
        {
        }

        book2b(const book2b&) = default;
        book2b(book2b&&) = default;
        book2b& operator=(const book2b&) = default;
        book2b& operator=(book2b&&) = default;

        const std::string& author() const
        {
            return author_;
        }

        const std::string& title() const
        {
            return title_;
        }

        double price() const
        {
            return price_;
        }

        const std::string& isbn() const
        {
            return isbn_;
        }
    };

    class book3a
    {
        std::string author_;
        std::string title_;
        double price_;
    public:
        book3a()
            : author_(), title_(), price_()
        {
        }

        book3a(const book3a&) = default;
        book3a(book3a&&) = default;
        book3a& operator=(const book3a&) = default;
        book3a& operator=(book3a&&) = default;

        const std::string& getAuthor() const
        {
            return author_;
        }

        void setAuthor(const std::string& value)
        {
            author_ = value;
        }

        const std::string& getTitle() const
        {
            return title_;
        }

        void setTitle(const std::string& value)
        {
            title_ = value;
        }

        double getPrice() const
        {
            return price_;
        }

        void setPrice(double value)
        {
            price_ = value;
        }
    };

    class book3b
    {
        std::string author_;
        std::string title_;
        double price_;
        std::string isbn_;
    public:
        book3b()
            : author_(), title_(), price_(), isbn_()
        {
        }

        book3b(const book3b&) = default;
        book3b(book3b&&) = default;
        book3b& operator=(const book3b&) = default;
        book3b& operator=(book3b&&) = default;

        const std::string& getAuthor() const
        {
            return author_;
        }

        void setAuthor(const std::string& value)
        {
            author_ = value;
        }

        const std::string& getTitle() const
        {
            return title_;
        }

        void setTitle(const std::string& value)
        {
            title_ = value;
        }

        double getPrice() const
        {
            return price_;
        }

        void setPrice(double value)
        {
            price_ = value;
        }

        const std::string& getIsbn() const
        {
            return isbn_;
        }

        void setIsbn(const std::string& value)
        {
            isbn_ = value;
        }
    };

    enum class float_format {scientific = 1,fixed = 2,hex = 4,general = fixed | scientific};

    class Employee
    {
        std::string firstName_;
        std::string lastName_;
    public:
        Employee(const std::string& firstName, const std::string& lastName)
            : firstName_(firstName), lastName_(lastName)
        {
        }
        virtual ~Employee() = default;

        virtual double calculatePay() const = 0;

        const std::string& firstName() const {return firstName_;}
        const std::string& lastName() const {return lastName_;}
    };

    class HourlyEmployee : public Employee
    {
        double wage_;
        unsigned hours_;
    public:
        HourlyEmployee(const std::string& firstName, const std::string& lastName, 
                       double wage, unsigned hours)
            : Employee(firstName, lastName), 
              wage_(wage), hours_(hours)
        {
        }
        HourlyEmployee(const HourlyEmployee&) = default;
        HourlyEmployee(HourlyEmployee&&) = default;
        HourlyEmployee& operator=(const HourlyEmployee&) = default;
        HourlyEmployee& operator=(HourlyEmployee&&) = default;

        double wage() const {return wage_;}

        unsigned hours() const {return hours_;}

        double calculatePay() const override
        {
            return wage_*hours_;
        }
    };

    class CommissionedEmployee : public Employee
    {
        double baseSalary_;
        double commission_;
        unsigned sales_;
    public:
        CommissionedEmployee(const std::string& firstName, const std::string& lastName, 
                             double baseSalary, double commission, unsigned sales)
            : Employee(firstName, lastName), 
              baseSalary_(baseSalary), commission_(commission), sales_(sales)
        {
        }
        CommissionedEmployee(const CommissionedEmployee&) = default;
        CommissionedEmployee(CommissionedEmployee&&) = default;
        CommissionedEmployee& operator=(const CommissionedEmployee&) = default;
        CommissionedEmployee& operator=(CommissionedEmployee&&) = default;

        double baseSalary() const
        {
            return baseSalary_;
        }

        double commission() const
        {
            return commission_;
        }

        unsigned sales() const
        {
            return sales_;
        }

        double calculatePay() const override
        {
            return baseSalary_ + commission_*sales_;
        }
    };
} // namespace json_type_traits_macro_tests
 
namespace ns = json_type_traits_macro_tests;

JSONCONS_ENUM_TRAITS_DECL(ns::float_format, scientific, fixed, hex, general)
JSONCONS_ALL_MEMBER_TRAITS_DECL(ns::book1a,author,title,price)
JSONCONS_N_MEMBER_TRAITS_DECL(ns::book1b,3,author,title,price,isbn)
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(ns::book2a, author, title, price)
JSONCONS_N_GETTER_CTOR_TRAITS_DECL(ns::book2b, 2, author, title, price, isbn)
JSONCONS_TPL_ALL_MEMBER_TRAITS_DECL(1,ns::MyStruct,typeContent,someString)
JSONCONS_TPL_ALL_MEMBER_TRAITS_DECL(1,ns::MyStruct2,typeContent,someString)
JSONCONS_TPL_ALL_GETTER_CTOR_TRAITS_DECL(1,ns::MyStruct3,typeContent,someString)
JSONCONS_TPL_ALL_MEMBER_TRAITS_DECL(2,ns::TemplatedStruct,aT1,aT2)

JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(ns::HourlyEmployee, firstName, lastName, wage, hours)
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(ns::CommissionedEmployee, firstName, lastName, baseSalary, commission, sales)
JSONCONS_POLYMORPHIC_TRAITS_DECL(ns::Employee, ns::HourlyEmployee, ns::CommissionedEmployee)

JSONCONS_ALL_PROPERTY_TRAITS_DECL(ns::book3a, get, set, Author, Title, Price)
JSONCONS_N_PROPERTY_TRAITS_DECL(ns::book3b, get, set, 2, Author, Title, Price, Isbn)

TEST_CASE("JSONCONS_ALL_MEMBER_TRAITS_DECL tests")
{
    std::string an_author = "Haruki Murakami"; 
    std::string a_title = "Kafka on the Shore";
    double a_price = 25.17;

    ns::book1a book{an_author, a_title, a_price};

    SECTION("book1a")
    {
        std::string s;

        encode_json(book, s);

        json j = decode_json<json>(s);

        REQUIRE(j.is<ns::book1a>() == true);
        REQUIRE(j.is<ns::book1b>() == true); // isbn is optional

        CHECK(j["author"].as<std::string>() == an_author);
        CHECK(j["title"].as<std::string>() == a_title);
        CHECK(j["price"].as<double>() == Approx(a_price).epsilon(0.001));

        json j2(book);

        CHECK(j == j2);

        ns::book1a val = j.as<ns::book1a>();

        CHECK(val.author == book.author);
        CHECK(val.title == book.title);
        CHECK(val.price == Approx(book.price).epsilon(0.001));
    }
}

TEST_CASE("JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL tests")
{
    std::string an_author = "Haruki Murakami"; 
    std::string a_title = "Kafka on the Shore";
    double a_price = 25.17;

    SECTION("is")
    {
        json j;
        j["author"] = an_author;
        j["title"] = a_title;
        j["price"] = a_price;

        bool val = j.is<ns::book2a>();
        CHECK(val == true);
    }
    SECTION("to_json")
    {
        ns::book2a book(an_author,a_title,a_price);

        json j(book);

        CHECK(j["author"].as<std::string>() == an_author);
        CHECK(j["title"].as<std::string>() == a_title);
        CHECK(j["price"].as<double>() == Approx(a_price).epsilon(0.001));
    }

    SECTION("as")
    {
        json j;
        j["author"] = an_author;
        j["title"] = a_title;
        j["price"] = a_price;

        ns::book2a book = j.as<ns::book2a>();

        CHECK(book.author() == an_author);
        CHECK(book.title() == a_title);
        CHECK(book.price() == Approx(a_price).epsilon(0.001));
    }
}

TEST_CASE("JSONCONS_N_GETTER_CTOR_TRAITS_DECL tests")
{
    std::string an_author = "Haruki Murakami"; 
    std::string a_title = "Kafka on the Shore";
    double a_price = 25.17;
    std::string an_isbn = "1400079276";

    SECTION("is")
    {
        json j;
        j["author"] = an_author;
        j["title"] = a_title;

        CHECK(j.is<ns::book2b>() == true);
        CHECK(j.is<ns::book2a>() == false); // has author, title, but not price

        j["price"] = a_price;
        CHECK(j.is<ns::book2a>() == true); // has author, title, price
    }

    SECTION("to_json")
    {
        ns::book2b book(an_author,a_title,a_price,an_isbn);

        json j(book);

        CHECK(j["author"].as<std::string>() == an_author);
        CHECK(j["title"].as<std::string>() == a_title);
        CHECK(j["price"].as<double>() == Approx(a_price).epsilon(0.001));
        CHECK(j["isbn"].as<std::string>() == an_isbn);
    }

    SECTION("as")
    {
        json j;
        j["author"] = an_author;
        j["title"] = a_title;
        j["price"] = a_price;

        ns::book2b book = j.as<ns::book2b>();

        CHECK(book.author() == an_author);
        CHECK(book.title() == a_title);
        CHECK(book.price() == Approx(a_price).epsilon(0.001));
    }
    SECTION("decode")
    {
        json j;
        j["author"] = an_author;
        j["title"] = a_title;

        std::string buffer;
        j.dump(buffer);
        auto book = decode_json<ns::book2b>(buffer);
        CHECK(book.author() == an_author);
        CHECK(book.title() == a_title);
        CHECK(book.price() == double());
        CHECK(book.isbn() == std::string());
    }
}

TEST_CASE("JSONCONS_TPL_ALL_MEMBER_TRAITS_DECL tests")
{
    SECTION("MyStruct<std::pair<int,int>>")
    {
        typedef ns::MyStruct<std::pair<int, int>> value_type;

        value_type val;
        val.typeContent = std::make_pair(1,2);
        val.someString = "A string";

        std::string s;
        encode_json(val, s, indenting::indent);

        auto val2 = decode_json<value_type>(s);

        CHECK(val2.typeContent.first == val.typeContent.first);
        CHECK(val2.typeContent.second == val.typeContent.second);
        CHECK(val2.someString == val.someString);

        //std::cout << val.typeContent.first << ", " << val.typeContent.second << ", " << val.someString << "\n";
    }
    SECTION("TemplatedStruct<int,double>")
    {
        typedef ns::TemplatedStruct<int,double> value_type;

        value_type val;
        val.aT1 = 1;
        val.aT2 = 2;

        std::string s;
        encode_json(val, s, indenting::indent);

        auto val2 = decode_json<value_type>(s);

        CHECK(val2.aT1 == val.aT1);
        CHECK(val2.aT2 == val.aT2);

        //std::cout << val.typeContent.first << ", " << val.typeContent.second << ", " << val.someString << "\n";
    }
    SECTION("TemplatedStruct<int,wstring>")
    {
        typedef ns::TemplatedStruct<int,std::wstring> value_type;

        value_type val;
        val.aT1 = 1;
        val.aT2 = L"sss";

        std::wstring s;
        encode_json(val, s, indenting::indent);

        auto val2 = decode_json<value_type>(s);

        CHECK(val2.aT1 == val.aT1);
        CHECK(val2.aT2 == val.aT2);

        //std::cout << val.typeContent.first << ", " << val.typeContent.second << ", " << val.someString << "\n";
    }
}

TEST_CASE("JSONCONS_TPL_ALL_GETTER_CTOR_TRAITS_DECL tests")
{
    SECTION("MyStruct<std::pair<int,int>>")
    {
        typedef ns::MyStruct3<std::pair<int, int>> value_type;

        value_type val(std::make_pair(1,2), "A string");

        std::string s;
        encode_json(val, s, indenting::indent);

        auto val2 = decode_json<value_type>(s);

        CHECK(val2.typeContent().first == val.typeContent().first);
        CHECK(val2.typeContent().second == val.typeContent().second);
        CHECK(val2.someString() == val.someString());

        //std::cout << val.typeContent.first << ", " << val.typeContent.second << ", " << val.someString << "\n";
    }
}

TEST_CASE("JSONCONS_ENUM_TRAITS_DECL tests")
{
    SECTION("float_format default")
    {
        ns::float_format val{ns::float_format::hex};

        std::string s;
        encode_json(val,s);

        auto val2 = decode_json<ns::float_format>(s);
        CHECK(val2 == val);
    }
    SECTION("float_format hex")
    {
        ns::float_format val{ns::float_format()};

        std::string s;
        encode_json(val,s);

        auto val2 = decode_json<ns::float_format>(s);
        CHECK(val2 == val);
    }
    SECTION("float_format default L")
    {
        ns::float_format val{ns::float_format::hex};

        std::wstring s;
        encode_json(val,s);

        auto val2 = decode_json<ns::float_format>(s);
        CHECK(val2 == val);
    }
    SECTION("float_format hex L")
    {
        ns::float_format val{ns::float_format::hex};

        std::wstring s;
        encode_json(val,s);

        auto val2 = decode_json<ns::float_format>(s);
        CHECK(val2 == val);
    }
}

TEST_CASE("JSONCONS_POLYMORPHIC_TRAITS_DECL tests")
{
    std::string input = R"(
[
    {
        "firstName": "John",
        "hours": 1000,
        "lastName": "Smith",
        "wage": 40.0
    },
    {
        "baseSalary": 30000.0,
        "commission": 0.25,
        "firstName": "Jane",
        "lastName": "Doe",
        "sales": 1000
    }
]
    )"; 

    const std::string firstName0 = "John";
    const std::string lastName0 = "Smith";
    const double pay0 = 40000;
    const std::string firstName1 = "Jane";
    const std::string lastName1 = "Doe";
    const double pay1 = 30250;

    SECTION("decode vector of shared_ptr test")
    {
        auto v = jsoncons::decode_json<std::vector<std::shared_ptr<ns::Employee>>>(input);
        REQUIRE(v.size() == 2);
        CHECK(v[0]->firstName() == firstName0);
        CHECK(v[0]->lastName() == lastName0);
        CHECK(v[0]->calculatePay() == pay0);
        CHECK(v[1]->firstName() == firstName1);
        CHECK(v[1]->lastName() == lastName1);
        CHECK(v[1]->calculatePay() == pay1);
    }

    SECTION("decode vector of unique_ptr test")
    {

        auto v = jsoncons::decode_json<std::vector<std::unique_ptr<ns::Employee>>>(input);
        REQUIRE(v.size() == 2);
        CHECK(v[0]->firstName() == firstName0);
        CHECK(v[0]->lastName() == lastName0);
        CHECK(v[0]->calculatePay() == pay0);
        CHECK(v[1]->firstName() == firstName1);
        CHECK(v[1]->lastName() == lastName1);
        CHECK(v[1]->calculatePay() == pay1);
    }
    SECTION("encode vector of shared_ptr test")
    {
        std::vector<std::shared_ptr<ns::Employee>> v;

        v.push_back(std::make_shared<ns::HourlyEmployee>("John", "Smith", 40.0, 1000));
        v.push_back(std::make_shared<ns::CommissionedEmployee>("Jane", "Doe", 30000, 0.25, 1000));

        jsoncons::json j(v);

        json expected = json::parse(input);
        CHECK(j == expected);
    }
    SECTION("encode vector of unique_ptr test")
    {
        std::vector<std::unique_ptr<ns::Employee>> v;

        v.emplace_back(new ns::HourlyEmployee("John", "Smith", 40.0, 1000));
        v.emplace_back(new ns::CommissionedEmployee("Jane", "Doe", 30000, 0.25, 1000));

        jsoncons::json j(v);

        json expected = json::parse(input);
        CHECK(j == expected);
    }
}

TEST_CASE("JSONCONS_N_PROPERTY_TRAITS_DECL tests")
{
    std::string an_author = "Haruki Murakami"; 
    std::string a_title = "Kafka on the Shore";
    double a_price = 25.17;

    SECTION("is")
    {
        json j;
        j["Author"] = an_author;
        j["Title"] = a_title;
        j["Price"] = a_price;

        bool val = j.is<ns::book3a>();
        CHECK(val == true);
    }
    SECTION("to_json")
    {
        ns::book3a book;
        book.setAuthor(an_author);
        book.setTitle(a_title);
        book.setPrice(a_price);

        json j(book);

        CHECK(j["Author"].as<std::string>() == an_author);
        CHECK(j["Title"].as<std::string>() == a_title);
        CHECK(j["Price"].as<double>() == Approx(a_price).epsilon(0.001));
    }

    SECTION("as")
    {
        json j;
        j["Author"] = an_author;
        j["Title"] = a_title;
        j["Price"] = a_price;

        ns::book3a book = j.as<ns::book3a>();

        CHECK(book.getAuthor() == an_author);
        CHECK(book.getTitle() == a_title);
        CHECK(book.getPrice() == Approx(a_price).epsilon(0.001));
    }
    SECTION("decode")
    {
        json j;
        j["author"] = an_author;
        j["title"] = a_title;
        j["price"] = a_price;

        std::string buffer;
        j.dump(buffer);
        auto book = decode_json<ns::book2b>(buffer);
        CHECK(book.author() == an_author);
        CHECK(book.title() == a_title);
        CHECK(book.price() == a_price);
    }
}

TEST_CASE("JSONCONS_ALL_PROPERTY_TRAITS_DECL tests")
{
    std::string an_author = "Haruki Murakami"; 
    std::string a_title = "Kafka on the Shore";
    double a_price = 25.17;
    std::string an_isbn = "1400079276";

    SECTION("is")
    {
        json j;
        j["Author"] = an_author;
        j["Title"] = a_title;

        CHECK(j.is<ns::book3b>() == true);
        CHECK(j.is<ns::book3a>() == false);

        j["Price"] = a_price;

        CHECK(j.is<ns::book3b>() == true);
        CHECK(j.is<ns::book3a>() == true);
    }
    SECTION("to_json")
    {
        ns::book3b book;
        book.setAuthor(an_author);
        book.setTitle(a_title);
        book.setPrice(a_price);
        book.setIsbn(an_isbn);

        json j(book);

        CHECK(j["Author"].as<std::string>() == an_author);
        CHECK(j["Title"].as<std::string>() == a_title);
        CHECK(j["Price"].as<double>() == Approx(a_price).epsilon(0.001));
        CHECK(j["Isbn"].as<std::string>() == an_isbn);
    }

    SECTION("as")
    {
        json j;
        j["Author"] = an_author;
        j["Title"] = a_title;
        j["Price"] = a_price;

        auto book = j.as<ns::book3b>();

        CHECK(book.getAuthor() == an_author);
        CHECK(book.getTitle() == a_title);
        CHECK(book.getPrice() == Approx(a_price).epsilon(0.001));
    }
    SECTION("decode")
    {
        json j;
        j["Author"] = an_author;
        j["Title"] = a_title;

        std::string buffer;
        j.dump(buffer);
        auto book = decode_json<ns::book3b>(buffer);
        CHECK(book.getAuthor() == an_author);
        CHECK(book.getTitle() == a_title);
        CHECK(book.getPrice() == double());
        CHECK(book.getIsbn() == std::string());
    }
}

