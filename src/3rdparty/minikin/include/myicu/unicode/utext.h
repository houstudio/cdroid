#ifndef __UTEXT_H__
#define __UTEXT_H__

#include "unicode/utypes.h"

struct UText {
    const UChar* chars;
    int32_t length;
};
typedef struct UText UText;

#endif
