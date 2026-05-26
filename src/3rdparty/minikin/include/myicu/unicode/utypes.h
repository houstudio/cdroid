#ifndef __UTYPES_H__
#define __UTYPES_H__
#include <stdint.h>
#define U_EXPORT2
#define U_CAPI
#define U_CALLCONV

typedef char16_t UChar;
typedef int32_t UChar32;
typedef int8_t UBool;

#ifdef U_DEFINE_FALSE_AND_TRUE
    // Use the predefined value.
#else
    // Default to avoiding collision with non-macro definitions of FALSE & TRUE.
#   define U_DEFINE_FALSE_AND_TRUE 0
#endif

#if U_DEFINE_FALSE_AND_TRUE || defined(U_IN_DOXYGEN)
#ifndef TRUE
/**
 * The TRUE value of a UBool.
 *
 * @deprecated ICU 68 Use standard "true" instead.
 */
#   define TRUE  1
#endif
#ifndef FALSE
/**
 * The FALSE value of a UBool.
 *
 * @deprecated ICU 68 Use standard "false" instead.
 */
#   define FALSE 0
#endif
#endif  // U_DEFINE_FALSE_AND_TRUE

enum UErrorCode {
    /* The ordering of U_ERROR_INFO_START Vs U_USING_FALLBACK_WARNING looks weird
     * and is that way because VC++ debugger displays first encountered constant,
     * which is not the what the code is used for
     */

    U_USING_FALLBACK_WARNING  = -128,   /**< A resource bundle lookup returned a fallback result (not an error) */

    U_ERROR_WARNING_START     = -128,   /**< Start of information results (semantically successful) */

    U_USING_DEFAULT_WARNING   = -127,   /**< A resource bundle lookup returned a result from the root locale (not an error) */

    U_SAFECLONE_ALLOCATED_WARNING = -126, /**< A SafeClone operation required allocating memory (informational only) */

    U_STATE_OLD_WARNING       = -125,   /**< ICU has to use compatibility layer to construct the service. Expect performance/memory usage degradation. Consider upgrading */

    U_STRING_NOT_TERMINATED_WARNING = -124,/**< An output string could not be NUL-terminated because output length==destCapacity. */

    U_SORT_KEY_TOO_SHORT_WARNING = -123, /**< Number of levels requested in getBound is higher than the number of levels in the sort key */

    U_AMBIGUOUS_ALIAS_WARNING = -122,   /**< This converter alias can go to different converter implementations */

    U_DIFFERENT_UCA_VERSION = -121,     /**< ucol_open encountered a mismatch between UCA version and collator image version, so the collator was constructed from rules. No impact to further function */

    U_PLUGIN_CHANGED_LEVEL_WARNING = -120, /**< A plugin caused a level change. May not be an error, but later plugins may not load. */


#ifndef U_HIDE_DEPRECATED_API
    /**
     * One more than the highest normal UErrorCode warning value.
     * @deprecated ICU 58 The numeric value may change over time, see ICU ticket #12420.
     */
    U_ERROR_WARNING_LIMIT,
#endif  // U_HIDE_DEPRECATED_API

    U_ZERO_ERROR              =  0,     /**< No error, no warning. */

    U_ILLEGAL_ARGUMENT_ERROR  =  1,     /**< Illegal argument error */
    
    U_MEMORY_ALLOCATION_ERROR =  7,     /**< Memory allocation error */
};

#   define U_SUCCESS(x) ((x)<=U_ZERO_ERROR)
#   define U_FAILURE(x) ((x)>U_ZERO_ERROR)

#ifndef UPRV_BLOCK_MACRO_BEGIN
#define UPRV_BLOCK_MACRO_BEGIN do
#endif

#ifndef UPRV_BLOCK_MACRO_END
#define UPRV_BLOCK_MACRO_END while (false)
#endif

#endif
