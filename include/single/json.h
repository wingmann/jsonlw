#ifndef SINGLE_JSON_H
#define SINGLE_JSON_H

#include <cctype>
#include <cmath>
#include <cstdint>
#include <deque>
#include <initializer_list>
#include <iostream>
#include <map>
#include <ostream>
#include <string>
#include <type_traits>
#include <utility>

namespace json
{
    namespace
    {
        std::string json_escape(const std::string& str)
        {
            std::string output;
            for (const char& i : str)
            {
                switch (i)
                {
                case '\"':    output += "\\\"";    break;
                case '\\':    output += "\\\\";    break;
                case '\b':    output += "\\b";     break;
                case '\f':    output += "\\f";     break;
                case '\n':    output += "\\n";     break;
                case '\r':    output += "\\r";     break;
                case '\t':    output += "\\t";     break;
                default:      output += i;         break;
                }
            }
            return std::move(output);
        }
    }

    class json_t;

    namespace types
    {
        using list_t = std::deque<json_t>;
        using map_t = std::map<std::string, json_t>;
    }

    class json_t
    {
    private:
        union backing_data_t
        {
            types::list_t* json_list;
            types::map_t* json_map;
            std::string* json_string;
            double json_float;
            std::int64_t json_int{};
            bool json_bool;

            backing_data_t() = default;

            explicit backing_data_t(double d) : json_float{d}
            {
            }

            explicit backing_data_t(std::int64_t i) : json_int{i}
            {
            }

            explicit backing_data_t(bool b) : json_bool{b}
            {
            }

            explicit backing_data_t(std::string s) : json_string{new std::string{std::move(s)}}
            {
            }
        } internal_;

    public:
        enum class class_t
        {
            null,
            object,
            array,
            string,
            floating,
            integral,
            boolean
        };

        template<typename container_t>
        class json_wrapper_t
        {
        private:
            container_t* object_;

        public:
            explicit json_wrapper_t(container_t* val) : object_(val)
            {
            }

            explicit json_wrapper_t(std::nullptr_t) : object_(nullptr)
            {
            }

            typename container_t::iterator begin()
            {
                return object_ ? object_->begin() : typename container_t::iterator();
            }

            typename container_t::iterator end()
            {
                return object_ ? object_->end() : typename container_t::iterator();
            }

            [[nodiscard]] typename container_t::const_iterator begin() const
            {
                return object_ ? object_->begin() : typename container_t::iterator();
            }

            [[nodiscard]] typename container_t::const_iterator end() const
            {
                return object_ ? object_->end() : typename container_t::iterator();
            }
        };

        template<typename container_t>
        class json_const_wrapper_t
        {
        private:
            const container_t* object_;

        public:
            explicit json_const_wrapper_t(const container_t* value) : object_{value}
            {
            }

            explicit json_const_wrapper_t(std::nullptr_t) : object_{nullptr}
            {
            }

            [[nodiscard]] typename container_t::const_iterator begin() const
            {
                return object_ ? object_->begin() : typename container_t::const_iterator();
            }

            [[nodiscard]] typename container_t::const_iterator end() const
            {
                return object_ ? object_->end() : typename container_t::const_iterator();
            }
        };

        json_t() : internal_{}, type_{class_t::null}
        {
        }

        json_t(std::initializer_list<json_t> list) : json_t()
        {
            set_type(class_t::object);
            for (auto i = list.begin(), e = list.end(); i != e; ++i, ++i)
                operator[](i->to_string()) = *std::next(i);
        }

        json_t(json_t&& other) noexcept : internal_(other.internal_), type_(other.type_)
        {
            other.type_ = class_t::null;
            other.internal_.json_map = nullptr;
        }

        json_t& operator=(json_t&& other) noexcept
        {
            clear_internal();
            internal_ = other.internal_;
            type_ = other.type_;
            other.internal_.json_map = nullptr;
            other.type_ = class_t::null;
            return *this;
        }

