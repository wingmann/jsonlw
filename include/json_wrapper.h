#ifndef WINGMANN_JSONLW_JSON_WRAPPER_H
#define WINGMANN_JSONLW_JSON_WRAPPER_H

#include <cstddef>

template<typename container_type>
class json_wrapper {
private:
    container_type* object_;

public:
    explicit json_wrapper(container_type* value) : object_{value}
    {
    }

    explicit json_wrapper(std::nullptr_t) : object_{nullptr}
    {
    }

    typename container_type::iterator begin()
    {
        return object_
            ? object_->begin()
            : typename container_type::iterator{};
    }

    typename container_type::iterator end()
    {
        return object_
            ? object_->end()
            : typename container_type::iterator{};
    }

    [[nodiscard]] typename container_type::const_iterator begin() const
    {
        return object_
            ? object_->begin()
            : typename container_type::iterator{};
    }

    [[nodiscard]] typename container_type::const_iterator end() const
    {
        return object_
            ? object_->end()
            : typename container_type::iterator{};
    }
};

#endif // WINGMANN_JSONLW_JSON_WRAPPER_H
