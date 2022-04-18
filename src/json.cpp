#include "json.h"

#include <limits>

using namespace wingmann;

json::backing_data::backing_data(json::float_type value) : json_float{value}
{
}

json::backing_data::backing_data(json::int_type value) : json_int{value}
{
}

json::backing_data::backing_data(json::bool_type value) : json_bool{value}
{
}

json::backing_data::backing_data(json::string_type value)
    : json_string{new string_type{std::move(value)}}
{
}

json::json(std::initializer_list<json> list) : json{}
{
    set_type(class_type::object);
    for (auto i = list.begin(), e = list.end(); i != e; ++i, ++i)
        operator[](i->to_string()) = *std::next(i);
}

json::json(json&& other) noexcept : internal_{other.internal_}, type_{other.type_}
{
    other.type_ = class_type::null;
    other.internal_.json_map = nullptr;
}

json::json(const json& other)
{
    switch (other.type_) {
    case class_type::object:
        internal_.json_map =
            new map_type{other.internal_.json_map->begin(), other.internal_.json_map->end()};
        break;
    case class_type::array:
        internal_.json_list =
            new list_type{other.internal_.json_list->begin(), other.internal_.json_list->end()};
        break;
    case class_type::string:
        internal_.json_string = new string_type{*other.internal_.json_string};
        break;
    default:
        internal_ = other.internal_;
        break;
    }
    type_ = other.type_;
}

json::json(std::nullptr_t) : internal_{}, type_{class_type::null}
{
}

json::~json()
{
    switch (type_) {
    case class_type::array:
        delete internal_.json_list;
        break;
    case class_type::object:
        delete internal_.json_map;
        break;
    case class_type::string:
        delete internal_.json_string;
        break;
    default:
        break;
    }
}

json& json::operator=(json&& other) noexcept
{
    clear_internal();

    internal_ = other.internal_;
    type_ = other.type_;
    other.internal_.json_map = nullptr;
    other.type_ = class_type::null;
    return *this;
}

json& json::operator=(const json& other)
{
    clear_internal();

    switch (other.type_) {
    case class_type::object:
        internal_.json_map =
            new map_type{other.internal_.json_map->begin(), other.internal_.json_map->end()};
        break;
    case class_type::array:
        internal_.json_list =
            new list_type{other.internal_.json_list->begin(), other.internal_.json_list->end()};
        break;
    case class_type::string:
        internal_.json_string = new string_type{*other.internal_.json_string};
        break;
    default:
        internal_ = other.internal_;
        break;
    }
    type_ = other.type_;
    return *this;
}

json& json::operator[](const string_type& key)
{
    set_type(class_type::object);
    return internal_.json_map->operator[](key);
}

json& json::operator[](unsigned int index)
{
    set_type(class_type::array);
    if (index >= internal_.json_list->size())
        internal_.json_list->resize(index + 1);

    return internal_.json_list->operator[](index);
}

json json::make(json::class_type type)
{
    json ret;
    ret.set_type(type);
    return ret;
}

json json::load(const string_type& value)
{
    size_type offset{};
    return std::move(parse_next(value, offset));
}

json& json::at(const string_type& key)
{
    return operator[](key);
}

const json& json::at(const string_type& key) const
{
    return internal_.json_map->at(key);
}

json& json::at(unsigned int index)
{
    return operator[](index);
}

const json& json::at(unsigned int index) const
{
    return internal_.json_list->at(index);
}

json::size_type json::length() const
{
    return (type_ == class_type::array)
        ? internal_.json_list->size()
        : std::numeric_limits<size_type>::max();
}

bool json::has_key(const string_type& key) const
{
    return (type_ == class_type::object) &&
           (internal_.json_map->find(key) != internal_.json_map->end());
}

json::size_type json::size() const
{
    switch (type_) {
    case class_type::object: return internal_.json_map->size();
    case class_type::array:  return internal_.json_list->size();
    default:                 return std::numeric_limits<size_type>::max();
    }
}

json::class_type json::json_type() const
{
    return type_;
}

bool json::is_null() const
{
    return type_ == class_type::null;
}

json::string_type json::to_string() const
{
    bool b;
    return std::move(to_string(b));
}

json::string_type json::to_string(bool& ok) const
{
    return (ok = type_ == class_type::string)
        ? std::move(json_escape(*internal_.json_string))
        : string_type{};
}

double json::to_float() const
{
    bool b;
    return to_float(b);
}

double json::to_float(bool& ok) const
{
    return (ok = type_ == class_type::floating)
        ? internal_.json_float
        : double{};
}

json::int_type json::to_int() const
{
    bool b;
    return to_int(b);
}

json::int_type json::to_int(bool& ok) const
{
    return (ok = type_ == class_type::integral)
        ? internal_.json_int
        : int_type{};
}

bool json::to_bool() const
{
    bool b;
    return to_bool(b);
}

