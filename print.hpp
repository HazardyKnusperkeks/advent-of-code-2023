#ifndef PRINT_HPP
#define PRINT_HPP

#include <format>
#include <iterator>

extern std::ostream_iterator<char> outIterator;
extern std::ostream_iterator<char> errIterator;

template<typename... Args>
void myPrint(std::format_string<Args...> str, Args&&... args) {
    std::format_to(outIterator, str, std::forward<Args>(args)...);
}

template<typename... Args>
void myErr(std::format_string<Args...> str, Args&&... args) {
    std::format_to(errIterator, str, std::forward<Args>(args)...);
}

#endif //PRINT_HPP