// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

TEST_CASE("test_byte_string_serialization")
{
    std::vector<uint8_t> bs = {'H','e','l','l','o'};
    json j(byte_string_view(bs.data(), bs.size()));

    std::ostringstream os;
    os << j;

    std::string expected; 
    expected.push_back('\"');
    encode_base64url(bs.begin(),bs.end(),expected);
    expected.push_back('\"');

    //std::cout << expected << " " << os.str() << std::endl;

    CHECK(os.str() == expected);
}