bool json::to_bool(bool& ok) const
{
    return (ok = type_ == class_type::boolean) && internal_.json_bool;
}

auto json::array_range()
{
    return (type_ == class_type::array)
        ? json_list_wraper_type{internal_.json_list}
        : json_list_wraper_type{nullptr};
}

auto json::array_range() const
{
    return (type_ == class_type::array)
        ? json_const_list_wraper_type{internal_.json_list}
        : json_const_list_wraper_type{nullptr};
}

auto json::object_range()
{
    return (type_ == class_type::object)
        ? json_map_wraper_type{internal_.json_map}
        : json_map_wraper_type{nullptr};
}

auto json::object_range() const
{
    return (type_ == class_type::array)
        ? json_const_map_wraper_type{internal_.json_map}
        : json_const_map_wraper_type{nullptr};
}

json::string_type json::dump(int depth, const string_type& tab) const
{
    string_type pad;
    for (int i = 0; i < depth; ++i, pad += tab) { }

    switch (type_) {
    case class_type::null:
        return "null";
    case class_type::object:
    {
        string_type s{"{\n"};
        bool skip{true};

        for (auto& p : *internal_.json_map) {
            if (!skip)
                s += ",\n";

            s += (pad + "\"" + p.first + "\" : " + p.second.dump(depth + 1, tab));
            skip = false;
        }
        s += ("\n" + pad.erase(0, 2) + "}");
        return s;
    }
    case class_type::array:
    {
        string_type s{"["};
        bool skip{true};

        for (auto& p : *internal_.json_list) {
            if (!skip)
                s += ", ";

            s += p.dump(depth + 1, tab);
            skip = false;
        }
        s += "]";
        return s;
    }
    case class_type::string:   return "\"" + json_escape(*internal_.json_string) + "\"";
    case class_type::floating: return std::to_string(internal_.json_float);
    case class_type::integral: return std::to_string(internal_.json_int);
    case class_type::boolean:  return internal_.json_bool ? "true" : "false";
    default:                   return {};
    }
}

json json::array()
{
    return std::move(json::make(json::class_type::array));
}

json json::object()
{
    return std::move(json::make(json::class_type::object));
}

void json::set_type(json::class_type type)
{
    if (type == type_) return;
    clear_internal();

    switch (type) {
    case class_type::null:     internal_.json_map    = nullptr;           break;
    case class_type::object:   internal_.json_map    = new map_type{};    break;
    case class_type::array:    internal_.json_list   = new list_type{};   break;
    case class_type::string:   internal_.json_string = new string_type{}; break;
    case class_type::floating: internal_.json_float  = double{};          break;
    case class_type::integral: internal_.json_int    = int_type{};    break;
    case class_type::boolean:  internal_.json_bool   = false;             break;
    }
    type_ = type;
}

void json::clear_internal()
{
    switch (type_) {
    case class_type::object: delete internal_.json_map;    break;
    case class_type::array:  delete internal_.json_list;   break;
    case class_type::string: delete internal_.json_string; break;
    default:                                               break;
    }
}

void json::consume_ws(const string_type& str, size_type& offset)
{
    while (isspace(str[offset])) ++offset;
}

json json::parse_next(const string_type& str, size_type& offset)
{
    char value;
    consume_ws(str, offset);
    value = str[offset];

    switch (value) {
    case '[':
        return std::move(parse_array(str, offset));
    case '{':
        return std::move(parse_object(str, offset));
    case '\"':
        return std::move(parse_string(str, offset));
    case 't':
    case 'f':
        return std::move(parse_bool(str, offset));
    case 'n':
        return std::move(parse_null(str, offset));
    default:
        if ((value <= '9' && value >= '0') || value == '-')
            return std::move(parse_number(str, offset));
        break;
    }
    std::cerr << "ERROR: Parse: Unknown starting character '" << value << "'\n";
    return {};
}

json json::parse_object(const string_type& str, size_type& offset)
{
    auto json_object = json::make(json::class_type::object);

    ++offset;
    consume_ws(str, offset);

    if (str[offset] == '}') {
        ++offset;
        return std::move(json_object);
    }

    while (true) {
        auto key = parse_next(str, offset);
        consume_ws(str, offset);

        if (str[offset] != ':') {
            std::cerr << "Error: Object: Expected colon, found '" << str[offset] << "'\n";
            break;
        }
        consume_ws(str, ++offset);
        auto value = parse_next(str, offset);
        json_object[key.to_string()] = value;
        consume_ws(str, offset);

        if (str[offset] == ',') {
            ++offset;
            continue;
        }
        else if (str[offset] == '}') {
            ++offset;
            break;
        }
        else {
            std::cerr << "ERROR: Object: Expected comma, found '" << str[offset] << "'\n";
            break;
        }
    }
    return std::move(json_object);
}

