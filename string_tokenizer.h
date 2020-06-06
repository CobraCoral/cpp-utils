#ifndef STRINGTOKENIZER_H__
#define STRINGTOKENIZER_H__

#include <string>
#include <vector>
#include <cstring>

namespace string_utils {

    /** This class breaks a line into tokens separated by a given character
     *  and allows to traverse on the resulting tokens or access by index.
     *  Samples (the parenthesis are not included -- separator=';') :
     *  +-------------------------------------------+
     *  | line              | qty | tokens          |
     *  +-------------------------------------------+
     *  | a;b;c;d           |  4  | (a)(b)(c)(d)    |
     *  | <empty line>      |  1  | ()              |
     *  | a;b;;d            |  4  | (a)(b)()(d)     |
     *  | ;a;b;c            |  4  | ()(a)(b)(c)     |
     *  | a;b;c;            |  4  | (a)(b)(c)()     |
     *  | ;                 |  2  | ()()            |
     *  | a;; ;b            |  4  | (a)()( )(b)     |
     *  +-------------------------------------------+
     * Example Usage
     * const char* data("this,is,a,,test,");
     * StringTokenizer<1024> tok;
     * std::size_t tokens ( tok.tokenize(data, ',') );
     * std::cout << "Data[" << data << "] has [" << tokens << "] tokens" << std::endl;
     * for ( std::size_t i=0; i<tokens; ++i )
     * {
     *     std::cout << "[" << i << "] -> [" << tok[i] << "]" << std::endl;
     * }
     */
    template <std::size_t SIZE=1024>
    class StringTokenizer
    {
    public:
        /** 
         * Breaks a line into tokens separated by 'separator'.
         * Returns the quantity of tokens found. This qty
         * is always greater than 0 (see samples above).
         */
        inline size_t tokenize(const char* data, char separator);

        // Access a token by the position
        inline const char* operator[](size_t index);

        /** 
         * Returns the quantity of tokens. This qty
         * is always greater than 0 (see samples above).
         */
        inline size_t size() const { return tokens_.size(); }
        
    private:
        // token position
        std::vector<std::size_t> tokens_;
        char buf_[SIZE];
        std::size_t len_;
    };

    // tokenize
    template<std::size_t SIZE>
    inline size_t StringTokenizer<SIZE>::tokenize(const char* data, char separator)
    {
        len_ = std::min(SIZE, strlen(data)+1);
        if ( len_ == 0 ) return 0;

        memcpy(buf_, data, len_);
        buf_[SIZE-1] = 0;

        tokens_.clear();

        char* cursor = const_cast<char*>(buf_); // traverse on data
        char* mark = const_cast<char*>(buf_); // points to a token begin
        char* dataEnd = &(buf_[len_-1]); // points to data end

        while ( cursor != dataEnd )
        {
            if ( cursor[0] == separator ) // token found
            {
                tokens_.push_back(mark-buf_);
                *cursor = 0;
                mark = cursor + 1; //-- Points to the beginning of next token
            }
            ++cursor; 
        }

        if ( mark != dataEnd )
        {
            tokens_.push_back(mark-buf_); // The last token
        }
        else
        {
            // Last char is a separator: add an empty string token
            if ( data[len_ - 1] == 0 )
            {
                tokens_.push_back(len_-1);
            }
        }

        return tokens_.size(); // Number of tokens found
    }

    // operator[]
    template<std::size_t SIZE>
    const char* StringTokenizer<SIZE>::operator[](size_t index)
    {
        if ( index >= tokens_.size() ) return 0;
        return &buf_[tokens_[index]];
    }

} // namespace string_utils

#endif // STRINGTOKENIZER_H__
