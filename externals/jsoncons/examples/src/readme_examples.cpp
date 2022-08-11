// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include "sample_types.hpp"
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>
#include <jsoncons_ext/csv/csv.hpp>
#include <string>
#include <vector>
#include <iomanip>

namespace readme
{
    using namespace jsoncons;    

    void as_a_variant_like_structure()
    {
        // Some JSON input data
        std::string data = R"(
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
        )";

        // Parse the string of data into a json value
        json j = json::parse(data);

        // Does object member reputons exist?
        std::cout << "(1) " << std::boolalpha << j.contains("reputons") << "\n\n";

        // Get a reference to reputons array 
        const json& v = j["reputons"]; 

        // Iterate over reputons array 
        std::cout << "(2)\n";
        for (const auto& item : v.array_range())
        {
            // Access rated as string and rating as double
            std::cout << item["rated"].as<std::string>() << ", " << item["rating"].as<double>() << "\n";
        }
        std::cout << "\n";

        // Select all "rated" with JSONPath
        std::cout << "(3)\n";
        json result = jsonpath::json_query(j,"$..rated");
        std::cout << pretty_print(result) << "\n\n";

        // Serialize back to JSON
        std::cout << "(4)\n" << pretty_print(j) << "\n\n";
    }

    void as_a_strongly_typed_cpp_structure()
    {
        // Some JSON input data
        std::string data = R"(
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
        )";

        // Decode the string of data into a c++ structure
        ns::hiking_reputation v = decode_json<ns::hiking_reputation>(data);

        // Iterate over reputons array value
        std::cout << "(1)\n";
        for (const auto& item : v.reputons())
        {
            std::cout << item.rated() << ", " << item.rating() << "\n";
        }

        // Encode the c++ structure into a string
        std::string s;
        encode_json<ns::hiking_reputation>(v, s, indenting::indent);
        std::cout << "(2)\n";
        std::cout << s << "\n";
    }

    void as_a_stream_of_json_events()
    {
        // Some JSON input data
        std::string data = R"(
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
        )";

        json_cursor cursor(data);
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
    }

    void as_a_filtered_stream_of_json_events()
    {
        // Some JSON input data
        std::string data = R"(
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
        )";

        std::string name;
        auto filter = [&](const staj_event& ev, const ser_context&) -> bool
        {
            if (ev.event_type() == staj_event_type::name)
            {
                name = ev.get<std::string>();
                return false;
            }
            else if (name == "rated")
            {
                name.clear();
                return true;
            }
            else
            {
                return false;
            }
        };

        json_cursor cursor(data, filter);

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
    }

    void playing_around()
    {
        // Construct some CBOR using the streaming API
        std::vector<uint8_t> b;
        cbor::cbor_bytes_encoder encoder(b);
        encoder.begin_array(); // indefinite length outer array
        encoder.begin_array(3); // a fixed length array
        encoder.string_value("foo");
        encoder.byte_string_value(byte_string{'P','u','s','s'}); // no suggested conversion
        encoder.string_value("-18446744073709551617", semantic_tag::bigint);
        encoder.end_array();
        encoder.end_array();
        encoder.flush();

        // Print bytes
        std::cout << "(1) ";
        for (auto c : b)
        {
            std::cout << std::hex << std::setprecision(2) << std::setw(2)
                      << std::setfill('0') << static_cast<int>(c);
        }
        std::cout << "\n\n";
/*
        9f -- Start indefinte length array
          83 -- Array of length 3
            63 -- String value of length 3
              666f6f -- "foo" 
            44 -- Byte string value of length 4
              50757373 -- 'P''u''s''s'
            c3 -- Tag 3 (negative bignum)
              49 -- Byte string value of length 9
                010000000000000000 -- Bytes content
          ff -- "break" 
*/
        // Unpack bytes into a json variant value, and add some more elements
        json j = cbor::decode_cbor<json>(b);

        // Loop over the rows
        std::cout << "(2)\n";
        for (const json& row : j.array_range())
        {
            std::cout << row << "\n";
        }
        std::cout << "\n";

        // Get bignum value at position 0/2 using jsonpointer 
        json& v = jsonpointer::get(j, "/0/2");
        std::cout << "(3) " << v.as<std::string>() << "\n\n";

        // Print JSON representation with default options
        std::cout << "(4)\n";
        std::cout << pretty_print(j) << "\n\n";

        // Print JSON representation with different options
        json_options options;
        options.byte_string_format(byte_string_chars_format::base64)
               .bigint_format(bigint_chars_format::base64url);
        std::cout << "(5)\n";
        std::cout << pretty_print(j, options) << "\n\n";

        // Add some more elements

        json another_array = json::array(); 
        another_array.emplace_back(byte_string({'P','u','s','s'}),
                                   semantic_tag::base64); // suggested conversion to base64
        another_array.emplace_back("273.15", semantic_tag::bigdec);
        another_array.emplace(another_array.array_range().begin(),"bar"); // place at front

        j.push_back(std::move(another_array));
        std::cout << "(6)\n";
        std::cout << pretty_print(j) << "\n\n";

        // Get big decimal value at position /1/2 using jsonpointer
        json& ref = jsonpointer::get(j, "/1/2");
        std::cout << "(7) " << ref.as<std::string>() << "\n\n";

#if (defined(__GNUC__) || defined(__clang__)) && (!defined(__ALL_ANSI__) && defined(_GLIBCXX_USE_INT128))
        // e.g. if code compiled with GCC and std=gnu++11 (rather than std=c++11)
        __int128 i = j[1][2].as<__int128>();
#endif

        // Get byte string value at position /1/1 as a byte_string
        byte_string bs = j[1][1].as<byte_string>();
        std::cout << "(8) " << bs << "\n\n";

        // or alternatively as a std::vector<uint8_t>
        std::vector<uint8_t> u = j[1][1].as<std::vector<uint8_t>>();

        // Repack bytes
        std::vector<uint8_t> b2;
        cbor::encode_cbor(j, b2);

        // Print the repacked bytes
        std::cout << "(9) ";
        for (auto c : b2)
        {
            std::cout << std::hex << std::setprecision(2) << std::setw(2)
                      << std::setfill('0') << static_cast<int>(c);
        }
        std::cout << "\n\n";
/*
        82 -- Array of length 2
          83 -- Array of length 3
            63 -- String value of length 3
              666f6f -- "foo" 
            44 -- Byte string value of length 4
              50757373 -- 'P''u''s''s'
            c3 -- Tag 3 (negative bignum)
            49 -- Byte string value of length 9
              010000000000000000 -- Bytes content
          83 -- Another array of length 3
          63 -- String value of length 3
            626172 -- "bar"
          d6 - Expected conversion to base64
          44 -- Byte string value of length 4
            50757373 -- 'P''u''s''s'
          c4 -- Tag 4 (decimal fraction)
            82 -- Array of length 2
              21 -- -2
              19 6ab3 -- 27315
*/
        // Encode to CSV
        csv::csv_options csv_options;
        csv_options.column_names("Column 1,Column 2,Column 3");

        std::cout << "(10)\n";
        csv::encode_csv(j, std::cout, csv_options);
    }
}

void readme_examples()
{
    std::cout << "\nReadme examples\n\n";

    readme::playing_around();
    readme::as_a_strongly_typed_cpp_structure();
    readme::as_a_filtered_stream_of_json_events();
    readme::as_a_stream_of_json_events();
    readme::as_a_variant_like_structure();
    std::cout << std::endl;
}