        json_t(const json_t& other)
        {
            switch (other.type_)
            {
            case class_t::object:
                internal_.json_map =
                    new types::map_t{other.internal_.json_map->begin(), other.internal_.json_map->end()};
                break;
            case class_t::array:
                internal_.json_list =
                    new types::list_t{other.internal_.json_list->begin(), other.internal_.json_list->end()};
                break;
            case class_t::string:
                internal_.json_string = new std::string{*other.internal_.json_string};
                break;
            default:
                internal_ = other.internal_;
                break;
            }

            type_ = other.type_;
        }

        json_t& operator=(const json_t& other)
        {
            clear_internal();
            switch (other.type_)
            {
            case class_t::object:
                internal_.json_map =
                    new types::map_t{other.internal_.json_map->begin(), other.internal_.json_map->end()};
                break;
            case class_t::array:
                internal_.json_list =
                    new types::list_t{other.internal_.json_list->begin(), other.internal_.json_list->end()};
                break;
            case class_t::string:
                internal_.json_string = new std::string{*other.internal_.json_string};
                break;
            default:
                internal_ = other.internal_;
                break;
            }

            type_ = other.type_;
            return *this;
        }

        ~json_t()
        {
            switch (type_)
            {
            case class_t::array:
                delete internal_.json_list;
                break;
            case class_t::object:
                delete internal_.json_map;
                break;
            case class_t::string:
                delete internal_.json_string;
                break;
            default:
                break;
            }
        }

        template<typename T>
        requires std::is_same_v<T, bool>
        explicit json_t(T value, T* = nullptr) : internal_{value}, type_{class_t::boolean}
        {
        }

        template<typename T>
        requires std::is_integral_v<T> && (!std::is_same_v<T, bool>)
        explicit json_t(T value, T* = nullptr) : internal_{static_cast<std::int64_t>(value)}, type_{class_t::integral}
        {
        }

        template<typename T>
        requires std::is_floating_point_v<T>
        explicit json_t(T value, T* = nullptr) : internal_{static_cast<double>(value)}, type_{class_t::floating}
        {
        }

        template<typename T>
        requires std::is_convertible_v<T, std::string>
        explicit json_t(T value, T* = nullptr) : internal_{std::string{value}}, type_{class_t::string}
        {
        }

        explicit json_t(std::nullptr_t) : internal_{}, type_{class_t::null}
        {
        }

        static json_t make(class_t type)
        {
            json_t ret;
            ret.set_type(type);
            return ret;
        }

        static json_t load(const std::string&);

        template<typename T>
        void append(T arg)
        {
            set_type(class_t::array);
            internal_.json_list->emplace_back(arg);
        }

        template<typename T, typename... U>
        void append(T arg, U... args)
        {
            append(arg);
            append(args...);
        }

        template<typename T>
        requires std::is_same_v<T, bool>
        json_t& operator=(T b)
        {
            set_type(class_t::boolean);
            internal_.json_bool = b;
            return *this;
        }

        template<typename T>
        requires std::is_integral_v<T> && (!std::is_same_v<T, bool>)
        json_t& operator=(T i)
        {
            set_type(class_t::integral);
            internal_.json_int = i;
            return *this;
        }

        template<typename T>
        requires std::is_floating_point_v<T>
        json_t& operator=(T f)
        {
            set_type(class_t::floating);
            internal_.json_float = f;
            return *this;
        }

        template<typename T>
        requires std::is_convertible_v<T, std::string>
        json_t& operator=(T s)
        {
            set_type(class_t::string);
            *internal_.json_string = std::string(s);
            return *this;
        }

        json_t& operator[](const std::string& key)
        {
            set_type(class_t::object);
            return internal_.json_map->operator[](key);
        }

        json_t& operator[](unsigned index)
        {
            set_type(class_t::array);
            if (index >= internal_.json_list->size())
                internal_.json_list->resize(index + 1);
            return internal_.json_list->operator[](index);
        }

