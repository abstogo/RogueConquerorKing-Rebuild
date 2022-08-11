# Examples

### Parse and decode

[Parse JSON from a string](#A1)  
[Parse JSON from a file](#A2)  
[Parse numbers without loosing precision](#A8)  
[Validate JSON without incurring parse exceptions](#A3)  
[How to allow comments? How not to?](#A4)  
[Set a maximum nesting depth](#A5)  
[Prevent the alphabetic sort of the outputted JSON, retaining the original insertion order](#A6)  
[Parse a very large JSON file with json_cursor](#A7)  
[Decode a JSON text using stateful result and work allocators](#A9)  

### Encode

[Encode a json value to a string](#B1)  
[Encode a json value to a stream](#B2)  
[Escape all non-ascii characters](#B3)  
[Replace the representation of NaN, Inf and -Inf when serializing. And when reading in again.](#B4)

### Decode JSON to C++ data structures, encode C++ data structures to JSON

[Serialize with the C++ member names of the class](#G1)  
[Serialize with the provided names using the `_NAMED_` macros](#G2)  
[Serialize a templated class with the `_TPL_` macros](#G3)  
[Specialize json_type_traits explicitly](#G4)  
[Mapping to C++ data structures with and without defaults allowed](#G5)  
[An example using JSONCONS_ENUM_TRAITS_DECL and JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL](#G6)  
[Serialize a polymorphic type based on the presence of members](#G7)  
[Ensuring type selection is possible](#G8)  
[Convert JSON numbers to/from boost multiprecision numbers](#G9)

### Construct

[Construct a json object](#C1)  
[Construct a json array](#C2)  
[Insert a new value in an array at a specific position](#C3)  
[Create arrays of arrays of arrays of ...](#C4)  
[Merge two json objects](#C5)  
[Construct a json byte string](#C6)  

### Access

[Use string_view to access the actual memory that's being used to hold a string](#E1)  
[Given a string in a json object that represents a decimal number, assign it to a double](#E2)  
[Retrieve a big integer that's been parsed as a string](#E3)  
[Look up a key, if found, return the value converted to type T, otherwise, return a default value of type T](#E4)  
[Retrieve a value in a hierarchy of JSON objects](#E5)  
[Retrieve a json value as a byte string](#E6)

### Iterate

[Iterate over a json array](#D1)  
[Iterate over a json object](#D2)  

### Search and Replace

[Search for and repace an object member key](#F1)  
[Search for and replace a value](#F2)  

### Parse and decode

<div id="A1"/> 

#### Parse JSON from a string

```
std::string s = R"({"first":1,"second":2,"fourth":3,"fifth":4})";    

json j = json::parse(s);
```

or

```c++
using namespace jsoncons::literals;

json j = R"(
{
    "StartDate" : "2017-03-01",
    "MaturityDate" : "2020-12-30"          
}
)"_json;
```

<div id="A2"/> 

#### Parse JSON from a file

```
std::ifstream is("myfile.json");    

json j = json::parse(is);
```

<div id="A8"/> 

#### Parse numbers without loosing precision

By default, jsoncons parses a number with an exponent or fractional part
into a double precision floating point number. If you wish, you can
keep the number as a string with semantic tagging `bigdec`, 
using the `lossless_number` option. You can then put it into a `float`, 
`double`, a boost multiprecision number, or whatever other type you want. 

```c++
#include <jsoncons/json.hpp>

int main()
{
    std::string s = R"(
    {
        "a" : 12.00,
        "b" : 1.23456789012345678901234567890
    }
    )";

    // Default
    json j = json::parse(s);

    std::cout.precision(15);

    // Access as string
    std::cout << "(1) a: " << j["a"].as<std::string>() << ", b: " << j["b"].as<std::string>() << "\n"; 
    // Access as double
    std::cout << "(2) a: " << j["a"].as<double>() << ", b: " << j["b"].as<double>() << "\n\n"; 

    // Using lossless_number option
    json_options options;
    options.lossless_number(true);

    json j2 = json::parse(s, options);
    // Access as string
    std::cout << "(3) a: " << j2["a"].as<std::string>() << ", b: " << j2["b"].as<std::string>() << "\n";
    // Access as double
    std::cout << "(4) a: " << j2["a"].as<double>() << ", b: " << j2["b"].as<double>() << "\n\n"; 
}
```
Output:
```
(1) a: 12.0, b: 1.2345678901234567
(2) a: 12, b: 1.23456789012346

(3) a: 12.00, b: 1.23456789012345678901234567890
(4) a: 12, b: 1.23456789012346
```

<div id="A3"/> 

#### Validate JSON without incurring parse exceptions
```c++
std::string s = R"(
{
    "StartDate" : "2017-03-01",
    "MaturityDate" "2020-12-30"          
}
)";

json_reader reader(s);

// or,
// std::stringstream is(s);
// json_reader reader(is);

std::error_code ec;
reader.read(ec);

if (ec)
{
    std::cout << ec.message() 
              << " on line " << reader.line()
              << " and column " << reader.column()
              << std::endl;
}
```
Output:
```
Expected name separator ':' on line 4 and column 20
```

<div id="A4"/> 

#### How to allow comments? How not to?

jsoncons, by default, accepts and ignores C-style comments

```c++
std::string s = R"(
{
    // Single line comments
    /*
        Multi line comments 
    */
}
)";

// Default
json j = json::parse(s);
std::cout << "(1) " << j << std::endl;

// Strict
try
{
    json j = json::parse(s, strict_json_parsing());
}
catch (const ser_error& e)
{
    std::cout << "(2) " << e.what() << std::endl;
}
```
Output:
```
(1) {}
(2) Illegal comment at line 3 and column 10
```

<div id="A5"/> 

#### Set a maximum nesting depth

Like this,
```c++
std::string s = "[[[[[[[[[[[[[[[[[[[[[\"Too deep\"]]]]]]]]]]]]]]]]]]]]]";
try
{
    json_options options;
    options.max_nesting_depth(20);
    json j = json::parse(s, options);
}
catch (const ser_error& e)
{
     std::cout << e.what() << std::endl;
}
```
Output:
```
Maximum JSON depth exceeded at line 1 and column 21
```

<div id="A6"/> 

#### Prevent the alphabetic sort of the outputted JSON, retaining the original insertion order

Use `ojson` instead of `json` (or `wojson` instead of `wjson`) to retain the original insertion order. 

```c++
ojson j = ojson::parse(R"(
{
    "street_number" : "100",
    "street_name" : "Queen St W",
    "city" : "Toronto",
    "country" : "Canada"
}
)");
std::cout << "(1)\n" << pretty_print(j) << std::endl;

// Insert "postal_code" at end
j.insert_or_assign("postal_code", "M5H 2N2");
std::cout << "(2)\n" << pretty_print(j) << std::endl;

// Insert "province" before "country"
auto it = j.find("country");
j.insert_or_assign(it,"province","Ontario");
std::cout << "(3)\n" << pretty_print(j) << std::endl;
```
Output:
```
(1)
{
    "street_number": "100",
    "street_name": "Queen St W",
    "city": "Toronto",
    "country": "Canada"
}
(2)
{
    "street_number": "100",
    "street_name": "Queen St W",
    "city": "Toronto",
    "country": "Canada",
    "postal_code": "M5H 2N2"
}
(3)
{
    "street_number": "100",
    "street_name": "Queen St W",
    "city": "Toronto",
    "province": "Ontario",
    "country": "Canada",
    "postal_code": "M5H 2N2"
}
```

<div id="A7"/> 

### Parse a very large JSON file with json_cursor

A typical pull parsing application will repeatedly process the `current()` 
event and call `next()` to advance to the next event, until `done()` 
returns `true`.

Input JSON file `book_catalog.json`:

```json
[ 
  { 
      "author" : "Haruki Murakami",
      "title" : "Hard-Boiled Wonderland and the End of the World",
      "isbn" : "0679743464",
      "publisher" : "Vintage",
      "date" : "1993-03-02",
      "price": 18.90
  },
  { 
      "author" : "Graham Greene",
      "title" : "The Comedians",
      "isbn" : "0099478374",
      "publisher" : "Vintage Classics",
      "date" : "2005-09-21",
      "price": 15.74
  }
]
```

#### Read JSON parse events
```c++
std::ifstream is("book_catalog.json");

json_cursor cursor(is);

for (; !cursor.done(); cursor.next())
{
    const auto& event = cursor.current();
    switch (event.event_type())
    {
        case staj_event_type::begin_array:
            std::cout << event.event_type() << " " << "\n";
            break;
        case staj_event_type::end_array:
            std::cout << event.event_type() << " " << "\n";
            break;
        case staj_event_type::begin_object:
            std::cout << event.event_type() << " " << "\n";
            break;
        case staj_event_type::end_object:
            std::cout << event.event_type() << " " << "\n";
            break;
        case staj_event_type::name:
            // Or std::string_view, if supported
            std::cout << event.event_type() << ": " << event.get<jsoncons::string_view>() << "\n";
            break;
        case staj_event_type::string_value:
            // Or std::string_view, if supported
            std::cout << event.event_type() << ": " << event.get<jsoncons::string_view>() << "\n";
            break;
        case staj_event_type::null_value:
            std::cout << event.event_type() << "\n";
            break;
        case staj_event_type::bool_value:
            std::cout << event.event_type() << ": " << std::boolalpha << event.get<bool>() << "\n";
            break;
        case staj_event_type::int64_value:
            std::cout << event.event_type() << ": " << event.get<int64_t>() << "\n";
            break;
        case staj_event_type::uint64_value:
            std::cout << event.event_type() << ": " << event.get<uint64_t>() << "\n";
            break;
        case staj_event_type::double_value:
            std::cout << event.event_type() << ": " << event.get<double>() << "\n";
            break;
        default:
            std::cout << "Unhandled event type: " << event.event_type() << " " << "\n";;
            break;
    }
}
```
Output:
```
begin_array
begin_object
name: author
string_value: Haruki Murakami
name: title
string_value: Hard-Boiled Wonderland and the End of the World
name: isbn
string_value: 0679743464
name: publisher
string_value: Vintage
name: date
string_value: 1993-03-02
name: price
double_value: 19
end_object
begin_object
name: author
string_value: Graham Greene
name: title
string_value: The Comedians
name: isbn
string_value: 0099478374
name: publisher
string_value: Vintage Classics
name: date
string_value: 2005-09-21
name: price
double_value: 16
end_object
end_array
```

<div id="A9"/> 

### Decode a JSON text using stateful result and work allocators

```c++
// Given allocator my_alloc with a single-argument constructor my_alloc(int),
// use my_alloc(1) to allocate basic_json memory, my_alloc(2) to allocate
// working memory used by json_decoder, and my_alloc(3) to allocate
// working memory used by basic_json_reader. 

typedef basic_json<char,sorted_policy,my_alloc> my_json;

std::ifstream is("book_catalog.json");
json_decoder<my_json,my_alloc> decoder(my_alloc(1),my_alloc(2));

basic_json_reader<char,stream_source<char>,my_alloc> reader(is, decoder, my_alloc(3));
reader.read();

my_json j = decoder.get_result();
std::cout << pretty_print(j) << "\n";
```

#### Filter JSON parse events

```c++
// A stream filter to filter out all events except name 
// and restrict name to "author"

struct author_filter 
{
    bool accept_next_ = false;

    bool operator()(const staj_event& event, const ser_context&) 
    {
        if (event.event_type()  == staj_event_type::name &&
            event.get<jsoncons::string_view>() == "author")
        {
            accept_next_ = true;
            return false;
        }
        else if (accept_next_)
        {
            accept_next_ = false;
            return true;
        }
        else
        {
            accept_next_ = false;
            return false;
        }
    }
};

std::ifstream is("book_catalog.json");

author_filter filter;
json_cursor cursor(is, filter);

for (; !cursor.done(); cursor.next())
{
    const auto& event = cursor.current();
    switch (event.event_type())
    {
        case staj_event_type::string_value:
            std::cout << event.get<jsoncons::string_view>() << "\n";
            break;
        default:
            std::cout << "Unhandled event type: " << event.event_type() << " " << "\n";;
            break;
    }
}
```
Output:
```
Haruki Murakami
Graham Greene
```

See [basic_json_cursor](doc/ref/basic_json_cursor.md) 

<div id="G0"/>

### Decode JSON to C++ data structures, encode C++ data structures to JSON

<div id="G1"/>

#### Serialize with the C++ member names of the class

```c++
#include <jsoncons/json.hpp>

namespace ns {

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
    BookCategory category;
    std::string author;
    std::string title;
    double price;
    Book2() = default;

    JSONCONS_TYPE_TRAITS_FRIEND
public:
    BookCategory get_category() const {return category;}

    const std::string& get_author() const {return author;}

    const std::string& get_title() const{return title;}

    double get_price() const{return price;}
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

} // namespace ns

// Declare the traits at global scope
JSONCONS_ENUM_TRAITS_DECL(ns::BookCategory,fiction,biography)

JSONCONS_ALL_MEMBER_TRAITS_DECL(ns::Book1,category,author,title,price)
JSONCONS_ALL_MEMBER_TRAITS_DECL(ns::Book2,category,author,title,price)
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(ns::Book3,category,author,title,price)

using namespace jsoncons; // for convenience

int main()
{
    const std::string input = R"(
    [
        {
            "category" : "fiction",
            "author" : "Haruki Murakami",
            "title" : "Kafka on the Shore",
            "price" : 25.17
        },
        {
            "category" : "biography",
            "author" : "Robert A. Caro",
            "title" : "The Path to Power: The Years of Lyndon Johnson I",
            "price" : 16.99
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
        std::cout << item.get_category() << ", " 
                  << item.get_author() << ", " 
                  << item.get_title() << ", " 
                  << item.get_price() << "\n";
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
}
```
Output:
```
(1)

fiction, Haruki Murakami, Kafka on the Shore, 25.170000
biography, Robert A. Caro, The Path to Power: The Years of Lyndon Johnson I, 16.990000

[
    {
        "author": "Haruki Murakami",
        "category": "fiction",
        "price": 25.17,
        "title": "Kafka on the Shore"
    },
    {
        "author": "Robert A. Caro",
        "category": "biography",
        "price": 16.99,
        "title": "The Path to Power: The Years of Lyndon Johnson I"
    }
]
```

The output for (2), (3) and (4) is the same.

<div id="G2"/>

#### Serialize with the provided names using the `_NAMED_` macros

```c++
#include <jsoncons/json.hpp>

namespace ns {

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

} // namespace ns

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

using namespace jsoncons; // for convenience

int main()
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
```
Output:
```
(1)

fiction, Haruki Murakami, Kafka on the Shore, 25.170000
biography, Robert A. Caro, The Path to Power: The Years of Lyndon Johnson I, 16.990000

[
    {
        "Author": "Haruki Murakami",
        "Category": "Fiction",
        "Price": 25.17,
        "Title": "Kafka on the Shore"
    },
    {
        "Author": "Robert A. Caro",
        "Category": "Biography",
        "Price": 16.99,
        "Title": "The Path to Power: The Years of Lyndon Johnson I"
    }
]
```

The output for (2), (3) and (4) is the same.

<div id="G3"/>

#### Serialize a templated class with the `_TPL_` macros

```c++
#include <cassert>
#include <jsoncons/json.hpp>

namespace ns {
    template <typename T1, typename T2>
    struct TemplatedStruct
    {
          T1 aT1;
          T2 aT2;

          friend bool operator==(const TemplatedStruct& lhs, const TemplatedStruct& rhs)
          {
              return lhs.aT1 == rhs.aT1 && lhs.aT2 == rhs.aT2;  
          }

          friend bool operator!=(const TemplatedStruct& lhs, const TemplatedStruct& rhs)
          {
              return !(lhs == rhs);
          }
    };

} // namespace ns

// Declare the traits. Specify the number of template parameters and which data members need to be serialized.
JSONCONS_TPL_ALL_MEMBER_TRAITS_DECL(2,ns::TemplatedStruct,aT1,aT2)

using namespace jsoncons; // for convenience

int main()
{
    typedef ns::TemplatedStruct<int,std::wstring> value_type;

    value_type val{1, L"sss"};

    std::wstring s;
    encode_json(val, s);

    auto val2 = decode_json<value_type>(s);
    assert(val2 == val);
}
```

<div id="G4"/>

#### Specialize json_type_traits explicitly

jsoncons supports conversion between JSON text and C++ data structures. The functions [decode_json](doc/ref/decode_json.md) 
and [encode_json](doc/ref/encode_json.md) convert JSON formatted strings or streams to C++ data structures and back. 
Decode and encode work for all C++ classes that have 
[json_type_traits](https://github.com/danielaparker/jsoncons/blob/master/doc/ref/json_type_traits.md) 
defined. The standard library containers are already supported, 
and your own types will be supported too if you specialize `json_type_traits`
in the `jsoncons` namespace. 


```c++
#include <iostream>
#include <jsoncons/json.hpp>
#include <vector>
#include <string>

namespace ns {
    struct book
    {
        std::string author;
        std::string title;
        double price;
    };
} // namespace ns

namespace jsoncons {

    template<class Json>
    struct json_type_traits<Json, ns::book>
    {
        typedef typename Json::allocator_type allocator_type;

        static bool is(const Json& j) noexcept
        {
            return j.is_object() && j.contains("author") && 
                   j.contains("title") && j.contains("price");
        }
        static ns::book as(const Json& j)
        {
            ns::book val;
            val.author = j.at("author").template as<std::string>();
            val.title = j.at("title").template as<std::string>();
            val.price = j.at("price").template as<double>();
            return val;
        }
        static Json to_json(const ns::book& val, 
                            allocator_type allocator=allocator_type())
        {
            Json j(allocator);
            j.try_emplace("author", val.author);
            j.try_emplace("title", val.title);
            j.try_emplace("price", val.price);
            return j;
        }
    };
} // namespace jsoncons
```

To save typing and enhance readability, the jsoncons library defines macros, 
so you could also write

```c++
JSONCONS_ALL_MEMBER_TRAITS_DECL(ns::book, author, title, price)
```

which expands to the code above.

```c++
using namespace jsoncons; // for convenience

int main()
{
    const std::string s = R"(
    [
        {
            "author" : "Haruki Murakami",
            "title" : "Kafka on the Shore",
            "price" : 25.17
        },
        {
            "author" : "Charles Bukowski",
            "title" : "Pulp",
            "price" : 22.48
        }
    ]
    )";

    std::vector<ns::book> book_list = decode_json<std::vector<ns::book>>(s);

    std::cout << "(1)\n";
    for (const auto& item : book_list)
    {
        std::cout << item.author << ", " 
                  << item.title << ", " 
                  << item.price << "\n";
    }

    std::cout << "\n(2)\n";
    encode_json(book_list, std::cout, indenting::indent);
    std::cout << "\n\n";
}
```
Output:
```
(1)
Haruki Murakami, Kafka on the Shore, 25.17
Charles Bukowski, Pulp, 22.48

(2)
[
    {
        "author": "Haruki Murakami",
        "price": 25.17,
        "title": "Kafka on the Shore"
    },
    {
        "author": "Charles Bukowski",
        "price": 22.48,
        "title": "Pulp"
    }
]
```

<div id="G5"/>

#### Mapping to C++ data structures with and without defaults allowed

The macros `JSONCONS_N_MEMBER_TRAITS_DECL` and `JSONCONS_ALL_MEMBER_TRAITS_DECL` both generate
the code to specialize `json_type_traits` from member data. The difference is that `JSONCONS_N_MEMBER_TRAITS_DECL`
does not require all member names to be present in the JSON data, while `JSONCONS_ALL_MEMBER_TRAITS_DECL` does.

```c++
#include <iostream>
#include <jsoncons/json.hpp>
#include <vector>
#include <string>

namespace ns {

    class Person
    {
    public:
        Person(const std::string& name, const std::string& surname,
               const std::string& ssn, unsigned int age)
           : name(name), surname(surname), ssn(ssn), age(age) { }

    private:
        // Make json_type_traits specializations friends to give accesses to private members
        JSONCONS_TYPE_TRAITS_FRIEND

        Person() : age(0) {}

        std::string name;
        std::string surname;
        std::string ssn;
        unsigned int age;
    };

} // namespace ns

// Declare the traits. Specify which data members need to be serialized, and how many are mandatory.
JSONCONS_N_MEMBER_TRAITS_DECL(ns::Person, 2, name, surname, ssn, age)

int main()
{
    try
    {
        // Incomplete JSON data: field ssn missing
        std::string data = R"({"name":"Rod","surname":"Bro","age":30})";
        auto person = jsoncons::decode_json<ns::Person>(data);

        std::string s;
        jsoncons::encode_json(person, s, indenting::indent);
        std::cout << s << "\n";
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << "";
    }
}
```
Output:
```
{
    "age": 30,
    "name": "Rod",
    "ssn": "",
    "surname": "Bro"
}
```

If all members of the JSON data must be present, use
```
JSONCONS_ALL_MEMBER_TRAITS_DECL(ns::Person, name, surname, ssn, age)
```
instead. This will cause an exception to be thrown with the message
```
Key 'ssn' not found
```

<div id="G6"/>

#### An example using JSONCONS_ENUM_TRAITS_DECL and JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL

This example makes use of the convenience macros `JSONCONS_ENUM_TRAITS_DECL`
and `JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL` to specialize the 
[json_type_traits](doc/ref/json_type_traits.md) for the enum type
`ns::hiking_experience` and the classes `ns::hiking_reputon` and 
`ns::hiking_reputation`.
The macro `JSONCONS_ENUM_TRAITS_DECL` generates the code from
the enum values, and the macro `JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL` 
generates the code from the getter functions and a constructor. 
These macro declarations must be placed outside any namespace blocks.

```c++
#include <cassert>
#include <iostream>
#include <jsoncons/json.hpp>

namespace ns {
    enum class hiking_experience {beginner,intermediate,advanced};

    class hiking_reputon
    {
        std::string rater_;
        hiking_experience assertion_;
        std::string rated_;
        double rating_;
    public:
        hiking_reputon(const std::string& rater,
                       hiking_experience assertion,
                       const std::string& rated,
                       double rating)
            : rater_(rater), assertion_(assertion), rated_(rated), rating_(rating)
        {
        }

        const std::string& rater() const {return rater_;}
        hiking_experience assertion() const {return assertion_;}
        const std::string& rated() const {return rated_;}
        double rating() const {return rating_;}

        friend bool operator==(const hiking_reputon& lhs, const hiking_reputon& rhs)
        {
            return lhs.rater_ == rhs.rater_ && lhs.assertion_ == rhs.assertion_ && 
                   lhs.rated_ == rhs.rated_ && lhs.rating_ == rhs.rating_;
        }

        friend bool operator!=(const hiking_reputon& lhs, const hiking_reputon& rhs)
        {
            return !(lhs == rhs);
        };
    };

    class hiking_reputation
    {
        std::string application_;
        std::vector<hiking_reputon> reputons_;
    public:
        hiking_reputation(const std::string& application, 
                          const std::vector<hiking_reputon>& reputons)
            : application_(application), 
              reputons_(reputons)
        {}

        const std::string& application() const { return application_;}
        const std::vector<hiking_reputon>& reputons() const { return reputons_;}

        friend bool operator==(const hiking_reputation& lhs, const hiking_reputation& rhs)
        {
            return (lhs.application_ == rhs.application_) && (lhs.reputons_ == rhs.reputons_);
        }

        friend bool operator!=(const hiking_reputation& lhs, const hiking_reputation& rhs)
        {
            return !(lhs == rhs);
        };
    };

} // namespace ns

// Declare the traits. Specify which data members need to be serialized.
JSONCONS_ENUM_TRAITS_DECL(ns::hiking_experience, beginner, intermediate, advanced)
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(ns::hiking_reputon, rater, assertion, rated, rating)
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(ns::hiking_reputation, application, reputons)

using namespace jsoncons; // for convenience

int main()
{
    ns::hiking_reputation val("hiking", { ns::hiking_reputon{"HikingAsylum",ns::hiking_experience::advanced,"Marilyn C",0.90} });

    std::string s;
    encode_json(val, s, indenting::indent);
    std::cout << s << "\n";

    auto val2 = decode_json<ns::hiking_reputation>(s);

    assert(val2 == val);
}
```
Output:
```
{
    "application": "hiking",
    "reputons": [
        {
            "assertion": "advanced",
            "rated": "Marilyn C",
            "rater": "HikingAsylum",
            "rating": 0.9
        }
    ]
}
```

<div id="G7"/>

#### Serialize a polymorphic type based on the presence of members

This example uses the convenience macro `JSONCONS_N_GETTER_CTOR_TRAITS_DECL`
to generate the `json_type_traits` boilerplate for the `HourlyEmployee` and `CommissionedEmployee` 
derived classes, and `JSONCONS_POLYMORPHIC_TRAITS_DECL` to generate the `json_type_traits` boilerplate
for `std::shared_ptr<Employee>` and `std::unique_ptr<Employee>`. The type selection strategy is based
on the presence of mandatory members, in particular, to the `firstName`, `lastName`, and `wage` members of an
`HourlyEmployee`, and to the `firstName`, `lastName`, `baseSalary`, and `commission` members of a `CommissionedEmployee`.
Non-mandatory members are not considered for the purpose of type selection.

```c++
#include <cassert>
#include <iostream>
#include <vector>
#include <jsoncons/json.hpp>

using namespace jsoncons;

namespace ns {

class Employee
{
    std::string firstName_;
    std::string lastName_;
    std::string socialSecurityNumber_;
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

} // ns

JSONCONS_N_GETTER_CTOR_TRAITS_DECL(ns::HourlyEmployee, 3, firstName, lastName, wage, hours)
JSONCONS_N_GETTER_CTOR_TRAITS_DECL(ns::CommissionedEmployee, 4, firstName, lastName, baseSalary, commission, sales)
JSONCONS_POLYMORPHIC_TRAITS_DECL(ns::Employee, ns::HourlyEmployee, ns::CommissionedEmployee)

int main()
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

    auto v = decode_json<std::vector<std::unique_ptr<ns::Employee>>>(input);

    std::cout << "(1)\n";
    for (const auto& p : v)
    {
        std::cout << p->firstName() << " " << p->lastName() << ", " << p->calculatePay() << "\n";
    }

    std::cout << "\n(2)\n";
    encode_json(v, std::cout, indenting::indent);

    std::cout << "\n\n(3)\n";
    json j(v);
    std::cout << pretty_print(j) << "\n\n";
}
```
Output:
```
(1)
John Smith, 40000
Jane Doe, 30250

(2)
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

(3)
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
```

<div id="G8"/>

#### Ensuring type selection is possible

When deserializing a polymorphic type, jsoncons needs to know how
to convert a json value to the proper derived class. In the Employee
example above, the type selection strategy is based
on the presence of members in the derived classes. If
derived classes cannot be distinguished in this way, 
you can introduce extra members. The convenience
macros `JSONCONS_N_MEMBER_TRAITS_DECL`, `JSONCONS_ALL_MEMBER_TRAITS_DECL`,
`JSONCONS_TPL_N_MEMBER_TRAITS_DECL`, `JSONCONS_TPL_ALL_MEMBER_TRAITS_DECL`,
`JSONCONS_N_MEMBER_TRAITS_NAMED_DECL`, `JSONCONS_ALL_MEMBER_TRAITS_NAMED_DECL`,
`JSONCONS_TPL_N_MEMBER_TRAITS_NAMED_DECL`, and `JSONCONS_TPL_ALL_MEMBER_TRAITS_NAMED_DECL`
allow you to have `const` or `static const` data members that are serialized and that 
particpate in the type selection strategy during deserialization. 

```c++
namespace ns {

class Foo
{
public:
    virtual ~Foo() = default;
};

class Bar : public Foo
{
    static const bool bar = true;
    JSONCONS_TYPE_TRAITS_FRIEND
};

class Baz : public Foo 
{
    static const bool baz = true;
    JSONCONS_TYPE_TRAITS_FRIEND
};

} // ns

JSONCONS_N_MEMBER_TRAITS_DECL(ns::Bar,1,bar)
JSONCONS_N_MEMBER_TRAITS_DECL(ns::Baz,1,baz)
JSONCONS_POLYMORPHIC_TRAITS_DECL(ns::Foo, ns::Bar, ns::Baz)

int main()
{
    std::vector<std::unique_ptr<ns::Foo>> u;
    u.emplace_back(new ns::Bar());
    u.emplace_back(new ns::Baz());

    std::string buffer;
    encode_json(u, buffer);
    std::cout << "(1)\n" << buffer << "\n\n";

    auto v = decode_json<std::vector<std::unique_ptr<ns::Foo>>>(buffer);

    std::cout << "(2)\n";
    for (const auto& ptr : v)
    {
        if (dynamic_cast<ns::Bar*>(ptr.get()))
        {
            std::cout << "A bar\n";
        }
        else if (dynamic_cast<ns::Baz*>(ptr.get()))
        {
            std::cout << "A baz\n";
        } 
    }
}
```

Output:
```
(1)
[{"bar":true},{"baz":true}]

(2)
A bar
A baz
```

<div id="G9"/>

#### Convert JSON numbers to/from boost multiprecision numbers

```
#include <jsoncons/json.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>

namespace jsoncons 
{
    template <class Json, class Backend>
    struct json_type_traits<Json,boost::multiprecision::number<Backend>>
    {
        typedef boost::multiprecision::number<Backend> multiprecision_type;

        static bool is(const Json& val) noexcept
        {
            if (!(val.is_string() && val.semantic_tag() == semantic_tag::bigdec))
            {
                return false;
            }
            else
            {
                return true;
            }
        }

        static multiprecision_type as(const Json& val)
        {
            return multiprecision_type(val.template as<std::string>());
        }

        static Json to_json(multiprecision_type val)
        {
            return Json(val.str(), semantic_tag::bigdec);
        }
    };
}

int main()
{
    typedef boost::multiprecision::number<boost::multiprecision::cpp_dec_float_50> multiprecision_type;

    std::string s = "[100000000000000000000000000000000.1234]";
    json_options options;
    options.lossless_number(true);
    json j = json::parse(s, options);

    multiprecision_type x = j[0].as<multiprecision_type>();

    std::cout << "(1) " << std::setprecision(std::numeric_limits<multiprecision_type>::max_digits10)
        << x << "\n";

    json j2 = json::array{x};
    std::cout << "(2) " << j2[0].as<std::string>() << "\n";
}
```
Output:
```
(1) 100000000000000000000000000000000.1234
(2) 100000000000000000000000000000000.1234
```

### Encode

<div id="B1"/>

#### Encode a json value to a string

```
std::string s;

j.dump(s); // compressed

j.dump(s, indenting::indent); // pretty print
```

<div id="B2"/>

#### Encode a json value to a stream

```
j.dump(std::cout); // compressed

j.dump(std::cout, indenting::indent); // pretty print
```
or
```
std::cout << j << std::endl; // compressed

std::cout << pretty_print(j) << std::endl; // pretty print
```

<div id="B3"/>

#### Escape all non-ascii characters

```
json_options options;
options.escape_all_non_ascii(true);

j.dump(std::cout, options); // compressed

j.dump(std::cout, options, indenting::indent); // pretty print
```
or
```
std::cout << print(j, options) << std::endl; // compressed

std::cout << pretty_print(j, options) << std::endl; // pretty print
```

<div id="B4"/>

#### Replace the representation of NaN, Inf and -Inf when serializing. And when reading in again.

Set the serializing options for `nan` and `inf` to distinct string values.

```c++
json j;
j["field1"] = std::sqrt(-1.0);
j["field2"] = 1.79e308 * 1000;
j["field3"] = -1.79e308 * 1000;

json_options options;
options.nan_to_str("NaN")
       .inf_to_str("Inf"); 

std::ostringstream os;
os << pretty_print(j, options);

std::cout << "(1)\n" << os.str() << std::endl;

json j2 = json::parse(os.str(),options);

std::cout << "\n(2) " << j2["field1"].as<double>() << std::endl;
std::cout << "(3) " << j2["field2"].as<double>() << std::endl;
std::cout << "(4) " << j2["field3"].as<double>() << std::endl;

std::cout << "\n(5)\n" << pretty_print(j2,options) << std::endl;
```

Output:
```json
(1)
{
    "field1": "NaN",
    "field2": "Inf",
    "field3": "-Inf"
}

(2) nan
(3) inf
(4) -inf

(5)
{
    "field1": "NaN",
    "field2": "Inf",
    "field3": "-Inf"
}
```

### Construct

<div id="C1"/>

#### Construct a json object

Start with an empty json object and insert some name-value pairs,

```c++
json image_sizing;
image_sizing.insert_or_assign("Resize To Fit",true);  // a boolean 
image_sizing.insert_or_assign("Resize Unit", "pixels");  // a string
image_sizing.insert_or_assign("Resize What", "long_edge");  // a string
image_sizing.insert_or_assign("Dimension 1",9.84);  // a double
image_sizing.insert_or_assign("Dimension 2",json::null());  // a null value
```

or use an object initializer-list,

```c++
json file_settings = json::object{
    {"Image Format", "JPEG"},
    {"Color Space", "sRGB"},
    {"Limit File Size", true},
    {"Limit File Size To", 10000}
};
```

<div id="C2"/>

#### Construct a json array

```c++
json color_spaces = json::array(); // an empty array
color_spaces.push_back("sRGB");
color_spaces.push_back("AdobeRGB");
color_spaces.push_back("ProPhoto RGB");
```

or use an array initializer-list,
```c++
json image_formats = json::array{"JPEG","PSD","TIFF","DNG"};
```

<div id="C3"/>

#### Insert a new value in an array at a specific position

```c++
json cities = json::array(); // an empty array
cities.push_back("Toronto");  
cities.push_back("Vancouver");
// Insert "Montreal" at beginning of array
cities.insert(cities.array_range().begin(),"Montreal");  

std::cout << cities << std::endl;
```
Output:
```
["Montreal","Toronto","Vancouver"]
```

<div id="C4"/>

#### Create arrays of arrays of arrays of ...

Like this:

```c++
json j = json::make_array<3>(4, 3, 2, 0.0);
double val = 1.0;
for (size_t i = 0; i < a.size(); ++i)
{
    for (size_t j = 0; j < j[i].size(); ++j)
    {
        for (size_t k = 0; k < j[i][j].size(); ++k)
        {
            j[i][j][k] = val;
            val += 1.0;
        }
    }
}
std::cout << pretty_print(j) << std::endl;
```
Output:
```json
[
    [
        [1.0,2.0],
        [3.0,4.0],
        [5.0,6.0]
    ],
    [
        [7.0,8.0],
        [9.0,10.0],
        [11.0,12.0]
    ],
    [
        [13.0,14.0],
        [15.0,16.0],
        [17.0,18.0]
    ],
    [
        [19.0,20.0],
        [21.0,22.0],
        [23.0,24.0]
    ]
]
```

<div id="C5"/>

#### Merge two json objects

[json::merge](ref/json/merge.md) inserts another json object's key-value pairs into a json object,
unless they already exist with an equivalent key.

[json::merge_or_update](ref/json/merge_or_update.md) inserts another json object's key-value pairs 
into a json object, or assigns them if they already exist.

The `merge` and `merge_or_update` functions perform only a one-level-deep shallow merge,
not a deep merge of nested objects.

```c++
json another = json::parse(R"(
{
    "a" : "2",
    "c" : [4,5,6]
}
)");

json j = json::parse(R"(
{
    "a" : "1",
    "b" : [1,2,3]
}
)");

j.merge(std::move(another));
std::cout << pretty_print(j) << std::endl;
```
Output:
```json
{
    "a": "1",
    "b": [1,2,3],
    "c": [4,5,6]
}
```

<div id="C6"/>

#### Construct a json byte string

```c++
#include <jsoncons/json.hpp>

namespace jc=jsoncons;

int main()
{
    byte_string bs = {'H','e','l','l','o'};

    // default suggested encoding (base64url)
    json j1(bs);
    std::cout << "(1) "<< j1 << "\n\n";

    // base64 suggested encoding
    json j2(bs, semantic_tag::base64);
    std::cout << "(2) "<< j2 << "\n\n";

    // base16 suggested encoding
    json j3(bs, semantic_tag::base16);
    std::cout << "(3) "<< j3 << "\n\n";
}
```

Output:
```
(1) "SGVsbG8"

(2) "SGVsbG8="

(3) "48656C6C6F"
```

### Iterate

<div id="D1"/>

#### Iterate over a json array

```c++
json j = json::array{1,2,3,4};

for (auto val : j.array_range())
{
    std::cout << val << std::endl;
}
```

<div id="D2"/>

#### Iterate over a json object

```c++
json j = json::object{
    {"author", "Haruki Murakami"},
    {"title", "Kafka on the Shore"},
    {"price", 25.17}
};

for (const auto& member : j.object_range())
{
    std::cout << member.key() << "=" 
              << member.value() << std::endl;
}
```

### Access

<div id="E1"/>

#### Use string_view to access the actual memory that's being used to hold a string

You can use `j.as<jsoncons::string_view>()`, e.g.
```c++
json j = json::parse("\"Hello World\"");
auto sv = j.as<jsoncons::string_view>();
```
`jsoncons::string_view` supports the member functions of `std::string_view`, including `data()` and `size()`. 

If your compiler supports `std::string_view`, you can also use `j.as<std::string_view>()`.

<div id="E2"/>

#### Given a string in a json object that represents a decimal number, assign it to a double

```c++
json j = json::object{
    {"price", "25.17"}
};

double price = j["price"].as<double>();
```

<div id="E3"/>
 
#### Retrieve a big integer that's been parsed as a string

If an integer exceeds the range of an `int64_t` or `uint64_t`, jsoncons parses it as a string 
with semantic tagging `bigint`.

```c++
#include <jsoncons/json.hpp>
#include <iostream>
#include <iomanip>

using jsoncons::json;

int main()
{
    std::string input = "-18446744073709551617";

    json j = json::parse(input);

    // Access as string
    std::string s = j.as<std::string>();
    std::cout << "(1) " << s << "\n\n";

    // Access as double
    double d = j.as<double>();
    std::cout << "(2) " << std::setprecision(17) << d << "\n\n";

    // Access as jsoncons::bignum
    jsoncons::bignum bn = j.as<jsoncons::bignum>();
    std::cout << "(3) " << bn << "\n\n";

    // If your compiler supports extended integral types for which std::numeric_limits is specialized 
#if (defined(__GNUC__) || defined(__clang__)) && (!defined(__ALL_ANSI__) && defined(_GLIBCXX_USE_INT128))
    __int128 i = j.as<__int128>();
    std::cout << "(4) " << i << "\n\n";
#endif
}
```
Output:
```
(1) -18446744073709551617

(2) -1.8446744073709552e+19

(3) -18446744073709551617

(4) -18446744073709551617
```

<div id="E4"/>

#### Look up a key, if found, return the value converted to type T, otherwise, return a default value of type T.
 
```c++
json j = json::object{
    {"price", "25.17"}
};

double price = j.get_with_default("price", 25.00); // returns 25.17

double sale_price = j.get_with_default("sale_price", 22.0); // returns 22.0
```

<div id="E5"/>
 
#### Retrieve a value in a hierarchy of JSON objects

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>

int main()
{
    json j = json::parse(R"(
    {
       "application": "hiking",
       "reputons": [
       {
           "rater": "HikingAsylum",
           "assertion": "advanced",
           "rated": "Marilyn C",
           "rating": 0.90
         }
       ]
    }
    )");

    // Using index or `at` accessors
    std::string result1 = j["reputons"][0]["rated"].as<std::string>();
    std::cout << "(1) " << result1 << std::endl;
    std::string result2 = j.at("reputons").at(0).at("rated").as<std::string>();
    std::cout << "(2) " << result2 << std::endl;

    // Using JSON Pointer
    std::string result3 = jsonpointer::get(j, "/reputons/0/rated").as<std::string>();
    std::cout << "(3) " << result3 << std::endl;

    // Using JSONPath
    json result4 = jsonpath::json_query(j, "$.reputons.0.rated");
    if (result4.size() > 0)
    {
        std::cout << "(4) " << result4[0].as<std::string>() << std::endl;
    }
    json result5 = jsonpath::json_query(j, "$..0.rated");
    if (result5.size() > 0)
    {
        std::cout << "(5) " << result5[0].as<std::string>() << std::endl;
    }
}
```

<div id="E6"/>
 
#### Retrieve a json value as a byte string

```c++
#include <jsoncons/json.hpp>

namespace jc=jsoncons;

int main()
{
    json j;
    j["ByteString"] = byte_string({'H','e','l','l','o'});
    j["EncodedByteString"] = json("SGVsbG8=", semantic_tag::base64);

    std::cout << "(1)\n";
    std::cout << pretty_print(j) << "\n\n";

    // Retrieve a byte string as a jsoncons::byte_string
    byte_string bs1 = j["ByteString"].as<byte_string>();
    std::cout << "(2) " << bs1 << "\n\n";

    // or alternatively as a std::vector<uint8_t>
    std::vector<uint8_t> v = j["ByteString"].as<std::vector<uint8_t>>();

    // Retrieve a byte string from a text string containing base64 character values
    byte_string bs2 = j["EncodedByteString"].as<byte_string>();
    std::cout << "(3) " << bs2 << "\n\n";

    // Retrieve a byte string view  to access the memory that's holding the byte string
    byte_string_view bsv3 = j["ByteString"].as<byte_string_view>();
    std::cout << "(4) " << bsv3 << "\n\n";

    // Can't retrieve a byte string view of a text string 
    try
    {
        byte_string_view bsv4 = j["EncodedByteString"].as<byte_string_view>();
    }
    catch (const std::exception& e)
    {
        std::cout << "(5) "<< e.what() << "\n\n";
    }
}
```
Output:
```
(1)
{
    "ByteString": "SGVsbG8",
    "EncodedByteString": "SGVsbG8="
}

(2) 48 65 6c 6c 6f

(3) 48 65 6c 6c 6f

(4) 48 65 6c 6c 6f

(5) Not a byte string
```

### Search and Replace
 
<div id="F1"/>

#### Search for and repace an object member key

You can rename object members with the built in filter [rename_object_member_filter](ref/rename_object_member_filter.md)

```c++
#include <sstream>
#include <jsoncons/json.hpp>
#include <jsoncons/json_content_filter.hpp>

using namespace jsoncons;

int main()
{
    std::string s = R"({"first":1,"second":2,"fourth":3,"fifth":4})";    

    json_stream_encoder encoder(std::cout);

    // Filters can be chained
    rename_object_member_filter filter2("fifth", "fourth", encoder);
    rename_object_member_filter filter1("fourth", "third", filter2);

    // A filter can be passed to any function that takes a json_content_handler ...
    std::cout << "(1) ";
    std::istringstream is(s);
    json_reader reader(is, filter1);
    reader.read();
    std::cout << std::endl;

    // or 
    std::cout << "(2) ";
    ojson j = ojson::parse(s);
    j.dump(filter1);
    std::cout << std::endl;
}
```
Output:
```json
(1) {"first":1,"second":2,"third":3,"fourth":4}
(2) {"first":1,"second":2,"third":3,"fourth":4}
```
 
<div id="F2"/>

#### Search for and replace a value

You can use [json_replace](ref/jsonpath/json_replace.md) in the `jsonpath` extension

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>

using namespace jsoncons;

int main()
{
    json j = json::parse(R"(
        { "store": {
            "book": [ 
              { "category": "reference",
                "author": "Nigel Rees",
                "title": "Sayings of the Century",
                "price": 8.95
              },
              { "category": "fiction",
                "author": "Evelyn Waugh",
                "title": "Sword of Honour",
                "price": 12.99
              },
              { "category": "fiction",
                "author": "Herman Melville",
                "title": "Moby Dick",
                "isbn": "0-553-21311-3",
                "price": 8.99
              }
            ]
          }
        }
    )");

    // Change the price of "Moby Dick" from $8.99 to $10
    jsonpath::json_replace(j,"$.store.book[?(@.isbn == '0-553-21311-3')].price",10.0);
    std::cout << pretty_print(booklist) << std::endl;
}

