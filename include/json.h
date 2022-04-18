#ifndef WINGMANN_JSONLW_JSON_H
#define WINGMANN_JSONLW_JSON_H

#include "json_wrapper.h"
#include "json_const_wrapper.h"

#include <string>
#include <deque>
#include <map>
#include <initializer_list>
#include <iostream>

namespace wingmann {

class json {
public:
    using list_type   = std::deque<json>;
    using map_type    = std::map<std::string, json>;
    using string_type = std::string;
    using float_type  = double;
    using int_type    = std::int64_t;
    using bool_type   = bool;
    using size_type   = std::size_t;

    using json_list_wraper_type = json_wrapper<json::list_type>;
    using json_const_list_wraper_type = json_const_wrapper<json::list_type>;
    using json_map_wraper_type = json_wrapper<json::map_type>;
    using json_const_map_wraper_type = json_const_wrapper<json::map_type>;

private:
    union backing_data {
        list_type*   json_list;
        map_type*    json_map;
        string_type* json_string;
        float_type   json_float;
        int_type     json_int;
        bool_type    json_bool;

        backing_data() = default;
        explicit backing_data(float_type value);
        explicit backing_data(int_type value);
        explicit backing_data(bool_type value);
        explicit backing_data(string_type value);
    } internal_{};

public:
    enum class class_type {
        null,
        object,
        array,
        string,
        floating,
        integral,
        boolean
    };

private:
    class_type type_{class_type::null};

public:
    json() = default;
    json(std::initializer_list<json> list);
    json(json&& other) noexcept;
    json(const json& other);
    explicit json(std::nullptr_t);

    template<typename T>
    explicit json(T value, typename std::enable_if<std::is_same<T, bool>::value>::type* = nullptr)
        : internal_{value}, type_{class_type::boolean}
    {
    }

    template<typename T>
    explicit json(
        T value,
        typename std::enable_if<std::is_integral<T>::value &&
            !std::is_same<T, bool>::value>::type* = nullptr)
        : internal_{static_cast<std::int64_t>(value)}, type_{class_type::integral}
    {
    }

    template<typename T>
    explicit json(
        T value,
        typename std::enable_if<std::is_floating_point<T>::value>::type* = nullptr)
        : internal_{static_cast<double>(value)}, type_{class_type::floating}
    {
    }

    template<typename T>
    explicit json(
        T value,
        typename std::enable_if<std::is_convertible<T, std::string>::value>::type* = nullptr)
        : internal_{std::string{value}}, type_{class_type::string}
    {
    }

    virtual ~json();

    // Operators.
    json& operator=(json&& other) noexcept;
    json& operator=(const json& other);

    template<typename T>
    typename std::enable_if<std::is_same<T, bool>::value, json&>::type
    operator=(T b)
    {
        set_type(class_type::boolean);
        internal_.json_bool = b;
        return *this;
    }

    template<typename T>
    typename std::enable_if<std::is_integral<T>::value &&
                            !std::is_same<T, bool>::value, json&>::type
    operator=(T i)
    {
        set_type(class_type::integral);
        internal_.json_int = i;
        return *this;
    }

    template<typename T>
    typename std::enable_if<std::is_floating_point<T>::value, json&>::type
    operator=(T f)
    {
        set_type(class_type::floating);
        internal_.json_float = f;
        return *this;
    }

    template<typename T>
    typename std::enable_if<std::is_convertible<T, std::string>::value, json&>::type
    operator=(T s)
    {
        set_type(class_type::string);
        *internal_.json_string = std::string{s};
        return *this;
    }

    json& operator[](const std::string& key);
    json& operator[](unsigned index);

    // Methods.
    static json make(class_type type);
    json load(const std::string& value);

    template<typename T>
    void append(T arg)
    {
        set_type(class_type::array);
        internal_.json_list->emplace_back(arg);
    }

    template<typename T, typename... U>
    void append(T arg, U... args)
    {
        append(arg);
        append(args...);
    }

    json& at(const std::string& key);
    [[nodiscard]] const json& at(const std::string& key) const;
    json& at(unsigned index);
    [[nodiscard]] const json& at(unsigned index) const;

    [[nodiscard]] size_type length() const;
    [[nodiscard]] bool has_key(const std::string& key) const;
    [[nodiscard]] size_type size() const;
    [[nodiscard]] class_type json_type() const;

    // Functions for getting primitives from the Json object.
    [[nodiscard]] bool is_null() const;

    [[nodiscard]] std::string to_string() const;
    std::string to_string(bool& ok) const;

    [[nodiscard]] double to_float() const;
    double to_float(bool& ok) const;

    [[nodiscard]] std::int64_t to_int() const;
    std::int64_t to_int(bool& ok) const;

    [[nodiscard]] bool to_bool() const;
    bool to_bool(bool& ok) const;

    auto array_range();
    [[nodiscard]] auto array_range() const;

    auto object_range();
    [[nodiscard]] auto object_range() const;

    [[nodiscard]] std::string dump(int depth = 1, const std::string& tab = "    ") const;

    friend std::ostream& operator<<(std::ostream& os, const json& value)
    {
        os << value.dump();
        return os;
    }

    template<typename... T>
    json array(T... args)
    {
        json arr = json::make(json::class_type::array);
        arr.append(args...);
        return std::move(arr);
    }

    static json array();
    static json object();

    static std::string json_escape(const std::string& value);

private:
    void set_type(class_type type);

    /**
     * @warning Only call if you know that Internal is allocated.
     * No checks performed here.
     * This function should be called in a constructed json just before you are going to
     * overwrite internal.
     */
    void clear_internal();

public:
    void consume_ws(const std::string& str, std::size_t& offset);

    json parse_next(const std::string& str, std::size_t& offset);
    json parse_object(const std::string& str, std::size_t& offset);
    json parse_array(const std::string& str, std::size_t& offset);
    json parse_string(const std::string& str, std::size_t& offset);
    static json parse_number(const std::string& str, std::size_t& offset);
    static json parse_bool(const std::string& str, std::size_t& offset);
    static json parse_null(const std::string& str, std::size_t& offset);
};

} // namespace wingmann

#endif // WINGMANN_JSONLW_JSON_H