        json_t& at(const std::string& key)
        {
            return operator[](key);
        }

        [[nodiscard]] const json_t& at(const std::string& key) const
        {
            return internal_.json_map->at(key);
        }

        json_t& at(unsigned index)
        {
            return operator[](index);
        }

        [[nodiscard]] const json_t& at(unsigned index) const
        {
            return internal_.json_list->at(index);
        }

        [[nodiscard]] std::size_t length() const
        {
            return (type_ == class_t::array) ? internal_.json_list->size() : static_cast<std::size_t>(-1);
        }

        [[nodiscard]] bool has_key(const std::string& key) const
        {
            return (type_ == class_t::object) && (internal_.json_map->find(key) != internal_.json_map->end());
        }

        [[nodiscard]] std::size_t size() const
        {
            switch (type_)
            {
            case class_t::object:    return internal_.json_map->size();
            case class_t::array:     return internal_.json_list->size();
            default:                 return static_cast<std::size_t>(-1);
            }
        }

        [[nodiscard]] class_t json_type() const
        {
            return type_;
        }

        // Functions for getting primitives from the Json object.
        [[nodiscard]] bool is_null() const
        {
            return type_ == class_t::null;
        }

        [[nodiscard]] std::string to_string() const
        {
            bool b;
            return std::move(to_string(b));
        }

        std::string to_string(bool& ok) const
        {
            return (ok = type_ == class_t::string) ? std::move(json_escape(*internal_.json_string)) : std::string{};
        }

        [[nodiscard]] double to_float() const
        {
            bool b;
            return to_float(b);
        }

        double to_float(bool& ok) const
        {
            return (ok = type_ == class_t::floating) ? internal_.json_float : double{};
        }

        [[nodiscard]] std::int64_t to_int() const
        {
            bool b;
            return to_int(b);
        }

        std::int64_t to_int(bool& ok) const
        {
            return (ok = type_ == class_t::integral) ? internal_.json_int : std::int64_t{};
        }

        [[nodiscard]] bool to_bool() const
        {
            bool b;
            return to_bool(b);
        }

        bool to_bool(bool& ok) const
        {
            return (ok = type_ == class_t::boolean) && internal_.json_bool;
        }

        auto object_range()
        {
            return (type_ == class_t::object) ? json_wrapper_t<types::map_t>{internal_.json_map}
                                              : json_wrapper_t<types::map_t>(nullptr);
        }

        auto array_range()
        {
            return (type_ == class_t::array) ? json_wrapper_t<types::list_t>(internal_.json_list)
                                             : json_wrapper_t<types::list_t>(nullptr);
        }

        [[nodiscard]] auto object_range() const
        {
            return (type_ == class_t::object) ? json_const_wrapper_t<types::map_t>(internal_.json_map)
                                              : json_const_wrapper_t<types::map_t>(nullptr);
        }

        [[nodiscard]] auto array_range() const
        {
            return (type_ == class_t::array) ? json_const_wrapper_t<types::list_t>(internal_.json_list)
                                             : json_const_wrapper_t<types::list_t>(nullptr);
        }

        [[nodiscard]] std::string dump(int depth = 1, const std::string& tab = "  ") const
        {
            std::string pad;
            for (int i = 0; i < depth; ++i, pad += tab)
            {
            }

            switch (type_)
            {
            case class_t::null:
                return "null";
            case class_t::object:
            {
                std::string s{"{\n"};
                bool skip{true};
                for (auto& p : *internal_.json_map)
                {
                    if (!skip)
                        s += ",\n";
                    s += (pad + "\"" + p.first + "\" : " + p.second.dump(depth + 1, tab));
                    skip = false;
                }
                s += ("\n" + pad.erase(0, 2) + "}");
                return s;
            }
            case class_t::array:
            {
                std::string s{"["};
                bool skip{true};

                for (auto& p : *internal_.json_list)
                {
                    if (!skip)
                        s += ", ";
                    s += p.dump(depth + 1, tab);
                    skip = false;
                }
                s += "]";
                return s;
            }
            case class_t::string:      return "\"" + json_escape(*internal_.json_string) + "\"";
            case class_t::floating:    return std::to_string(internal_.json_float);
            case class_t::integral:    return std::to_string(internal_.json_int);
            case class_t::boolean:     return internal_.json_bool ? "true" : "false";
            default:                   return {};
            }
        }

