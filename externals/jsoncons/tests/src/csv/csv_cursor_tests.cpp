// Copyright 2018 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <jsoncons_ext/csv/csv_cursor.hpp>
#include <jsoncons_ext/csv/csv.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

TEST_CASE("csv_cursor n_rows test")
{
    const std::string data = R"(index_id,observation_date,rate
EUR_LIBOR_06M,2015-10-23,0.0000214
EUR_LIBOR_06M,2015-10-26,0.0000143
EUR_LIBOR_06M,2015-10-27,0.0000001
)";

    SECTION("n_rows test")
    {
        csv::csv_options options;
        options.assume_header(true)
               .mapping(csv::mapping_kind::n_rows);
        csv::csv_cursor cursor(data, options);

        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("index_id"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("observation_date"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("rate"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("EUR_LIBOR_06M"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("2015-10-23"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::double_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("EUR_LIBOR_06M"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::double_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("EUR_LIBOR_06M"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::double_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();
        CHECK(cursor.done());
    }
    SECTION("m_columns test")
    {
        csv::csv_options options;
        options.assume_header(true)
               .mapping(csv::mapping_kind::m_columns);
        csv::csv_cursor cursor(data, options);

        CHECK(cursor.current().event_type() == staj_event_type::begin_object);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        CHECK(cursor.current().get<std::string>() == std::string("index_id"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::name);
        CHECK(cursor.current().get<std::string>() == std::string("observation_date"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::name);
        CHECK(cursor.current().get<std::string>() == std::string("rate"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::double_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::double_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::double_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::end_object);
        cursor.next();
        CHECK(cursor.done());
    }
}

TEST_CASE("csv_cursor n_rows with quotes test")
{
    const std::string data = R"("index_id","observation_date","rate"
EUR_LIBOR_06M,2015-10-23,0.0000214
EUR_LIBOR_06M,2015-10-26,0.0000143
EUR_LIBOR_06M,2015-10-27,0.0000001
)";

    SECTION("test 1")
    {
        csv::csv_options options;
        options.assume_header(true)
               .mapping(csv::mapping_kind::n_rows);
        csv::csv_cursor cursor(data, options);
        /* for (; !cursor.done(); cursor.next())
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
        }*/

        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("index_id"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("observation_date"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("rate"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::double_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::double_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::double_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();
        CHECK(cursor.done());
    }
}

TEST_CASE("csv_cursor n_objects test")
{
    const std::string data = R"(index_id,observation_date,rate
EUR_LIBOR_06M,2015-10-23,0.0000214
EUR_LIBOR_06M,2015-10-26,0.0000143
EUR_LIBOR_06M,2015-10-27,0.0000001
)";

    SECTION("test 2")
    {
        csv::csv_options options;
        options.assume_header(true)
               .mapping(csv::mapping_kind::n_objects);
        csv::csv_cursor cursor(data, options);
/*
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
*/
        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::begin_object);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        CHECK(cursor.current().get<std::string>() == std::string("index_id"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        CHECK(cursor.current().get<std::string>() == std::string("observation_date"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        CHECK(cursor.current().get<std::string>() == std::string("rate"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::double_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_object);
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::begin_object);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        CHECK(cursor.current().get<std::string>() == std::string("index_id"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        CHECK(cursor.current().get<std::string>() == std::string("observation_date"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        CHECK(cursor.current().get<std::string>() == std::string("rate"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::double_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_object);
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::begin_object);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        CHECK(cursor.current().get<std::string>() == std::string("index_id"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        CHECK(cursor.current().get<std::string>() == std::string("observation_date"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        CHECK(cursor.current().get<std::string>() == std::string("rate"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::double_value);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_object);
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();
        CHECK(cursor.done());
    }
}

TEST_CASE("csv_cursor n_objects subfields test")
{
    const std::string data = R"(calculationPeriodCenters,paymentCenters,resetCenters
NY;LON,TOR,LON
NY,LON,TOR;LON
"NY";"LON","TOR","LON"
"NY","LON","TOR";"LON"
)";

    SECTION("test 1")
    {
        csv::csv_options options;
        options.assume_header(true)
               .subfield_delimiter(';');

        csv::csv_cursor cursor(data, options);

        /* 
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
        */ 

        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::begin_object);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        CHECK(cursor.current().get<std::string>() == std::string("calculationPeriodCenters"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("NY"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("LON"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        CHECK(cursor.current().get<std::string>() == std::string("paymentCenters"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("TOR"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        CHECK(cursor.current().get<std::string>() == std::string("resetCenters"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("LON"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_object);
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::begin_object);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        CHECK(cursor.current().get<std::string>() == std::string("calculationPeriodCenters"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("NY"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        CHECK(cursor.current().get<std::string>() == std::string("paymentCenters"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("LON"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        CHECK(cursor.current().get<std::string>() == std::string("resetCenters"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("TOR"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("LON"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_object);
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::begin_object);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        CHECK(cursor.current().get<std::string>() == std::string("calculationPeriodCenters"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("NY"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("LON"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        CHECK(cursor.current().get<std::string>() == std::string("paymentCenters"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("TOR"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        CHECK(cursor.current().get<std::string>() == std::string("resetCenters"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("LON"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_object);
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::begin_object);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        CHECK(cursor.current().get<std::string>() == std::string("calculationPeriodCenters"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("NY"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        CHECK(cursor.current().get<std::string>() == std::string("paymentCenters"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("LON"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::name);
        CHECK(cursor.current().get<std::string>() == std::string("resetCenters"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("TOR"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("LON"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_object);
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();
        CHECK(cursor.done()); 

    }
}

TEST_CASE("csv_cursor n_rows, no header test")
{
    std::string data = "\"b\"";

    SECTION("test 1")
    {
        csv::csv_options options;
        options.mapping(csv::mapping_kind::n_rows)
               .assume_header(false);

        csv::csv_cursor cursor(data, options);
        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("b"));
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();
        CHECK(cursor.done()); 
    }
}

TEST_CASE("csv_cursor n_objects, header test")
{
    std::string data = "a\n\"4\"";

    SECTION("test 2")
    {
        csv::csv_options options;
        options.assume_header(true);

        csv::csv_cursor cursor(data, options);

        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::begin_object);
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::name);
        CHECK(cursor.current().get<std::string>() == std::string("a"));
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("4"));
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::end_object);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();
        CHECK(cursor.done());
    }
}

TEST_CASE("csv_cursor header, subfield no terminating new line test")
{
    std::string data = "a\n4;-5";

    SECTION("test 1")
    {
        csv::csv_options options;
        options.assume_header(true)
               .subfield_delimiter(';')
               .mapping(csv::mapping_kind::n_rows);
        csv::csv_cursor cursor(data, options);

        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::string_value);
        CHECK(cursor.current().get<std::string>() == std::string("a"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::uint64_value);
        CHECK(cursor.current().get<int>() == 4);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::int64_value);
        CHECK(cursor.current().get<int>() == -5);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();
        CHECK(cursor.done());
    }

    SECTION("test 2")
    {
        csv::csv_options options;
        options.assume_header(true)
               .subfield_delimiter(';');
        csv::csv_cursor cursor(data, options);

        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::begin_object);
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::name);
        CHECK(cursor.current().get<std::string>() == std::string("a"));
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::begin_array);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::uint64_value);
        CHECK(cursor.current().get<int>() == 4);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::int64_value);
        CHECK(cursor.current().get<int>() == -5);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();

        CHECK(cursor.current().event_type() == staj_event_type::end_object);
        cursor.next();
        CHECK(cursor.current().event_type() == staj_event_type::end_array);
        cursor.next();
        CHECK(cursor.done());
    }
}
struct remove_mark_csv_filter
{
    bool reject_next_ = false;

    bool operator()(const staj_event& event, const ser_context&) 
    {
        if (event.event_type()  == staj_event_type::name &&
            event.get<jsoncons::string_view>() == "mark")
        {
            reject_next_ = true;
            return false;
        }
        else if (reject_next_)
        {
            reject_next_ = false;
            return false;
        }
        else
        {
            return true;
        }
    }
};

TEST_CASE("csv_cursor with filter tests")
{
    auto j = ojson::parse(R"(
    [
        {
            "enrollmentNo" : 100,
            "firstName" : "Tom",
            "lastName" : "Cochrane",
            "mark" : 55
        },
        {
            "enrollmentNo" : 101,
            "firstName" : "Catherine",
            "lastName" : "Smith",
            "mark" : 95
        },
        {
            "enrollmentNo" : 102,
            "firstName" : "William",
            "lastName" : "Skeleton",
            "mark" : 60
        }
    ]
    )");

    std::string data;
    csv::encode_csv(j, data);

    csv::csv_options options;
    options.assume_header(true);
    csv::csv_cursor cursor(data, remove_mark_csv_filter(), options);

    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::begin_array);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::begin_object);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::name);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::uint64_value);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::name);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::string_value);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::name);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::string_value);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::end_object);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::begin_object);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::name);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::uint64_value);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::name);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::string_value);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::name);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::string_value);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::end_object);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::begin_object);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::name);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::uint64_value);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::name);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::string_value);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::name);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::string_value);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::end_object);
    cursor.next();
    REQUIRE_FALSE(cursor.done());
    CHECK(cursor.current().event_type() == staj_event_type::end_array);
    cursor.next();
    CHECK(cursor.done());
}

