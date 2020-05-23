#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits> // std::enable_if

// TODO add string versioning and other stuff to compiled binary (e.g. getinfo)
namespace {
    const char rcsid_template_utils_h[] = "$Id: template_utils.h 426 2011-06-08 21:27:14Z a803980 $";
}

// type forwaring 
template <typename T> T& ref_type_forwarding(T);

//{{{ Min/Max
namespace extrema {
    // helper templates
    template<typename T>
    constexpr T const& max(T const& a, T const& b)
    {
        return a > b ? a : b;
    }
    
    template<typename T>
    constexpr T const& min(T const& a, T const& b)
    {
        return a < b ? a : b;
    }
} // extrema
//}}}

//{{{ power
namespace power {
    // NOTE: if you want this to resolve in compile time, you need to:
    // constexpr uint64_t value = base2log::value(128);
    template <typename T>
    constexpr uint64_t value(T num, uint32_t pow)
    {
        if (num < 0) return value(-num, pow);
        if (pow == 0) return 1;
        if (pow == 1) return num;
        return num * value(num, pow-1);
    }
}
//}}}

//{{{ base 2 logarithmic
namespace base2log {
    // NOTE: if you want this to resolve in compile time, you need to:
    // constexpr uint64_t value = base2log::value(128);
    constexpr uint64_t value(uint64_t i)
    {
        if (i <= 1) return 0;
        uint64_t ret_val = 0;
        while (i>1)
        {
            ret_val += 1;
            i /= 2;
        }
        return ret_val;
    }

    template<uint64_t VAL>
    struct input
    {
        static constexpr uint64_t value = 1 + input<VAL/2>::value;
    };
    
    template<>
    struct input<1>
    {
        static constexpr uint64_t value = 0;
    };
    
    template<>
    struct input<0>
    {
        static constexpr uint64_t value = 0;
    };
}
//}}}

//{{{ BitSet Counting - Counts how many bits are set in a number.
//
// This is also known as hamming weight, population count, popcount,
// sideways sum, or bit summation.
//
// Of interest, some examples of applications for this:
// - In modular exponentiation by squaring, the number of modular multiplications
// required for an exponent e is log2 e + weight(e). This is the reason that
// the public key value e used in RSA is typically chosen to be a number of low
// Hamming weight.
// 
// - The Hamming weight determines path lengths between nodes in Chord 
// distributed hash tables.
// 
// - IrisCode lookups in biometric databases are typically implemented by
// calculating the Hamming distance to each stored record.
// 
// - In computer chess programs using a bitboard representation,
// the Hamming weight of a bitboard gives the number of pieces of a given type
// remaining in the game, or the number of squares of the board controlled by
// one player's pieces, and is therefore an important contributing term to the
// value of a position.
// 
// - Hamming weight can be used to efficiently compute find first set using the
// identity ffs(x) = pop(x ^ ~(-x)). This is useful on platforms such as SPARC
// that have hardware Hamming weight instructions but no hardware find first
// set instruction.
// 
// - The Hamming weight operation can be interpreted as a conversion from the
// unary numeral system to binary numbers.
// 
// - In implementation of some succinct data structures like bit vectors and
// wavelet trees.
namespace count_bits {
    // for runtime, can use:
    // 1) std::bitset has a count() method that counts the number of bits set
    // 2) __builtin_popcount in GCC (but look at #3 below)
    // 3) Intel CPUs have a popcnt flag available!
    //    1) Either code it urself in ASM or...
    //    2) Make sure to use GCC: #pragma GCC target ("sse4.2") so that GCC
    //       makes good use of it (instead of doing it in software) as it will
    //       be at least 4x faster!
    template<uint64_t VAL>
    struct input
    {
        static constexpr uint64_t value = (1&VAL) + input<(VAL>>1)>::value;
    };
    
    template<>
    struct input<1>
    {
        static constexpr uint64_t value = 1;
    };
    
    template<>
    struct input<0>
    {
        static constexpr uint64_t value = 0;
    };
}
//}}}

//{{{ system native type
namespace system_native_type
{
    template <size_t PointerSize> struct unsigned_system_native_type;
    template <> 
    struct unsigned_system_native_type<4> 
    {
        typedef uint32_t type;
    };

    template <> 
    struct unsigned_system_native_type<8> 
    {
        typedef uint64_t type;
    };

    template <size_t PointerSize> struct signed_system_native_type;
    template <> 
    struct signed_system_native_type<4> 
    {
        typedef int32_t type;
    };

    template <> 
    struct signed_system_native_type<8> 
    {
        typedef int64_t type;
    };

    struct get_unsigned
    {
        typedef typename unsigned_system_native_type<sizeof(std::uintptr_t)>::type type;
    };
    
    struct get_signed
    {
        typedef typename signed_system_native_type<sizeof(std::intptr_t)>::type type;
    };
}
//}}}

// validates if a structure is size aligned
template <typename T, std::size_t N>
struct is_structure_size_aligned
{
    enum { VALUE = (sizeof(T) == (sizeof(T)/N)*N) ? true : false };
};

//{{{ Array tools
// Size of Array
template <typename T, std::size_t N>
constexpr std::size_t array_size(const T (&)[N])
{
    return N;
}

template <typename T>
struct not_an_array
{
    static constexpr bool error = false;
};

template <typename T>
constexpr std::size_t array_size(T)
{ // we could just =delete this (see https://foonathan.net/2015/11/overload-resolution-2/), but this is better
    static_assert(not_an_array<T>::error, "array-to-pointer decay has occured, cannot give you the size");
    return 0; // to silence warnings
}
//}}}

//{{{ RAII-Guard-Pthread Lock
/*
#include <pthread.h>
// pthread lock guard - RAII
template <typename Mutex>
struct pthread_lock_guard
{

    pthread_lock_guard(Mutex &mutex) : mutex_(mutex) 
    {
        ::pthread_mutex_lock(&mutex_);
    }
    ~pthread_lock_guard()
    {
        ::pthread_mutex_unlock(&mutex_);
    }

    private:
    // not copiable
    pthread_lock_guard& operator=(const pthread_lock_guard &) = delete;
    pthread_lock_guard(const pthread_lock_guard &) = delete;
    pthread_lock_guard() = delete;
    Mutex & mutex_;
};
*/
//}}}

//{{{ For SFINAE magic : see https://foonathan.net/2015/11/overload-resolution-4/
template <typename...>
struct voider
{
	using type = void;
};

/**
    Example:
    template <typename Cont, typename Key>
    auto erase(Cont &c, const Key &value) -> void_t<decltype(c.erase(value))>
*/
template <typename ... Ts>
using void_t = typename voider<Ts...>::type;
// NOTE / TODO -> on c++17, we have std::void_t! use that instead!
//}}}

//{{{ Better static_assert messages, from: https://stackoverflow.com/questions/13837668/display-integer-at-compile-time-in-static-assert/45127063
template<uint64_t N>
struct TriggerOverflowWarning
{
    static constexpr uint64_t value() { return N + 256; }
};

template <uint64_t N, uint64_t M, typename Enable = void>
struct CheckEqualityWithWarning
{
    static constexpr bool value = true;
};

template <uint64_t N, uint64_t M>
struct CheckEqualityWithWarning<N, M, typename std::enable_if<N != M>::type>
{
    static constexpr bool value = (TriggerOverflowWarning<N>::value() == TriggerOverflowWarning<M>::value());
};
//}}}
