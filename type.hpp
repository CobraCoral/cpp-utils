#ifndef TYPE_HPP
#define TYPE_HPP

#include <string>
#include <typeinfo>

#ifdef __GNUG__
#include <memory>
#include <cxxabi.h>

// from https://stackoverflow.com/questions/27440953/stdunique-ptr-for-c-functions-that-need-free
struct free_deleter {
    template <typename T>
    void operator()(T *p) const {
        std::free(const_cast<std::remove_const_t<T>*>(p));
    }
};

template <typename T>
using unique_C_ptr = std::unique_ptr<T, free_deleter>;
static_assert(sizeof(char *)== sizeof(unique_C_ptr<char>),""); // ensure no overhead

inline std::string demangle(const char* name)
{
    int status{};
    unique_C_ptr<char const> realname { abi::__cxa_demangle(name, 0, 0, &status) };
    return (status == 0 ? realname.get() : name);
}
#else
inline std::string demangle(const char* name)
{
  return name;
}
#endif //__GNUG__

template <class T>
std::string type(const T& t) {
    return demangle(typeid(t).name());
}

#endif // TYPE_HPP
