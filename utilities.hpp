//
// Created by mads on 14-01-25.
//

#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <utility>

namespace utilities {
template<typename T, typename TDestroyer>
class raii_wrapper {
    T m_value;
    TDestroyer m_destroyer;

    bool m_moved;

public:
    raii_wrapper() = delete;

    template<typename U = T>
    raii_wrapper(T value, TDestroyer&& destroyer, std::enable_if_t<std::is_trivial_v<U> >* = nullptr)
        : m_value(value), m_destroyer(std::forward<TDestroyer>(destroyer)), m_moved(false) {
    }

    template<typename U = T>
    raii_wrapper(T&& value, TDestroyer&& destroyer, std::enable_if_t<!std::is_trivial_v<U>>* = nullptr)
        : m_value(std::forward<T>(value)), m_destroyer(std::forward<TDestroyer>(destroyer)), m_moved(false) {
    }


    raii_wrapper(raii_wrapper&& other) noexcept
        : m_value(std::move(other.m_value)), m_destroyer(other.m_destroyer), m_moved(false) {
        other.m_moved = true;
    }

    raii_wrapper(const raii_wrapper&) = delete;

    ~raii_wrapper() {
        if (!m_moved) {
            std::cerr << "Destroyed raii_wrapper." << std::endl;
            m_destroyer(m_value);
        }
    }

    [[nodiscard]] constexpr const T& value() const noexcept {
        return m_value;
    }

    [[nodiscard]] constexpr T& value() noexcept {
        return m_value;
    }
};
}


#endif //UTILITIES_HPP
