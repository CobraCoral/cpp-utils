#ifndef ENUMTOSTRINGS_H__
    #define ENUMTOSTRINGS_H__
#endif // ENUMTOSTRINGS_H__

#undef BEGIN_ENUM
#undef BEGIN_ENUM_TYPE
#undef BEGIN_ENUM_CLASS
#undef BEGIN_ENUM_CLASS_TYPE
#undef DECL_ENUM_ELEMENT
#undef DECL_ENUM_ELEMENT_VAL
#undef DECL_ENUM_CLASS_ELEMENT
#undef DECL_ENUM_CLASS_ELEMENT_VAL
#undef END_ENUM

#include <string>

#if !(defined(GENERATE_ENUM_STR_FROM_ID) || defined(GENERATE_ENUM_ID_FROM_STR)) 
    #define BEGIN_ENUM(ENUM_NAME) typedef enum tag##ENUM_NAME {
    #define BEGIN_ENUM_TYPE(ENUM_NAME, TYPE) typedef enum tag##ENUM_NAME : TYPE {
    #define BEGIN_ENUM_CLASS(ENUM_NAME) typedef enum struct tag##ENUM_NAME {
    #define BEGIN_ENUM_CLASS_TYPE(ENUM_NAME, TYPE) typedef enum struct tag##ENUM_NAME : TYPE {
    #define DECL_ENUM_ELEMENT(element) element,
    #define DECL_ENUM_ELEMENT_VAL(element, val) element=val,
    #define DECL_ENUM_CLASS_ELEMENT(ENUM_NAME, element) DECL_ENUM_ELEMENT(element)
    #define DECL_ENUM_CLASS_ELEMENT_VAL(ENUM_NAME, element, val) DECL_ENUM_ELEMENT_VAL(element, val)
    #define END_ENUM(ENUM_NAME) } ENUM_NAME; \
        const std::string& GetString##ENUM_NAME(tag##ENUM_NAME index); \
        tag##ENUM_NAME GetEnum##ENUM_NAME(const char* str);
    #include <unordered_map>
#else
    #if (defined(GENERATE_ENUM_STR_FROM_ID))
        #define BEGIN_ENUM(ENUM_NAME) std::unordered_map<tag##ENUM_NAME, std::string> strFrom##ENUM_NAME = {
        #define BEGIN_ENUM_TYPE(ENUM_NAME, TYPE) BEGIN_ENUM(ENUM_NAME)
        #define BEGIN_ENUM_CLASS(ENUM_NAME) BEGIN_ENUM(ENUM_NAME)
        #define BEGIN_ENUM_CLASS_TYPE(ENUM_NAME, TYPE) BEGIN_ENUM(ENUM_NAME)
        #define DECL_ENUM_ELEMENT(element) {element, #element},
        #define DECL_ENUM_ELEMENT_VAL(element, val) {element, #element},
        #define DECL_ENUM_CLASS_ELEMENT(ENUM_NAME, element) {tag##ENUM_NAME::element, #element},
        #define DECL_ENUM_CLASS_ELEMENT_VAL(ENUM_NAME, element, val) {tag##ENUM_NAME::element, #element},
        #define END_ENUM(ENUM_NAME) }; const std::string& GetString##ENUM_NAME(tag##ENUM_NAME index){ return strFrom##ENUM_NAME[index]; }
    #else // (defined(GENERATE_ENUM_ID_FROM_STR))
        #define BEGIN_ENUM(ENUM_NAME) std::unordered_map<std::string, tag##ENUM_NAME> strTo##ENUM_NAME = {
        #define BEGIN_ENUM_TYPE(ENUM_NAME, TYPE) BEGIN_ENUM(ENUM_NAME)
        #define BEGIN_ENUM_CLASS(ENUM_NAME) BEGIN_ENUM(ENUM_NAME)
        #define BEGIN_ENUM_CLASS_TYPE(ENUM_NAME, TYPE) BEGIN_ENUM(ENUM_NAME)
        #define DECL_ENUM_ELEMENT(element) {#element,element},
        #define DECL_ENUM_ELEMENT_VAL(element, val) {#element,element},
        #define DECL_ENUM_CLASS_ELEMENT(ENUM_NAME, element) {#element, tag##ENUM_NAME::element},
        #define DECL_ENUM_CLASS_ELEMENT_VAL(ENUM_NAME, element, val) {#element, tag##ENUM_NAME::element},
        #define END_ENUM(ENUM_NAME) }; tag##ENUM_NAME GetEnum##ENUM_NAME(const char* str){ return strTo##ENUM_NAME[str]; }
    #endif // GENERATE_ENUM_STR_FROM_ID || GENERATE_ENUM_ID_FROM_STR
#endif // !(GENERATE_ENUM_STR_FROM_ID && GENERATE_ENUM_ID_FROM_STR)
