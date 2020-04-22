# Collection of header files that provide functions, and other utilities that are used throughout other projects...

## Benchmarking
- You just need to call 'benchmark::benchmark("<label>", function, ...);' where '...' are any parameters you may want to pass to the function to be tested (or none). Example:
```
void noop() {}
int main()
{
    benchmark::benchmark("noop", noop);
}
```
- You can also call measure time directly. But you need to keep track of your best measurement, and deduct the cost of RDTSC from it as below:
```
uint64_t best{~0UL};                                                                
measure_time(best, benchmark::rdtsc(), noop());                                             
printf("noop: %8li ticks; (%.02f) ns\n",  best-benchmark::RDTSC_COST, benchmark::get_nanos_from_ticks(best-benchmark::RDTSC_COST));
```
- Resources for benchmarking: Check http://www.open-std.org/jtc1/sc22/wg21/docs/TR18015.pdf

## EnumToString
- EnumToString uses three macros to properly generate the enumeration code:
 - GENERATE_ENUM_STR_FROM_ID -> Allows to retrieve the enumeration 'string' representation from the enumeration ID.
 - GENERATE_ENUM_ID_FROM_STR -> Allows to retrieve the enumeration element from the 'string' representation.
- Example:
  - 1) Create a header .h file to hold your enumeration declaration.
      NOTE: It is important that your .h file follow the template below, where you would replace <SOME_UNIQUE_NAME_H__> with your file name, and add all the necessary enum names and declarations you want:
```
////////////////////////////////////////////////
// HEADER
////////////////////////////////////////////////
#if (!defined(<SOME_UNIQUE_NAME_H__>) || defined(GENERATE_ENUM_STR_FROM_ID) || defined(GENERATE_ENUM_ID_FROM_STR))
#include "EnumToString.h"

////////////////////////////////////////////////
// BODY
////////////////////////////////////////////////
BEGIN_ENUM('NAME')
    DECL_ENUM_ELEMENT('ELEMENT NAME')
    ...
END_ENUM('NAME')

////////////////////////////////////////////////
// TAIL
////////////////////////////////////////////////
  #endif // ( !defined(<SOME_UNIQUE_NAME_H__>) || defined(GENERATE_ENUM_STR_FROM_ID) || defined(GENERATE_ENUM_ID_FROM_STR))
  // GENERATE_ENUM_MAPS must be defined in a single compilation unit (.cpp) before including this file
  #if defined(GENERATE_ENUM_MAPS) && !(defined(GENERATE_ENUM_STR_FROM_ID) || defined(GENERATE_ENUM_ID_FROM_STR))
      #define GENERATE_ENUM_STR_FROM_ID  // Start (Enum -> String) generation
          #include __FILE__ // Generate Map and Functions
      #undef GENERATE_ENUM_STR_FROM_ID // Stop (Enum -> String) generation

      #define GENERATE_ENUM_ID_FROM_STR  // Start (String -> Enum) generation
          #include __FILE__ // Generate Map and Functions
      #undef GENERATE_ENUM_ID_FROM_STR // Stop (String -> Enum) generation
  #endif // !(defined(GENERATE_ENUM_STR_FROM_ID) || defined(GENERATE_ENUM_ID_FROM_STR))
  #define <SOME_UNIQUE_NAME_H__>
```
  - 2) Define a .cpp file, where you just do the following:
```
#define GENERATE_ENUM_MAPS
#include "<Your Filename.h>"
```

    on .h file:
```
////////////////////////////////////////////////
// HEADER
////////////////////////////////////////////////
#if (!defined(LOGGERTYPES_H__) || defined(GENERATE_ENUM_STR_FROM_ID) || defined(GENERATE_ENUM_ID_FROM_STR))
#include "EnumToString.h"

////////////////////////////////////////////////
// BODY
////////////////////////////////////////////////
BEGIN_ENUM(Severity)
    DECL_ENUM_ELEMENT(TRACE)
    DECL_ENUM_ELEMENT(DEBUG)
    DECL_ENUM_ELEMENT(INFO)
    DECL_ENUM_ELEMENT(WARN)
    DECL_ENUM_ELEMENT(ERROR)
    DECL_ENUM_ELEMENT(FATAL)
    DECL_ENUM_ELEMENT(UNDEF_SEVERITY)
    DECL_ENUM_ELEMENT(TOTAL)
END_ENUM(Severity)

BEGIN_ENUM(Verbosity)
    DECL_ENUM_ELEMENT_VAL(LOW,0)
    DECL_ENUM_ELEMENT_VAL(MEDIUM,5)
    DECL_ENUM_ELEMENT_VAL(HIGH,50)
    DECL_ENUM_ELEMENT_VAL(INSANE,100)
    DECL_ENUM_ELEMENT_VAL(UNDEF_VERBOSITY,1000)
END_ENUM(Verbosity)

BEGIN_ENUM(OutputFlags)
    DECL_ENUM_ELEMENT_VAL(UNDEF_OUTPUT,0x0)
    DECL_ENUM_ELEMENT_VAL(STDOUT,0x1)
    DECL_ENUM_ELEMENT_VAL(STDERR,0x2)
    DECL_ENUM_ELEMENT_VAL(FILEOUT,0x4)
    DECL_ENUM_ELEMENT_VAL(EMAIL,0x8)
END_ENUM(OutputFlags)

BEGIN_ENUM_TYPE(TypeTest, char)
    DECL_ENUM_ELEMENT_VAL(A,'a')
    DECL_ENUM_ELEMENT_VAL(B,'b')
END_ENUM(TypeTest)

BEGIN_ENUM_CLASS(ClassTest)
    DECL_ENUM_CLASS_ELEMENT_VAL(ClassTest, X, 0x30)
    DECL_ENUM_CLASS_ELEMENT_VAL(ClassTest, Y, 'b')
    DECL_ENUM_CLASS_ELEMENT(ClassTest, Z)
END_ENUM(ClassTest)

BEGIN_ENUM_CLASS_TYPE(ClassTestType, char)
    DECL_ENUM_CLASS_ELEMENT_VAL(ClassTestType, C, 0x30)
    DECL_ENUM_CLASS_ELEMENT_VAL(ClassTestType, D, 'b')
    DECL_ENUM_CLASS_ELEMENT(ClassTestType, E)
END_ENUM(ClassTestType)
#endif // ( !defined(LOGGERTYPES_H__) || defined(GENERATE_ENUM_STR_FROM_ID) || defined(GENERATE_ENUM_ID_FROM_STR))

////////////////////////////////////////////////
// TAIL
////////////////////////////////////////////////
// GENERATE_ENUM_MAPS must be defined in a single compilation unit (.cpp) before including this file
#if defined(GENERATE_ENUM_MAPS) && !(defined(GENERATE_ENUM_STR_FROM_ID) || defined(GENERATE_ENUM_ID_FROM_STR))
    #define GENERATE_ENUM_STR_FROM_ID  // Start (Enum -> String) generation
        #include __FILE__ // Generate Map and Functions
    #undef GENERATE_ENUM_STR_FROM_ID // Stop (Enum -> String) generation
    
    #define GENERATE_ENUM_ID_FROM_STR  // Start (String -> Enum) generation
        #include __FILE__ // Generate Map and Functions
    #undef GENERATE_ENUM_ID_FROM_STR // Stop (String -> Enum) generation
#endif // ( !defined(GENERATE_ENUM_STR_FROM_ID) && !defined(GENERATE_ENUM_ID_FROM_STR) )
#define LOGGERTYPES_H__
```