        friend std::ostream& operator<<(std::ostream&, const json_t&);

    private:
        void set_type(class_t type)
        {
            if (type == type_)
                return;

            clear_internal();

            switch (type)
            {
            case class_t::null:        internal_.json_map    = nullptr;                break;
            case class_t::object:      internal_.json_map    = new types::map_t{};     break;
            case class_t::array:       internal_.json_list   = new types::list_t{};    break;
            case class_t::string:      internal_.json_string = new std::string{};      break;
            case class_t::floating:    internal_.json_float  = double{};               break;
            case class_t::integral:    internal_.json_int    = std::int64_t{};         break;
            case class_t::boolean:     internal_.json_bool   = false;                  break;
            }
            type_ = type;
        }

    private:
        /// @warning Only call if you know that Internal is allocated.
        /// No checks performed here.
        /// This function should be called in a constructed Json just before you are going to overwrite internal.
        void clear_internal()
        {
            switch (type_)
            {
            case class_t::object:    delete internal_.json_map;       break;
            case class_t::array:     delete internal_.json_list;      break;
            case class_t::string:    delete internal_.json_string;    break;
            default:                                                  break;
            }
        }

    private:
        class_t type_{class_t::null};
    };

    json_t array()
    {
        return std::move(json_t::make(json_t::class_t::array));
    }

    template<typename... T>
    json_t array(T... args)
    {
        json_t arr = json_t::make(json_t::class_t::array);
        arr.append(args...);
        return std::move(arr);
    }

    json_t object()
    {
        return std::move(json_t::make(json_t::class_t::object));
    }

    std::ostream& operator<<(std::ostream& os, const json_t& json)
    {
        os << json.dump();
        return os;
    }

    namespace
    {
        json_t parse_next(const std::string&, std::size_t&);

        void consume_ws(const std::string& str, std::size_t& offset)
        {
            while (isspace(str[offset]))
                ++offset;
        }

        json_t parse_object(const std::string& str, std::size_t& offset)
        {
            auto json_object = json_t::make(json_t::class_t::object);

            ++offset;
            consume_ws(str, offset);
            if (str[offset] == '}')
            {
                ++offset;
                return std::move(json_object);
            }

            while (true)
            {
                auto key = parse_next(str, offset);
                consume_ws(str, offset);

                if (str[offset] != ':')
                {
                    std::cerr << "Error: Object: Expected colon, found '" << str[offset] << "'\n";
                    break;
                }

                consume_ws(str, ++offset);
                auto value = parse_next(str, offset);
                json_object[key.to_string()] = value;
                consume_ws(str, offset);

                if (str[offset] == ',')
                {
                    ++offset;
                    continue;
                }
                else if (str[offset] == '}')
                {
                    ++offset;
                    break;
                }
                else
                {
                    std::cerr << "ERROR: Object: Expected comma, found '" << str[offset] << "'\n";
                    break;
                }
            }
            return std::move(json_object);
        }

        json_t parse_array(const std::string& str, std::size_t& offset)
        {
            auto json_array = json_t::make(json_t::class_t::array);
            unsigned index{};

            ++offset;
            consume_ws(str, offset);
            if (str[offset] == ']')
            {
                ++offset;
                return std::move(json_array);
            }

            while (true)
            {
                json_array[index++] = parse_next(str, offset);
                consume_ws(str, offset);

                if (str[offset] == ',')
                {
                    ++offset;
                    continue;
                }
                else if (str[offset] == ']')
                {
                    ++offset;
                    break;
                }
                else
                {
                    std::cerr << "ERROR: array: Expected ',' or ']', found '" << str[offset] << "'\n";
                    return std::move(json_t::make(json_t::class_t::array));
                }
            }
            return std::move(json_array);
        }