json json::parse_array(const string_type& str, size_type& offset)
{
    auto json_array = json::make(json::class_type::array);
    size_type index{};

    ++offset;
    consume_ws(str, offset);

    if (str[offset] == ']') {
        ++offset;
        return std::move(json_array);
    }

    while (true) {
        json_array[index++] = parse_next(str, offset);
        consume_ws(str, offset);

        if (str[offset] == ',') {
            ++offset;
            continue;
        }
        else if (str[offset] == ']') {
            ++offset;
            break;
        }
        else {
            std::cerr << "ERROR: array: Expected ',' or ']', found '" << str[offset] << "'\n";
            return std::move(json::make(json::class_type::array));
        }
    }
    return std::move(json_array);
}

json json::parse_string(const string_type& str, size_type& offset)
{
    json json_string;
    string_type value;

    for (char c = str[++offset]; c != '\"'; c = str[++offset]) {
        if (c == '\\') {
            switch (str[++offset])
            {
            case '\"':    value += '\"';    break;
            case '\\':    value += '\\';    break;
            case '/':     value += '/';     break;
            case 'b':     value += '\b';    break;
            case 'f':     value += '\f';    break;
            case 'n':     value += '\n';    break;
            case 'r':     value += '\r';    break;
            case 't':     value += '\t';    break;
            case 'u':
            {
                value += "\\u";
                for (unsigned i = 1; i <= 4; ++i) {
                    c = str[offset + i];

                    if ((c >= '0' && c <= '9') ||
                        (c >= 'a' && c <= 'f') ||
                        (c >= 'A' && c <= 'F'))
                    {
                        value += c;
                    }
                    else {
                        std::cerr << "ERROR: String: Expected hex character in unicode escape, found '" << c
                                  << "'\n";
                        return std::move(json::make(json::class_type::string));
                    }
                }
                offset += 4;
                break;
            }
            default:
                value += '\\';
                break;
            }
        }
        else {
            value += c;
        }
    }
    ++offset;
    json_string = value;
    return std::move(json_string);
}

json json::parse_number(const string_type& str, size_type& offset)
{
    json number;
    string_type value;
    string_type expression_value;
    char c;
    bool_type is_floating{};
    int_type exp{};

    while (true) {
        c = str[offset++];

        if ((c == '-') || (c >= '0' && c <= '9')) {
            value += c;
        }
        else if (c == '.') {
            value += c;
            is_floating = true;
        }
        else {
            break;
        }
    }

    if (c == 'E' || c == 'e') {
        c = str[offset++];

        if (c == '-') {
            ++offset;
            expression_value += '-';
        }

        while (true) {
            c = str[offset++];
            if (c >= '0' && c <= '9') {
                expression_value += c;
            }
            else if (!isspace(c) && c != ',' && c != ']' && c != '}') {
                std::cerr << "ERROR: Number: Expected a number for exponent, found '" << c << "'\n";
                return std::move(json::make(json::class_type::null));
            }
            else {
                break;
            }
        }
        exp = std::stoll(expression_value);
    }
    else if (!isspace(c) && c != ',' && c != ']' && c != '}') {
        std::cerr << "ERROR: Number: unexpected character '" << c << "'\n";
        return std::move(json::make(json::class_type::null));
    }
    --offset;

    if (is_floating) {
        number = std::stod(value) * std::pow(10, exp);
    }
    else {
        number = (!expression_value.empty())
            ? static_cast<float_type>(std::stoll(value)) * std::pow(10, exp)
            : static_cast<float_type>(std::stoll(value));
    }

    return std::move(number);
}

json json::parse_bool(const string_type& str, size_type& offset)
{
    json json_bool;

    if (str.substr(offset, 4) == "true") {
        json_bool = true;
    }
    else if (str.substr(offset, 5) == "false") {
        json_bool = false;
    }
    else {
        std::cerr << "ERROR: Bool: Expected 'true' or 'false', found '"
                  << str.substr(offset, 5) << "'\n";

        return std::move(json::make(json::class_type::null));
    }
    offset += json_bool.to_bool() ? 4 : 5;
    return std::move(json_bool);
}

json json::parse_null(const string_type& str, size_type& offset)
{
    json json_null;

    if (str.substr(offset, 4) != "null") {
        std::cerr << "ERROR: Null: Expected 'null', found '" << str.substr(offset, 4) << "'\n";
        return std::move(json::make(json::class_type::null));
    }
    offset += 4;
    return std::move(json_null);
}

json::string_type json::json_escape(const string_type& value)
{
    string_type output;

    for (const char& i : value) {
        switch (i) {
            case '\"': output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            case '\b': output += "\\b";  break;
            case '\f': output += "\\f";  break;
            case '\n': output += "\\n";  break;
            case '\r': output += "\\r";  break;
            case '\t': output += "\\t";  break;
            default:   output += i;      break;
        }
    }
    return std::move(output);
}
