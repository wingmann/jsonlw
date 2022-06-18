#ifndef WINGMANN_JSONLW_JSON_CONST_WRAPPER_H
#define WINGMANN_JSONLW_JSON_CONST_WRAPPER_H

#include <cstddef>

template<typename container_type>
class json_const_wrapper {
private:
    const container_type* object_;

public:
    explicit json_const_wrapper(const container_type* value) : object_{value}
    {
    }

    explicit json_const_wrapper(std::nullptr_t) : object_{nullptr}
    {
    }

    [[nodiscard]] typename container_type::const_iterator begin() const
    {
        return object_
            ? object_->begin()
            : typename container_type::const_iterator{};
    }

    [[nodiscard]] typename container_type::const_iterator end() const
    {
        return object_
            ? object_->end()
            : typename container_type::const_iterator{};
    }
};

#endif // WINGMANN_JSONLW_JSON_CONST_WRAPPER_H
