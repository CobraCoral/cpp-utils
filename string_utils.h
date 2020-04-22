#pragma once
#include <string>
#include <climits> // CHAR_BIT
#include "template_utils.h"

namespace string_utils {
inline namespace v2 {

std::string trim_zeroes(std::string str, int target_length=0) 
{ 
    int i = 0; 
    while (str[i] == '0')
    {
        // if a target_length was given, only remove enough zeroes to keep
        // the remaining string left with target_length
        if (target_length && (str.length()-i) <= target_length) break;
        ++i; 
    }
    return str.erase(0, i);
}

std::string pad_string(const std::string& input, int target_length=10, char padding_character='0', bool left_padding=true)
{
    std::string retval = input;
    int insert_length{0};

    if (input.length() > target_length)
    {
        insert_length = target_length;
    }
    else
    {
        insert_length = target_length - input.length();
    }

    if (left_padding)
    {
        retval.insert(0, insert_length, padding_character);
    }
    else
    {
        retval.insert(retval.end(), insert_length, padding_character);
    }
    return trim_zeroes(retval, target_length);
}

template<typename T>
std::string get_binary_representation(const T& value, int pad_length=8, bool bitcap=false, std::size_t bits=0)
{
    std::string count;

    std::size_t bytes = sizeof(T);

    //auto loop_size = (bitcap?bits:bytes * CHAR_BIT);
    constexpr std::size_t BITS_IN_BYTE = 1<<3; // same as *8

    // Why is the below important? Not sure if we care, but maybe in some
    // weird architecture elsewhere that could be true? If so, <<3 might not be
    // the right multiplier for us to use...
    static_assert(CHAR_BIT==BITS_IN_BYTE, "CHAR_BIT does not equal 8 bits");

    // if we were given some bits to cap the printing at:
    // We need to make sure that someone didn't try to trick us and gave more
    // bits than what we have in a byte (assuming 8 bits in this case, see
    // static_assert above...).
    // Otherwise, we will print all K bytes we have times N bits
    auto loop_size = (bitcap?extrema::min(bits, bytes<<3):bytes<<3);
    char* ptr((char*)(&value));
    int byte{-1};
    for(size_t bit=0; bit<loop_size; ++bit)
    {
        if (bit%8 == 0) ++byte;
        bool is_1 = (((ptr[byte])>>bit)&1);
        count.insert(0, is_1 ? "1" : "0");
    }
    return pad_string(count, pad_length, '0');
}
//template<typename T>
//std::string get_binary_representation(const T& value, int pad_length=8)
//{
//    std::string count;
//
//    size_t bytes = sizeof(T);
//    char* ptr((char*)(&value));
//    for(size_t byte=0; byte<bytes; ++byte)
//    {
//        for(size_t bit=0; bit<CHAR_BIT; ++bit)
//        {
//            bool is_1 = (((ptr[byte])>>bit)&1);
//            count.insert(0, is_1 ? "1" : "0");
//        }
//    }
//    return pad_string(count, pad_length, '0');
//}
} // namespace v2

namespace v1 {
std::string pad_string(const std::string& input, int target_length=10, char padding_character='0', bool left_padding=true)
{
    constexpr auto CAP{100UL};
    const std::string padding(CAP, padding_character);
    char *buffer = new char[std::max(CAP, input.length())+10]; // capping at 100 characters
    const int pad_length = std::max(0L, (long)(target_length - input.length())); // to avoid negatives
    if (left_padding)
    {
        sprintf(buffer, "[%*.*s%s]", pad_length, pad_length, padding.c_str(), input.c_str());
    }
    else
    {
        sprintf(buffer, "[%s%*.*s]", input.c_str(), pad_length, pad_length, padding.c_str());
    }
    // TODO is there a way to avoid this copy? It would be great to be able to use RVO somehow...
    // NOTE: It seems the compiler does NRVO for us automatically here! Neat!
    std::string retval(buffer);
    delete [] buffer;
    return retval;
}
} // namespace v1

} // namespace string_utils
