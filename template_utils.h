#pragma once

#include <cstdint>
#include <cstddef>

namespace {
    const char rcsid_template_utils_h[] = "$Id: template_utils.h 426 2011-06-08 21:27:14Z a803980 $";
}

// type forwaring 
template <typename T> T& ref_type_forwarding(T);

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

// base 2 logarithmic
namespace base2log {
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

// Counts how many bits are set in a number.
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


// system native type
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

// validates if a structure is size aligned
template <typename T, std::size_t N>
struct is_structure_size_aligned
{
    enum { VALUE = (sizeof(T) == (sizeof(T)/N)*N) ? true : false };
};

// Size of Array
template <typename T, std::size_t N>
std::size_t size_of_array(const T (&array)[N])
{
    return N;
}

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
