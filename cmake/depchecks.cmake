include(CheckSymbolExists)
check_symbol_exists(M_PI "math.h" HAVE_M_PI)
check_symbol_exists(M_E "math.h" HAVE_M_E)

if(NOT HAVE_M_PI)
    message(STATUS "M_PI is not defined. Defining it manually.")
    add_definitions(-DM_PI=3.14159265358979323846)  #define M_PI
    add_definitions(-DM_PI_2=1.57079632679489661923) #dfine M_PI_2
endif()

if(NOT HAVE_M_E)
    add_definitions(-DM_E=2.71828182845904523536) #define M_E
endif()