        json_t parse_string(const std::string& str, std::size_t& offset)
        {
            json_t json_string;
            std::string value;

            for (char c = str[++offset]; c != '\"'; c = str[++offset])
            {
                if (c == '\\')
                {
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
                        for (unsigned i = 1; i <= 4; ++i)
                        {
                            c = str[offset + i];
                            if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
                            {
                                value += c;
                            }
                            else
                            {
                                std::cerr << "ERROR: String: Expected hex character in unicode escape, found '" << c
                                          << "'\n";
                                return std::move(json_t::make(json_t::class_t::string));
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
                else
                {
                    value += c;
                }
            }
            ++offset;
            json_string = value;
            return std::move(json_string);
        }

        json_t parse_number(const std::string& str, std::size_t& offset)
        {
            json_t number;
            std::string val;
            std::string exp_str;
            char c;
            bool is_double{};
            std::int64_t exp{};

            while (true)
            {
                c = str[offset++];
                if ((c == '-') || (c >= '0' && c <= '9'))
                {
                    val += c;
                }
                else if (c == '.')
                {
                    val += c;
                    is_double = true;
                }
                else
                {
                    break;
                }
            }

            if (c == 'E' || c == 'e')
            {
                c = str[offset++];
                if (c == '-')
                {
                    ++offset;
                    exp_str += '-';
                }

                while (true)
                {
                    c = str[offset++];
                    if (c >= '0' && c <= '9')
                    {
                        exp_str += c;
                    }
                    else if (!isspace(c) && c != ',' && c != ']' && c != '}')
                    {
                        std::cerr << "ERROR: Number: Expected a number for exponent, found '" << c << "'\n";
                        return std::move(json_t::make(json_t::class_t::null));
                    }
                    else
                    {
                        break;
                    }
                }
                exp = std::stol(exp_str);
            }
            else if (!isspace(c) && c != ',' && c != ']' && c != '}')
            {
                std::cerr << "ERROR: Number: unexpected character '" << c << "'\n";
                return std::move(json_t::make(json_t::class_t::null));
            }
            --offset;

            if (is_double)
                number = std::stod(val) * std::pow(10, exp);
            else
                number = (!exp_str.empty()) ? std::stol(val) * std::pow(10, exp) : std::stol(val);
            return std::move(number);
        }

        json_t parse_bool(const std::string& str, std::size_t& offset)
        {
            json_t json_bool;

            if (str.substr(offset, 4) == "true")
            {
                json_bool = true;
            }
            else if (str.substr(offset, 5) == "false")
            {
                json_bool = false;
            }
            else
            {
                std::cerr << "ERROR: Bool: Expected 'true' or 'false', found '" << str.substr(offset, 5) << "'\n";
                return std::move(json_t::make(json_t::class_t::null));
            }

            offset += (json_bool.to_bool() ? 4 : 5);
            return std::move(json_bool);
        }

        json_t parse_null(const std::string& str, std::size_t& offset)
        {
            json_t json_null;
            if (str.substr(offset, 4) != "null")
            {
                std::cerr << "ERROR: Null: Expected 'null', found '" << str.substr(offset, 4) << "'\n";
                return std::move(json_t::make(json_t::class_t::null));
            }
            offset += 4;
            return std::move(json_null);
        }

        json_t parse_next(const std::string& str, std::size_t& offset)
        {
            char value;
            consume_ws(str, offset);
            value = str[offset];

            switch (value)
            {
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
    }

    json_t json_t::load(const std::string& str)
    {
        std::size_t offset{};
        return std::move(parse_next(str, offset));
    }
}

#endif // SINGLE_JSON_H
