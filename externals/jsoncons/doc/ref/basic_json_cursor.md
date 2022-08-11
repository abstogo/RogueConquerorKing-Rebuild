### jsoncons::basic_json_cursor

```c++
#include <jsoncons/json_cursor.hpp>

template<
    class CharT,
    class Src=jsoncons::stream_source<CharT>,
    class Allocator=std::allocator<char>> basic_json_cursor;
```

A pull parser for reporting JSON parse events. A typical application will 
repeatedly process the `current()` event and call the `next()`
function to advance to the next event, until `done()` returns `true`.

`basic_json_cursor` is noncopyable and nonmoveable.

Typedefs for common character types are provided:

Type                |Definition
--------------------|------------------------------
json_cursor     |basic_json_cursor<char>
wjson_cursor    |basic_json_cursor<wchar_t>

### Implemented interfaces

[basic_staj_reader](staj_reader.md)

#### Constructors

    template <class Source>
    basic_json_cursor(Source&& source, 
                      const basic_json_decode_options<CharT>& options = basic_json_decode_options<CharT>(),
                      std::function<bool(json_errc,const ser_context&)> err_handler = default_json_parsing()); // (1)

    template <class Source>
    basic_json_cursor(Source&& source, 
                      std::function<bool(const basic_staj_event<CharT>&, const ser_context&)> filter,
                      const basic_json_decode_options<CharT>& options = basic_json_decode_options<CharT>(),
                      std::function<bool(json_errc,const ser_context&)> err_handler = default_json_parsing()); // (2)

    template <class Source>
    basic_json_cursor(Source&& source, std::error_code& ec); // (3)

    template <class Source>
    basic_json_cursor(Source&& source, 
                      const basic_json_decode_options<CharT>& options,
                      std::error_code& ec) // (4)

    template <class Source>
    basic_json_cursor(Source&& source, 
                      const basic_json_decode_options<CharT>& options,
                      std::function<bool(json_errc,const ser_context&)> err_handler,
                      std::error_code& ec) // (5)

    template <class Source>
    basic_json_cursor(Source&& source,
                      std::function<bool(const basic_staj_event<CharT>&, const ser_context&)> filter,
                      std::error_code& ec); // (6)

    template <class Source>
    basic_json_cursor(Source&& source, 
                      std::function<bool(const basic_staj_event<CharT>&, const ser_context&)> filter,
                      const basic_json_decode_options<CharT>& options,
                      std::error_code& ec) // (7)

    template <class Source>
    basic_json_cursor(Source&& source, 
                      std::function<bool(const basic_staj_event<CharT>&, const ser_context&)> filter,
                      const basic_json_decode_options<CharT>& options,
                      std::function<bool(json_errc,const ser_context&)> err_handler,
                      std::error_code& ec) // (8)


Constructors (1)-(2) read from a character sequence or stream and throw a 
[ser_error](ser_error.md) if a parsing error is encountered while processing the initial event.
Constructors (3)-(8) read from a character sequence or stream and set `ec`
if a parsing error is encountered while processing the initial event.

Note: It is the programmer's responsibility to ensure that `basic_json_cursor` does not outlive any source or 
content handler, as `basic_json_cursor` holds pointers to but does not own these resources.

#### Parameters

`source` - a value from which a `jsoncons::basic_string_view<char_type>` is constructible, 
or a value from which a `source_type` is constructible. In the case that a `jsoncons::basic_string_view<char_type>` is constructible
from `source`, `source` is dispatched immediately to the parser. Otherwise, the `json_cursor` reads from a `source_type` in chunks. 

#### Member functions

    bool done() const override;
Checks if there are no more events.

    const basic_staj_event& current() const override;
Returns the current [basic_staj_event](staj_event.md).

    void read(json_content_handler& handler) override
Feeds the current and succeeding [staj events](staj_event.md) through the provided
[handler](basic_json_content_handler.md), until the handler indicates
to stop. If a parsing error is encountered, throws a [ser_error](ser_error.md).

    void read(basic_json_content_handler<char_type>& handler,
                std::error_code& ec) override
Feeds the current and succeeding [staj events](staj_event.md) through the provided
[handler](basic_json_content_handler.md), until the handler indicates
to stop. If a parsing error is encountered, sets `ec`.

    void next() override;
Advances to the next event. If a parsing error is encountered, throws a 
[ser_error](ser_error.md).

    void next(std::error_code& ec) override;
Advances to the next event. If a parsing error is encountered, sets `ec`.

    const ser_context& context() const override;
Returns the current [context](ser_context.md)

### Examples

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
#include <jsoncons/json_cursor.hpp>
#include <string>
#include <fstream>

using namespace jsoncons;

int main()
{
    std::ifstream is("book_catalog.json");

    json_cursor cursor(is);

    for (; !cursor.done(); cursor.next())
    {
        const auto& event = cursor.current();
        switch (event.event_type())
        {
            case staj_event_type::begin_array:
                std::cout << "begin_array\n";
                break;
            case staj_event_type::end_array:
                std::cout << "end_array\n";
                break;
            case staj_event_type::begin_object:
                std::cout << "begin_object\n";
                break;
            case staj_event_type::end_object:
                std::cout << "end_object\n";
                break;
            case staj_event_type::name:
                // Or std::string_view, if supported
                std::cout << "name: " << event.get<jsoncons::string_view>() << "\n";
                break;
            case staj_event_type::string_value:
                // Or std::string_view, if supported
                std::cout << "string_value: " << event.get<jsoncons::string_view>() << "\n";
                break;
            case staj_event_type::null_value:
                std::cout << "null_value: " << "\n";
                break;
            case staj_event_type::bool_value:
                std::cout << "bool_value: " << std::boolalpha << event.get<bool>() << "\n";
                break;
            case staj_event_type::int64_value:
                std::cout << "int64_value: " << event.get<int64_t>() << "\n";
                break;
            case staj_event_type::uint64_value:
                std::cout << "uint64_value: " << event.get<uint64_t>() << "\n";
                break;
            case staj_event_type::double_value:
                std::cout << "double_value: " << event.get<double>() << "\n";
                break;
            default:
                std::cout << "Unhandled event type\n";
                break;
        }
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

#### Filter JSON parse events

```c++
#include <jsoncons/json_cursor.hpp>
#include <string>
#include <fstream>

using namespace jsoncons;

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

int main()
{
    std::ifstream is("book_catalog.json");

    author_filter filter;
    json_cursor cursor(is, filter);

    for (; !cursor.done(); cursor.next())
    {
        const auto& event = cursor.current();
        switch (event.event_type())
        {
            case staj_event_type::string_value:
                std::cout << event.as<jsoncons::string_view>() << "\n";
                break;
        }
    }
}
```
Output:
```
Haruki Murakami
Graham Greene
```

### See also

- [staj_reader](staj_reader.md) 
- [staj_array_iterator](staj_array_iterator.md) 
- [staj_object_iterator](staj_object_iterator.md)

