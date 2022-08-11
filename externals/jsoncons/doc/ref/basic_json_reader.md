### jsoncons::basic_json_reader

```c++
#include <jsoncons/json_reader.hpp>

template<
    class CharT,
    class Src=jsoncons::stream_source<CharT>,
    class WorkAllocator=std::allocator<char>
>
class basic_json_reader 
```
`basic_json_reader` uses the incremental parser [basic_json_parser](json_parser.md) 
to read arbitrarily large files in chunks.
A `basic_json_reader` can read a sequence of JSON texts from a stream, using `read_next()`,
which omits the check for unconsumed non-whitespace characters. 

`basic_json_reader` is noncopyable and nonmoveable.

Two specializations for common character types are defined:

Type                       |Definition
---------------------------|------------------------------
json_reader            |basic_json_reader<char>
wjson_reader           |basic_json_reader<wchar_t>

#### Member types

Type                       |Definition
---------------------------|------------------------------
char_type                  |CharT
source_type                |Src
string_view_type           |

#### Constructors

    template <class Source>
    explicit basic_json_reader(Source&& source, 
                               const WorkAllocator& allocator = WorkAllocator()); // (1)

    template <class Source>
    basic_json_reader(Source&& source, 
                      const basic_json_options<CharT>& options, 
                      const WorkAllocator& allocator = WorkAllocator()); // (2)

    template <class Source>
    basic_json_reader(Source&& source,
                      std::function<bool(json_errc,const ser_context&)> err_handler, 
                      const WorkAllocator& allocator = WorkAllocator()); // (3)

    template <class Source>
    basic_json_reader(Source&& source, 
                      const basic_json_options<CharT>& options,
                      std::function<bool(json_errc,const ser_context&)> err_handler, 
                      const WorkAllocator& allocator = WorkAllocator()); // (4)

    template <class Source>
    basic_json_reader(Source&& source, 
                      basic_json_content_handler<CharT>& handler, 
                      const WorkAllocator& allocator = WorkAllocator()); // (5)

    template <class Source>
    basic_json_reader(Source&& source, 
                      basic_json_content_handler<CharT>& handler,
                      const basic_json_options<CharT>& options, 
                      const WorkAllocator& allocator = WorkAllocator()); // (6)

    template <class Source>
    basic_json_reader(Source&& source,
                      basic_json_content_handler<CharT>& handler,
                      std::function<bool(json_errc,const ser_context&)> err_handler, 
                      const WorkAllocator& allocator = WorkAllocator()); // (7)

    template <class Source>
    basic_json_reader(Source&& source,
                      basic_json_content_handler<CharT>& handler, 
                      const basic_json_options<CharT>& options,
                      std::function<bool(json_errc,const ser_context&)> err_handler, 
                      const WorkAllocator& allocator = WorkAllocator()); // (8)

Constructors (1)-(4) use a default [basic_json_content_handler](basic_json_content_handler.md) that discards the JSON parse events, and are for validation only.

(1) Constructs a `basic_json_reader` that reads from a character sequence or stream `source`, uses default [options](basic_json_options.md) and a default [parse_error_handler](parse_error_handler.md).

(2) Constructs a `basic_json_reader` that reads from a character sequence or stream `source`, 
uses the specified [options](basic_json_options.md)
and a default [parse_error_handler](parse_error_handler.md).

(3) Constructs a `basic_json_reader` that reads from a character sequence or stream `source`, 
uses default [options](basic_json_options.md)
and a specified [parse_error_handler](parse_error_handler.md).

(4) Constructs a `basic_json_reader` that reads from a character sequence or stream `source`, 
uses the specified [options](basic_json_options.md)
and a specified [parse_error_handler](parse_error_handler.md).

Constructors (5)-(8) take a user supplied [basic_json_content_handler](basic_json_content_handler.md) that receives JSON parse events, such as a [json_decoder](json_decoder). 

(5) Constructs a `basic_json_reader` that reads from a character sequence or stream `source`,
emits JSON parse events to the specified 
[basic_json_content_handler](basic_json_content_handler.md), and uses default [options](basic_json_options.md)
and a default [parse_error_handler](parse_error_handler.md).

(6) Constructs a `basic_json_reader` that reads from a character sequence or stream `source`,
emits JSON parse events to the specified [basic_json_content_handler](basic_json_content_handler.md) 
and uses the specified [options](basic_json_options.md)
and a default [parse_error_handler](parse_error_handler.md).

