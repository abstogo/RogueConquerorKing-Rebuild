// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>
#include <fstream>

using namespace jsoncons;

void json_reader_examples();
void readme_examples();
void basics_examples();
void basics_wexamples();
void json_content_filter_examples();
void array_examples();
void container_examples();
void wjson_examples();
void serialization_examples();
void type_extensibility_examples();
void ojson_examples();
void unicode_examples();
void run_csv_examples();
void jsonpath_examples();
void json_accessor_examples();
void msgpack_examples();
void jsonpointer_examples();
void jsonpatch_examples();
void run_cbor_examples();
void run_ubjson_examples();
void json_parser_examples();
void byte_string_examples();
void json_cursor_examples();
void data_model_examples();
void staj_iterator_examples();
void bson_examples();
void json_type_traits_macros_examples();
void json_type_traits_macros_named_examples();
void run_cbor_typed_array_examples();

void comment_example()
{
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
}

void first_example_a()
{
    std::string path = "./input/books.json"; 
    std::fstream is(path);
    if (!is)
    {
        std::cout << "Cannot open " << path << std::endl;
        return;
    }
    json books = json::parse(is);

    for (size_t i = 0; i < books.size(); ++i)
    {
        try
        {
            json& book = books[i];
            std::string author = book["author"].as<std::string>();
            std::string title = book["title"].as<std::string>();
            double price = book["price"].as<double>();
            std::cout << author << ", " << title << ", " << price << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
}

void first_example_b()
{
    std::string path = "./input/books.json"; 
    std::fstream is(path);
    if (!is)
    {
        std::cout << "Cannot open " << path << std::endl;
        return;
    }
    json books = json::parse(is);

    for (size_t i = 0; i < books.size(); ++i)
    {
        try
        {
            json& book = books[i];
            std::string author = book["author"].as<std::string>();
            std::string title = book["title"].as<std::string>();
            std::string price = book.get_with_default("price", "N/A");
            std::cout << author << ", " << title << ", " << price << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
}

void first_example_c()
{
    const json books = json::parse(R"(
    [
        {
            "title" : "Kafka on the Shore",
            "author" : "Haruki Murakami",
            "price" : 25.17
        },
        {
            "title" : "Women: A Novel",
            "author" : "Charles Bukowski",
            "price" : 12.00
        },
        {
            "title" : "Cutter's Way",
            "author" : "Ivan Passer"
        }
    ]
    )");

    json_options options;

    for (const auto& book : books.array_range())
    {
        try
        {
            std::string author = book["author"].as<std::string>();
            std::string title = book["title"].as<std::string>();
            std::string price;
            book.get_with_default<json>("price", "N/A").dump(price,options);
            std::cout << author << ", " << title << ", " << price << std::endl;
        }
        catch (const ser_error& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
}

void first_example_d()
{
    std::string path = "./input/books.json"; 
    std::fstream is(path);
    if (!is)
    {
        std::cout << "Cannot open " << path << std::endl;
        return;
    }
    json books = json::parse(is);

    json_options options;
    //options.floatfield(std::ios::fixed);
    options.precision(2);

    for (size_t i = 0; i < books.size(); ++i)
    {
        try
        {
            json& book = books[i];
            std::string author = book["author"].as<std::string>();
            std::string title = book["title"].as<std::string>();
            if (book.contains("price") && book["price"].is_number())
            {
                double price = book["price"].as<double>();
                std::cout << author << ", " << title << ", " << price << std::endl;
            }
            else
            {
                std::cout << author << ", " << title << ", " << "n/a" << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }

}

void second_example_a()
{
    try
    {
        json books = json::array();

        {
            json book;
            book["title"] = "Kafka on the Shore";
            book["author"] = "Haruki Murakami";
            book["price"] = 25.17;
            books.push_back(std::move(book));
        }

        {
            json book;
            book["title"] = "Women: A Novel";
            book["author"] = "Charles Bukowski";
            book["price"] = 12.00;
            books.push_back(std::move(book));
        }

        {
            json book;
            book["title"] = "Cutter's Way";
            book["author"] = "Ivan Passer";
            books.push_back(std::move(book));
        }

        std::cout << pretty_print(books) << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void json_constructor_examples()
{   
    json j1; // An empty object
    std::cout << "(1) " << j1 << std::endl;

    json j2 = json::object({{"baz", "qux"}, {"foo", "bar"}}); // An object 
    std::cout << "(2) " << j2 << std::endl;

    json j3 = json::array({"bar", "baz"}); // An array 
    std::cout << "(3) " << j3 << std::endl;
  
    json j4(json::null()); // A null value
    std::cout << "(4) " << j4 << std::endl;
    
    json j5(true); // A boolean value
    std::cout << "(5) " << j5 << std::endl;

    double x = 1.0/7.0;

    json j6(x); // A double value
    std::cout << "(6) " << j6 << std::endl;

    json j8("Hello"); // A text string
    std::cout << "(8) " << j8 << std::endl;

    std::vector<uint8_t> bs = {'H','e','l','l','o'};
    json j9(byte_string_view(bs.data(), bs.size())); // A byte string
    std::cout << "(9) " << j9 << std::endl;

    std::vector<int> v = {10,20,30};
    json j10 = v; // From a sequence container
    std::cout << "(10) " << j10 << std::endl;

    std::map<std::string, int> m{ {"one", 1}, {"two", 2}, {"three", 3} };
    json j11 = m; // From an associative container
    std::cout << "(11) " << j11 << std::endl;

    // An object value with four members
    json obj;
    obj["first_name"] = "Jane";
    obj["last_name"] = "Roe";
    obj["events_attended"] = 10;
    obj["accept_waiver_of_liability"] = true;

    std::string first_name = obj["first_name"].as<std::string>();
    std::string last_name = obj.at("last_name").as<std::string>();
    int events_attended = obj["events_attended"].as<int>();
    bool accept_waiver_of_liability = obj["accept_waiver_of_liability"].as<bool>();

    // An array value with four elements
    json arr = json::array();
    arr.push_back(j1);
    arr.push_back(j2);
    arr.push_back(j3);
    arr.push_back(j4);

    json_options options;
    std::cout << pretty_print(arr) << std::endl;
}

void object_range_based_for_loop()
{
    json j = json::parse(R"(
{
    "category" : "Fiction",
    "title" : "Pulp",
    "author" : "Charles Bukowski",
    "date" : "2004-07-08",
    "price" : 22.48,
    "isbn" : "1852272007"  
}
)");

    for (const auto& member : j.object_range())
    {
        std::cout << member.key() << " => " << member.value().as<std::string>() << std::endl;
    }
}

void parse_error_example()
{
    std::string s = "[1,2,3,4,]";
    try 
    {
        jsoncons::json val = jsoncons::json::parse(s);
    } 
    catch(const jsoncons::ser_error& e) 
    {
        std::cout << "Caught ser_error with category " << e.code().category().name() 
                  << ", code " << e.code().value() 
                  << " and message " << e.what() << std::endl;
    }
}

void validation_example()
{
    std::string s = R"(
{
    "StartDate" : "2017-03-01",
    "MaturityDate" "2020-12-30"          
}
    )";
    std::stringstream is(s);

    json_reader reader(is);

    std::error_code ec;
    reader.read(ec);
    if (ec)
    {
        std::cout << ec.message() 
                  << " on line " << reader.line()
                  << " and column " << reader.column()
                  << std::endl;
    }
}

void max_nesting_path_example()
{
    std::string s = "[[[[[[[[[[[[[[[[[[[[[\"Too deep\"]]]]]]]]]]]]]]]]]]]]]";
    try
    {
        json_options options;
        options.max_nesting_depth(20);
        json::parse(s, options);
    }
    catch (const ser_error& e)
    {
         std::cout << e.what() << std::endl;
    }
}

void get_example()
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

int main()
{
    try
    {

        std::cout << "jsoncons version: " << version() << std::endl;

        object_range_based_for_loop();

        basics_examples();
        basics_wexamples();
        ojson_examples();

        first_example_a();
        first_example_b();
        first_example_c();
        first_example_d();

        second_example_a();

        array_examples();
        container_examples();

        wjson_examples();

        unicode_examples();

        parse_error_example();

        json_content_filter_examples();

        msgpack_examples();

        validation_example();

        comment_example();

        json_constructor_examples();

        jsonpatch_examples();

        max_nesting_path_example();

        get_example();

        json_parser_examples();

        data_model_examples();

        staj_iterator_examples();
 
        bson_examples();

        serialization_examples();

        jsonpointer_examples();

        byte_string_examples();

        jsonpath_examples();

        json_cursor_examples();

        json_accessor_examples();

        json_accessor_examples();

        run_ubjson_examples();

        run_cbor_examples();

        readme_examples();

        type_extensibility_examples();

        json_reader_examples();

        json_type_traits_macros_named_examples();

        run_csv_examples();

        run_cbor_typed_array_examples();

        json_type_traits_macros_examples();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    return 0;
}
