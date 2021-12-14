// These macros must be set to these specific values in order to
// pass all tests for ndiff-2.0.0.
#define VERSION "2.00"
#define DATE "10-Dec-2000"

// Note that ndiff code uses #if style rather than #ifdef style to
// test the macros below which therefore must be #defined
// to be either true or false.

// Define if the `long double' type works.
#define HAVE_LONG_DOUBLE @HAVE_LONG_DOUBLE@

// Define to `unsigned' if <sys/types.h> doesn't define.
// #define size_t

// Define if you have the ANSI C header files.
//#define STDC_HEADERS @STDC_HEADERS@

// Define if you have the <ctype.h> header file.
#define HAVE_CTYPE_H @HAVE_CTYPE_H@

// Define if you have the <limits.h> header file.
#define HAVE_LIMITS_H @HAVE_LIMITS_H@

// Define if you have the <stddef.h> header file.
#define HAVE_STDDEF_H @HAVE_STDDEF_H@

// Define if you have the <stdio.h> header file.
#define HAVE_STDIO_H @HAVE_STDIO_H@

// Define if you have the <stdlib.h> header file.
#define HAVE_STDLIB_H @HAVE_STDLIB_H@

// Define if you have the <string.h> header file.
#define HAVE_STRING_H @HAVE_STRING_H@

// Define if have the <gmp.h> header file.
#define HAVE_GMP @HAVE_GMP@