(7) Constructs a `basic_json_reader` that reads from a character sequence or stream `source`,
emits JSON parse events to the specified [basic_json_content_handler](basic_json_content_handler.md) 
and uses default [options](basic_json_options.md)
and a specified [parse_error_handler](parse_error_handler.md).

(8) Constructs a `basic_json_reader` that reads from a character sequence or stream `source`,
emits JSON parse events to the specified [basic_json_content_handler](basic_json_content_handler.md) and
uses the specified [options](basic_json_options.md)
and a specified [parse_error_handler](parse_error_handler.md).

Note: It is the programmer's responsibility to ensure that `basic_json_reader` does not outlive any source or 
content handler passed in the constuctor, as `basic_json_reader` holds pointers to but does not own these resources.

#### Parameters

`source` - a value from which a `jsoncons::basic_string_view<char_type>` is constructible, 
or a value from which a `source_type` is constructible. In the case that a `jsoncons::basic_string_view<char_type>` is constructible
from `source`, `source` is dispatched immediately to the parser. Otherwise, the `json_reader` reads from a `source_type` in chunks. 

#### Member functions

    bool eof() const
Returns `true` when there are no more JSON texts to be read from the stream, `false` otherwise

    void read(); // (1)
    void read(std::error_code& ec); // (2)
Reads the next JSON text from the stream and reports JSON events to a [basic_json_content_handler](basic_json_content_handler.md), such as a [json_decoder](json_decoder.md).
Override (1) throws if parsing fails, or there are any unconsumed non-whitespace characters left in the input.
Override (2) sets `ec` to a [json_errc](jsoncons::json_errc.md) if parsing fails or if there are any unconsumed non-whitespace characters left in the input.

    void read_next()
    void read_next(std::error_code& ec)
Reads the next JSON text from the stream and reports JSON events to a [basic_json_content_handler](basic_json_content_handler.md), such as a [json_decoder](json_decoder.md).
Override (1) throws [ser_error](ser_error.md) if parsing fails.
Override (2) sets `ec` to a [json_errc](jsoncons::json_errc.md) if parsing fails.

    void check_done(); // (1)
    void check_done(std::error_code& ec); // (2)
Override (1) throws if there are any unconsumed non-whitespace characters in the input.
Override (2) sets `ec` to a [json_errc](jsoncons::json_errc.md) if there are any unconsumed non-whitespace characters left in the input.

    size_t buffer_length() const

    void buffer_length(size_t length)

    size_t line() const

    size_t column() const

### Examples

#### Parsing JSON text with exceptions
```
std::string input = R"({"field1"{}})";    

json_decoder<json> decoder;
json_reader reader(input,decoder);

try
{
    reader.read();
    json j = decoder.get_result();
}
catch (const ser_error& e)
{
    std::cout << e.what() << std::endl;
}

```
Output:
```
Expected name separator ':' at line 1 and column 10
```

#### Parsing JSON text with error codes
```
std::string input = R"({"field1":ru})";    
std::istringstream is(input);

json_decoder<json> decoder;
json_reader reader(is,decoder);

std::error_code ec;
reader.read(ec);

if (!ec)
{
    json j = decoder.get_result();   
}
else
{
    std::cerr << ec.message() 
              << " at line " << reader.line() 
              << " and column " << reader.column() << std::endl;
}
```
Output:
```
Expected value at line 1 and column 11
```

#### Reading a sequence of JSON texts from a stream

`jsoncons` supports reading a sequence of JSON texts, such as shown below (`json-texts.json`):
```json
{"a":1,"b":2,"c":3}
{"a":4,"b":5,"c":6}
{"a":7,"b":8,"c":9}
```
This is the code that reads them: 
```c++
std::ifstream is("json-texts.json");
if (!is.is_open())
{
    throw std::runtime_error("Cannot open file");
}

json_decoder<json> decoder;
json_reader reader(is,decoder);

while (!reader.eof())
{
    reader.read_next();
    if (!reader.eof())
    {
        json val = decoder.get_result();
        std::cout << val << std::endl;
    }
}
```
Output:
```json
{"a":1,"b":2,"c":3}
{"a":4,"b":5,"c":6}
{"a":7,"b":8,"c":9}
```
